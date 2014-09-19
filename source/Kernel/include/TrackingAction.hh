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
// $Id: TrackingAction.hh,v 1.3 2003/08/12 09:41:58 mora Exp $
// $Name: mokka-07-00 $
//
#ifndef TrackingAction_h
#define TrackingAction_h 1

#include "G4UserTrackingAction.hh"
#include "globals.hh"

class G4Event;

class TrackingAction : public G4UserTrackingAction
{
public:
  TrackingAction();
  ~TrackingAction(){}

public:
  void PreUserTrackingAction(const G4Track* aTrack);
  void PostUserTrackingAction(const G4Track* );
private:
};

#endif

    
