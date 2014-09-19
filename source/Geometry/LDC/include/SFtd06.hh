/*
 * FTD Self-Scaling Driver for Mokka 
 *
 * SFtd06.hh - driver class header
 * 
 */

#ifndef SFtd06_h
#define SFtd06_h 1

class G4LogicalVolume;
class Database;
class TRKSD_FTD01;
class G4VisAttributes;

#include "VSubDetectorDriver.hh"
#include "G4Material.hh"
#include "G4Trap.hh"
#include "G4VisAttributes.hh"
#include "G4ReflectionFactory.hh"

#include <map>

class gearpar
{
	public:
		static const short NPETALS             = 1; 
                static const short ZOFFSET             = 2;
                static const short ALPHA               = 3;
                static const short PHI0                = 4;
		static const short PETAL0SIGNOFFSET    = 5;
		static const short HALFANGLEPETAL      = 6;
		static const short ZPOSITION           = 7;
                static const short SUPPORTRINNER       = 8;
                static const short SUPPORTLENGTHMIN    = 9;
                static const short SUPPORTLENGTHMAX    =10; 
                static const short SUPPORTWIDTH        =11; 
                static const short SUPPORTTHICKNESS    =12;
		static const short SUPPORTRANDLENGTH   =13; 
                static const short SENSITIVERINNER     =14; 
                static const short SENSITIVELENGTHMIN  =15; 
                static const short SENSITIVELENGTHMAX  =16;
                static const short SENSITIVEWIDTH      =17; 
		static const short SENSIVIVERANLENGHT  =18;
                static const short SENSITIVETHICKNESS  =19; 
		static const short SENSORTYPE          =20; 
                static const short NSENSORS            =21;
                static const short ISDOUBLESIDED       =22;

};

// Structures to store common parameters (database, etc.. )
// Possible improvement: map<string,G4double>, then no need
// of struct, string = parameter name in database
struct glEnviron
{
      	G4double TPC_Ecal_Hcal_barrel_halfZ;
       	G4double Ecal_endcap_zmin;
       	G4double TPC_inner_radius;
	
       	G4double SIT1_Half_Length_Z;
       	G4double SIT2_Half_Length_Z;
       	G4double SIT1_Radius;
       	G4double SIT2_Radius;
       	G4double VXD_layer3_maxZ;
	
       	G4double zEnd_IPOuterTube;
       	G4double rEnd_IPOuterTube;
       	G4double zEnd_IPOuterBulge;
       	G4double rEnd_IPOuterBulge;
	
       	G4double beamTubeTangent;
};

struct dbInfoCommon
{
	G4double ftd1_vtx3_distance_z;
	G4double ftd7_ecal_distance_z;
	G4double ftd1_sit1_radial_diff;
	G4double ftd2_sit1_radial_diff;
	G4double ftd3_sit2_radial_diff;
	G4double ftd4to7_tpc_radial_gap;

	G4double beamTubeClearance;
	G4double cables_thickness;
	G4double cable_shield_thickness;

	G4double outer_cylinder_total_thickness;
	G4double inner_cylinder_total_thickness;

	// Petal specific
	G4double petal_half_angle_support;  // theta
	G4double petal_wings_length;
	G4double petal_wings_thickness;
	G4double petal_y_ratio;

	G4double chip_strips_thickness;
	G4double chip_strips_width;
	G4double chip_strips_length;
};

struct dbInfoDisk
{
	G4int disk_number;
	
	G4double disks_Si_thickness;  
	// Support Cylinders
	G4double ZStartOuterCylinder;
	G4double ZStopOuterCylinder;
	G4double ZStartInnerCylinder;
       	G4double ZStopInnerCylinder;
	// Petal	
	G4double petal_cp_support_thickness;
	G4double petal_cp_support_dxMax;
	G4double petal_cp_holes_separation;
	G4double petal_cp_holes_edges_separation;
	G4double petal_cp_holes_width_support;
	G4double padUp_Si_dxMax;
	G4double petal_inclination_angle_support; //alpha
	G4double petal_support_zoffset;  // NEW

	G4double kapton_petal_thickness;
	G4double kapton_petal_interspace;
};


class SFtd06 : public VSubDetectorDriver
{
	public:
	      	SFtd06(void) : VSubDetectorDriver("SFtd06","ftd")  {} 
	      	~SFtd06(void) {};
	      	
	      	G4bool ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog);
	      	
#ifdef MOKKA_GEAR    
	      	void GearSetup();
		G4double GetdEdx( const G4Material* material );
#endif
		
	private:
		// register PV
		void registerPV( const G4PhysicalVolumesPair & pvPair );

		// Setters for parameters -----
		void SetEnvironPar( const CGAGeometryEnvironment & env );
		void SetdbParCommon( Database * db );
		void SetParDisk( Database * db );
		// ----------------------------
		void RegisterSD( Database * db );

		// Construction of parametrized values for the petals (dy, dxMin,..)
		G4double Getdy( const G4double & the_inner_radius );
		G4double Getdx( const G4double & the_inner_radius );
		// -----------------------------------------------------------------

		// Petal support construction ------------------------------------
		void DoAndPlaceDisk( std::map<std::string,G4double> valuesDict, G4LogicalVolume * mother );
		
		void petalSupportPixel( std::map<std::string,G4double>  valuesDict, G4LogicalVolume * mother ); 
		void pixelSensors( std::map<std::string,G4double>  valuesDict, G4LogicalVolume * mother ); 
		void elecServPixels( std::map<std::string,G4double>  valuesDict, G4LogicalVolume * mother ); 
		
		void petalSupportMStrips( std::map<std::string,G4double>  valuesDict, G4LogicalVolume * mother ); 
		void microStripsSensors( std::map<std::string,G4double>  valuesDict, G4LogicalVolume * mother ); 
		void elecServMStrips( std::map<std::string,G4double>  valuesDict, G4LogicalVolume * mother ); 

		std::vector<G4double> * GetPetalDimensions( const G4double& petal_cp_support_dy, 
				const G4String& whereItgoes, const G4bool isSilicon );
		G4Trap * SemiPetalSolid(const G4double& petal_cp_support_dy, const G4String& whereItgoes,
				const G4bool isSilicon ); 
		// -----------------------------------------------------------------
	      	
		// DB Parameters
		dbInfoCommon _dbParCommon;
		dbInfoDisk _dbParDisk;
		// Global environment variables
		glEnviron _glEnv;

	      	G4Material *SiMat;
	      	G4Material *KaptonMat;
	      	G4Material *CuMat;
	      	G4Material *Air;
		G4Material *CarbonFiberMat;

		G4VisAttributes * VisAttSupport;
		G4VisAttributes * VisAttHolePetal;
		G4VisAttributes * VisAttSensitive;
		G4VisAttributes * VisAttKaptonSensors;

		// Disk positioning parameters
		G4double z_position;
		G4double zEnd;
		G4double inner_radius;
		G4double outer_radius;
		G4double beamTubeRadius;
		
	      	TRKSD_FTD01 *theFTDSD;

		// Storing all the gear parameters
		std::map<int,std::vector<double> > _ftdparameters;

		// Registring the Physical Volumes to be deleted
		std::vector<G4VPhysicalVolume*> _registerPV;
};

#endif


