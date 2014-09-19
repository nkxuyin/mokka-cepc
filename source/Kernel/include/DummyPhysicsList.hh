#ifndef DummyPhysicsList_h
#define DummyPhysicsList_h 1

#include "G4VModularPhysicsList.hh"
#include "globals.hh"

#define kNuCut  5*m

class DummyPhysicsList: public G4VModularPhysicsList
{
public:
  DummyPhysicsList(G4int mode=0);
  virtual ~DummyPhysicsList();
  
public:
  // SetCuts() 
  virtual void SetCuts();

};

#endif



