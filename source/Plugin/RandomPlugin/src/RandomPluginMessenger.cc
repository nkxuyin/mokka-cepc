// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: RandomPluginMessenger.cc,v 1.1 2006/10/24 10:00:29 adrian Exp $
// $Name: mokka-07-00 $

#include "RandomPluginMessenger.hh"
#include "RandomPlugin.hh"

#include "G4UIcommand.hh"
#include "G4UIcmdWithAnInteger.hh"

RandomPluginMessenger::RandomPluginMessenger(RandomPlugin *handler): _randomPlugin(handler)
{
  _randomSeedCmd = new G4UIcmdWithAnInteger("/Mokka/randomSeed", this);
  _randomSeedCmd->SetGuidance("Sets the seed of the internal random generator.");
  _randomSeedCmd->SetGuidance("A parameter value of zero will set the seed to an arbitrary value");
  _randomSeedCmd->SetGuidance("(calculated from Unix time and current process ID).");
  _randomSeedCmd->SetParameterName("seed", true, false); // omittable, do not use current as default
  _randomSeedCmd->GetParameter(0)->SetParameterRange("seed >= 0");
  _randomSeedCmd->SetDefaultValue(0);
  
  G4cout << "RandomPlugin has created a command \"/Mokka/randomSeed\"." << G4endl;
  G4cout << "See the Mokka help for further explanations." << G4endl;
}

RandomPluginMessenger::~RandomPluginMessenger(void)
{
  delete _randomSeedCmd;
}

void RandomPluginMessenger::SetNewValue(G4UIcommand *command, G4String value)
{
  if      (command == _randomSeedCmd) _randomPlugin->SetRandomSeed(_randomSeedCmd->GetNewIntValue(value));
}

G4String RandomPluginMessenger::GetCurrentValue(G4UIcommand *command)
{
  if      (command == _randomSeedCmd) return _randomSeedCmd->ConvertToString(int(_randomPlugin->GetRandomSeed()));
  else                                return G4String(); // nothing
}
