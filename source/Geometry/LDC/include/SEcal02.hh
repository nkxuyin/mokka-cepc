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
//
// $Id: SEcal02.hh,v 1.6 2007/12/20 17:28:27 mora Exp $
// $Name: mokka-07-00 $
//
#ifndef SEcal02_h
#define SEcal02_h 1

class G4LogicalVolume;
class G4VSolid;
//class G4Box;
class G4Tubs;
class SEcalSD02;
class SEcalECSD02;
class SEcalSDRing02;		     

#include "VSubDetectorDriver.hh"

class SEcal02 : public VSubDetectorDriver
{
 public:
  SEcal02() : VSubDetectorDriver("SEcal02","ecal"), 
	      theBarrelSD(0),theEndCapSD(0),theEndCapRingSD(0),
	      theMaxStepAllowed(DBL_MAX)
  {}
  
  ~SEcal02();

  G4bool 
  ContextualConstruct(const CGAGeometryEnvironment&,
		      G4LogicalVolume*);

  G4bool 
  PostConstructAction(CGAGeometryEnvironment& );

  // To reload hits for visualisation
  void LoadEvent(FILE* theSubDetectorEventHitsFileInput);
  
private:

  // Main and calculated parameters
  G4double Ecal_Alveolus_Air_Gap,Ecal_Slab_shielding,
    Ecal_fiber_thickness,Ecal_Si_thickness,
    Si_Slab_Y_offset,
    Ecal_Slab_copper_thickness,Ecal_Slab_PCB_thickness,
    Ecal_Slab_glue_gap,Ecal_Slab_ground_thickness,
    Ecal_total_Slab_thickness, Ecal_inner_radius,
    Ecal_guard_ring_size;
  G4double Ecal_cells_size,Ecal_cables_gap;
  G4double Ecal_endcap_center_box_size, Ecal_Lcal_ring_gap,
    TUBE_crossing_angle,
    Lcal_outer_radius, Ecal_endcap_rmax,Ecal_endcap_extra_size,
    Ecal_support_thickness,Ecal_front_face_thickness,
    Ecal_lateral_face_thickness,Ecal_Slab_H_fiber_thickness;
 
  G4String Ecal_radiator_material;

  G4int Ecal_barrel_number_of_towers;

  G4int Ecal_nlayers1, Ecal_nlayers2, Ecal_nlayers3;
  G4int total_number_of_layers;

  G4double Ecal_radiator_thickness1,Ecal_radiator_thickness2,
    Ecal_radiator_thickness3;

  G4double Ecal_Barrel_halfZ,Ecal_Barrel_module_dim_z,
    Ecal_cell_size;
  
  G4double module_thickness, endcap_module_dim_x,
    alveolus_dim_z,bottom_dim_x,
    top_dim_x;
  G4double cell_dim_x,cell_dim_z;
  G4int N_cells_in_X,N_cells_in_Z;

  // Setup
  G4bool Setup(const CGAGeometryEnvironment &);
  G4bool  EC_Initialize();
  G4bool Build(G4LogicalVolume*);
  
  // Barrel
  void BarrelStandardModule(G4LogicalVolume*);
  G4double BuildBarrelAlveolus(G4int,G4double,G4LogicalVolume*);
  G4double BuildBarrelStructureLayer(G4int,G4double,G4LogicalVolume*);
  G4LogicalVolume* BuildRadiatorPlate(G4double,G4double,G4double);
  G4double GiveMeRadiatorThickness(G4int);
  G4LogicalVolume* BuildSlab(G4double,
			     G4double,
			     G4double,
			     SEcalSD02*);
  
  // EndCaps
  G4LogicalVolume* EndcapStandardModule(G4bool Zminus=false);
  
  void EndcapRadiatorPlates(G4LogicalVolume*&,
			    G4LogicalVolume*&,
			    G4LogicalVolume*&,
			    G4LogicalVolume*&,
			    G4LogicalVolume*&,
			    G4LogicalVolume*&);
  
  G4double BuildECAlveolus (G4int,
			    G4double,
			    G4LogicalVolume*,
			    G4bool Zminus=false);

  void BuildECRingAlveolus (G4int,
			    G4double,
			    G4LogicalVolume*,
			    G4bool Zminus=false);

  G4Material * RadiatorMaterial;

  G4LogicalVolume *EnvLogEcalModuleBarrel;
  G4LogicalVolume *EnvLogEcalModuleEndCap;
  SEcalSD02* theBarrelSD;
  SEcalSD02* theEndCapSD;
  SEcalSDRing02* theEndCapRingSD;

  G4double theMaxStepAllowed;
  // Central box become a Tub...
  G4Tubs *CenterECTub;
  G4TranslateX3D* FollowLcal, *FollowLcalZminus;

  G4Box *ECRingSiBox;
  G4LogicalVolume *ECRingSiLog, *ECRingSiLogZminus;

  G4LogicalVolume *ECRingRadiatorL1;
  G4LogicalVolume *ECRingRadiatorL2;
  G4LogicalVolume *ECRingRadiatorL3;

  G4LogicalVolume* EndCapRingSlabRadiatorL1;
  G4LogicalVolume* EndCapRingSlabRadiatorL2;
  G4LogicalVolume* EndCapRingSlabRadiatorL3;


  std::vector<G4LogicalVolume*> EC_TowerSlabs;
  std::vector<G4ThreeVector> EC_TowerXYCenters;
  std::vector<G4LogicalVolume*> EC_TowerR1;
  std::vector<G4LogicalVolume*> EC_TowerR2;
  std::vector<G4LogicalVolume*> EC_TowerR3;
  G4double EC_y_botton;
  G4double EC_alveolus_dim_y;
  G4double EC_alveolus_dim_x;
  G4double EC_module_z_offset;
  G4double fiber_inter_alveolus;

#ifdef MOKKA_GEAR
  // MokkaGear
  
  struct helpParameters{
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
  
};

#endif


