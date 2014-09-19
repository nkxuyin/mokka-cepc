#ifndef TBscint06_h
#define TBscint06_h 1

#include "Control.hh"
#include "VSubDetectorDriver.hh"
#include "G4Transform3D.hh"
#include "G4Element.hh"
#include "G4Material.hh"

class Database;

class TBscint06 : public VSubDetectorDriver
{
public:
  TBscint06() : VSubDetectorDriver("TBscint06","TBscint"),
		db(0)
  {}
  
  ~TBscint06();
  
  G4bool ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment, G4LogicalVolume *theWorld);

private:
  
  G4bool SciConstruct(G4LogicalVolume *WorldLog, 
		      G4double x_place, G4double y_place, G4double z_place, G4int idscintillator);
  void FetchAll();
  void BuildElements(G4int idscintillator);
  G4bool BuildSci(G4double x_place, G4double y_place, G4double z_place, G4int idscintillator);
  void Print();

  /*Set the level on which the layer is implemented in G4*/
  void SetDepthToLayer(G4int i);

private:

  /* configuration angle of detector*/
  G4double config_angle, translateX, translateY;

  /* Variables containing info on detector dimensions*/
  G4double dimX_scintillator10x10;
  G4double dimY_scintillator10x10;
  G4double thickness_scintillator10x10;

  G4double density;		  
  G4double polystyrene_fraction;	  
  G4double polypropylene_fraction;   
  G4double aluminium_fraction;	  
  G4double x_place1, x_place2;
  G4double y_place1, y_place2;
  G4double z_place1, z_place2;
  G4double sci_hx, sci_hy, sci_hz;

  /*depth of where the layer is implemented within the G4 volumes hierarchy*/
  G4int depthToLayer; 

  /* transform scintillator position*/
  G4Transform3D *transformSci;

  /* translation of scintillator*/
  G4ThreeVector translateSci;

  /* material definition */
  G4Element  *elO, *elN;
  G4Material *air;
  G4Material *poly;

  /* logical volumes*/
  G4LogicalVolume *WorldLogVol, *ScintillatorLogical;

  Database* db;
};


#endif
