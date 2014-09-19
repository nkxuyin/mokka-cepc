// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: FieldX03.cc,v 1.2 2009/05/13 07:30:19 steve Exp $
// $Name: mokka-07-00 $
//
// History:
// - first implementation: F.Gaede, DESY May 2009
//   based on FiledX02.cc by P.Mora de Freitas and A.Vogel
//   using  a 2-dim field map for the solenoid field
//   (including the return field in the yoke)  

#include "FieldX03.hh"
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

#include <cmath>

//AS
#include "G4PropagatorInField.hh"
//Used for displaying Numbers with best units
#include "G4UnitsTable.hh"
//AS END

//#define DEBUGFIELDMAP 1

INSTANTIATE(FieldX03)

G4bool FieldX03::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog)
{
  worldLog = 0; // suppresses a compiler warning, worldLog is not needed
  
  if (Control::BFactor != 1) {
    Control::Log("WARNING: Control::BFactor != 1");
    return false; // Mokka will abort now
  }

  // Gets the nominal magnetic field from environment
  //   G4double modelBField = env.GetParameterAsDouble("Field_nominal_value");

  //   // calculates a bfactor as the Adrian driver was written for 4 T
  //   G4double modelBFactor = modelBField / 4;
  //FG: we dont't need  factor here - the solenoid field will be rescaled with the region->fieldValue
  // specified in the DB table 'magnetic'.
  G4double modelBFactor =  1. ;
//   G4cout << "Nominal magnetic field is " << modelBField << " T.";
//   G4cout << " Applying a factor of " << modelBFactor << " into field tables." << G4endl;
  
  
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
        << "FieldX03: Using " << globalName << " = "
        << currentValue / mm << " mm instead of "
        << assumedValue / mm << " mm" << G4endl;
      usingOffsets = true;
    }
  }
  if (usingOffsets) Control::Log("FieldX03: Be sure you know what you're doing!");

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

#ifdef DEBUGFIELDMAP
    G4cout << " field type : " <<  region.fieldType
 	   << " rMinSqr =  "   <<  region.rMinSqr 
	   << "  - rMaxSqr = " <<  region.rMaxSqr << G4endl ;
#endif


    if (_crossingAngle == 0 && (region.fieldType == kUpstreamQuad || region.fieldType == kDnstreamQuad || region.fieldType == kDID || region.fieldType == kMapDID)) {
      Control::Log("FieldX03: You are trying to build a crossing geometry without a crossing angle.\n"
        "This is probably not what you want - better check your geometry data!");
      return false; // Mokka will abort now
    }
    




    switch (region.fieldType) {
      case kCenterQuad:
      case kUpstreamQuad:
      case kDnstreamQuad:
        region.fieldValue = db->fetchDouble("fieldValue") *modelBFactor * tesla / m;
        region.fieldMap = 0; // unused
        break;
      case kSolenoid:
      case kDID:
        region.fieldValue = db->fetchDouble("fieldValue") *modelBFactor * tesla;
        region.fieldMap = 0; // unused
        break;
      case kMapSolenoid:
        region.fieldValue = db->fetchDouble("fieldValue"); // scaling factor for given data
        region.fieldMap = new FieldX03MapSolenoid(db->fetchString("fieldData").data(),modelBFactor);
        break;
      case kMapDID:
        region.fieldValue = db->fetchDouble("fieldValue"); // scaling factor for given data
        region.fieldMap = new FieldX03MapDID(db->fetchString("fieldData").data(),modelBFactor);
        break;
      default:
        Control::Log("FieldX03: Unimplemented \"fieldType\" code.");
        return false; // fatal failure
    }
    _fieldStorage.push_back(region);
  }
  delete db;

  G4FieldManager *fieldMgr = G4TransportationManager::GetTransportationManager()->GetFieldManager();
  fieldMgr->SetDetectorField(this);
  fieldMgr->CreateChordFinder(this);

//Inspired from AS: Limit the length of the steps in the region
G4double FieldPropagator_LargestAcceptableStep = 
	env.GetParameterAsDouble("FieldPropagator_LargestAcceptableStep") * m;

