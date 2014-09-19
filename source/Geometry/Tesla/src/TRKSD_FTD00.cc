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
// $Id: TRKSD_FTD00.cc,v 1.10 2010/10/26 11:42:12 duarte Exp $
// $Name: mokka-07-00 $
// 
// Based on the TRDSD00.cc; minor changes in order to 
// recognize the new hierarchy of the FTD volumes. As I don't
// know if the TRDSD00 is used by any other silicon detector and
// to avoid possible conflict with the changes needed for the FTD,
// we are decided to create this separate class. Is possible to
// improve this class if FTD will be its only client. It is possible
// also to create a base class for all the silicon detectors (if
// they share enough properties) developing this class as derived class.
// (J. Duarte Campderros, October 2010)
// 
#include "Control.hh"
#include "TRKSD_FTD00.hh"

// NEW 
#include "EncoderSi.hh"

#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4VPhysicalVolume.hh"
#include "G4TransportationManager.hh"
#include "G4TouchableHistory.hh"
#include "G4AffineTransform.hh"

#include "G4SDManager.hh"
#include "G4ios.hh"
#include "globals.hh"
#include <assert.h>
    //dodao PK
#include "UserTrackInformation.hh"
#include "TrackSummary.hh"

TRKSD_FTD00::TRKSD_FTD00(G4String SDname, 
		 G4double pThreshold,
		 G4double thePrimaryTPCCut) 
  : VSensitiveDetector(SDname),Threshold(pThreshold),
    PrimaryTPCCut(thePrimaryTPCCut),
    HCID(-1), currentCylinder(0), currentPID(-1), 
    currentPDG(-1), currentSecondaryPID(-1),
    EntryPoint(0.,0.,0.), ExitPoint(0.,0.,0.),
    EntryMomentum (0.,0.,0.), ExitMomentum (0.,0.,0.),
    DepositedEnergy(0.), HitTime(0.), StepLength(0.), 
    CalCollection(0)
{
  G4String CollName=SDname+"Collection";
  collectionName.insert(CollName);
}

void TRKSD_FTD00::Initialize(G4HCofThisEvent *)
{
  CalCollection = new TRKHitsCollection
    (SensitiveDetectorName,collectionName[0]);
  currentCylinder = 0;
  currentPID = -1;
  currentSecondaryPID = -1;
}

G4bool TRKSD_FTD00::ProcessHits(G4Step *aStep,G4TouchableHistory*)
{
  // It's a first approach for hit's processing for all the tracking
  // detectors (VXD, SIT, FTD and TPC). In this first release, if
  // the total energy deposited by a primary and its secondaries
  // is upper then a given threshould it keeps:
  // 
  // o the layer number (the plan number for the FTD)
  // o the mean step position when crossing the layer
  // o the mean momentum when crossing the layer
  // o the primary PID number
  // o the PDG particle code (it can be the secondary one)
  // o the total energy deposited when crossing the layer
  //
  // The current  
  //
  // (Paulo Sept. 2002)
  //

  // Just for particles with more than the PrimaryTPCCut
  // given at constructor call (to control TPC output
  // file length).
  if(aStep->GetPreStepPoint()->GetKineticEnergy() 
     < PrimaryTPCCut) return true;

//NEW: FTD
  // FTD subdet = 6
  EncoderSi theEncoder; //FIXME: El side es del touchable!! Hay que cambiar la clase EncoderSi!
  G4int PrelayerNumber = theEncoder.encode( aStep->GetPreStepPoint()->GetTouchable() );

/*  G4int PrelayerNumber = 
    aStep->GetPreStepPoint()->
    GetPhysicalVolume()->GetCopyNo(); OLD*/
  
  G4StepPoint* postStep = aStep->GetPostStepPoint();
  G4VPhysicalVolume * physVol = postStep->GetPhysicalVolume();
  if(physVol == 0) 
    {	
      G4cout << "WARNING: TRKSiD00::ProcessHits: post step point physical volume pointer is null!!!\n"
	     << "It's a Geant4 bug. TRKSD will skip this hit to avoid aborting the job!" << G4endl;
      return true;
    }

//NEW: FTD
G4int PostlayerNumber = theEncoder.encode( aStep->GetPostStepPoint()->GetTouchable() );
  //G4int PostlayerNumber = physVol->GetCopyNo(); OLD
  /*
  G4int PostlayerNumber = 
    aStep->GetPostStepPoint()->
    GetPhysicalVolume()->GetCopyNo();
  */
  //#define DEBUGTRKSD
#ifdef DEBUGTRKSD
  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  G4cout << GetName()
	 << ", PrelayerNumber = " << PrelayerNumber
	 << " PreStepPoint = " << aStep->GetPreStepPoint()->GetPosition()
	 << ", PostlayerNumber = " << PostlayerNumber
	 << " PostStepPoint = " << aStep->GetPostStepPoint()->GetPosition()
	 << G4endl;
  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif

  // If new primary tracking or another secondary
  // dump and reset the counters.
  if ( (Control::primaryId != currentPID) ||
       (aStep->GetTrack()->GetTrackID() !=
	currentSecondaryPID) )
    {
      DumpHit(aStep); 
      Clear(); // forces to start a new hit
    }
  // First case, starting traversal
  if(PrelayerNumber != currentCylinder)
    {
      // dump and start a new hit.
      DumpHit(aStep); 
      StartNewHit(PrelayerNumber,
		  aStep->GetPreStepPoint()->GetPosition(),
		  aStep->GetPreStepPoint()->GetMomentum(),
		  aStep->GetTrack()->GetDefinition()->GetPDGEncoding(),
		  aStep->GetTrack()->GetTrackID());
      // take the energy and the exit momentum
      UpdateHit(aStep);
      
      // Perhaps it's already on the next boundary:
      if(PrelayerNumber != PostlayerNumber)
	{
	  // So dump and start a new hit if the layers
	  // share surfaces (the case of TPC)
	  DumpHit(aStep);
	  if( PostlayerNumber!= 0)
	    StartNewHit(PostlayerNumber,
			aStep->GetPostStepPoint()->GetPosition(),
			aStep->GetPostStepPoint()->GetMomentum(),
			aStep->GetTrack()->GetDefinition()->GetPDGEncoding(),
			aStep->GetTrack()->GetTrackID());
	  else
	    // If the layers don't share surfaces, reset the counters.
	    Clear();
	}
      
      // PAY ATTENTION TO THE RETURN HERE!
      return true;
    }

  // Second case, traveling and perhaps on the next boundary
  // add energy and update the exit momentum.
  // We test here if currentCylinder !=0 just to be sure, it should 
  // never happens except if the user plugged the TRKSD in a zero
  // numbered layer.
  if( currentCylinder !=0 ) UpdateHit(aStep);

  // Is it on the next boundary?
  if(PrelayerNumber != PostlayerNumber) 
    {
      // Yes, dump the Hit and perhaps start a new one, if the
      // layers share surfaces (the TPC case)
      DumpHit(aStep);
      if( PostlayerNumber!= 0) 
	StartNewHit(PostlayerNumber,
		    aStep->GetPostStepPoint()->GetPosition(),
		    aStep->GetPostStepPoint()->GetMomentum(),
		    aStep->GetTrack()->GetDefinition()->GetPDGEncoding(),
		    aStep->GetTrack()->GetTrackID());
      else 
	// Else clear the counters.
	Clear();
    }
    else if (aStep->GetTrack()->GetTrackStatus() == fStopAndKill) 
    {
        DumpHit(aStep);
        Clear();
    }
  
  return true;
}

