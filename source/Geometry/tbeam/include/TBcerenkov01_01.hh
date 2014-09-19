#ifndef TBcerenkov01_01_h
#define TBcerenkov01_01_h 1

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

class TBcerenkov01_01 : public VSubDetectorDriver
{
public:
  TBcerenkov01_01() : VSubDetectorDriver("TBcerenkov01_01","TBcerenkov"),
	       db(0)
  {}

  ~TBcerenkov01_01();

  G4bool ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
			     G4LogicalVolume *theWorld);

  G4double GetGridSize() { return grid_size; }
  G4int GetDepthToLayer() { return depthToLayer; }

private:
  
  G4bool CerenConstruct(G4LogicalVolume *WorldLog,
			G4double x_place, G4double y_place, G4double z_place,
			G4int iceren);
  G4bool BuildCeren(G4double x_place, G4double y_place, G4double z_place);
  void FetchAll();
  void BuildElements();
  void SetSD(G4int iceren);
  void Print();

  //Set the level on which the layer is implemented in G4
  void SetDepthToLayer(G4int i);

private:

  // configuration angle of detector
  G4double config_angle, ecal_front_face, translateX, translateY;
  G4int nslice;

  // Variables containing info on detector dimensions
  G4double ncell_xy[2];
  G4double x_place1, y_place1, z_place1;
  G4double ceren_hx, ceren_hy, ceren_hz, layer_hthickness;
  G4double mylarF_hz, mylarB_hz, gas_hz;
  G4double winfront_hthickness, winback_hthickness, gas_hthickness, ceren_hthickness;
  G4double gas_temperature, gas_pressure;

  //depth of where the layer is implemented within the G4 volumes hierarchy
  G4int depthToLayer, grid_size;

  // translation of Cerenkov
  G4ThreeVector translateCeren;
  G4Transform3D *transformCeren;

  // material definition 
  G4Element  *elH, *elC, *elO, *elN;
  G4Material *elAr, *elHe, *mylar, *air;

  // logical volumes
  G4LogicalVolume *MylarFrontLogical, *GasLogical, *MylarBackLogical;
  G4LogicalVolume *WorldLogVol, *DetectorLogical;

  Database* db;
};


#endif
