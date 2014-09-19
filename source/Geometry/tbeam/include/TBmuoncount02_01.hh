#ifndef TBmuoncount02_01_h
#define TBmuoncount02_01_h 1

#include "Control.hh"
#include "VSubDetectorDriver.hh"
#include "TBSD_VCell03.hh"
//#include "TBSD_Dch01.hh"
#include "G4Transform3D.hh"

class TBSD_VCell03;
//class TBSD_Dch01;
class Database;

class G4LogicalVolume;
class G4Material;

class TBmuoncount02_01 : public VSubDetectorDriver
{
public:
  TBmuoncount02_01() : VSubDetectorDriver("TBmuoncount02_01","TBmuoncount"),
	       db(0)
  {}

  ~TBmuoncount02_01();

  G4bool ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
			     G4LogicalVolume *theWorld);

  G4double GetGridSize() { return grid_size; }
  G4int GetDepthToLayer() { return depthToLayer; }

private:
  
  G4bool SciConstruct(G4LogicalVolume *WorldLog, G4double xdim, G4double ydim,
		      G4double x_place, G4double y_place, G4double z_place, G4int idsc);
  void FetchAll();
  void BuildElements(G4double xdim, G4double ydim);
  G4bool BuildSci(G4double x_place, G4double y_place, G4double z_place, G4int idsc);
  void SetSD(G4int idsc);
  void Print();

  //Set the level on which the layer is implemented in G4
  void SetDepthToLayer(G4int i);

private:

  // configuration angle of detector
  G4double config_angle, translateX, translateY;
  G4int nslice;
  G4double diamond_angle;

  // Variables containing info on detector dimensions
  G4double ncell_xy[2];
  G4double x_place1, y_place1, z_place1;
  G4double sci_hx, sci_hy, sci_hz, sci_hthickness, sci_window;
  G4double steelF_hz, steelB_hz;

  //depth of where the layer is implemented within the G4 volumes hierarchy
  G4int depthToLayer;
  G4double grid_size;

  // transform Scintillator position
  G4Transform3D *transformSci;

  // translation of Scintillator
  G4ThreeVector translateSci;

  // material definition 
  G4Element  *elO, *elN;
  G4Material *air;
  G4Material *poly;
  G4Material *steel;

  // logical volumes
  G4LogicalVolume *SensitiveLogical;
  G4LogicalVolume *SteelFrontLogical, *SteelBackLogical;
  G4LogicalVolume *WorldLogVol, *DetectorLogical;

//  TBSD_VCell03 *dchSD;
  Database* db;
};


#endif
