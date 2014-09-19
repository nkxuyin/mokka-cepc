// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: JDoePluginMessenger.cc,v 1.4 2006/07/24 17:30:21 adrian Exp $
// $Name: mokka-07-00 $

#include "JDoePluginMessenger.hh"
#include "JDoePlugin.hh"

#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWith3Vector.hh"

JDoePluginMessenger::JDoePluginMessenger(JDoePlugin *handler): _handler(handler)
{
  _jdoeDir = new G4UIdirectory("/Mokka/jdoe/");
  _jdoeDir->SetGuidance("Commands to display a size reference person.");
  _jdoeDir->SetGuidance("John Doe (or Jane, whatever you prefer) is intended as a size reference.");
  _jdoeDir->SetGuidance("He (or she) is 1.80 m tall from tip to toe and consists of air.");
  
  _newCmd = new G4UIcmdWith3VectorAndUnit("/Mokka/jdoe/new", this);
  _newCmd->SetGuidance("Create another instance of J. Doe at a given position.");
  _newCmd->SetGuidance("Be careful not to produce geometry overlaps!");
  _newCmd->SetParameterName("X", "Y", "Z", true, true);
  _newCmd->SetDefaultUnit("mm");
  
  _deleteCmd = new G4UIcmdWithoutParameter("/Mokka/jdoe/delete", this);
  _deleteCmd->SetGuidance("Delete the most recent instance of J. Doe.");
  
  _positionCmd = new G4UIcmdWith3VectorAndUnit("/Mokka/jdoe/position", this);
  _positionCmd->SetGuidance("Set the position of J. Doe's feet.");
  _positionCmd->SetGuidance("This command acts on the most recent instance.");
  _positionCmd->SetGuidance("Be careful not to produce geometry overlaps!");
  _positionCmd->SetParameterName("X", "Y", "Z", true, true);
  _positionCmd->SetDefaultUnit("mm");
  
  _directionCmd = new G4UIcmdWithADoubleAndUnit("/Mokka/jdoe/direction", this);
  _directionCmd->SetGuidance("Set the direction of J. Doe's sight.");
  _directionCmd->SetGuidance("This command acts on the most recent instance.");
  _directionCmd->SetGuidance("Be careful not to produce geometry overlaps!");
  _directionCmd->SetParameterName("angle", true, true);
  _directionCmd->SetDefaultUnit("deg");
  
  _colourCmd = new G4UIcmdWith3Vector("/Mokka/jdoe/colour", this);
  _colourCmd->SetGuidance("Set J. Doe's colour.");
  _colourCmd->SetGuidance("This command acts on all instances.");
  _colourCmd->SetParameterName("R", "G", "B", true, true);
  _colourCmd->GetParameter(0)->SetParameterRange("R >= 0 && R <= 1");
  _colourCmd->GetParameter(1)->SetParameterRange("G >= 0 && G <= 1");
  _colourCmd->GetParameter(2)->SetParameterRange("B >= 0 && B <= 1");
  
  G4cout << "JDoePlugin has created a command directory \"/Mokka/jdoe/\"." << G4endl;
  G4cout << "See the Mokka help for further explanations." << G4endl;
}

JDoePluginMessenger::~JDoePluginMessenger(void)
{
  delete _directionCmd;
  delete _positionCmd;
  delete _deleteCmd;
  delete _newCmd;
  delete _jdoeDir;
}

void JDoePluginMessenger::SetNewValue(G4UIcommand *command, G4String newValue)
{
  if      (command == _newCmd)     { _handler->SetPosition(_newCmd->GetNew3VectorValue(newValue), false); _handler->NewInstance(); }
  else if (command == _deleteCmd)    _handler->DeleteInstance();
  else if (command == _positionCmd)  _handler->SetPosition(_positionCmd->GetNew3VectorValue(newValue), true);
  else if (command == _directionCmd) _handler->SetDirection(_directionCmd->GetNewDoubleValue(newValue), true);
  else if (command == _colourCmd)    _handler->SetColour(_colourCmd->GetNew3VectorValue(newValue));
}

G4String JDoePluginMessenger::GetCurrentValue(G4UIcommand *command)
{
  if      (command == _newCmd)       return _newCmd->ConvertToStringWithDefaultUnit(_handler->GetPosition());
  else if (command == _positionCmd)  return _positionCmd->ConvertToStringWithDefaultUnit(_handler->GetPosition());
  else if (command == _directionCmd) return _directionCmd->ConvertToStringWithDefaultUnit(_handler->GetDirection());
  else if (command == _colourCmd)    return _colourCmd->ConvertToString(_handler->GetColour());
  else                               return G4String(); // nothing
}
