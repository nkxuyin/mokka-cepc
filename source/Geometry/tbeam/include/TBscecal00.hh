#ifndef TBscecal00_h
#define TBscecal00_h 1

#include "Control.hh"
#include "VSubDetectorDriver.hh"
#include "TBSD_VCell02.hh"

//class TBSD_VCell02;
class G4LogicalVolume;
class Database;
class G4Material;

class TBscecal00 : public VSubDetectorDriver
{
public:
  TBscecal00() : VSubDetectorDriver("TBscecal00","TBscecal"),
	       db(0)
  {}

  ~TBscecal00();

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
//  G4double w_hthickness, cu_hthickness, g10_hthickness, si_hthickness, air_hthickness;
  G4double w_hthickness, reffront_hthickness, sc_hthickness, refrear_hthickness, mixgap_hthickness, air_hthickness;

//  G4Material *w, *cu, *g10, *si, *air;
  G4Material *w, *reffront, *sc, *refrear, *mixgap, *g10, *air;
//  G4LogicalVolume *WholeLayerLogical, *WLogical, *CuLogical, *SiLogical, *CellLogical, *G10Logical, *AirLogical;
  G4LogicalVolume *WholeLayerLogical, *WLogical, *FrontRefLogical, *ScLogical, *RearRefLogical, *MixgapLogical, *AirLogical;
  G4LogicalVolume *WorldLogical, *DetectorLogical;

  TBSD_VCell02 *ecalSD;
  Database* db;
};


#endif
