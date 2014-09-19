// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: TPCSD01.hh,v 1.2 2006/03/13 17:59:21 adrian Exp $
// $Name: mokka-07-00 $

#ifndef TPCSD01_hh
#define TPCSD01_hh 1

#include "VSensitiveDetector.hh"
#include "TRKHit.hh"

class TPCSD01: public VSensitiveDetector
{
public:
  TPCSD01(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy);
  ~TPCSD01(void) {}
  
  void Initialize(G4HCofThisEvent *eventHC);
  G4bool ProcessHits(G4Step *step, G4TouchableHistory *history);
  void EndOfEvent(G4HCofThisEvent *eventHC);

  void LoadEvent(FILE *eventFile);

private:
  G4double fThresholdEnergyDeposit;
  G4double fThresholdKineticEnergy;
  TRKHitsCollection *fHitCollection;
  G4int fHCID;
};

#endif
