/*
 * FTD Self-Scaling Driver for Mokka 
 *
 * FTD_Simple_Staggered.hh - driver class header
 * 
 */

#ifndef FTD_Simple_Staggered_h
#define FTD_Simple_Staggered_h 1

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
	G4double petal_y_ratio;
    
};

struct dbExtended_reconstruction_parameters
{

  G4double strip_width_mm;    
  G4double strip_length_mm;   
  G4double strip_pitch_mm;    
  G4double strip_angle_deg;   
  
};

struct dbInfoDisk
{
	G4int disk_number;
  
  G4int sensor_is_pixel;
  G4int double_sided;
  
	G4double disks_Si_thickness;  
        // Support Cylinders
	G4double ZStartOuterCylinder;
	G4double ZStopOuterCylinder;
	G4double ZStartInnerCylinder;
    G4double ZStopInnerCylinder;
        // Petal	
	G4double petal_cp_support_thickness;
	G4double petal_cp_support_dxMax;
	G4double petal_support_zoffset;
    
};


class FTD_Simple_Staggered : public VSubDetectorDriver
{
public:
    FTD_Simple_Staggered(void) : VSubDetectorDriver("FTD_Simple_Staggered","ftd")  {} 
    ~FTD_Simple_Staggered(void) {};
    
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
    void RegisterSDs( Database * db ); // registers two sensitive detectors one for pixel disks and one for strips
    
		// Construction of parametrized values for the petals (dy, dxMin,..)
    G4double Getdy( const G4double & the_inner_radius );
    G4double Getdx( const G4double & the_inner_radius );
		// -----------------------------------------------------------------
    
		// Petal support construction ------------------------------------
    void DoAndPlaceDisk( std::map<std::string,G4double> valuesDict, G4LogicalVolume * mother );
    
    void petalSupport( std::map<std::string,G4double>  valuesDict, G4LogicalVolume * mother ); 
    void petalSensor( std::map<std::string,G4double>  valuesDict, G4LogicalVolume * mother ); 
    

    std::vector<G4double> * GetPetalDimensions( const G4double& petal_cp_support_dy, 
                                               const G4String& whereItgoes, const G4bool isSilicon );

    G4Trap * SemiPetalSolid(const G4double& petal_cp_support_dy, const G4String& whereItgoes,
                            const G4bool isSilicon ); 
		// -----------------------------------------------------------------
    
		// DB Parameters
    dbInfoCommon _dbParCommon;
    dbInfoDisk _dbParDisk;
	  dbExtended_reconstruction_parameters _dbParExReco;

    // Global environment variables
    glEnviron _glEnv;
  
  
    G4Material* _SiMat;
    G4Material* _KaptonMat;
    G4Material* _CuMat;
    G4Material* _AirMat;
    G4Material* _CarbonFiberMat;
    
    G4VisAttributes * _VisAttSupport;
    G4VisAttributes * _VisAttSensitive;
    G4VisAttributes * _VisAttHolePetal;
    
		// Disk positioning parameters
    G4double _z_position;
    G4double _zEnd;
    G4double _inner_radius;
    G4double _outer_radius;
    G4double _beamTubeRadius;
    
    TRKSD_FTD01* _theFTDSD_pixel;
    TRKSD_FTD01* _theFTDSD_strip;
    
		// Storing all the gear parameters
    std::map<int,std::vector<double> > _ftdparameters;
    
		// Registring the Physical Volumes to be deleted
    std::vector<G4VPhysicalVolume*> _registerPV;
};

#endif


