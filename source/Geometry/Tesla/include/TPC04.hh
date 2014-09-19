// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: TPC04.hh,v 1.3 2007/08/13 08:12:42 kristian Exp $
// $Name: mokka-07-00 $

#ifndef TPC04_hh
#define TPC04_hh 1
class G4LogicalVolume;
class Database;
class TRKSD00;

#include "G4Material.hh"
#include "VSubDetectorDriver.hh"

class TPC04: public VSubDetectorDriver
{
public:
  TPC04(void): VSubDetectorDriver("tpc04", "tpc") {}
  ~TPC04(void) {}

  G4bool ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog);

#ifdef MOKKA_GEAR
 void GearSetup();
#endif

private:
  G4double rInner;
  G4double rOuter;
  G4double dzSensitive;
  G4double dzEndplate;
  G4double drInnerWall;
  G4double drOuterWall;
  G4double rInnerSensitive;
  G4double rOuterSensitive;
  G4double TPCWallProperties_RadLen;
  G4double TPCWallProperties_dEdx;
  G4double TPCGasProperties_RadLen;
  G4double TPCGasProperties_dEdx;
  G4double Ar_IonPotential;
  G4Material *materialAl;
  G4Material *materialGas;
  G4Material *materialAir;
  G4Material *materialCu;
 
  TRKSD00* theTPCSD;
  TRKSD00* theFCHSD;
};

#endif
