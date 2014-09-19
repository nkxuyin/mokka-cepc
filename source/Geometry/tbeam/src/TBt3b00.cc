//#######################################################################################
//                                                                                      #
//  Driver used to simulate the T3B experiment used at CERN 2011 together with WHCAL    #
//                                                                                      #
// author: Jacopo Nardulli, CERN                                                        #
//#######################################################################################

#include "TBt3b00.hh"
#include "Control.hh"
#include "CGADefs.h"
#include "MySQLWrapper.hh"
#include "CGAGeometryManager.hh"

#include "G4UnitsTable.hh"
#include "G4Box.hh"
#include "G4PVPlacement.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4NistManager.hh"

#include <sstream>

INSTANTIATE(TBt3b00)

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
TBt3b00::~TBt3b00()
{}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
G4bool TBt3b00::ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment, G4LogicalVolume *theWorld)
{
 G4cout << "\nBuilding TBt3b00..." << G4endl;
 db = new Database(aGeometryEnvironment.GetDBName());

 /* fetch data base parameters*/
 FetchAll();
  
 G4bool doT3B = T3BConstruct(theWorld,
			     T3B_X_dim,
			     T3B_Y_dim,
			     T3B_Scint_X_dim,
			     T3B_Scint_Y_dim,
			     T3B_PCB_X_dim,
			     T3B_PCB_Y_dim,
			     T3B_X_Pos_ScintAndPCB,
			     T3B_Y_Pos_ScintAndPCB,
			     T3B_X_Pos, 
			     T3B_Y_Pos, 
			     T3B_Z_Pos); 

 delete db;
 db = 0;
  
 G4cout << "\nDone building TBt3b00" << G4endl;
 return doT3B;

}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
G4bool TBt3b00::T3BConstruct(G4LogicalVolume *WorldLog, 
			     G4double xdim,       
			     G4double ydim, 
			     G4double xdim_scint, 
			     G4double ydim_scint,
			     G4double xdim_pcb,   
			     G4double ydim_pcb,
			     G4double T3B_X_Pos_ScintAndPCB,
			     G4double T3B_Y_Pos_ScintAndPCB,
			     G4double T3B_X_Pos,  
			     G4double T3B_Y_Pos,
			     G4double T3B_Z_Pos)
{

  WorldLogVol = WorldLog;

  //build boxes and logical volumes
  G4cout << " Building T3B" << G4endl;
  BuildElements(xdim, ydim, xdim_scint, ydim_scint, xdim_pcb, ydim_pcb);

  // do build process
  G4bool cokay = BuildT3B(T3B_X_Pos, T3B_Y_Pos, T3B_Z_Pos, T3B_X_Pos_ScintAndPCB, T3B_Y_Pos_ScintAndPCB);
  return cokay;
}


