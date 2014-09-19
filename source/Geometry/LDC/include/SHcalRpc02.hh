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

#ifndef SHcalRpc02_h
#define SHcalRpc02_h 1

#include "G4IntersectionSolid.hh"

#include "VSubDetectorDriver.hh"
#include "SDHcalBarrel.hh"

#include "SDHcalEndCap.hh"
#include "SDHcalEndCapTesla.hh"
    
/** @class SHcalRpc02 SHcalRpc02.hh "SHcalRpc02.hh" 
    \brief HCAL superdriver

    HCAL superdriver for the scintillator option, 
    with more realistic description, compared to SHcal03.
    For a description of the geometry and parameters used by this drivers,
    please have a look at the Linear Collider note: 

    http://www-flc.desy.de/lcnotes/   
    -> LC-TOOL-2008-001
*/
class SHcalRpc02 : public VSubDetectorDriver
{
public:
  
  /**Constructor
   */
  SHcalRpc02() : VSubDetectorDriver("SHcalRpc02","hcal"),
		theBarrilRegSD(0),
		theENDCAPEndSD(0),
		theMaxStepAllowed(DBL_MAX)
  {}

  /**Destructor
   */
  ~SHcalRpc02();

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

  /** Build RPC sensitive layer
  */  
  void BarrelHalfRegularChambersTrapRPC(G4LogicalVolume *MotherLog, 
				     G4double chambers_y_off_correction);

  // RPC2
  G4LogicalVolume * BuildRPC2Box(G4Box* ChamberSolid,
                                 SDHcalBarrel* theSD,
                                 G4int layerId,
                                 G4UserLimits* pULimits);

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


//   /** Build the HCAL endcaps (not used anymore)
//       @param MotherLog logical mother volume
//   */
//   void Endcaps(G4LogicalVolume *MotherLog);




  /**Build the HCAL endcaps as in the Tesla design
     @param MotherLog logical volume of the mother (world)
  */
  void EndcapsTesla(G4LogicalVolume* MotherLog);
  
  /**Build the HCAL endcap rings
     @param MotherLog logical volume of the mother (world)
  */
  void EndcapRings(G4LogicalVolume *MotherLog);

  /**Build the HCAL endcap chambers
     @param MotherLog logical volume of the mother (endcaps)
     @param endcapSD sensitive detector of the endcaps
     @param debug flag for debug (if true, print out value of inner and outer radius)
  */
  void EndcapChambers(G4LogicalVolume* MotherLog, SDHcalEndCap* endcapSD, bool debug);
  
  void EndcapChambersRPC(G4LogicalVolume* MotherLog, SDHcalEndCap* theSD);

  G4IntersectionSolid  * EndcapRingRPCComponent(
        G4Box *IntersectionStaveBox, G4RotationMatrix *rot,
        G4ThreeVector &IntersectXYZtrans, G4double halfWidth,
        G4double phiStart, G4double phiTotal, G4int numSide,
        G4int numZPlanes, G4double rInner[2], G4double rOuter[2]);

  /**Build the HCAL endcap chambers as in the Tesla design
     @param MotherLog logical volume of the mother
     @param endcapSD sensitive detector of the endcaps
  */
  void EndcapChambersTesla(G4LogicalVolume* MotherLog, SDHcalEndCapTesla* endcapSD);

  void EndcapChambersTeslaRPC(G4LogicalVolume* MotherLog, SDHcalEndCapTesla* endcapSD);
  
  void BuildMaterials(void);

  SDHcalBarrel      *theBarrilRegSD;  /**<sensitive detector of the HCAL barrel*/
  
  SDHcalEndCapTesla *theENDCAPEndSD;  /**<sensitive detector of the HCAL endcaps in the Tesla design*/
  SDHcalEndCap      *theENDCAPRingSD; /**<sensitive detector of the HCAL endcap rings*/

  G4Material *BarrelRadiatorMaterial; /**<radiator (i.e. absorber) material in the HCAL barrel*/
  G4Material *EndcapRadiatorMaterial; /**<radiator (i.e. absorber) material in the HCAL endcaps*/
  G4Material *S235;                   /**<stainless steel of type S235*/
  G4Material *PCB;                    /**<PCB material (FR-4)*/
  G4Material *Cu;                     /**<G4_Cu*/

  G4Material *theRPC2ECRMixture;
  G4double mixThickness;

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

  G4double Hcal_layer_support_length;        /**<length of the HCAL layer support*/
  G4double Hcal_layer_air_gap;               /**<air gap in the HCAL layer*/
  G4double Hcal_chamber_thickness;           /**<thickness of the HCAL chamber*/
  G4double Hcal_middle_stave_gaps;           /**<gap in the middle of HCAL staves*/
  G4bool   Hcal_apply_Birks_law;             /**<flag for applying (or not) the Birks law*/
  G4double Hcal_scintillator_thickness;      /**<thickness of the HCAL scintillator*/

  G4double Hcal_steel_cassette_thickness;   /**<thickness of the HCAL steel cassette*/
  G4double Hcal_Cu_thickness;               /**<thickness of Cu in the HCAL layer*/
  G4double Hcal_PCB_thickness;              /**<thickness of PCB (FR-4) in the HCAL layer*/

  G4Material * theGraphiteMaterial;
  G4Material * theMylarMaterial;
  G4Material * theG10Material;
  G4Material * theAirMaterial;
  G4double  Hcal_spacer_thickness;
  G4double  Hcal_spacer_separation;

//EndCap/Ring gas offset
//  G4double ECR_RPC_GapPosX;
//Emmanuel's parameters
  G4double RPC_GapPosX;
  G4double RPC_PCB_Thickness, RPC_mylar_ThicknessAnode;
  G4double RPC_PadSeparation;
  G4double RPC_mylar_ThicknessCathode;
  G4double RPC_Graphite_ThicknessAnode;
  G4double RPC_Graphite_ThicknessCathode;
  G4double RPC_ThinGlass, RPC_Gap_Thickness;
  G4double RPC_ThickGlass, RPC_EdgeWidth;
  G4double RPCGazInletInnerRadius, RPCGazInletOuterRadius;
  G4double RPCGazInletLength, RPC_ChipPackageThickness;
  G4double RPC_Free_Thickness;

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
    std::vector<double> sensThickness ;
    std::vector<double> PCBThickness;
    std::vector<double> CuThickness;
    std::vector<double> thickGlassThickness;
    std::vector<double> thinGlassThickness;
    std::vector<double> mixThickness;
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
  G4double dblParamSteelCassetteThickness;
  G4double dblParamScintillatorThickness;
  G4double dblParamCuThickness;
  G4double dblParamPCBThickness;
#endif
};

#endif
