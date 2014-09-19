//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for Tesla -      *
//*                                                     *
//* For more information about Mokka, please, go to the *
//*                                                     *
//*  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
//*                                                     *
//* Mokka home page.                                    *
//*                                                     *
//*******************************************************
//
// $Id: TrackingAction.cc,v 1.12 2008/05/16 12:05:36 musat Exp $
// $Name: mokka-07-00 $
//

#include "G4Track.hh"
#include "G4TrackingManager.hh"
#include "G4ParticleTable.hh"
#include "Control.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "G4Trajectory.hh"
#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"

#include "TrackingAction.hh"
#include "UserTrackInformation.hh"
#include "TrackSummary.hh"
#include "PluginManager.hh"
#include "CGAGeometryManager.hh"

TrackingAction::TrackingAction() 
{
  Control::stepNumber = 0;
}

void 
TrackingAction::PreUserTrackingAction(const G4Track* aTrack)
{  
  // resets the step loop control.
  Control::stepNumber = 0;
  
  // Retrieves the TrackSummary if postponed back scattering.
  TrackSummary* theTrackSummary = NULL;

  UserTrackInformation* theUserTrackInformation =
    (UserTrackInformation*) (aTrack->GetUserInformation());

  // If theUserTrackInformation exists nothing to do except
  // to resets the PID for Cal Hit if it's a back scatter.
  if(theUserTrackInformation) 
    {
      // If back postponed back scattering, resets the PID for Cal Hit
      if(theUserTrackInformation->GetTheTrackSummary()->GetBackScattering())
	Control::GetControl()->ResetPIDForCalHit(aTrack->GetTrackID());
      PluginManager::getInstance()->PreUserTrackingAction( aTrack ) ;
      // TAKE NOT OF THIS RETURN HERE.
      return;
    }
  
  // If theUserTrackInformation doesn't exist yet lets decide
  // if it has to created right now.
  //
  // if GetParentID()==0 it's a Generator Primary
  G4bool TruePrimary = aTrack->GetParentID()==0;
  G4bool DecayInsideTracker = false;

  // if it's not a generator primary, test if it's a decay
  // inside the tracker region.
  G4bool InsideTrackerRegion = false;
  if(!TruePrimary)
    {      
      CGAGeometryManager* theCGAGeometryManager =
	CGAGeometryManager::GetCGAGeometryManager();
      
      G4ThreeVector vertex =
	aTrack->GetPosition();

      InsideTrackerRegion =
	theCGAGeometryManager->
	GetTrackerRegionRmax() > vertex.perp() 
	&&
	theCGAGeometryManager->
	GetTrackerRegionZmax() > std::fabs(vertex.z());
      
      // G4cout << " ************** PreUserTrackingAction : trackerRMax=" << theCGAGeometryManager->GetTrackerRegionRmax()  
      //  	     << " trackerZMax = " << theCGAGeometryManager->GetTrackerRegionZmax()  << "  InsideTrackerRegion = " << InsideTrackerRegion
      //  	     << " vertex = " << vertex 
      // 	     << "   track " << aTrack->GetTrackID() << std::endl ;

      if(InsideTrackerRegion) 
	{
	  const G4VProcess* theCreatorProcess =
	    aTrack->GetCreatorProcess();
	  
	  if(theCreatorProcess)
	    DecayInsideTracker = 
	      theCreatorProcess->GetProcessName() == "Decay";
	}
    }
  
  // If a generator primary or a decay inside the tracker 
  // region:
  // - reset the PID for Cal hits
  // - keep the trajectory
  // - create its initial TrackSummary
  //
  //  Following a Predrag Krstonosic argument that : 
  //  "... since GetProcessName() == "Decay"; does not take into
  //  account conversion and processes where mother continues to 
  //  live after interaction, which were also considered "decay" 
  //  according to Ron Cassels proposal."
  //  
  //  in mokka-05-00 we replaced 
  //  if( TruePrimary || DecayInsideTracker )
  //  by:
  if( TruePrimary || ( InsideTrackerRegion &&
		       aTrack->GetKineticEnergy()>Control::TPCCut)  ) 
    {
      // reset the PID for Cal hits
      Control::GetControl()->ResetPIDForCalHit(aTrack->GetTrackID());

      // keep the trajectory
      Control::TrackingPrimary=true;
      if(Control::SavingPrimaries || Control::SavingTrajectories)
	fpTrackingManager->SetStoreTrajectory(true);
      
      // book the particle in the MCParticleList
      if(!theUserTrackInformation)
	{
	  Control::GetControl()->
	    GetTrackedtracks()->
	    push_back(theTrackSummary = 
		      new TrackSummary(aTrack,true) );
	  ((G4Track*) aTrack)->
	    SetUserInformation(new 
			       UserTrackInformation(theTrackSummary));
	}
    }
  else 
    // if not, stop keeping trajectory and leave
    {
      // PK  track summary is created for all particles that are created in 
      // tracking volume but with toBeSaved false only those that make hits
      // change the status to toBeSaved=true in TRKSD00
      if( InsideTrackerRegion &&  aTrack->GetKineticEnergy()<Control::TPCCut)
	{
	  if(!theUserTrackInformation)
	    {
	      Control::GetControl()->
		GetTrackedtracks()->
		push_back(theTrackSummary = 
			  new TrackSummary(aTrack,false) );
	      ((G4Track*) aTrack)->
		SetUserInformation(new 
				   UserTrackInformation(theTrackSummary));
	    }

	}else{
         
	Control::TrackingPrimary=false;
	fpTrackingManager->SetStoreTrajectory(false);
      } 
    }
  //GM: add corrections suggested by Steve and Frank
  G4FieldManager* fieldMgr
    = G4TransportationManager::GetTransportationManager()
    ->GetFieldManager();

  if(TruePrimary || InsideTrackerRegion) {
    if(Control::UserDeltaIntersection > 0.)
      fieldMgr->SetDeltaIntersection(Control::UserDeltaIntersection);
    if(Control::UserDeltaOneStep > 0.)
      fieldMgr->SetDeltaOneStep(Control::UserDeltaOneStep);
  }else {
    fieldMgr->SetDeltaIntersection(Control::G4DefaultDeltaIntersection);
    fieldMgr->SetDeltaOneStep(Control::G4DefaultDeltaOneStep);
  }

  PluginManager::getInstance()->PreUserTrackingAction( aTrack ) ;
}

