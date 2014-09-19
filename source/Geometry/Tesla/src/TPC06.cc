// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: TPC06.cc,v 1.7 2008/05/07 23:14:41 steve Exp $
// $Name: mokka-07-00 $
//
// History:  
// - modified version of TPC driver by Ties Behnke
// - modified version of TPC02 as TPC03 with selectable chamber gas -- Adrian Vogel, 2005-06-09
// - modified version of TPC03 as TPC04 with limit of step length   -- Adrian Vogel, 2006-02-01
// - introduced self-scalability, no superdriver needed anymore     -- Adrian Vogel, 2006-03-11
// - modified version of TPC04 as TPC05 in order to have full MC
//   information both at entry and exit hits in the TPC ,
//   more realistic central electrode and endplate             -- Predrag Krstonosic, 2006-07-12
// - implemented new GEAR interface -- K. Harder, T. Pinto Jayawardena                2007-07-31

            
#include "TPC06.hh"

#include "TPCSD02.hh"
#include "CGAGeometryManager.hh"
#include "MySQLWrapper.hh"
#include "Control.hh"
#include "CGADefs.h"

#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4PVPlacement.hh"
#include "G4UserLimits.hh"
#include "G4SubtractionSolid.hh"
#include "G4ThreeVector.hh"

#ifdef MOKKA_GEAR
#include "MokkaGear.h"
#include "gear/TPCParameters.h"
#include "gearimpl/TPCParametersImpl.h"
#include "gearimpl/FixedPadSizeDiskLayout.h"
#include "G4EmCalculator.hh"
#include "G4ParticleTable.hh"



#endif



#include <iomanip>
#include <vector>
INSTANTIATE(TPC06)

