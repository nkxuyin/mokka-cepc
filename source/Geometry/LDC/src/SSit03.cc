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
 * SSit03.cc Sit Self Scaling-Driver for Mokka
 */

// Radius of the sensitive layer of the outer layer is defined by the distance to the TPC inner radius
// Radius of the sensitive layer of the inner layer is defined by the relative position between the SIT outer layer and the VTX outer layer
// Half-lengths in z are defined relative to the TPC ECal barrel length

// October 15th 2008, Steve Aplin using description from SiLC Collaboration

 
#include "Control.hh"
#include "SSit03.hh"
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
#include "UserInit.hh"

#ifdef MOKKA_GEAR
#include "gearimpl/GearParametersImpl.h"
#include "MokkaGear.h"
#include "G4EmCalculator.hh"
#include "G4ParticleTable.hh"
#endif 

INSTANTIATE(SSit03)

G4bool SSit03::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog)

{

  G4double start_phi = 0.0*deg;
  G4double stop_phi = 360.0*deg;
  
  G4PVPlacement *Phys ;
  G4VisAttributes *VisAtt ;
  
  G4double VXD_outer_radius = env.GetParameterAsDouble("VXD_outer_radius");
  G4double TPC_inner_radius = env.GetParameterAsDouble("TPC_inner_radius");
  G4double TPC_Ecal_Hcal_barrel_halfZ = env.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");

  db = new Database(env.GetDBName());
  //... db common_parameters
  db->exec("select * from sit;");
  db->getTuple();

  // Sensitive Thickness  
  sensitive_thickness = db->fetchDouble("sensitive_thickness") *mm;
  // Support Thickness
  support_thickness = db->fetchDouble("support_thickness") *mm;

  G4double sit1_sit2_relative_gap = db->fetchDouble("sit1_sit2_relative_gap");
  G4double sit2_tpc_gap = db->fetchDouble("sit2_tpc_gap");

  //... The SIT Sensitive detector
  // Threshold is 20% of a MIP. For Si we have 
  // 340 KeV/mm as MIP.
  theSITSD = 
    new TRKSD00("SIT", 
		sensitive_thickness * mm 
		* 340 * keV
		* 0.2);

  RegisterSensitiveDetector(theSITSD);
    
  db->exec("select * from sit_layers;");
  db->getTuple();

  do 
    {      

      G4int layer_id = db->fetchInt("layer_id");
      G4double half_z(0);
      G4double sensitive_radius(0);
      G4double support_radius(0);

      if(layer_id==1){

	sensitive_radius = VXD_outer_radius + sit1_sit2_relative_gap * ( TPC_inner_radius - sit2_tpc_gap - VXD_outer_radius ) *mm;

	half_z = TPC_Ecal_Hcal_barrel_halfZ *  db->fetchDouble("relative_half_z") *mm;
	std::ostringstream ossradius;
	std::ostringstream osshalfz;
	ossradius << sensitive_radius;
	osshalfz << half_z;
	(*Control::globalModelParameters)["SIT1_Radius"] = ossradius.str();
	(*Control::globalModelParameters)["SIT1_Half_Length_Z"] = osshalfz.str();
      }

      if(layer_id==2){
	sensitive_radius = TPC_inner_radius - sit2_tpc_gap *mm;
	half_z = TPC_Ecal_Hcal_barrel_halfZ *  db->fetchDouble("relative_half_z") *mm;
	std::ostringstream ossradius;
	std::ostringstream osshalfz;
	ossradius << sensitive_radius;
	osshalfz << half_z;
	(*Control::globalModelParameters)["SIT2_Radius"] = ossradius.str();
	(*Control::globalModelParameters)["SIT2_Half_Length_Z"] = osshalfz.str();
      }
	

      inner_radiusVec.push_back(sensitive_radius);

      support_radius = sensitive_radius + 0.5*sensitive_thickness + 0.5*support_thickness;
      support_radiusVec.push_back(support_radius);

      half_zVec.push_back(half_z) ;
      support_half_zVec.push_back(half_z);      

      assert( (sensitive_radius - 0.5*sensitive_thickness)  > VXD_outer_radius && ( support_radius + 0.5*support_thickness ) < TPC_inner_radius );

      std::cout << "SSit03: Layer:" << layer_id
		<< "\t half length = " << half_z
		<< "\t radius of sensitive= " << sensitive_radius
		<< std::endl;
      
      //... Sensitive Cylinders Si
      G4Tubs *SitSolid
	= new G4Tubs("Sit",
		     sensitive_radius - 0.5*sensitive_thickness,
		     sensitive_radius + 0.5*sensitive_thickness,
		     half_z,
		     start_phi, 
		     stop_phi);

      SITMat = CGAGeometryManager::GetMaterial("silicon_2.33gccm");

      G4LogicalVolume *SitLogical=
	new G4LogicalVolume(SitSolid,
			    SITMat, 
			    "Sit", 
			    0, 
			    0, 
			    0);

      VisAtt = new G4VisAttributes(G4Colour(0.,0.,.75)); 
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true); 

      SitLogical->SetVisAttributes(VisAtt);
      SitLogical->SetSensitiveDetector(theSITSD);
      
      Phys=
        new G4PVPlacement(0,
			  G4ThreeVector(0., 0., 0.),
			  SitLogical,
			  "Sit",
			  worldLog,
			  false,
			  layer_id,
			  false);

      //... Sit support Cylinder
      G4Tubs *SitSupportSolid
        = new G4Tubs("SitSupport",
                     support_radius - 0.5*support_thickness,
                     support_radius + 0.5*support_thickness,
                     half_z,
                     start_phi,
                     stop_phi);
                
      VisAtt = new G4VisAttributes(G4Colour(.5,.5,.5));
      VisAtt->SetForceWireframe(true);
      // VisAtt->SetForceSolid(true);

      SupportMat = CGAGeometryManager::GetMaterial("graphite");

      G4LogicalVolume *SitSupportLogical=
	new G4LogicalVolume(SitSupportSolid,
			    SupportMat,
			    "SitSupport",
			    0,
			    0,
			    0);

      SitSupportLogical->SetVisAttributes(VisAtt);

      Phys=
        new G4PVPlacement(0,
                          G4ThreeVector(0., 0., 0.),
                          SitSupportLogical,
                          "SitSupport",
                          worldLog,
                          false,
                          0,
                          false);
      


    } while(db->getTuple()!=NULL);
  
  // Closes Database connection
  delete db;
  db = 0;
  G4cout << "SSit03 done.\n" << G4endl;
     
  return true;
}

