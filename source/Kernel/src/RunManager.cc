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
// $Id: RunManager.cc,v 1.3 2008/01/29 15:28:04 musat Exp $
// $Name: mokka-07-00 $
//

#include "RunManager.hh"
#include "Control.hh"

#include "G4Run.hh"
#include "G4StateManager.hh"
#include "G4VPersistencyManager.hh"
#include "G4UserRunAction.hh"

#include "G4VVisManager.hh"
#include "G4VisManager.hh"
#include "G4VViewer.hh"

void RunManager::RunInitialization()
{
  G4RunManager::RunInitialization();
  G4VVisManager * fpVisMan = G4VisManager::GetConcreteInstance();
  G4VViewer * viewer = 0;
  if(fpVisMan) viewer = ((G4VisManager*)fpVisMan)->GetCurrentViewer();

  if(!viewer && LastEvt) delete LastEvt;
//  if(LastEvt) delete LastEvt;
}

void 
RunManager::RunTermination()
{
  if(previousEvents->size()!=1)
    Control::Abort("Mokka, PANIC: previousEvents->size()!=1 in RunManager::RunTermination()!!!",MOKKA_OTHER_ERRORS);
  LastEvt=(*previousEvents)[0];
  previousEvents->pop_back();
  G4RunManager::RunTermination();
}

void 
RunManager::BeamOn(G4int n_event,const char* macroFile,G4int n_select)
{
  if(Control::VISUMODE)
    G4cout << "Sorry, in visualisation mode Mokka doesn't do simulations at all!!!!!" << G4endl;
  else
    G4cout << "###$$$ RunManager::BeamOn:   Random Seed Value = " << Control::RandomSeed << " $$$###" << std::endl;
    G4RunManager::BeamOn(n_event,macroFile,n_select);
}
