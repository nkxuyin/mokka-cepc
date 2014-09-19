#ifndef TBhcal07_h
#define TBhcal07_h 1

#include "Control.hh"
#include "TBSD_VCell03.hh"
#include "VSubDetectorDriver.hh"
#include "CGAGeometryEnvironment.hh"

#define MAX_TBHCAL_LAYERS 100

class TBSD_VCell03;
class Database;
class G4LogicalVolume;
class G4Material;
class G4VPhysicalVolume;

class TBhcal07 : public VSubDetectorDriver
{

public:
  /** Default constructor
   */
  TBhcal07();
  /** Destructor
   */
  ~TBhcal07();

  /** Main function called in Mokka
   */
  G4bool ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
			     G4LogicalVolume *theWorld);

  /** Get HCAL grid size
   */
  G4double GetGridSize() { return grid_size; }

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

      Note: only one terminating absorber plate is build, of thickness 20.5 mm.
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

  
  /** Check if 'sensitiveLayerPattern' is a valid string consisting of only 0's 
      and 1's with a length of 'n_layers'.
      The variable 'n_layers' needs to be set correctly before using this method.
      @param sensitiveLayerPattern string to be checked
  */
  G4bool isValidSensitiveLayerPattern(G4String sensitiveLayerPattern);

  /** Fill std::vector<G4int> according to the string 'sensitiveLayerPattern'
      The variable 'n_layers' needs to be set correctly before using this method.
  */
  std::vector<G4int> getSensitiveLayerPatternVector(G4String sensitiveLayerPattern);

private: 

  Database* db;        /**< pointer to Mokka data base*/
  TBSD_VCell03 *hcalSD; /**<HCAL sensitive detector*/
  
  CGAGeometryEnvironment _aGeometryEnvironment;/**<The Environment object*/

  G4int n_layers;         /**< number of HCAL layers*/
  G4int ncell_xy[2];      /**< number of cells in x and y direction */
  G4double grid_size;     /**< parameter determing the cellwise subdivision of a layer */
  G4double cal_hx;        /**< half x-dimension of HCAL detector box*/
  G4double cal_hy;        /**< half y-dimension of HCAL detector box*/
  G4double cal_hz;        /**< half z-dimension of HCAL detector box*/
  G4double x_begin;       /**< x-position where the HCAL detector starts*/
  G4double y_begin;       /**< y-position where the HCAL detector starts*/
  G4double z_begin;       /**< z-position where the HCAL detector starts*/
  G4double z_place;       /**< z-position where the HCAL detector is placed*/
  G4double rotationAngle; /**<rotation angle*/
  G4double config_angle;  /**< configuration angle of detector*/
  
  G4int depthToLayer; /**<Variable describing the depth of where the layer is implemented within the G4 volumes hierarchy*/

  G4double poly_hthickness;           /**< half thickness (along z) of the scintillator (made of polystyrene)*/
  G4double airgap_hthickness;         /**< half thickness of the air gap*/
  G4double steel_cassette_hthickness; /**< half thickness of the steel cassette*/
  G4double foil_hthickness;           /**< half thickness of the 3M foil*/
  G4double pcb_hthickness;            /**< half thickness of the PCB board*/
  G4double cablefibre_mix_hthickness; /**< half thickness of the cable-fibre mixture*/
    
  G4double airGapThickness;           /**< full thickness of the airgap, which can be placed instead of a sensitive cassette*/
  G4double steel_hthickness_term;     /**<half thickness of the terminating absorber plate (made of steel)*/

  G4double scinCassette_hthickness; /**< half thickness of the scintillator cassette + contents + air*/

  G4LogicalVolume *WholeLayerLogical[MAX_TBHCAL_LAYERS];     /**< logical volume for the fully instrumented HCAL layer*/
  G4LogicalVolume *AbsLayerLogical[MAX_TBHCAL_LAYERS];       /**< logical volume for the absorber plate */
  G4LogicalVolume *ScinHousLogical[MAX_TBHCAL_LAYERS];       /**< logical volume for the steel cassette */
  G4LogicalVolume *CFmix_LayerLogical[MAX_TBHCAL_LAYERS];    /**< logical volume for the cable-fibre mix*/
  G4LogicalVolume *PCBLayerLogical[MAX_TBHCAL_LAYERS];       /**< logical volume for the PCB board*/
  G4LogicalVolume *FoilLayerLogical_1[MAX_TBHCAL_LAYERS];    /**< logical volume for the first 3M foil */
  G4LogicalVolume *FoilLayerLogical_2[MAX_TBHCAL_LAYERS];    /**< logical volume for the second 3M foil */
  G4LogicalVolume *WholeSensLayerLogical[MAX_TBHCAL_LAYERS]; /**< logical volume for the scintillator*/

  G4LogicalVolume *WholeScinCassetteLogical[MAX_TBHCAL_LAYERS]; /**< logical volume for the scintillator cassette */
//   G4LogicalVolume *WholeAbsLogical[MAX_TBHCAL_LAYERS];          /**< logical volume for the absorber plate */

  G4double steel_hthickness[MAX_TBHCAL_LAYERS];              /**< half thickness of the absorber plate (made of steel)*/
  G4double layer_hthickness[MAX_TBHCAL_LAYERS];              /**< half thickness of the HCAL layer*/



  G4LogicalVolume *AbsLayerLogical_term;  /**< logical volume for the terminating absorber plate */
  G4LogicalVolume *DetectorLogical;       /**< logical volume for the HCAL detector */
  G4LogicalVolume *WorldLogical;          /**< world logical volume*/

  G4LogicalVolume *WholeLayer2Logical[MAX_TBHCAL_LAYERS]; /**< logical volume for the uninstrumented HCAL layer*/
  G4LogicalVolume *AirGap_LayerLogical;                   /**< logical volume for the air gap in the uninstrumented HCAL layer*/
  std::vector<G4int> sensitiveLayerPatternVector;         /**<vector of integers indicating the arrangement of sensitive 
							     layers in the HCAL prototype (e.g. "101010...10 indicates that
							     each second layer is equiped with a sensitive casette)*/

  G4Material *poly;          /**< polystyrene - used as scintillator material*/
  G4Material *Polystyrole;   /**< material of the 3M foil*/
  G4Material *air;           /**< guess :) air*/
  G4Material *PCB;           /**< material used for the PCB board*/       
  G4Material *S235;          /**< steel of type S235 - used for absorber plate and steel cassette material*/
  G4Material *CF_MIX;        /**< material of the cable-fibre mix*/

  G4bool checkForOverlappingVolumes;/**< flag to check for overlapping volumes (for debug purposes)*/


};

#endif


