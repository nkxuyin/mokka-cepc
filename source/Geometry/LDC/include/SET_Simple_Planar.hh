/*
 * Simple Cylinder based SET 

 * SET_Simple_Planar.hh - driver class header
 * Feb 7th 2011, Steve Aplin 
 */

#ifndef SET_SIMPLE_hh
#define SET_SIMPLE_hh 1

class G4LogicalVolume;
class Database;
class TRKSD02;

#include "VSubDetectorDriver.hh"
#include "G4Material.hh"

class SET_Simple_Planar : public VSubDetectorDriver
{
 public:
  
  SET_Simple_Planar(void) : VSubDetectorDriver("SET_Simple_Planar","set"), db(NULL), _sensitive_thickness(0.0), _support_thickness(0.0), _sensitiveMat(NULL), _supportMat(NULL), _theSETSD(NULL) {} 
  ~SET_Simple_Planar(void) {};
    

  G4bool ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog);

#ifdef MOKKA_GEAR    
  void GearSetup();
#endif

private:
  Database* db;

  struct SET_Layer {
    int     n_ladders;
    int     n_sensors_per_ladder;
    double  sensor_length;
    double  half_z;
    double  sensitive_inner_radius ;
    double  support_inner_radius ;
    double  ladder_width ;
    double  ladder_dphi ;
  };    

  std::vector<SET_Layer> _SET_Layers;

  struct extended_reconstruction_parameters {
    double sensor_length_mm;
    double strip_width_mm;
    double strip_length_mm;
    double strip_pitch_mm;
    double strip_angle_deg;
  };
  
  extended_reconstruction_parameters _e_r_p;
  
  
  G4double _sensitive_thickness;
  G4double _support_thickness;
  G4double _sensor_length;

  G4Material* _sensitiveMat;
  G4Material* _supportMat;

  TRKSD02 * _theSETSD;
};

#endif

