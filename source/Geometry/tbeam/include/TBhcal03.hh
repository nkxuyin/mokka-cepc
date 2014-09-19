#ifndef TBhcal03_h
#define TBhcal03_h 1

#include "Control.hh"
#include "TBSD_VCell03.hh"
#include "VSubDetectorDriver.hh"

class TBSD_VCell03;

class Database;

class G4LogicalVolume;
class G4Material;
class G4VPhysicalVolume;

class TBhcal03 : public VSubDetectorDriver
{

public:

  TBhcal03();
  ~TBhcal03();

public:

  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *WorldLog);

  G4double GetGridSize() { return grid_size; }
  G4int GetDepthToLayer() const { return depthToLayer; }
  
private:

  G4bool BuildHcal();
  void BuildElements();  
  void SetSD(); // called only after cell LV creation
  void PlaceLayer(G4LogicalVolume*, G4int);

  void FetchAll();

  void Print();

  // New Materaial needed for prototype and not yet included in Mokka
  // Material list should be defined here;
  void AddMaterial();

  //Set the level on which the layer is implemented in G4
  void SetDepthToLayer(G4int i) {
    depthToLayer = i;
#ifdef TBSD_DEBUG
    G4cout <<"DepthToLayer in Hcal: " << depthToLayer << G4endl;
#endif
  }
private: 

  Database* db;
  TBSD_VCell03 *hcalSD;
  
  //Variables containing info on detector dimensions
  G4int n_layers, ncell_xy[2]; // n layers, number of cells in xy-direction
  G4double grid_size; // Parameter determing the cellwise subdivision of a layer
  G4double cal_hx, cal_hy, cal_hz; // // Outer dimensions of the detector
  G4double z_begin, z_place, x_offset; //Coordinates where the detector begins and where it is to be put

  //configuration angle of detector
  G4double config_angle;

  //Variable describing the depth of where the layer is implemented
  // within the G4 volumes hierarchy
  G4int depthToLayer; 

  //dimensions of the layer components along z
  G4double poly_hthickness, steel_hthickness, layer_hthickness; 
  G4double airgap_hthickness, steel_cassette_hthickness, foil_hthickness,
    pcb_hthickness, cablefibre_mix_hthickness;
    
  //Logical volumes introduced for detector construction
  G4LogicalVolume *WholeLayerLogical, 
    *WholeSensLayerLogical, 
    *AbsLayerLogical, 
    *AbsLayerLogical_term, 
    *DetectorLogical, 
    *ScinHousLogical,
    *FoilLayerLogical_1,
    *FoilLayerLogical_2,
    *PCBLayerLogical,
    *CFmix_LayerLogical,
    *WorldLogical;

  //Materials used for detector construction
  G4Material *steel, *poly, *air, *pcb, *foil_3m, *cf_mix;
  G4Material *PCB, *S235, *CF_MIX, *Polystyrole;
};

#endif


