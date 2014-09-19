// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: MarkerPlugin.hh,v 1.2 2006/07/26 17:28:39 adrian Exp $
// $Name: mokka-07-00 $

#ifndef MarkerPlugin_hh
#define MarkerPlugin_hh 1

#include "Plugin.hh"
#include "G4ThreeVector.hh"

class MarkerPluginMessenger;

class MarkerPlugin: public Plugin
{
public:
  MarkerPlugin(const std::string &name): Plugin(name) {}
  ~MarkerPlugin(void) {}

  void Init(void);
  void Exit(void);

public:
  void PutMarker(const G4ThreeVector &position);
  void SetMarkerColour(const G4ThreeVector &colour);
  void SetMarkerLength(const G4double length);
  void SetMarkerType(const G4int type);
  void SetRulerFromPoint(const G4ThreeVector &point);
  void SetRulerToPoint(const G4ThreeVector &point);
  void SetRulerStepLength(const G4double length);
  void SetRulerColour(const G4ThreeVector &colour);
  void SetRulerLabel(const G4bool label);

  void DrawRuler(void) const;
  
  G4ThreeVector GetMarkerPosition(void) const;
  G4ThreeVector GetMarkerColour(void) const;
  G4double GetMarkerLength(void) const;
  G4int GetMarkerType(void) const;
  G4ThreeVector GetRulerFromPoint(void) const;
  G4ThreeVector GetRulerToPoint(void) const;
  G4double GetRulerStepLength(void) const;
  G4ThreeVector GetRulerColour(void) const;
  G4bool GetRulerLabel(void) const;

private:
  typedef enum {
    kPlus   = 0,
    kCross  = 1,
    kStar   = 2,
    kCube   = 3,
    kFill   = 4,
    kOcta   = 5,
    kCuboct = 6,
    kTetra1 = 7,
    kTetra2 = 8,
    kTwin   = 9
  } EMarkerType;
  
  G4ThreeVector _markerPosition;
  G4ThreeVector _markerColour;
  G4double      _markerLength;
  EMarkerType   _markerType;
  
  G4ThreeVector _rulerFromPoint;
  G4ThreeVector _rulerToPoint;
  G4double      _rulerStepLength;
  G4ThreeVector _rulerColour;
  G4bool        _rulerLabel;

  MarkerPluginMessenger *_messenger;
};

#endif
