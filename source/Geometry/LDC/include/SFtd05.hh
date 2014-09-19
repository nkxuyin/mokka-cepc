/*
 * FTD Self-Scaling Driver for Mokka 
 *
 * SFtd05.hh - driver class header
 * 
 */

#ifndef SFtd05_h
#define SFtd05_h 1

class G4LogicalVolume;
class Database;
class TRKSD00;
class G4VisAttributes;

#include "G4RotationMatrix.hh"
#include "VSubDetectorDriver.hh"
#include "G4Material.hh"

class SFtd05 : public VSubDetectorDriver
{
public:
  SFtd05(void) : VSubDetectorDriver("SFtd05","ftd")  {} 
  ~SFtd05(void) {};
  
  G4bool ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog);
  
#ifdef MOKKA_GEAR    
  void GearSetup();
#endif

private:
  
  Database* db;
  std::vector<double> Disks_Si_thicknessVec;
  std::vector<double> Disks_Support_thicknessVec;
  std::vector<double> z_positionVec;
  std::vector<double> inner_radiusVec;
  std::vector<double> outer_radiusVec;
  G4double Disks_Si_thickness;
  G4double Disks_Support_thickness;
  G4double outer_cylinder_total_thickness;
  G4double cables_thickness;
  G4double cable_shield_thickness;
  G4double ZStartOuterCylinder,ZStopOuterCylinder;
  G4double ZStartInnerCylinder,ZStopInnerCylinder;
  G4Material *SiMat;
  G4Material *KaptonMat;
  G4Material *CuMat;
  G4int LastHeavyLayer;

  TRKSD00 *theFTDSD;
};

#endif


