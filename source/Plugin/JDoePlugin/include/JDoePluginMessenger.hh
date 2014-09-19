// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: JDoePluginMessenger.hh,v 1.2 2006/07/05 17:52:27 adrian Exp $
// $Name: mokka-07-00 $

#ifndef JDoePluginMessenger_hh
#define JDoePluginMessenger_hh 1

#include "G4UImessenger.hh"

class G4UIdirectory;
class G4UIcommand;
class G4UIcmdWithoutParameter;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWith3VectorAndUnit;
class G4UIcmdWith3Vector;

class JDoePlugin;

class JDoePluginMessenger: public G4UImessenger
{
public:
  JDoePluginMessenger(JDoePlugin *handler);
  ~JDoePluginMessenger(void);
  
  void SetNewValue(G4UIcommand *command, G4String newValue);
  G4String GetCurrentValue(G4UIcommand *command);

private:
  G4String PrintVisibleCount(const G4int count) const;

private:
  G4UIdirectory             *_jdoeDir;
  G4UIcmdWith3VectorAndUnit *_newCmd;
  G4UIcmdWithoutParameter   *_deleteCmd;
  G4UIcmdWith3VectorAndUnit *_positionCmd;
  G4UIcmdWithADoubleAndUnit *_directionCmd;
  G4UIcmdWith3Vector        *_colourCmd;
  
  JDoePlugin                *_handler;
};

#endif
