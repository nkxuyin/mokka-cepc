// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: TPC06.hh,v 1.4 2007/11/30 14:20:39 steve Exp $
// $Name: mokka-07-00 $

#ifndef TPC06_hh
#define TPC06_hh 1

class G4LogicalVolume;
class Database;
class TRKSD00;

#include "VSubDetectorDriver.hh"
#include "G4Material.hh"
class TPC06 : public VSubDetectorDriver
{
public:
  TPC06(void): VSubDetectorDriver("tpc06", "tpc") {}
  ~TPC06(void) {}

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
  
  TRKSD00* theTPCSD;
  TRKSD00* theFCHSD;

};

#endif
