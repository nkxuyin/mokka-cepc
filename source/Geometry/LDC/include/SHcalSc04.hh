/*******************************************************/
/*                                                     */
/*                      Mokka                          */ 
/*   - the detailed geant4 simulation for ILC -        */
/*					               */  
/* For more information about Mokka, please, go to the */
/* Mokka home page:                                    */
/*                                                     */
/* http://polzope.in2p3.fr:8081/MOKKA                  */
/*                                                     */
/*******************************************************/

#ifndef SHcalSc04_h
#define SHcalSc04_h 1

#include "VSubDetectorDriver.hh"
#include "SDHcalBarrel.hh"
#include "SDHcalEndCap.hh"
#include "SDAHcalEndCapScalable.hh"
    
/** @class SHcalSc04 SHcalSc04.hh "SHcalSc04.hh"
    \brief HCAL superdriver

    HCAL superdriver for the scintillator option,
    with more realistic description,
    and flexible scalable deminesions.
*/
class SHcalSc04 : public VSubDetectorDriver
{
public:
  
  /**Constructor
   */
  SHcalSc04() : VSubDetectorDriver("SHcalSc04","hcal"),
		theBarrilRegSD(0),
		theENDCAPEndSD(0),
		theMaxStepAllowed(DBL_MAX)
  {}

  /**Destructor
   */
  ~SHcalSc04();

  /**Function requested by Mokka, common to all drivers
     @param aGeometryEnvironment the geometry environment
     @param theWorld logical mother volume
     @return true if detector construction was successful 
  */
  G4bool ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
			     G4LogicalVolume *theWorld);

  /* Propagates the changes to the coil, if any. 
   */
  G4bool PostConstructAction(CGAGeometryEnvironment& );


