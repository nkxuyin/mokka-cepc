// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: TPC03.cc,v 1.5 2007/10/17 22:06:09 kristian Exp $
// $Name: mokka-07-00 $
//
// History:  
// - modified version of TPC driver by Ties Behnke
// - modified version of TPC02 with selectable chamber gas -- Adrian Vogel, 2005-06-09
// - implemented new GEAR interface -- K. Harder, T. Pinto Jayawardena  2007-07-31


#include "TPC03.hh"

#include "CGAGeometryManager.hh"
#include "MySQLWrapper.hh"
#include "TRKSD00.hh"
#include "Control.hh"
#include "CGADefs.h"

#include "globals.hh"
#include "G4Tubs.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

#ifdef MOKKA_GEAR
#include "gear/TPCParameters.h"
#include "gearimpl/TPCParametersImpl.h"
#include "gearimpl/FixedPadSizeDiskLayout.h"
#include "MokkaGear.h"
#include "G4EmCalculator.hh"
#include "G4ParticleTable.hh"

#endif


INSTANTIATE(TPC03)

G4bool TPC03::construct(const G4String &dbName, G4LogicalVolume *worldLog)
{
  Database *db = new Database(dbName.data());
  db->exec("SELECT * FROM tpc;");
  db->getTuple();

  // Geometrical Measures

  const G4double phi_start = 0 * deg;
  const G4double phi_span = 360 * deg;

  inner_radius = db->fetchDouble("inner_radius") * mm;
  outer_radius = db->fetchDouble("outer_radius") * mm;
  z_half_length = db->fetchDouble("z_half_length") * mm;

  inner_wall_thickness = db->fetchDouble("inner_wall_thickness") * mm;
  outer_wall_thickness = db->fetchDouble("outer_wall_thickness") * mm;
  endplate_thickness = db->fetchDouble("endplate_thickness") * mm;
  const G4double z_endplate = z_half_length + endplate_thickness / 2;

  number_of_layers = db->fetchInt("number_of_layers");
  inner_sensitive_radius = db->fetchDouble("inner_sensitive_radius") * mm;
  outer_sensitive_radius = db->fetchDouble("outer_sensitive_radius") * mm;
  layer_thickness = (outer_sensitive_radius - inner_sensitive_radius) / number_of_layers;

  const G4double fch_inner_radius = db->fetchDouble("fch_inner_radius") * mm;
  const G4double fch_outer_radius = db->fetchDouble("fch_outer_radius") * mm;
  const G4double fch_thickness = db->fetchDouble("fch_thickness") * mm;
  const G4double z_fch = z_half_length + endplate_thickness + fch_thickness / 2;

  // Materials

  materialAir = CGAGeometryManager::GetMaterial("air");
  materialCu  = CGAGeometryManager::GetMaterial("copper");
  materialKp  = CGAGeometryManager::GetMaterial("kapton");
  materialAl  = CGAGeometryManager::GetMaterial("aluminium");
  materialSi  = CGAGeometryManager::GetMaterial("silicon_2.33gccm");
  materialGas = CGAGeometryManager::GetMaterial(db->fetchString("chamber_gas"));

  // Material Mixture for the TPC Endplates

  const G4double fractionKp  = db->fetchDouble("endplate_kapton_percent") * perCent;
  const G4double fractionCu  = db->fetchDouble("endplate_cu_percent") * perCent;
  const G4double fractionAir = db->fetchDouble("endplate_air_percent") * perCent;

  if (fabs(fractionKp + fractionCu + fractionAir - 1) > 1E-06)
    Control::Abort("TPC endplate material fractions do not add up to 100%",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

  const G4double densityKp  = materialKp->GetDensity()  * fractionKp;
  const G4double densityCu  = materialCu->GetDensity()  * fractionCu;
  const G4double densityAir = materialAir->GetDensity() * fractionAir;
  const G4double densityTotal = densityKp + densityCu + densityAir;

  G4Material *materialMix = new G4Material("TPC_endplates_mix", densityTotal, 3);
  materialMix->AddMaterial(materialKp,  densityKp  / densityTotal);
  materialMix->AddMaterial(materialCu,  densityCu  / densityTotal);
  materialMix->AddMaterial(materialAir, densityAir / densityTotal);

  G4cout << "materialMix->GetDensity() = " << materialMix->GetDensity() / g * cm3 << " g/cm3" << G4endl;
  G4cout << "materialMix->GetRadlen() = " << materialMix->GetRadlen() / mm << " mm" << G4endl;
  G4cout << "Fraction of radiation lengths in " << endplate_thickness / mm << " mm of TPC endplate: " << endplate_thickness / materialMix->GetRadlen() << G4endl;

  // Visualisation Attributes

  G4VisAttributes *wallVisAttributes = new G4VisAttributes(G4Colour(0.0, 0.5, 0.5)); // dull cyan
  wallVisAttributes->SetForceWireframe(true);

  G4VisAttributes *gasVisAttributes = new G4VisAttributes(false, G4Colour(1.0, 0.5, 0.5)); // bright pink
  gasVisAttributes->SetDaughtersInvisible(true);
  gasVisAttributes->SetForceWireframe(true);

  G4VisAttributes *fchVisAttributes = new G4VisAttributes(G4Colour(0.2, 0.0, 0.8)); // rich blue
  fchVisAttributes->SetForceWireframe(true);

  // TPC Vessel

  G4Tubs *motherSolid = new G4Tubs("TPC_mother_solid", inner_radius, outer_radius, z_half_length + endplate_thickness, phi_start, phi_span);
  G4LogicalVolume *motherLog = new G4LogicalVolume(motherSolid, materialAir, "TPC_mother_log", 0, 0, 0);
  motherLog->SetVisAttributes(G4VisAttributes::Invisible);
  new G4PVPlacement(0, G4ThreeVector(), motherLog, "TPC_mother", worldLog, false, 0);

  G4Tubs *innerWallSolid = new G4Tubs("TPC_inner_wall_solid", inner_radius, inner_radius + inner_wall_thickness, z_half_length, phi_start, phi_span);
  G4LogicalVolume *innerWallLog = new G4LogicalVolume(innerWallSolid, materialAl, "TPC_inner_wall_log", 0, 0, 0);
  innerWallLog->SetVisAttributes(wallVisAttributes);
  new G4PVPlacement(0, G4ThreeVector(), innerWallLog, "TPC_inner_wall", motherLog, false, 0);

  G4Tubs *outerWallSolid = new G4Tubs("TPC_outer_wall_solid", outer_radius - outer_wall_thickness, outer_radius, z_half_length, phi_start, phi_span);
  G4LogicalVolume *outerWallLog = new G4LogicalVolume(outerWallSolid, materialAl, "TPC_outer_wall_log", 0, 0, 0);
  outerWallLog->SetVisAttributes(wallVisAttributes);
  new G4PVPlacement(0, G4ThreeVector(), outerWallLog, "TPC_outer_wall", motherLog, false, 0);

  G4Tubs *endplateSolid = new G4Tubs("TPC_endplate_solid", inner_radius, outer_radius, endplate_thickness / 2, phi_start, phi_span);
  G4LogicalVolume *endplateLog = new G4LogicalVolume(endplateSolid, materialMix, "TPC_endplate_log", 0, 0, 0);
  endplateLog->SetVisAttributes(wallVisAttributes);
  new G4PVPlacement(0, G4ThreeVector(0, 0, +z_endplate), endplateLog, "TPC_endplate", motherLog, false, 0);
  new G4PVPlacement(0, G4ThreeVector(0, 0, -z_endplate), endplateLog, "TPC_endplate", motherLog, false, 1);

  // TPC Gas

  // TPC sensitive detector: threshold is twice the energy for Ar ionisation, that is 2 * 15.7596 eV.

  Ar_IonPotential=(2*15.7596)* eV;
  
  theTPCSD = new TRKSD00("TPC",Ar_IonPotential , Control::TPCCut);
  RegisterSensitiveDetector(theTPCSD);

  G4Tubs *gasSolid = new G4Tubs("TPC_gas_solid", inner_radius + inner_wall_thickness, outer_radius - outer_wall_thickness, z_half_length, phi_start, phi_span);
  G4LogicalVolume *gasLog = new G4LogicalVolume(gasSolid, materialGas, "TPC_gas_log", 0, 0, 0);
  gasLog->SetVisAttributes(gasVisAttributes);
  new G4PVPlacement(0, G4ThreeVector(), gasLog, "TPC_gas", motherLog, false, 0);

  for (G4int layer = 0; layer < number_of_layers; layer++) {
    const G4double inner_layer_radius = inner_sensitive_radius + (layer * layer_thickness);
    const G4double outer_layer_radius = inner_layer_radius + layer_thickness;

    G4Tubs *layerSolid = new G4Tubs("TPC_layer_solid", inner_layer_radius, outer_layer_radius, z_half_length, phi_start, phi_span);
    G4LogicalVolume *layerLog = new G4LogicalVolume(layerSolid, materialGas, "TPC_layer_log", 0, theTPCSD, 0);
    layerLog->SetVisAttributes(G4VisAttributes::Invisible);
    new G4PVPlacement(0, G4ThreeVector(), layerLog, "TPC_layer", gasLog, false, layer + 1);
  }

  // FCH (two thin sensitive Si plates, just to register the particle step inside it)

  // FCH sensitive detector: threshold is 20% of a MIP, in Si you get 340 keV/mm for a MIP.
  theFCHSD = new TRKSD00("FCH", 0.2 * 340 * keV/mm * fch_thickness);
  RegisterSensitiveDetector(theFCHSD);

  G4Tubs *fchSolid = new G4Tubs("FCH_solid", fch_inner_radius, fch_outer_radius, fch_thickness / 2, phi_start, phi_span);
  G4LogicalVolume *fchLog = new G4LogicalVolume(fchSolid, materialSi, "FCH_log", 0, theFCHSD, 0);
  fchLog->SetVisAttributes(fchVisAttributes);
  new G4PVPlacement(0, G4ThreeVector(0, 0, +z_fch), fchLog, "FCH", worldLog, false, 1000);
  new G4PVPlacement(0, G4ThreeVector(0, 0, -z_fch), fchLog, "FCH", worldLog, false, 1001);

  delete db;
  return true;
}


//new routine to ensure all data is obtained before they called to
//be written to the xml file

#ifdef MOKKA_GEAR
void TPC03::GearSetup()
{
  // write data to MokkaGear that will print out as xml
  // set parameters for PadRowLayout
   
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


  G4double rMin             = inner_sensitive_radius ;
  G4double rMax             = outer_sensitive_radius ;
  G4double padHeight        = floor(layer_thickness*1000.)/1000. ;
  G4double padWidth         = 2. ; // gear does not allow zero for the pad width
  G4int nRows               = number_of_layers ;
  G4double padGap           = 0 ;

  // set parameters for tpc
  G4double maxDriftLength   = z_half_length ;
  G4double driftVelocity    = 0 ;
  G4double readoutFrequency = 0 ;
  
  // set parameters to PadRowLayout
  gear::PadRowLayout2D* padLayout = 
    new gear::FixedPadSizeDiskLayout( rMin, rMax, padHeight, padWidth, nRows, padGap );
  
  // Set TPCParameters
  gear::TPCParametersImpl* tpcParameters = new gear::TPCParametersImpl;
  tpcParameters -> setPadLayout ( padLayout );
  tpcParameters -> setMaxDriftLength( maxDriftLength );
  tpcParameters -> setDriftVelocity( driftVelocity );
  tpcParameters -> setReadoutFrequency( readoutFrequency );

  // set non-standard parameters in map
  tpcParameters -> setDoubleVal( "tpcOuterRadius" , outer_radius ) ;
  tpcParameters -> setDoubleVal( "tpcInnerRadius", inner_radius ) ;
  tpcParameters -> setDoubleVal( "tpcInnerWallThickness", inner_wall_thickness ) ;
  tpcParameters -> setDoubleVal( "tpcOuterWallThickness", outer_wall_thickness ) ;
  tpcParameters -> setDoubleVal( "TPCWallProperties_RadLen",TPCWallProperties_RadLen ) ;
  tpcParameters -> setDoubleVal( "TPCWallProperties_dEdx",TPCWallProperties_dEdx) ;
  tpcParameters -> setDoubleVal( "TPCGasProperties_RadLen",TPCGasProperties_RadLen);
  tpcParameters -> setDoubleVal( "TPCGasProperties_dEdx", TPCGasProperties_dEdx);
  tpcParameters -> setDoubleVal( "tpcIonPotential",IonPotential);


  // Write tpcParameters to GearMgr
  // Parameters for TPC
  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setTPCParameters( tpcParameters ) ;
}
#endif
