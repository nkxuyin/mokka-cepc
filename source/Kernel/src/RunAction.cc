#include "Control.hh"
#include "RunAction.hh"

#include "PluginManager.hh"


RunAction::RunAction(){ 
}


void RunAction::BeginOfRunAction(const G4Run* aRun) {

   Control::RunAborted = false;
   PluginManager::getInstance()->BeginOfRunAction( aRun ) ;

}


void RunAction::EndOfRunAction(const G4Run* aRun) {

   PluginManager::getInstance()->EndOfRunAction( aRun ) ;

}

