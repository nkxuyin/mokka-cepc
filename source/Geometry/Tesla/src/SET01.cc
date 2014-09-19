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
// SET01.cc
// SiLC implementation (V.Saveliev): 
// 2 Sensitive layers (Si) + Support (Carbon Fiber Material)
// Total Radiation Length 0.275 Si +1 mm CFM = 2x0.65% (1.3%)

#include "Control.hh"
#include "MySQLWrapper.hh"
#include "SET01.hh"
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
 
INSTANTIATE(SET01)

SET01::~SET01()
{
  //  if (!theSETSD) delete theSETSD;
}

G4bool SET01::construct(const G4String &aSubDetectorName,
			 G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding  " << aSubDetectorName << G4endl;

  Database* db = new Database(aSubDetectorName.data());
// 
  G4double start_phi = 0.0*deg;
  G4double stop_phi = 360.0*deg;

  G4PVPlacement *Phys ;
  G4VisAttributes * VisAtt ;
  // VisAtt->SetForceWireframe(true); 
  // VisAtt->SetForceSolid(true);

  // **********************************************
  // Cylindres of Sensitive/Support structure
  // **********************************************

  G4double  inner_radious,half_z; 
  G4String  support_structure_material ;

     db->exec("select * from EST;");
     db->getTuple();

  //... Sensitive Layer (Si)  
     sensitive_thickness = db->fetchDouble("sensitive_thickness") *mm;

  //... Support (Carbon composite material)
     support_thickness = UserInit::getInstance()->getDouble("SET_sp_thickness") *mm;
  if ( support_thickness < 1e-100 ){
     std::cout << "No UserInit \"sit_sp_thickness\" defined, using database value as default..." << std::endl;
     support_thickness = db->fetchDouble("support_thickness") *mm ;
  }

     G4cout << aSubDetectorName <<":  " <<"Si: "      << sensitive_thickness <<" "<<
                                          "support: " << support_thickness   <<G4endl;

  //... Support Layer Material. 
  // support_structure_material = UserInit::getInstance()->getString("SET_sp_material");
     support_structure_material = "graphite";
   
  //... The SET Sensitive Detector (Si)
  //... Threshold is 20% of a MIP. For Si we have 340 KeV/mm as MIP.

     theSETSD = new TRKSD00("SET", 
			    sensitive_thickness * mm 
			    * 340 * keV
			    * 0.2);
     RegisterSensitiveDetector(theSETSD);

     do 
       {
          G4int layer_id = db->fetchInt("layer_id");

   //... Inner R is taken in accordance to 20 mm Gap between TPC and Ecal
       inner_radious = db->fetchDouble("inner_radious") *mm;
       radiusVec.push_back(inner_radious+0.5*sensitive_thickness);

       half_z = db->fetchDouble("half_z") *mm;
       half_zVec.push_back(half_z) ;

   //... Sensitive Layers

       G4Tubs *SETSolid = new G4Tubs("SET",
				     inner_radious,
				     inner_radious+sensitive_thickness,
				     half_z,
				     start_phi, 
				     stop_phi);

      SETMat = CGAGeometryManager::GetMaterial("silicon_2.33gccm");

      G4LogicalVolume *SETLogical = new G4LogicalVolume(SETSolid,
							SETMat, 
							"SET", 
							0, 
							0, 
							0);

      VisAtt = new G4VisAttributes(G4Colour(0.,0.,0.75)); 
      VisAtt->SetForceWireframe(true);
      // VisAtt->SetForceSolid(true); 

      SETLogical->SetVisAttributes(VisAtt);
      SETLogical->SetSensitiveDetector(theSETSD);

      Phys = new G4PVPlacement(0,
			       G4ThreeVector(0., 0., 0.),
			       SETLogical,
			       "SET",
			       WorldLog,
			       false,
			       layer_id,
			       false);

   //... Support
      supportRadiusVec.push_back(inner_radious+sensitive_thickness+0.5*support_thickness);

      G4Tubs *SETSupportSolid = new G4Tubs("SETSupport",
					   inner_radious+sensitive_thickness,
					   inner_radious+sensitive_thickness+support_thickness,
					   half_z,
					   start_phi,
					   stop_phi);

      VisAtt = new G4VisAttributes(G4Colour(.5,.5,.5));
      VisAtt->SetForceWireframe(true);
      // VisAtt->SetForceSolid(true);

      SETSupport = CGAGeometryManager::GetMaterial(support_structure_material);
      G4LogicalVolume *SETSupportLogical=
      new G4LogicalVolume(SETSupportSolid,
                          SETSupport,
                          "SETSupport",
                          0,
                          0,
                          0);

      SETSupportLogical->SetVisAttributes(VisAtt);

      Phys = new G4PVPlacement(0,
			       G4ThreeVector(0., 0., 0.),
			       SETSupportLogical,
			       "SETSupport",
			       WorldLog,
			       false,
			       0,
			       false);

    } while(db->getTuple()!=NULL);

      //... Closes Database connection
  delete db;
  db = 0;
  G4cout <<"Driver " <<aSubDetectorName << " done.\n" << G4endl;
  return true;
}

#ifdef MOKKA_GEAR

void SET01::GearSetup()
{
  G4double CurrentdEdx, SETLayer_RadLen, SET_dEdx;
  G4double SETSupport_RadLen, SETSupport_dEdx;
  G4EmCalculator findDEdx;
  G4ParticleTable* theParticleTable=G4ParticleTable::GetParticleTable();

  SETLayer_RadLen = SETMat->GetRadlen();
  SETSupport_RadLen = SETSupport->GetRadlen();

  //... Looping over bins in the DEDX table to obtain the mip DEDX 
  //... From energy 0.0001MeV to 1000MeV in steps of 10

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

  SET_dEdx=(mindEdx)/1000;

  mindEdx=99999;
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx = 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  SETSupport);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }

  SETSupport_dEdx=(mindEdx)/1000;


  gear::GearParametersImpl* gearParameters = new gear::GearParametersImpl;

  gearParameters -> setDoubleVals( "SETLayerRadius" , radiusVec ) ;
  gearParameters -> setDoubleVals( "SETLayerHalfLength" , half_zVec ) ;
  gearParameters -> setDoubleVals( "SETSupportLayerRadius" , supportRadiusVec ) ;
  gearParameters -> setDoubleVals( "SETSupportLayerHalfLength" , half_zVec ) ;
  gearParameters -> setDoubleVal( "SETLayerThickness" , sensitive_thickness ) ;
  gearParameters -> setDoubleVal( "SETSupportLayerThickness" , support_thickness ) ;
  gearParameters -> setDoubleVal( "SETLayer_dEdx" , SET_dEdx ) ;
  gearParameters -> setDoubleVal( "SETLayer_RadLen" , SETLayer_RadLen ) ;
  gearParameters -> setDoubleVal( "SETSupportLayer_dEdx" , SETSupport_dEdx ) ;
  gearParameters -> setDoubleVal( "SETSupportLayer_RadLen" , SETSupport_RadLen ) ;


  //... Write gearParameters to GearMgr
  //... Parameters for SET
  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setGearParameters("SET", gearParameters ) ;
}

#endif
