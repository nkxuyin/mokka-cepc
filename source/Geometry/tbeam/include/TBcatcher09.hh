#ifndef TBcatcher09_h
#define TBcatcher09_h 1

#include "Control.hh"
#include "VSubDetectorDriver.hh"
#include "CGAGeometryEnvironment.hh"
#include "TBSD_VCell04.hh"

#include "G4Material.hh"
#include "G4LogicalVolume.hh"
#include "G4Transform3D.hh"

class Database;

class TBcatcher09 : public VSubDetectorDriver
{

public:

  TBcatcher09();
  ~TBcatcher09();

  /*Use contruction method via environment object as available since Mokka release 4.0
    to access setup parameters*/
  G4bool ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
			     G4LogicalVolume *theWorld);

  G4bool PostConstructAction(CGAGeometryEnvironment& );

  G4double GetGridSize() { return grid_size; }
  G4int GetDepthToLayer() { return depthToLayer; }

private:

  void SetSD();

  void DefineMaterials();

  void FetchAll();
  void Print();

  G4bool BuildCatcher();
  G4LogicalVolume* BuildCoarseLayerLogical(bool withCassette, G4ThreeVector& offset, G4int layerNumber);
  G4LogicalVolume* BuildFineLayerLogical(bool withCassette, G4ThreeVector& offset, G4int layerNumber);
  G4LogicalVolume* BuildReadoutModuleLogical();
  void BuildCatcherLayers();
  G4LogicalVolume* BuildCatcherEnvelope();
  void PlaceCatcher();

  void ComputeCatcherTransform();

  //Set the level on which the layer is implemented in G4
  void SetDepthToLayer(G4int i); 

private:

  Database *db;
  TBSD_VCell04 *catcherSD;
  TBSD_VCell04 *catcherSD_steelFront;
  TBSD_VCell04 *catcherSD_steelBack;
  TBSD_VCell04 *catcherSD_steelAbsorberFine;
  TBSD_VCell04 *catcherSD_steelAbsorberCoarse;

  /*The Environment object*/
  CGAGeometryEnvironment _aGeometryEnvironment;

  G4bool checkForOverlappingVolumes;/**< flag to check for overlapping volumes (for debug purposes)*/

  G4int n_fine_layers;
  G4int n_coarse_layers;

  /*Variables containing info on detector dimensions*/
  G4int  n_layers, ncell_xy[2];             /**< n layers, number of cells in xy-direction*/
  G4double grid_size;                       /**< Parameter determining the cellwise subdivision of a layer */
  G4double xlen_tcmt, ylen_tcmt, zlen_tcmt; /**< Outer dimensions of the detector*/
  G4double z_begin, z_place;                /**<Coordinates where the detector begins and where it is to be put*/
  G4double dist_hcal_tcmt; 

  /*configuration angle of detector*/
  G4double config_angle;

  /* layer pattern*/
  G4String _layerPattern;

  /*depth of where the layer is implemented within the G4 volumes hierarchy*/
  G4int depthToLayer; 

  /* transform of Catcher*/
  G4Transform3D* transformCatcher;
  G4ThreeVector translateCatcher;

  /* Adjustment for catcher alignment*/
  G4double rotationAdjust;
  G4ThreeVector translationAdjust;

  /* sub-layer thicknesses*/
  G4double steel_front_hthickness,
    poly_front_hthickness,
    tyvek_front_hthickness,
    poly_active_hthickness,
    tyvek_back_hthickness,
    poly_back_hthickness,
    steel_back_hthickness,
    steel_absorber_fine_hthickness,
    steel_absorber_coarse_hthickness;

  const static int TCMT_NLAYERS = 16;
  G4double air_front_hthickness[TCMT_NLAYERS];
  G4double air_back_hthickness[TCMT_NLAYERS];
  G4double fine_layer_hthickness[TCMT_NLAYERS/2];
  G4double coarse_layer_hthickness[TCMT_NLAYERS/2];

  /* computed thicknesses*/
  G4double readout_module_hthickness;

  /* "pv_" for PV name*/
  G4String pre_name;

  /* world and detector logical volumes*/
  G4LogicalVolume *WorldLogical, *DetectorLogical;

  /* active module sublayer*/
  G4LogicalVolume *ReadoutModuleLogical;

  /* sublayers in readout module*/
  G4LogicalVolume *SteelFrontLogical,
    *PolyFrontLogical,
    *TyvekFrontLogical,
    *PolyActiveLogical,
    *TyvekBackLogical,
    *PolyBackLogical,
    *SteelBackLogical;

  /* absorbers*/
  G4LogicalVolume *FineAbsorberLogical,
    *CoarseAbsorberLogical;

  /* full layers, fine and coarse types*/
  G4LogicalVolume *FineLayerLogical, *CoarseLayerLogical;
  G4LogicalVolume *FineLayerLogicalWithoutCassette,
    *CoarseLayerLogicalWithoutCassette;

  /* material pointers*/
  G4Material *steel, *poly, *tyvek, *air;

  /*HCAL*/
  G4int ncell_xy_hcal[2];
  G4double grid_size_hcal;
  G4double xlen_hcal, ylen_hcal, zlen_hcal;
  G4int n_layers_hcal;
  G4double z_begin_hcal;
  G4double poly_hthickness_hcal, absorber_hthickness_hcal, layer_hthickness_hcal; 
  G4double airgap_hthickness_hcal, steel_cassette_hthickness_hcal; 
  G4double foil_hthickness_hcal, pcb_hthickness_hcal, cablefibre_mix_hthickness_hcal;
  G4double steel_support_hthickness_hcal;
  /*Added parameters for terminating absorber of Hcal*/
  G4int n_layer_term_hcal; 
  G4double absorber_hthickness_term_hcal;
  
  /*GM: the following is to be used by muon counter*/
  G4double z_end_tcmt;
};

#endif
