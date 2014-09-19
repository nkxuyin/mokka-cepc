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
// $Id: Mask02.cc,v 1.4 2006/02/07 16:50:15 musat Exp $
// $Name: mokka-07-00 $
//
//
// Mask02.cc
//
// History:  
// - first implementation G. Musat (Feb. 2003)

#include "Control.hh"
#include "G4PVPlacement.hh"
#include "Mask02.hh"
#include "CGAGeometryManager.hh"

#include "globals.hh"

#include "G4Material.hh"
#include "G4Cons.hh"
#include "G4Tubs.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"

#include "MySQLWrapper.hh"
#include <assert.h>

#include "CGADefs.h"

INSTANTIATE(Mask02)

G4bool Mask02::construct(const G4String &aSubDetectorName,
			 G4LogicalVolume *WorldLog)
{
  G4VisAttributes * VisAttMaskLat = 
    new G4VisAttributes(G4Colour(20.5,0,0));
  
  G4VisAttributes * VisAttMaskW = 
    new G4VisAttributes(G4Colour(.5,.5,.5));
  
  G4VisAttributes * VisAttMaskC = 
    new G4VisAttributes(G4Colour(0,0,20.5));
  
  G4RotationMatrix *rot=new G4RotationMatrix();
  rot->rotateX(pi); // the same but other side

  G4cout << "\nBuilding Mask..." << G4endl;
  db = new Database(aSubDetectorName.data());
  
  // take the parameters from the database
  db->exec("select * from mask;");
  db->getTuple();

  G4double start_phi,end_phi;
  start_phi = 0.*deg;
  end_phi = 360.*deg;

  G4double tan_inner_angle1, tan_inner_angle2, tan_out_angle;
  tan_inner_angle1 = tan(db->fetchDouble("lat_inner_angle"));
  tan_out_angle = tan(db->fetchDouble("lat_out_angle"));
  tan_inner_angle2 = tan(db->fetchDouble("lcal_out_angle"));

  G4double fRmin1,fRmin2,fRmax1,fRmax2,fDz;
  fRmin1 = db->fetchDouble("lat_begin_z") * tan_inner_angle1;
  fRmin2 = db->fetchDouble("w1_begin_z") * tan_inner_angle1;
  fRmax1 = db->fetchDouble("lat_begin_z") * tan_out_angle;
  fRmax2 = db->fetchDouble("w1_begin_z") * tan_out_angle;
  fDz =  (db->fetchDouble("w1_begin_z") - db->fetchDouble("lat_begin_z"))/2.0;
  
  //*******************************
  // W solid cones
  //*******************************
  G4Cons *MASKLat
    = new G4Cons("MASKLat",
		 fRmin1, fRmax1,
		 fRmin2, fRmax2,
		 fDz,
		 start_phi,
		 end_phi);
  
  //VisAttMask->SetForceWireframe(true);
  VisAttMaskLat->SetForceSolid(true);
  
// Tungsten pour LAT
  G4double a, z, density;
  G4Material * WLat;
  G4String name;
  a = 183.84*g/mole;
  density = 9.65 *g/cm3;
  WLat= new G4Material(name="TungstenForLAT", z=74., a, density);
  G4cout << "WLat->GetRadlen() = " << WLat->GetRadlen() /mm << " mm\n";

  G4LogicalVolume *MASKLatLogical=
    new G4LogicalVolume(MASKLat,
			WLat,
			"MASKLatLogical", 
			0, 
			0, 
			0);
  
  MASKLatLogical->SetVisAttributes(VisAttMaskLat);

  G4double ZMaskLat = db->fetchDouble("lat_begin_z") + fDz;
  
  G4PVPlacement *MASKPhys;
  MASKPhys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., ZMaskLat),
		      MASKLatLogical,
		      "MASKPhysical",
		      WorldLog,
		      false,0);
  MASKPhys=
    new G4PVPlacement(rot,
		      G4ThreeVector(0., 0., -ZMaskLat),
		      MASKLatLogical,
		      "MASKPhysical",
		      WorldLog,
		      false,0);

  fRmin1 = db->fetchDouble("w1_begin_z") * tan_inner_angle1;
  fRmin2 = db->fetchDouble("w2_begin_z") * tan_inner_angle1;
  fRmax1 = db->fetchDouble("w1_begin_z") * tan_out_angle;
  fRmax2 = db->fetchDouble("w2_out_radius");
  fDz =  (db->fetchDouble("w2_begin_z") - db->fetchDouble("w1_begin_z"))/2.0;

  G4Cons *MASKW1
    = new G4Cons("MASKW1",
		 fRmin1, fRmax1,
		 fRmin2, fRmax2,
		 fDz,
		 start_phi,
		 end_phi);

  //VisAttMask->SetForceWireframe(true);
  VisAttMaskW->SetForceSolid(true);

  G4LogicalVolume *MASKW1Logical=
    new G4LogicalVolume(MASKW1,
		    	CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
			"MASKW1Logical", 
			0, 
			0, 
			0);
  
  MASKW1Logical->SetVisAttributes(VisAttMaskW);

  G4double ZMaskW1 = db->fetchDouble("w1_begin_z") + fDz;

  MASKPhys=
    new G4PVPlacement(0,
		    G4ThreeVector(0., 0., ZMaskW1),
		    MASKW1Logical,
		    "MASKW1Physical",
		    WorldLog,
		    false,0);
  MASKPhys=
    new G4PVPlacement(rot,
		    G4ThreeVector(0., 0., -ZMaskW1),
		    MASKW1Logical,
		    "MASKW1Physical",
		    WorldLog,
		    false,0);

  fDz = (db->fetchDouble("w2_end_z") - db->fetchDouble("w2_begin_z"))/2.0;
  fRmin1 = db->fetchDouble("w2_inner_radius");
  fRmax1 = db->fetchDouble("w2_out_radius");

  G4Tubs * MASKW2
     = new G4Tubs("MaskW2",
		  fRmin1,
		  fRmax1,
		  fDz,
		  start_phi,
		  end_phi);
		  
  G4LogicalVolume *MASKW2Logical=
    new G4LogicalVolume(MASKW2,
		    	CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
			"MASKW2Logical", 
			0, 
			0, 
			0);
  
  MASKW2Logical->SetVisAttributes(VisAttMaskW);

  G4double ZMaskW2 = db->fetchDouble("w2_begin_z") + fDz;

  MASKPhys=
    new G4PVPlacement(0,
		    G4ThreeVector(0., 0., ZMaskW2),
		    MASKW2Logical,
		    "MASKW2Physical",
		    WorldLog,
		    false,0);
  MASKPhys=
    new G4PVPlacement(rot,
		    G4ThreeVector(0., 0., -ZMaskW2),
		    MASKW2Logical,
		    "MASKW2Physical",
		    WorldLog,
		    false,0);

  
  fDz = (db->fetchDouble("w3_begin_z") - db->fetchDouble("c_begin_z"))/2.0;
  fRmin1 = db->fetchDouble("c_inner_radius");
  fRmax1 = db->fetchDouble("w2_inner_radius");
  
  G4Tubs * MASKC
     = new G4Tubs("MaskC",
		  fRmin1,
		  fRmax1,
		  fDz,
		  start_phi,
		  end_phi);
		  
  VisAttMaskC->SetForceSolid(true);

  G4LogicalVolume *MASKCLogical=
    new G4LogicalVolume(MASKC,
		    	CGAGeometryManager::GetMaterial("graphite"),
			"MASKCLogical", 
			0, 
			0, 
			0);
  
  MASKCLogical->SetVisAttributes(VisAttMaskC);

  G4double ZMaskC = db->fetchDouble("c_begin_z") + fDz;
  MASKPhys=
    new G4PVPlacement(0,
		    G4ThreeVector(0., 0., ZMaskC),
		    MASKCLogical,
		    "MASKCPhysical",
		    WorldLog,
		    false,0);
  MASKPhys=
    new G4PVPlacement(rot,
		    G4ThreeVector(0., 0., -ZMaskC),
		    MASKCLogical,
		    "MASKCPhysical",
		    WorldLog,
		    false,0);
  
  fRmin1 = db->fetchDouble("w3_begin_z") * tan_inner_angle2;
  fRmin2 = db->fetchDouble("w3_end_z") * tan_inner_angle2;
  fRmax1 = db->fetchDouble("w2_inner_radius");
  fRmax2 = db->fetchDouble("w2_inner_radius");
  fDz =  (db->fetchDouble("w3_end_z") - db->fetchDouble("w3_begin_z"))/2.0;
  G4Cons *MASKW3
    = new G4Cons("MASKW3",
		 fRmin1, fRmax1,
		 fRmin2, fRmax2,
		 fDz,
		 start_phi,
		 end_phi);

  G4LogicalVolume *MASKW3Logical=
    new G4LogicalVolume(MASKW3,
		    	CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
			"MASKW3Logical", 
			0, 
			0, 
			0);
  
  MASKW3Logical->SetVisAttributes(VisAttMaskW);

  G4double ZMaskW3 = db->fetchDouble("w3_begin_z") + fDz;

  MASKPhys=
    new G4PVPlacement(0,
		    G4ThreeVector(0., 0., ZMaskW3),
		    MASKW3Logical,
		    "MASKW3Physical",
		    WorldLog,
		    false,0);
  MASKPhys=
    new G4PVPlacement(rot,
		    G4ThreeVector(0., 0., -ZMaskW3),
		    MASKW3Logical,
		    "MASKW3Physical",
		    WorldLog,
		    false,0);

  // Closes Database connection
  delete db;
  db = 0;
  G4cout << "Mask done.\n" << G4endl;
  return true;
}