#ifdef MOKKA_GEAR

void SSit03::GearSetup()
{
  
  G4double CurrentdEdx;
  G4EmCalculator findDEdx;
  G4ParticleTable* theParticleTable=G4ParticleTable::GetParticleTable();

  G4double SITLayer_RadLen = SITMat->GetRadlen();
  G4double support_RadLen  = SupportMat->GetRadlen();

  //Looping over bins in the DEDX table to obtain the mip DEDX 
  //From energy 0.0001MeV to 1000MeV in steps of 10
  G4double step_size=10,step,mindEdx=99999;
  
  
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx = 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  SITMat);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }
  
  G4double SIT_dEdx=(mindEdx)/1000;
  
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


  G4int model = 1;
  gearParameters -> setIntVal    ("SITModel",model);
  gearParameters -> setDoubleVals( "SITLayerRadius" , inner_radiusVec ) ;
  gearParameters -> setDoubleVals( "SITLayerHalfLength" , half_zVec ) ;
  gearParameters -> setDoubleVals( "SITSupportLayerRadius" , support_radiusVec );
  gearParameters -> setDoubleVals( "SITSupportLayerHalfLength" , support_half_zVec );  
  gearParameters -> setDoubleVal ( "SITLayerThickness" , sensitive_thickness ) ;
  gearParameters -> setDoubleVal ( "SITSupportLayerThickness" , support_thickness ) ;
  gearParameters -> setDoubleVal ( "SITLayer_dEdx" , SIT_dEdx ) ;
  gearParameters -> setDoubleVal ( "SITLayer_RadLen" , SITLayer_RadLen ) ;
  gearParameters -> setDoubleVal ( "SITSupportLayer_dEdx" , support_dEdx ) ;
  gearParameters -> setDoubleVal ( "SITSupportLayer_RadLen" , support_RadLen ) ;

  // Write gearParameters to GearMgr
  // Parameters for SIT
  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setGearParameters("SIT", gearParameters ) ;
}

#endif
