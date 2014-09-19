// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: FindParents.cc,v 0.0 2011/01/31 M. Ramilli $
// $Name: mokka-07-02 $

#ifndef ShowerDecomposition_hh
#define ShowerDecomposition_hh 1

#include <map>
#include <vector>

#include "Control.hh"
#include "Plugin.hh"
#include "CalHit.hh"
#include "CGADefs.h"
#include "EncoderTB.hh"
//#include "../../Geometry/tbeam/include/TBhcal4d.hh"
//#include "../../Geometry/tbeam/include/TBhcal08.hh"
//#include "../../Geometry/tbeam/include/TBhcal07.hh"
//#include "../../Geometry/tbeam/include/TBSD_VCell4d.hh"
#include "CGAGeometryEnvironment.hh"
#include "G4EmSaturation.hh"

#include "IDG4TrackMapping.hh"

class ShowerDecompositionMessenger;

class ShowerDecomposition: public Plugin
{
public:
  ShowerDecomposition(const std::string &name): Plugin(name) {}
  ~ShowerDecomposition(void) {}

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
  bool     inEcal                    ( const G4Track *&trk   );
  bool     inHcalSD                  ( const G4Track *&trk   );
  bool     inTcmt                    ( const G4Track *&trk   );
  G4int    nHadronsFromFirstInelastic( const G4int hadronPDG );
  G4double GetBirksAttenuatedEnergy  ( const G4Step* aStep   );

  G4EmSaturation * emSaturation;

  G4double cellSize;
  G4int _detectorModel;

  std::map<G4int,G4int> _map_DaughtParent;//maps track ID into parent track ID  
  std::map<G4int,G4int> _map_TrackPDG;//maps track ID into corresponding PDG code
  std::map<G4int,IDG4TrackMapping> _map_IDG4Track;//maps G4track into corresponding track ID code


  //quantities relative to each hit are stored in vectors during the event
  std::vector<G4int> _trackIDs; //track ID of the energy deposition
  std::vector<G4int> _isFEM;//1 if there's pi0/eta in e+/e-/gamma history, 0 otherwise
  std::vector<G4int> _isNelastic;
  std::vector<G4int> _isNcapture;
  std::vector<G4int> _isNinelastic;
  std::vector<G4int> _hasNeutronAncestor;
  std::vector<G4int> _isNelasticProton;

  std::vector<G4double> _hitsEnergy; //energy deposited
  std::vector<G4double> _hitsTime; //time of deposition
  std::vector<G4int> _hitsPDG; //PDG of the particle that deposited energy
  std::vector<G4int> _hitsParentPDG; //PDG of the parent that deposited energy
  std::vector<G4int> _hitsGrandParentPDG;

  std::vector<G4float> _hitsPosX;
  std::vector<G4float> _hitsPosY;
  std::vector<G4float> _hitsPosZ;

  std::vector<G4int> _hitsPosI;
  std::vector<G4int> _hitsPosJ;
  std::vector<G4int> _hitsPosK;

  G4int _nProtonsFromFirstInelastic;
  G4int _nNeutronsFromFirstInelastic;
  G4int _nPionsFromFirstInelastic;


};

#endif
