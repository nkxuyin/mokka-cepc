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
// $Id: RunManager.hh,v 1.1 2003/07/18 09:05:46 musat Exp $
// $Name: mokka-07-00 $
//
#ifndef RunManager_h
#define RunManager_h 1

#include "G4RunManager.hh"
#include "globals.hh"

class RunManager : public G4RunManager
{
public:
  RunManager() : LastEvt(0) {}
  ~RunManager(){}

public:
  void RunInitialization();
  void RunTermination();
  void BeamOn(G4int n_event,const char* macroFile,G4int n_select);

  G4Event* GetLastEvt() const { return LastEvt; }

private:
    G4Event* LastEvt;
};
#endif

    
