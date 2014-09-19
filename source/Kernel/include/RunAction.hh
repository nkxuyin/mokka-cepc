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
// $Id: RunAction.hh,v 1.2 2004/04/08 14:27:03 frank Exp $
// $Name: mokka-07-00 $
//
// Calls Begin/EndOfRun methods for registered plugins
//-------------------------------------------------------

#ifndef RunAction_h
#define RunAction_h

#include "G4UserRunAction.hh"

class RunAction: public G4UserRunAction {

public:

  RunAction() ;
  virtual ~RunAction() { /*no_op*/ };
      
  virtual void BeginOfRunAction(const G4Run* aRun);    
  virtual void EndOfRunAction(const G4Run* aRun);    

};

#endif
