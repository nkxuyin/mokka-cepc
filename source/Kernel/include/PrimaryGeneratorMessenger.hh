// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: PrimaryGeneratorMessenger.hh,v 1.7 2007/06/22 14:42:50 musat Exp $
// $Name: mokka-07-00 $

#ifndef PrimaryGeneratorMessenger_hh
#define PrimaryGeneratorMessenger_hh 1

#include "G4UImessenger.hh"
#include "globals.hh"

class PrimaryGeneratorAction;

class G4UIdirectory;
class G4UIcommand;
class G4UIcmdWithoutParameter;
class G4UIcmdWithAString;
class G4UIcmdWithADoubleAndUnit;

class PrimaryGeneratorMessenger: public G4UImessenger
{
public:
  PrimaryGeneratorMessenger(PrimaryGeneratorAction *primaryGenerator);
  ~PrimaryGeneratorMessenger(void);
    
  void SetNewValue(G4UIcommand *command, G4String newValues);
  G4String GetCurrentValue(G4UIcommand *command);

private:
  PrimaryGeneratorAction    *fPrimaryGenerator;
    
  G4UIdirectory             *fGeneratorDir;
  G4UIcmdWithAString        *fGeneratorNameCmd;
  G4UIcmdWithoutParameter   *fGeneratorInfoCmd;
};

#endif
