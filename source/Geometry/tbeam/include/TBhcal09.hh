#ifndef TBhcal09_h
#define TBhcal09_h 1

#include "Control.hh"
#include "TBSD_VCell04.hh"
#include "VSubDetectorDriver.hh"
#include "CGAGeometryEnvironment.hh"

#define MAX_TBHCAL_LAYERS 38

class Database;

class TBhcal09 : public VSubDetectorDriver
{
public:
  /** Default constructor
   */
  TBhcal09();
  /** Destructor
   */
  ~TBhcal09();

  /** Main function called in Mokka
   */
  G4bool ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
			     G4LogicalVolume *theWorld);

  /** Get HCAL grid size
   */
  G4double GetGridSize() { return HCAL_grid_size; }

  /** Get depth to layer
   */
  G4int GetDepthToLayer() const { return depthToLayer; }
  
private:

  /** Build HCAL elements. An HCAL layers contains:
      - absorber plate
      - air gap
      - steel cassette
      - cable-fibre mix
      - PCB
      - 3M foil
      - scintillator
      - 3M foil
      - steel cassette
      - air gap
      HCAL contains 38 such layers, plus an additional terminating absorber plate.
   */
  void BuildHcalElements();

  /** Set variable absorber thicknesses for each layer
   */
  void SetHcalAbsorberThickness();
  /** Place HCAL elements into layer
   */
  void PlaceHcalElementsIntoLayer();

  /** Set HCAL sensitive detector
   */
  void SetSD(); 

  /** Place HCAL layers into the world volume
      @ worldLog - world logical volume

      Note: only one terminating absorber plate is build
   */
  G4bool PlaceHcalLayers(G4LogicalVolume *worldLog);

  /** Fetch HCAL related MySQL variables from the Mokka data base
   */
  void FetchMySQLVariables();

  /** Print out HCAL dimensions
   */
  void Print();

  /** Define new GEANT4 materials needed for HCAL
   */
  void DefineHcalMaterials();

  /** Set the level on which the layer is implemented in G4
   */
  void SetDepthToLayer(G4int i) {
    depthToLayer = i;
#ifdef TBSD_DEBUG
    G4cout <<"DepthToLayer in Hcal: " << depthToLayer << G4endl;
#endif
  }

 private: 

  Database* db;        /**< pointer to Mokka data base*/
  TBSD_VCell04 *hcalSD; /**<HCAL sensitive detector*/
  TBSD_VCell04 *hcalSD_W; /**<HCAL sensitive detector*/
  TBSD_VCell04 *hcalSD_FeSupport; /**<HCAL sensitive detector*/
  TBSD_VCell04 *hcalSD_FeCassette1;
  TBSD_VCell04 *hcalSD_FeCassette2;
  
  CGAGeometryEnvironment _aGeometryEnvironment;/**<The Environment object*/

  G4int HCAL_n_layers;     /**< number of HCAL layers*/
  G4int HCAL_Layer_ncell_x;/**< number of cells in x and y direction */
  G4int HCAL_Layer_ncell_y;/**< number of cells in x and y direction */
  G4double HCAL_grid_size;/**< parameter determing the cellwise subdivision of a layer */
  G4double cal_hx;        /**< half x-dimension of HCAL detector box*/
  G4double cal_hy;        /**< half y-dimension of HCAL detector box*/
  G4double cal_hz;        /**< half z-dimension of HCAL detector box*/
  G4double x_begin;       /**< x-position where the HCAL detector starts*/
  G4double y_begin;       /**< y-position where the HCAL detector starts*/
  G4double z_begin;       /**< z-position where the HCAL detector starts*/
  G4double z_place;       /**< z-position where the HCAL detector is placed*/
  G4double rotationAngle; /**<rotation angle*/
  G4double config_angle;  /**< configuration angle of detector*/
  G4double HCAL_Layer_X_dimension; /**< X dimension of HCAL Layer*/
  G4double HCAL_Layer_Y_dimension; /**< X dimension of HCAL Layer*/

  /*Absorber*/
  G4double Octagonal_absorber_inner_radius[2]; /**< Inner radius of octagonal absorber*/
  G4double Octagonal_absorber_outer_radius[2]; /**< Outer radius of octagonal absorber*/
  G4double Octagonal_absorber_z[2];             /**< Z of octagonal absorber*/
  G4int Octagonal_absorber_number_of_sides;     /**< Nr of sides of ctagonal absorber*/
  G4int Octagonal_absorber_number_of_layers;    /**< Nr of layers octagonal absorber*/

  G4int depthToLayer; /**<Variable describing the depth of where the layer is implemented within the G4 volumes hierarchy*/

  G4double poly_hthickness;           /**< half thickness (along z) of the scintillator (made of polystyrene)*/
  G4double airgap_hthickness;         /**< half thickness of the air gap*/
  G4double tungsten_hthickness;       /**< half thickness of the tungsten layer*/
  G4double steel_support_hthickness;  /**< half thickness of the steel support layer*/
  G4double steel_cassette_hthickness; /**< half thickness of the steel cassette*/
  G4double foil_hthickness;           /**< half thickness of the 3M foil*/
  G4double pcb_hthickness;            /**< half thickness of the PCB board*/
  G4double cablefibre_mix_hthickness; /**< half thickness of the cable-fibre mixture*/
    
  G4double airGapThickness;           /**< full thickness of the airgap, which can be placed instead of a sensitive cassette*/
  G4double scinCassette_hthickness; /**< half thickness of the scintillator cassette + contents + air*/

  G4LogicalVolume *WholeLayerLogical[MAX_TBHCAL_LAYERS];         /**< logical volume for the fully instrumented HCAL layer*/
  G4LogicalVolume *AbsLayerLogical[MAX_TBHCAL_LAYERS];           /**< logical volume for the absorber plate */
  G4LogicalVolume *ScinHousLogical[MAX_TBHCAL_LAYERS];           /**< logical volume for the steel cassette */
  G4LogicalVolume *ScinHousLogical2[MAX_TBHCAL_LAYERS];          /**< logical volume for the steel cassette */
  G4LogicalVolume *CFmix_LayerLogical[MAX_TBHCAL_LAYERS];        /**< logical volume for the cable-fibre mix*/
  G4LogicalVolume *PCBLayerLogical[MAX_TBHCAL_LAYERS];           /**< logical volume for the PCB board*/
  G4LogicalVolume *FoilLayerLogical_1[MAX_TBHCAL_LAYERS];        /**< logical volume for the first 3M foil */
  G4LogicalVolume *FoilLayerLogical_2[MAX_TBHCAL_LAYERS];        /**< logical volume for the second 3M foil */
  G4LogicalVolume *WholeSensLayerLogical[MAX_TBHCAL_LAYERS];     /**< logical volume for the scintillator*/
  G4LogicalVolume *SteelSupportLogical[MAX_TBHCAL_LAYERS];       /**< logical volume for the steel support*/ 
  G4LogicalVolume *AluminiumframeLogical[MAX_TBHCAL_LAYERS];     /**< logical volume for the steel frame*/ 
  G4LogicalVolume *WholeScinCassetteLogical[MAX_TBHCAL_LAYERS];  /**< logical volume for the scintillator cassette */
  G4LogicalVolume *AbsLayerLogical_term;                         /**< logical volume for the terminating absorber plate */
 
  G4double steel_hthickness[MAX_TBHCAL_LAYERS];              /**< half thickness of the absorber plate (made of steel)*/
  G4double layer_hthickness[MAX_TBHCAL_LAYERS];              /**< half thickness of the HCAL layer*/

  G4LogicalVolume *DetectorLogical;       /**< logical volume for the HCAL detector */
  G4LogicalVolume *WorldLogical;          /**< world logical volume*/

  G4LogicalVolume *WholeLayer2Logical[MAX_TBHCAL_LAYERS]; /**< logical volume for the uninstrumented HCAL layer*/
  G4LogicalVolume *AirGap_LayerLogical;                   /**< logical volume for the air gap in the uninstrumented HCAL layer*/


  G4Material *poly;          /**< polystyrene - used as scintillator material*/
  G4Material *Polystyrole;   /**< material of the 3M foil*/
  G4Material *air;           /**< guess :) air*/
  G4Material *PCB;           /**< material used for the PCB board*/       
  G4Material *S235;          /**< steel of type S235 - used for absorber plate and steel cassette material*/
  G4Material *tungstenalloy; /**< tungsten composed of tungsten Nikel and Copper*/
  G4Material *aluminium;     /** < Aluminiuml*/
  G4Material *CF_MIX;        /**< material of the cable-fibre mix*/
  G4Material *nikel;         /**< material nikel for tungsten alloy*/
  G4Material *coretun;       /**< material tungsten core for tungsten alloy*/
  G4bool checkForOverlappingVolumes;/**< flag to check for overlapping volumes (for debug purposes)*/

  G4double PCB_density;		
  G4double PCB_silicon_fractiomass;
  G4double PCB_elO_fractionmass;		
  G4double PCB_graphite_fractionmass;	
  G4double PCB_elH_fractionmass;		
  G4double PCB_elBr_fractionmass;		
  G4double S235_density;			
  G4double S235_iron_fractionmass;	
  G4double S235_graphite_fractionmass;
  G4double S235_manganese_fractionmass;
  G4double tungstenalloy_density;
  G4double coretun_density;		
  G4double nikel_density;		
  G4double tungsten_fractionmass;
  G4double nikel_fractionmass;		
  G4double copper_fractionmass;		
  G4double PVC_density;				
  G4double Polystyrole_density;			
  G4double CF_Mix_density;			
  G4double CF_MIX_air_fractionmass;		
  G4double CF_MIX_PVC_fractionmass;		
  G4double CF_MIX_Polystyrole_fractionmass;	


};

#endif


