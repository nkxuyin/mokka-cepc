// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: TPCSD02.hh,v 1.3 2007/12/05 19:59:51 steve Exp $
// $Name: mokka-07-00 $

#ifndef TPCSD02_hh
#define TPCSD02_hh 1


#include "VSensitiveDetector.hh"
#include "TRKHit.hh"

class TPCSD02: public VSensitiveDetector
{
public:
  TPCSD02(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy);
  ~TPCSD02(void) {}
  
  void Initialize(G4HCofThisEvent *eventHC);
  G4bool ProcessHits(G4Step *step, G4TouchableHistory *history);
  void EndOfEvent(G4HCofThisEvent *eventHC);

  void LoadEvent(FILE *eventFile);

private:
  G4double fThresholdEnergyDeposit;
  G4double fThresholdKineticEnergy;
  TRKHitsCollection *fHitCollection;
  TRKHitsCollection *fSpaceHitCollection;
  G4int fHCID;
  G4int fSpaceHitCollectionID;

  G4ThreeVector CrossingOfPadRingCentre;
  G4ThreeVector MomentumAtPadRingCentre;
  G4double dEInPadRow;
  G4double globalTimeAtPadRingCentre;
  G4double pathLengthInPadRow;
 

  
};

#endif
