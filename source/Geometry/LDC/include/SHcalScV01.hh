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
//
// S.Lu: Barrel identical to SDHcalRpc01 videau Geometry
//       Endcap and EndcapRing identical to SHcalSc03

#ifndef SHcalScV01_h
#define SHcalScV01_h 1

class G4LogicalVolume;
class G4VPhysicalVolume;

class Database;
class G4VSolid;
class G4Box;

class G4UserLimits;

#include "globals.hh"
#include "SDHcalBarrelV.hh"
#include "VSubDetectorDriver.hh"
#include "SDHcalEndCap.hh"
#include "SDAHcalEndCap.hh"


class G4Polyhedra;
class G4Material;

class SHcalScV01 : public VSubDetectorDriver
{
public:
	
	/**Constructor
	 */
	SHcalScV01() : VSubDetectorDriver("SHcalScV01","hcal"),
	theBarrilRegSD(0),
	theENDCAPEndSD(0),
	theMaxStepAllowed(DBL_MAX)
	{}
	
	/**Destructor
	 */
	~SHcalScV01();
	
	/**Function requested by Mokka, common to all drivers
	   @param aGeometryEnvironment the geometry environment
	   @param theWorld logical mother volume
	   @return true if detector construction was successful 
	 */
	G4bool 
	ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
			    G4LogicalVolume *theWorld);
	
	/* Propagates the changes to the coil, if any. 
	 */
	G4bool 
	PostConstructAction(CGAGeometryEnvironment& );
	
	
