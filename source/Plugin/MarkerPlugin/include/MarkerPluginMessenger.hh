// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: MarkerPluginMessenger.hh,v 1.2 2006/07/26 17:28:39 adrian Exp $
// $Name: mokka-07-00 $

#ifndef MarkerPluginMessenger_hh
#define MarkerPluginMessenger_hh 1

#include "G4UImessenger.hh"
#include "G4ThreeVector.hh"

class G4UIdirectory;
class G4UIcommand;
class G4UIcmdWithoutParameter;
class G4UIcmdWithAnInteger;
class G4UIcmdWith3Vector;
class G4UIcmdWith3VectorAndUnit;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithABool;

class MarkerPlugin;

class MarkerPluginMessenger: public G4UImessenger
{
public:
  MarkerPluginMessenger(MarkerPlugin *handler);
  ~MarkerPluginMessenger(void);
  
  void SetNewValue(G4UIcommand *command, G4String newValue);
  G4String GetCurrentValue(G4UIcommand *command);
  
private:
  G4UIdirectory             *_markerDir;
  G4UIdirectory             *_rulerDir;
  
  G4UIcmdWith3VectorAndUnit *_putMarkerCmd;
  G4UIcmdWith3Vector        *_markerColourCmd;
  G4UIcmdWithADoubleAndUnit *_markerLengthCmd;
  G4UIcmdWithAnInteger      *_markerTypeCmd;
  G4UIcmdWith3VectorAndUnit *_rulerFromCmd;
  G4UIcmdWith3VectorAndUnit *_rulerToCmd;
  G4UIcmdWithADoubleAndUnit *_rulerStepCmd;
  G4UIcmdWith3Vector        *_rulerColourCmd;
  G4UIcmdWithABool          *_rulerLabelCmd;
  G4UIcmdWithoutParameter   *_rulerDrawCmd;
  
  MarkerPlugin              *_markerPlugin;
};

#endif
