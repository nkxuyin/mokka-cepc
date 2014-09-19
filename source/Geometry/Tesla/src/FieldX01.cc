// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: FieldX01.cc,v 1.4 2007/08/14 16:18:34 kristian Exp $
// $Name: mokka-07-00 $
//
// History:
// - first implementation: Paulo Mora de Freitas, Apr 2001
// - modified for a crossing angle: Adrian Vogel, 2005-05-25
// - modified for field maps with DID as FieldX00: Adrian Vogel, 2005-07-28
// - modified for scaling geometries as FieldX01: Adrian Vogel, 2006-04-20

#include "FieldX01.hh"
#include "MySQLWrapper.hh"
#include "CGAGeometryEnvironment.hh"
#include "Control.hh"

#include "CGADefs.h"
#include "globals.hh"

#include "G4MagneticField.hh"
#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"
#include "G4ThreeVector.hh"

#ifdef MOKKA_GEAR
#include "gearimpl/GearParametersImpl.h"
#include "MokkaGear.h"
#include "gearimpl/Vector3D.h"
#include "gearimpl/ConstantBField.h"
 
#endif


INSTANTIATE(FieldX01)

G4bool FieldX01::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog)
{
  worldLog = 0; // suppresses a compiler warning, worldLog is not needed
  
  if (Control::BFactor != 1) {
    Control::Log("WARNING: Control::BFactor != 1");
    // return false; // Mokka will abort now
  }

  _crossingAngle = env.GetParameterAsDouble("ILC_Main_Crossing_Angle") / 2 * mrad; // only half the angle
  const G4String dbName = env.GetDBName() + "_" + env.GetParameterAsString("ILC_Main_Crossing_Angle");
  Database *db = new Database(dbName.c_str());

  G4bool usingOffsets = false;
  TReferenceMap referenceOffsets;
  db->exec("SELECT * FROM `_references`;");
  while (db->getTuple()) {
    const G4String globalName   = db->fetchString("globalName");
    const G4String localName    = db->fetchString("localName");
    const G4double assumedValue = db->fetchDouble("assumption") * mm;
    const G4double currentValue = env.GetParameterAsDouble(globalName);
    const G4double offsetValue  = currentValue - assumedValue;
    referenceOffsets[localName] = offsetValue;

    if (offsetValue != 0) {
      G4cout
        << "FieldX01: Using " << globalName << " = "
        << currentValue / mm << " mm instead of "
        << assumedValue / mm << " mm" << G4endl;
      usingOffsets = true;
    }
  }
  if (usingOffsets) Control::Log("FieldX01: Be sure you know what you're doing!");

  db->exec("SELECT * FROM `magnetic`;");
  while (db->getTuple()) {
    TFieldRegion region;

    // reference values for r- and z-values
    const G4String rInnerRef    = db->fetchString("rInnerRef");
    const G4String rOuterRef    = db->fetchString("rOuterRef");
    const G4String zStartRef    = db->fetchString("zStartRef");
    const G4String zEndRef      = db->fetchString("zEndRef");
    
    const G4double rInnerOffset = (rInnerRef == "") ? (0) : (referenceOffsets[rInnerRef]);
    const G4double rOuterOffset = (rOuterRef == "") ? (0) : (referenceOffsets[rOuterRef]);
    const G4double zStartOffset = (zStartRef == "") ? (0) : (referenceOffsets[zStartRef]);
    const G4double zEndOffset   = (zEndRef   == "") ? (0) : (referenceOffsets[zEndRef]);
  
    region.fieldType            = EFieldType(db->fetchInt("fieldType"));
    region.zMin                 = db->fetchDouble("zStart")     * mm + zStartOffset;
    region.zMax                 = db->fetchDouble("zEnd")       * mm + zEndOffset;
    region.rMinSqr              = sqr(db->fetchDouble("rInner") * mm + rInnerOffset);
    region.rMaxSqr              = sqr(db->fetchDouble("rOuter") * mm + rOuterOffset);

    if (_crossingAngle == 0 && (region.fieldType == kUpstreamQuad || region.fieldType == kDnstreamQuad || region.fieldType == kDID || region.fieldType == kMapDID)) {
      Control::Log("FieldX01: You are trying to build a crossing geometry without a crossing angle.\n"
        "This is probably not what you want - better check your geometry data!");
      return false; // Mokka will abort now
    }
    
    switch (region.fieldType) {
      case kCenterQuad:
      case kUpstreamQuad:
      case kDnstreamQuad:
        region.fieldValue = db->fetchDouble("fieldValue") * tesla / m;
        region.fieldMap = 0; // unused
        break;
      case kSolenoid:
      case kDID:
        region.fieldValue = db->fetchDouble("fieldValue") * tesla;
        region.fieldMap = 0; // unused
        break;
      case kMapSolenoid:
        region.fieldValue = db->fetchDouble("fieldValue"); // scaling factor for given data
        region.fieldMap = new FieldX01MapSolenoid(db->fetchString("fieldData").data());
        break;
      case kMapDID:
        region.fieldValue = db->fetchDouble("fieldValue"); // scaling factor for given data
        region.fieldMap = new FieldX01MapDID(db->fetchString("fieldData").data());
        break;
      default:
        Control::Log("FieldX01: Unimplemented \"fieldType\" code.");
        return false; // fatal failure
    }
    _fieldStorage.push_back(region);
  }
  delete db;

  G4FieldManager *fieldMgr = G4TransportationManager::GetTransportationManager()->GetFieldManager();
  fieldMgr->SetDetectorField(this);
  fieldMgr->CreateChordFinder(this);

#ifdef MOKKA_GEAR

  MokkaGear* gearMgr = MokkaGear::getMgr() ; 
  double pos[4]={0,0,0,0};
  double b_field[3];
  GetFieldValue(pos,b_field);
  gear::Vector3D b_vect(b_field[0]/ tesla,b_field[1]/tesla,b_field[2]/tesla);
  gear::ConstantBField* magfield = new gear::ConstantBField(b_vect);

  gearMgr->setBField(magfield);
#endif

  return true;
}

