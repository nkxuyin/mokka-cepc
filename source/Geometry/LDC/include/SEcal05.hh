//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for ILC   -      *
//*                                                     *
//* For more information about Mokka, visit the         *
//*                                                     *
//*  Mokka.in2p3.fr  Mokka home page.                   *
//*                                                     *
//*******************************************************
/*
  March 2013: add posibility of having different cell sizes per layer, in case of
  using scintillator as active material (Angela Lucaci)

  Updated version 05 : Daniel Jeans
  allows layer-by-layer configuration si-scint
  scalable endcap geometry
*/

#ifndef SEcal05_h
#define SEcal05_h 1

#include "VSubDetectorDriver.hh"
#include "G4LogicalVolume.hh"
#include "G4Tubs.hh"
#include "SEcalSD02.hh"
#include "SEcalSD04.hh"
#include "SEcalSDRing02.hh"


class SEcal05 : public VSubDetectorDriver
{
 public:
  SEcal05() : VSubDetectorDriver("SEcal05", "ecal"), 
	      theBarrelSiliconSD(0),
	      theEndCapSiliconSD(0),
	      theBarrelScintillatorSD(0),
	      theEndCapScintillatorSD(0),
	      theEndCapRingSD(0),
	      theMaxStepAllowed(DBL_MAX)
  {}
  
  ~SEcal05();

  G4bool ContextualConstruct(const CGAGeometryEnvironment &, G4LogicalVolume *);

  G4bool PostConstructAction(CGAGeometryEnvironment &);

  /* To reload hits for visualisation*/
  void LoadEvent(FILE *theSubDetectorEventHitsFileInput);
  
private:

  /* Main and calculated parameters*/
  G4double Ecal_Alveolus_Air_Gap, Ecal_Slab_shielding,
    Ecal_fiber_thickness, Ecal_Si_thickness,
    Si_Slab_Y_offset, Sc_Slab_Y_offset,
    Ecal_Slab_copper_thickness, Ecal_Slab_PCB_thickness,
    Ecal_Slab_glue_gap, Ecal_Slab_ground_thickness,
    Ecal_total_SiSlab_thickness, Ecal_inner_radius,
    Ecal_guard_ring_size;
  G4double Ecal_cells_size, Ecal_cables_gap;
  G4double Ecal_endcap_center_box_size, Ecal_Lcal_ring_gap,
    TUBE_crossing_angle,
    Lcal_outer_radius, Ecal_endcap_rmax,Ecal_endcap_extra_size,
    Ecal_support_thickness, Ecal_front_face_thickness,
    Ecal_lateral_face_thickness, Ecal_Slab_H_fiber_thickness;
 
  G4String Ecal_radiator_material;

  G4int Ecal_barrel_number_of_towers;

  G4int Ecal_nlayers1, Ecal_nlayers2, Ecal_nlayers3;
  G4int total_number_of_layers;

  G4double Ecal_radiator_thickness1,Ecal_radiator_thickness2,
    Ecal_radiator_thickness3;

  G4double Ecal_Barrel_halfZ,Ecal_Barrel_module_dim_z,
    Ecal_cell_size;
  
  G4double module_thickness, endcap_module_dim_x,
    alveolus_dim_z, bottom_dim_x,
    top_dim_x;
  G4double cell_dim_x, cell_dim_z;
  G4int N_cells_in_X, N_cells_in_Z;

  G4double EC_cell_dim_x, EC_cell_dim_z;
  G4int EC_N_cells_in_X, EC_N_cells_in_Z;


  /* Setup*/
  G4bool Setup(const CGAGeometryEnvironment &);
  G4bool EC_Initialize(std::vector<G4LogicalVolume*>&, char, G4int layer = 1);
  G4bool Build(G4LogicalVolume *);
  
  /* Barrel*/
  void BarrelStandardModule(G4LogicalVolume*);
  G4double BuildBarrelAlveolus(G4int,G4double,G4LogicalVolume*);
  G4double BuildBarrelStructureLayer(G4int,G4double,G4LogicalVolume*);
  G4double GiveMeRadiatorThickness(G4int);

  G4LogicalVolume *BuildRadiatorPlate(G4double, G4double, G4double);
  G4LogicalVolume *BuildSiliconSlab(G4double, G4double, G4double, SEcalSD02*);  
  G4LogicalVolume *BuildScintillatorSlab(G4double, G4double, G4double, char, SEcalSD04*, G4double, G4double);
  G4LogicalVolume *BuildSensitiveVolume(G4double, G4double, char, SEcalSD04*, G4double, G4double);

  G4LogicalVolume *BuildStripVolume(G4double, G4double, G4double, SEcalSD04*);

  /* EndCaps*/
  G4LogicalVolume *EndcapStandardModule(G4bool Zminus = false);
  
  void EndcapRadiatorPlates(G4LogicalVolume *&, G4LogicalVolume *&, G4LogicalVolume *&,
			    G4LogicalVolume *&, G4LogicalVolume *&, G4LogicalVolume *&);
  
  G4double BuildECAlveolus (G4int, G4double, G4LogicalVolume *, G4bool Zminus = false);

  void BuildECRingAlveolus (G4int, G4double, G4LogicalVolume *, G4bool Zminus = false);

  void DefineMaterial(void);

  /*Angela Lucaci: scintillator cell sizes varying with layer*/
  bool useScintillatorTilesOfVaryingSize;

  typedef std::vector<double> DoubleVector;
  typedef std::vector<int> IntVector;
  std::string inputEcal_Sc_cellDim1_string, inputEcal_Sc_cellDim2_string;
  DoubleVector  inputEcal_Sc_cellDim1_vector, inputEcal_Sc_cellDim2_vector;

