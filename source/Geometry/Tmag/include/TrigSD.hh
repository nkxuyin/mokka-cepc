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
// $Id: TrigSD.hh,v 1.1 2007/02/09 15:57:48 predrag Exp $
// $Name: mokka-07-00 $
//

#ifndef TrigSD_h
#define TrigSD_h 1

#include "CalHit.hh"
#include "VSensitiveDetector.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"

class TrigSD : public VSensitiveDetector
{
  
public:
  TrigSD(G4String SDname);
  virtual ~TrigSD();
  
  void Initialize(G4HCofThisEvent*HCE);
  G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
 
  void EndOfEvent(G4HCofThisEvent*HCE);
  void clear();
  void DrawAll();
  void PrintAll();

  HitsCollection *CalCollection; 
  G4int SDPiece;
  G4int HCID;
};

#endif