/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void TBt3b00::BuildElements(G4double xdim, G4double ydim, G4double xdim_scint, G4double ydim_scint, G4double xdim_pcb, G4double ydim_pcb)
{
  G4String name; 
  G4int nel;
 
  /*Materials*/
  G4Material* poly      = CGAGeometryManager::GetMaterial("polystyrene");
  G4Material* aluminium = CGAGeometryManager::GetMaterial("Aluminium");
  G4Material* air       = CGAGeometryManager::GetMaterial("air");

  /*for PCB*/
  G4Element* elH  = CGAGeometryManager::GetElement("H", true);  /*Hydrogen */
  G4Element* elBr = CGAGeometryManager::GetElement("Br", true); /*Bromine */
  G4Element* elO  = CGAGeometryManager::GetElement("O", true);  /*Oxygen */

  /*PCB*/
  G4Material* PCB = new G4Material(name="PCB", PCB_density, nel=5);
  PCB->AddMaterial(CGAGeometryManager::GetMaterial("silicon_2.33gccm"),PCB_silicon_fractiomass);
  PCB->AddElement(elO, PCB_elO_fractionmass);
  PCB->AddMaterial(CGAGeometryManager::GetMaterial("graphite"),PCB_graphite_fractionmass);
  PCB->AddElement(elH, PCB_elH_fractionmass);
  PCB->AddElement(elBr, PCB_elBr_fractionmass);

  /* Dimensions*/
  G4double T3B_x        = (xdim)/2;
  G4double T3B_y        = (ydim)/2;  
  G4double T3B_x_scint  = (xdim_scint)/2;
  G4double T3B_y_scint  = (ydim_scint)/2;  
  G4double T3B_x_pcb    = (xdim_pcb)/2;
  G4double T3B_y_pcb    = (ydim_pcb)/2;  

  /*thickness*/
  G4double Al_z         = Aluminium_thickness;
  G4double Air_z        = Air_thickness;
  G4double Poly_z       = Poly_thickness;
  G4double PCB_z        = PCB_thickness;
 
  G4Box *AlSolid        = new G4Box("AlSolid",    T3B_x,       T3B_y,       Al_z);
  G4Box *AirSolid       = new G4Box("AirSolid",   T3B_x,       T3B_y,       Air_z);
  G4Box *PCBSolid       = new G4Box("PCBSolid",   T3B_x_pcb,   T3B_y_pcb,   PCB_z);
  G4Box *PolySolid      = new G4Box("PolySolid",  T3B_x_scint, T3B_y_scint, Poly_z);

  /*Logical volumes*/
  AlLogical             = new G4LogicalVolume(AlSolid,    aluminium, G4String("T3BAluminiumLogical"), 0, 0, 0);	
  AirLogical            = new G4LogicalVolume(AirSolid,   air,       G4String("T3BAirLogical"),       0, 0, 0);	
  PolyLogical           = new G4LogicalVolume(PolySolid,  poly,      G4String("T3BPolyLogical"),      0, 0, 0);	
  PCBLogical            = new G4LogicalVolume(PCBSolid,   PCB,       G4String("T3BPCBLogical"),       0, 0, 0);	

  /*debug*/
  G4cout << " Dimension of T3B box " << G4endl;
  G4cout << " T3B_x: " << T3B_x*2 << " mm " << G4endl;
  G4cout << " T3B_y: " << T3B_y*2 << " mm " << G4endl;
  G4cout << " T3B_z: " << Air_z + Al_z + Poly_z + PCB_z  << " mm " << G4endl;

}


/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
G4bool TBt3b00::BuildT3B(G4double  T3B_X_Pos, G4double  T3B_Y_Pos, G4double  T3B_Z_Pos, 
			 G4double T3B_X_Pos_ScintAndPCB, G4double T3B_Y_Pos_ScintAndPCB)
{
  G4cout << " \nBuilding T3B structure " << G4endl;
  G4cout << " x_place of T3B detector " <<  T3B_X_Pos  << G4endl;
  G4cout << " y_place of T3B detector " <<  T3B_Y_Pos  << G4endl;
  G4cout << " z_place of T3B detector " <<  T3B_Z_Pos  << G4endl;

  /*translations for all boxes*/
  translateAl     = G4ThreeVector(T3B_X_Pos,             T3B_Y_Pos,             T3B_Z_Pos );
  translateAir    = G4ThreeVector(T3B_X_Pos,             T3B_Y_Pos,             T3B_Z_Pos + Aluminium_thickness);
  translatePoly   = G4ThreeVector(T3B_X_Pos_ScintAndPCB, T3B_Y_Pos_ScintAndPCB, T3B_Z_Pos + Aluminium_thickness + Air_thickness);
  translatePCB    = G4ThreeVector(T3B_X_Pos_ScintAndPCB, T3B_Y_Pos_ScintAndPCB, T3B_Z_Pos + Aluminium_thickness + Air_thickness + Poly_thickness);
  
  /*rotations*/
  G4RotationMatrix rotate;
  
  /*transformartions*/
  G4Transform3D *transformAl     = new G4Transform3D(rotate, translateAl);
  G4Transform3D *transformAir    = new G4Transform3D(rotate, translateAir);
  G4Transform3D *transformPoly   = new G4Transform3D(rotate, translatePoly);
  G4Transform3D *transformPCB    = new G4Transform3D(rotate, translatePCB);

  new G4PVPlacement(*transformAl,    AlLogical,    G4String("AluminimumPhys"),WorldLogVol,0,0);  
  new G4PVPlacement(*transformAir,   AirLogical,   G4String("AirPhys"),       WorldLogVol,0,0);  
  new G4PVPlacement(*transformPoly,  PolyLogical,  G4String("PolyPhys"),      WorldLogVol,0,0);  
  new G4PVPlacement(*transformPCB,   PCBLogical,   G4String("PCBPhys"),       WorldLogVol,0,0);  

  G4VisAttributes *AlColour = new G4VisAttributes(G4Colour::Green());
  AlColour->SetVisibility(true);
  AlLogical->SetVisAttributes(AlColour);

  G4VisAttributes *PolyColour = new G4VisAttributes(G4Colour::Blue());
  PolyColour->SetVisibility(true);
  PolyLogical->SetVisAttributes(PolyColour);
 
  G4VisAttributes *PCBColour = new G4VisAttributes(G4Colour::Magenta());
  PCBColour->SetVisibility(true);
  PCBLogical->SetVisAttributes(PCBColour);

  return true;
}


