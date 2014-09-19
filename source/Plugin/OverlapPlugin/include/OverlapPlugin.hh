// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: OverlapPlugin.hh,v 1.1 2008/04/26 14:04:49 adrian Exp $
// $Name: mokka-07-00 $

#ifndef OverlapPlugin_hh
#define OverlapPlugin_hh 1

#include "Plugin.hh"
#include "G4UImessenger.hh"

class G4UIdirectory;
class G4UIcommand;
class G4UIcmdWithAString;
class G4UIcmdWithAnInteger;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithABool;

class OverlapPlugin: public Plugin, public G4UImessenger
{
public:
    OverlapPlugin(const std::string &name): Plugin(name) {}
    ~OverlapPlugin() {}

public: // functions inherited from Plugin
    void Init();
    void Exit();

public: // functions inherited from G4UImessenger
    void SetNewValue(G4UIcommand *command, G4String newValue);
    G4String GetCurrentValue(G4UIcommand *command);

private:
    void CheckOverlaps(const G4String &name) const;
  
private:
    G4UIdirectory             *_overlapDir;
    G4UIcmdWithAString        *_checkCmd;
    G4UIcmdWithAnInteger      *_resolutionCmd;
    G4UIcmdWithADoubleAndUnit *_toleranceCmd;
    G4UIcmdWithABool          *_verboseCmd;

    G4int _resolution;
    G4double _tolerance;
    G4bool _verbose;
};

#endif
