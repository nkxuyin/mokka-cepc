#ifndef TBdchXY03_h
#define TBdchXY03_h 1

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

class TBdchXY03 : public VSubDetectorDriver
{
public:
  TBdchXY03() : VSubDetectorDriver("TBdchXY03","TBdch"),
	       db(0)
  {}

  ~TBdchXY03();

  G4bool ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
			     G4LogicalVolume *theWorld);

  G4double GetGridSize() { return grid_size; }
  G4int GetDepthToLayer() { return depthToLayer; }

private:
  
  G4bool DchConstruct(G4LogicalVolume *WorldLog,
		      G4double x_place, G4double y_place, G4double z_place,
		      G4int idch);
  G4bool BuildDch(G4double x_place, G4double y_place, G4double z_place, G4int idch);
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
  G4double x_place1, x_place2, x_place3;
  G4double y_place1, y_place2, y_place3;
  G4double z_place1, z_place2, z_place3;
  G4double dch_hx, dch_hy, dch_hz, layer_hthickness;
  G4double kaptonF_hz, kaptonB_hz, gas_hz;
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
  G4Material *elAr, *CO2, *gas_mix, *kapton, *air;

  // logical volumes
  G4LogicalVolume *KaptonFrontLogical, *GasLogical, *KaptonBackLogical;
  G4LogicalVolume *WorldLogVol, *DetectorLogical;
  TBSD_Dch01 *dchSD;

//  TBSD_VCell03 *dchSD;
  Database* db;
};


#endif
