/*
 * ETD Self-Scaling Driver for Mokka 
 *
 * SEtd02.hh - driver class header
 * 
 */

#ifndef SETD02_hh
#define SETD02_hh 1

class Database;
class TRKSD00;

#include "VSubDetectorDriver.hh"
#include "G4Material.hh"

class SEtd02 : public VSubDetectorDriver
{
 public:
  
  SEtd02(void) : VSubDetectorDriver("SEtd02", "etd")  {};
  ~SEtd02(void) {};
    
G4bool ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog);

#ifdef MOKKA_GEAR    
  void GearSetup();
#endif

private:
  Database* db;
  std::vector<double> inner_radiusVec ;
  std::vector<double> outer_radiusVec ;
  std::vector<double> zVec ;
  std::vector<double> dzVec ;
  G4double sensitive_thickness;
  G4double support_thickness;
  G4Material *ETDMat;
  G4Material *SupportMat;
  TRKSD00 *theETDSD;

};

#endif

