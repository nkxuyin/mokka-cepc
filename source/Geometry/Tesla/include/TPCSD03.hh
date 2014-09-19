// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: TPCSD03.hh,v 1.1 2009/05/13 08:22:57 steve Exp $
// $Name: mokka-07-00 $

#ifndef TPCSD03_hh
#define TPCSD03_hh 1


#include "VSensitiveDetector.hh"
#include "TRKHit.hh"

class TPCSD03: public VSensitiveDetector
{
public:
  TPCSD03(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy);
  ~TPCSD03(void) {}
  
  void Initialize(G4HCofThisEvent *eventHC);
  G4bool ProcessHits(G4Step *step, G4TouchableHistory *history);
  void EndOfEvent(G4HCofThisEvent *eventHC);

  void LoadEvent(FILE *eventFile);

private:
  G4double fThresholdEnergyDeposit;
  G4double fThresholdKineticEnergy;
  TRKHitsCollection *fHitCollection;
  TRKHitsCollection *fSpaceHitCollection;
  TRKHitsCollection *fLowPtHitCollection;
  G4int fHCID;
  G4int fSpaceHitCollectionID;
  G4int fLowPtHitCollectionID;

  G4ThreeVector CrossingOfPadRingCentre;
  G4ThreeVector MomentumAtPadRingCentre;
  G4double dEInPadRow;
  G4double globalTimeAtPadRingCentre;
  G4double pathLengthInPadRow;
  G4double CumulitivePathLength;
  G4double CumulitiveEnergyDeposit;
  G4ThreeVector CumulitiveMeanPosition; 
  G4ThreeVector CumulitiveMeanMomentum; 
  G4int CumulitiveNumSteps;
  
};

#endif
