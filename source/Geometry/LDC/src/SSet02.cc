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
 * SSet02.cc SET Self Scaling-Driver for Mokka
 */
 
// SET is composed of two support backed silicon layers
// The radius of the outer layer is specified by the distance to the min Radius of the ECal Barrel
// The radius of the inner layer is specified by the radial distance between the two layers
// Both layers have the same Half-length as the TPC 

// October 15th 2008 Steve Aplin using input from SiLC Collaboration

#include "Control.hh"
#include "SSet02.hh"
#include "TRKSD00.hh"
#include "G4PVPlacement.hh"
#include "CGAGeometryManager.hh"


#include "G4Cons.hh"
#include "G4Tubs.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
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

INSTANTIATE(SSet02)

G4bool SSet02::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog)

{

  G4double start_phi = 0.0*deg;
  G4double stop_phi = 360.0*deg;
  
  G4PVPlacement *Phys ;
  G4VisAttributes * VisAtt ;
  
  const G4double TPC_outer_radius = env.GetParameterAsDouble("TPC_outer_radius");
  const G4double TPC_Ecal_Hcal_barrel_halfZ = env.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");
  const G4double Ecal_Tpc_gap = env.GetParameterAsDouble("Ecal_Tpc_gap");
  const G4double ECal_min_r = TPC_outer_radius + Ecal_Tpc_gap;

  SETMat = CGAGeometryManager::GetMaterial("silicon_2.33gccm");
  SupportMat = CGAGeometryManager::GetMaterial("graphite");
  
  db = new Database(env.GetDBName());
  //... db common_parameters
  db->exec("select * from SSET;");
  db->getTuple();

  // Sensitive Thickness  
  sensitive_thickness = db->fetchDouble("sensitive_thickness") *mm;
  // Support Thickness
  support_thickness = db->fetchDouble("support_thickness") *mm;

  //... The SET Sensitive detector
  // Threshold is 20% of a MIP. For Si we have 
  // 340 KeV/mm as MIP.
  theSETSD = 
    new TRKSD00("SET", sensitive_thickness * mm * 340 * keV * 0.2);

  RegisterSensitiveDetector(theSETSD);
    
  // build outer layer first

  // radius defined by distance from Rmin of ECal Barrel
  const G4double inner_radius2 = ECal_min_r - db->fetchDouble("distance_set2_ecal_barrel") *mm; 
  inner_radiusVec.push_back(inner_radius2);

  const G4double support_radius2 = inner_radius2 + 0.5*sensitive_thickness + 0.5*support_thickness;
  support_radiusVec.push_back(support_radius2);

  // half length is the same a the TPC
  const G4double half_z2 = TPC_Ecal_Hcal_barrel_halfZ;
  half_zVec.push_back(half_z2) ;
  support_half_zVec.push_back(half_z2);  

  std::cout << "SSet02: Layer:" << 2
	    << "\t half length = " << half_z2
	    << "\t radius of sensitive = " << inner_radius2
	    << "\t radius of support = " << support_radius2
	    << std::endl;

  //... Sensitive Cylinders Si
  G4Tubs *SETSolid2 = new G4Tubs("SET2", inner_radius2, inner_radius2+sensitive_thickness, half_z2, start_phi, stop_phi);

  G4LogicalVolume *SETLogical2 = new G4LogicalVolume(SETSolid2,	SETMat,	"SET2",	0, 0, 0);

  VisAtt = new G4VisAttributes(G4Colour(0.,0.,.75)); 
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true); 

  SETLogical2->SetVisAttributes(VisAtt);
  SETLogical2->SetSensitiveDetector(theSETSD);
      
  Phys = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), SETLogical2, "SET2", worldLog, false, 2, false);

  //... SET2 support Cylinder
  G4Tubs *SET2SupportSolid = new G4Tubs("SET2Support", 
					inner_radius2+sensitive_thickness, 
					inner_radius2+sensitive_thickness+support_thickness,
					half_z2,
					start_phi,
					stop_phi);
                
  VisAtt = new G4VisAttributes(G4Colour(.5,.5,.5));
  VisAtt->SetForceWireframe(true);
  // VisAtt->SetForceSolid(true);

  G4LogicalVolume *SET2SupportLogical = new G4LogicalVolume(SET2SupportSolid, SupportMat, "SET2Support", 0, 0, 0);

  SET2SupportLogical->SetVisAttributes(VisAtt);

  Phys = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), SET2SupportLogical, "SET2Support", worldLog, false, 0, false);

  // now build inner layer 

  const G4double inner_radius1 = inner_radius2 - db->fetchDouble("set_layer_radial_diff") *mm; 
  inner_radiusVec.push_back(inner_radius1);

  const G4double support_radius1 = inner_radius1 + 0.5*sensitive_thickness + 0.5*support_thickness;
  support_radiusVec.push_back(support_radius1);

  const G4double half_z1 = TPC_Ecal_Hcal_barrel_halfZ;
  half_zVec.push_back(half_z1) ;
  support_half_zVec.push_back(half_z1);  

  std::cout << "SSet02: Layer:" << 1
	    << "\t half length = " << half_z1
	    << "\t radius of sensitive = " << inner_radius1
	    << "\t radius of support = " << support_radius1
	    << std::endl;

  //... Sensitive Cylinders Si
  G4Tubs *SET1Solid = new G4Tubs("SET1", inner_radius1, inner_radius1+sensitive_thickness, half_z1, start_phi, stop_phi);

  G4LogicalVolume *SET1Logical= new G4LogicalVolume(SET1Solid, SETMat, "SET1", 0, 0, 0);

  VisAtt = new G4VisAttributes(G4Colour(0.,0.,.75)); 
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true); 

  SET1Logical->SetVisAttributes(VisAtt);
  SET1Logical->SetSensitiveDetector(theSETSD);
      
  Phys = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), SET1Logical, "SET1", worldLog, false, 1, false);

  //... SET1 support Cylinder
  G4Tubs *SET1SupportSolid = new G4Tubs("SET1Support",
					inner_radius1+sensitive_thickness,
					inner_radius1+sensitive_thickness+support_thickness,
					half_z1,
					start_phi,
					stop_phi);
                
  VisAtt = new G4VisAttributes(G4Colour(.5,.5,.5));
  VisAtt->SetForceWireframe(true);
  // VisAtt->SetForceSolid(true);

  G4LogicalVolume *SET1SupportLogical = new G4LogicalVolume(SET1SupportSolid, SupportMat, "SET1Support", 0, 0, 0);

  SET1SupportLogical->SetVisAttributes(VisAtt);

  Phys = new G4PVPlacement(0,G4ThreeVector(0., 0., 0.), SET1SupportLogical, "SET1Support", worldLog, false, 0, false);

  // Closes Database connection
  delete db;
  db = 0;
     
  return true;
}

