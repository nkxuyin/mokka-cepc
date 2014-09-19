// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: Checkplots.hh,v 1.2 2008/02/08 14:01:24 adrian Exp $
// $Name: mokka-07-00 $
//
// "Checkplots" is an example plugin for Mokka and AIDA.

#ifndef Checkplots_hh
#define Checkplots_hh 1

#include "Plugin.hh"

#ifdef G4ANALYSIS_USE
#include "AIDA/AIDA.h"
#endif

class Checkplots: public Plugin
{
public:
  Checkplots(const std::string &name): Plugin(name) {}
  virtual ~Checkplots() {}

  virtual void Init();
  virtual void Exit();

  virtual void BeginOfRunAction(const G4Run *run);
  virtual void EndOfRunAction(const G4Run *run);

  virtual void BeginOfEventAction(const G4Event *evt);
  virtual void EndOfEventAction(const G4Event *evt);

  virtual void PreUserTrackingAction(const G4Track *trk);
  virtual void PostUserTrackingAction(const G4Track *trk);

  virtual void UserSteppingAction(const G4Step *step);

protected:
  Checkplots();

#ifdef G4ANALYSIS_USE
  AIDA::IAnalysisFactory *_analysisFactory;
  AIDA::IHistogramFactory *_hFactory;
  AIDA::ITupleFactory *_tFactory;
  AIDA::ITree *_tree;

  AIDA::ICloud1D *_trkEnergyCloud ;
  AIDA::ICloud2D *_eDepVsZCloud ;
  AIDA::ICloud2D *_eDepVsRCloud ;
  AIDA::IHistogram1D *_eDepHist ;
  AIDA::IHistogram3D *_eDep3DHist ;
#endif
};  

#endif // Checkplots_hh
