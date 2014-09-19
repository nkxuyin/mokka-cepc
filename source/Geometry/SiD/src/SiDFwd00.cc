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
// $Id: SiDFwd00.cc,v 1.4 2006/02/07 16:50:15 musat Exp $
// $Name: mokka-07-00 $
//
//
// SiDFwd00.cc
//
// History:  
// - first implementation V.Saveliev (May 05)

#include "Control.hh"
#include "G4PVPlacement.hh"
#include "SiDFwd00.hh"
#include "TRKSD00.hh"
#include "CGAGeometryManager.hh"

#include "globals.hh"

#include "G4Cons.hh"
#include "G4Tubs.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4RotationMatrix.hh"

#include "MySQLWrapper.hh"
#include <assert.h>

#include "CGADefs.h"


INSTANTIATE(SiDFwd00)

SiDFwd00::~SiDFwd00()

{
  // if (!theSiDFwd00SD) delete theSiDFwd00SD;
}


 G4bool SiDFwd00::construct(const G4String &aSubDetectorName,
			 G4LogicalVolume *WorldLog)
{

  G4VisAttributes * VisAttDisks;
  VisAttDisks = new G4VisAttributes(G4Colour(0.0,0.2,0.8));
  VisAttDisks->SetForceWireframe(true);
  //VisAttDisks->SetForceSolid(true);

  G4VisAttributes * VisAttRings;
  VisAttRings = new G4VisAttributes(G4Colour(1,.5,.5));
  //VisAttRings->SetForceWireframe(true);
  VisAttRings->SetForceSolid(true);

  G4VisAttributes * VisAttCyl;
  VisAttCyl = new G4VisAttributes(G4Colour(0.45,.2,0.9));
  VisAttCyl->SetForceWireframe(true);
  //VisAttCyl->SetForceSolid(true);

  G4VisAttributes * VisAttCables;
  VisAttCables = new G4VisAttributes(G4Colour(0.8,0.0,0.));
  VisAttCables->SetForceWireframe(true);
  //VisAttCables->SetForceSolid(true);

  G4RotationMatrix *rot=new G4RotationMatrix();
  rot->rotateX(pi); // the same but other side

  G4PVPlacement *Phys;
  
  G4double start_phy = 0.*deg;
  G4double stop_phy = 360.*deg;

  G4cout << "\n Building SiDFwd00..." << G4endl;

  db = new Database(aSubDetectorName.data());
  
  //****************************************
  // Si Disks without segments structure 
  //****************************************
  //
  // Common disks parameters
  db->exec("select * from common_parameters;");
  db->getTuple();
  G4double  Disks_Si_thickness, inner_support_thickness,
    inner_support_length, outer_support_thickness, 
    outer_support_length,outer_cylinder_total_thickness,
    cables_thickness;
  Disks_Si_thickness =  db->fetchDouble("Si_thickness");
  inner_support_thickness =  
    db->fetchDouble("inner_support_thickness");
  inner_support_length  =  
    db->fetchDouble("inner_support_length");
  outer_support_thickness =  
    db->fetchDouble("outer_support_thickness");
  outer_support_length  =  
    db->fetchDouble("outer_support_length");
  outer_cylinder_total_thickness = 
    db->fetchDouble("outer_cylinder_total_thickness");
  cables_thickness = 
    db->fetchDouble("cables_thickness");

  // The SiDFwd00 Sensitive detector
  // Threshold is 20% of a MIP. For Si we have 
  // 340 KeV/mm as MIP.

  theSiDFwd00SD = 
    new TRKSD00("SiDFwd00", 
		Disks_Si_thickness * mm 
		* 340 * keV
		* 0.2);

  RegisterSensitiveDetector(theSiDFwd00SD);

  // Build disks
  db->exec("select * from disk;");
  db->getTuple();

//   G4double ZStartOuterCylinder=0,ZstopOuterCylinder=0,
//     OuterCylinderInnerRadious=0;
//   G4double ZStartInnerCylinder=0,ZstopInnerCylinder=0,
//     InnerCylinderInnerRadious1=0,InnerCylinderInnerRadious2=0;

  do 
    {
      // Get the disk parameters
      G4int disk_number;
      G4double z_position;
      G4double inner_radious,outer_radious;
      disk_number = db->fetchInt("disk_number");
      inner_radious = db->fetchDouble("inner_radious");
      outer_radious  = db->fetchDouble("outer_radious");
      z_position = db->fetchDouble("z_position");

      G4Material *DiskMaterial;

      // Si tube

      G4Tubs *SiDFwd00SiDiskSolid
	= new G4Tubs("SiDFwd00SiDisk",
		     inner_radious,
		     outer_radious,
		     Disks_Si_thickness/2.,
		     start_phy, 
		     stop_phy);
      
      // DiskMaterial = CGAGeometryManager::SiVXD;
      DiskMaterial = CGAGeometryManager::GetMaterial("silicon_2.33gccm");
      G4LogicalVolume *SiDFwd00SiDiskLogical=

	new G4LogicalVolume(SiDFwd00SiDiskSolid,
			    DiskMaterial,
			    "SiDFwd00SiDisk", 
			    0, 
			    0, 
			    0);
      
      SiDFwd00SiDiskLogical->SetVisAttributes(VisAttDisks);
      SiDFwd00SiDiskLogical->SetSensitiveDetector(theSiDFwd00SD);

      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., z_position),
			  SiDFwd00SiDiskLogical,
			  "SiDFwd00SiDisk",
			  WorldLog,
			  false,
			  disk_number);      

      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., -z_position),
			  SiDFwd00SiDiskLogical,
			  "SiExtFtdSiDisk",
			  WorldLog,
			  false,
			  -disk_number);      

     }


  
    while(db->getTuple()!=NULL);


  // Closes Database connection
  delete db;
  db = 0;
  G4cout << "SiDFwd00 done.\n" << G4endl;
  return true;
}
