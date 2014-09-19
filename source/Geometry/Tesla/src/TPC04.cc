// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: TPC04.cc,v 1.9 2007/08/14 16:18:34 kristian Exp $
// $Name: mokka-07-00 $
//
// History:  
// - modified version of TPC driver by Ties Behnke
// - modified version of TPC02 as TPC03 with selectable chamber gas -- Adrian Vogel, 2005-06-09
// - modified version of TPC03 as TPC04 with limit of step length   -- Adrian Vogel, 2006-02-01
// - introduced self-scalability, no superdriver needed anymore     -- Adrian Vogel, 2006-03-11
// - implemented new GEAR interface -- K. Harder, T. Pinto Jayawardena               2007-07-31


#include "TPC04.hh"

#include "TPCSD01.hh"
#include "CGAGeometryManager.hh"
#include "MySQLWrapper.hh"
#include "Control.hh"
#include "CGADefs.h"

#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4PVPlacement.hh"
#include "G4UserLimits.hh"

#ifdef MOKKA_GEAR
#include "MokkaGear.h"
#include "gear/TPCParameters.h"
#include "gearimpl/TPCParametersImpl.h"
#include "gearimpl/FixedPadSizeDiskLayout.h"
#include "G4EmCalculator.hh"
#include "G4ParticleTable.hh"


#endif


#include <iomanip>

INSTANTIATE(TPC04)

