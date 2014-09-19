// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: FieldX00.cc,v 1.2 2007/08/14 16:18:34 kristian Exp $
// $Name: mokka-07-00 $
//
// History:
// - first implementation P. Mora de Freitas (apr 01)
// - modified for a crossing angle: Adrian Vogel, 2005-05-25
// - modified for field maps with DID as FieldX00: Adrian Vogel, 2005-07-28

#include "FieldX00.hh"
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

INSTANTIATE(FieldX00)

G4bool FieldX00::ContextualConstruct(const CGAGeometryEnvironment &geometryEnv, G4LogicalVolume *worldLog)
{
  worldLog = 0; // suppresses a compiler warning, worldLog is not needed
  
  if (Control::BFactor != 1) {
    Control::Log("WARNING: Control::BFactor != 1");
    // return false; // Mokka will abort now
  }

  Database *db = new Database(geometryEnv.GetDBName());
  db->exec("SELECT value FROM parameters WHERE name='crossingAngle';");
  if (!db->getTuple())
    Control::Abort("Parameter \"crossingAngle\" not found.",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
  fCrossingAngle = db->fetchDouble("value") / 2 * mrad; // only half the angle

  db->exec("SELECT * FROM magnetic;");
  while (db->getTuple()) {
    TFieldRegion region;

    region.fieldType = EFieldType(db->fetchInt("fieldType"));
    region.zMin = db->fetchDouble("zStart") * mm;
    region.zMax = db->fetchDouble("zEnd") * mm;
    region.rMinSqr = sqr(db->fetchDouble("rInner") * mm);
    region.rMaxSqr = sqr(db->fetchDouble("rOuter") * mm);

    if (fCrossingAngle == 0 && (region.fieldType == kUpstreamQuad || region.fieldType == kDnstreamQuad)) {
      Control::Log("You are trying to build a crossing geometry without a crossing angle.\n"
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
      case kSolenoidDID:
        region.fieldValue = db->fetchDouble("fieldValue") * tesla;
        region.fieldMap = 0; // unused
        break;
      case kMapSolenoid:
      case kMapSolenoidDID:
        region.fieldValue = 0; // unused
        region.fieldMap = new FieldX00Map(geometryEnv.GetDBName(), db->fetchString("fieldData").data());
        break;
    }
    fFieldStorage.push_back(region);
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

void FieldX00::GetFieldValue(const double point[4], double *bField) const
{
  G4ThreeVector field = G4ThreeVector(0, 0, 0); // default return value
  const G4bool mirror = (point[2] < 0); // are we inside the mirrored part with z < 0?

  for (TFieldStorage::size_type i = 0; i < fFieldStorage.size(); i++) {
    TFieldRegion region = fFieldStorage[i];
  
    register G4double tmpRotateAngle = 0; // default value for kCenterQuad and all solenoid fields
    if      (region.fieldType == kUpstreamQuad) tmpRotateAngle = -fCrossingAngle;
    else if (region.fieldType == kDnstreamQuad) tmpRotateAngle = +fCrossingAngle;
    if (mirror) tmpRotateAngle = -tmpRotateAngle; // for parts at z < 0
    const G4double rotateAngle = tmpRotateAngle; // (better make it const now...)
    
    G4ThreeVector pos = G4ThreeVector(point[0], point[1], point[2]).rotateY(-rotateAngle);
    // undo the rotation to get into the local, unrotated coordinate system
    
    const G4double z = fabs(pos.getZ());
    const G4int signZ = (pos.getZ() >= 0) ? (+1) : (-1); // (ignore z = 0 here)
    const G4double rSqr = pos.perp2();
    
    if (z >= region.zMin && z < region.zMax && rSqr >= region.rMinSqr && rSqr < region.rMaxSqr) {
      switch (region.fieldType) {
        case kSolenoid:
          field.setZ(region.fieldValue);
          break;
        case kSolenoidDID:
          field.setX(region.fieldValue * signZ * tan(-fCrossingAngle));
          field.setZ(region.fieldValue);
          break;
        case kCenterQuad:
        case kUpstreamQuad:
        case kDnstreamQuad:
          field.setX(region.fieldValue * pos.getY());
          field.setY(region.fieldValue * pos.getX());
          break;
        case kMapSolenoid:
          region.fieldMap->GetFieldValue(pos, field, false);
          break;
        case kMapSolenoidDID:
          region.fieldMap->GetFieldValue(pos, field, true);
          break;
      } // switch (fieldType)
      field.rotateY(+rotateAngle);
      // redo the rotation to get back into the global coordinate system
      break; // for (region)
    } // if (inside)
  } // for (region)
  
  // decompose the G4ThreeVector into a plain array of doubles
  bField[0] = field.getX();
  bField[1] = field.getY();
  bField[2] = field.getZ();
}

FieldX00Map::FieldX00Map(G4String databaseName, G4String tableName)
{
  Database *fieldMapDB = new Database(databaseName);
  fieldMapDB->exec(G4String("SELECT * FROM " + tableName + ";").data());
  
  for (G4int i = 0; i < MAP_BINS; i++) {
    if (!fieldMapDB->getTuple())
      Control::Abort("Premature end of field map data.",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
    const G4double fieldMapZ  = fieldMapDB->fetchDouble("z") * m;
    const G4double fieldMapBz = fieldMapDB->fetchDouble("Bz") * tesla;
    const G4double fieldMapBx = fieldMapDB->fetchDouble("Bx") * tesla;

    if (G4int(fieldMapZ / MAP_STEP + 0.5) != i) {
      G4cout << "Expected z = " << i * MAP_STEP / m << "m , but got z = " << fieldMapZ / m << " m" << G4endl;
      Control::Abort(G4String("Table \"" + tableName + "\" has bad format.").data(),MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
    }
    fFieldMapBz[i] = fieldMapBz;
    fFieldMapBx[i] = fieldMapBx;
  }
  
  delete fieldMapDB;
}

void FieldX00Map::GetFieldValue(const G4ThreeVector &pos, G4ThreeVector &value, G4bool useDID)
{
  const G4int signZ = (pos.getZ() >= 0) ? (+1) : (-1); // neglect the case z = 0 here...
  
  const G4double raw__Index = fabs(pos.getZ() / MAP_STEP); // the exact position in units of bins (fractional)
  const G4int    floorIndex = G4int(raw__Index + 0.0); // the lower neighbouring bin (integer)
  const G4int    roundIndex = G4int(raw__Index + 0.5); // the closest of the two neighbouring bins (integer)
  const G4int    ceil_Index = G4int(raw__Index + 1.0); // the higher neighbouring bin (integer)
  
  if (ceil_Index >= MAP_BINS) return;
  
  const G4double floorWeight = ceil_Index - raw__Index; // how close are we to the lower bin? (one minus distance)
  const G4double ceil_Weight = raw__Index - floorIndex; // how close are we to the higher bin? (one minus distance)
  const G4double Bz = floorWeight * fFieldMapBz[floorIndex] + ceil_Weight * fFieldMapBz[ceil_Index]; // interpolation
  
  value.setZ(Bz);
  
  // interpolation of the differential quotient needs two slopes, i.e. three points

  const G4int prev_Index = (roundIndex == 0)            ? (roundIndex) : (roundIndex - 1); // the previous bin
  const G4int next_Index = (roundIndex == MAP_BINS - 1) ? (roundIndex) : (roundIndex + 1); // the next bin
  
  const G4double prev_Weight = (roundIndex - raw__Index) + 0.5; // how close are we to the previous slope?
  const G4double next_Weight = (raw__Index - roundIndex) + 0.5; // how close are we to the next slope?
  
  const G4double prev_dBdz = (fFieldMapBz[roundIndex] - fFieldMapBz[prev_Index]) / MAP_STEP; // the previous slope
  const G4double next_dBdz = (fFieldMapBz[next_Index] - fFieldMapBz[roundIndex]) / MAP_STEP; // the next slope
  const G4double dBdz = prev_Weight * prev_dBdz + next_Weight * next_dBdz; // interpolation of slopes
  
  value.setX(-0.5 * pos.getX() * dBdz * signZ);
  value.setY(-0.5 * pos.getY() * dBdz * signZ);
  
  if (useDID) {
    const G4double Bx = floorWeight * fFieldMapBx[floorIndex] + ceil_Weight * fFieldMapBx[ceil_Index];
    value.setX(value.getX() + Bx * signZ);
  }
}
