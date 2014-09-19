// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: Pi0Tracker.hh,v 0.0 2010/05/12 15:49:00 A.Kaplan Exp $
// $Name: mokka-07-02 $

#ifndef Pi0Tracker_hh
#define Pi0Tracker_hh 1

#include "Control.hh"
#include "Plugin.hh"

class Pi0TrackerMessenger;

class Pi0Tracker: public Plugin
{
public:
  Pi0Tracker(const std::string &name): Plugin(name) {}
  ~Pi0Tracker(void) {}

public: // from Plugin
  void Init(void);
  void Exit(void);
  
  virtual void BeginOfRunAction(const G4Run *run);
  virtual void EndOfRunAction(const G4Run *run);

  virtual void BeginOfEventAction(const G4Event *evt);
  virtual void EndOfEventAction(const G4Event *evt);

  virtual void PreUserTrackingAction(const G4Track *trk);
  virtual void PostUserTrackingAction(const G4Track *trk);

  virtual void UserSteppingAction(const G4Step *step);

private:
  bool inEcal( const G4Track *&trk );
  bool inHcal( const G4Track *&trk );
  bool inTcmt( const G4Track *&trk );

  const static G4int PI0 = 111;
  const static G4int ETA = 221;
  const static G4int PHOTON = 22;

  G4double _eEta;
  G4int    _nEta;
  G4double _ePi0;
  G4int    _nPi0;
  G4double _eEtaPhoton; //photons from eta decay
  G4int    _nEtaPhoton;

  G4double _hcal_eEta;
  G4int    _hcal_nEta;
  G4double _hcal_ePi0;
  G4int    _hcal_nPi0;
  G4double _hcal_eEtaPhoton; //photons from eta decay
  G4int    _hcal_nEtaPhoton;
  
  G4double _ecal_eEta;
  G4int    _ecal_nEta;
  G4double _ecal_ePi0;
  G4int    _ecal_nPi0;
  G4double _ecal_eEtaPhoton; //photons from eta decay
  G4int    _ecal_nEtaPhoton;
   
  G4double _tcmt_eEta;
  G4int    _tcmt_nEta;
  G4double _tcmt_ePi0;
  G4int    _tcmt_nPi0;
  G4double _tcmt_eEtaPhoton; //photons from eta decay
  G4int    _tcmt_nEtaPhoton;
   
  G4double _ePrimary;
  G4int    _tPrimary;

  G4double _fhi_z; //z coordinate of first hadronic interaction
  G4double _fhi_inEcal; //first hadronic interaction was in Ecal
  G4double _fhi_inHcal; //first hadronic interaction was in Hcal
  G4double _fhi_inTcmt; //first hadronic interaction was in Tcmt
  G4int    _fhi_pSubType; //first hadronic interaction process subType
  G4int    _fhi_nSec; //first hadronic interaction number of secondaries
  std::vector<G4int> _fhi_secPdgCode; // pdg codes of secondaries
  std::vector<G4double> _fhi_secEnergy; // total energies of secondaries

  bool  _fhio; //first hadronic interaction occured?
  G4int _stepno; //step number of primary particle
  G4int _lastindex; //internal variable for UserStepAction
};

#endif
