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
// $Id: SteppingActionMessenger.hh,v 1.1 2003/07/18 09:05:48 musat Exp $
// $Name: mokka-07-00 $
//

#ifndef SteppingActionMessenger_h
#define SteppingActionMessenger_h 1

#include "globals.hh"
#include "G4UImessenger.hh"
#include "G4UIcommand.hh"

class SteppingAction;
class G4UIdirectory;
//class G4UIcmdWithABool;
class G4UIcmdWithAnInteger;

class SteppingActionMessenger: public G4UImessenger
{
public:
  SteppingActionMessenger(SteppingAction *SA);
  void SetNewValue(G4UIcommand* command, G4String newValues);
private:
  SteppingAction* theSteppingAction;
  G4UIdirectory*       stepDirectory;
  //  G4UIcmdWithABool*    drawStepCmd;
  G4UIcmdWithAnInteger*   drawStepCmd;
};

#endif

