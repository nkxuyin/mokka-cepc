// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: TPC07.hh,v 1.1 2009/05/13 08:22:57 steve Exp $
// $Name: mokka-07-00 $

#ifndef TPC07_hh
#define TPC07_hh 1

class G4LogicalVolume;
class Database;

#include "VSubDetectorDriver.hh"
#include "G4Material.hh"
class TPC07 : public VSubDetectorDriver
{
public:
  TPC07(void): VSubDetectorDriver("tpc07", "tpc") {}
  ~TPC07(void) {}

  G4bool ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog);

#ifdef MOKKA_GEAR
  void GearSetup();
#endif

private:
  G4double skinThickness;
  G4double padHeight;
  G4double padWidth;
  G4int numberPadRows;
  G4double rInner;
  G4double rOuter;
  G4double dzSensitive;
  G4double dzEndplate;
  G4double drInnerWall;
  G4double drOuterWall;
  G4double rInnerSensitive;
  G4double rOuterSensitive;
  G4double TPCMaxStepLength;
  G4double TPCWallProperties_RadLen;
  G4double TPCWallProperties_dEdx;
  G4double TPCGasProperties_RadLen;
  G4double TPCGasProperties_dEdx;
  G4double Ar_IonPotential;
  G4Material *materialAl;
  G4Material *materialGas;
  G4Material *materialAir;
  G4Material *materialCu;
  G4Material * materialMylar; 
  G4Material *materialG10; 
  

};

#endif
