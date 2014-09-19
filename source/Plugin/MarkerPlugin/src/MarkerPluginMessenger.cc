// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: MarkerPluginMessenger.cc,v 1.2 2006/07/26 17:28:39 adrian Exp $
// $Name: mokka-07-00 $

#include "MarkerPluginMessenger.hh"
#include "MarkerPlugin.hh"

#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWith3Vector.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithABool.hh"

MarkerPluginMessenger::MarkerPluginMessenger(MarkerPlugin *handler): _markerPlugin(handler)
{
  _markerDir = new G4UIdirectory("/Mokka/marker/");
  _markerDir->SetGuidance("Commands to put markers into the 3D space.");
  
  _putMarkerCmd = new G4UIcmdWith3VectorAndUnit("/Mokka/marker/put", this);
  _putMarkerCmd->SetGuidance("Puts a marker at the given position.");
  _putMarkerCmd->SetParameterName("X", "Y", "Z", true, true);
  _putMarkerCmd->SetDefaultUnit("mm");
  
  _markerColourCmd = new G4UIcmdWith3Vector("/Mokka/marker/colour", this);
  _markerColourCmd->SetGuidance("Sets the colour of the markers to be drawn.");
  _markerColourCmd->SetParameterName("R", "G", "B", true, true);
  _markerColourCmd->GetParameter(0)->SetParameterRange("R >= 0 && R <= 1");
  _markerColourCmd->GetParameter(1)->SetParameterRange("G >= 0 && G <= 1");
  _markerColourCmd->GetParameter(2)->SetParameterRange("B >= 0 && B <= 1");
  
  _markerLengthCmd = new G4UIcmdWithADoubleAndUnit("/Mokka/marker/length", this);
  _markerLengthCmd->SetGuidance("Sets the length (measured from the centre) of the markers to be drawn.");
  _markerLengthCmd->SetParameterName("length", true, true);
  _markerLengthCmd->SetDefaultValue(100);
  _markerLengthCmd->SetDefaultUnit("mm");
  
  _markerTypeCmd = new G4UIcmdWithAnInteger("/Mokka/marker/type", this);
  _markerTypeCmd->SetGuidance("Sets the type of the markers to be drawn.");
  _markerTypeCmd->SetGuidance("  0 = plus\n  1 = cross\n  2 = star\n  3 = cube\n  4 = crossed cube\n  5 = octahedron\n"
    "  6 = cuboctahedron\n  7 = tetrahedron_1\n  8 = tetrahedron_2\n  9 = twin_tetrahedra");
  _markerTypeCmd->SetParameterName("type", true, true);
  _markerTypeCmd->GetParameter(0)->SetParameterRange("type >= 0 && type <= 9");
  _markerTypeCmd->SetDefaultValue(1);

  _rulerDir = new G4UIdirectory("/Mokka/ruler/");
  _rulerDir->SetGuidance("Commands to draw a ruler into the 3D space.");

  _rulerFromCmd = new G4UIcmdWith3VectorAndUnit("/Mokka/ruler/from", this);
  _rulerFromCmd->SetGuidance("Sets the starting point of the ruler.");
  _rulerFromCmd->SetParameterName("X", "Y", "Z", true, true);
  _rulerFromCmd->SetDefaultUnit("mm");
  
  _rulerToCmd= new G4UIcmdWith3VectorAndUnit("/Mokka/ruler/to", this);
  _rulerToCmd->SetGuidance("Sets the end point of the ruler.");
  _rulerToCmd->SetParameterName("X", "Y", "Z", true, true);
  _rulerToCmd->SetDefaultUnit("mm");
  
  _rulerStepCmd = new G4UIcmdWithADoubleAndUnit("/Mokka/ruler/step", this);
  _rulerStepCmd->SetGuidance("Sets the step length of the ruler.");
  _rulerStepCmd->SetParameterName("Length", true, true);
  _rulerStepCmd->SetDefaultUnit("mm");
  
  _rulerColourCmd = new G4UIcmdWith3Vector("/Mokka/ruler/colour", this);
  _rulerColourCmd->SetGuidance("Sets the colour of the ruler.");
  _rulerColourCmd->SetParameterName("R", "G", "B", true, true);
  _rulerColourCmd->GetParameter(0)->SetParameterRange("R >= 0 && R <= 1");
  _rulerColourCmd->GetParameter(1)->SetParameterRange("G >= 0 && G <= 1");
  _rulerColourCmd->GetParameter(2)->SetParameterRange("B >= 0 && B <= 1");

  _rulerLabelCmd = new G4UIcmdWithABool("/Mokka/ruler/label", this);
  _rulerLabelCmd->SetGuidance("Selects whether the ruler should have text labels or not.");
  _rulerLabelCmd->SetParameterName("Flag", false, false);
  _rulerLabelCmd->SetDefaultValue(true);

  _rulerDrawCmd = new G4UIcmdWithoutParameter("/Mokka/ruler/draw", this);
  _rulerDrawCmd->SetGuidance("Draws a ruler with the current settings.");
  
  G4cout << "MarkerPlugin has created two command directories \"/Mokka/marker/\" and \"/Mokka/ruler/\"." << G4endl;
  G4cout << "See the Mokka help for further explanations." << G4endl;
}

