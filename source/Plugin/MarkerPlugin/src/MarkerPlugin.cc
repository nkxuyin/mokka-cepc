// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: MarkerPlugin.cc,v 1.3 2006/07/26 17:28:39 adrian Exp $
// $Name: mokka-07-00 $

#include "MarkerPlugin.hh"
#include "MarkerPluginMessenger.hh"

#include "G4VVisManager.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4Polyline.hh"
#include "G4Text.hh"
#include "G4UIcommand.hh"

INITPLUGIN(MarkerPlugin, "MarkerPlugin")

void MarkerPlugin::Init(void)
{
  _messenger = new MarkerPluginMessenger(this);
  _markerPosition = G4ThreeVector();
  _markerColour = G4ThreeVector(1, 0, 0);
  _markerLength = 100 * mm;
  _markerType = MarkerPlugin::kPlus;
  _rulerFromPoint = G4ThreeVector();
  _rulerToPoint = G4ThreeVector(0, 0, 5 * m);
  _rulerStepLength = 1000 * mm;
  _rulerColour = G4ThreeVector(1, 0, 0);
  _rulerLabel = true;
}

void MarkerPlugin::Exit(void)
{
  delete _messenger;
}

void MarkerPlugin::PutMarker(const G4ThreeVector &position)
{
  _markerPosition = position;

  G4VVisManager *visManager = G4VVisManager::GetConcreteInstance();
  if (!visManager) return;
  
  const G4ThreeVector x(_markerLength, 0, 0);
  const G4ThreeVector y(0, _markerLength, 0);
  const G4ThreeVector z(0, 0, _markerLength);
  
  G4Polyline polyline;
  polyline.SetVisAttributes(G4VisAttributes(G4Colour(_markerColour)));
  
  switch (_markerType) {

  case MarkerPlugin::kStar:
  case MarkerPlugin::kPlus:
    polyline.push_back(position - x);
    polyline.push_back(position + x);
    visManager->Draw(polyline);
    polyline.clear();

    polyline.push_back(position - y);
    polyline.push_back(position + y);
    visManager->Draw(polyline);
    polyline.clear();

    polyline.push_back(position - z);
    polyline.push_back(position + z);
    visManager->Draw(polyline);
    polyline.clear();

    if (_markerType != MarkerPlugin::kStar) break;
    // kStar = kPlus + kCross, no break

  case MarkerPlugin::kFill:
  case MarkerPlugin::kCross:
    polyline.push_back(position - x - y - z);
    polyline.push_back(position + x + y + z);
    visManager->Draw(polyline);
    polyline.clear();

    polyline.push_back(position - x + y - z);
    polyline.push_back(position + x - y + z);
    visManager->Draw(polyline);
    polyline.clear();

    polyline.push_back(position + x - y - z);
    polyline.push_back(position - x + y + z);
    visManager->Draw(polyline);
    polyline.clear();

    polyline.push_back(position + x + y - z);
    polyline.push_back(position - x - y + z);
    visManager->Draw(polyline);
    polyline.clear();

    if (_markerType != MarkerPlugin::kFill) break;
    // kFill = kCross + kCube, no break

  case MarkerPlugin::kCube:
    polyline.push_back(position - x - y - z);
    polyline.push_back(position + x - y - z);
    polyline.push_back(position + x + y - z);
    polyline.push_back(position - x + y - z);
    polyline.push_back(position - x + y + z);
    polyline.push_back(position + x + y + z);
    polyline.push_back(position + x - y + z);
    polyline.push_back(position - x - y + z);
    polyline.push_back(position - x - y - z);
    visManager->Draw(polyline);
    polyline.clear();

    polyline.push_back(position + x - y - z);
    polyline.push_back(position + x - y + z);
    visManager->Draw(polyline);
    polyline.clear();

    polyline.push_back(position + x + y - z);
    polyline.push_back(position + x + y + z);
    visManager->Draw(polyline);
    polyline.clear();

    polyline.push_back(position - x - y - z);
    polyline.push_back(position - x + y - z);
    visManager->Draw(polyline);
    polyline.clear();

    polyline.push_back(position - x - y + z);
    polyline.push_back(position - x + y + z);
    visManager->Draw(polyline);
    polyline.clear();

    break;

  case MarkerPlugin::kOcta:
    polyline.push_back(position - z);
    polyline.push_back(position - x);
    polyline.push_back(position + z);
    polyline.push_back(position + x);
    polyline.push_back(position - z);
    polyline.push_back(position - y);
    polyline.push_back(position + z);
    polyline.push_back(position + y);
    polyline.push_back(position - x);
    polyline.push_back(position - y);
    polyline.push_back(position + x);
    polyline.push_back(position + y);
    polyline.push_back(position - z);
    visManager->Draw(polyline);
    polyline.clear();

    break;

  case MarkerPlugin::kCuboct:
    polyline.push_back(position - x - z);
    polyline.push_back(position - y - z);
    polyline.push_back(position + x - z);
    polyline.push_back(position + y - z);
    polyline.push_back(position - x - z);
    polyline.push_back(position - x - y);
    polyline.push_back(position - x + z);
    polyline.push_back(position - y + z);
    polyline.push_back(position + x + z);
    polyline.push_back(position + y + z);
    polyline.push_back(position - x + z);
    polyline.push_back(position - x + y);
    polyline.push_back(position + y + z);
    polyline.push_back(position + x + y);
    polyline.push_back(position + x + z);
    polyline.push_back(position + x - y);
    polyline.push_back(position - y + z);
    polyline.push_back(position - x - y);
    polyline.push_back(position - y - z);
    polyline.push_back(position + x - y);
    polyline.push_back(position + x - z);
    polyline.push_back(position + x + y);
    polyline.push_back(position + y - z);
    polyline.push_back(position - x + y);
    polyline.push_back(position - x - z);
    visManager->Draw(polyline);
    polyline.clear();

    break;

  case MarkerPlugin::kTwin:
  case MarkerPlugin::kTetra1:
    polyline.push_back(position - x - y - z);
    polyline.push_back(position + x + y - z);
    polyline.push_back(position + x - y + z);
    polyline.push_back(position - x + y + z);
    polyline.push_back(position + x + y - z);
    visManager->Draw(polyline);
    polyline.clear();

    polyline.push_back(position - x + y + z);
    polyline.push_back(position - x - y - z);
    polyline.push_back(position + x - y + z);
    visManager->Draw(polyline);
    polyline.clear();

    if (_markerType != MarkerPlugin::kTwin) break;
    // kTwin = kTetra1 + kTetra2, no break

  case MarkerPlugin::kTetra2:
    polyline.push_back(position + x - y - z);
    polyline.push_back(position - x + y - z);
    polyline.push_back(position - x - y + z);
    polyline.push_back(position + x + y + z);
    polyline.push_back(position - x + y - z);
    visManager->Draw(polyline);
    polyline.clear();

    polyline.push_back(position + x + y + z);
    polyline.push_back(position + x - y - z);
    polyline.push_back(position - x - y + z);
    visManager->Draw(polyline);
    polyline.clear();

    break;
  }
}