private:
  static const double eps; /**<setup a 1*10E-6 mm space to seperate touch surface*/ 
  G4LogicalVolume *EnvLogHcalModuleBarrel;/**<logical volume of the HCAL barrel*/
  G4LogicalVolume *EnvLogHcalModuleEndCap;/**<logical volume of the HCAL endcap*/
  G4String SensitiveModel;                /**<the sensitive model: 'RPC1' or 'Scintillator'*/
  Database* db;                           /**<pointer to the data base*/
  
  /** Setup of the run time parameters
      @param aGeometryEnvironment the geometry environment
      @return true if setting pf the parameters was succesfull
  */
  G4bool Setup(const CGAGeometryEnvironment &aGeometryEnvironment);

  /** To reload hits for visualisation
      @param theSubDetectorEventHitsFileInput file with subdetector hits
  */
  void LoadEvent(FILE* theSubDetectorEventHitsFileInput);

  /**Build the HCAL barrel regular modules 
     @param MotherLog mother logical volume (world)
  */
  void BarrelRegularModules(G4LogicalVolume* MotherLog);
  

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
  //void BarrelModuleGap(G4LogicalVolume *MotherLog);

  /** Build the partial HCAL barrel chambers 
      @param MotherLog logical mother volume (i.e. logical barrel volume)
      @param chambers_y_off_correction correction on the y-axis (the barrel is build 
      from bottom and top barrel, but Geant4 keeps the axis of coordinate centered 
      in the bottom barrel, such that we need to shift the layers with
      chambers_y_off_correction = Hcal_y_dim2_for_x / 2. along the y-axis)
  */  
  void BarrelHalfRegularChambersTrap(G4LogicalVolume *MotherLog, 
				     G4double chambers_y_off_correction);

  /** Build the chambers air gap
      @param MotherLog logical mother volume (i.e. barrel chambers)
      @param chambers_y_off_correction correction on the y-axis (the barrel is build 
      from bottom and top barrel, but Geant4 keeps the axis of coordinate centered 
      in the bottom barrel, such that we need to shift the layers with
      chambers_y_off_correction = Hcal_y_dim2_for_x / 2. along the y-axis)
  */
  void BarrelChambersGap(G4LogicalVolume *MotherLog,
			 G4double chambers_y_off_correction);

  /** Calculate the x-dimension of the active layer
      @param layer_id id of the layer (>0 and <=2*Hcal_nlayers)
      @param logical_layer_id logical id of the layer (>0 and <= Hcal_nlayers)
      @param xOffset offset depending on the symmetry angle (pi/8), to be set
      @param x_halfLength half length in the x-direction of the HCAL layer (to be set)
      @param xShift shift from the origin of the local coordinate system (to be set)
  */
  void CalculateXLayer(G4int layer_id, G4int &logical_layer_id,
		       G4double &xOffset, G4double &x_halfLength, G4double &xShift);

  
  /**Build the HCAL endcap rings
     @param MotherLog logical volume of the mother (world)
  */
  void EndcapRings(G4LogicalVolume *MotherLog);

  /**Build the HCAL endcap Ring chambers
     @param MotherLog logical volume of the mother (endcaps)
     @param endcapSD sensitive detector of the endcaps
     @param debug flag for debug (if true, print out value of inner and outer radius)
  */
  void EndcapRingChambers(G4LogicalVolume* MotherLog, SDHcalEndCap* endcapSD, bool debug);
  
	
  //=========Endcap===============
  /**Build the HCAL endcaps as in the new Geometry design
     @param MotherLog logical volume of the mother (world)
  */
  void EndcapsAhcal(G4LogicalVolume* MotherLog);
  
  /**Build the HCAL endcap chambers as in the new Geometry design
     @param MotherLog logical volume of the mother
     @param endcapSD sensitive detector of the endcaps
     @param layerId Layer ID number
     @param pULimits 
  */
  void EndcapChambersAhcal(G4LogicalVolume* MotherLog, 
			   SDAHcalEndCapScalable* endcapSD,
			   G4int layerId,
			   G4UserLimits* pULimits);

  /** Calculate the y-dimension of the endcaps module active layer
  */
  G4double CalculateHalfYEndcapModule(G4double moduleXOffset);

	
  /** Build the Endcaps front end electronics
      @param MotherLog logical mother volume (i.e. EndcapsAhcalFrontEndElectronics)
      @param layerId Layer ID number
  */
  void EndcapsAhcalFrontEndElectronics(G4LogicalVolume *MotherLog, G4int layerId);
	

  SDHcalBarrel      *theBarrilRegSD;  /**<sensitive detector of the HCAL barrel*/
  SDAHcalEndCapScalable     *theENDCAPEndSD;  /**<sensitive detector of the HCAL endcaps in the Tesla design*/
  SDHcalEndCap      *theENDCAPRingSD; /**<sensitive detector of the HCAL endcap rings*/

  G4Material *BarrelRadiatorMaterial; /**<radiator (i.e. absorber) material in the HCAL barrel*/
  G4Material *EndcapRadiatorMaterial; /**<radiator (i.e. absorber) material in the HCAL endcaps*/
  G4Material *S235;                   /**<stainless steel of type S235*/
  G4Material *PCB;                    /**<PCB material (FR-4)*/
  G4Material *Cu;                     /**<G4_Cu*/
  G4Material *Air;                    /**<G4_AIR*/

  G4double theMaxStepAllowed;/**<maximum step allowed in GEANT4*/

  /* Run time parameters: taken from the data base (default values) 
     or from the user's steering file (user value).
  */
  G4double Hcal_radiator_thickness;          /**<thickness of the HCAL absorber */
  G4String Hcal_radiator_material;           /**<type of the HCAL absorber, i.e. steel or tungsten*/
  G4int    Hcal_ring;                        /**<=0 no rings, =1 use rings*/
  G4double Hcal_radial_ring_inner_gap;       /**<inner gap of the radial ring*/
  G4String Hcal_sensitive_model;             /**<HCAL sensitive mode: 'Scintillator' or 'RPC1'*/
  G4double Hcal_back_plate_thickness;        /**<thickness of the HCAL back plate*/
  G4double Hcal_stave_gaps;                  /**<gapg between HCAL staves*/
  G4double Hcal_modules_gap;                 /**<gap between HCAL modules*/
  G4int    Hcal_nlayers;                     /**<number of HCAL layers (default: 48)*/
  G4int    Hcal_barrel_end_module_type;      /**<type of the HCAL modules (default: 1)*/
  G4double Hcal_fiber_gap;                   /**<gap between HCAL fibers*/
  G4double Hcal_chamber_tickness;            /**<thickness of the HCAL chambers*/
  G4double Hcal_inner_radius;                /**<inner radius of the HCAL*/
  G4double TPC_Ecal_Hcal_barrel_halfZ;       /**<half-length of the HCAL barrel (including gap between modules)*/
  G4double Hcal_normal_dim_z;                /**<length of the HCAL modules along z*/
  G4double Hcal_top_end_dim_z;               /**<length of the HCAL module's top along z*/
  G4double Hcal_start_z;                     /**<HCAL start position in z*/
  G4double Ecal_endcap_outer_radius;         /**<outer radius of the ECAL endcap*/
  G4double Ecal_endcap_zmin;                 /**<minimum z of the ECAL endcap*/
  G4double Ecal_endcap_zmax;                 /**<maximum z of the ECAL endcap*/
  G4double Hcal_lateral_plate_thickness;     /**<thickness of the HCAL lateral plate*/
  G4double Hcal_cells_size;                  /**<size of the HCAL cell*/
  G4double Hcal_digi_cells_size;             /**<size of the HCAL cell*/

  G4double Hcal_endcap_cables_gap;           /**<cables gap in the endcap*/
  G4double Hcal_endcap_ecal_gap;             /**<gap between ECAL and HCAL*/
  G4double Hcal_endcap_rmax;                 /**<maximum radius of the endcaps*/
  G4double Hcal_endcap_center_box_size;      /**<size of the HCAL endcap center box*/
  G4double Hcal_endcap_sensitive_center_box; /**<siez of the HCAL sensitive center box*/
  G4double Hcal_endcap_radiator_thickness;   /**<thickness of the radiator in the HCAL endcap*/
  G4String Hcal_endcap_radiator_material;    /**<type of absorber material in the HCAL endcap*/
  G4int    Hcal_endcap_nlayers;              /**<number of layers in the HCAL endcap*/
  G4double Hcal_endcap_total_z;              /**<total length along z of the HCAL endcap*/
  G4double Hcal_endcap_module_width;         /**<total width along x of the HCAL endcap*/
  G4int Hcal_endcap_module_number;            /**<total number of endcap module along x of the HCAL endcap*/
  G4double Hcal_endcap_lateral_structure_thickness;  /**<total lateral wall structure thickness along x of the HCAL endcap*/
  G4double Hcal_endcap_layer_air_gap;          /**<total length along x of the HCAL endcap layer air gap*/

  G4double Hcal_total_dim_y;                 /**<total dimension of the HCAL detector along the y-axis*/
  G4double Hcal_module_radius;               /**<radius of an HCAL module*/
  G4double Hcal_y_dim2_for_x;                /**<y-dimension of the lower part of the HCAL module*/
  G4double Hcal_y_dim1_for_x;                /**<y-dimension of the upper part of the HCAL module*/
  G4double Hcal_bottom_dim_x;                /**<x-dimension of the bottom part of the HCAL module*/
  G4double Hcal_midle_dim_x;                 /**<x-dimension of the middle part of the HCAL module*/
  G4double Hcal_top_dim_x;                   /**<x-dimension of the top part of the HCAL module*/
  G4double Hcal_regular_chamber_dim_z;       /**<z-dimension of the HCAL chamber*/
  G4double Hcal_cell_dim_x;                  /**<dimension of the HCAL cell along the x-axis*/
  G4double Hcal_cell_dim_z;                  /**<dimension of the HCAL cell along the z-axis*/
  G4double Hcal_digi_cell_dim_x;             /**<x-dimension of the HCAL cell*/
  G4double Hcal_digi_cell_dim_z;             /**<z-dimension of the HCAL cell*/ 

  G4double Hcal_layer_air_gap;               /**<air gap in the HCAL layer*/
  G4double Hcal_chamber_thickness;           /**<thickness of the HCAL chamber*/
  G4double Hcal_middle_stave_gaps;           /**<gap in the middle of HCAL staves*/
  G4bool   Hcal_apply_Birks_law;             /**<flag for applying (or not) the Birks law*/
  G4double Hcal_scintillator_thickness;      /**<thickness of the HCAL scintillator*/

  G4double Hcal_steel_cassette_thickness;   /**<thickness of the HCAL steel cassette*/
  G4double Hcal_Cu_thickness;               /**<thickness of Cu in the HCAL layer*/
  G4double Hcal_PCB_thickness;              /**<thickness of PCB (FR-4) in the HCAL layer*/
  G4double layerThickness;                  /**<thickness of endcap layers*/
  G4double Hcal_endcap_services_module_width; /**<total width of endcap front end electronics*/
  G4double HcalServices_outer_FR4_thickness; /**<thickness of PCB (FR-4) in the endcap services*/
  G4double HcalServices_outer_Cu_thickness;  /**<thickness of Cu  in the the endcap services*/
	
#ifdef MOKKA_GEAR
  struct helpParameters
  {
    G4double innerRadius ;
    G4double outerRadius ;
    G4double zMax ;
    G4double phi0 ;
    std::vector<double> layerPos ;
    std::vector<double> layerPos2 ;
    std::vector<double> steelCassetteThickness;
    std::vector<double> fiberGap;
    std::vector<double> sensThickness ;
    std::vector<double> PCBThickness;
    std::vector<double> CuThickness;
    std::vector<double> fractCellSize1;/*x-dimension of the fractional cell*/
    G4int count ;
    G4double leastZ ;
    G4double mostZ ;
  };

  helpParameters helpBarrel;
  helpParameters helpEndcap;
  helpParameters helpEndcapRing;

  /*parameters*/
  G4double dblParamModulesGap;
  G4double dblParamFiberGap;
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
  G4double dblParamSteelCassetteThickness;
  G4double dblParamScintillatorThickness;
  G4double dblParamCuThickness;
  G4double dblParamPCBThickness;
#endif
};

const double SHcalSc04::eps = 0.0000001;

#endif
