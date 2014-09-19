/*
 * SIT Self Scaling Driver for Mokka 
 *
 * SSit03.hh - driver class header
 * 
 */

#ifndef SSIT03_hh
#define SSIT03_hh 1

class G4LogicalVolume;
class Database;
class TRKSD00;

#include "VSubDetectorDriver.hh"
#include "G4Material.hh"

class SSit03 : public VSubDetectorDriver
{
 public:
  
  SSit03(void) : VSubDetectorDriver("SSit03","sit")  {} 
  ~SSit03(void) {};
    

  G4bool ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog);

#ifdef MOKKA_GEAR    
  void GearSetup();
#endif

private:
  Database* db;

  std::vector<double> inner_radiusVec ;
  std::vector<double> half_zVec ;
  std::vector<double>  support_radiusVec;
  std::vector<double>  support_half_zVec;
  G4double sensitive_thickness;
  G4double support_thickness;
  G4Material *SITMat;
  G4Material *SupportMat;

  G4String  support_structure_material ;

  TRKSD00 * theSITSD;
};

#endif