void MarkerPlugin::DrawRuler(void) const
{
  G4VVisManager *visManager = G4VVisManager::GetConcreteInstance();
  if (!visManager) return;
  
  G4ThreeVector delta = _rulerToPoint - _rulerFromPoint;
  const G4int nSteps = int(delta.mag() / _rulerStepLength);
  G4ThreeVector step = delta.unit() * _rulerStepLength;
  
  G4VisAttributes rulerVisAttributes = G4VisAttributes(G4Colour(_rulerColour));
  G4Polyline polyline;
  polyline.SetVisAttributes(rulerVisAttributes);

  if (_rulerLabel) visManager->Draw(G4Text(""));
  // workaround because the first string is drawn twice
  // I don't know why, but it works for now...
  
  for (G4int i = 0; i < nSteps; i++) {
    G4ThreeVector rulerPosition = _rulerFromPoint + i * step;
    G4double rulerLength = i * _rulerStepLength;

    polyline.push_back(rulerPosition);
    if (i % 2) {
      visManager->Draw(polyline);
      polyline.clear();
    }
    if (_rulerLabel) {
      G4Text text(G4UIcommand::ConvertToString(rulerLength / mm), rulerPosition);
      text.SetVisAttributes(rulerVisAttributes);
      visManager->Draw(text);
    }
  }
}

void MarkerPlugin::SetMarkerColour(const G4ThreeVector &colour)  { _markerColour = colour; }
void MarkerPlugin::SetMarkerLength(const G4double length)        { _markerLength = length; }
void MarkerPlugin::SetMarkerType(const G4int type)               { _markerType = MarkerPlugin::EMarkerType(type); }
void MarkerPlugin::SetRulerFromPoint(const G4ThreeVector &point) { _rulerFromPoint = point; }
void MarkerPlugin::SetRulerToPoint(const G4ThreeVector &point)   { _rulerToPoint = point; }
void MarkerPlugin::SetRulerStepLength(const G4double length)     { _rulerStepLength = length; }
void MarkerPlugin::SetRulerColour(const G4ThreeVector &colour)   { _rulerColour = colour; }
void MarkerPlugin::SetRulerLabel(const G4bool label)             { _rulerLabel = label; }

G4ThreeVector MarkerPlugin::GetMarkerPosition(void) const        { return _markerPosition; }
G4ThreeVector MarkerPlugin::GetMarkerColour(void) const          { return _markerColour; }
G4double MarkerPlugin::GetMarkerLength(void) const               { return _markerLength; }
G4int MarkerPlugin::GetMarkerType(void) const                    { return G4int(_markerType); }
G4ThreeVector MarkerPlugin::GetRulerFromPoint(void) const        { return _rulerFromPoint; }
G4ThreeVector MarkerPlugin::GetRulerToPoint(void) const          { return _rulerToPoint; }
G4double MarkerPlugin::GetRulerStepLength(void) const            { return _rulerStepLength; }
G4ThreeVector MarkerPlugin::GetRulerColour(void) const           { return _rulerColour; }
G4bool MarkerPlugin::GetRulerLabel(void) const                   { return _rulerLabel; }
