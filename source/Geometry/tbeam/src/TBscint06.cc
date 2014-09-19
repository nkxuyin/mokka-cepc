//######################################################################
//                                                                      #
//  Driver used to simulate CERN June 2011 Test beam scintillators      #
//                                                                      #
//#######################################################################

#include "TBscint06.hh"
#include "Control.hh"
#include "CGADefs.h"
#include "MySQLWrapper.hh"
#include "CGAGeometryManager.hh"

#include "G4UnitsTable.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4NistManager.hh"

#include <sstream>

INSTANTIATE(TBscint06)

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
TBscint06::~TBscint06()
{}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
G4bool TBscint06::ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment, G4LogicalVolume *theWorld)
{
  /* variable config_angle (a double) will be used by method "construct" to place the detector*/
 config_angle    = 0.0;
 
 /* variables translateX and translateY have default value (0,0) and are used
    to translate Sci wrt beam axis (user defined parameters)
    translateX      = aGeometryEnvironment.GetParameterAsDouble("TranslateX");
    translateY      = aGeometryEnvironment.GetParameterAsDouble("TranslateY"); */
    
 translateX      = 0.0;
 translateY      = 0.0;

 G4cout << "\nBuilding TBscint06..." << G4endl;
 db = new Database(aGeometryEnvironment.GetDBName());

 /* fetch data base parameters*/
 FetchAll();
  
 G4bool doScintillator_1 = SciConstruct(theWorld,
					x_place1,
					y_place1,
					z_place1,
					1); 

 G4bool doScintillator_2 = SciConstruct(theWorld,
					x_place2,
					y_place2,
					z_place2,
					2); 

 /*put something for the vacuum pipe*/
 G4bool doScintillator_3 = SciConstruct(theWorld,
					0,
					0,
					-55*m,/*world box is 60000 × 60000 × 60000 mm^3, so keep it inside*/
					3); 
 

 delete db;
 db = 0;
  
 G4bool doScintillators = false;
 if (doScintillator_1 && doScintillator_2 && doScintillator_3) doScintillators = true;

 G4cout << "\nDone building TBscint06" << G4endl;
 return doScintillators;

}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
G4bool TBscint06::SciConstruct(G4LogicalVolume *WorldLog,
			       G4double x_place, G4double y_place, G4double z_place,
			       G4int idscintillator)
{
  G4cout << " Building Scintillator " << idscintillator << G4endl;
  WorldLogVol = WorldLog;

  /* depth to layer*/
  SetDepthToLayer(1);

  G4cout << " Building Scintillator elements " << G4endl;
  BuildElements(idscintillator);

  /* do build process*/
  G4bool cokay = BuildSci(x_place, y_place, z_place, idscintillator);
  return cokay;
}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void TBscint06::BuildElements(G4int idscintillator)
{
  /*Useful stuff*/
  G4String name; 
  G4int nelements;

  /*Scintillator materials
    The density is calculated as 90% of polystyrene (1.06), 4% Al (2.699) and 6% tape (0.9)*/
  G4Material* Scintillator_material = new G4Material(name = "Scintillator_material",
						     density,
						     nelements=3); 
  Scintillator_material->AddMaterial(CGAGeometryManager::GetMaterial("polystyrene"),
				     polystyrene_fraction);
  Scintillator_material->AddMaterial(CGAGeometryManager::GetMaterial("Aluminium"),
				     aluminium_fraction);
  G4NistManager* man = G4NistManager::Instance();
  G4Material*   polypropylene = man->FindOrBuildMaterial("G4_POLYPROPYLENE");
  Scintillator_material->AddMaterial(polypropylene, polypropylene_fraction);

  if (idscintillator == 1)
    {
      G4cout<<"Fraction of materials in Scintillators : "<< G4endl;
      G4cout<<"polystyrene_fraction " << polystyrene_fraction << G4endl;
      G4cout<<"aluminium_fraction " <<   aluminium_fraction << G4endl;
      G4cout<<"polypropylene_fraction "<<polypropylene_fraction << G4endl;
      G4cout<<"density " << G4BestUnit(density ,"Volumic Mass") << G4endl;
    }

  sci_hx = dimX_scintillator10x10/2.;
  sci_hy = dimY_scintillator10x10/2.;
  sci_hz = thickness_scintillator10x10;

  /*Scintillator number*/
  std::stringstream stringForScintillatorNo;
  stringForScintillatorNo << (idscintillator); 	   

  if (idscintillator == 3)
    {
      sci_hz = 5*mm;
      Scintillator_material = CGAGeometryManager::GetMaterial("iron");
    }

  G4Box *ScintillatorSolid = new G4Box("ScintillatorSolid", sci_hx, sci_hy, sci_hz);
  ScintillatorLogical = new G4LogicalVolume(ScintillatorSolid,Scintillator_material,
					    G4String("ScintillatorLogical") + G4String(stringForScintillatorNo.str()), 0, 0, 0);	

  G4cout << "\n Dimension of detector box " << G4endl;
  G4cout << " sci_hx: " << sci_hx*2 << " mm " << G4endl;
  G4cout << " sci_hy: " << sci_hy*2 << " mm " << G4endl;
  G4cout << " sci_hz: " << sci_hz   << " mm " << G4endl;

}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
G4bool TBscint06::BuildSci(G4double x_place, G4double y_place, G4double z_place,G4int idscintillator)
{
  G4cout << "\n Building Scintillator structure: Sci " << idscintillator << G4endl;
  G4cout << " x_place of Scintillator detector " << x_place << G4endl;
  G4cout << " y_place of Scintillator detector " << y_place << G4endl;
  G4cout << " z_place of Scintillator detector " << z_place << G4endl;
  
  translateSci = G4ThreeVector(x_place, y_place, z_place);
  G4RotationMatrix rotateSci;
  rotateSci.rotateY(config_angle);
  transformSci = new G4Transform3D(rotateSci, translateSci);

  std::stringstream stringForScintillatorNo;
  stringForScintillatorNo << (idscintillator); 	   
  new G4PVPlacement(*transformSci, ScintillatorLogical,G4String("ScintillatorPhys") 
		    + G4String(stringForScintillatorNo.str()),WorldLogVol,0,0);  

  G4VisAttributes *sciColour = new G4VisAttributes(G4Colour::Green());
  sciColour->SetVisibility(true);
  ScintillatorLogical->SetVisAttributes(sciColour);
 
  return true;
}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void TBscint06::SetDepthToLayer(G4int i) 
{
  depthToLayer = i;
  G4cout <<" DepthToLayer in Scintillator: " << depthToLayer << G4endl;
}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void TBscint06::FetchAll()
{
  config_angle  = 0.0*deg;

  db->exec("select * from scint_cern_2011July_virt;");
  db->getTuple();

  /* Scintillator dimensions*/
  dimX_scintillator10x10      = db->fetchDouble("Scint10x10_xDim");//10*cm; 
  dimY_scintillator10x10      = db->fetchDouble("Scint10x10_yDim");//10*cm; 
  thickness_scintillator10x10 = db->fetchDouble("Scint10x10_thickness");//1*cm; 

  x_place1 = 0;
  y_place1 = 0;
  z_place1 = db->fetchDouble("Scint10x10_first_z_position");// -948.3*mm; 

  x_place2 = 0;
  y_place2 = 0;
  z_place2 = db->fetchDouble("Scint10x10_second_z_position");//-328.3*mm;

  
  /* take into account configuration angle and translation*/
  density                = db->fetchDouble("scintillator_density")*g/cm3;;
  polystyrene_fraction	 = db->fetchDouble("scintillator_polystyrene_fraction");
  polypropylene_fraction = db->fetchDouble("scintillator_polypropylene_fraction");
  aluminium_fraction	 = db->fetchDouble("scintillator_aluminium_fraction");
  
  x_place1 = translateX*mm;
  y_place1 = translateY*mm;
  x_place2 = translateX*mm;
  y_place2 = translateY*mm;
  
  Print();
}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void TBscint06::Print()
{
  G4cout << "\nTBscint06 information: " << G4endl
	 << " z_place1: "            << z_place1 << " mm " << G4endl
	 << " z_place2: "            << z_place2 << " mm " << G4endl
	 << G4endl;  
}