  DoubleVector  barrelEcal_Sc_cellDim1_vector, barrelEcal_Sc_cellDim2_vector;
  IntVector     barrelEcal_Sc_N_strips_across_module_vector, barrelEcal_Sc_N_strip_containers_along_z_vector;
  DoubleVector  endcapEcal_Sc_cellDim1_vector, endcapEcal_Sc_cellDim2_vector;
  IntVector     endcapEcal_Sc_N_strips_across_module_vector, endcapEcal_Sc_N_strip_containers_along_z_vector;

  /**
   *    @brief  Read scintillator cell configuration string and populate vectors of requested cell dimensions
   * 
   *    @param  Ecal_Sc_cellDim_string the configuration string
   *    @param  Ecal_Sc_cellDim_vector to receive the layer-by-layer cell dimensions
   */
  void FillCellDimVector(const std::string &Ecal_Sc_cellDim_string, DoubleVector &Ecal_Sc_cellDim_vector);

  std::vector<G4LogicalVolume*> EC_ScintillatorParallelToZTowerSlabs_first;
  std::vector<G4LogicalVolume*> EC_ScintillatorParallelToXTowerSlabs_first;
  std::vector<G4LogicalVolume*> EC_ScintillatorParallelToZTowerSlabs_second;
  std::vector<G4LogicalVolume*> EC_ScintillatorParallelToXTowerSlabs_second;

  G4Material *RadiatorMaterial;
  G4Material *siPCBMix;
  G4Material *scPCBMix;
  G4Material *groundMix;

  G4LogicalVolume *EnvLogEcalModuleBarrel;
  G4LogicalVolume *EnvLogEcalModuleEndCap;

  SEcalSD02 *theBarrelSiliconSD;
  SEcalSD02 *theEndCapSiliconSD;
  SEcalSD04 *theBarrelScintillatorSD;
  SEcalSD04 *theEndCapScintillatorSD;

  SEcalSDRing02 *theEndCapRingSD;

  G4double theMaxStepAllowed;
  G4double EC_theMaxStepAllowed;
  /* Central box become a Tub...*/
  G4Tubs *CenterECTub;
  G4TranslateX3D *FollowLcal, *FollowLcalZminus;
  
  G4double Ecal_EC_Ring_gap,ECRingSiplateSize;

  G4Box *ECRingSiBox;
  G4LogicalVolume *ECRingSiLog, *ECRingSiLogZminus;

  G4LogicalVolume *ECRingRadiatorL1;
  G4LogicalVolume *ECRingRadiatorL2;
  G4LogicalVolume *ECRingRadiatorL3;

  G4LogicalVolume* EndCapRingSlabRadiatorL1;
  G4LogicalVolume* EndCapRingSlabRadiatorL2;
  G4LogicalVolume* EndCapRingSlabRadiatorL3;


  std::vector<G4LogicalVolume*> EC_SiliconTowerSlabs;
  std::vector<G4LogicalVolume*> EC_ScintillatorParallelToZTowerSlabs;
  std::vector<G4LogicalVolume*> EC_ScintillatorParallelToXTowerSlabs;
  std::vector<G4ThreeVector> EC_TowerXYCenters;
  std::vector<G4LogicalVolume*> EC_TowerR1;
  std::vector<G4LogicalVolume*> EC_TowerR2;
  std::vector<G4LogicalVolume*> EC_TowerR3;

  G4double EC_y_botton;
  G4double EC_y_top;
  G4double EC_alveolus_dim_y;
  G4double EC_alveolus_dim_x;

  /* Daniel Jeans*/
  G4double EC_alveolus_width;
  G4int EC_n_alveolus;

  G4double EC_module_z_offset;
  G4double fiber_inter_alveolus;

#ifdef MOKKA_GEAR
  void getCellDims(G4double &, G4double &, int, char, bool isBarrel = true); 

  struct helpParameters
  {
    G4double innerRadius;
    G4double outerRadius;
    G4double zMax;
    G4double phi0;
    std::vector<double> layerPos;
    std::vector<double> radiThickness;
    G4int count;
    G4double leastZ;
    G4double mostZ;
  };
  
  helpParameters helpBarrel;
  helpParameters helpEndcap;
  helpParameters helpPlug;
#endif

  G4bool Ecal_Scale_Endcap;

  
  G4double Ecal_Slab_Sc_PCB_thickness;
  G4double Ecal_Sc_thickness;
  G4double Ecal_Sc_reflector_thickness;
  G4double Ecal_MPPC_size;
  G4double Ecal_Sc_MPPC_breadth;
  G4int    Ecal_Sc_N_strips_across_module;
  G4int    Ecal_Sc_number_of_virtual_cells;
  G4String Ecal_Barrel_Sc_Si_Mix;
  G4String Ecal_EndCap_Sc_Si_Mix;

  G4double Ecal_total_ScSlab_thickness;

  G4int Number_of_Si_Layers_in_Barrel;
  G4int Number_of_Sc_Layers_in_Barrel;

  G4int Number_of_Si_Layers_in_EC;
  G4int Number_of_Sc_Layers_in_EC;

  G4double barrelStripSizeinX;
  G4double barrelStripSizeParallelToZ;

  G4double EC_StripSizeinX;
  G4double EC_StripSizeParallelToZ;
  G4double EC_virtualCellDim;

  G4bool EC_Scint_Along_X;
  G4bool EC_Scint_Along_Z;

  G4double virtualCellDim;
};

#endif


