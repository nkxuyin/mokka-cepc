// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: SteppingAction.cc,v 1.11 2006/09/13 17:11:03 adrian Exp $
// $Name: mokka-07-00 $

#include "SteppingAction.hh"
#include "SteppingActionMessenger.hh"

#include "G4VVisManager.hh"
#include "G4Polyline.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "G4SteppingManager.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "G4Trajectory.hh"

#include "Control.hh"
#include "UserTrackInformation.hh"
#include "PluginManager.hh"
#include "CGAGeometryManager.hh"

SteppingAction::SteppingAction()
  :drawFlag(false), MaxStepNumber(100000)
{
  theSteppingActionMessenger = new SteppingActionMessenger((SteppingAction*)this);
}

SteppingAction::~SteppingAction()
{ 
	delete theSteppingActionMessenger;
}

void SteppingAction::UserSteppingAction(const G4Step* aStep)
{
  // static objects which are allocated only once per program run
  static G4VisAttributes *visAttributesRed   = new G4VisAttributes(G4Colour::Red());
  static G4VisAttributes *visAttributesGreen = new G4VisAttributes(G4Colour::Green());
  static G4VisAttributes *visAttributesBlue  = new G4VisAttributes(G4Colour::Blue());
  
  if( Control::stepNumber++ > MaxStepNumber ) 
    {
      if(Control::PrintLevel > 1)
	G4cout << "MOKKA WARNING: number of steps > " << MaxStepNumber 
	       << ", this track will be killed right now." << G4endl;
      fpSteppingManager->GetTrack()->SetTrackStatus(fStopAndKill);
    }
  
  // Turn around to Geant4 strange behavior
  // when filling G4Track in the begin of Track. 
  // Filled here first step and used perhaps
  // by TrackSummary::update method.
  if (Control::stepNumber == 1)
    {
      Control::VertexPosition = 
	aStep->GetPreStepPoint()->GetPosition();
      Control::VertexEnergy = 
	aStep->GetPreStepPoint()->GetKineticEnergy();
      Control::VertexMomentum = 
	aStep->GetPreStepPoint()->GetMomentum();
    }
  
  // if drawing steps on the screen, draw it.
  if(drawFlag)
    {
      G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
      if (pVVisManager) {
	G4double charge = fpSteppingManager->GetTrack()->GetDefinition()->GetPDGCharge();
	
	//if(charge == 0.) return;
	
        G4VisAttributes *visAttributes;
        if      (charge < 0) visAttributes = visAttributesRed;
        else if (charge > 0) visAttributes = visAttributesBlue;
        else                 visAttributes = visAttributesGreen;
	
#ifdef CIRCLES
	G4Circle circle;
	circle.SetVisAttributes(visAttributes); 
	circle.SetPosition(aStep->GetPreStepPoint()->GetPosition());
	circle.SetScreenDiameter (1.0); 
	circle.SetFillStyle (G4Circle::filled);
	pVVisManager->Draw(circle);
#endif
	
#define POLYLINE
#ifdef POLYLINE
	G4Polyline polyline;
	polyline.SetVisAttributes(visAttributes);
	polyline.push_back(aStep->GetPreStepPoint()->GetPosition());
	polyline.push_back(aStep->GetPostStepPoint()->GetPosition());
	pVVisManager -> Draw(polyline);
#endif
      }
    }
  
  // If generator primary just returns
  if(fpSteppingManager->GetTrack()->GetParentID()==0)
    {
      PluginManager::getInstance()->UserSteppingAction( aStep ) ;
      return;
    }
  
  // If secondary, test if it's back scattering
  CGAGeometryManager* theCGAGeometryManager =
    CGAGeometryManager::GetCGAGeometryManager();
  
  G4ThreeVector position =
    aStep->GetPreStepPoint()->GetPosition();
  
  G4bool PrePointInsideTrackerRegion =
    theCGAGeometryManager->
    GetTrackerRegionRmax() > position.perp() 
    &&
    theCGAGeometryManager->
    GetTrackerRegionZmax() > std::fabs(position.z());

  position = aStep->GetPostStepPoint()->GetPosition();

  G4bool PostPointInsideTrackerRegion =
    theCGAGeometryManager->
    GetTrackerRegionRmax() > position.perp() 
    &&
    theCGAGeometryManager->
    GetTrackerRegionZmax() > std::fabs(position.z());
  
  // If back scattering, flag it in the TrackSummary and suspend 
  // the track, to force to be postponed for the end of 
  // the shower development by the StackingAction.

  // AZ: We have to save Vertex information before we suspend back scattered
  // track, otherwise it will be lost. I do this by calling update()
  // before BackScattered mark (so, I can detect this in update).

  if(!(PrePointInsideTrackerRegion) && PostPointInsideTrackerRegion)
    {
      UserTrackInformation* theUserTrackInformation =
	(UserTrackInformation*) (fpSteppingManager->
				 GetTrack()->
				 GetUserInformation());
      
      if(theUserTrackInformation){
	theUserTrackInformation->GetTheTrackSummary()->
	  update(fpSteppingManager->GetTrack());
	theUserTrackInformation->
	  GetTheTrackSummary()->SetBackScattering();
      }else
	{
	  TrackSummary* theTrackSummary = NULL;
	  Control::GetControl()->
	    GetTrackedtracks()->
	    push_back(theTrackSummary =new 
		      TrackSummary(fpSteppingManager->
				   GetTrack(),
				   false,
				   false));
	  ((G4Track*) fpSteppingManager->
	   GetTrack())->
	    SetUserInformation(new 
			       UserTrackInformation(theTrackSummary));
	  theTrackSummary->update(fpSteppingManager->GetTrack());
	  theTrackSummary->SetBackScattering();
	}
      fpSteppingManager->GetTrack()->SetTrackStatus(fSuspend);
    }

  PluginManager::getInstance()->UserSteppingAction( aStep ) ;
}
