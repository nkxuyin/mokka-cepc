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
// $Id: Yoke00.cc,v 1.5 2005/12/01 15:34:31 musat Exp $
// $Name: mokka-07-00 $
//
//
// Yoke00.cc
//
// History:  
// - first implementation P. Mora de Freitas (may 01)

#include "globals.hh"
#include "G4Polyhedra.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

#include "Yoke00.hh"
#include "MySQLWrapper.hh"
#include "Control.hh"
#include "CGAGeometryManager.hh"

#include "CGADefs.h"

INSTANTIATE(Yoke00)

G4bool Yoke00::construct(const G4String &aSubDetectorName,
			 G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding Yoke..." << G4endl;
  Database *db = new Database(aSubDetectorName.data());
  
  db->exec("select * from yoke;");
  db->getTuple();

  // yoke pieces as Polyhedras
  // First the big barrel
  G4double zPlane[2];
  zPlane[0]=-db->fetchDouble("barrel_half_z");
  zPlane[1]=-zPlane[0];

  G4double rInner[2],rOuter[2];
  rInner[0]=rInner[1]=db->fetchDouble("barrel_inner_radius");
  rOuter[0]=rOuter[1]=db->fetchDouble("outer_radius");
  
  G4Polyhedra *YokeBarrelSolid=
    new G4Polyhedra("YokeBarrelSolid",
		    15.,
		    360.,
		    12,
		    2,
		    zPlane,
		    rInner,
		    rOuter);

  G4LogicalVolume* YokeBarrelLogical =
    new G4LogicalVolume(YokeBarrelSolid,
			CGAGeometryManager::GetMaterial("iron"),
			"YokeBarrelLogical",
			0, 0, 0);

  G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(.1,0.8,0.8));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  YokeBarrelLogical->SetVisAttributes(VisAtt);


  G4PVPlacement *Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., 0),
		      YokeBarrelLogical,
		      "YokeBarrelEnvelope",
		      WorldLog,
		      false,0);
  
  // Yoke endcaps
  // ye1 is still "inside" the coil
  zPlane[0]=-db->fetchDouble("ye1_half_z");
  zPlane[1]=-zPlane[0];

  rInner[0]=rInner[1]=db->fetchDouble("endcap_inner_radius");
  rOuter[0]=rOuter[1]=db->fetchDouble("ye1_outer_radius");
  
  G4Polyhedra *YokeYe1Solid=
    new G4Polyhedra("YokeYe1Solid",
		    15.,
		    360.,
		    12,
		    2,
		    zPlane,
		    rInner,
		    rOuter);

  G4LogicalVolume* YokeYe1Logical =
    new G4LogicalVolume(YokeYe1Solid,
			CGAGeometryManager::GetMaterial("iron"),
			"YokeYe1Logical",
			0, 0, 0);

  VisAtt = new G4VisAttributes(G4Colour(.5,0.8,0.8));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  YokeYe1Logical->SetVisAttributes(VisAtt);

  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0.,db->fetchDouble("ye1_z_center")),
		      YokeYe1Logical,
		      "YokeYe1+Envelope",
		      WorldLog,
		      false,0);
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0.,-db->fetchDouble("ye1_z_center")),
		      YokeYe1Logical,
		      "YokeYe1-Envelope",
		      WorldLog,
		      false,0);
  
  // ye2 is the rest and nexted to the barril

  G4double half_ye2_z = 
    (db->fetchDouble("barrel_half_z") -
     db->fetchDouble("ye1_z_center") -
     db->fetchDouble("ye1_half_z"))/2.;
    
  zPlane[0]=-half_ye2_z;
  zPlane[1]=-zPlane[0];
  
  rInner[0]=rInner[1]=db->fetchDouble("endcap_inner_radius");
  rOuter[0]=rOuter[1]=db->fetchDouble("barrel_inner_radius");
  
  G4Polyhedra *YokeYe2Solid=
    new G4Polyhedra("YokeYe2Solid",
		    15.,
		    360.,
		    12,
		    2,
		    zPlane,
		    rInner,
		    rOuter);

  G4LogicalVolume* YokeYe2Logical =
    new G4LogicalVolume(YokeYe2Solid,
			CGAGeometryManager::GetMaterial("iron"),
			"YokeYe2Logical",
			0, 0, 0);

  VisAtt = new G4VisAttributes(G4Colour(.5,0.8,0.8));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  YokeYe2Logical->SetVisAttributes(VisAtt);

  G4double ye2_z_center =
    db->fetchDouble("barrel_half_z")-half_ye2_z;

  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0.,ye2_z_center),
		      YokeYe2Logical,
		      "YokeYe2+Envelope",
		      WorldLog,
		      false,0);
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0.,-ye2_z_center),
		      YokeYe2Logical,
		      "YokeYe2-Envelope",
		      WorldLog,
		      false,0);
  
  G4cout << "Coil done.\n" << G4endl;
  delete db;
  return true;
}

