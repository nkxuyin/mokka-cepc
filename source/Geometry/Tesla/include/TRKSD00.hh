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
// $Id: TRKSD00.hh,v 1.4 2006/03/15 15:50:48 mora Exp $
// $Name: mokka-07-00 $
//

#ifndef TRKSD00_h
#define TRKSD00_h 1
#include "TRKHit.hh"

#include "VSensitiveDetector.hh"
#include "G4Step.hh"

class TRKSD00 : public VSensitiveDetector
{
  
public:
  TRKSD00(G4String TRKSD00name, 
	  G4double Threshold,
	  G4double thePrimaryTPCCut=0);
  virtual ~TRKSD00(){}
  
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
		   G4int theSecondaryPID);
  
  void UpdateHit(G4Step *aStep);

  void DumpHit(G4Step *aStep);

  void Clear();

  G4double Threshold;
  G4double PrimaryTPCCut;
  G4int HCID;
  G4int currentCylinder;
  G4int currentPID;
  G4int currentPDG;
  G4int currentSecondaryPID;

  G4ThreeVector EntryPoint;
  G4ThreeVector ExitPoint;
  G4ThreeVector EntryMomentum;
  G4ThreeVector ExitMomentum;
  G4double DepositedEnergy;
  G4double HitTime ;
  G4double StepLength;

  TRKHitsCollection *CalCollection;

private:
};

#endif