/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void TBt3b00::FetchAll()
{
  db->exec("select * from t3b_cern_2011_virt;");
  db->getTuple();

  /*PCB*/
  PCB_density				= db->fetchDouble("PCB_density")*g/cm3;//1.7*g/cm3;//db->fetchDouble("PCB_density")*g/cm3;
  PCB_silicon_fractiomass		= db->fetchDouble("PCB_silicon_2.33_fractiomass"); //0.180774;//db->fetchDouble("PCB_silicon_2.33_fractiomass"); 
  PCB_elO_fractionmass			= db->fetchDouble("PCB_elO_fractionmass"); //0.405633;//db->fetchDouble("PCB_elO_fractionmass"); 
  PCB_graphite_fractionmass		= db->fetchDouble("PCB_graphite_fractionmass"); //0.278042;//db->fetchDouble("PCB_graphite_fractionmass"); 
  PCB_elH_fractionmass			= db->fetchDouble("PCB_elH_fractionmass"); //0.0684428;//db->fetchDouble("PCB_elH_fractionmass"); 
  PCB_elBr_fractionmass			= db->fetchDouble("PCB_elBr_fractionmass"); //0.0671091;//db->fetchDouble("PCB_elBr_fractionmass"); 

  /* T3B dimensions*/
  T3B_X_dim                             = db->fetchDouble("T3B_X_dim");//1*m;//db->fetchDouble("T3B_X_dim");
  T3B_Y_dim                             = db->fetchDouble("T3B_Y_dim");//1*m;//db->fetchDouble("T3B_Y_dim");

  /*T3B scintillator dimensions 
    in X you have 15 tiles of 30 mm each. A tile is 30x30mm2*/
  T3B_Scint_X_dim                       = db->fetchDouble("T3B_Scint_X_dim");//450*mm;//db->fetchDouble("T3B_Scint_X_dim");
  T3B_Scint_Y_dim                       = db->fetchDouble("T3B_Scint_Y_dim");//30*mm; //db->fetchDouble("T3B_Scint_Y_dim");

  /*T3B PCB dimensions
    In X the PCB covers the same area of the scintillators, in Y you have twice 44mm */
  T3B_PCB_X_dim                         = db->fetchDouble("T3B_PCB_X_dim");//450*mm;//db->fetchDouble("T3B_PCB_X_dim");
  T3B_PCB_Y_dim                         = db->fetchDouble("T3B_PCB_Y_dim");//88*mm; //db->fetchDouble("T3B_PCB_Y_dim");

  /*Get the T3B pos
    in X and Y the T3B is centered around 0, in z you are at 488.7mm (distance from 0
    to HCAL) plus 39(nr layers)*24.73mm(thickness of an HCALlayer)*/ 
  T3B_X_Pos                             = db->fetchDouble("T3B_X_Pos");//0*mm;//db->fetchDouble("T3B_X_Pos");
  T3B_Y_Pos                             = db->fetchDouble("T3B_Y_Pos");//0*mm;//db->fetchDouble("T3B_Y_Pos");
  T3B_Z_Pos                             = db->fetchDouble("T3B_Z_Pos");//1477.9*mm;//db->fetchDouble("T3B_Z_Pos");

  //Get the T3B pos
  T3B_X_Pos_ScintAndPCB                 = db->fetchDouble("T3B_X_Scint_PCB_Pos");//-195*mm;//db->fetchDouble("T3B_X_Scint_PCB_Pos");
  T3B_Y_Pos_ScintAndPCB                 = db->fetchDouble("T3B_Y_Scint_PCB_Pos");//15*mm;//db->fetchDouble("T3B_Y_Scint_PCB_Pos");

  Aluminium_thickness                   = db->fetchDouble("Al_Z_dim");//3*mm;//db->fetchDouble("Al_Z_dim");
  Poly_thickness                        = db->fetchDouble("Poly_Z_dim");//2*mm;//db->fetchDouble("Poly_Z_dim");
  PCB_thickness                         = db->fetchDouble("PCB_Z_dim");//5*mm;//db->fetchDouble("PCB_Z_dim");
  Air_thickness                         = db->fetchDouble("Air_Z_dim");//1*mm;//db->fetchDouble("Air_Z_dim");

}
