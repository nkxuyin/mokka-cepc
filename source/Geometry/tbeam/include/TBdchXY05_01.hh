#ifndef TBdchXY05_01_h
#define TBdchXY05_01_h 1

#include "Control.hh"
#include "VSubDetectorDriver.hh"
//#include "TBSD_VCell03.hh"
#include "TBSD_Dch01.hh"
#include "G4Transform3D.hh"

//class TBSD_VCell03;
class TBSD_Dch01;
class Database;

class G4LogicalVolume;
class G4Material;

class TBdchXY05_01 : public VSubDetectorDriver
{
public:
  TBdchXY05_01() : VSubDetectorDriver("TBdchXY05_01","TBdch"),
	       db(0)
  {}

  ~TBdchXY05_01();

  G4bool ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
			     G4LogicalVolume *theWorld);

  G4double GetGridSize() { return grid_size; }
  G4int GetDepthToLayer() { return depthToLayer; }

private:
  
  G4bool DchConstruct(G4LogicalVolume *WorldLog,
		      G4double x_place, G4double y_place, G4double z_place,
		      G4int idch);
  G4bool BuildDchX(G4double x_place, G4double y_place, G4double z_place, G4int idch);
  G4bool BuildDchY(G4double x_place, G4double y_place, G4double z_place, G4int idch);
  void FetchAll();
  void BuildElements();
  void SetSD(G4int idch);
  void Print();

  //Set the level on which the layer is implemented in G4
  void SetDepthToLayer(G4int i);

private:

  // configuration angle of detector
  G4double ecal_front_face, config_angle, translateX, translateY;
  G4int nslice;

  // Variables containing info on detector dimensions
  G4double ncell_xy[2];
  G4double x_placeX1, x_placeX2, x_placeX3, x_placeX4;
  G4double y_placeX1, y_placeX2, y_placeX3, y_placeX4;
  G4double z_placeX1, z_placeX2, z_placeX3, z_placeX4;
  G4double x_placeY1, x_placeY2, x_placeY3, x_placeY4;
  G4double y_placeY1, y_placeY2, y_placeY3, y_placeY4;
  G4double z_placeY1, z_placeY2, z_placeY3, z_placeY4;
  G4double dch_hx, dch_hy, dch_hz, layer_hthickness;
  G4double mylarF_hz, mylarB_hz, gas_hz;
  G4double winfront_hthickness, winback_hthickness, gas_hthickness, dch_hthickness;
  G4double gas_temperature, gas_pressure;

  //depth of where the layer is implemented within the G4 volumes hierarchy
  G4int depthToLayer, grid_size;

  // transform Dch position
  G4Transform3D *transformDch;

  // translation of Dch
  G4ThreeVector translateDch;

  // material definition 
  G4Element  *elH, *elC, *elO, *elN;
  G4Material *elAr, *C2H6, *gas_mix, *mylar, *air;

  // logical volumes
  G4LogicalVolume *MylarFrontLogical, *GasLogical, *MylarBackLogical;
  G4LogicalVolume *WorldLogVol, *DetectorLogical;
  TBSD_Dch01 *dchSD;

//  TBSD_VCell03 *dchSD;
  Database* db;
};


#endif
