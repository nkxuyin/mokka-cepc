//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for Tesla -      *
//*                                                     *
//* For more information about Mokka, please, go to the *
//*                                                     *
//*  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
//*                                                     *
//* Mokka home page.                                    *
//*                                                     *
//*******************************************************

/*
 * SEtd02.cc ETD Self Scaling Driver for Mokka
 */

// inner radius of disks defined by the radial clearance between the ETD  and ECal-EndCap Plug
// outer radius of disks defined by radial difference from the TPC outer radius
// z positions are given by the distance between the last ETD sensitive layer and the ECal-EndCap face plus layer separation and thickness 

// October 15th 2008, Steve Aplin using description from SiLC Collaboration

 
#include "Control.hh" 
#include "SEtd02.hh"
#include "TRKSD00.hh"
#include "G4PVPlacement.hh"
#include "CGAGeometryManager.hh"

#include "G4Tubs.hh"
#include "G4VisAttributes.hh"
#include "G4LogicalVolume.hh"
#include "G4RotationMatrix.hh"

#include "MySQLWrapper.hh"
#include <assert.h>

#include "globals.hh"
#include "CGADefs.h"

#ifdef MOKKA_GEAR

#include "gearimpl/GearParametersImpl.h"
#include "MokkaGear.h"
#include "G4EmCalculator.hh"
#include "G4ParticleTable.hh"
#endif 

INSTANTIATE(SEtd02)

G4bool SEtd02::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog) {

  const G4double start_phi = 0.0*deg;
  const G4double stop_phi = 360.0*deg;
  
  G4PVPlacement *Phys ;
  G4VisAttributes * VisAtt ;

  const G4double TPC_outer_radius = env.GetParameterAsDouble("TPC_outer_radius");
  const G4double Ecal_endcap_zmin = env.GetParameterAsDouble("Ecal_endcap_zmin");
  const G4double ECal_endcap_center_box_size = env.GetParameterAsDouble("Ecal_endcap_center_box_size");
  const G4double ECal_EndCap_Plug_MaxR = 0.5 * sqrt( 2 * ECal_endcap_center_box_size * ECal_endcap_center_box_size);


  ETDMat = CGAGeometryManager::GetMaterial("silicon_2.33gccm");
  SupportMat = CGAGeometryManager::GetMaterial("graphite");

  db = new Database(env.GetDBName());
  //... db common_parameters
  db->exec("select * from ETD;");
  db->getTuple();



  // inner radius defined radial clearance between the ETD  and ECal-EndCap Plug
  const G4double inner_radius = ECal_EndCap_Plug_MaxR - db->fetchDouble("etd_ecalplug_radial_clearance") *mm;

  // outer radius defined by radial difference to the TPC outer radius
  const G4double outer_radius = TPC_outer_radius +  db->fetchDouble("etd_tpcOuterR_radial_diff");

  // z positions are given by the distance between the last ETD sensitive layer and the ECal-EndCap face plus layer separation and thickness 
  const G4double Etd3_ECalEndCap_distance_z = db->fetchDouble("etd3_ecalendcap_distance_z") *mm;
  const G4double layer_separation_z = db->fetchDouble("layer_separation_z");
  sensitive_thickness = db->fetchDouble("sensitive_thickness") *mm;
  support_thickness = db->fetchDouble("support_thickness") *mm;

  //... The ETD Sensitive Detector
  //... Threshold is 20% of a MIP. For Si we have 340 KeV/mm as MIP.
  theETDSD =  new TRKSD00("ETD", sensitive_thickness * mm * 340 * keV * 0.2);

  RegisterSensitiveDetector(theETDSD);


  // build the disk volumes 

  //... Support
  G4Tubs *ETDSupportSolid = new G4Tubs("ETDSupport", inner_radius, outer_radius, 0.5*support_thickness, start_phi, stop_phi);
  
  VisAtt = new G4VisAttributes(G4Colour(0.5,0.5,0.5));
  VisAtt->SetForceWireframe(true);

  G4LogicalVolume *ETDSupportLogical= new G4LogicalVolume(ETDSupportSolid, SupportMat, "ETDSupport", 0, 0, 0);

  ETDSupportLogical->SetVisAttributes(VisAtt);

  //... Sensitive layer (Si)
  G4Tubs *ETDSolid = new G4Tubs("ETD", inner_radius, outer_radius, 0.5*sensitive_thickness, start_phi, stop_phi);

  G4LogicalVolume *ETDLogical = new G4LogicalVolume(ETDSolid, ETDMat, "ETD", 0, 0, 0);

  VisAtt = new G4VisAttributes(G4Colour(0.,0.,0.75)); 
  VisAtt->SetForceWireframe(true);

  ETDLogical->SetVisAttributes(VisAtt);
  ETDLogical->SetSensitiveDetector(theETDSD);

  // place support disk furthest from the IP first, then build the others from that

  for(int i = 0; i<3; ++i) {
	
    G4double zsensitive = Ecal_endcap_zmin - Etd3_ECalEndCap_distance_z - (i*layer_separation_z);

    inner_radiusVec.push_back(inner_radius);
    outer_radiusVec.push_back(outer_radius);
    zVec.push_back(zsensitive);

    G4int disk_number = 3-i;

    // +z side 
    // first sensitive
    Phys = new G4PVPlacement 
      (0, G4ThreeVector(0., 0., zsensitive), ETDLogical, "ETD", worldLog, false, disk_number, false);
    // then support
    Phys = new G4PVPlacement 
      (0, G4ThreeVector(0., 0., zsensitive + 0.5*sensitive_thickness + 0.5*support_thickness),ETDSupportLogical, "ETDSupport", worldLog, false, 0, false);
	
    // -z side
    // first sensitive
    Phys = new G4PVPlacement 
      (0, G4ThreeVector(0., 0., -zsensitive), ETDLogical, "ETD", worldLog, false, -disk_number, false);
    // then support
    Phys = new G4PVPlacement 
      (0, G4ThreeVector(0., 0., -zsensitive - 0.5*sensitive_thickness - 0.5*support_thickness),ETDSupportLogical, "ETDSupport", worldLog, false, 0, false);

	 G4cout << "SEtd02:  Disk:" << disk_number
		   << "\t z = " << zsensitive
		   << "\t inner_radius = " << inner_radius
		   << "\t outer_radius = " << outer_radius
		   << G4endl;

  }


  // Closes Database connection
  delete db;
  db = 0;
     
  return true;

}