void FieldX01::GetFieldValue(const double point[4], double *bField) const
{
  G4ThreeVector field = G4ThreeVector(0, 0, 0); // default return value
  const G4bool mirror = (point[2] < 0); // are we inside the mirrored part with z < 0?

  for (TFieldStorage::const_iterator region = _fieldStorage.begin(); region != _fieldStorage.end(); region++) {
  
    G4double rotateAngle = 0; // default value for kCenterQuad and all solenoid fields
    if      (region->fieldType == kUpstreamQuad) rotateAngle = -_crossingAngle;
    else if (region->fieldType == kDnstreamQuad) rotateAngle = +_crossingAngle;
    if      (mirror)                             rotateAngle = -rotateAngle; // for parts at z < 0
    
    G4ThreeVector pos = G4ThreeVector(point[0], point[1], point[2]).rotateY(-rotateAngle);
    // undo the rotation to get into the local, unrotated coordinate system
    
    const G4double z = fabs(pos.getZ());
    const G4int signZ = (pos.getZ() >= 0) ? (+1) : (-1); // (ignore z = 0 here)
    const G4double rSqr = pos.perp2();
    
    if (z >= region->zMin && z < region->zMax && rSqr >= region->rMinSqr && rSqr < region->rMaxSqr) {
      G4ThreeVector fieldPart = G4ThreeVector(0, 0, 0);
      switch (region->fieldType) {
        case kSolenoid:
          fieldPart.setZ(region->fieldValue);
          break;
        case kDID:
          fieldPart.setX(region->fieldValue * signZ * tan(-_crossingAngle));
          break;
        case kCenterQuad:
        case kUpstreamQuad:
        case kDnstreamQuad:
          fieldPart.setX(region->fieldValue * pos.getY());
          fieldPart.setY(region->fieldValue * pos.getX());
          fieldPart.rotateY(+rotateAngle);
          // redo the rotation to get back into the global coordinate system
          break;
        case kMapSolenoid:
          region->fieldMap->GetFieldValue(pos, fieldPart);
          fieldPart *= region->fieldValue;
          break;
        case kMapDID:
          region->fieldMap->GetFieldValue(pos, fieldPart);
          fieldPart *= region->fieldValue;
          break;
      } // switch (fieldType)
      field += fieldPart;
    } // if (inside)
  } // for (region)
  
  // decompose the G4ThreeVector into a plain array of doubles
  bField[0] = field.getX();
  bField[1] = field.getY();
  bField[2] = field.getZ();
}

