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
// $Id: EventAction.cc,v 1.4 2005/02/25 14:13:27 roman Exp $
// $Name: mokka-07-00 $
//

#include "Control.hh"
#include "EventAction.hh"

#include "G4ios.hh"
#include <errno.h>
#include <stdio.h>
#include "PluginManager.hh"


EventAction::EventAction()
{
}

EventAction::~EventAction()
{;}

void EventAction::BeginOfEventAction(const G4Event* anEvent)
{


  // Just bypass to Control
  Control::GetControl()->BeginOfEventAction(anEvent);
  PluginManager::getInstance()->BeginOfEventAction( anEvent ) ;
}

void EventAction::EndOfEventAction(const G4Event* evt)
{    

  //CRP Interchanged PluginManager <-> GetControl ...
  PluginManager::getInstance()->EndOfEventAction( evt ) ;
  // Just bypass to Control
  Control::GetControl()->EndOfEventAction(evt);

}


