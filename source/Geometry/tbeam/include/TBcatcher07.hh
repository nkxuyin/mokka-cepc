// $Header: /home/flc/cvs/Mokka/source/Geometry/tbeam/include/TBcatcher07.hh,v 1.3 2008/01/22 14:48:12 musat Exp $

#ifndef TBcatcher07_h
#define TBcatcher07_h 1

#include "Control.hh"
#include "VSubDetectorDriver.hh"

#include "TBSD_VCell03.hh"

#include "G4Material.hh"
#include "G4VPhysicalVolume.hh"
#include "G4Transform3D.hh"

#include "CGAGeometryEnvironment.hh"

class G4LogicalVolume;
class Database;

class TBcatcher07 : public VSubDetectorDriver
{

public:

  TBcatcher07();
  ~TBcatcher07();

  //Use contruction method via environment object as available since Mokka release 4.0
  //to access setup parameters
  G4bool ContextualConstruct(const
    CGAGeometryEnvironment &aGeometryEnvironment,
    G4LogicalVolume *theWorld);

  G4bool PostConstructAction(CGAGeometryEnvironment& );

  G4double GetGridSize() { return grid_size; }
  G4int GetDepthToLayer() { return depthToLayer; }

private:

  void SetSD();

  void DefineMaterials();

  void FetchAll();
  void Print();

  // build subfunctions
  G4bool BuildCatcher();
  G4LogicalVolume* BuildCoarseLayerLogical(bool withCassette, G4ThreeVector& offset);
  G4LogicalVolume* BuildFineLayerLogical(bool withCassette, G4ThreeVector& offset);
  G4LogicalVolume* BuildReadoutModuleLogical();
  void BuildCatcherLayers();
  G4LogicalVolume* BuildCatcherEnvelope();
  void PlaceCatcher();

  void ComputeCalXY();
  void ComputeCatcherTransform();

  //Set the level on which the layer is implemented in G4
  void SetDepthToLayer(G4int i); 

private:

  Database *db;
  TBSD_VCell03 *catcherSD;

  //The Environment object
  CGAGeometryEnvironment _aGeometryEnvironment;


  G4int n_fine_layers;
  G4int n_coarse_layers;

  //Variables containing info on detector dimensions
  G4int  n_layers, ncell_xy[2]; // n layers, number of cells in xy-direction
  G4double grid_size; // Parameter determining the cellwise subdivision of a layer
  G4double xlen_tcmt, ylen_tcmt, zlen_tcmt; // Outer dimensions of the detector
  G4double z_begin, z_place; // Coordinates where the detector begins and where it is to be put
  G4double dist_hcal_tcmt; // no comments...

  //configuration angle of detector
  G4double config_angle;

  // layer pattern
  G4String _layerPattern;

  //depth of where the layer is implemented within the G4 volumes hierarchy
  G4int depthToLayer; 

  // transform of Catcher
  G4Transform3D* transformCatcher;
  G4ThreeVector translateCatcher;

  // Adjustment for catcher alignment
  G4double rotationAdjust;
  G4ThreeVector translationAdjust;

  // sublayer thicknesses
  G4double steel_front_hthickness,
    poly_front_hthickness,
    tyvek_front_hthickness,
    poly_active_hthickness,
    tyvek_back_hthickness,
    poly_back_hthickness,
    steel_back_hthickness,
    air_front_hthickness,
    air_back_hthickness,
    steel_absorber_fine_hthickness,
    steel_absorber_coarse_hthickness;

  // computed thicknesses
  G4double readout_module_hthickness,
    fine_layer_hthickness,
    coarse_layer_hthickness;

  // "pv_" for PV name
  G4String pre_name;

  // world and detector logical volumes
  G4LogicalVolume *WorldLogical, *DetectorLogical;

  // active module sublayer
  G4LogicalVolume *ReadoutModuleLogical;

  // sublayers in readout module
  G4LogicalVolume *SteelFrontLogical,
    *PolyFrontLogical,
    *TyvekFrontLogical,
    *PolyActiveLogical,
    *TyvekBackLogical,
    *PolyBackLogical,
    *SteelBackLogical;

  // absorbers
  G4LogicalVolume *FineAbsorberLogical,
    *CoarseAbsorberLogical;

  // full layers, fine and coarse types
  G4LogicalVolume *FineLayerLogical, *CoarseLayerLogical;
  G4LogicalVolume *FineLayerLogicalWithoutCassette,
    *CoarseLayerLogicalWithoutCassette;

  // material pointers
  G4Material *steel, *poly, *tyvek, *air;

  // TODO: compute this statically
  //necessary parameters of Hcal
  G4int ncell_xy_hcal[2];
  G4double grid_size_hcal;
  G4double xlen_hcal, ylen_hcal, zlen_hcal;
  G4int n_layers_hcal;
  G4double z_begin_hcal;
  G4double poly_hthickness_hcal, steel_hthickness_hcal, layer_hthickness_hcal; 
  G4double airgap_hthickness_hcal, steel_cassette_hthickness_hcal; 
  G4double foil_hthickness_hcal, pcb_hthickness_hcal, cablefibre_mix_hthickness_hcal;
  //Added parameters for terminating absorber of Hcal
  G4int n_layer_term_hcal; 
  G4double steel_hthickness_term_hcal;
  
  //GM: the following is to be used by muon counter
  G4double z_end_tcmt;
};

#endif