private:
        static const double eps; /**<setup a 1*10E-6 mm space to seperate touch surface*/ 
	G4String SensitiveModel;
	Database* db;
	
	/** Setup of the run time parameters
	    @param aGeometryEnvironment the geometry environment
	    @return true if setting pf the parameters was succesfull
	 */
	G4bool Setup(const CGAGeometryEnvironment &);
	
	/** To reload hits for visualisation
	    @param theSubDetectorEventHitsFileInput file with subdetector hits
	 */
	void LoadEvent(FILE* theSubDetectorEventHitsFileInput);
	
	/**Build the HCAL barrel modules 
	   @param MotherLog mother logical volume (world)
	 */
	void Barrel(G4LogicalVolume*);
	/**Build the HCAL barrel  virtual modules 
	   @param MotherLog mother logical volume (world)
	 */
	void BarrelVirtualModules(G4LogicalVolume*);
	
	/**Build the HCAL barrel  chamber 
	   @param ChamberSolid Chamber logical volume
	   @param theSD sensitive detector
	   @param layerId Layer ID number
	   @param pULimits 
	 */
	G4LogicalVolume * BuildChamberBox(G4Box* ChamberSolid, 
					  SDHcalBarrelV* theSD, 
					  G4int layerId,
					  G4UserLimits* pULimits);
	
	
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
				 SDAHcalEndCap* endcapSD,
				 G4int layerId,
				 G4UserLimits* pULimits);
		
	/** Build the Endcaps front end electronics
	 @param MotherLog logical mother volume (i.e. EndcapsAhcalFrontEndElectronics)
	 @param layerId Layer ID number
	 */
	void EndcapsAhcalFrontEndElectronics(G4LogicalVolume *MotherLog, G4int layerId);
	
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
	
	SDHcalBarrelV     *theBarrilRegSD;  /**<sensitive detector of the HCAL barrel*/
	SDAHcalEndCap     *theENDCAPEndSD;  /**<sensitive detector of the HCAL endcaps in the update design*/
	SDHcalEndCap      *theENDCAPRingSD; /**<sensitive detector of the HCAL endcap rings*/
	
	G4Material *RadiatorMaterial;       /**<radiator (i.e. absorber) material in the HCAL endcaps*/
	
	G4Material *EndcapRadiatorMaterial; /**<radiator (i.e. absorber) material in the HCAL endcaps*/
	G4Material *S235;                   /**<stainless steel of type S235*/
	G4Material *PCB;                    /**<PCB material (FR-4)*/
	G4Material *Cu;                     /**<G4_Cu*/
	G4Material *Air;                    /**<G4_AIR*/
	G4String Hcal_sensitive_model;      /**<HCAL sensitive mode: 'Scintillator' or 'RPC1'*/
	
	G4double theMaxStepAllowed;
	
	// Run time parameters
	G4double Hcal_radiator_thickness;
	G4String Hcal_radiator_material;
	G4int    Hcal_ring;
	G4double Hcal_radial_ring_inner_gap;
	G4double Hcal_stave_gaps;
	G4double Hcal_modules_gap;
	G4int    Hcal_nlayers;
	G4int    Hcal_barrel_number_modules;
	G4double hPrime;
	G4double layerThickness;
	G4double Hcal_inner_radius, Hcal_outer_radius;
	G4double Hcal_endcap_cables_gap, Hcal_endcap_ecal_gap;
	G4double Hcal_endcap_rmax;
	G4double TPC_Ecal_Hcal_barrel_halfZ;
	G4double Hcal_normal_dim_z;
	G4double Hcal_start_z;
	G4double Ecal_endcap_outer_radius;
	G4double Ecal_endcap_zmin, Ecal_endcap_zmax;
	G4double Hcal_lateral_plate_thickness;
	G4double Hcal_cells_size;
	G4double Hcal_endcap_center_box_size;
	G4double Hcal_total_dim_y;
	G4double Hcal_endcap_thickness;
	G4double Hcal_endcap_module_width;         /**<total width along x of the HCAL endcap*/
	G4int Hcal_endcap_module_number;            /**<total number of endcap module along x of the HCAL endcap*/
	G4double Hcal_endcap_lateral_structure_thickness;  /**<total lateral wall structure thickness along x of the HCAL endcap*/
	G4double Hcal_endcap_layer_air_gap;          /**<total length along x of the HCAL endcap layer air gap*/
	
	G4int    Hcal_endcap_nlayers;              /**<number of layers in the HCAL endcap*/
	G4double Hcal_endcap_total_z;              /**<total length along z of the HCAL endcap*/
	G4double Hcal_endcap_radiator_thickness;   /**<thickness of the radiator in the HCAL endcap*/
	G4double Hcal_chamber_thickness;           /**<thickness of the HCAL chamber*/
	G4String Hcal_endcap_radiator_material;    /**<type of absorber material in the HCAL endcap*/
	G4double Hcal_endcap_sensitive_center_box; /**<siez of the HCAL sensitive center box*/
	
	G4double Hcal_back_plate_thickness;
	G4double Hcal_regular_chamber_dim_z;
	G4double Hcal_cell_dim_x;
	G4double Hcal_cell_dim_z;
	
	G4double Hcal_scintillator_thickness;
	G4double Hcal_Cu_thickness;
	G4double Hcal_PCB_thickness;
	G4double Hcal_fiber_gap;
	G4double Hcal_apply_Birks_law;
	G4double Hcal_steel_cassette_thickness;   /**<thickness of the HCAL steel cassette*/
	G4double Hcal_endcap_services_module_width; /**<total width of endcap front end electronics*/
	G4double HcalServices_outer_FR4_thickness; /**<thickness of PCB (FR-4) in the endcap services*/
	G4double HcalServices_outer_Cu_thickness;  /**<thickness of Cu  in the the endcap services*/
	
	G4double d_InnerOctoSize;
	
#ifdef MOKKA_GEAR
	// MokkaGear
    
	struct helpParameters{
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
		G4int count ;
		G4double leastZ ;
		G4double mostZ ;
	};
	
	helpParameters helpBarrel;
	helpParameters helpEndcap;
	helpParameters helpEndcapRing;
	
	// parameters
	
	G4double dblParamHalfZ ;
	G4double dblParamModulesGap ;
	G4double dblParamStavesGap ;
	G4double dblParamBarrelMostZ ;
        G4double dblParamFiberGap;
        G4double dblParamBackPlateThickness;
        G4double dblParamHcalModulesGap;
        G4double dblParamHcalStaveGaps;
        G4double dblParamTPCEcalHcalbarrelHalfZ;
        G4double dblParamHcalLateralStructureThickness ;
        G4double dblParamHcalVirtualCellSize;
        G4int    dblHcalBarrelEndModuleType; 
        G4double dblParamHcalEndcapSensitiveCenterBox;
        G4double dblParamSteelCassetteThickness;
        G4double dblParamScintillatorThickness;
        G4double dblParamCuThickness;
        G4double dblParamPCBThickness;
#endif
};

const double SHcalScV01::eps = 0.0000001;

#endif


