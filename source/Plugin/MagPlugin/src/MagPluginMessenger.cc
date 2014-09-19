// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: MagPluginMessenger.cc,v 1.2 2006/12/05 12:17:37 adrian Exp $
// $Name: mokka-07-00 $

#include "MagPluginMessenger.hh"
#include "MagPlugin.hh"

#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"

MagPluginMessenger::MagPluginMessenger(MagPlugin *handler): _fieldPosition(), _fluxPosition(), _magPlugin(handler)
{
  _magnetDir = new G4UIdirectory("/Mokka/magnet/");
  _magnetDir->SetGuidance("Commands related to the magnetic field.");
  
  _getMagneticFieldCmd = new G4UIcmdWith3VectorAndUnit("/Mokka/magnet/getMagneticField", this);
  _getMagneticFieldCmd->SetGuidance("Prints the magnetic field vector at a given position.");
  _getMagneticFieldCmd->SetParameterName("X", "Y", "Z", true, true);
  _getMagneticFieldCmd->SetDefaultUnit("mm");
  // taken from /gun/position
  
  _showFluxLineCmd = new G4UIcmdWith3VectorAndUnit("/Mokka/magnet/showFluxLine", this);
  _showFluxLineCmd->SetGuidance("Shows the magnetic flux line going through a given position.");
  _showFluxLineCmd->SetGuidance("The field vectors point from green to red.");
  _showFluxLineCmd->SetParameterName("X", "Y", "Z", true, true);
  _showFluxLineCmd->SetDefaultUnit("mm");
  // taken from /gun/position
  
  _setStepLengthCmd = new G4UIcmdWithADoubleAndUnit("/Mokka/magnet/setStepLength", this);
  _setStepLengthCmd->SetGuidance("Sets the step length for calculating flux lines.");
  _setStepLengthCmd->SetGuidance("Every tenth stepping point is connected and drawn.");
  _setStepLengthCmd->SetParameterName("length", true, false);
  _setStepLengthCmd->SetDefaultValue(1);
  _setStepLengthCmd->SetDefaultUnit("mm");
  
  G4cout << "MagPlugin has created a command directory \"/Mokka/magnet/\"." << G4endl;
  G4cout << "See the Mokka help for further explanations." << G4endl;
}

MagPluginMessenger::~MagPluginMessenger(void)
{
  delete _setStepLengthCmd;
  delete _showFluxLineCmd;
  delete _getMagneticFieldCmd;
  delete _magnetDir;
}

void MagPluginMessenger::SetNewValue(G4UIcommand *command, G4String newValue)
{
  if        (command == _getMagneticFieldCmd) {
    _fieldPosition = _getMagneticFieldCmd->GetNew3VectorValue(newValue);
    _magPlugin->GetFieldValue(_fieldPosition);
  } else if (command == _showFluxLineCmd) {
    _fluxPosition = _showFluxLineCmd->GetNew3VectorValue(newValue);
    _magPlugin->ShowFluxLine(_fluxPosition);
  } else if (command == _setStepLengthCmd) {
    _magPlugin->SetStepLength(_setStepLengthCmd->GetNewDoubleValue(newValue));
  }
}

G4String MagPluginMessenger::GetCurrentValue(G4UIcommand *command)
{
  if      (command == _getMagneticFieldCmd)
    return _getMagneticFieldCmd->ConvertToStringWithDefaultUnit(_fieldPosition);
  else if (command == _showFluxLineCmd)
    return _showFluxLineCmd->ConvertToStringWithDefaultUnit(_fluxPosition);
  else if (command == _setStepLengthCmd)
    return _setStepLengthCmd->ConvertToStringWithDefaultUnit(_magPlugin->GetStepLength());
  else
    return G4String(); // nothing
}