#ifdef MOKKA_GEAR

void SSet02::GearSetup()
{
  
  G4double CurrentdEdx;
  G4EmCalculator findDEdx;
  G4ParticleTable* theParticleTable=G4ParticleTable::GetParticleTable();

  G4double SETLayer_RadLen = SETMat->GetRadlen();
  G4double support_RadLen  = SupportMat->GetRadlen();

  //Looping over bins in the DEDX table to obtain the mip DEDX 
  //From energy 0.0001MeV to 1000MeV in steps of 10
  G4double step_size=10,step,mindEdx=99999;
  
  
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx = 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  SETMat);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }
  
  G4double SET_dEdx=(mindEdx)/1000;
  
  mindEdx=99999;

  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx = 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  SupportMat);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }

  G4double support_dEdx=(mindEdx)/1000;

  gear::GearParametersImpl* gearParameters = new gear::GearParametersImpl;


  gearParameters -> setDoubleVals( "SETLayerRadius" , inner_radiusVec ) ;
  gearParameters -> setDoubleVals( "SETLayerHalfLength" , half_zVec ) ;
  gearParameters -> setDoubleVals( "SETSupportLayerRadius" , support_radiusVec );
  gearParameters -> setDoubleVals( "SETSupportLayerHalfLength" , support_half_zVec );  
  gearParameters -> setDoubleVal ( "SETLayerThickness" , sensitive_thickness ) ;
  gearParameters -> setDoubleVal ( "SETSupportLayerThickness" , support_thickness ) ;
  gearParameters -> setDoubleVal ( "SETLayer_dEdx" , SET_dEdx ) ;
  gearParameters -> setDoubleVal ( "SETLayer_RadLen" , SETLayer_RadLen ) ;
  gearParameters -> setDoubleVal ( "SETSupportLayer_dEdx" , support_dEdx ) ;
  gearParameters -> setDoubleVal ( "SETSupportLayer_RadLen" , support_RadLen ) ;

  // Write gearParameters to GearMgr
  // Parameters for SET
  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setGearParameters("SET", gearParameters ) ;
}

#endif
