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
// $Id: SteppingActionMessenger.cc,v 1.2 2006/01/12 12:50:30 mora Exp $
// $Name: mokka-07-00 $
//
#include "SteppingActionMessenger.hh"
#include "SteppingAction.hh"

#include "globals.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAnInteger.hh"

SteppingActionMessenger::SteppingActionMessenger(SteppingAction* SA)
:theSteppingAction(SA)
{
  stepDirectory = new G4UIdirectory("/step/");
  stepDirectory->SetGuidance("Step control commands.");

  drawStepCmd = new G4UIcmdWithAnInteger("/step/draw",this);
  drawStepCmd->SetGuidance("Draw each step on the fly.");
  drawStepCmd->SetParameterName("drawflag", true);
  drawStepCmd->SetDefaultValue(1);
}

void SteppingActionMessenger::SetNewValue(G4UIcommand* command, G4String newValues)
{
  if( command->GetCommandName() == "draw" )
  {
    G4int vl;
    const char* t = newValues;
    std::istringstream is((char*)t);
    is >> vl;
    theSteppingAction->SetDrawFlag(vl!=0);
  }
}