if(FieldPropagator_LargestAcceptableStep > 0)
{  
  G4PropagatorInField* propMgr = G4TransportationManager::GetTransportationManager()->GetPropagatorInField();

  //Largest acceptable Step is 1km by default
  G4cout << "Largest acceptable Step was " << G4BestUnit(propMgr->GetLargestAcceptableStep(),"Length") << G4endl;
  propMgr->SetLargestAcceptableStep( FieldPropagator_LargestAcceptableStep );
  G4cout << "Largest acceptable Step is  " << G4BestUnit(propMgr->GetLargestAcceptableStep(),"Length") << G4endl;
}
//AS END

  // -----  check the field magnitude at the origin  ---------------------------
  double pos[4]={0,0,0,0};
  double b_field[3];
  GetFieldValue(pos,b_field);
  double Bz = b_field[2]/tesla ;
  
  G4cout  << " FieldX03: field at (0,0,0) : " 
	  << b_field[0]/tesla << ", " 
	  << b_field[1]/tesla << ", "
	  << b_field[2]/tesla << G4endl ;
  
  // Gets the nominal magnetic field from environment
  G4double modelBField = env.GetParameterAsDouble("Field_nominal_value");

  if(  std::abs( 1. - Bz / modelBField ) > 0.01 )
    G4cout  << " ##################################################################### " << G4endl 
	    << " WARNING : Field_nominal_value : " << modelBField 
	    << " differs more than 1% from actual value of field at origin ! " << G4endl 
	    << " The field maps might need some re-scalling in the DB table 'magnetic'." << G4endl 
	    << " ##################################################################### " << G4endl 
	    << G4endl ;



#ifdef MOKKA_GEAR
  MokkaGear* gearMgr = MokkaGear::getMgr() ; 
  gear::Vector3D b_vect(b_field[0]/ tesla,b_field[1]/tesla,b_field[2]/tesla);
  gear::ConstantBField* magfield = new gear::ConstantBField(b_vect);
  gearMgr->setBField(magfield);
#endif

  return true;
}

