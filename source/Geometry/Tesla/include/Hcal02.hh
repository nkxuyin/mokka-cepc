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
// $Id: Hcal02.hh,v 1.3 2007/12/20 11:37:21 kristian Exp $
// $Name: mokka-07-00 $
//

#ifndef Hcal02_h
#define Hcal02_h 1

class G4LogicalVolume;
class Database;
class G4VSolid;

class SD;
class HECSD;

#include "VSubDetectorDriver.hh"

class Hcal02 : public VSubDetectorDriver
{
public:

  Hcal02() : VSubDetectorDriver("hcal02","hcal"),
	     theBarrilRegSD(0),theBarrilEndSD(0),
	     theENDCAPEndSD(0)
  {}
  ~Hcal02();
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);

  // To reload hits for visualisation
  void LoadEvent(FILE* theSubDetectorEventHitsFileInput);
  
private:
  G4LogicalVolume *EnvLogHcalModuleBarrel;
  G4LogicalVolume *EnvLogHcalModuleEndCap;
  Database* db;

  // Barrel
  void Barrel(G4LogicalVolume*);
  void BarrelRegularModules(G4LogicalVolume*);
  void BarrelEndModules(G4LogicalVolume*);
  void BarrelRegularChambers(G4LogicalVolume*, G4double chambers_y_off_correction);
  void BarrelEndChambers(G4LogicalVolume*, G4double chambers_y_off_correction);
  // EndCaps
  void Endcaps(G4LogicalVolume*);
  void EndcapChambers(G4LogicalVolume*);

  SD* theBarrilRegSD;
  SD* theBarrilEndSD;
  HECSD* theENDCAPEndSD;

#ifdef MOKKA_GEAR
  // MokkaGear
    
  struct helpParameters{
    G4double innerRadius ;
    G4double outerRadius ;
    G4double zMax ;
    G4double phi0 ;
    std::vector<double> layerPos ;
    std::vector<double> layerPos2 ;
    std::vector<double> sensThickness ;
    std::vector<double> gapThickness ;
    G4int count ;
    G4double leastZ ;
    G4double mostZ ;
  };
 
  helpParameters helpBarrel;
  helpParameters helpEndcap;

  // parameters

  G4int    intParamEndModuleType ;
  G4double dblParamHalfZ ;
  G4double dblParamLateralStructureThickness ;
  G4double dblParamModulesGap ;
  G4double dblParamStavesGap ;
  G4double dblParamBackPlateThickness ;
  G4double dblParamBarrelMostZ ;
  G4double dblParamDigiTileSize ;

#endif
};

#endif


