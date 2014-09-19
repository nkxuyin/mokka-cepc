// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: TPC03.hh,v 1.3 2007/08/13 08:12:42 kristian Exp $
// $Name: mokka-07-00 $

#ifndef TPC03_hh
#define TPC03_hh 1


class G4LogicalVolume;
class Database;
class TRKSD00;

#include "VSubDetectorDriver.hh"
#include "G4Material.hh"


class TPC03: public VSubDetectorDriver
{
public:
  TPC03(void): VSubDetectorDriver("tpc03", "tpc"), theTPCSD(0), theFCHSD(0) {}
  ~TPC03(void) {}
  
  G4bool construct(const G4String &dbName, G4LogicalVolume *worldLog);
 
#ifdef MOKKA_GEAR
  void GearSetup();
#endif

private:
 G4double inner_radius;
 G4double outer_radius;
 G4double z_half_length;
 G4double endplate_thickness;
 G4double inner_wall_thickness;
 G4double outer_wall_thickness;
 G4double inner_sensitive_radius;
 G4double outer_sensitive_radius;
 G4double layer_thickness;
 G4int number_of_layers;
  G4double TPCWallProperties_RadLen;
  G4double TPCWallProperties_dEdxArray[1000];
  G4double TPCWallProperties_dEdx;
  G4double TPCGasProperties_RadLen;
  G4double TPCGasProperties_dEdxArray[1000];
  G4double TPCGasProperties_dEdx;
  G4double Ar_IonPotential;
  G4Material *materialAir; 
  G4Material *materialCu; 
  G4Material *materialKp; 
  G4Material *materialAl; 
  G4Material *materialSi; 
  G4Material *materialGas; 

  
private:
  TRKSD00 *theTPCSD;
  TRKSD00 *theFCHSD;
};

#endif
