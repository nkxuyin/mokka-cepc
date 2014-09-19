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
// $Id: Tmag06.cc,v 1.3 2007/02/27 11:47:08 predrag Exp $
// $Name: mokka-07-00 $
//
// 
//----------------------------------------------------
// Tmag06.cc
//
// History:  
// - first implementation P.Krstonosic

#include "globals.hh"
#include "G4Tubs.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4String.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4Box.hh"
#include "Tmag06.hh"
#include "MySQLWrapper.hh"
#include "Control.hh"
#include "CGAGeometryManager.hh"
#include "TrigSD.hh"

#include "CGADefs.h"

INSTANTIATE(Tmag06)

G4bool Tmag06::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog)
{
  G4cout << "\nBuilding Tmag..." << G4endl;

  Database *db = new Database(env.GetDBName());
  db->exec("SELECT * FROM `tmag`;");

  // seting the visual atributes 
  G4VisAttributes * VisAtt1 = new G4VisAttributes(G4Colour(0.,0.,1.0)) ; // iron -blue
  G4VisAttributes * VisAtt2 = new G4VisAttributes(G4Colour(1.0,0.,0.0)); // copper -red
  G4VisAttributes * VisAtt3 = new G4VisAttributes(G4Colour(0.,1.0,1.0)); // aluminium -cyan
  G4VisAttributes * VisAtt4 = new G4VisAttributes(G4Colour(1.,1.,0.0)) ; // scintilator -yellow
  VisAtt1->SetForceWireframe(true);
  VisAtt2->SetForceWireframe(true);
  VisAtt3->SetForceWireframe(true);
  VisAtt4->SetForceWireframe(true);



  while (db->getTuple()) {
  
    // volume names 
    G4String ime= db->fetchString("name");
    G4String imesolid=ime+"solid";
    G4String imelog=ime+"logic";
    G4String imeplac=ime+"logicplace";

    // dimensions 
    const G4double Rin  = db->fetchDouble("inner_radius") * mm;
    const G4double Rout = db->fetchDouble("outer_radius") * mm;
    const G4double Z_half   = db->fetchDouble("z_length") * mm;

    G4Tubs *Solidpeace=new G4Tubs(imesolid,Rin,Rout,Z_half,0*deg,360.0*deg);
    G4String matname=db->fetchString("material");

    G4Material* mat=CGAGeometryManager::GetMaterial(matname);
    G4LogicalVolume *Logicpeace=new G4LogicalVolume(Solidpeace,mat,imelog,0,0,0);

    if(matname=="iron")
      Logicpeace->SetVisAttributes(VisAtt1);
    if(matname=="aluminium")
      Logicpeace->SetVisAttributes(VisAtt3);
    if(matname=="copper")
      Logicpeace->SetVisAttributes(VisAtt2);

    const G4double dX   = db->fetchDouble("x_shift") * mm;
    const G4double dY   = db->fetchDouble("y_shift") * mm;
    const G4double dZ   = db->fetchDouble("z_shift") * mm;

     new G4PVPlacement(0,G4ThreeVector(dX,dY, dZ), Logicpeace,imeplac, worldLog,  false,0);

 }

 db->exec("SELECT * FROM `trigger`;");

 TrigSD* theTrigger =new TrigSD("Trigger");
 RegisterSensitiveDetector(theTrigger);

 while (db->getTuple()) {
  
    // volume names 
    G4String ime= db->fetchString("name");
    G4String imesolid=ime+"solid";
    G4String imelog=ime+"logic";
    G4String imeplac=ime+"logicplace";

    // dimensions 
    const G4double lx  = db->fetchDouble("x") * mm;
    const G4double ly  = db->fetchDouble("y") * mm;
    const G4double lz  = db->fetchDouble("z") * mm;

    G4Box*Solidpeace=new G4Box(imesolid,lx,ly,lz);
    G4String matname=db->fetchString("material");

    G4Material* mat=CGAGeometryManager::GetMaterial(matname);
    G4LogicalVolume *Logicpeace=new G4LogicalVolume(Solidpeace,mat,imelog,0,0,0);

   
      Logicpeace->SetVisAttributes(VisAtt4);
      Logicpeace->SetSensitiveDetector(theTrigger);

    const G4double dX   = db->fetchDouble("x_shift") * mm;
    const G4double dY   = db->fetchDouble("y_shift") * mm;
    const G4double dZ   = db->fetchDouble("z_shift") * mm;

     new G4PVPlacement(0,G4ThreeVector(dX,dY, dZ), Logicpeace,imeplac, worldLog,  false,0);

 }


  G4cout << "Tmag done.\n" << G4endl;
  delete db;
  return true;
}

