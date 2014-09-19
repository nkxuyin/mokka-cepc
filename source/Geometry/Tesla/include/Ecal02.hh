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
// $Id: Ecal02.hh,v 1.3 2006/05/23 15:12:40 frank Exp $
// $Name: mokka-07-00 $
//
#ifndef Ecal02_h
#define Ecal02_h 1

class G4LogicalVolume;
class Database;
class G4VSolid;

class SD;
class ECSD;

#include "VSubDetectorDriver.hh"

class Ecal02 : public VSubDetectorDriver
{
public:
  Ecal02() : VSubDetectorDriver("ecal02","ecal"), 
	     theBarrelSD(0),theEndCapSD(0),
	     theMaxStepAllowed(DBL_MAX)
  {}

  ~Ecal02();
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);

  // To reload hits for visualisation
  void LoadEvent(FILE* theSubDetectorEventHitsFileInput);

private:

  // Barrel
  void BarrelStandardModule(G4LogicalVolume*);
  void BarrelWPlate(G4LogicalVolume*);
  void BarrelAlveolusModule(G4LogicalVolume*);
  // EndCaps
  void EndcapStandardModule(G4LogicalVolume*);
  void EndcapWPlate(G4LogicalVolume*);
  void EndcapAlveolusModule(G4LogicalVolume*);
  void EndcapAlveolusPads(G4LogicalVolume*);

  G4VSolid* BuildECShape(G4double L,
			 G4double dim_x,
			 G4double dim_y,
			 G4double dim_z);

  //#####################################################
  // NO MORE AL PLATES FOR THE ECAL ENDCAPS WITH ecal02
  // ####################################################
  //void EndcapAlPlate(G4LogicalVolume*);
  G4Material * RadiatorMaterial;

  G4LogicalVolume *EnvLogEcalModuleBarrel;
  G4LogicalVolume *EnvLogEcalModuleEndCap;
  Database* db;
  SD* theBarrelSD;
  ECSD* theEndCapSD;
  G4double theMaxStepAllowed;

#ifdef MOKKA_GEAR
  // MokkaGear
  
  struct helpParameters{
    G4double innerRadius;
    G4double outerRadius;
    G4double zMax;
    G4double phi0;
    std::vector<double> layerPos;
    std::vector<double> radiThickness;
    G4int count;
    G4double leastZ;
    G4double mostZ;
  };
  
  helpParameters helpBarrel;
  helpParameters helpEndcap;
  
#endif
  
};

#endif