G4bool TPC04::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog)
{
  const G4double phi1 =   0 * deg;
  const G4double phi2 = 360 * deg;

  // Geometry parameters from the geometry environment and from the database

  Database *db = new Database(env.GetDBName());
  db->exec("SELECT * FROM `barrel`;");
  db->getTuple();

  rInner             = env.GetParameterAsDouble("TPC_inner_radius");
  rOuter             = env.GetParameterAsDouble("TPC_outer_radius");
  drInnerWall        = env.GetParameterAsDouble("TPC_inner_wall_thickness");
  drOuterWall        = env.GetParameterAsDouble("TPC_outer_wall_thickness");
  const G4double drInnerInsensitive = db->fetchDouble("drInnerInsensitive") * mm;
  const G4double drOuterInsensitive = db->fetchDouble("drOuterInsensitive") * mm;

  const G4double dzCathode          = db->fetchDouble("dzCathode") * mm;
  dzEndplate         = env.GetParameterAsDouble("TPC_electronics_backend_thickness");
  const G4double dzTotal            = env.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");

  (*Control::globalModelParameters)["tracker_region_rmax"] = env.GetParameterAsString("TPC_outer_radius");
  (*Control::globalModelParameters)["tracker_region_zmax"] = env.GetParameterAsString("TPC_Ecal_Hcal_barrel_halfZ");

  // Simple calculations and some named constants

  const G4double rInnerGas          = rInner + drInnerWall;
  const G4double rOuterGas          = rOuter - drOuterWall;
  rInnerSensitive    = rInner + drInnerInsensitive;
  rOuterSensitive    = rOuter - drOuterInsensitive;
  dzSensitive        = dzTotal - dzEndplate - dzCathode / 2;
  const G4double zSensitive         = dzSensitive / 2 + dzCathode / 2;
  const G4double zEndplate          = dzTotal - dzEndplate / 2;

  // Materials which will always be used

  materialAir = CGAGeometryManager::GetMaterial("air");
  materialCu  = CGAGeometryManager::GetMaterial("copper");
  materialAl  = CGAGeometryManager::GetMaterial("aluminium");
  materialGas = CGAGeometryManager::GetMaterial(db->fetchString("chamberGas"));

  // Visualisation attributes

  G4VisAttributes *wallVisAttributes = new G4VisAttributes(G4Colour(0.0, 0.5, 0.5)); // dull cyan
  wallVisAttributes->SetForceWireframe(true);
  wallVisAttributes->SetDaughtersInvisible(true);

  G4VisAttributes *cathodeVisAttributes = new G4VisAttributes(G4Colour(0.9, 0.3, 0.1)); // coppery brown
  cathodeVisAttributes->SetForceWireframe(true);

  // Volumes for the whole TPC, Walls, Cathode, and Endplate

  G4Tubs *motherSolid = new G4Tubs("TPCSolid", rInner, rOuter, dzTotal, phi1, phi2);
  G4LogicalVolume *motherLog = new G4LogicalVolume(motherSolid, materialGas, "TPCLog", 0, 0, 0);
  motherLog->SetVisAttributes(G4VisAttributes::Invisible);
  new G4PVPlacement(0, G4ThreeVector(), motherLog, "TPC", worldLog, false, 0);

  G4Tubs *innerWallSolid = new G4Tubs("TPCInnerWallSolid", rInner, rInnerGas, dzTotal, phi1, phi2);
  G4LogicalVolume *innerWallLog = new G4LogicalVolume(innerWallSolid, materialAl, "TPCInnerWallLog", 0, 0, 0);
  innerWallLog->SetVisAttributes(wallVisAttributes);
  new G4PVPlacement(0, G4ThreeVector(), innerWallLog, "TPCInnerWall", motherLog, false, 0);

  G4Tubs *outerWallSolid = new G4Tubs("TPCOuterWallSolid", rOuterGas, rOuter, dzTotal, phi1, phi2);
  G4LogicalVolume *outerWallLog = new G4LogicalVolume(outerWallSolid, materialAl, "TPCOuterWallLog", 0, 0, 0);
  outerWallLog->SetVisAttributes(wallVisAttributes);
  new G4PVPlacement(0, G4ThreeVector(), outerWallLog, "TPCOuterWall", motherLog, false, 0);

  G4Tubs *cathodeSolid = new G4Tubs("TPCCathodeSolid", rInnerGas, rOuterGas, dzCathode / 2, phi1, phi2);
  G4LogicalVolume *cathodeLog = new G4LogicalVolume(cathodeSolid, materialCu, "TPCCathodeLog", 0, 0, 0);
  cathodeLog->SetVisAttributes(cathodeVisAttributes);
  new G4PVPlacement(0, G4ThreeVector(), cathodeLog, "TPCCathode", motherLog, false, 0);

  G4Tubs *endplateSolid = new G4Tubs("TPCEndplateSolid", rInnerGas, rOuterGas, dzEndplate / 2, phi1, phi2);
  G4LogicalVolume *endplateLog = new G4LogicalVolume(endplateSolid, materialAir, "TPCEndplateLog", 0, 0, 0);
  endplateLog->SetVisAttributes(wallVisAttributes);
  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(  0 * deg), G4ThreeVector(0, 0, +zEndplate)), endplateLog, "TPCEndplate", motherLog, false, 0);
  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(180 * deg), G4ThreeVector(0, 0, -zEndplate)), endplateLog, "TPCEndplate", motherLog, false, 1);

  // TPC Sensitive Detector

  // threshold is twice the energy for Ar ionisation, that is 2 * 16 eV.
   
  Ar_IonPotential=(2*15.7596)* eV;

  TPCSD01 *sensitiveDetector = new TPCSD01("TPC",Ar_IonPotential , Control::TPCCut);
  RegisterSensitiveDetector(sensitiveDetector);

  // possible user limits which can be assigned to the logical TPC volume (from G4UserLimits.hh)
  // if database entries are NULL, then use the same default values as in the G4UserLimits constructor
  db->exec("SELECT * FROM `userLimits`;");
  char **result = db->getTuple();
  const G4double maxStep  = (result[0]) ? (db->fetchDouble("maxStep")  * mm) : (DBL_MAX); // max allowed step size in this volume
  const G4double maxTrack = (result[1]) ? (db->fetchDouble("maxTrack") * mm) : (DBL_MAX); // max total track length
  const G4double maxTime  = (result[2]) ? (db->fetchDouble("maxTime")  * s)  : (DBL_MAX); // max time
  const G4double minEkine = (result[3]) ? (db->fetchDouble("minEkine") * eV) : (0);       // min kinetic energy  (only for charged particles)
  const G4double minRange = (result[4]) ? (db->fetchDouble("minRange") * mm) : (0);       // min remaining range (only for charged particles)
  G4UserLimits *userLimits = new G4UserLimits(maxStep, maxTrack, maxTime, minEkine, minRange);

  G4Tubs *sensitiveSolid = new G4Tubs("TPCSensitiveSolid", rInnerSensitive, rOuterSensitive, dzSensitive / 2, phi1, phi2);
  G4LogicalVolume *sensitiveLog = new G4LogicalVolume(sensitiveSolid, materialGas, "TPCSensitiveLog", 0, sensitiveDetector, userLimits);
  sensitiveLog->SetVisAttributes(G4VisAttributes::Invisible);
  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(  0 * deg), G4ThreeVector(0, 0, +zSensitive)), sensitiveLog, "TPCSensitive", motherLog, false, 0);
  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(180 * deg), G4ThreeVector(0, 0, -zSensitive)), sensitiveLog, "TPCSensitive", motherLog, false, 1);

  // Assembly of the TPC Endplate

  G4int pieceCounter = 0;
  G4double fracRadLength = 0;
  G4double zCursor = -dzEndplate / 2;

  db->exec("SELECT * FROM `endplate`;");
  while (db->getTuple()) {
    const G4double dzPiece = db->fetchDouble("dz") * mm;
    G4Material *pieceMaterial = CGAGeometryManager::GetMaterial(db->fetchString("material"));

    G4Tubs *pieceSolid = new G4Tubs("TPCEndplatePieceSolid", rInnerGas, rOuterGas, dzPiece / 2, phi1, phi2);
    G4LogicalVolume *pieceLog = new G4LogicalVolume(pieceSolid, pieceMaterial, "TPCEndplatePieceLog", 0, 0, 0);
    pieceLog->SetVisAttributes(G4VisAttributes::Invisible);
    new G4PVPlacement(0, G4ThreeVector(0, 0, zCursor + dzPiece / 2), pieceLog, "TPCEndplatePiece", endplateLog, false, pieceCounter);

    pieceCounter++;
    fracRadLength += dzPiece / pieceMaterial->GetRadlen();
    zCursor += dzPiece;
    if (zCursor > +dzEndplate / 2) Control::Abort("TPC04: Overfull TPC endplate.",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
  }

  // Some verbose output

  G4cout << "TPC04: Inner radius of the sensitive volume is " << std::setw(4) << rInnerSensitive / mm << " mm." << G4endl;
  G4cout << "TPC04: Outer radius of the sensitive volume is " << std::setw(4) << rOuterSensitive / mm << " mm." << G4endl;
  G4cout << "TPC04: Limiting the step length in the TPC to  " << std::setw(4) << maxStep / mm << " mm." << G4endl;
  G4cout << "TPC04: Endplate material corresponds to " << G4int(fracRadLength * 1000) / 10.0 << "% of a radiation length." << G4endl;

  delete db;
  return true;
}

