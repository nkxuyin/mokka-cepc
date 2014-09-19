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
// $Id: TPCSD00.hh,v 1.1 2003/07/18 09:05:15 musat Exp $
// $Name: mokka-07-00 $
//

#ifndef TPCSD00_h
#define TPCSD00_h 1

#include "TPCHit.hh"

#include "VSensitiveDetector.hh"
#include "G4Step.hh"

class TPCSD00 : public VSensitiveDetector
{
  
public:
  TPCSD00(G4String TPCSD00name);
  virtual ~TPCSD00(){}
  
  void Initialize(G4HCofThisEvent*HCE);
  G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
  void EndOfEvent(G4HCofThisEvent*HCE);
  void clear();
  void DrawAll();
  void PrintAll();

  // Basic reload hits from Ascii Files
  void LoadEvent(FILE* theSubDetectorEventHitsFileInput);

  G4int HCID;
  G4int lastCylinder;
  G4int lastPID;
  TPCHitsCollection *CalCollection;

private:
};

#endif