#ifdef MOKKA_GEAR

void SEtd02::GearSetup()
{  
  G4double CurrentdEdx, ETDLayer_RadLen, ETD_dEdx;
  G4EmCalculator findDEdx;
  G4ParticleTable* theParticleTable=G4ParticleTable::GetParticleTable();
  
  ETDLayer_RadLen = ETDMat->GetRadlen();
  
  //Looping over bins in the DEDX table to obtain the mip DEDX 
  //From energy 0.0001MeV to 1000MeV in steps of 10
  G4double step_size=10,step,mindEdx=99999;
  
  for (step=0.0001;step<=1000;step+=step_size){
    CurrentdEdx = 
      findDEdx.ComputeTotalDEDX(step,
				theParticleTable->FindParticle("mu-"),
				ETDMat);
    if(CurrentdEdx<mindEdx)
      {
	mindEdx=CurrentdEdx;
      }
  }
  
  ETD_dEdx=(mindEdx)/1000;
  
  // As we build the disk furthest in z first the zvec is reversed. reorder it here using temp vec
  vector<double>::iterator zIterator;
  std::vector<double> tempZ_vec;
  
  zIterator = zVec.end();
  while( zIterator != zVec.begin() ) {
    --zIterator;
    tempZ_vec.push_back(*zIterator);
  }
  
  gear::GearParametersImpl* gearParameters = new gear::GearParametersImpl;
  
  gearParameters -> setDoubleVals( "ETDLayerRadius_in" , inner_radiusVec ) ;
  gearParameters -> setDoubleVals( "ETDLayerRadius_out" , outer_radiusVec ) ;
  gearParameters -> setDoubleVals( "ETDLayerZ" , tempZ_vec ) ;

  gearParameters -> setDoubleVal( "ETDLayerThickness"        , sensitive_thickness ) ;
  gearParameters -> setDoubleVal( "ETDLayerSupportThickness" , support_thickness ) ;
  gearParameters -> setDoubleVal( "ETDLayer_dEdx"            , ETD_dEdx ) ;
  gearParameters -> setDoubleVal( "ETDLayer_RadLen"          , ETDLayer_RadLen ) ;

  // Write gearParameters to GearMgr
  // Parameters for ETD
  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setGearParameters("ETD", gearParameters ) ;
}

#endif


