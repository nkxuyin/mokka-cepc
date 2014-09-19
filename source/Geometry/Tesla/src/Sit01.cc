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
// 
// 
// $ $
// $Name: mokka-07-00 $
//
//
// Sit01.cc
// 
//
// Note: 
// New Geometry implemented by Hengne Li @ LAL (21 Jun 2007)
//  - New Geometry:
//     - Sampling structure: support/sensitive
//     - real silicon instead of silicon_8.72gccm
//  - User can modify geometry through Mokka steer file:
//     "/Mokka/init/userInitDouble sit_sp_thickness 1.01"
//     (thickness of support part, in mm)
//     "/Mokka/init/userInitString sit_sp_material beryllium"
//     (material of support part, a string, the name of materials
//      should exist in Mokka material database, if not, Mokka 
//      will report error and exit automatically)
//
//History:
// 
// - implemented new GEAR interface -- K. Harder, T. Pinto Jayawardena  2007-07-31
//
// - GEAR interface modified: "SITLayerSupportThickness" parameter is 
//   added, since this is a useful value for track reconstruction to 
//   get the correct hit position. -- Hengne LI @ LAL 2007-11-30
//
 

#include "Control.hh"
#include "G4PVPlacement.hh"
#include "Sit01.hh"
#include "TRKSD00.hh"

#include "CGAGeometryManager.hh"

#include "globals.hh"

#include "G4Tubs.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

#include "MySQLWrapper.hh"
#include <assert.h>

#include "CGADefs.h"
#include "UserInit.hh"

#ifdef MOKKA_GEAR

#include "gearimpl/GearParametersImpl.h"
#include "MokkaGear.h"
#include "G4EmCalculator.hh"
#include "G4ParticleTable.hh"
#endif 
 

INSTANTIATE(Sit01)

Sit01::~Sit01()
{
//   if (!theSITSD) delete theSITSD;
}


G4bool Sit01::construct(const G4String &aSubDetectorName,
			 G4LogicalVolume *WorldLog)
{
  G4double start_phi = 0.0*deg;
  G4double stop_phi = 360.0*deg;

  G4PVPlacement *Phys ;
  G4VisAttributes * VisAtt ;
  //VisAtt->SetForceWireframe(true); 
  //VisAtt->SetForceSolid(true);


  G4cout << "\nBuilding SIT..." << G4endl;
  G4cout << "Driver Sit01..." << G4endl;
  db = new Database(aSubDetectorName.data());
  
  //**********************************************
  // Two cylindres of Sensitive/Support structure
  //**********************************************
  G4double  inner_radious,half_z; 
  G4String  support_structure_material ;

  db->exec("select * from sit;");
  db->getTuple();

  // Sensitive Part thickness  
  sensitive_thickness = db->fetchDouble("sensitive_thickness") *mm;

  // Support/dead part thickness
  support_thickness = UserInit::getInstance()->getDouble("sit_sp_thickness") *mm;
  if ( support_thickness < 1e-100 ){
     std::cout << "No UserInit \"sit_sp_thickness\" defined, using database value as default..." << std::endl;
     support_thickness = db->fetchDouble("support_thickness") *mm ;
  }
  // Support/dead part material. 
  support_structure_material = UserInit::getInstance()->getString("sit_sp_material");
  if ( support_structure_material == "" ) { 
     std::cout << "No UserInit \"sit_sp_material\" defined, using database value as default..." << std::endl;
     support_structure_material = "beryllium";
     //     support_structure_material = "graphite";
  }

  support_structure_material = "beryllium";
  //  support_structure_material = "graphite";



  // The SIT Sensitive detector
  // Threshold is 20% of a MIP. For Si we have 
  // 340 KeV/mm as MIP.
  theSITSD = 
    new TRKSD00("SIT", 
		sensitive_thickness * mm 
		* 340 * keV
		* 0.2);
  RegisterSensitiveDetector(theSITSD);
  
  do 
    {
      inner_radious = db->fetchDouble("inner_radious") *mm;
      half_z = db->fetchDouble("half_z") *mm;
      G4int layer_id = db->fetchInt("layer_id");
#ifdef LCIO_MODE
      G4double support_radious = inner_radious + 0.5*support_thickness;
      G4double sensitive_radious = inner_radious+support_thickness + 0.5*sensitive_thickness;
      inner_radiusVec.push_back(sensitive_radious);
      half_zVec.push_back(half_z) ;
      support_half_zVec.push_back(half_z);
      support_radiusVec.push_back(support_radious);
#endif


      // Start added by Hengne
      // Sit support tube
      G4Tubs *SitSupportSolid
        = new G4Tubs("SitSupport",
                     inner_radious,
                     inner_radious+support_thickness,
                     half_z,
                     start_phi,
                     stop_phi);
                                                                                                                                                                                 
      VisAtt = new G4VisAttributes(G4Colour(1.,1.,0.));
      //VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);

      SupportMat = CGAGeometryManager::GetMaterial(support_structure_material);
                                                                                                                                                                                    
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
                          WorldLog,
                          false,
                          0,
                          false);
                                                                                                                                                                                   

      // Sensitive tube
      G4Tubs *SitSolid
	= new G4Tubs("Sit",
		     inner_radious+support_thickness,
		     inner_radious+support_thickness+sensitive_thickness,
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

      VisAtt = new G4VisAttributes(G4Colour(1.,0.,0.)); 
      //VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true); 

      
      SitLogical->SetVisAttributes(VisAtt);
      SitLogical->SetSensitiveDetector(theSITSD);
      
      Phys=
        new G4PVPlacement(0,
			G4ThreeVector(0., 0., 0.),
			SitLogical,
			"Sit",
			WorldLog,
			false,
			layer_id,
                        false);
    


    } while(db->getTuple()!=NULL);
  
  // Closes Database connection
  delete db;
  db = 0;
  G4cout << "Sit done.\n" << G4endl;
  return true;
}

#ifdef MOKKA_GEAR

void Sit01::GearSetup()
{
  
  G4double CurrentdEdx, SITLayer_RadLen, SIT_dEdx;
  G4double support_RadLen, support_dEdx;
  G4EmCalculator findDEdx;
  G4ParticleTable* theParticleTable=G4ParticleTable::GetParticleTable();

  SITLayer_RadLen = SITMat->GetRadlen();
  support_RadLen  = SupportMat->GetRadlen();

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

  SIT_dEdx=(mindEdx)/1000;

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

  support_dEdx=(mindEdx)/1000;


  gear::GearParametersImpl* gearParameters = new gear::GearParametersImpl;

  G4int model = 1;
  gearParameters -> setIntVal    ( "SITModel" , model ) ;
#ifdef LCIO_MODE
  gearParameters -> setDoubleVals( "SITLayerRadius" , inner_radiusVec ) ;
  gearParameters -> setDoubleVals( "SITLayerHalfLength" , half_zVec ) ;
  gearParameters -> setDoubleVals( "SITSupportLayerRadius" , support_radiusVec );
  gearParameters -> setDoubleVals( "SITSupportLayerHalfLength" , support_half_zVec );
#endif
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
