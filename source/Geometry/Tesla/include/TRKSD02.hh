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
//

#ifndef TRKSD02_h
#define TRKSD02_h 1
#include "TRKHit.hh"

#include "VSensitiveDetector.hh"
#include "G4Step.hh"

class TRKSD02 : public VSensitiveDetector
{
  
public:
  TRKSD02(G4String TRKSD02name, 
	  G4double Threshold,
	  G4double KineticEnergyCut = 0.0);
  virtual ~TRKSD02(){}
  
  void Initialize(G4HCofThisEvent*HCE);
  G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
  void EndOfEvent(G4HCofThisEvent*HCE);
  void DrawAll();
  void PrintAll();

  // Basic reload hits from Ascii Files
  void LoadEvent(FILE* theSubDetectorEventHitsFileInput);

private:
  void StartNewHit(G4int copyNumber,G4int currentCellID0,
		   G4ThreeVector theEntryPoint,
		   G4ThreeVector theEntryMomentum,
       G4int thePDG,
		   G4int theTrackID);
  
  void UpdateHit(G4Step *aStep);

  void DumpHit(G4Step *aStep);

  void Clear();

  G4double Threshold;
  G4double KineticEnergyCut;
  G4int HCID;
  G4int currentCopyNumber;
  G4int currentCellID0;
  G4int currentPID;
  G4int currentPDG;


  G4ThreeVector EntryPoint;
  G4ThreeVector ExitPoint;
  G4ThreeVector EntryMomentum;
  G4ThreeVector ExitMomentum;
  G4double DepositedEnergy;
  G4double HitTime ;
  G4double StepLength;


  TRKHitsCollection *HitCollection;

private:
};

#endif