void 
TRKSD_FTD00::EndOfEvent(G4HCofThisEvent*HCE)
{
  if(HCID<0)
    { HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]); }
  HCE->AddHitsCollection( HCID, CalCollection );
}

void 
TRKSD_FTD00::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
  TRKHit* newHit = new TRKHit();
  while (newHit->Load(theSubDetectorEventHitsFileInput))
    {
      CalCollection->insert(newHit);
      newHit = new TRKHit();
    }
  delete newHit;
}


void TRKSD_FTD00::DrawAll()
{
} 

void TRKSD_FTD00::PrintAll()
{
} 

void TRKSD_FTD00::DumpHit(G4Step* aStep)
{
  // If currentCylinder==0 there is nothing to dump
  if(currentCylinder == 0) return;

  // It keeps the hit only if the total deposited energy
  // is upper then the given Threshold.
  if( DepositedEnergy < Threshold ) return;

  G4ThreeVector MiddlePoint = 
    ( EntryPoint + ExitPoint ) / 2.;
  
  G4ThreeVector MeanMomentum = 
    ( ExitMomentum + EntryMomentum ) / 2.;

  StepLength += aStep->GetStepLength();

  //PK: fix for delta electrons: all particles causing hits
  // have to be saved in the LCIO file
   UserTrackInformation* theUserTrackInformation =
    (UserTrackInformation*) (aStep->GetTrack()->GetUserInformation());
  
 
   if(theUserTrackInformation) {
     
     theUserTrackInformation->GetTheTrackSummary()->SetToBeSaved();
   }

  CalCollection->
    insert(new TRKHit (currentCylinder,
		       MiddlePoint (0),
		       MiddlePoint (1),
		       MiddlePoint (2),
		       MeanMomentum (0),
		       MeanMomentum (1),
		       MeanMomentum (2),
		       currentPID,
		       currentPDG,
		       DepositedEnergy, 
		       HitTime,
		       StepLength));

#ifdef DEBUGTRKSD
  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  G4cout << GetName()
	 << "====>DumpHit, currentCylinder = " << currentCylinder
	 << ", MiddlePoint(0) = " << MiddlePoint(0)
	 << ", MiddlePoint(1) = " << MiddlePoint(1)
	 << ", EntryPoint = " << EntryPoint
	 << ", ExitPoint = " << ExitPoint
	 << G4endl;
  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif
}

void TRKSD_FTD00::StartNewHit(G4int aLayerNumber,
			  G4ThreeVector theEntryPoint,
			  G4ThreeVector theEntryMomentum,
			  G4int thePDG,
			  G4int theSecondaryPID)
{      
  currentCylinder = aLayerNumber;
  currentPID = Control::primaryId;
  currentPDG = thePDG;
  currentSecondaryPID = theSecondaryPID;
  EntryPoint = ExitPoint = theEntryPoint;
  EntryMomentum = ExitMomentum = theEntryMomentum;
  DepositedEnergy = 0.;
  HitTime = 0.;
  StepLength = 0.;
}

void TRKSD_FTD00::Clear()
{
  currentCylinder = 0;
  currentPID = -1; 
  currentSecondaryPID = -1;
  DepositedEnergy = 0;
  HitTime = 0. ;
  StepLength  = 0. ;
}

void TRKSD_FTD00::UpdateHit(G4Step *aStep)
{
  DepositedEnergy+=aStep->GetTotalEnergyDeposit();
  HitTime = aStep->GetTrack()->GetGlobalTime() ; 
  ExitPoint=aStep->GetPostStepPoint()->GetPosition();
  ExitMomentum=aStep->GetPostStepPoint()->GetMomentum();
  StepLength += aStep->GetStepLength();
}
