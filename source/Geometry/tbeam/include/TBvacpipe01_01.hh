#ifndef TBvacpipe01_01_h
#define TBvacpipe01_01_h 1

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

class TBvacpipe01_01 : public VSubDetectorDriver
{
public:
  TBvacpipe01_01() : VSubDetectorDriver("TBvacpipe01_01","TBvacpipe"),
	       db(0)
  {}

  ~TBvacpipe01_01();

  G4bool ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
			     G4LogicalVolume *theWorld);

  G4double GetGridSize() { return grid_size; }
  G4int GetDepthToLayer() { return depthToLayer; }

private:
  
  G4bool VacConstruct(G4LogicalVolume *WorldLog,
			G4double x_place, G4double y_place, G4double z_place,
			G4int ivac);
  G4bool BuildVac(G4double x_place, G4double y_place, G4double z_place);
  void FetchAll();
  void BuildElements();
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
  G4double vacpipe_hx, vacpipe_hy, vacpipe_hz;
  G4double vacpipe_hthickness;
  G4double temperature, pressure;

  //depth of where the layer is implemented within the G4 volumes hierarchy
  G4int depthToLayer, grid_size;

  // translation of Cerenkov
  G4ThreeVector translateVac;
  G4Transform3D *transformVac;

  G4Material *Vacuum;

  // logical volumes
  G4LogicalVolume *GasLogical;
  G4LogicalVolume *WorldLogVol, *DetectorLogical;

  Database* db;
};


#endif