void FieldX03::GetFieldValue(const double point[4], double *bField) const
{
  G4ThreeVector field = G4ThreeVector(0, 0, 0); // default return value
  const G4bool mirror = (point[2] < 0); // are we inside the mirrored part with z < 0?

//   G4cout << " GetFieldValue  at : " << point[0] << "," << point[1] << ","<< point[2] << G4endl ;

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
    
//     G4cout << " rSqr : " << rSqr << " ---- " << region->rMinSqr << " " << region->rMaxSqr << G4endl ;
    

    if (z >= region->zMin && z < region->zMax && rSqr >= region->rMinSqr && rSqr < region->rMaxSqr) {
      G4ThreeVector fieldPart = G4ThreeVector(0, 0, 0);

//       G4cout << " found field of type " << region->fieldType << G4endl ;

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

	  //	  fieldPart *= 1.75 ; //1.5 ;

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

FieldX03MapSolenoid::FieldX03MapSolenoid(G4String tableName, G4double theModelBFactor) {
  
  //FIXME: need to make dbName a parameter (could be part of tableName ?)
  Database *fieldMapDB = new Database("fieldmaps00");
  
#ifdef DEBUGFIELDMAP
  Control::Log( G4String("SELECT * FROM `" + tableName + "_Desc`;")  ) ;
#endif

  //------ read description table first ------------------------------------
  fieldMapDB->exec(G4String("SELECT * FROM `" + tableName + "_Desc`;").data());
  
  if (!fieldMapDB->getTuple())
    Control::Abort("No data in field map description table",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
  
  _rhoMin  = fieldMapDB->fetchDouble("rhoMin") * mm ;
  _rhoMax  = fieldMapDB->fetchDouble("rhoMax") * mm ;
  _drho    = fieldMapDB->fetchDouble("drho")   * mm ;
  _zMin  = fieldMapDB->fetchDouble("zMin") * mm ;
  _zMax  = fieldMapDB->fetchDouble("zMax") * mm ;
  _dz    = fieldMapDB->fetchDouble("dz")   * mm ;  
  
  _nrho = int( (_rhoMax - _rhoMin) / _drho ) + 1 ; 
  _nz =   int( (_zMax - _zMin) / _dz ) + 1 ; 
  
  _Brho.resize( _nz * _nrho ) ;
  _Bz.resize(  _nz * _nrho ) ;
  
#ifdef DEBUGFIELDMAP
  G4cout << " _rhoMin: " << _rhoMin << G4endl 
	 << " _rhoMax:  " << _rhoMax << G4endl 
	 << " _drho: "  << _drho   << G4endl 
	 << " _zMin: "   << _zMin   << G4endl 
	 << " _zMax: "  << _zMax   << G4endl 
	 << " _dz: "    << _dz     << G4endl  
	 << " _nrho: "  << _nrho   << G4endl 
	 << " _nz: "    << _nz     << G4endl  ;
#endif    


  //------ read field map data ----------------------------------------------
  
  fieldMapDB->exec(G4String("SELECT * FROM `" + tableName + "`;").data());
  
  for (G4int i = 0; i < _nrho ; i++) {
    for (G4int j = 0; j < _nz ; j++) {
      
      if (!fieldMapDB->getTuple())
	Control::Abort("Premature end of field map data.",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
      
      const G4double rho = fieldMapDB->fetchDouble("rho_mm") * mm;
      const G4double z   = fieldMapDB->fetchDouble("z_mm") * mm;
      
      //      const G4double Brho = fieldMapDB->fetchDouble("Brho") * theModelBFactor * tesla;
      // --- fixme: rename in database
      const G4double Brho = fieldMapDB->fetchDouble("Brho") * theModelBFactor * tesla;
      const G4double Bz   = fieldMapDB->fetchDouble("Bz") * theModelBFactor * tesla;
      
      if (G4int( (rho-_rhoMin) / _drho + 0.5) != i) {
	G4cout << "Expected rho = " << (_rhoMin + i*_drho)/ mm << "mm , but got rho = " << rho / mm << " mm" << G4endl;
	Control::Abort(G4String("Table \"" + tableName + "\" has bad format.").data(),MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
      }
      if (G4int( (z-_zMin) / _dz + 0.5) != j) {
	G4cout << "Expected z = " << (_zMin+j*_dz) / mm << "mm , but got z = " << z / mm << " mm" << G4endl;
	Control::Abort(G4String("Table \"" + tableName + "\" has bad format.").data(),MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
      }
      
      int index = i * _nz + j  ;
      
      _Brho[ index ] = Brho ;
      _Bz[   index ] = Bz  ;
      
#ifdef DEBUGFIELDMAP
      if( i == 0 && j < 20 ) 
	G4cout << " i,j: " << i<<", " <<j 
	       << " Brho :"  << _Brho[ index ] / theModelBFactor / tesla
	       << " Bz : " << _Bz[   index ] / theModelBFactor / tesla
	       << " index" << index << G4endl ;

#endif


    }
  }
  
  delete fieldMapDB;
}

void FieldX03MapSolenoid::GetFieldValue(const G4ThreeVector &pos, G4ThreeVector &value) {

  double x =  pos.rho() ;
  double y =  pos.z()  ; // field is symmetric in z

  double phi = pos.phi() ;
  if( y < 0 ) {
    y = -y ;
    phi += M_PI ;
  }
  double field[2] ; 

  // Check that the point is within the defined region 
//   if ( x >= _rhoMin && x <= _rhoMax &&
//        y >= _zMin   && y <= _zMax ) {

  
#ifdef DEBUGFIELDMAP
  G4cout << " ----- GetFieldValue() : " << pos << " x,y: " << x << ", " << y << G4endl ;
#endif

  if ( x >= 0. && x <= _rhoMax &&    // we actually have a field map that starts at (5mm,5mm)
       y >= 0. && y <= _zMax ) {     // so we need to include 0<r<rMin and 0<z<zMin
    
    if( x < _rhoMin )  
      x = _rhoMin;  // for 0<r<rMin use value at rMin
    
    if( y < _zMin )  
      y = _zMin ;  // for 0<z<zMin use value at zMin
   

    // Position of given point within region, normalized to the range
    // [0,1]
    double xfraction = ( x - _rhoMin ) / ( _rhoMax - _rhoMin )  ;
    double yfraction = ( y - _zMin )   / ( _zMax   - _zMin   )  ;
    
    // Need addresses of these to pass to modf below.
    // modf uses its second argument as an OUTPUT argument.
    double xdindex, ydindex ;

    // Position of the point within the cuboid defined by the
    // nearest surrounding tabulated points
    double xlocal = ( std::modf(xfraction*( _nrho - 1), &xdindex));
    double ylocal = ( std::modf(yfraction*( _nz   - 1), &ydindex));
    
    
    // The indices of the nearest tabulated point whose coordinates
    // are all less than those of the given point
    int xindex = static_cast<int>(xdindex);
    int yindex = static_cast<int>(ydindex);


#ifdef DEBUGFIELDMAP
    G4cout << " -----  xdindex  " <<  xindex << " ydindex " <<  yindex 
	   << " xlocal" << xlocal << " ylocal" << ylocal << G4endl ;
#endif

    int index0 =  xindex    * _nz + yindex   ;
    int index1 =  xindex    * _nz + yindex+1 ;
    int index2 = (xindex+1) * _nz + yindex   ;
    int index3 = (xindex+1) * _nz + yindex+1 ;

#ifdef DEBUGFIELDMAP
    G4cout << " index0 " << index0 << " br: " <<  _Brho[index0] << " bz: " <<  _Bz[index0] 
	   << " index1 " << index1 << " br: " <<  _Brho[index1] << " bz: " <<  _Bz[index1]
	   << " index2 " << index2 << " br: " <<  _Brho[index2] << " bz: " <<  _Bz[index2]  
	   << " index3 " << index3 << " br: " <<  _Brho[index3] << " bz: " <<  _Bz[index3] 
	   << G4endl ;
#endif


  field[0] =
      _Brho[index0] * (1-xlocal) * (1-ylocal)  +
      _Brho[index1] * (1-xlocal) *    ylocal   +
      _Brho[index2] *    xlocal  * (1-ylocal)  +
      _Brho[index3] *    xlocal  *    ylocal   ;
    
    field[1] =
      _Bz[index0] * (1-xlocal) * (1-ylocal)  +
      _Bz[index1] * (1-xlocal) *    ylocal   +
      _Bz[index2] *    xlocal  * (1-ylocal)  +
      _Bz[index3] *    xlocal  *    ylocal   ;


  } else{

#ifdef DEBUGFIELDMAP
    G4cout << "    -----> GetFieldValue() oops  "  << G4endl ;
#endif
    field[0] = 0.0;
    field[1] = 0.0;
  }

  
  // compute Bx and By 
  //  double phi = pos.phi() ;
  value[0] = field[0] * sin( phi ) ;
  value[1] = field[0] * cos( phi ) ;
  value[2] = field[1] ;

#ifdef DEBUGFIELDMAP
  G4cout << "    -----> GetFieldValue() : " << value << " field[0]:" << field[0] << G4endl ;
#endif
}



FieldX03MapDID::FieldX03MapDID(G4String tableName, G4double theModelBFactor)
{
  Database *fieldMapDB = new Database("fieldmaps00");

#ifdef DEBUGFIELDMAP
  Control::Log( G4String("SELECT * FROM `" + tableName + "`;")  ) ;
#endif
 
  fieldMapDB->exec(G4String("SELECT * FROM `" + tableName + "`;").data());
  
  for (G4int i = 0; i < MAP_BINS; i++) {
    if (!fieldMapDB->getTuple())
      Control::Abort("Premature end of field map data.",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
    const G4double fieldMapZ = fieldMapDB->fetchDouble("z") * m;
    const G4double fieldMapB = fieldMapDB->fetchDouble("B") * theModelBFactor * tesla;

    if (G4int(fieldMapZ / MAP_STEP + 0.5) != i) {
      G4cout << "Expected z = " << i * MAP_STEP / m << "m , but got z = " << fieldMapZ / m << " m" << G4endl;
      Control::Abort(G4String("Table \"" + tableName + "\" has bad format.").data(),MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
    }
    _fieldMap[i] = fieldMapB;
  }
  
  delete fieldMapDB;
}



void FieldX03MapDID::GetFieldValue(const G4ThreeVector &pos, G4ThreeVector &value)
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
