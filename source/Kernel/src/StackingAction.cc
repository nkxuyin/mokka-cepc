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
// $Id: StackingAction.cc,v 1.1 2004/07/15 13:35:46 mora Exp $
// GEANT4 tag $Name: mokka-07-00 $
//

#include "StackingAction.hh"
#include "G4Track.hh"
#include "G4DecayProducts.hh"
#include "G4ios.hh"

StackingAction::StackingAction()
{;}

StackingAction::~StackingAction()
{;}

G4ClassificationOfNewTrack StackingAction::ClassifyNewTrack
(const G4Track* aTrack)
{
  G4ClassificationOfNewTrack classification = fUrgent;
  if(aTrack->GetTrackStatus()==fSuspend) classification = fWaiting;

  return classification;
}

// void StackingAction::NewStage()
// {
// //   G4cout << "StackingAction::NewStage"<< G4endl;
// //   char a;
// //   G4cin >> a;
// }

// void StackingAction::PrepareNewEvent()
// {;}


