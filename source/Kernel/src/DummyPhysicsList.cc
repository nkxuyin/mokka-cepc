
#include "DummyPhysicsList.hh"
#include "Control.hh"

#include "globals.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleWithCuts.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessVector.hh"
#include "G4ParticleTypes.hh"
#include "G4ParticleTable.hh"

#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4ios.hh"
#include <iomanip>   

#include "GeneralPhysics.hh"

DummyPhysicsList::DummyPhysicsList(G4int mode):  G4VModularPhysicsList()
{
  defaultCutValue = Control::RangeCut;

  // fg: document the usage of LHEP in the logfile
  G4cout << "You are using the dummy simulation engine: Mokka DummyPhysicsList" << G4endl << G4endl ;

  // General Physics
  RegisterPhysics( new GeneralPhysics("general") );

  // mode != 0 means just for visualisation
  if(mode!=0) 
    return;
  else
    Control::Abort("DummyPhysicsList cannot be used for physics, just for the CGA API.",MOKKA_OTHER_ERRORS);

}

DummyPhysicsList::~DummyPhysicsList()
{
}

void DummyPhysicsList::SetCuts()
{
  if (verboseLevel >1){
    G4cout << "DummyPhysicsList::SetCuts:";
  }  
  //  " G4VUserPhysicsList::SetCutsWithDefault" method sets 
  //   the default cut value for all particle types 

}

