// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: RandomPluginMessenger.hh,v 1.1 2006/10/24 10:00:29 adrian Exp $
// $Name: mokka-07-00 $

#ifndef RandomPluginMessenger_hh
#define RandomPluginMessenger_hh 1

#include "G4UImessenger.hh"

class G4UIcommand;
class G4UIcmdWithAnInteger;

class RandomPlugin;

class RandomPluginMessenger: public G4UImessenger
{
public:
  RandomPluginMessenger(RandomPlugin *handler);
  ~RandomPluginMessenger();
  
  void SetNewValue(G4UIcommand *command, G4String);
  G4String GetCurrentValue(G4UIcommand *command);
  
private:
  G4UIcmdWithAnInteger *_randomSeedCmd;
  RandomPlugin         *_randomPlugin;
};

#endif
