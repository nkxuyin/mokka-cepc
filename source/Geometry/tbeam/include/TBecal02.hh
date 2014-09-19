#ifndef TBecal02_h
#define TBecal02_h 1

#include "Control.hh"
#include "VSubDetectorDriver.hh"
#include "TBSD_VCell02.hh"

//class TBSD_VCell02;
class G4LogicalVolume;
class Database;
class G4Material;

class TBecal02 : public VSubDetectorDriver
{
public:
  TBecal02() : VSubDetectorDriver("TBecal02","TBecal"),
	       db(0)
  {}

  ~TBecal02();

  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *WorldLog);

  const G4int* GetNCells() {return &ncell_xz[0];}
  const G4double* GetGridSize() {return &grid_size;}

private:
  
  G4bool BuildEcal();
  void FetchAll();
  void BuildElements();
  void SetSD();
  void BuildLayer(G4LogicalVolume *DetLog,G4int nlay);
  void Print();

private:

  G4int n_layers, ncell_xz[2];
  G4double grid_size;
  G4double y_place;
  G4double cal_hx, cal_hy, cal_hz, layer_hthickness;
  //G4double cell_dim_hx, cell_dim_hz, n_cell_x, n_cell_z;
  G4double w_hthickness, cu_hthickness, g10_hthickness, si_hthickness, air_hthickness;

  G4Material *w, *cu, *g10, *si, *air;
  G4LogicalVolume *WholeLayerLogical, *WLogical, *CuLogical, *SiLogical, *CellLogical, *G10Logical, *AirLogical;
  G4LogicalVolume *WorldLogical, *DetectorLogical;

  TBSD_VCell02 *ecalSD;
  Database* db;
};


#endif