void TrackingAction::PostUserTrackingAction(const G4Track* aTrack)
{
  
  // If no persistency, exit!
  if( Control::PersistencyMode == mokka::NONE ){
    PluginManager::getInstance()->PostUserTrackingAction( aTrack ) ;
    return;
  }

  // MCParticle list manageament: if the track is not
  // yet in the Trackedtracks control list 
  // it should be added to the list if:
  //  1) it satisfy the secondary cut parameters.
  //  or
  //  2) it has secondaries, because perhaps one of 
  // these secondaries could touch the calorimeter.
  // In this case the tobesaved flag is set 
  // "false" in such way it'll dropped 
  // later if there is no use.

  UserTrackInformation* theUserTrackInformation =
    (UserTrackInformation*) (aTrack->GetUserInformation());
  
  TrackSummary* theTrackSummary = NULL;
  if(theUserTrackInformation)
    theTrackSummary = 
      theUserTrackInformation->GetTheTrackSummary();

  // If already booked, update it and returns. Perhaps 
  // it'll change in the future, but the only information
  // for the moment in the theUserTrackInformation
  // object is a TrackSummary pointer. So if it's
  // already set it means that the current track is
  // already booked.
  if(theTrackSummary) 
    {
      theTrackSummary->update(aTrack);
      PluginManager::getInstance()->PostUserTrackingAction( aTrack ) ;
      return;
    }

  // So it's not. Let's see if it has secondaries,
  // if any book it with tobesaved = false.
  
  if(fpTrackingManager->GimmeSecondaries()->size() > 0)
    {
      theTrackSummary = 
	new TrackSummary(aTrack,false);
      Control::GetControl()->
	GetTrackedtracks()->
	push_back( theTrackSummary );
      theTrackSummary->update(aTrack);
    }

  PluginManager::getInstance()->PostUserTrackingAction( aTrack ) ;
}
