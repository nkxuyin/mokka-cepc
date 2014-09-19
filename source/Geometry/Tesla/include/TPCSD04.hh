// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: TPCSD04.hh,v 1.1 2009/05/13 08:22:57 steve Exp $
// $Name: mokka-07-00 $

#ifndef TPCSD04_hh
#define TPCSD04_hh 1


#include "VSensitiveDetector.hh"
#include "TRKHit.hh"

class TPCSD04: public VSensitiveDetector
{
public:
  TPCSD04(G4String name, G4double thresholdEnergyDeposit);
  ~TPCSD04(void) {}
  
  void Initialize(G4HCofThisEvent *eventHC);
  G4bool ProcessHits(G4Step *step, G4TouchableHistory *history);
  void EndOfEvent(G4HCofThisEvent *eventHC);
  
  void LoadEvent(FILE *eventFile);
  
  /// helper function to avoid code duplication, writes a low Pt hit to the collection
  void DepositLowPtHit();
  
  /// helper function to avoid code duplication, resets all cumulative variables
  void ResetCumulativeVariables();
  
  /// helper function to avoid code duplication,
  /// adds energy, track length and momentum of a low pt step to the cumulative variables
  void CumulateLowPtStep(G4Step *step);
  
  
  
  
private:
  G4double fThresholdEnergyDeposit;
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
  G4double CumulativePathLength;
  G4double CumulativeEnergyDeposit;
  G4ThreeVector CumulativeMeanPosition; 
  G4ThreeVector CumulativeMeanMomentum; 
  G4int CumulativeNumSteps;
  
  G4ThreeVector PreviousPostStepPosition; //< the end point of the previous step
  G4int CurrentPDGEncoding; //< the PDG encoding of the particle causing the cumulative energy deposit
  G4int CurrentTrackID; //< the TrackID of the particle causing the cumulative energy deposit
  G4double CurrentGlobalTime; ///< the global time of the track causing the cumulative energy deposit
  G4int CurrentCopyNumber; ///< copy number of the preStepPoint's TouchableHandle for the cumulative energy deposit
  
  
};

#endif
