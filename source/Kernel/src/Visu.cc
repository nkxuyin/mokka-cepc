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
// $Id: Visu.cc,v 1.2 2006/01/31 09:39:22 mora Exp $
// $Name: mokka-07-00 $
//
// 
#include "VisuMessenger.hh"
#include "Visu.hh"
#include "globals.hh"
#include "G4Event.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"


Visu::Visu() 
  : currentEvt(0)
{
  theVisuMessenger = new VisuMessenger((Visu*)this);
}

Visu::~Visu() 
{ 
	delete theVisuMessenger;
}

void 
Visu::Refresh() 
{
  if(currentEvt) currentEvt->Draw();
}

void 
Visu::SetCurrentEvt(const G4Event* theCurrentEvt)
{
  currentEvt=(G4Event*) theCurrentEvt;
}

void 
Visu::SetLevel(G4String aNewLevel)
{
  const char* t = aNewLevel;
  G4int newLevel = 99999999;
  std::istringstream is((char*)t);
  is >> newLevel;
  G4cout << "Setting the visualisation level to " 
	 << newLevel
	 << G4endl;

}



