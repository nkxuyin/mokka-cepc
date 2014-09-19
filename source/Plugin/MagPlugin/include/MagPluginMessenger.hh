// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: MagPluginMessenger.hh,v 1.2 2006/12/05 12:17:37 adrian Exp $
// $Name: mokka-07-00 $

#ifndef MagPluginMessenger_hh
#define MagPluginMessenger_hh 1

#include "G4UImessenger.hh"
#include "G4ThreeVector.hh"

class G4UIdirectory;
class G4UIcommand;
class G4UIcmdWith3VectorAndUnit;
class G4UIcmdWithADoubleAndUnit;

class MagPlugin;

class MagPluginMessenger: public G4UImessenger
{
public:
  MagPluginMessenger(MagPlugin *handler);
  ~MagPluginMessenger(void);
  
  void SetNewValue(G4UIcommand *command, G4String newValue);
  G4String GetCurrentValue(G4UIcommand *command);
  
private:
  G4UIdirectory             *_magnetDir;
  G4UIcmdWith3VectorAndUnit *_getMagneticFieldCmd;
  G4UIcmdWith3VectorAndUnit *_showFluxLineCmd;
  G4UIcmdWithADoubleAndUnit *_setStepLengthCmd;
  
  G4ThreeVector _fieldPosition;
  G4ThreeVector _fluxPosition;
    
  MagPlugin *_magPlugin;
};

#endif
