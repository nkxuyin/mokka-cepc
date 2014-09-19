// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: MagPlugin.hh,v 1.2 2006/12/05 12:17:37 adrian Exp $
// $Name: mokka-07-00 $

#ifndef MagPlugin_hh
#define MagPlugin_hh 1

#include "Plugin.hh"
#include "G4ThreeVector.hh"
#include "G4Point3D.hh"

class MagPluginMessenger;
class G4Field;

class MagPlugin: public Plugin
{
public:
  MagPlugin(const std::string &name): Plugin(name) {}
  ~MagPlugin(void) {}

  void Init(void);
  void Exit(void);

  void GetFieldValue(const G4ThreeVector &position) const;
  void ShowFluxLine(const G4ThreeVector &position) const;
  void SetStepLength(const G4double stepLength);
  G4double GetStepLength(void) const;

private:
  G4ThreeVector GetFieldVector(const G4Field *field, const G4ThreeVector &position) const;
  void MarkPosition(const G4Point3D &position) const;

  MagPluginMessenger *_messenger; // kept only for later destruction
  G4double _stepLength;
};

#endif
