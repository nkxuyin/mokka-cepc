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
// $Id: SteppingAction.hh,v 1.3 2004/04/19 13:45:20 mora Exp $
// $Name: mokka-07-00 $
//
#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4UserSteppingAction.hh"
#include "globals.hh"

class TrackSummary;
class SteppingActionMessenger;

class SteppingAction : public G4UserSteppingAction
{
public:
  SteppingAction();
  virtual ~SteppingAction();
  
  virtual void UserSteppingAction(const G4Step* aStep);

private:
  G4bool drawFlag;
  G4int MaxStepNumber;

  SteppingActionMessenger * theSteppingActionMessenger;

  public:
  inline void SetDrawFlag(G4bool val)
    { drawFlag = val; };
};

#endif
