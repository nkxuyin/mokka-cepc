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
// $Id: FTD01.hh,v 1.4 2008/04/25 07:42:47 steve Exp $
// $Name: mokka-07-00 $
//
#ifndef FTD01_h
#define FTD01_h 1

class G4LogicalVolume;
class Database;
class TRKSD00;

#include "Control.hh"
#include "VSubDetectorDriver.hh"
#include "G4Material.hh"

class FTD01 : public VSubDetectorDriver
{
public:
  FTD01() : VSubDetectorDriver("ftd01","ftd"), 
	    db(0),theFTDSD(0)
  {}
  ~FTD01();
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);
#ifdef MOKKA_GEAR    
  void GearSetup();
#endif

private:
  Database* db;
    std::vector<double> z_positionVec;
    std::vector<double> inner_radiusVec;
    std::vector<double> outer_radiusVec;
  G4double Disks_Si_thickness;
  G4double Disks_Si_thickness_2;
  G4double gear_Disks_Si_thickness;
  G4double gear_Disks_Si_thickness_2;
  G4double inner_support_length;
  G4double inner_support_thickness;
  G4double outer_support_thickness; 
  G4double  outer_support_length,outer_cylinder_total_thichness,cables_thichness;
  G4double ZStartOuterCylinder,ZstopOuterCylinder;
  G4double ZStartInnerCylinder,ZstopInnerCylinder;
  G4Material *SiMat;
  G4Material *Si872Mat;
  G4Material *KaptonMat;
  G4Material *CuMat;
  G4int LastHeavyLayer;

  TRKSD00 *theFTDSD;
};

#endif


