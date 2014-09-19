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
// $Id: HodoscopeSD00.hh,v 1.2 2006/01/31 14:49:20 musat Exp $
// $Name: mokka-07-00 $
//

#ifndef HodoscopeSD00_h
#define HodoscopeSD00_h 1

#include "VSensitiveDetector.hh"
#include "G4Step.hh"
#include "CalHit.hh"

class HodoscopeSD00 : public VSensitiveDetector
{
  
public:
  HodoscopeSD00(G4String HodoscopeSD00name);
  virtual ~HodoscopeSD00() {}

  void Initialize(G4HCofThisEvent*HCE);
  G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
  void EndOfEvent(G4HCofThisEvent*HCE);
  void clear();
  void DrawAll();
  void PrintAll();
  
  G4int HCID;
  HitsCollection *CalCollection;


private:
};

#endif

