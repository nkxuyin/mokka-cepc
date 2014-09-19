// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: TrackingOnlyPlugin.cc,v 1.3 2008/02/08 14:01:24 adrian Exp $
// $Name: mokka-06-06 $

#include <vector>
#include "TrackingOnlyPlugin.hh"
#include "PluginManager.hh"
#include "UserInit.hh"
#include "Control.hh"

#ifdef G4ANALYSIS_USE
#include "AIDA/AIDA.h"
using namespace AIDA;
#endif

INITPLUGIN(TrackingOnlyPlugin, "TrackingOnlyPlugin")

void TrackingOnlyPlugin::Init()
{
  std::cout << "#######################################################################################################" << std::endl;
  std::cout << "Using TrackingOnlyPlugin " << std::endl 
	    << "  Particles reaching the limits of the tracking volume will be stopped and not tracked any further." 
	    << std::endl 
	    << std::endl ;

  std::istringstream string_rad((*Control::globalModelParameters)["tracker_region_rmax"]);
  std::istringstream string_z((*Control::globalModelParameters)["tracker_region_zmax"]);

  string_rad >> tracking_radius_max;
  string_z >> tracking_z_max;

  std::cout << "Transverse   distance from the IP at which particles will be stopped = " << tracking_radius_max << " mm" << std::endl;
  std::cout << "Longitudinal distance from the IP at which particles will be stopped = " << tracking_z_max << " mm" << std::endl;

  std::cout << "#######################################################################################################" << std::endl;

}

void TrackingOnlyPlugin::Exit()
{
  //  std::cout << " in TrackingOnlyPlugin::Exit " << std::endl;
}

void TrackingOnlyPlugin::BeginOfRunAction(const G4Run *run)
{
  //  std::cout << " in TrackingOnlyPlugin::BeginOfRunAction " << run->GetRunID() << std::endl;
  run = 0; // avoid a compiler warning
}

void TrackingOnlyPlugin::EndOfRunAction(const G4Run *run)
{
  //  std::cout << " in TrackingOnlyPlugin::EndOfRunAction " << run->GetRunID() << std::endl;
  run = 0; // avoid a compiler warning
}

void TrackingOnlyPlugin::BeginOfEventAction(const G4Event *evt)
{
  //  std::cout << " in TrackingOnlyPlugin::BeginOfEventAction " << evt->GetEventID() << std::endl;
  evt = 0; // avoid a compiler warning
}

void TrackingOnlyPlugin::EndOfEventAction(const G4Event *evt)
{
  //  std::cout << " in TrackingOnlyPlugin::EndOfEventAction " << evt->GetEventID() << std::endl;
  evt = 0; // avoid a compiler warning
}

void TrackingOnlyPlugin::PreUserTrackingAction(const G4Track *trk)
{
  //  std::cout << " in TrackingOnlyPlugin::PreUserTrackingAction " << trk->GetTrackID() << std::endl;

  trk = 0; // avoid a compiler warning
}

void TrackingOnlyPlugin::PostUserTrackingAction(const G4Track *trk)
{
  //  std::cout << " in TrackingOnlyPlugin::PostUserTrackingAction " << trk->GetTrackID() << std::endl;
  trk = 0; // avoid a compiler warning
}

void TrackingOnlyPlugin::UserSteppingAction(const G4Step *step)
{
  //  std::cout << " in TrackingOnlyPlugin::UserSteppingAction -- track " << step->GetTrack()->GetTrackID() << std::endl;

  G4Track* theTrack  =  step->GetTrack(); 

  G4TouchableHandle touchPost = step->GetPostStepPoint()->GetTouchableHandle(); 
 
  const G4ThreeVector PostPosition = step->GetPostStepPoint()->GetPosition();


  const G4ThreeVector &postPos = step->GetPostStepPoint()->GetPosition();

  double radius_sqrd = postPos[0]*postPos[0] + postPos[1]*postPos[1];
  

  if(radius_sqrd > (tracking_radius_max*tracking_radius_max) ) { 
//    std::cout << "outside Tracking in R:" << tracking_radius_max << std::endl;
//    std::cout << "PID = " << step->GetTrack()->GetDefinition()->GetParticleName() << std::endl;     
//    std::cout << "Momentum = " << step->GetPostStepPoint()->GetMomentum() << std::endl;     
    theTrack->SetTrackStatus(fStopAndKill);
  }
  

  if(fabs(postPos[2]) > tracking_z_max ) {
//    std::cout << "outside Tracking in Z:" << tracking_z_max << std::endl;   
//    std::cout << "PID = " << step->GetTrack()->GetDefinition()->GetParticleName() << std::endl;     
//    std::cout << "Momentum = " << step->GetPostStepPoint()->GetMomentum() << std::endl;     
    theTrack->SetTrackStatus(fStopAndKill);
  }
  
}
