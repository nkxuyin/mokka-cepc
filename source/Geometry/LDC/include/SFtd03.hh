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
// $Id: SFtd03.hh,v 1.2 2008/05/06 16:21:40 steve Exp $
// $Name: mokka-07-00 $
//
#ifndef SFtd03_h
#define SFtd03_h 1

class G4LogicalVolume;
class Database;
class TRKSD00;

#include "VSubDetectorDriver.hh"
#include "G4Material.hh"

class SFtd03 : public VSubDetectorDriver
{
public:
  SFtd03(void) : VSubDetectorDriver("SFtd03","ftd")  {} 

  ~SFtd03(void) {};
  
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
  G4double Disks_Si_thickness_2;
  G4double outer_cylinder_total_thickness,cables_thickness,cable_shield_thickness;
  G4double ZStartOuterCylinder,ZStopOuterCylinder;
  G4double ZStartInnerCylinder,ZStopInnerCylinder;
  G4Material *SiMat;
  G4Material *KaptonMat;
  G4Material *CuMat;
  G4int LastHeavyLayer;

  TRKSD00 *theFTDSD;
};

#endif


