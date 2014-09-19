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
// $Id: HcalWMod.hh,v 1.1 2003/07/18 09:05:10 musat Exp $
// $Name: mokka-07-00 $
//
#ifndef HcalWMod_h
#define HcalWMod_h 1

class G4LogicalVolume;
class Database;
class G4VSolid;

class SD;
class HECSD;

#include "VSubDetectorDriver.hh"

class HcalWMod : public VSubDetectorDriver
{
public:

  HcalWMod() : VSubDetectorDriver("HcalWMod","hcal"),
	     theBarrilRegSD(0),theBarrilEndSD(0),
	     theENDCAPEndSD(0)
  {}

  ~HcalWMod();
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);

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
};

#endif


