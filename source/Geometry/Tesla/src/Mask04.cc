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
// $Id: Mask04.cc,v 1.4 2006/02/07 16:50:15 musat Exp $
// $Name: mokka-07-00 $
//
//
//
// History:  
// Mask02.cc
// - first implementation G. Musat (Feb. 2003)
// Mask04.cc
// - remake for new TESLA "3m geometry" cylindrical mask (B.Pawlik March 2005)

#include "Control.hh"
#include "G4PVPlacement.hh"
#include "Mask04.hh"
#include "CGAGeometryManager.hh"

#include "globals.hh"

#include "G4Material.hh"
#include "G4Tubs.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4SubtractionSolid.hh"

#include "MySQLWrapper.hh"
#include <assert.h>

#include "CGADefs.h"

INSTANTIATE(Mask04)

G4bool Mask04::construct(const G4String &aSubDetectorName,
			 G4LogicalVolume *WorldLog)
{
  G4VisAttributes * VisAttMask1 = 
    new G4VisAttributes(G4Colour(20.5,0,0));
  
  G4VisAttributes * VisAttMask2 = 
    new G4VisAttributes(G4Colour(20.5,0,0));
  
  G4VisAttributes * VisAttMask3 = 
    new G4VisAttributes(G4Colour(20.5,0,0));

  G4VisAttributes * VisAttMask4 = 
    new G4VisAttributes(G4Colour(20.5,0,0));

  G4VisAttributes * VisAttCoil = 
    new G4VisAttributes(G4Colour(12.5,6,0));

  G4VisAttributes * VisAttYoke = 
    new G4VisAttributes(G4Colour(12.5,0,6));
  
  G4RotationMatrix *rot=new G4RotationMatrix();
  rot->rotateX(pi); // the same but other side

  G4cout << "\nBuilding the Shielding Mask..."<<aSubDetectorName.data() << G4endl;
  db = new Database(aSubDetectorName.data());
  
  // take the parameters from the database
  db->exec("select * from mask;");
  db->getTuple();

  G4double start_phi,tot_phi;
  start_phi = 0.*deg;
  tot_phi = 360.*deg;

  //
  mask1_z1   = db->fetchDouble("mask1_z1");      // 3260 mm
  mask1_z2   = db->fetchDouble("mask1_z2");      // 3340 mm
  mask1_rmin = db->fetchDouble("mask1_rmin");    //  100 mm
  mask1_rout = db->fetchDouble("mask1_rout");    //  170 mm
  G4double mask1_dz = (mask1_z2-mask1_z1)/2.*mm;
  G4double mask1_z0 = (mask1_z1+mask1_dz)*mm;
  // 
  mask2_z1   = mask1_z2;                       // 3340 mm
  mask2_z2   = db->fetchDouble("mask2_z2");    // 3510 mm
  mask2_rmin = db->fetchDouble("mask2_rmin");  //  210 mm
  mask2_rout = db->fetchDouble("mask2_rout");  //  240 mm
  G4double mask2_dz = (mask2_z2-mask2_z1)/2.*mm;
  G4double mask2_z0 = (mask2_z1+mask2_dz)*mm;
   //
  mask3_z1   = mask2_z2;                      // 3510 mm
  mask3_z2   = db->fetchDouble("mask3_z2");   // 3580 mm
  mask3_rmin = mask1_rmin;                    //  100 mm
  mask3_rout = mask2_rout;                    //  240 mm
  G4double mask3_dz = (mask3_z2-mask3_z1)/2.*mm;
  G4double mask3_z0 = (mask3_z1+mask3_dz)*mm;
   //
  mask4_z1   = mask3_z2;                       // 3580 mm
  mask4_z2   = db->fetchDouble("mask4_z2");    // 7000 mm 
  mask4_rmin = db->fetchDouble("mask4_rmin");  //  170 mm
  mask4_rout = mask2_rout;                     //  240 mm
  G4double mask4_dz = (mask4_z2-mask4_z1)/2.*mm;
  G4double mask4_z0 = (mask4_z1+mask4_dz)*mm;
  // Solid tubs
  //
    G4Tubs *MASK1 = new G4Tubs("MASK1",
			       mask1_rmin,
			       mask1_rout,
			       mask1_dz,
			       start_phi,
			       tot_phi);
  //
    G4Tubs *MASK2 = new G4Tubs("MASK2",
			       mask2_rmin,
			       mask2_rout,
			       mask2_dz,
			       start_phi,
			       tot_phi);
  //
    G4Tubs *MASK3 = new G4Tubs("MASK3",
			       mask3_rmin,
			       mask3_rout,
			       mask3_dz,
			       start_phi,
			       tot_phi);
  //
    G4Tubs *MASK4 = new G4Tubs("MASK4",
			       mask4_rmin,
			       mask4_rout,
			       mask4_dz,
			       start_phi,
			       tot_phi);
  
//     VisAttMask1->SetForceWireframe(true);
//     VisAttMask2->SetForceWireframe(true);
//     VisAttMask3->SetForceWireframe(true);
//     VisAttMask4->SetForceWireframe(true);

    VisAttMask1->SetForceSolid(true);
    VisAttMask2->SetForceSolid(true);
    VisAttMask3->SetForceSolid(true);
    VisAttMask4->SetForceSolid(true);
    //
    // logical masks
    //
  
  G4Material *tungsten = CGAGeometryManager::GetMaterial("tungsten_19.3gccm");

  G4LogicalVolume *MASK1Log = new G4LogicalVolume(MASK1,
						  tungsten,
						  "MASK1Log", 
						  0,
						  0,
						  0);
  
  G4LogicalVolume *MASK2Log = new G4LogicalVolume(MASK2,
						  tungsten,
						  "MASK2Log", 
						  0,
						  0,
						  0);
  
  G4LogicalVolume *MASK3Log = new G4LogicalVolume(MASK3,
						  tungsten,
						  "MASK3Log", 
						  0,
						  0,
						  0);
  
  G4LogicalVolume *MASK4Log = new G4LogicalVolume(MASK4,
						  tungsten,
						  "MASK4Log", 
						  0,
						  0,
						  0);
  
  MASK1Log->SetVisAttributes(VisAttMask1);
  MASK2Log->SetVisAttributes(VisAttMask1);
  MASK3Log->SetVisAttributes(VisAttMask1);
  MASK4Log->SetVisAttributes(VisAttMask1);
  //
  // put the mask in place
  //

  
  G4PVPlacement *MASKPhys;
  MASKPhys = new G4PVPlacement(0,
			       G4ThreeVector(0., 0., mask1_z0),
			       MASK1Log,
			       "MASK1+",
			       WorldLog,
			       false,0),
  MASKPhys= new G4PVPlacement(rot,
			      G4ThreeVector(0., 0., -mask1_z0),
			      MASK1Log,
			      "MASK1-",
			      WorldLog,
			      false,0);
  //
  MASKPhys = new G4PVPlacement(0,
			       G4ThreeVector(0., 0., mask2_z0),
			       MASK2Log,
			       "MASK2+",
			       WorldLog,
			       false,0),
  MASKPhys= new G4PVPlacement(rot,
			      G4ThreeVector(0., 0., -mask2_z0),
			      MASK2Log,
			      "MASK2-",
			      WorldLog,
			      false,0);
  //
  MASKPhys = new G4PVPlacement(0,
			       G4ThreeVector(0., 0., mask3_z0),
			       MASK3Log,
			       "MASK3+",
			       WorldLog,
			       false,0),
  MASKPhys= new G4PVPlacement(rot,
			      G4ThreeVector(0., 0., -mask3_z0),
			      MASK3Log,
			      "MASK3-",
			      WorldLog,
			      false,0);
  MASKPhys = new G4PVPlacement(0,
			       G4ThreeVector(0., 0., mask4_z0),
			       MASK4Log,
			       "MASK4+",
			       WorldLog,
			       false,0),
  MASKPhys= new G4PVPlacement(rot,
			      G4ThreeVector(0., 0., -mask4_z0),
			      MASK4Log,
			      "MASK4-",
			      WorldLog,
			      false,0);
  // building quadrupole

  db->exec("select * from quadrupole;");
  db->getTuple();
  G4double yoke_z0=db->fetchDouble("yoke_z0");
  G4double yoke_rmin=db->fetchDouble("yoke_rmin");
  G4double yoke_rmax=mask4_rmin-1.0;
  G4double yoke_dz = (db->fetchDouble("quad_end")-yoke_z0)/2.;
  G4double yoke_position = yoke_z0+yoke_dz;
  G4double coil_z0=db->fetchDouble("coil_z0");
  G4double coil_rmin=db->fetchDouble("coil_rmin");
  G4double coil_rmax=db->fetchDouble("coil_rmax");
  G4double coil_dz = (db->fetchDouble("quad_end")-coil_z0)/2.;
  G4double coil_position = coil_z0+coil_dz;
  G4double zshift = coil_position - yoke_position+10.;
  //
  G4Tubs *IronSolid = new G4Tubs("Quad_Yoke",
				 yoke_rmin,
				 yoke_rmax,
				 yoke_dz,
				 start_phi, tot_phi);

  G4Tubs *CaveSolid = new G4Tubs("Quad_Cave",
				 coil_rmin,
				 coil_rmax,
				 coil_dz+10.,
				 start_phi, tot_phi);

  G4SubtractionSolid *YokeSolid = new  G4SubtractionSolid("Quad_Yoke",
							  IronSolid,
							  CaveSolid,
							  0,
							  G4ThreeVector(0.,0,zshift));

  G4Tubs *CoilSolid = new G4Tubs("Quad_Coil",
				 coil_rmin,
				 coil_rmax,
				 coil_dz,
				 start_phi, tot_phi);
  //VisAttYoke->SetForceSolid(true);
  //VisAttCoil->SetForceSolid(true);
  VisAttYoke->SetForceWireframe(true);
  VisAttCoil->SetForceWireframe(true);
  //
  G4LogicalVolume *YokeLog = new G4LogicalVolume(YokeSolid,
						 CGAGeometryManager::GetMaterial("iron"),
						 "Quad_Yoke",
						 0, 0, 0);
  G4LogicalVolume *CoilLog = new G4LogicalVolume(CoilSolid,
						 CGAGeometryManager::GetMaterial("copper"),
						 "Quad_Coil",
						 0, 0, 0);
   YokeLog->SetVisAttributes(VisAttYoke);
   CoilLog->SetVisAttributes(VisAttCoil);
  // put quadrupoles into world
  new G4PVPlacement (0,
		     G4ThreeVector(0., 0., yoke_position),
		     YokeLog,
		     "QuadYoke",
		     WorldLog,
		     0,
		     0);
  new G4PVPlacement (rot,
		     G4ThreeVector(0., 0.,-yoke_position),
		     YokeLog,
		     "QuadYoke",
		     WorldLog,
		     0,
		     0);
  new G4PVPlacement (0,
		     G4ThreeVector(0., 0., coil_position),
		     CoilLog,
		     "QuadCoil",
		     WorldLog,
		     0,
		     0);
  new G4PVPlacement (0,
		     G4ThreeVector(0., 0.,-coil_position),
		     CoilLog,
		     "QuadCoil",
		     WorldLog,
		     0,
		     0);



  // Closes Database connection
  delete db;
  db = 0;
  G4cout << "Mask done.\n" << G4endl;
  return true;
}

