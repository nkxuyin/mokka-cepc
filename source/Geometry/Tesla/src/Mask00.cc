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
// $Id: Mask00.cc,v 1.3 2005/04/07 16:17:02 musat Exp $
// $Name: mokka-07-00 $
//
//
// Mask00.cc
//
// History:  
// - first implementation P. Mora de Freitas (apr 01)

#include "G4PVPlacement.hh"
#include "Mask00.hh"
#include "CGAGeometryManager.hh"

#include "globals.hh"

#include "G4Cons.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"

#include "MySQLWrapper.hh"
#include "Control.hh"

#include "CGADefs.h"

INSTANTIATE(Mask00)

G4bool Mask00::construct(const G4String &aSubDetectorName,
			 G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding Mask..." << G4endl;
  db = new Database(aSubDetectorName.data());
  
  db->exec("select * from mask;");
  db->getTuple();

  // From Lineaire
  //
  //   Masks
  //
  //	fRmin1	inside radius at  -fDz
  //	fRmin2	inside radius at  +fDz
  //	fRmax1	outside radius at -fDz
  //	fRmax2	outside radius at +fDz
  //	fDz	half length in z
  //
  
  double fRmin1,fRmin2,fRmax1,fRmax2,fDz;
  
  fRmin1 = db->fetchDouble("begin_z") * sin(db->fetchDouble("inner_angle"));
  fRmin2 = db->fetchDouble("end_z") * sin(db->fetchDouble("inner_angle"));
  fRmax1 = db->fetchDouble("begin_z") * sin(db->fetchDouble("out_angle"));
  fRmax2 = db->fetchDouble("end_z") * sin(db->fetchDouble("out_angle"));
  fDz =    (db->fetchDouble("end_z") - db->fetchDouble("begin_z"))/2. ;
  
  G4Cons *MASKSolid1
    = new G4Cons("MASKSolid",
		 fRmin1, fRmax1,
		 fRmin2, fRmax2,
		 fDz,
		 0.*deg,
		 360.*deg);
  
  G4Cons *MASKSolid2
    = new G4Cons("MASKSolid",
		 fRmin2, fRmax2,
		 fRmin1, fRmax1,
		 fDz,
		 0.*deg,
		 360.*deg);
  
  G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(.5,.5,.5));
  //VisAtt->SetForceWireframe(true);
  VisAtt->SetForceSolid(true);
  
  G4LogicalVolume *MASKLogical1=
    new G4LogicalVolume(MASKSolid1,
			CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
			"MASKLogical1", 
			0, 
			0, 
			0);
  
  MASKLogical1->SetVisAttributes(VisAtt);

  G4LogicalVolume *MASKLogical2=
    new G4LogicalVolume(MASKSolid2,
			CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
			"MASKLogical2", 
			0, 
			0, 
			0);
  
  MASKLogical2->SetVisAttributes(VisAtt);

  G4String MASKName("MASKPhysical");

  double ZMask = db->fetchDouble("begin_z") + fDz;
  
  G4PVPlacement *MASKPhys;
  MASKPhys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., ZMask),
		      MASKLogical1,
		      MASKName,
		      WorldLog,
		      false,0);

  MASKPhys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., -ZMask),
		      MASKLogical2,
		      MASKName,
		      WorldLog,
		      false,0);
  
  // Closes Database connection
  delete db;
  db = 0;
  G4cout << "Mask done.\n" << G4endl;
  return true;
}

