//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for Tesla -      *
//*                                                     *
//* For more information about Mokka, please, go to the *
//*                                                     *
//*  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
//*                                                     *
//* Mokka home page.                                    *
//*                                                     *
//*******************************************************
//
#ifndef SHcalSc01_h
#define SHcalSc01_h 1

#include "VSubDetectorDriver.hh"
#include "SDHcalBarrel.hh"
#include "SDHcalEndCap.hh"

/** @class SHcalSc01 HCAL superdriver, for the scintillator option, with more realistic description, 
  compared to SHcal03
*/
class SHcalSc01 : public VSubDetectorDriver
{
public:

  SHcalSc01() : VSubDetectorDriver("SHcalSc01","hcal"),
		  theBarrilRegSD(0),
		  theENDCAPEndSD(0),
		  theMaxStepAllowed(DBL_MAX)
  {}
  ~SHcalSc01();

  G4bool ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
			       G4LogicalVolume *theWorld);

  G4bool PostConstructAction(CGAGeometryEnvironment& );


private:
  G4LogicalVolume *EnvLogHcalModuleBarrel;
  G4LogicalVolume *EnvLogHcalModuleEndCap;
  G4String SensitiveModel;
  Database* db;

  // Setup run time parameters
  G4bool Setup(const CGAGeometryEnvironment &);

  // To reload hits for visualisation
  void LoadEvent(FILE* theSubDetectorEventHitsFileInput);

  // Barrel
  void Barrel(G4LogicalVolume*);
  void BarrelRegularModules(G4LogicalVolume*);
  void BarrelRegularChambers(G4LogicalVolume*, G4double chambers_y_off_correction);

  /** Calculate the size of the fractional tile; with this algorithm, the fractional
	tile size is always >= integer tile size/2 and < integer tile size
	@param x_length total x-dimension of the scintillator
	@param x_integerTileSize x-dimension of the integer tile
	@param x_fractionalTileSize x-dimension of the fractional tile
	*/
  void CalculateFractTileSize(G4double x_length, G4double x_integerTileSize, G4double &x_fractionalTileSize);

  /** Build a box filled with air in the middle of the HCAL module barrel,
	to simulate the gap between the 2 real halves of the module
	@param MotherLog logical mother volume (i.e. logical barrel module)
	*/
  void BarrelModuleGap(G4LogicalVolume *MotherLog);

  /** Build the partial HCAL barrel chambers 
	@param MotherLog logical mother volume (i.e. logical barrel volume)
	@param chambers_y_off_correction correction on the y-axis (the barrel is build 
	from bottom and top barrel, but Geant4 keeps the axis of coordinate centered 
	in the bottom barrel, such that we need to shift the layers with
	chambers_y_off_correction = Hcal_y_dim2_for_x / 2. along the y-axis)
	*/  
  void BarrelHalfRegularChambersTrap(G4LogicalVolume *MotherLog, 
				       G4double chambers_y_off_correction);

  /** Build the chambers support (this is a right angular wedge, apart from some special cases,
	when the layer is both in the bottom and top barrel, when just a box is drawn for the support,
	since otherwise there are volume overlaps)
	@param MotherLog logical mother volume (i.e. logical barrel volume)
	@param chambers_y_off_correction correction on the y-axis (the barrel is build 
	from bottom and top barrel, but Geant4 keeps the axis of coordinate centered 
	in the bottom barrel, such that we need to shift the layers with
	chambers_y_off_correction = Hcal_y_dim2_for_x / 2. along the y-axis)
	*/
  void BarrelChambersSupportTrap(G4LogicalVolume *MotherLog,
				   G4double chambers_y_off_correction);

 /** Build the chambers air gap
	@param MotherLog logical mother volume (i.e. barrel chambers)
	@param x_chamber x-dimension of the barrel chamber 
	@param y_chamber y-dimension of the barrel chamber 
	@param z_chamber z-dimension of the barrel chamber 
	*/
  void BarrelChambersGap(G4LogicalVolume *MotherLog,
			   G4double chambers_y_off_correction);

  /** Calculate the x-dimension of the active layer
	@param layer_id id of the layer (>0 and <=2*Hcal_nlayers)
	@param logical_layer_id logical id of the layer (>0 and <= Hcal_nlayers)
	@xOffset offset depending on the symmetry angle (pi/8), to be set
	@x_halfLengt half length in the x-direction of the HCAL layer (to be set)
	@xShift shift from the origin of the local coordinate system (to be set)
	*/
  void CalculateXLayer(G4int layer_id, G4int &logical_layer_id,
			 G4double &xOffset, G4double &x_halfLength, G4double &xShift);
  // EndCaps
  void Endcaps(G4LogicalVolume*);
  void EndcapRings(G4LogicalVolume*);
  void EndcapChambers(G4LogicalVolume*, SDHcalEndCap*, bool);


  SDHcalBarrel *theBarrilRegSD;//HCAL barrel sensitive region
  SDHcalEndCap *theENDCAPEndSD;
  SDHcalEndCap *theENDCAPRingSD;

  G4Material * RadiatorMaterial;

  G4double theMaxStepAllowed;

  // Run time parameters
  G4double Hcal_radiator_thickness;
  G4String Hcal_radiator_material;
  G4int    Hcal_ring;
  G4double Hcal_radial_ring_inner_gap;
  G4String Hcal_sensitive_model;
  G4double Hcal_back_plate_thickness;
  G4double Hcal_stave_gaps;
  G4double Hcal_modules_gap;
  G4int    Hcal_nlayers;
  G4int    Hcal_barrel_end_module_type;
  G4double Hcal_fiber_gap;
  G4double Hcal_chamber_tickness;
  G4double Hcal_inner_radius;
  G4double Hcal_endcap_cables_gap, Hcal_endcap_ecal_gap;
  G4double Hcal_endcap_rmax;
  G4double TPC_Ecal_Hcal_barrel_halfZ;
  G4double Hcal_normal_dim_z;
  G4double Hcal_top_end_dim_z;
  G4double Hcal_start_z;
  G4double Ecal_endcap_outer_radius;
  G4double Ecal_endcap_zmin, Ecal_endcap_zmax;
  G4double Hcal_lateral_plate_thickness;
  G4double Hcal_cells_size;
  G4double Hcal_digi_cells_size;
  G4double Hcal_endcap_center_box_size;
  G4double Hcal_endcap_sensitive_center_box;
  G4double Hcal_total_dim_y;
  G4double Hcal_module_radius;
  G4double Hcal_y_dim2_for_x;
  G4double Hcal_y_dim1_for_x;
  G4double Hcal_bottom_dim_x;
  G4double Hcal_midle_dim_x;
  G4double Hcal_top_dim_x;
  G4double Hcal_y_dim1_for_z;
  G4double Hcal_y_dim2_for_z;
  G4double Hcal_y_dim3_for_z;
  G4double Hcal_regular_chamber_dim_z;
  G4double Hcal_cell_dim_x;
  G4double Hcal_cell_dim_z;
  G4double Hcal_digi_cell_dim_x;
  G4double Hcal_digi_cell_dim_z;  

  //G4double Hcal_sectors_gap;
  G4double Hcal_layer_support_length;
  G4double Hcal_layer_air_gap;
  G4double Hcal_chamber_thickness;
  G4double Hcal_middle_stave_gaps;
  G4bool   Hcal_apply_Birks_law;

