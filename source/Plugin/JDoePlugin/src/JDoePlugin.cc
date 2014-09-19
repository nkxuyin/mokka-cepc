// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: JDoePlugin.cc,v 1.4 2006/07/24 17:30:21 adrian Exp $
// $Name: mokka-07-00 $

#include "JDoePlugin.hh"
#include "JDoePluginMessenger.hh"

#include "CGAGeometryManager.hh"

#include "G4Material.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Orb.hh"
#include "G4EllipticalTube.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4Navigator.hh"
#include "G4TransportationManager.hh"
#include "G4RunManager.hh"
#include "G4UImanager.hh"

INITPLUGIN(JDoePlugin, "JDoePlugin")

void JDoePlugin::Init(void)
{
  _attributes = new G4VisAttributes();
  _attributes->SetForceSolid(true);

  G4Material *air = CGAGeometryManager::GetMaterial("air");
  
  // container for all other parts of the body
  G4VSolid *containerSolid = new G4Box("J. Doe", 30 * cm, 12 * cm, 90 * cm);
  G4LogicalVolume *containerLog = new G4LogicalVolume(containerSolid, air, "J. Doe");
  containerLog->SetVisAttributes(G4VisAttributes::Invisible);
  
  G4VSolid *headSolid = new G4Orb("J. Doe's Head", 12 * cm);
  G4LogicalVolume *headLog = new G4LogicalVolume(headSolid, air, "J. Doe's Head");
  headLog->SetVisAttributes(_attributes);
  new G4PVPlacement(0, G4ThreeVector(0 * cm, 0, 78 * cm), headLog, "J. Doe's Head", containerLog, false, 0);
  
  G4VSolid *bodySolid = new G4EllipticalTube("J. Doe's Body", 18 * cm, 12 * cm, 36 * cm);
  G4LogicalVolume *bodyLog = new G4LogicalVolume(bodySolid, air, "J. Doe's Body");
  bodyLog->SetVisAttributes(_attributes);
  new G4PVPlacement(0, G4ThreeVector(0 * cm, 0, 26 * cm), bodyLog, "J. Doe's Body", containerLog, false, 0);

  G4VSolid *armSolid = new G4Tubs("J. Doe's Arm", 0 * cm, 4 * cm, 41 * cm, 0 * deg, 360 * deg);
  G4LogicalVolume *armLog = new G4LogicalVolume(armSolid, air, "J. Doe's Arm");
  armLog->SetVisAttributes(_attributes);
  new G4PVPlacement(0, G4ThreeVector(-26 * cm, 0, 21 * cm), armLog, "J. Doe's Arm", containerLog, false, 1);
  new G4PVPlacement(0, G4ThreeVector(+26 * cm, 0, 21 * cm), armLog, "J. Doe's Arm", containerLog, false, 2);

  G4VSolid *legSolid = new G4Tubs("J. Doe's Leg", 0 * cm, 6 * cm, 38 * cm, 0 * deg, 360 * deg);
  G4LogicalVolume *legLog = new G4LogicalVolume(legSolid, air, "J. Doe's Leg");
  legLog->SetVisAttributes(_attributes);
  new G4PVPlacement(0, G4ThreeVector(-9 * cm, 0, -52 * cm), legLog, "J. Doe's Leg", containerLog, false, 1);
  new G4PVPlacement(0, G4ThreeVector(+9 * cm, 0, -52 * cm), legLog, "J. Doe's Leg", containerLog, false, 2);
  
  _log = containerLog;
  _world = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->GetWorldVolume()->GetLogicalVolume();
  _phys.clear(); // the body has not yet been placed anywhere
  
  this->SetPosition(G4ThreeVector(5000 * mm, -6060 * mm, 4000 * mm), false); // default values suitable for LDC01
  this->SetDirection(0 * deg, false); // default value, facing in z-direction
  this->SetColour(G4ThreeVector(1, 0, 0)); // default value, red colour
  
  _messenger = new JDoePluginMessenger(this); // add some commands to control J. Doe from the user interface
}

void JDoePlugin::Exit(void)
{
  delete _messenger;
}

const G4ThreeVector JDoePlugin::GetPosition(void) const
{
  return _position;
}

const G4double JDoePlugin::GetDirection(void) const
{
  return _direction;
}

const G4ThreeVector JDoePlugin::GetColour(void) const
{
  const G4Colour &colour = _attributes->GetColour();
  return G4ThreeVector(colour.GetRed(), colour.GetGreen(), colour.GetBlue());
}

void JDoePlugin::SetPosition(const G4ThreeVector &position, const G4bool updateCurrent)
{
  _position = position;
  if (updateCurrent && !_phys.empty()) {
    _phys.back()->SetTranslation(_position + G4ThreeVector(0, 90 * cm, 0));
    
    UpdateGeometry(); // tell the RunManager about the change
    UpdateView(); // tell the visualisation system to redisplay the geometry
  }
}

void JDoePlugin::SetDirection(const G4double direction, const G4bool updateCurrent)
{
  _direction = direction;
  if (updateCurrent && !_phys.empty()) {
    _phys.back()->GetRotation()->setPsi(_direction);
    
    UpdateGeometry(); // tell the RunManager about the change
    UpdateView(); // tell the visualisation system to redisplay the geometry
  }
}

void JDoePlugin::SetColour(const G4ThreeVector &colour)
{
  _attributes->SetColour(colour);
  UpdateView();
}

void JDoePlugin::NewInstance(void)
{
  G4PVPlacement *phys = new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateX(-90 * deg).rotateY(_direction), _position + G4ThreeVector(0, 90 * cm, 0)), _log, "J. Doe", _world, false, _phys.size());
  _phys.push_back(phys);
  
  UpdateGeometry(); // tell the RunManager about the change
  UpdateView(); // tell the visualisation system to redisplay the geometry
}

void JDoePlugin::DeleteInstance(void)
{
  if (!_phys.empty()) {
    delete _phys.back();
    _phys.pop_back();

    UpdateGeometry(); // tell the RunManager about the change
    UpdateView(); // tell the visualisation system to redisplay the geometry
  }
}

void JDoePlugin::UpdateGeometry(void) const
{
  G4RunManager::GetRunManager()->GeometryHasBeenModified();
}

void JDoePlugin::UpdateView(void) const
{
  G4UImanager::GetUIpointer()->ApplyCommand("/vis/scene/notifyHandlers");
}
