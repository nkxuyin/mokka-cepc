/*
 * SET Self Scaling Driver for Mokka 
 *
 * SSet02.hh - driver class header
 * 
 */

#ifndef SSET02_hh
#define SSET02_hh 1

class Database;
class TRKSD00;

#include "VSubDetectorDriver.hh"
#include "G4Material.hh"

class SSet02 : public VSubDetectorDriver
{
 public:
  
  SSet02(void) : VSubDetectorDriver("SSet02","set")  {};
  ~SSet02(void) {};
    
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
  G4Material *SETMat;
  G4Material *SupportMat;

  G4String  support_structure_material ;

  TRKSD00 * theSETSD;
};

#endif

