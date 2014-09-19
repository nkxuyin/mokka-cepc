//##########################################
//                                         #
//  Driver used to simulate the 100x100mm  #
//  CERN 2010 Test beam scintillators      #
//  of the coordinate system)              #
//                                         #
//##########################################

#include "TBscint05.hh"
#include "Control.hh"
#include "CGADefs.h"
#include "MySQLWrapper.hh"
#include "G4Material.hh"
#include "CGAGeometryManager.hh"

#include "G4UnitsTable.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4NistManager.hh"

#include <sstream>

INSTANTIATE(TBscint05)

TBscint05::~TBscint05()
{}

G4bool TBscint05::ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment, G4LogicalVolume *theWorld)
{

 // variable config_angle (a double) will be used by method "construct" to place the detector
 config_angle    = 0.0;
 
 /* variables translateX and translateY have default value (0,0) and are used
    to translate Sci wrt beam axis (user defined parameters)
    translateX      = aGeometryEnvironment.GetParameterAsDouble("TranslateX");
    translateY      = aGeometryEnvironment.GetParameterAsDouble("TranslateY"); */
    
 translateX      = 0.0;
 translateY      = 0.0;

 G4cout << "\nBuilding TBscint05..." << G4endl;
 db = new Database(aGeometryEnvironment.GetDBName());

 // fetch db parms
 FetchAll();
  
 G4bool doScintillator_1 = SciConstruct(theWorld,
					Scintillator_betweenHCAL_And_WireChamber_X,
					Scintillator_betweenHCAL_And_WireChamber_Y,
					x_place1,
					y_place1,
					First_Scintillator_betweenHCAL_And_WireChamber_z_position,
					1); 
 G4bool doScintillator_2 = SciConstruct(theWorld,
					Scintillator_betweenHCAL_And_WireChamber_X,
					Scintillator_betweenHCAL_And_WireChamber_Y,
					x_place2,
					y_place2,
					Second_Scintillator_betweenHCAL_And_WireChamber_z_position,
					2); 

 delete db;
 db = 0;
  
 G4bool doScintillators = false;
 if (doScintillator_1 && doScintillator_2 ) doScintillators = true;

 G4cout << "\nDone building TBscint05" << G4endl;
 return doScintillators;

}

G4bool TBscint05::SciConstruct(G4LogicalVolume *WorldLog, G4double xdim, G4double ydim,
 		 	       G4double x_place, G4double y_place, G4double z_place, 
			       G4int idscintillator)
{

  G4cout << " Building Scintillator " << idscintillator << G4endl;
  WorldLogVol = WorldLog;

  // depth to layer
  SetDepthToLayer(1);

  G4cout << " Building Scintillator elements " << G4endl;
  BuildElements(xdim, ydim, idscintillator);

  // do build process
  G4bool cokay = BuildSci(x_place, y_place, z_place, idscintillator);
  return cokay;
}


void TBscint05::BuildElements(G4double xdim, G4double ydim, G4int idscintillator)
{
 
  //Useful stuff
  G4String name; 
  G4int nelements;

  //Scintillator materials
  //The density is callculated as 90% of polystyrene (1.06), 4% Al (2.699) and 6% tape (0.9)
  G4Material* Scintillator_material = new G4Material(name="Scintillator_material",
						     scintillator_betweenHCAL_And_WireChamber_density,
						     nelements=3); 
  Scintillator_material->AddMaterial(CGAGeometryManager::GetMaterial("polystyrene"),
				     scintillator_betweenHCAL_And_WireChamber_polystyrene_fraction);
  Scintillator_material->AddMaterial(CGAGeometryManager::GetMaterial("Aluminium"),
				     scintillator_betweenHCAL_And_WireChamber_aluminium_fraction);
  G4NistManager* man = G4NistManager::Instance();
  G4Material*   polypropylene = man->FindOrBuildMaterial("G4_POLYPROPYLENE");
  Scintillator_material->AddMaterial(polypropylene,scintillator_betweenHCAL_And_WireChamber_polypropylene_fraction);

  G4cout<<"Fraction of materials in Scintillators : "<< G4endl;
  G4cout<<"scintillator_betweenHCAL_And_WireChamber_polystyrene_fraction " << scintillator_betweenHCAL_And_WireChamber_polystyrene_fraction << G4endl;
  G4cout<<"scintillator_betweenHCAL_And_WireChamber_aluminium_fraction " << scintillator_betweenHCAL_And_WireChamber_aluminium_fraction << G4endl;
  G4cout<<"scintillator_betweenHCAL_And_WireChamber_polypropylene_fraction "<<scintillator_betweenHCAL_And_WireChamber_polypropylene_fraction << G4endl;
  G4cout<<"scintillator_betweenHCAL_And_WireChamber_density " << G4BestUnit(scintillator_betweenHCAL_And_WireChamber_density ,"Volumic Mass") << G4endl;

  // sci (half) dimensions
  sci_hx = (xdim)/2;
  sci_hy = (ydim)/2;  
  sci_hz = Scintillator_betweenHCAL_And_WireChamber_thickness;

  //Scintillator Nr
  std::stringstream stringForScintillatorNo;
  stringForScintillatorNo << (idscintillator); 	   

  G4Box *ScintillatorSolid = new G4Box("ScintillatorSolid",sci_hx,sci_hy,sci_hz);
  ScintillatorLogical = new G4LogicalVolume(ScintillatorSolid,Scintillator_material,G4String("ScintillatorLogical") + G4String(stringForScintillatorNo.str()), 0, 0, 0);	

  G4cout << " Dimension of detector box " << G4endl;
  G4cout << " sci_hx: " << sci_hx*2 << " mm " << G4endl;
  G4cout << " sci_hy: " << sci_hy*2 << " mm " << G4endl;
  G4cout << " sci_hz: " << sci_hz   << " mm " << G4endl;

}


