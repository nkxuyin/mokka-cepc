#ifndef TBecal00_h
#define TBecal00_h 1

#include "Control.hh"
#include "VSubDetectorDriver.hh"
#include "TBSD.hh"

class G4LogicalVolume;
class Database;
class G4Material;

class TBecal00 : public VSubDetectorDriver
{
public:
  TBecal00() : VSubDetectorDriver("TBecal00","TBecal"),
	       db(0)
  {}

  ~TBecal00();

  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *WorldLog);

private:
  
  G4bool BuildEcal();
  void FetchAll();
  void BuildElements();
  void SetSD();
  void BuildLayer(G4LogicalVolume *DetLog,G4int nlay);
  void Print();

private:

  G4Material *w, *cu, *g10, *si, *air;
  G4LogicalVolume *WholeLayerLogical, *WLogical, *CuLogical, *SiLogical, *CellLogical, *G10Logical, *AirLogical;
  G4LogicalVolume *WorldLogical, *DetectorLogical;

  G4int n_layers;
  G4double y_place;
  G4double cal_hx, cal_hy, cal_hz, layer_hthickness;
  G4double cell_dim_hx, cell_dim_hz, n_cell_x, n_cell_z;
  G4double w_hthickness, cu_hthickness, g10_hthickness, si_hthickness, air_hthickness;

  TBSD *ecalSD;
  Database* db;
};


#endif
