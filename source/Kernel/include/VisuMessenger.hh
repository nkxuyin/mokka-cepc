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
// $Id: VisuMessenger.hh,v 1.3 2006/01/31 16:19:23 mora Exp $
// $Name: mokka-07-00 $
//

#ifndef VisuMessenger_h
#define VisuMessenger_h 1

#include "globals.hh"
#include "G4UImessenger.hh"
#include "G4UIcommand.hh"

class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithoutParameter;
class G4UIcmdWithAnInteger;
class G4VisAttributes;
class Visu;

class VisuMessenger: public G4UImessenger
{
public:
  VisuMessenger(Visu* aVisu);
  ~VisuMessenger();
  void SetNewValue(G4UIcommand* command, G4String newValues);
private:
  Visu* theVisu;
  G4UIcmdWithoutParameter* theRefreshCmd;
  G4UIcommand *theModeCmd;
  G4UIcommand *theColorCmd;
  G4UIcommand *theDaughtersCmd;
  G4UIcommand *thevisibilityCmd;
  G4UIcommand *theListThreeCmd;
  G4UIcommand *theImmediateCmd;
  G4UIcommand *theDefaultCmd;
  G4UIcommand *theGDMLCmd;
  G4UIcommand *theVRMLCmd;

  G4bool theImmediateMode;

};

#endif