FieldX01VMap::FieldX01VMap(G4String tableName)
{
  Database *fieldMapDB = new Database("fieldmaps00");
  fieldMapDB->exec(G4String("SELECT * FROM `" + tableName + "`;").data());
  
  for (G4int i = 0; i < MAP_BINS; i++) {
    if (!fieldMapDB->getTuple())
      Control::Abort("Premature end of field map data.",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
    const G4double fieldMapZ = fieldMapDB->fetchDouble("z") * m;
    const G4double fieldMapB = fieldMapDB->fetchDouble("B") * tesla;

    if (G4int(fieldMapZ / MAP_STEP + 0.5) != i) {
      G4cout << "Expected z = " << i * MAP_STEP / m << "m , but got z = " << fieldMapZ / m << " m" << G4endl;
      Control::Abort(G4String("Table \"" + tableName + "\" has bad format.").data(),MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
    }
    _fieldMap[i] = fieldMapB;
  }
  
  delete fieldMapDB;
}

FieldX01VMap::~FieldX01VMap(void)
{
}

void FieldX01MapSolenoid::GetFieldValue(const G4ThreeVector &pos, G4ThreeVector &value)
{
  const G4int signZ = (pos.getZ() >= 0) ? (+1) : (-1); // neglect the case z = 0 here...
  
  const G4double raw__Index = fabs(pos.getZ() / MAP_STEP); // the exact position in units of bins (fractional)
  const G4int    floorIndex = G4int(raw__Index + 0.0); // the lower neighbouring bin (integer)
  const G4int    roundIndex = G4int(raw__Index + 0.5); // the closest of the two neighbouring bins (integer)
  const G4int    ceil_Index = G4int(raw__Index + 1.0); // the higher neighbouring bin (integer)
  
  if (ceil_Index >= MAP_BINS) return;
  
  const G4double floorWeight = ceil_Index - raw__Index; // how close are we to the lower bin? (one minus distance)
  const G4double ceil_Weight = raw__Index - floorIndex; // how close are we to the higher bin? (one minus distance)
  const G4double Bz = floorWeight * _fieldMap[floorIndex] + ceil_Weight * _fieldMap[ceil_Index]; // interpolation
  
  value.setZ(Bz);
  
  // interpolation of the differential quotient needs two slopes, i.e. three points

  const G4int prev_Index = (roundIndex == 0)            ? (roundIndex) : (roundIndex - 1); // the previous bin
  const G4int next_Index = (roundIndex == MAP_BINS - 1) ? (roundIndex) : (roundIndex + 1); // the next bin
  
  const G4double prev_Weight = (roundIndex - raw__Index) + 0.5; // how close are we to the previous slope?
  const G4double next_Weight = (raw__Index - roundIndex) + 0.5; // how close are we to the next slope?
  
  const G4double prev_dBdz = (_fieldMap[roundIndex] - _fieldMap[prev_Index]) / MAP_STEP; // the previous slope
  const G4double next_dBdz = (_fieldMap[next_Index] - _fieldMap[roundIndex]) / MAP_STEP; // the next slope
  const G4double dBdz = prev_Weight * prev_dBdz + next_Weight * next_dBdz; // interpolation of slopes
  
  value.setX(-0.5 * pos.getX() * dBdz * signZ);
  value.setY(-0.5 * pos.getY() * dBdz * signZ);
}

void FieldX01MapDID::GetFieldValue(const G4ThreeVector &pos, G4ThreeVector &value)
{
  const G4int signZ = (pos.getZ() >= 0) ? (+1) : (-1); // neglect the case z = 0 here...
  
  const G4double raw__Index = fabs(pos.getZ() / MAP_STEP); // the exact position in units of bins (fractional)
  const G4int    floorIndex = G4int(raw__Index + 0.0); // the lower neighbouring bin (integer)
  const G4int    ceil_Index = G4int(raw__Index + 1.0); // the higher neighbouring bin (integer)
  
  if (ceil_Index >= MAP_BINS) return;
  
  const G4double floorWeight = ceil_Index - raw__Index; // how close are we to the lower bin? (one minus distance)
  const G4double ceil_Weight = raw__Index - floorIndex; // how close are we to the higher bin? (one minus distance)
  const G4double Bx = floorWeight * _fieldMap[floorIndex] + ceil_Weight * _fieldMap[ceil_Index]; // interpolation
  
  value.setX(Bx * signZ);
}