#ifdef MOKKA_GEAR
  // MokkaGear
  struct helpParameters{
	G4double innerRadius ;
	G4double outerRadius ;
	G4double zMax ;
	G4double phi0 ;
	std::vector<double> layerPos ;
	std::vector<double> layerPos2 ;
	std::vector<double> sensThickness ;
	std::vector<double> gapThickness ;
// 	std::vector<double> fractCellSize1;//x-dimension of the fractional cell
	G4int count ;
	G4double leastZ ;
	G4double mostZ ;
  };

  helpParameters helpBarrel;
  helpParameters helpEndcap;
  helpParameters helpEndcapRing;

  // parameters
  G4double dblParamModulesGap;
  G4double dblParamFiberGap;
  G4double dblParamLayerSupportLength;
  G4double dblParamLayerAirGap;
  G4double dblParamMiddleStaveGaps;
  G4double dblParamBackPlateThickness;
  G4double dblParamBarrelMostZ;
  G4double dblParamHcalModulesGap;
  G4double dblParamHcalStaveGaps;
  G4double dblParamTPCEcalHcalbarrelHalfZ;
  G4double dblParamHcalLateralStructureThickness;
  G4double dblParamHcalVirtualCellSize;
  G4int    dblHcalBarrelEndModuleType; 
  G4double dblParamHcalEndcapSensitiveCenterBox;
#endif
};

#endif
