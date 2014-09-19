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
// $Id: Coil01.cc,v 1.6 2008/10/05 18:28:27 frank Exp $
// $Name:  $
//
// 
//----------------------------------------------------
// Coil01.cc
//
// History:  
// - first implementation P. Mora de Freitas (may 01)
// - F.Gaede:  write out parameters to GEAR  (Oct 08)

#include "globals.hh"
#include "G4Tubs.hh"
#include "G4Polyhedra.hh"
#include "G4Tubs.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

#include "Coil01.hh"
#include "TRKSD00.hh"

#include "MySQLWrapper.hh"
#include "assert.h"
#include "Control.hh"
#include "CGAGeometryManager.hh"

#include "CGADefs.h"
#include "UserInit.hh"

#ifdef MOKKA_GEAR
#include "gearimpl/GearParametersImpl.h" 
#include "MokkaGear.h"
#include "G4ParticleTable.hh"
#endif

INSTANTIATE(Coil01)

G4bool Coil01::construct(const G4String &aSubDetectorDBName,
			 G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding Coil..." << G4endl;
  Database *db = new Database(aSubDetectorDBName.data());
  
  db->exec("select * from coil;");
  db->getTuple();

  G4cout << "\n... cryostat inner_raduus " << db->fetchDouble("inner_radius")
         << "\n... cryostat outer_raduus " << db->fetchDouble("outer_radius")
         << "\n... cryostat half_z       " << db->fetchDouble("half_z") 
	 << G4endl;

  //... Coil Cryostat (Al, inside vacuum)
  //... inner cylinder
  G4Tubs *CoilEnvelopeSolid_1
    = new G4Tubs("CoilEnvelope_1", 
		 db->fetchDouble("inner_radius"), 
		 40.*mm + db->fetchDouble("inner_radius"),
		 db->fetchDouble("half_z"),
		 0*deg, 
		 360.*deg);
  
  G4LogicalVolume *CoilLogical_1=
    new G4LogicalVolume(CoilEnvelopeSolid_1,
			CGAGeometryManager::GetMaterial("aluminium"),
			"CoilEnvelope_1", 
			0,
			0,
			0);
  
  G4VisAttributes * VisAtt_1 = new G4VisAttributes(G4Colour(1.,1.,0.));
  VisAtt_1->SetForceWireframe(true);
  //VisAtt_1->SetForceSolid(true);
  CoilLogical_1->SetVisAttributes(VisAtt_1);

  new G4PVPlacement(0,
		    G4ThreeVector(0., 0., 0),
		    CoilLogical_1,
		    "CoilEnvelope_1",
		    WorldLog,
		    false,0);

  //... outer cylinder
  G4Tubs *CoilEnvelopeSolid_2
    = new G4Tubs("CoilEnvelope_2", 
		 -30*mm + db->fetchDouble("outer_radius"), 
		 db->fetchDouble("outer_radius"),
		 db->fetchDouble("half_z"),
		 0*deg, 
		 360.*deg);
  
  G4LogicalVolume *CoilLogical_2=
    new G4LogicalVolume(CoilEnvelopeSolid_2,
			CGAGeometryManager::GetMaterial("aluminium"),
			"CoilEnvelope_2", 
			0,
			0,
			0);
  
  G4VisAttributes * VisAtt_2 = new G4VisAttributes(G4Colour(1.,1.,0.));
  VisAtt_2->SetForceWireframe(true);
  //VisAtt_2->SetForceSolid(true);
  CoilLogical_2->SetVisAttributes(VisAtt_2);

  new G4PVPlacement(0,
		    G4ThreeVector(0., 0., 0),
		    CoilLogical_2,
		    "CoilEnvelope_2",
		    WorldLog,
		    false,0);

  //... side wall left
  G4Tubs *CoilEnvelopeSolid_3
    = new G4Tubs("CoilEnvelope_3", 
		 40*mm + db->fetchDouble("inner_radius"),
		 -30*mm + db->fetchDouble("outer_radius"), 
		 25.*mm,
		 0.*deg, 
		 360.*deg);
  
  G4LogicalVolume *CoilLogical_3=
    new G4LogicalVolume(CoilEnvelopeSolid_3,
			CGAGeometryManager::GetMaterial("aluminium"),
			"CoilEnvelope_3", 
			0,
			0,
			0);
  
  G4VisAttributes * VisAtt_3 = new G4VisAttributes(G4Colour(1.,1.,0.));
  VisAtt_3->SetForceWireframe(true);
  //VisAtt_3->SetForceSolid(true);
  CoilLogical_3->SetVisAttributes(VisAtt_3);

  new G4PVPlacement(0,
		    G4ThreeVector(0., 0.,-25.*mm + db->fetchDouble("half_z") ),
		    CoilLogical_3,
		    "CoilEnvelope_3",
		    WorldLog,
		    false,0);

  //... side wall right
  G4Tubs *CoilEnvelopeSolid_4
    = new G4Tubs("CoilEnvelope_4", 
		 40*mm + db->fetchDouble("inner_radius"),
		 -30*mm + db->fetchDouble("outer_radius"), 
		 25.*mm,
		 0*deg, 
		 360.*deg);
  
  G4LogicalVolume *CoilLogical_4=
    new G4LogicalVolume(CoilEnvelopeSolid_4,
			CGAGeometryManager::GetMaterial("aluminium"),
			"CoilEnvelope_4", 
			0,
			0,
			0);
  
  G4VisAttributes * VisAtt_4 = new G4VisAttributes(G4Colour(1.,1.,0.));
  VisAtt_4->SetForceWireframe(true);
  //VisAtt_4->SetForceSolid(true);
  CoilLogical_4->SetVisAttributes(VisAtt_4);

  new G4PVPlacement(0,
		    G4ThreeVector(0., 0.,25.*mm - db->fetchDouble("half_z") ),
		    CoilLogical_4,
		    "CoilEnvelope_4",
		    WorldLog,
		    false,0);

  //... Coil modules
  //... main coll module 1,2,3

  G4Tubs *CoilMainSolid_1
    = new G4Tubs("CoilMain_1", 
		 175*mm + db->fetchDouble("inner_radius"),
		 560*mm + db->fetchDouble("inner_radius"), 
		 1632.*mm/2-0.*mm,
		 0*deg, 
		 360.*deg);
  
  G4LogicalVolume *CoilMainLogical_1=
    new G4LogicalVolume(CoilMainSolid_1,
			CGAGeometryManager::GetMaterial("aluminium"),
			"CoilMain_1", 
			0,
			0,
			0);
  
  G4VisAttributes * VisAtt_m1 = new G4VisAttributes(G4Colour(0.7,0.7,0.7));
  VisAtt_m1->SetForceWireframe(true);
  //VisAtt_m1->SetForceSolid(true);
  CoilMainLogical_1->SetVisAttributes(VisAtt_m1);

  new G4PVPlacement(0,
		    G4ThreeVector(0., 0., -1632.*mm),
		    CoilMainLogical_1,
		    "CoilMain_1",
		    WorldLog,
		    false,0);

  new G4PVPlacement(0,
		    G4ThreeVector(0., 0., 0.),
		    CoilMainLogical_1,
		    "CoilMain_1",
		    WorldLog,
		    false,1);

  new G4PVPlacement(0,
		    G4ThreeVector(0., 0., 1632.*mm),
		    CoilMainLogical_1,
		    "CoilMain_1",
		    WorldLog,
		    false,2);

  //... corrected coil module 1,2
  G4Tubs *CoilCorrectSolid_1
    = new G4Tubs("CoilCorrect_1", 
		 175*mm + db->fetchDouble("inner_radius"),
		 560*mm + db->fetchDouble("inner_radius"), 
		 1224.*mm/2,
		 0*deg, 
		 360.*deg);
  
  G4LogicalVolume *CoilCorrectLogical_1=
    new G4LogicalVolume(CoilCorrectSolid_1,
			CGAGeometryManager::GetMaterial("aluminium"),
			"CoilCorrect_1", 
			0,
			0,
			0);
  
  G4VisAttributes * VisAtt_c1 = new G4VisAttributes(G4Colour(1.,0.5,0.5));
  VisAtt_c1->SetForceWireframe(true);
  //VisAtt_c1->SetForceSolid(true);
  CoilCorrectLogical_1->SetVisAttributes(VisAtt_c1);

  new G4PVPlacement(0,
		    G4ThreeVector(0., 0., -1632.*mm*3/2-612.),
		    CoilCorrectLogical_1,
		    "CoilCorrect_1",
		    WorldLog,
		    false,0);

  new G4PVPlacement(0,
		    G4ThreeVector(0., 0., 1632.*mm*3/2+612.),
		    CoilCorrectLogical_1,
		    "CoilCorrect_1",
		    WorldLog,
		    false,1);

//... Coil mandrel
  G4Tubs *CoilMandrelSolid
    = new G4Tubs("CoilMandrel", 
		 560*mm + db->fetchDouble("inner_radius"),
		 635*mm + db->fetchDouble("inner_radius"), 
		 3675.*mm,
		 0*deg, 
		 360.*deg);
  
  G4LogicalVolume *CoilMandrelLogical=
    new G4LogicalVolume(CoilMandrelSolid,
			CGAGeometryManager::GetMaterial("aluminium"),
			"CoilMandrel", 
			0,
			0,
			0);
  
  G4VisAttributes * VisAtt_md = new G4VisAttributes(G4Colour(0.7,0.7,0.7));
  VisAtt_md->SetForceWireframe(true);
  //VisAtt_md->SetForceSolid(true);
  CoilMandrelLogical->SetVisAttributes(VisAtt_md);

  new G4PVPlacement(0,
		    G4ThreeVector(0., 0., 0.),
		    CoilMandrelLogical,
		    "CoilMandrel",
		    WorldLog,
		    false,0);

//... Coil sensitive detectors
  int layer_id=1; //Put in the database!!!
// Threshold is 20%. mip = 200 keV/mm 
  const G4double sensitive_thickness = 10. *mm;
  theCoilSD = 
    new TRKSD00("COIL", 
		sensitive_thickness * mm 
		* 200 * keV
		* 0.01);
  RegisterSensitiveDetector(theCoilSD);


//... Scintillator Detector layer 1

  const G4double zPozBarrelArray_1[2]   = {-100.*mm+db->fetchDouble("half_z"), 
				           +100.*mm-db->fetchDouble("half_z")};
  const G4double rInnerBarrelArray_1[2] = 
                                      {90.*mm+db->fetchDouble("inner_radius"), 
				       90.*mm+db->fetchDouble("inner_radius")};
  const G4double rOuterBarrelArray_1[2] = 
                                     {100.*mm+db->fetchDouble("inner_radius"), 
			              100.*mm+db->fetchDouble("inner_radius")};
//... Coil detector geometry


  G4Polyhedra *CoilScintSolid_1
    = new G4Polyhedra("CoilScint_1", 
		      0.*deg,
		      360.*deg,
		      24,
		      2,
		      zPozBarrelArray_1,
		      rInnerBarrelArray_1,
		      rOuterBarrelArray_1);
 
  G4LogicalVolume *CoilScintLogical_1=
    new G4LogicalVolume(CoilScintSolid_1,
			CGAGeometryManager::GetMaterial("polystyrene"),
			"CoilScint_1", 
			0,
			0,
			0);
  
  G4VisAttributes * VisAtt_s1 = new G4VisAttributes(G4Colour(0.5,0.5,1.));
  VisAtt_s1->SetForceWireframe(true);
  //VisAtt_s1->SetForceSolid(true);
  CoilScintLogical_1->SetVisAttributes(VisAtt_s1);
  CoilScintLogical_1->SetSensitiveDetector(theCoilSD);

  new G4PVPlacement(0,
		    G4ThreeVector(0., 0., 0. ),
		    CoilScintLogical_1,
		    "CoilScint_1",
		    WorldLog,
		    false,layer_id);

  layer_id++;
  //... Scintillation Detector layer 2
  const G4double zPozBarrelArray_2[2]   = {-100.*mm+db->fetchDouble("half_z"), 
				          +100.*mm-db->fetchDouble("half_z")};
  const G4double rInnerBarrelArray_2[2] = 
                                     {105.*mm+db->fetchDouble("inner_radius"), 
				     105.*mm+db->fetchDouble("inner_radius")};
  const G4double rOuterBarrelArray_2[2] = 
                                     {115.*mm+db->fetchDouble("inner_radius"), 
				      115.*mm+db->fetchDouble("inner_radius")};

  G4Polyhedra *CoilScintSolid_2
    = new G4Polyhedra("CoilScint_2", 
		      0.*deg,
		      360.*deg,
		      24,
		      2,
		      zPozBarrelArray_2,
		      rInnerBarrelArray_2,
		      rOuterBarrelArray_2);
 
  G4LogicalVolume *CoilScintLogical_2=
    new G4LogicalVolume(CoilScintSolid_2,
			CGAGeometryManager::GetMaterial("polystyrene"),
			"CoilScint_2", 
			0,
			0,
			0);
  
  G4VisAttributes * VisAtt_s2 = new G4VisAttributes(G4Colour(0.75,0.75,1.));
  VisAtt_s2->SetForceWireframe(true);
  //VisAtt_s2->SetForceSolid(true);
  CoilScintLogical_2->SetVisAttributes(VisAtt_s2);
  CoilScintLogical_2->SetSensitiveDetector(theCoilSD);

  new G4PVPlacement(0,
		    G4ThreeVector(0., 0., 0. ),
		    CoilScintLogical_2,
		    "CoilScint_2",
		    WorldLog,
		    false,layer_id);

  layer_id++;
//... Scint detector layer 3 

  const G4double zPozBarrelArray_3[2]   = {-100.*mm+db->fetchDouble("half_z"), 
				           +100.*mm-db->fetchDouble("half_z")};
  const G4double rInnerBarrelArray_3[2] = 
                                     {-90.*mm+db->fetchDouble("outer_radius"), 
				      -90.*mm+db->fetchDouble("outer_radius")};
  const G4double rOuterBarrelArray_3[2] = 
                                     {-80.*mm+db->fetchDouble("outer_radius"), 
				      -80.*mm+db->fetchDouble("outer_radius")};

  G4Polyhedra *CoilScintSolid_3
    = new G4Polyhedra("CoilScint_3", 
		      0.*deg,
		      360.*deg,
		      24,
		      2,
		      zPozBarrelArray_3,
		      rInnerBarrelArray_3,
		      rOuterBarrelArray_3);
 
  G4LogicalVolume *CoilScintLogical_3=
    new G4LogicalVolume(CoilScintSolid_3,
			CGAGeometryManager::GetMaterial("polystyrene"),
			"CoilScint_3", 
			0,
			0,
			0);
  
  G4VisAttributes * VisAtt_s3 = new G4VisAttributes(G4Colour(0.5,0.5,1.));
  VisAtt_s3->SetForceWireframe(true);
  //VisAtt_s3->SetForceSolid(true);
  CoilScintLogical_3->SetVisAttributes(VisAtt_s3);
  CoilScintLogical_3->SetSensitiveDetector(theCoilSD);

  new G4PVPlacement(0,
		    G4ThreeVector(0., 0., 0. ),
		    CoilScintLogical_3,
		    "CoilScint_3",
		    WorldLog,
		    false,layer_id);
  layer_id++;
  //... Scintillation Detector layer 4
  const G4double zPozBarrelArray_4[2]   = 
                                  {-100.*mm+db->fetchDouble("half_z"), 
				   +100.*mm-db->fetchDouble("half_z")};
  const G4double rInnerBarrelArray_4[2] = 
                                  {-105.*mm+db->fetchDouble("outer_radius"), 
				   -105.*mm+db->fetchDouble("outer_radius")};
  const G4double rOuterBarrelArray_4[2] = 
                                  {-95.*mm+db->fetchDouble("outer_radius"), 
				   -95.*mm+db->fetchDouble("outer_radius")};

  G4Polyhedra *CoilScintSolid_4
    = new G4Polyhedra("CoilScint_4", 
		      0.*deg,
		      360.*deg,
		      24,
		      2,
		      zPozBarrelArray_4,
		      rInnerBarrelArray_4,
		      rOuterBarrelArray_4);
 
  G4LogicalVolume *CoilScintLogical_4=
    new G4LogicalVolume(CoilScintSolid_4,
			CGAGeometryManager::GetMaterial("polystyrene"),
			"CoilScint_4", 
			0,
			0,
			0);
  
  G4VisAttributes * VisAtt_s4 = new G4VisAttributes(G4Colour(0.75,0.75,1.));
  VisAtt_s4->SetForceWireframe(true);
  //VisAtt_s4->SetForceSolid(true);
  CoilScintLogical_4->SetVisAttributes(VisAtt_s4);
  CoilScintLogical_4->SetSensitiveDetector(theCoilSD);

  new G4PVPlacement(0,
		    G4ThreeVector(0., 0., 0. ),
		    CoilScintLogical_4,
		    "CoilScint_4",
		    WorldLog,
		    false,layer_id);

#ifdef MOKKA_GEAR
  //----------------------------------------------------
  // MokkaGear
  //----------------------------------------------------

  MokkaGear* gearMgr = MokkaGear::getMgr() ;
  
  gear::GearParametersImpl* gp = new gear::GearParametersImpl ;

  //Inner Cylinder
  gp->setDoubleVal("Coil_cryostat_inner_cyl_inner_radius", 
                                        db->fetchDouble("inner_radius"));
  gp->setDoubleVal("Coil_cryostat_inner_cyl_outer_radius", 
                                        40.*mm+db->fetchDouble("outer_radius"));
  gp->setDoubleVal("Coil_cryostat_inner_cyl_half_z", db->fetchDouble("half_z"));
  gp->setStringVal("Coil_material_inner_cyl", "aluminium");


 //Outer Cylinder
  gp->setDoubleVal("Coil_cryostat_outer_cyl_inner_radius", 
                                        -30*mm+db->fetchDouble("outer_radius"));
  gp->setDoubleVal("Coil_cryostat_outer_cyl_outer_radius", 
                                        db->fetchDouble("outer_radius"));
  gp->setDoubleVal("Coil_cryostat_outer_cyl_half_z", db->fetchDouble("half_z"));
  gp->setStringVal("Coil_material_outer_cyl", "aluminium");

  //FG: add the parameters under the 'old' names as expected by the reconstruction:
  gp->setDoubleVal("Coil_cryostat_inner_radius", db->fetchDouble("inner_radius"));
  gp->setDoubleVal("Coil_cryostat_outer_radius", db->fetchDouble("outer_radius"));
  gp->setDoubleVal("Coil_cryostat_half_z", db->fetchDouble("half_z"));

  //Side wall left

  gp->setDoubleVal("Coil_cryostat_side_l_inner_radius", 
                                        40*mm+db->fetchDouble("inner_radius"));
  gp->setDoubleVal("Coil_cryostat_side_l_outer_radius", 
                                        -30*mm+db->fetchDouble("outer_radius"));
  gp->setDoubleVal("Coil_cryostat_side_l_half_z", 25.*mm);
  gp->setStringVal("Coil_material_side_l", "aluminium");

  //Side wall right

  gp->setDoubleVal("Coil_cryostat_side_r_inner_radius", 
                                        40*mm+db->fetchDouble("inner_radius"));
  gp->setDoubleVal("Coil_cryostat_side_r_outer_radius", 
                                        -30*mm+db->fetchDouble("outer_radius"));
  gp->setDoubleVal("Coil_cryostat_side_r_half_z", 25.*mm);
  gp->setStringVal("Coil_material_side_r", "aluminium");

 // Coil modules

  gp->setDoubleVal("Coil_cryostat_modules_inner_radius", 
                                        175*mm+db->fetchDouble("inner_radius"));
  gp->setDoubleVal("Coil_cryostat_modules_outer_radius", 
                                        560*mm+db->fetchDouble("inner_radius"));
  gp->setDoubleVal("Coil_cryostat_modules_half_z", 1632.*mm/2-20.*mm);
  gp->setStringVal("Coil_material_modules", "aluminium");

  gp->setDoubleVal("Coil_cryostat_c_modules_inner_radius", 
                                        175*mm+db->fetchDouble("inner_radius"));
  gp->setDoubleVal("Coil_cryostat_c_modules_outer_radius", 
                                        560*mm+db->fetchDouble("inner_radius"));
  gp->setDoubleVal("Coil_cryostat_c_modules_half_z", 1224.*mm);
  gp->setStringVal("Coil_material_c_modules", "aluminium");


  //Coil mandrel


  gp->setDoubleVal("Coil_cryostat_mandrel_inner_radius", 
                                        560*mm+db->fetchDouble("inner_radius"));
  gp->setDoubleVal("Coil_cryostat_mandrel_outer_radius", 
                                        635*mm+db->fetchDouble("inner_radius"));
  gp->setDoubleVal("Coil_cryostat_mandrel_half_z", 3675.*mm);
  gp->setStringVal("Coil_material_mandrel", "aluminium");

  //Sensitive detectors

  gp->setDoubleVal("Coil_cryostat_scint1_inner_radius", 
                                        90*mm+db->fetchDouble("inner_radius"));
  gp->setDoubleVal("Coil_cryostat_scint1_outer_radius", 
                                        100*mm+db->fetchDouble("inner_radius"));
  gp->setDoubleVal("Coil_cryostat_scint1_zposin", 
                                        -100*mm+db->fetchDouble("half_z"));
  gp->setDoubleVal("Coil_cryostat_scint1_zposend", 
                                        +100*mm+db->fetchDouble("half_z"));
  gp->setStringVal("Coil_material_scint1", "polystyrene");

  gp->setDoubleVal("Coil_cryostat_scint2_inner_radius", 
                                        105*mm+db->fetchDouble("inner_radius"));
  gp->setDoubleVal("Coil_cryostat_scint2_outer_radius", 
                                        115*mm+db->fetchDouble("inner_radius"));
  gp->setDoubleVal("Coil_cryostat_scint2_zposin", 
                                        -100*mm+db->fetchDouble("half_z"));
  gp->setDoubleVal("Coil_cryostat_scint2_zposend", 
                                        +100*mm+db->fetchDouble("half_z"));
  gp->setStringVal("Coil_material_scint2", "polystyrene");

  gp->setDoubleVal("Coil_cryostat_scint3_inner_radius", 
                                        -90*mm+db->fetchDouble("outer_radius"));
  gp->setDoubleVal("Coil_cryostat_scint3_outer_radius", 
                                        -80*mm+db->fetchDouble("outer_radius"));
  gp->setDoubleVal("Coil_cryostat_scint3_zposin", 
                                        -100*mm+db->fetchDouble("half_z"));
  gp->setDoubleVal("Coil_cryostat_scint3_zposend", 
                                        +100*mm+db->fetchDouble("half_z"));
  gp->setStringVal("Coil_material_scint3", "polystyrene");

  gp->setDoubleVal("Coil_cryostat_scint4_inner_radius", 
                                        -105*mm+db->fetchDouble("outer_radius"));
  gp->setDoubleVal("Coil_cryostat_scint4_outer_radius", 
                                        -95*mm+db->fetchDouble("outer_radius"));
  gp->setDoubleVal("Coil_cryostat_scint4_zposin", 
                                        -100*mm+db->fetchDouble("half_z"));
  gp->setDoubleVal("Coil_cryostat_scint4_zposend", 
                                        +100*mm+db->fetchDouble("half_z"));
  gp->setStringVal("Coil_material_scint4", "polystyrene");
  gearMgr->setGearParameters("CoilParameters", gp);
#endif


  G4cout << "Coil done.\n" << G4endl;
  delete db;
  return true;
}

