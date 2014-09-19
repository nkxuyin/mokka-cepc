#ifndef TBdchXY07_h
#define TBdchXY07_h 1

#include "Control.hh"
#include "VSubDetectorDriver.hh"
#include "TBSD_Dch01.hh"
#include "G4Transform3D.hh"

#include "G4LogicalVolume.hh"
#include "G4Material.hh"

class Database;

class TBdchXY07 : public VSubDetectorDriver
{
public:
  TBdchXY07() : VSubDetectorDriver("TBdchXY07","TBdch"),
	       db(0)
  {}

  ~TBdchXY07();

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

  /*Set the level on which the layer is implemented in G4*/
  void SetDepthToLayer(G4int i);

private:

  bool checkForOverlappingVolumes;
  
  /* configuration angle of detector*/
  G4double config_angle, translateX, translateY;

  /* Variables containing info on detector dimensions*/
  G4double x_placeX1, x_placeX2, x_placeX3;
  G4double y_placeX1, y_placeX2, y_placeX3;
  G4double z_placeX1, z_placeX2, z_placeX3;
  G4double x_placeY1, x_placeY2, x_placeY3;
  G4double y_placeY1, y_placeY2, y_placeY3;
  G4double z_placeY1, z_placeY2, z_placeY3;
  G4double dch_hx, dch_hy, dch_hz, layer_hthickness;
  G4double kaptonF_hz, kaptonB_hz, gas_hz;
  G4double dch_hthickness;
  G4double gas_temperature, gas_pressure, gas_thickness;
  G4double wirechamber_X_dimension;
  G4double wirechamber_Y_dimension;
  G4double wirechamber_kapton_windowback_hthickness;
  G4double wirechamber_kapton_windowfront_hthickness;
  G4double wirechamber_thickness;
  G4double Wire_chambers_CO2_density;
  G4double Wire_chambers_ArCO2_density;

  /*depth of where the layer is implemented within the G4 volumes hierarchy*/
  G4int depthToLayer, grid_size;

  /* transform Dch position*/
  G4Transform3D *transformDch;

  /* translation of Dch*/
  G4ThreeVector translateDch;

  /* material definition */
  G4Element  *elC, *elO;
  G4Material *CO2, *gas_mix, *kapton, *air, *argon;

  /* logical volumes*/
  G4LogicalVolume *KaptonFrontLogical, *GasLogical_X, *GasLogical_Y, *KaptonBackLogical;
  G4LogicalVolume *WorldLogVol, *DetectorLogical;
  TBSD_Dch01 *dchSD;

  Database* db;
};


#endif
