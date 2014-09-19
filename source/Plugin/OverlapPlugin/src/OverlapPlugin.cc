// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: OverlapPlugin.cc,v 1.1 2008/04/26 14:04:49 adrian Exp $
// $Name: mokka-07-00 $

#include "OverlapPlugin.hh"

#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithABool.hh"

#include "G4PhysicalVolumeStore.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVParameterised.hh"

#include <set>

INITPLUGIN(OverlapPlugin, "OverlapPlugin")

void OverlapPlugin::Init()
{
    _resolution = 1000; // taken from the default values in "G4VPhysicalVolume.hh"
    _tolerance = 0; // taken from the default values in "G4VPhysicalVolume.hh"
    _verbose = true; // taken from the default values in "G4VPhysicalVolume.hh"

    std::set<G4String> candidateSet; // put all candidates in a set to avoid multiple equal entries in the candidate list
    G4String candidateString("all"); // special name for a check of all volumes, other names will be appended here

    G4PhysicalVolumeStore *store = G4PhysicalVolumeStore::GetInstance();

    for (G4PhysicalVolumeStore::iterator volume = store->begin(); volume != store->end(); ++volume)
        candidateSet.insert((*volume)->GetName()); // ignore entries that already exist in the set
    for (std::set<G4String>::iterator name = candidateSet.begin(); name != candidateSet.end(); ++name)
        candidateString.append(" ").append(*name); // not as fancy as an ostream_iterator, but it does the job
    
    _overlapDir = new G4UIdirectory("/Mokka/overlap/");
    _overlapDir->SetGuidance("Commands to run and control an overlap check of the geometry.");
  
    _checkCmd = new G4UIcmdWithAString("/Mokka/overlap/check", this);
    _checkCmd->SetGuidance("Perform an overlap check of some or all physical volumes.");
    _checkCmd->SetGuidance("This command invokes G4VPhysicalVolume::CheckOverlaps().");
    _checkCmd->SetParameterName("volume", true, false);
    _checkCmd->SetDefaultValue("all");
    _checkCmd->SetCandidates(candidateString);
  
    _resolutionCmd = new G4UIcmdWithAnInteger("/Mokka/overlap/resolution", this);
    _resolutionCmd->SetGuidance("Set the number of points that will be generated and verified per volume.");
    _resolutionCmd->SetGuidance("This value corresponds to the first argument of G4VPhysicalVolume::CheckOverlaps().");
    _resolutionCmd->SetParameterName("res", false, false);
    _resolutionCmd->GetParameter(0)->SetParameterRange("res > 0");
    _resolutionCmd->SetDefaultValue(1000);
  
    _toleranceCmd = new G4UIcmdWithADoubleAndUnit("/Mokka/overlap/tolerance", this);
    _toleranceCmd->SetGuidance("Set the tolerable error distance between volumes for the overlap check.");
    _toleranceCmd->SetGuidance("This value corresponds to the second argument of G4VPhysicalVolume::CheckOverlaps().");
    _toleranceCmd->SetParameterName("tol", false, false);
    _toleranceCmd->GetParameter(0)->SetParameterRange("tol >= 0");
    _toleranceCmd->SetDefaultValue(0);
    _toleranceCmd->SetDefaultUnit("mm");
  
    _verboseCmd = new G4UIcmdWithABool("/Mokka/overlap/verbose", this);
    _verboseCmd->SetGuidance("Enable or disable verbose output for the overlap check.");
    _verboseCmd->SetGuidance("If true, all checked volumes will be reported.");
    _verboseCmd->SetGuidance("If false, only error messages will be reported.");
    _verboseCmd->SetGuidance("This value corresponds to the third argument of G4VPhysicalVolume::CheckOverlaps().");
    _verboseCmd->SetParameterName("verbose", true, false);
    _verboseCmd->SetDefaultValue("true");

    G4cout << getName() << " has created a command directory \"" << _overlapDir->GetCommandPath() << "\"." << G4endl;
    G4cout << "See the Mokka help for further explanations." << G4endl;
}

void OverlapPlugin::Exit()
{
    delete _verboseCmd;
    delete _toleranceCmd;
    delete _resolutionCmd;
    delete _checkCmd;
    delete _overlapDir;
}

void OverlapPlugin::SetNewValue(G4UIcommand *command, G4String newValue)
{
    if      (command == _checkCmd)      CheckOverlaps(newValue);
    else if (command == _resolutionCmd) _resolution = _resolutionCmd->GetNewIntValue(newValue);
    else if (command == _toleranceCmd)  _tolerance = _toleranceCmd->GetNewDoubleValue(newValue);
    else if (command == _verboseCmd)    _verbose = _verboseCmd->GetNewBoolValue(newValue);
}

G4String OverlapPlugin::GetCurrentValue(G4UIcommand *command)
{
    if      (command == _resolutionCmd) return _resolutionCmd->ConvertToString(_resolution);
    else if (command == _toleranceCmd)  return _toleranceCmd->ConvertToStringWithBestUnit(_tolerance);
    else if (command == _verboseCmd)    return _verboseCmd->ConvertToString(_verbose);
    else                                return G4String(); // nothing
}

void OverlapPlugin::CheckOverlaps(const G4String &name) const
{
    G4int errorCount(0);
    G4int checkCount(0);
    const G4bool checkAll(name == G4String("all")); // don't do this comparison over and over again

    G4PhysicalVolumeStore *store = G4PhysicalVolumeStore::GetInstance();
  
    for (G4PhysicalVolumeStore::iterator volume = store->begin(); volume != store->end(); ++volume) {
        if (checkAll || (*volume)->GetName() == name) {
            // the default implementation of CheckOverlaps() from the base class doesn't do anything
            if (!dynamic_cast<G4PVPlacement *>(*volume) && !dynamic_cast<G4PVParameterised *>(*volume)) {
                G4cout << "WARNING - inherited member function G4VPhysicalVolume::CheckOverlaps()" << G4endl;
                G4cout << "          might not be implemented for volume " << (*volume)->GetName() << G4endl;
            }
            if ((*volume)->CheckOverlaps(_resolution, _tolerance, _verbose)) {
                ++errorCount;
            }
            ++checkCount;
        }
    }
    
    G4cout << checkCount << " of " << store->size() << " physical volumes were checked." << G4endl;
    if (errorCount) {
        G4cout << "WARNING - " << errorCount << " overlapping volumes were found." << G4endl;
    } else {
        G4cout << "No overlapping volumes were found." << G4endl;
    }
}
