//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for Tesla -      *
//*                                                     *
//* For more information about Mokka, please, go to the *
//*                                                     *
//* polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
//*                                                     *
//* Mokka home page.                                    *
//*                                                     *
//*******************************************************
//
//
// ETD01.cc
// SILC implementation (V.Saveliev): 
// 2 layers of Si-sensitive + Carbon Composite Support... 
//
#include "Control.hh"
#include "MySQLWrapper.hh"
#include "ETD01.hh"
#include "CGAGeometryManager.hh"

#include "globals.hh"
#include "G4Tubs.hh"
#include "G4Box.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"

#include "CGADefs.h"

#include "TRKSD00.hh"
#include <assert.h>

#include "CGADefs.h"
#include "UserInit.hh"

#ifdef MOKKA_GEAR

#include "gearimpl/GearParametersImpl.h"
#include "MokkaGear.h"
#include "G4EmCalculator.hh"
#include "G4ParticleTable.hh"
#endif 
 
INSTANTIATE(ETD01)

ETD01::~ETD01()
{
  //  if (!theETDSD) delete theETDSD;
}

G4bool ETD01::construct(const G4String &aSubDetectorName,
			 G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding " << aSubDetectorName  << G4endl;

  Database* db = new Database(aSubDetectorName.data());
// 
  G4double start_phi = 0.0*deg;
  G4double stop_phi = 360.0*deg;

  G4PVPlacement *Phys ;
  G4VisAttributes * VisAtt ;
  //VisAtt->SetForceWireframe(true); 
  //VisAtt->SetForceSolid(true);

  //... 3 (XUV): Discs of Sensitive (Si)/Support (CFM) structure

  G4double  inner_radious, outer_radious, half_z, dz; 
  G4String  support_structure_material ;

    db->exec("select * from ETD;");
    db->getTuple();

  //... Sensitive thickness  
    sensitive_thickness = db->fetchDouble("sensitive_thickness") *mm;
  
  //... Support thickness
     support_thickness = UserInit::getInstance()->getDouble("sit_sp_thickness") *mm;
  if ( support_thickness < 1e-100 ){
     std::cout << "No UserInit \"sit_sp_thickness\" defined, using database value as default..." << std::endl;
     support_thickness = db->fetchDouble("support_thickness") *mm ;
  }
  G4cout <<aSubDetectorName <<": Si: " << sensitive_thickness <<" CCM: "<< support_thickness << G4endl;

  //... Support material. 
  // support_structure_material = UserInit::getInstance()->getString("ETD_sp_material");
     support_structure_material = "graphite";
   
  //... The ETD Sensitive Detector
  //... Threshold is 20% of a MIP. For Si we have 340 KeV/mm as MIP.

    theETDSD =  new TRKSD00("ETD", 
		sensitive_thickness * mm 
		* 340 * keV
		* 0.2);

    RegisterSensitiveDetector(theETDSD);

  do 
    {
         G4int layer_id = db->fetchInt("layer_id");
         inner_radious = db->fetchDouble("inner_radious") *mm;
          inner_radiusVec.push_back(inner_radious);
         outer_radious = inner_radious+1185.*mm;
          outer_radiusVec.push_back(outer_radious);
         half_z = db->fetchDouble("half_z") *mm;
          half_zVec.push_back(half_z) ;
         dz = 5.*mm;
          dzVec.push_back(dz) ;

      //... Support
      G4Tubs *ETDSupportSolid
        = new G4Tubs("ETDSupport",
                     inner_radious,
                     outer_radious,
                     support_thickness/2.,
                     start_phi,
                     stop_phi);

      VisAtt = new G4VisAttributes(G4Colour(0.5,0.5,0.5));
      VisAtt->SetForceWireframe(true);
      // VisAtt->SetForceSolid(true);

      G4LogicalVolume *ETDSupportLogical=
        new G4LogicalVolume(ETDSupportSolid,
                            CGAGeometryManager::GetMaterial(support_structure_material),
                            "ETDSupport",
                            0,
                            0,
                            0);

      ETDSupportLogical->SetVisAttributes(VisAtt);

      Phys = new G4PVPlacement(0,
                          G4ThreeVector(0., 0., half_z - dz),
                          ETDSupportLogical,
                          "ETDSupport",
                          WorldLog,
                          false,
                          0,
                          false);

      Phys = new G4PVPlacement(0,
                          G4ThreeVector(0., 0., half_z),
                          ETDSupportLogical,
                          "ETDSupport",
                          WorldLog,
                          false,
                          0,
                          false);

      Phys = new G4PVPlacement(0,
                          G4ThreeVector(0., 0., half_z + dz),
                          ETDSupportLogical,
                          "ETDSupport",
                          WorldLog,
                          false,
                          0,
                          false);

       Phys = new G4PVPlacement(0,
                          G4ThreeVector(0., 0., -half_z - dz),
                          ETDSupportLogical,
                          "ETDSupport",
                          WorldLog,
                          false,
                          0,
                          false);

      Phys = new G4PVPlacement(0,
                          G4ThreeVector(0., 0., -half_z),
                          ETDSupportLogical,
                          "ETDSupport",
                          WorldLog,
                          false,
                          0,
                          false); 

      Phys = new G4PVPlacement(0,
                          G4ThreeVector(0., 0., -half_z + dz),
                          ETDSupportLogical,
                          "ETDSupport",
                          WorldLog,
                          false,
                          0,
                          false); 

      //... Sensitive layers (Si)

      G4Tubs *ETDSolid = new G4Tubs("ETD",
		     inner_radious,
		     outer_radious,
		     sensitive_thickness/2.,
		     start_phi, 
		     stop_phi);

      ETDMat = CGAGeometryManager::GetMaterial("silicon_2.33gccm");

      G4LogicalVolume *ETDLogical=
	new G4LogicalVolume(ETDSolid,
			    ETDMat, 
			    "ETD", 
			    0, 
			    0, 
			    0);

      VisAtt = new G4VisAttributes(G4Colour(0.,0.,0.75)); 
       VisAtt->SetForceWireframe(true);
       // VisAtt->SetForceSolid(true); 
      
      ETDLogical->SetVisAttributes(VisAtt);
      ETDLogical->SetSensitiveDetector(theETDSD);

      Phys = new G4PVPlacement(0,
			G4ThreeVector(0., 0., half_z - dz - (sensitive_thickness+support_thickness)/2),
			ETDLogical,
			"ETD",
			WorldLog,
			false,
			layer_id,
                        false);
      Phys = new G4PVPlacement(0,
			G4ThreeVector(0., 0., half_z - (sensitive_thickness+support_thickness)/2),
			ETDLogical,
			"ETD",
			WorldLog,
			false,
			layer_id,
			false);

       Phys = new G4PVPlacement(0,
			G4ThreeVector(0., 0., half_z + dz - (sensitive_thickness+support_thickness)/2),
			ETDLogical,
			"ETD",
			WorldLog,
			false,
			layer_id,
                        false);

       Phys = new G4PVPlacement(0,
		        G4ThreeVector(0., 0., -half_z - dz + (sensitive_thickness+support_thickness)/2),
			ETDLogical,
			"ETD",
			WorldLog,
			false,
			layer_id,
                        false);
       
       Phys = new G4PVPlacement(0,
			G4ThreeVector(0., 0., -half_z + (sensitive_thickness+support_thickness)/2),
			ETDLogical,
			"ETD",
			WorldLog,
			false,
			layer_id,
                        false);
       
       Phys = new G4PVPlacement(0,
			G4ThreeVector(0., 0., -half_z + dz + (sensitive_thickness+support_thickness)/2),
			ETDLogical,
			"ETD",
			WorldLog,
			false,
			layer_id,
                        false);

        } while(db->getTuple()!=NULL);

      //... Closes Database connection
  delete db;
  db = 0;
  G4cout <<"Driver " <<aSubDetectorName <<": done\n" << G4endl;
  return true;
}

#ifdef MOKKA_GEAR

void ETD01::GearSetup()
{  
  G4double CurrentdEdx, ETDLayer_RadLen, ETD_dEdx;
  G4EmCalculator findDEdx;
  G4ParticleTable* theParticleTable=G4ParticleTable::GetParticleTable();

  ETDLayer_RadLen = ETDMat->GetRadlen();

  //Looping over bins in the DEDX table to obtain the mip DEDX 
  //From energy 0.0001MeV to 1000MeV in steps of 10
  G4double step_size=10,step,mindEdx=99999;
  
  for (step=0.0001;step<=1000;step+=step_size)
    {
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

  gear::GearParametersImpl* gearParameters = new gear::GearParametersImpl;

  gearParameters -> setDoubleVals( "ETDLayerRadius_in" , inner_radiusVec ) ;
  gearParameters -> setDoubleVals( "ETDLayerRadius_out" , outer_radiusVec ) ;
  gearParameters -> setDoubleVals( "ETDLayerHalfLength" , half_zVec ) ;
  gearParameters -> setDoubleVals( "ETDGapLength" , dzVec ) ;

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


