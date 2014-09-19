#ifndef TBcatcher02_h
#define TBcatcher02_h 1

#include "Control.hh"
#include "VSubDetectorDriver.hh"

//#include "TBSD.hh"
#include "TBSD_VCell02.hh"

#include "G4Material.hh"
#include "G4VPhysicalVolume.hh"

class G4LogicalVolume;
class Database;

class TBcatcher02 : public VSubDetectorDriver
{

public:
  enum layer_start { AlongX, AlongZ, Undefined };

  TBcatcher02() : VSubDetectorDriver("TBcatcher02","TBcatcher"),
		  db(0)
  {}

  ~TBcatcher02();

  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *WorldLog);

  const G4int* GetNCells() {return &ncell_xz[0];}
  const G4double* GetGridSize() {return &grid_size;}

private:

  G4bool BuildCatcher();
  void BuildElements();
  void SetSD();
  void BuildLayer(G4LogicalVolume *WorldLog, G4int nlay);
  void BuildSensitive(G4VPhysicalVolume *sPV, layer_start ltype);

  void FetchAll();
  void Print();

private:
  G4int ncell_xz[2];
  G4double grid_size;
  G4double n_layers, y_place;

  // G4int n_cell;
  G4double cal_hx, cal_hy, cal_hz;
  //layer_start layer_config;
  //G4double cell_hwidth;
  G4double poly_hthickness, steel_hthickness, layer_hthickness;

  G4LogicalVolume *WorldLogical, *DetectorLogical, 
    *LayerLogical, *SteelLogical, *PolyLogical,
    *PolyHorLogical, *PolyVertLogical,
    *CellHorLogical, *CellVertLogical;
  
  G4Material *steel, *poly;

  Database *db;
  TBSD_VCell02 *catcherSD;
};

#endif