MarkerPluginMessenger::~MarkerPluginMessenger()
{
  delete _rulerDrawCmd;
  delete _rulerLabelCmd;
  delete _rulerColourCmd;
  delete _rulerStepCmd;
  delete _rulerToCmd;
  delete _rulerFromCmd;

  delete _markerTypeCmd;
  delete _markerLengthCmd;
  delete _markerColourCmd;
  delete _putMarkerCmd;
  
  delete _rulerDir;
  delete _markerDir;
}

void MarkerPluginMessenger::SetNewValue(G4UIcommand *command, G4String newValue)
{
  if      (command == _putMarkerCmd)    _markerPlugin->PutMarker(_putMarkerCmd->GetNew3VectorValue(newValue));
  else if (command == _markerColourCmd) _markerPlugin->SetMarkerColour(_markerColourCmd->GetNew3VectorValue(newValue));
  else if (command == _markerLengthCmd) _markerPlugin->SetMarkerLength(_markerLengthCmd->GetNewDoubleValue(newValue));
  else if (command == _markerTypeCmd)   _markerPlugin->SetMarkerType(_markerTypeCmd->GetNewIntValue(newValue));
  else if (command == _rulerFromCmd)    _markerPlugin->SetRulerFromPoint(_rulerFromCmd->GetNew3VectorValue(newValue));
  else if (command == _rulerToCmd)      _markerPlugin->SetRulerToPoint(_rulerToCmd->GetNew3VectorValue(newValue));
  else if (command == _rulerStepCmd)    _markerPlugin->SetRulerStepLength(_rulerStepCmd->GetNewDoubleValue(newValue));
  else if (command == _rulerColourCmd)  _markerPlugin->SetRulerColour(_rulerColourCmd->GetNew3VectorValue(newValue));
  else if (command == _rulerLabelCmd)   _markerPlugin->SetRulerLabel(_rulerLabelCmd->GetNewBoolValue(newValue));
  else if (command == _rulerDrawCmd)    _markerPlugin->DrawRuler();
}

G4String MarkerPluginMessenger::GetCurrentValue(G4UIcommand *command)
{
  if      (command == _putMarkerCmd)    return _putMarkerCmd->ConvertToStringWithDefaultUnit(_markerPlugin->GetMarkerPosition());
  else if (command == _markerColourCmd) return _markerColourCmd->ConvertToString(_markerPlugin->GetMarkerColour());
  else if (command == _markerLengthCmd) return _markerLengthCmd->ConvertToStringWithDefaultUnit(_markerPlugin->GetMarkerLength());
  else if (command == _markerTypeCmd)   return _markerTypeCmd->ConvertToString(_markerPlugin->GetMarkerType());
  else if (command == _rulerFromCmd)    return _rulerFromCmd->ConvertToStringWithDefaultUnit(_markerPlugin->GetRulerFromPoint());
  else if (command == _rulerToCmd)      return _rulerToCmd->ConvertToStringWithDefaultUnit(_markerPlugin->GetRulerToPoint());
  else if (command == _rulerStepCmd)    return _rulerStepCmd->ConvertToStringWithDefaultUnit(_markerPlugin->GetRulerStepLength());
  else if (command == _rulerColourCmd)  return _rulerColourCmd->ConvertToString(_markerPlugin->GetRulerColour());
  else if (command == _rulerLabelCmd)   return _rulerLabelCmd->ConvertToString(_markerPlugin->GetRulerLabel());
  else                                  return G4String(); // nothing
}
