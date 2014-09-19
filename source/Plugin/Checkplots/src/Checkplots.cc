// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: Checkplots.cc,v 1.3 2008/02/08 14:01:24 adrian Exp $
// $Name: mokka-07-00 $

#include "Checkplots.hh"
#include "PluginManager.hh"
#include "UserInit.hh"

#ifdef G4ANALYSIS_USE
#include "AIDA/AIDA.h"
using namespace AIDA;
#endif

INITPLUGIN(Checkplots, "Checkplots")

void Checkplots::Init()
{
#ifdef G4ANALYSIS_USE
  _analysisFactory = AIDA_createAnalysisFactory();
  if (_analysisFactory != 0) {
    ITreeFactory *treeFactory = _analysisFactory->createTreeFactory();

    std::string fileName = UserInit::getInstance()->getString("CheckplotFileName");
    if (fileName.empty())
      fileName = "Checkplots.aida";
    
    _tree = treeFactory->create(fileName, "xml", false, true, "compress=yes");

    _hFactory = _analysisFactory->createHistogramFactory(*_tree);
    _tFactory = _analysisFactory->createTupleFactory(*_tree);
    delete treeFactory; // Will not delete the ITree.

    _trkEnergyCloud = _hFactory->createCloud1D("Energy of all tracked Particles [MeV]");

    _eDepVsZCloud = _hFactory->createCloud2D("Deposited energy [MeV] in rz-plane [mm]");
    _eDepVsRCloud = _hFactory->createCloud2D("Deposited energy [MeV] in xy-plane [mm]");
    _eDepHist = _hFactory->createHistogram1D("Deposited energy [keV]", 160, 0.0, 160.0);
    _eDep3DHist = _hFactory->createHistogram3D("Deposited energy in x, y, z [cm]",
      18, -450.0, 450.0, 18, -450.0, 450.0, 18, -450.0, 450.0);
  } else {
    G4cout << this->getName() << ": Error: Couldn't create analysisFactory for AIDA!" << G4endl;
  }
#else
  G4cout << this->getName() << ": Warning: Mokka has not been built with AIDA support." << G4endl;
#endif
}

void Checkplots::Exit()
{
#ifdef G4ANALYSIS_USE
  // write the tree to disc
  _tree->commit();
#endif
}

void Checkplots::BeginOfRunAction(const G4Run *)
{
  //std::cout << " in BeginOfRunAction " << run->GetRunID() << std::endl;
}

void Checkplots::EndOfRunAction(const G4Run *)
{
  //std::cout << " in EndOfRunAction " << run->GetRunID() << std::endl;
}

void Checkplots::BeginOfEventAction(const G4Event *)
{
  //std::cout << " in BeginOfEventAction " << evt->GetEventID() << std::endl;
}

void Checkplots::EndOfEventAction(const G4Event *)
{
  //std::cout << " in EndOfEventAction " << evt->GetEventID() << std::endl;
}

void Checkplots::PreUserTrackingAction(const G4Track *trk)
{
  //std::cout << " in PreUserTrackingAction " << trk->GetTrackID() << std::endl;

#ifdef G4ANALYSIS_USE
  _trkEnergyCloud->fill(trk->GetTotalEnergy() / MeV);
#endif
  trk = 0; // avoid a compiler warning in case we're not using AIDA
}

void Checkplots::PostUserTrackingAction(const G4Track *)
{
  //std::cout << " in PostUserTrackingAction " << trk->GetTrackID() << std::endl;
}

void Checkplots::UserSteppingAction(const G4Step *step)
{
  //std::cout << " in UserSteppingAction -- track " << step->GetTrack()->GetTrackID() << std::endl;

#ifdef G4ANALYSIS_USE
  // hit is deposited in the middle of the step
  const G4ThreeVector &prePos = step->GetPreStepPoint()->GetPosition();
  const G4ThreeVector &postPos = step->GetPostStepPoint()->GetPosition();
  const G4ThreeVector pos = (prePos + postPos) / 2.0;

  const double eDep = step->GetTotalEnergyDeposit();

  _eDepVsZCloud->fill(pos.z() / mm, pos.rho() / mm, eDep / MeV);
  _eDepVsRCloud->fill(pos.x() / mm, pos.y() / mm, eDep / MeV);
  
  _eDepHist->fill(eDep / keV);
  _eDep3DHist->fill(pos.x() / mm, pos.y() / mm, pos.z() / mm, eDep / MeV);
#endif

  step = 0; // avoid a compiler warning in case we're not using AIDA
}
