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
// $Id: TRKSiSD00.hh,v 1.1 2008/10/06 12:58:35 musat Exp $
// $Name: mokka-07-00 $
//

#ifndef TRKSiSD00_h
#define TRKSiSD00_h 1
#include "TRKHit.hh"

#include "VSensitiveDetector.hh"
#include "G4Step.hh"

class TRKSiSD00 : public VSensitiveDetector
{
  
public:
  TRKSiSD00(G4String TRKSiSD00name, 
	  G4double Threshold,
	  G4double thePrimaryTPCCut=0);
  virtual ~TRKSiSD00(){}
  
  void Initialize(G4HCofThisEvent*HCE);
  G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
  void EndOfEvent(G4HCofThisEvent*HCE);
  void DrawAll();
  void PrintAll();

  // Basic reload hits from Ascii Files
  void LoadEvent(FILE* theSubDetectorEventHitsFileInput);

private:
  void StartNewHit(G4int aLayerNumber,
		   G4ThreeVector theEntryPoint,
		   G4ThreeVector theEntryMomentum,
		   G4int thePDG,
		   G4int theTrackPID);
  
  void UpdateHit(G4Step *aStep);

  void DumpHit(G4Step *aStep);

  void Clear();

  G4double Threshold;
  G4double PrimaryTPCCut;
  G4int HCID;
  G4int currentCylinder;
  G4int currentPID;
  G4int currentPDG;

  G4ThreeVector EntryPoint;
  G4ThreeVector ExitPoint;
  G4ThreeVector EntryMomentum;
  G4ThreeVector ExitMomentum;
  G4double DepositedEnergy;
  G4double HitTime ;
  G4double StepLength;

  G4bool detailedHitsStoring;
  G4bool defaultHitsStoring;

  TRKHitsCollection *CalCollection;

private:
};

#endif