G4bool TBscint05::BuildSci(G4double x_place, G4double y_place, G4double z_place,G4int idscintillator)
{
  
  G4cout << " Building Scintillator structure: Sci " << idscintillator << G4endl;
  G4cout << " x_place of Scintillator detector " << x_place << G4endl;
  G4cout << " y_place of Scintillator detector " << y_place << G4endl;
  G4cout << " z_place of Scintillator detector " << z_place << G4endl;
  
  translateSci = G4ThreeVector(x_place, y_place, z_place);
  G4RotationMatrix rotateSci;
  rotateSci.rotateY(config_angle);
  transformSci = new G4Transform3D(rotateSci, translateSci);

  std::stringstream stringForScintillatorNo;
  stringForScintillatorNo << (idscintillator); 	   
  new G4PVPlacement(*transformSci,ScintillatorLogical,G4String("ScintillatorPhys") + G4String(stringForScintillatorNo.str()),WorldLogVol,0,0);  

  G4VisAttributes *sciColour = new G4VisAttributes(G4Colour::Green());
  sciColour->SetVisibility(true);
  ScintillatorLogical->SetVisAttributes(sciColour);
 
  return true;
}


void TBscint05::SetDepthToLayer(G4int i) {
  depthToLayer = i;
  G4cout <<" DepthToLayer in Scintillator: " << depthToLayer << G4endl;
}



void TBscint05::FetchAll()
{
  diamond_angle = 0.0*deg;
  config_angle  = 0.0*deg;

  db->exec("select * from scint_betweenHCAlAndwch_cern_2010_virt;");
  db->getTuple();

  // Scintillator dimensions
  Scintillator_betweenHCAL_And_WireChamber_X                       = db->fetchDouble("Scintillator_betweenHCAL_And_WireChamber_X");
  Scintillator_betweenHCAL_And_WireChamber_Y                       = db->fetchDouble("Scintillator_betweenHCAL_And_WireChamber_Y");
  Scintillator_betweenHCAL_And_WireChamber_thickness               = db->fetchDouble("Scintillator_betweenHCAL_And_WireChamber_thickness");

  First_Scintillator_betweenHCAL_And_WireChamber_z_position         = db->fetchDouble("1st_Scintillator_betweenHCAL_And_WireChamber_z_position");	
  Second_Scintillator_betweenHCAL_And_WireChamber_z_position        = db->fetchDouble("2nd_Scintillator_betweenHCAL_And_WireChamber_z_position");

  // take into account configuration angle and translation
  scintillator_betweenHCAL_And_WireChamber_density                = db->fetchDouble("scintillator_betweenHCAL_And_WireChamber_density")*g/cm3;;
  scintillator_betweenHCAL_And_WireChamber_polystyrene_fraction	  = db->fetchDouble("scintillator_betweenHCAL_And_WireChamber_polystyrene_fraction");
  scintillator_betweenHCAL_And_WireChamber_polypropylene_fraction = db->fetchDouble("scintillator_betweenHCAL_And_WireChamber_polypropylene_fraction");
  scintillator_betweenHCAL_And_WireChamber_aluminium_fraction	  = db->fetchDouble("scintillator_betweenHCAL_And_WireChamber_aluminium_fraction");
  
  x_place1 = translateX*mm;
  y_place1 = translateY*mm;
  x_place2 = translateX*mm;
  y_place2 = translateY*mm;
  
  Print();
}


void TBscint05::Print()
{
  G4cout << "\nTBscint05 information: " << G4endl
	 << " z_place1: "            << First_Scintillator_betweenHCAL_And_WireChamber_z_position          << " mm " << G4endl
	 << " z_place2: "            << Second_Scintillator_betweenHCAL_And_WireChamber_z_position         << " mm " << G4endl
	 << G4endl;  
}
