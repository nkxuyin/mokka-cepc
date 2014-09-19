#ifndef TBcatcher00_h
#define TBcatcher00_h 1

#include "Control.hh"
#include "VSubDetectorDriver.hh"

#include "TBSD.hh"

#include "G4Material.hh"
#include "G4VPhysicalVolume.hh"

class G4LogicalVolume;
class Database;

class TBcatcher00 : public VSubDetectorDriver
{

public:
  enum layer_start { AlongX, AlongZ, Undefined };

  TBcatcher00() : VSubDetectorDriver("TBcatcher00","TBcatcher"),
		  db(0)
  {}

  ~TBcatcher00();

  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *WorldLog);

private:

  G4bool BuildCatcher();
  void BuildElements();
  void SetSD();
  void BuildLayer(G4LogicalVolume *WorldLog, G4int nlay);
  void BuildSensitive(G4VPhysicalVolume *sPV, layer_start ltype);

  void FetchAll();
  void Print();

private:
  
  G4LogicalVolume *WorldLogical, *DetectorLogical, 
    *LayerLogical, *SteelLogical, *PolyLogical,
    *PolyHorLogical, *PolyVertLogical,
    *CellHorLogical, *CellVertLogical;

  G4Material *steel, *poly;

  G4double n_layers, y_place;
  G4int n_cell;
  G4double cal_hx, cal_hy, cal_hz;
  layer_start layer_config;
  G4double cell_hwidth;
  G4double poly_hthickness, steel_hthickness, layer_hthickness;

  Database *db;
  TBSD *catcherSD;
};

#endif
