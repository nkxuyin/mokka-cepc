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
// $Id: Visu.hh,v 1.2 2006/01/31 09:39:42 mora Exp $
// $Name: mokka-07-00 $
//

#ifndef Visu_h
#define Visu_h 1

#include "globals.hh"

class G4Event;
class G4LogicalVolume;
class LV_level;
class VisuMessenger;

class Visu
{
public:
  Visu();
  virtual ~Visu() ;

  G4Event* GetCurrentEvt() { return currentEvt;}
  void SetCurrentEvt(const G4Event* theCurrentEvt);

  void Refresh();
  void SetLevel(G4String);

private:

  G4Event* currentEvt;
  VisuMessenger * theVisuMessenger;

};

#endif

