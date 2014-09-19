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
// $Id: StackingAction.hh,v 1.1 2004/07/15 13:35:45 mora Exp $
// $Name: mokka-07-00 $
//

#ifndef StackingAction_h
#define StackingAction_h 1

#include "globals.hh"
#include "G4UserStackingAction.hh"

  class StackingAction : public G4UserStackingAction
{
  public:
      StackingAction();
      virtual ~StackingAction();

  public:
//--------------------------------------------------
// vitual methods implemented by Mokka
//--------------------------------------------------
//
  G4ClassificationOfNewTrack 
  ClassifyNewTrack(const G4Track* aTrack);
  //
  //    Reply G4ClassificationOfNewTrack determined by the
  //  newly coming G4Track.
  //
  //    enum G4ClassificationOfNewTrack
  //    {
  //      fUrgent,    // put into the urgent stack
  //      fWaiting,   // put into the waiting stack
  //      fPostpone,  // postpone to the next event
  //      fKill       // kill without stacking
  //    };
  //
  //    The parent_ID of the track indicates the origin of it.
  //                
  //    G4int parent_ID = aTrack->get_parentID();
  //   
  //      parent_ID = 0 : primary particle
  //                > 0 : secondary particle
  //                < 0 : postponed from the previous event
  //
  //---------------------------------------------------------------
  //
  // virtual void NewStage();
//
//    This method is called by G4StackManager when the urgentStack
//  becomes empty and contents in the waitingStack are transtered
//  to the urgentStack.
//    Note that this method is not called at the begining of each
//  event, but "PrepareNewEvent" is called.
//
//    In case re-classification of the stacked tracks is needed,
//  use the following method to request to G4StackManager.
//
//    stackManager->ReClassify();
//
//  All of the stacked tracks in the waitingStack will be re-classified 
//  by "ClassifyNewTrack" method.
//    To abort current event, use the following method.
//
//    stackManager->clear();
//
//  Note that this way is valid and safe only for the case it is called
//  from this user class. The more global way of event abortion is
//
//    G4UImanager * UImanager = G4UImanager::GetUIpointer();
//    UImanager->ApplyCommand("/event/abort");
//
//---------------------------------------------------------------
//
//      virtual void PrepareNewEvent();
//
//    This method is called by G4StackManager at the begining of
//  each event.
//    Be careful that the urgentStack and the waitingStack of 
//  G4StackManager are empty at this moment, because this method
//  is called before accepting primary particles. Also, note that
//  the postponeStack of G4StackManager may have some postponed
//  tracks.
//
//---------------------------------------------------------------

};

#endif

