#ifndef TBscint05_h
#define TBscint05_h 1

#include "Control.hh"
#include "VSubDetectorDriver.hh"
#include "TBSD_VCell03.hh"
#include "G4Transform3D.hh"

class TBSD_VCell03;
//class TBSD_Dch01;
class Database;

class G4LogicalVolume;
class G4Material;

class TBscint05 : public VSubDetectorDriver
{
public:
  TBscint05() : VSubDetectorDriver("TBscint05","TBscint"),
	       db(0)
  {}

  ~TBscint05();

  G4bool ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
			     G4LogicalVolume *theWorld);
  //  G4double GetGridSize() { return grid_size; }
  //  G4int GetDepthToLayer() { return depthToLayer; }

private:
  
  G4bool SciConstruct(G4LogicalVolume *WorldLog, G4double xdim, G4double ydim,
		      G4double x_place, G4double y_place, G4double z_place, G4int idscintillator);
  void FetchAll();
  void BuildElements(G4double xdim, G4double ydim, G4int idscintillator);
  G4bool BuildSci(G4double x_place, G4double y_place, G4double z_place, G4int idscintillator);
  void Print();

  //Set the level on which the layer is implemented in G4
  void SetDepthToLayer(G4int i);

private:

  // configuration angle of detector
  G4double ecal_front_face, config_angle, translateX, translateY;
  G4int nslice;
  G4double diamond_angle;

  // Variables containing info on detector dimensions
  G4double Scintillator_betweenHCAL_And_WireChamber_X;      
  G4double Scintillator_betweenHCAL_And_WireChamber_Y;        
  G4double Scintillator_betweenHCAL_And_WireChamber_thickness;
  G4double First_Scintillator_betweenHCAL_And_WireChamber_z_position;
  G4double Second_Scintillator_betweenHCAL_And_WireChamber_z_position;
  G4double scintillator_betweenHCAL_And_WireChamber_density;		  
  G4double scintillator_betweenHCAL_And_WireChamber_polystyrene_fraction;	  
  G4double scintillator_betweenHCAL_And_WireChamber_polypropylene_fraction;   
  G4double scintillator_betweenHCAL_And_WireChamber_aluminium_fraction;	  
  G4double x_place1, x_place2;
  G4double y_place1, y_place2;
  G4double sci_hx, sci_hy, sci_hz;

  //depth of where the layer is implemented within the G4 volumes hierarchy
  G4int depthToLayer; 

  // transform Scintillator position
  G4Transform3D *transformSci;

  // translation of Scintillator
  G4ThreeVector translateSci;

  // material definition 
  G4Element  *elO, *elN;
  G4Material *air;
  G4Material *poly;

  // logical volumes
  G4LogicalVolume *WorldLogVol, *ScintillatorLogical;

//  TBSD_VCell03 *dchSD;
  Database* db;
};


#endif