//new routine to ensure all data is obtained before they called to
//be written to the xml file


#ifdef MOKKA_GEAR

void TPC04::GearSetup()
{
  //Approximate Ion Potential = Ar_IonPotential
  //Divide by 1000 to obtain in GeV
  G4double IonPotential = Ar_IonPotential/1000;

  G4EmCalculator findDEdx;
  G4ParticleTable* theParticleTable=G4ParticleTable::GetParticleTable();
 
  //Radiation Length from the Material database of aluminium
  TPCWallProperties_RadLen = materialAl->GetRadlen();

  //Looping over bins in the DEDX table to obtain the mip DEDX 

  //From energy 0.0001MeV to 1000MeV in steps of 10
  G4double step_size=10,step,mindEdx=99999;
  G4double CurrentdEdx;
   
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx = 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  materialAl);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }
  TPCWallProperties_dEdx=(mindEdx)/1000;

  
  //Obtaining the Radiation Length for Argon from the Materials database
  TPCGasProperties_RadLen = materialGas-> GetRadlen();

 
  mindEdx=99999;
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx=
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  materialGas);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }
  
 
  //the DEDX values in GeV/mm (converted from MeV to GeV)
  TPCGasProperties_dEdx=(mindEdx)/1000;

  // write data to MokkaGear that will print out as XML

  // parameters for PadRowLayout2D
  const G4double rMin             = rInnerSensitive;
  const G4double rMax             = rOuterSensitive;
  const G4double padHeight        = 6.  ; // gear does not allow zero for the pad height
  const G4double padWidth         = 2. ; // gear does not allow zero for the pad width
  const G4int    nRows            = 0;
  const G4double padGap           = 0;

  gear::PadRowLayout2D *padLayout = new gear::FixedPadSizeDiskLayout(rMin, rMax, padHeight, padWidth, nRows, padGap);

  // parameters for TPCParameters
  const G4double maxDriftLength   = dzSensitive;
  const G4double driftVelocity    = 0;
  const G4double readoutFrequency = 0;

  gear::TPCParametersImpl *tpcParameters = new gear::TPCParametersImpl();
  tpcParameters->setPadLayout(padLayout);
  tpcParameters->setMaxDriftLength(maxDriftLength);
  tpcParameters->setDriftVelocity(driftVelocity);
  tpcParameters->setReadoutFrequency(readoutFrequency);

  // set non-standard parameters in map
  tpcParameters -> setDoubleVal( "tpcOuterRadius" , rOuter ) ;
  tpcParameters -> setDoubleVal( "tpcInnerRadius", rInner ) ;
  tpcParameters -> setDoubleVal( "tpcInnerWallThickness", drInnerWall ) ;
  tpcParameters -> setDoubleVal( "tpcOuterWallThickness", drOuterWall ) ;
  tpcParameters -> setDoubleVal( "TPCWallProperties_RadLen",TPCWallProperties_RadLen ) ;
  tpcParameters -> setDoubleVal( "TPCWallProperties_dEdx",TPCWallProperties_dEdx) ;
  tpcParameters -> setDoubleVal( "TPCGasProperties_RadLen",TPCGasProperties_RadLen);
  tpcParameters -> setDoubleVal( "TPCGasProperties_dEdx", TPCGasProperties_dEdx);
  tpcParameters -> setDoubleVal( "tpcIonPotential",IonPotential);
  
  // write TPCParameters to GearMgr
  gear::GearMgr *gearMgr = MokkaGear::getMgr();
  gearMgr->setTPCParameters(tpcParameters);
}
#endif // MOKKA_GEAR
