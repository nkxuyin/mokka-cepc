#ifndef TBhcal05_h
#define TBhcal05_h 1

#include "Control.hh"
#include "TBSD_VCell03.hh"
#include "VSubDetectorDriver.hh"
#include "CGAGeometryEnvironment.hh"

class TBSD_VCell03;

class Database;

class G4LogicalVolume;
class G4Material;
class G4VPhysicalVolume;

class TBhcal05 : public VSubDetectorDriver
{

public:

  TBhcal05();
  ~TBhcal05();

public:

  //Use contruction method via environment object as available since Mokka release 4.0
  //to access setup parameters
  G4bool ContextualConstruct(const
    CGAGeometryEnvironment &aGeometryEnvironment,
    G4LogicalVolume *theWorld);


  G4double GetGridSize() { return grid_size; }
  G4int GetDepthToLayer() const { return depthToLayer; }
  
private:

  G4bool BuildHcal();
  void BuildElements();
  void buildLayerGeometry1();
  void buildLayerGeometry2();

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
  

  //The Environment object
  CGAGeometryEnvironment _aGeometryEnvironment;

  //Variables containing info on detector dimensions
  G4int n_layers, ncell_xy[2]; // n layers, number of cells in xy-direction
  G4double grid_size; // Parameter determing the cellwise subdivision of a layer
  G4double cal_hx, cal_hy, cal_hz; // // Outer dimensions of the detector
  G4double x_begin, y_begin, z_begin, z_place, x_offset; //Coordinates where the detector begins and where it is to be put
  
  //configuration angle of detector
  G4double config_angle;
  
  //arrangement of sensitive layers in the HCAL prototype (e.g. "101010...10 indicates that each second layer is equiped with a sensitive casette)
  std::vector<G4int> sensitiveLayerPatternVector;
  
  //Variable describing the depth of where the layer is implemented
  // within the G4 volumes hierarchy
  G4int depthToLayer; 

  //dimensions of the layer components along z
  G4double poly_hthickness, steel_hthickness, layer_hthickness; 
  G4double airgap_hthickness, steel_cassette_hthickness, foil_hthickness,
    pcb_hthickness, cablefibre_mix_hthickness;
    
  G4double airGapThickness; // full thickness of the airgap, which can be placed instead of a sensitive cassette

  //Variables for terminating absorber
  G4int n_layer_term;
  G4double steel_hthickness_term;

  //Logical volumes introduced for detector construction
  G4LogicalVolume *WholeLayerLogical, *WholeLayer2Logical,
    *WholeSensLayerLogical, 
    *AbsLayerLogical, 
    *AbsLayerLogical_term, 
    *DetectorLogical, 
    *ScinHousLogical,
    *FoilLayerLogical_1,
    *FoilLayerLogical_2,
    *PCBLayerLogical,
    *CFmix_LayerLogical,
    *WorldLogical,
    *AirGap_LayerLogical;

  //Materials used for detector construction
  G4Material *steel, *poly, *air, *pcb, *foil_3m, *cf_mix;
  G4Material *PCB, *S235, *CF_MIX, *Polystyrole;


  // checks if 'sensitiveLayerPattern' is a valid string consisting of only 0's and 1's with a length of 'n_layers'.
  // The variable 'n_layers' needs to be set correctly before using this method.
  G4bool isValidSensitiveLayerPattern(G4String sensitiveLayerPattern);
  // fill std::vector<G4int> according to the string 'sensitiveLayerPattern'
  // The variable 'n_layers' needs to be set correctly before using this method.
  std::vector<G4int> getSensitiveLayerPatternVector(G4String sensitiveLayerPattern);

};

#endif


