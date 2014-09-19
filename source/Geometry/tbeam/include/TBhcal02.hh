#ifndef TBhcal02_h
#define TBhcal02_h 1

#include "Control.hh"
#include "TBSD_VCell02.hh"
#include "VSubDetectorDriver.hh"

class G4LogicalVolume;
class Database;
class G4Material;
class G4VPhysicalVolume;
class TBSD_VCell02;

class TBhcal02 : public VSubDetectorDriver
{
public:
  TBhcal02() : VSubDetectorDriver("TBhcal02","TBhcal"),
               db(0)
  {
  }

  ~TBhcal02();

  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *WorldLog);

  void Print();
  const G4int* GetNCells(){return &ncell_xz[0];}
  const G4double* GetGridSize(){return &grid_size;}


private:
  void BuildElements();  
  void SetSD(); // called only after cell LV creation
  void PlaceLayer(G4LogicalVolume*,G4int);
  //void BuildSensitive(G4VPhysicalVolume* SensPhys, G4bool is_complex);
  G4bool BuildHcal();

  //  void ReplicateCells(G4LogicalVolume *mLV, G4LogicalVolume *cLV);

  void FetchAll();

  // New Materaial needed for prototype and not yet included in Mokka
  // Material list should be defined here;
  void AddMaterial();

private:
  G4int n_layers, ncell_xz[2]; // n layers, number of cells in xz-direction
  G4double grid_size;
  G4double poly_hthickness, steel_hthickness, layer_hthickness; // along Y
  G4double airgap_hthickness, steel_cassette_hthickness, foil_hthickness,
    pcb_hthickness, cablefibre_mix_hthickness;
  G4double cal_hx, cal_hy, cal_hz; // cal parameters
  G4double y_place;

    
  G4LogicalVolume *WholeLayerLogical, 
    *WholeSensLayerLogical, 
    *AbsLayerLogical, 
    *DetectorLogical, 
    *ScinHousLogical,
    *AluLayerLogical,
    *PCBLayerLogical,
    *CFmix_LayerLogical,
    *WorldLogical;

  G4Material *steel, *poly, *air, *pcb, *alu;

  //add PCB as Material
  // namespace causes error on gcc 3.3 --JM
  // G4Material* TBhcal02::PCB;
  G4Material *PCB;
  Database* db;
  TBSD_VCell02 *hcalSD;
};

#endif
