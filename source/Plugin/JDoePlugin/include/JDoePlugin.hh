// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: JDoePlugin.hh,v 1.3 2006/07/24 17:30:21 adrian Exp $
// $Name: mokka-07-00 $

#ifndef JDoePlugin_hh
#define JDoePlugin_hh 1

#include "Plugin.hh"
#include <vector>

class JDoePluginMessenger;
class G4LogicalVolume;
class G4PVPlacement;
class G4VisAttributes;

class JDoePlugin: public Plugin
{
public:
  JDoePlugin(const std::string &name): Plugin(name) {}
  ~JDoePlugin(void) {}

public:
  void Init(void);
  void Exit(void);
  
  const G4ThreeVector GetPosition(void) const;
  const G4double GetDirection(void) const;
  const G4ThreeVector GetColour(void) const;
  
  void SetPosition(const G4ThreeVector &position, const G4bool updateCurrent);
  void SetDirection(const G4double direction, const G4bool updateCurrent);
  void SetColour(const G4ThreeVector &colour);

  void NewInstance(void);
  void DeleteInstance(void);

private:
  void UpdateGeometry(void) const;
  void UpdateView(void) const;

private:
  JDoePluginMessenger *_messenger; // G4UImessenger for additional user interface commands

  G4LogicalVolume *_log; // the logical volume containing all parts of J. Doe's body
  G4LogicalVolume *_world; // the world volume in which J. Doe will be placed
  std::vector<G4PVPlacement *> _phys; // vector of physical volumes as which J. Doe is placed
  
  G4ThreeVector _position; // position for the next placement of J. Doe as a physical volume
  G4double _direction; // direction (y-axis rotation) for the next placement of J. Doe
  G4VisAttributes *_attributes; // vis attributes (the colour), shared between all placements
};

#endif