G4bool TPC06::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog)
{
  const G4double phi1 =   0.0 * deg;
  const G4double phi2 = 360.0 * deg;

  // Geometry parameters from the geometry environment and from the database

  Database *db = new Database(env.GetDBName());
  db->exec("SELECT * FROM `barrel`;");
  db->getTuple();

  rInner             = env.GetParameterAsDouble("TPC_inner_radius") * mm;
  rOuter             = env.GetParameterAsDouble("TPC_outer_radius") * mm;
  drInnerWall        = env.GetParameterAsDouble("TPC_inner_wall_thickness") * mm;
  drOuterWall        = env.GetParameterAsDouble("TPC_outer_wall_thickness") * mm;
  TPCMaxStepLength   = env.GetParameterAsDouble("TPC_max_step_length") * mm;
  padHeight          = env.GetParameterAsDouble("TPC_pad_height") * mm;
  padWidth          = env.GetParameterAsDouble("TPC_pad_width") * mm;


  const G4double drInnerInsensitive = db->fetchDouble("drInnerInsensitive") * mm;
  const G4double drOuterInsensitive = db->fetchDouble("drOuterInsensitive") * mm;
  const G4double dzCathode          = db->fetchDouble("dzCathode") * mm;
  const G4double cathode_cupper     = db->fetchDouble("cathode_cupper") * mm;
  const G4double cathode_mylar      = db->fetchDouble("cathode_mylar") * mm;
  const G4double dzTotal            = env.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");
  dzEndplate         = env.GetParameterAsDouble("TPC_electronics_backend_thickness");


  G4double tracking_tpc_ecal_gap = env.GetParameterAsDouble("Ecal_Tpc_gap") * mm;;
  G4double tracking_region_rmax = rOuter + tracking_tpc_ecal_gap - 0.1*mm; // give 100 micron clearance 

  std::stringstream tracking_region_rmax_as_string;
  tracking_region_rmax_as_string <<  tracking_region_rmax;

  (*Control::globalModelParameters)["tracker_region_rmax"] = tracking_region_rmax_as_string.str();
  (*Control::globalModelParameters)["tracker_region_zmax"] = env.GetParameterAsString("TPC_Ecal_Hcal_barrel_halfZ");


  // Simple calculations and some named constants
  const G4double rInnerGas          = rInner + drInnerWall;
  const G4double rOuterGas          = rOuter - drOuterWall;
  rInnerSensitive    = rInner + drInnerInsensitive;
  rOuterSensitive    = rOuter - drOuterInsensitive;
  dzSensitive        = dzTotal - dzEndplate - dzCathode / 2.0;
  const G4double zSensitive         = dzSensitive / 2.0 + dzCathode / 2.0;
  const G4double zEndplate          = dzTotal - dzEndplate / 2.0;
  numberPadRows = (int)((rOuterSensitive - (rInnerSensitive) )/padHeight) ;

  // Materials which will always be used

  materialAir = CGAGeometryManager::GetMaterial("air");
  materialCu  = CGAGeometryManager::GetMaterial("copper");
  materialAl  = CGAGeometryManager::GetMaterial("aluminium");
  materialGas = CGAGeometryManager::GetMaterial(db->fetchString("chamberGas"));
  materialMylar  = CGAGeometryManager::GetMaterial("mylar");
  materialG10  = CGAGeometryManager::GetMaterial("g10");
  // Material mixture for end of endplate zone 
  db->exec("SELECT * FROM `endplate_mixture`;");
  db->getTuple();
  const G4double fractionG10 = db->fetchDouble("endplate_end_g10_percent") * perCent;
  const G4double fractionAir = db->fetchDouble("endplate_end_air_percent") * perCent;

  if (fabs(fractionG10+ fractionAir - 1) > 1E-06)
    Control::Abort("TPC endplate material fractions do not add up to 100%",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);


  const G4double densityG10 = materialG10->GetDensity() * fractionG10;
  const G4double densityAir = materialAir->GetDensity() * fractionAir;
  const G4double densityTotal =  densityG10 + densityAir;

  G4Material *materialMix = new G4Material("TPC_endplates_mix", densityTotal, 2);
 
  materialMix->AddMaterial(materialG10, densityG10 / densityTotal);
  materialMix->AddMaterial(materialAir, densityAir / densityTotal);

  G4cout << "materialMix->GetDensity() = " << materialMix->GetDensity() / g * cm3 << " g/cm3" << G4endl;
  G4cout << "materialMix->GetRadlen() = " << materialMix->GetRadlen() / mm << " mm" << G4endl;
  //  G4cout << "Fraction of radiation lengths in " << endplate_thickness / mm << " mm of TPC endplate: " << endplate_thickness / materialMix->GetRadlen() << G4endl;


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





  //-------------------------------- cathode construction ----------------------------------------//
  G4Tubs *cathodeSolid = new G4Tubs("TPCCathodeSolid", rInnerGas, rOuterGas, dzCathode / 2, phi1, phi2);
  G4LogicalVolume *cathodeLog = new G4LogicalVolume(cathodeSolid, materialAir, "TPCCathodeLog", 0, 0, 0);
  cathodeLog->SetVisAttributes(cathodeVisAttributes);
  new G4PVPlacement(0, G4ThreeVector(), cathodeLog, "TPCCathode", motherLog, false, 0);

    

   G4Tubs *cathodeCu = new G4Tubs("TPCCathodeCopper", rInnerGas, rOuterGas,cathode_cupper/2, phi1, phi2);
   G4LogicalVolume *cathodeCuLog = new G4LogicalVolume(cathodeCu, materialCu, "TPCCathodeCuLog", 0, 0, 0);

   new G4PVPlacement(0, G4ThreeVector(0.,0.,(dzCathode-cathode_cupper)/2), cathodeCuLog, "TPCCathodePiece",cathodeLog, false, 0);
 new G4PVPlacement(0, G4ThreeVector(0.,0.,-(dzCathode-cathode_cupper)/2), cathodeCuLog, "TPCCathodePiece", cathodeLog, false, 1);

 G4Tubs *cathodeMy = new G4Tubs("TPCCathodeMylar", rInnerGas, rOuterGas,cathode_mylar/2, phi1, phi2);
 G4LogicalVolume *cathodeMyLog = new G4LogicalVolume(cathodeMy, materialMylar, "TPCCathodeMyLog", 0, 0, 0);

   new G4PVPlacement(0, G4ThreeVector(0.,0.,(dzCathode-cathode_mylar)/2-cathode_cupper), cathodeMyLog, "TPCCathodePiece",cathodeLog, false, 0);
 new G4PVPlacement(0, G4ThreeVector(0.,0.,-(dzCathode-cathode_mylar)/2+cathode_cupper), cathodeMyLog, "TPCCathodePiece", cathodeLog, false, 1);



  //-----------------------------------------------------------------------------------------------//

  G4Tubs *endplateSolid = new G4Tubs("TPCEndplateSolid", rInnerGas, rOuterGas, dzEndplate / 2, phi1, phi2);
  G4LogicalVolume *endplateLog = new G4LogicalVolume(endplateSolid, materialAir, "TPCEndplateLog", 0, 0, 0);
    endplateLog->SetVisAttributes(wallVisAttributes);
 
  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(  0 * deg), G4ThreeVector(0, 0, +zEndplate)), endplateLog, "TPCEndplate", motherLog, false, 0);
  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(180 * deg), G4ThreeVector(0, 0, -zEndplate)), endplateLog, "TPCEndplate", motherLog, false, 1);

  // TPC Sensitive Detector

  // threshold is twice the energy for Ar ionisation, that is 2 * 16 eV.
  
  Ar_IonPotential=(2*16)* eV;
  
  TPCSD02 *sensitiveDetector = new TPCSD02("TPC", Ar_IonPotential, Control::TPCCut);
  RegisterSensitiveDetector(sensitiveDetector);

  G4UserLimits *userLimits = new G4UserLimits(TPCMaxStepLength);

  G4Tubs * solid1 = new G4Tubs("Solid1", rInnerSensitive, rOuterSensitive, dzSensitive / 2.0, phi1, phi2);

  G4ThreeVector translation(0,0,0);
  G4RotationMatrix rot;
  G4Transform3D transform(rot,translation);

  G4LogicalVolume *sensitiveLog = new G4LogicalVolume(solid1, materialGas, "TPCSensitiveLog", 0, 0, userLimits);

  sensitiveLog->SetVisAttributes(G4VisAttributes::Invisible);
  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(  0 * deg), G4ThreeVector(0, 0, +zSensitive)), sensitiveLog, "TPCSensitive", motherLog, false, 0);
  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(180 * deg), G4ThreeVector(0, 0, -zSensitive)), sensitiveLog, "TPCSensitive", motherLog, false, 1);

  // SJA lets have a go at putting in the pad rows

  for (G4int layer = 0; layer < numberPadRows; layer++) {

    // create twice the number of rings as there are pads, producing an lower and upper part of the pad with the boundry between them the pad-ring centre

    const G4double inner_lowerlayer_radius = rInnerSensitive + (layer * (padHeight));
    const G4double outer_lowerlayer_radius = inner_lowerlayer_radius + (padHeight/2.0);

    const G4double inner_upperlayer_radius = outer_lowerlayer_radius ;
    const G4double outer_upperlayer_radius = inner_upperlayer_radius + (padHeight/2.0);

    G4Tubs *lowerlayerSolid = new G4Tubs("TPC_lowerlayer_solid", inner_lowerlayer_radius, outer_lowerlayer_radius, dzSensitive / 2.0, phi1, phi2);
    G4Tubs *upperlayerSolid = new G4Tubs("TPC_upperlayer_solid", inner_upperlayer_radius, outer_upperlayer_radius, dzSensitive / 2.0, phi1, phi2);

    G4LogicalVolume *lowerlayerLog = new G4LogicalVolume(lowerlayerSolid, materialGas, "TPC_lowerlayer_log", 0, sensitiveDetector, userLimits);
    G4LogicalVolume *upperlayerLog = new G4LogicalVolume(upperlayerSolid, materialGas, "TPC_upperlayer_log", 0, sensitiveDetector, userLimits);

    lowerlayerLog->SetVisAttributes(G4VisAttributes::Invisible);
    upperlayerLog->SetVisAttributes(G4VisAttributes::Invisible);

    new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(  0 * deg), G4ThreeVector(0, 0, 0)), 
		      lowerlayerLog, "TPCLowerLayer", sensitiveLog, false, layer+1);

    new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(  0 * deg), G4ThreeVector(0, 0, 0)), 
		      upperlayerLog, "TPCUpperLayer", sensitiveLog, false, layer+1);

  }

