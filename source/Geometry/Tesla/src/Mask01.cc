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
// $Id: Mask01.cc,v 1.4 2006/02/07 16:50:15 musat Exp $
// $Name: mokka-07-00 $
//
//
// Mask01.cc
//
// History:  
// - first implementation P. Mora de Freitas (apr 01)

#include "Control.hh"
#include "G4PVPlacement.hh"
#include "Mask01.hh"
#include "CGAGeometryManager.hh"

#include "globals.hh"

#include "G4Cons.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"

#include "MySQLWrapper.hh"
#include <assert.h>

#include "CGADefs.h"

INSTANTIATE(Mask01)

G4bool Mask01::construct(const G4String &aSubDetectorName,
			 G4LogicalVolume *WorldLog)
{
  G4VisAttributes * VisAttMask = 
    new G4VisAttributes(G4Colour(.5,.5,.5));
  G4VisAttributes * VisAttElect = 
    new G4VisAttributes(G4Colour(1,1,1));
  
  G4RotationMatrix *rot=new G4RotationMatrix();
  rot->rotateX(pi); // the same but other side

  G4double start_phi,end_phi;
  start_phi = 0.*deg;
  end_phi = 360.*deg;

  G4cout << "\nBuilding Mask..." << G4endl;
  db = new Database(aSubDetectorName.data());
  
  // take the parameters from the database
  db->exec("select * from mask;");
  db->getTuple();
  
  G4double sin_inner_angle,sin_out_angle;
  sin_inner_angle = sin(db->fetchDouble("inner_angle"));
  sin_out_angle = sin(db->fetchDouble("out_angle"));

  G4double fRmin1,fRmin2,fRmax1,fRmax2,fDz;
  fRmin1 = db->fetchDouble("begin_z") * sin_inner_angle;
  fRmin2 = db->fetchDouble("end_z") * sin_inner_angle;
  fRmax1 = db->fetchDouble("begin_z") * sin_out_angle;
  fRmax2 = db->fetchDouble("end_z") * sin_out_angle;
  fDz =    (db->fetchDouble("end_z") - db->fetchDouble("begin_z"))/2. ;
  
  G4double  electronics_start_z, electronics_stop_z,
    electronics_thickness ;
  electronics_start_z = db->fetchDouble("electronics_start_z");
  electronics_stop_z = db->fetchDouble("electronics_stop_z");
  electronics_thickness = db->fetchDouble("electronics_thickness");
  
  //*******************************
  // W solid cones
  //*******************************
  G4Cons *MASKSolid
    = new G4Cons("MASKSolid",
		 fRmin1, fRmax1,
		 fRmin2, fRmax2,
		 fDz,
		 start_phi,
		 end_phi);
  
  //VisAttMask->SetForceWireframe(true);
  VisAttMask->SetForceSolid(true);
  
  G4LogicalVolume *MASKLogical=
    new G4LogicalVolume(MASKSolid,
			CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
			"MASKLogical1", 
			0, 
			0, 
			0);
  
  MASKLogical->SetVisAttributes(VisAttMask);

  G4double ZMask = db->fetchDouble("begin_z") + fDz;
  
  G4PVPlacement *MASKPhys;
  MASKPhys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., ZMask),
		      MASKLogical,
		      "MASKPhysical",
		      WorldLog,
		      false,0);
  MASKPhys=
    new G4PVPlacement(rot,
		      G4ThreeVector(0., 0., -ZMask),
		      MASKLogical,
		      "MASKPhysical",
		      WorldLog,
		      false,0);

  //*******************************
  // Electronics on the masks
  //*******************************
  G4double Elect_half_z;
  Elect_half_z = 
    (electronics_stop_z - electronics_start_z)/2.;
  assert (Elect_half_z>0);

  fRmax1 = electronics_start_z * sin_out_angle;
  fRmax2 = electronics_stop_z * sin_out_angle;

  G4Cons *ElectSolid
    = new G4Cons("ElectSolid",
		 fRmax1, fRmax1+electronics_thickness,
		 fRmax2, fRmax2+electronics_thickness,
		 Elect_half_z,
		 start_phi,
		 end_phi);
  
  //VisAttElect->SetForceWireframe(true);
  VisAttElect->SetForceSolid(true);
  
  // About material, from the Brahms sources:
  // "Electronics porridge - use scintillator for now"...
  //(brgeom_205.car)
  G4LogicalVolume *ElectLogical=
    new G4LogicalVolume(ElectSolid,
			CGAGeometryManager::GetMaterial("polystyrene"),
			"ElectLogical1", 
			0, 
			0, 
			0);
  
  ElectLogical->SetVisAttributes(VisAttElect);

  G4double ZElect = 
    electronics_start_z + Elect_half_z;
  
  G4PVPlacement *ElectPhys;
  ElectPhys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., ZElect),
		      ElectLogical,
		      "ElectPhysical",
		      WorldLog,
		      false,0);
  ElectPhys=
    new G4PVPlacement(rot,
		      G4ThreeVector(0., 0., -ZElect),
		      ElectLogical,
		      "ElectPhysical",
		      WorldLog,
		      false,0);

  // Closes Database connection
  delete db;
  db = 0;
  G4cout << "Mask done.\n" << G4endl;
  return true;
}

