// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: MagPlugin.cc,v 1.3 2006/12/05 12:17:37 adrian Exp $
// $Name: mokka-07-00 $

#include "MagPlugin.hh"
#include "MagPluginMessenger.hh"

#include "G4TransportationManager.hh"
#include "G4FieldManager.hh"
#include "G4MagneticField.hh"
#include "G4UIcommand.hh"

#include "G4VVisManager.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4Polyline.hh"
#include "G4Circle.hh"

INITPLUGIN(MagPlugin, "MagPlugin")

void MagPlugin::Init(void)
{
  _messenger = new MagPluginMessenger(this);
  _stepLength = 1 * mm;
}

void MagPlugin::Exit(void)
{
  delete _messenger;
}

void MagPlugin::GetFieldValue(const G4ThreeVector &position) const
{
  const G4Field *detectorField = G4TransportationManager::GetTransportationManager()->GetFieldManager()->GetDetectorField();
  if (!detectorField) {
    G4cout << "No detector field defined." << G4endl;
    return;
  }
  
  G4cout << G4UIcommand::ConvertToString(GetFieldVector(detectorField, position), "T") << G4endl;
  MarkPosition(position);
}

void MagPlugin::ShowFluxLine(const G4ThreeVector &position) const
{
  G4VVisManager *pVVisManager = G4VVisManager::GetConcreteInstance();
  if (!pVVisManager) return;

  const G4Field *detectorField = G4TransportationManager::GetTransportationManager()->GetFieldManager()->GetDetectorField();
  if (!detectorField) {
    G4cout << "No detector field defined." << G4endl;
    return;
  }
  
  static const G4VisAttributes *visRed   = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0));
  static const G4VisAttributes *visGreen = new G4VisAttributes(G4Colour(0.0, 0.8, 0.0));
  
  for (G4int direction = -1; direction <= +1; direction += 2) { // direction = -1, +1
    G4ThreeVector wanderer(position);
    G4Polyline polyline;
    polyline.push_back(wanderer);
  
    for (G4int i = 1; i < 50000; i++) {
      const G4ThreeVector preStepVector = GetFieldVector(detectorField, wanderer);
      const G4double preStepMag = preStepVector.mag();
      const G4double preStepTheta = preStepVector.getTheta();

      if (preStepMag < 1.0E-04 * tesla) break; // field is vanishing
      wanderer += (preStepVector.unit() * _stepLength * direction); // step forward or backward

      const G4ThreeVector postStepVector = GetFieldVector(detectorField, wanderer);
      const G4double postStepMag = postStepVector.mag();
      const G4double postStepTheta = postStepVector.getTheta();
    
      if (fabs(postStepMag   - preStepMag)   > 1 * tesla) break; // seems to be a boundary of a field region
      if (fabs(postStepTheta - preStepTheta) > 60 * deg)  break; // seems to be a boundary of a field region
      if (i % 10 == 0) polyline.push_back(wanderer); // connect every 10th step
    }
    
    if (direction > 0) polyline.SetVisAttributes(visRed);   // where the field is pointing to
    else               polyline.SetVisAttributes(visGreen); // where the field is coming from

    if (polyline.size() > 1) pVVisManager->Draw(polyline);
  }
}

G4ThreeVector MagPlugin::GetFieldVector(const G4Field *field, const G4ThreeVector &position) const
{
  G4double point[4] = { position.getX(), position.getY(), position.getZ(), 0 };
  G4double bField[3] = { 0, 0, 0 };
  field->GetFieldValue(point, bField);
  return G4ThreeVector(bField[0], bField[1], bField[2]);
}

void MagPlugin::MarkPosition(const G4Point3D &position) const
{
  G4VVisManager *pVVisManager = G4VVisManager::GetConcreteInstance();
  if (!pVVisManager) return;

  static const G4VisAttributes *visPink = new G4VisAttributes(G4Colour(1.0, 0.5, 0.5));

  G4Circle circle(position);
  circle.SetScreenDiameter(3);
  circle.SetFillStyle(G4Circle::filled);
  circle.SetVisAttributes(visPink);
  pVVisManager->Draw(circle);
}

void MagPlugin::SetStepLength(const G4double stepLength)
{
  _stepLength = stepLength;
}

G4double MagPlugin::GetStepLength(void) const
{
  return _stepLength;
}