//  G4cout << "number of layers in the TPC = " << numberPadRows << G4endl;
//  G4cout << "layer thickness in the TPC = " <<  padHeight << G4endl;
//  G4cout << "the inner layer of the TPC = " <<  rInnerSensitive << G4endl;
//  G4cout << "final outer ring of the TPC = " <<   rInnerSensitive + (numberPadRows * padHeight)   << G4endl;
//  G4cout << "the outer layer of the TPC = " <<  rOuterSensitive << G4endl;


  // SJA end of pad rows 


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
    if (zCursor > +dzEndplate / 2) Control::Abort("TPC06: Overfull TPC endplate.",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
  }
  // ovde treba dotati dodavanje mixa !!!
  const G4double dzPiece= (dzEndplate/2 -zCursor);
 G4Tubs *pieceSolid = new G4Tubs("TPCEndplateendSolid", rInnerGas, rOuterGas, dzPiece / 2, phi1, phi2);
 G4LogicalVolume *pieceLog = new G4LogicalVolume(pieceSolid, materialMix, "TPCEndplatePieceLog", 0, 0, 0);
  
 pieceLog->SetVisAttributes(G4VisAttributes::Invisible);
new G4PVPlacement(0, G4ThreeVector(0, 0, zCursor + dzPiece / 2), pieceLog, "TPCEndplateEndPiece", endplateLog, false, 0);
  fracRadLength += dzPiece / materialMix->GetRadlen();

  // Some verbose output

  G4cout << "TPC06: Inner radius of the sensitive volume is " << std::setw(4) << rInnerSensitive / mm << " mm." << G4endl;
  G4cout << "TPC06: Outer radius of the sensitive volume is " << std::setw(4) << rOuterSensitive / mm << " mm." << G4endl;
  G4cout << "TPC06: Number of Pad Rows in the TPC  " << std::setw(4) << numberPadRows << G4endl;
  G4cout << "TPC06: Limiting the step length in the TPC to  " << std::setw(4) << TPCMaxStepLength / mm << " mm." << G4endl;
  G4cout << "TPC06: Endplate material corresponds to " << G4int(fracRadLength * 1000) / 10.0 << "% of a radiation length." << G4endl;



  delete db;
  return true;
}

//new routine to ensure all data is obtained before they called to
//be written to the xml file


#ifdef MOKKA_GEAR

void TPC06::GearSetup()
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
      CurrentdEdx= 
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
  const G4double rMax             = rInnerSensitive + (padHeight*numberPadRows);
  const G4int    nRows            = numberPadRows;
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
