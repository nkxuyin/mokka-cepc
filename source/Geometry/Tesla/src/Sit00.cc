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
// $Id: Sit00.cc,v 1.7 2008/07/03 14:35:26 musat Exp $
// $Name: mokka-07-00 $
//
//
// Sit00.cc
//
// History:  
// - first implementation P. Mora de Freitas (sept 02)
// - implemented new GEAR interface -- K. Harder, T. Pinto Jayawardena  2007-07-31


#include "Control.hh"
#include "G4PVPlacement.hh"
#include "Sit00.hh"
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

#ifdef MOKKA_GEAR

#include "gearimpl/GearParametersImpl.h"
#include "MokkaGear.h"
#include "G4EmCalculator.hh"
#include "G4ParticleTable.hh"
#endif 

INSTANTIATE(Sit00)

Sit00::~Sit00()
{
//   if (!theSITSD) delete theSITSD;
}


G4bool Sit00::construct(const G4String &aSubDetectorName,
			 G4LogicalVolume *WorldLog)
{
  G4double start_phi = 0.0*deg;
  G4double stop_phi = 360.0*deg;

  G4VisAttributes * VisAtt = 
    new G4VisAttributes(G4Colour(1,0,0));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);

  G4cout << "\nBuilding SIT..." << G4endl;
  G4cout << "Driver Sit00..." << G4endl;
  db = new Database(aSubDetectorName.data());
  
  //**********************************************
  // Two cylindres in modified Si to keep good X0
  //**********************************************
  G4double  inner_radious,half_z;
  db->exec("select * from sit;");
  db->getTuple();
  thickness = db->fetchDouble("thickness");
  
  // The SIT Sensitive detector
  // Threshold is 20% of a MIP. For Si we have 
  // 340 KeV/mm as MIP.
  theSITSD = 
    new TRKSD00("SIT", 
		thickness * mm 
		* 340 * keV
		* 0.2);
  RegisterSensitiveDetector(theSITSD);

  SITMat = CGAGeometryManager::GetMaterial("silicon_8.72gccm");
  
  do 
    {
      inner_radious = db->fetchDouble("inner_radious");
      half_z = db->fetchDouble("half_z");
#ifdef LCIO_MODE
      inner_radiusVec.push_back(inner_radious);
      half_zVec.push_back(half_z);
#endif
      G4int layer_id = db->fetchInt("layer_id");

      //  tube
      G4Tubs *SitSolid
	= new G4Tubs("Sit",
		     inner_radious,
		     inner_radious+thickness,
		     half_z,
		     start_phi, 
		     stop_phi);

      G4LogicalVolume *SitLogical=
	new G4LogicalVolume(SitSolid,
			    SITMat,
			    "Sit", 
			    0, 
			    0, 
			    0);
      
      SitLogical->SetVisAttributes(VisAtt);
      SitLogical->SetSensitiveDetector(theSITSD);
      
      //      G4PVPlacement *Phys=
      new G4PVPlacement(0,
			G4ThreeVector(0., 0., 0.),
			SitLogical,
			"Sit",
			WorldLog,
			false,
			layer_id);      

    } while(db->getTuple()!=NULL);
  
  // Closes Database connection
  delete db;
  db = 0;
  G4cout << "Sit done.\n" << G4endl;
  return true;
}

#ifdef MOKKA_GEAR

void Sit00::GearSetup()
{
  
  G4double CurrentdEdx, SITLayer_RadLen, SIT_dEdx;
  G4EmCalculator findDEdx;
  G4ParticleTable*theParticleTable=G4ParticleTable::GetParticleTable();

  SITLayer_RadLen = SITMat->GetRadlen();

  //Looping over bins in the DEDX table to obtain the mip DEDX 
  //From energy 0.0001MeV to 10000MeV in steps of 10
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

  gear::GearParametersImpl* gearParameters = new gear::GearParametersImpl;

  G4int model = 0;
  gearParameters -> setIntVal    ("SITModel",model);
#ifdef LCIO_MODE
  gearParameters -> setDoubleVals( "SITLayerRadius" , inner_radiusVec ) ;
  gearParameters -> setDoubleVals( "SITLayerHalfLength" , half_zVec ) ;
#endif
  gearParameters -> setDoubleVal ( "SITLayerThickness" , thickness ) ;
  gearParameters -> setDoubleVal ( "SITLayer_dEdx" , SIT_dEdx ) ;
  gearParameters -> setDoubleVal ( "SITLayer_RadLen" , SITLayer_RadLen ) ;


  // Write gearParameters to GearMgr
  // Parameters for SIT
  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setGearParameters("SIT", gearParameters ) ;
}

#endif
