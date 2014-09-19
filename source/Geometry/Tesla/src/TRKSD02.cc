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
//
#include "Control.hh"
#include "TRKSD02.hh"

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

#include <UTIL/BitField64.h>
#include <UTIL/ILDConf.h>

#include "UserTrackInformation.hh"
#include "TrackSummary.hh"

TRKSD02::TRKSD02(G4String SDname, 
		 G4double pThreshold,
		 G4double tKineticEnergyCut)
  : VSensitiveDetector(SDname),Threshold(pThreshold),KineticEnergyCut(tKineticEnergyCut),
    HCID(-1), currentCopyNumber(0), currentCellID0(0), currentPID(-1), 
    currentPDG(-1),
    EntryPoint(0.,0.,0.), ExitPoint(0.,0.,0.),
    EntryMomentum (0.,0.,0.), ExitMomentum (0.,0.,0.),
    DepositedEnergy(0.), HitTime(0.), StepLength(0.), 
    HitCollection(0)
{
  G4String CollName=SDname+"Collection";
  collectionName.insert(CollName);
}

void TRKSD02::Initialize(G4HCofThisEvent *)
{
  HitCollection = new TRKHitsCollection
    (SensitiveDetectorName,collectionName[0]);
  currentCopyNumber = 0;
  currentCellID0 = 0;
  currentPID = -1;

}

G4bool TRKSD02::ProcessHits(G4Step *aStep,G4TouchableHistory*)
{
  // Approach for hit processing for the tracking
  // detectors. If the total energy deposited is greater 
  // than a given threshold then the following is 
  // stored:
  // 
  // o the copy number of traversed volume 
  // o the mean step position when crossing the layer
  // o the mean momentum when crossing the layer
  // o the primary PID number
  // o the PDG particle code (it can be the secondary one)
  // o the total energy deposited when crossing the layer
  //
  //

  // Only particles with more KineticEnergy than KineticEnergyCut will be treated
  // Note "return" statement 

//#define DEBUGTRKSD 1


#ifdef DEBUGTRKSD
  G4cout << GetName()
	 << " KE = " <<  aStep->GetPreStepPoint()->GetKineticEnergy() 
    	 << " KE cut = " <<  KineticEnergyCut 
	 << G4endl;
#endif 

  if(aStep->GetPreStepPoint()->GetKineticEnergy() 
     < KineticEnergyCut) return true;

  if (fabs(aStep->GetTrack()->GetDefinition()->GetPDGCharge()) < 0.01) return true;
  
  G4int PreStepCopyNumber = 
    aStep->GetPreStepPoint()->
    GetPhysicalVolume()->GetCopyNo();
  
  G4StepPoint* postStep = aStep->GetPostStepPoint();
  G4VPhysicalVolume * physVol = postStep->GetPhysicalVolume();
  if(physVol == 0) 
    {	
      G4cout << "WARNING: TRKSiD00::ProcessHits: post step point physical volume pointer is null!!!\n"
	     << "It's a Geant4 bug. TRKSD will skip this hit to avoid aborting the job!" << G4endl;
      return true;
    }

  G4int PostStepCopyNumber = physVol->GetCopyNo();


  int module(0);
  int sensor(0);
  
  UTIL::BitField64 encoder( ILDCellID0::encoder_string ) ; 
  
  G4int copyNumber = 0 ;
  G4int Cellid0 = 0 ;
  for( int i = 0 ; i < aStep->GetPreStepPoint()->GetTouchableHandle()->GetHistoryDepth() ; ++i ){

    copyNumber = aStep->GetPreStepPoint()->GetTouchableHandle()->GetCopyNumber(i) ;

    encoder.setValue(copyNumber) ;

    //SJA:FIXME: As side is only able to have values of +1 -1 and 0 here cannot use the n-1 trick?
    //           So we will just have to make sure that the side is always in the top level volume with should not be too hard. 
    //           Layer should also be store in the top level copy number
    if (encoder[ILDCellID0::module] != 0) module = encoder[ILDCellID0::module] ;
    if (encoder[ILDCellID0::sensor] != 0) sensor = encoder[ILDCellID0::sensor] ;

    
    if( encoder[ILDCellID0::subdet] != ILDDetID::NOTUSED ) {
      
      encoder[ILDCellID0::layer] = encoder[ILDCellID0::layer] - 1;
      encoder[ILDCellID0::module] = module - 1;
      encoder[ILDCellID0::sensor] = sensor - 1;
      
#ifdef DEBUGTRKSD
      //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
      G4cout << "Detector = " << encoder[ILDCellID0::subdet] << G4endl ;
      G4cout << "Layer = " << encoder[ILDCellID0::layer] << G4endl ;
      G4cout << "Module = " << encoder[ILDCellID0::module] << G4endl ;
      G4cout << "Sensor = " << encoder[ILDCellID0::sensor] << G4endl ;
      //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif      
      break ;
    }

    }
  
  Cellid0 = encoder.lowWord();

  /*
  G4int PostStepCopyNumber = 
    aStep->GetPostStepPoint()->
    GetPhysicalVolume()->GetCopyNo();
  */

#ifdef DEBUGTRKSD

  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  G4cout << GetName()
	 << ", PreStepCopyNumber = " << PreStepCopyNumber
	 << " PreStepPoint = " << aStep->GetPreStepPoint()->GetPosition()
	 << ", PostStepCopyNumber = " << PostStepCopyNumber 
	 << " PostStepPoint = " << aStep->GetPostStepPoint()->GetPosition()
	 << " copyNumber = " << copyNumber 
   << " Cellid0 = " << Cellid0
	 << G4endl;
  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif

  // If new primary tracking or another secondary
  // dump and reset the counters.
  if ( aStep->GetTrack()->GetTrackID() != currentPID ) {
    DumpHit(aStep); 
  }
  
  
  
  // First case, starting traversal
  if(PreStepCopyNumber != currentCopyNumber) {
    // dump and start a new hit.
    DumpHit(aStep); 
    
    StartNewHit(copyNumber, Cellid0,
                aStep->GetPreStepPoint()->GetPosition(),
                aStep->GetPreStepPoint()->GetMomentum(),
                aStep->GetTrack()->GetDefinition()->GetPDGEncoding(),
                aStep->GetTrack()->GetTrackID());
    
    // take the energy and the exit momentum
    UpdateHit(aStep);
    
    // Perhaps it's already on the next boundary:
    if(PreStepCopyNumber != PostStepCopyNumber){

      // So dump and start a new hit if the layers
      // share surfaces (the case of TPC)
      DumpHit(aStep);
      
    }
    
    // PAY ATTENTION TO THE RETURN HERE!
    return true;
  }

  // Second case, traveling and perhaps on the next boundary
  // add energy and update the exit momentum.
  // We test here if currentCopyNumber !=0 just to be sure, it should 
  // never happens except if the user plugged the TRKSD in a zero
  // numbered layer.
  if( currentCopyNumber !=0 ) UpdateHit(aStep);

  
  // Is it on the next boundary?
  if(PreStepCopyNumber != PostStepCopyNumber) {
    // Yes, dump the Hit and perhaps start a new one, if the
    // layers share surfaces (the TPC case)
    DumpHit(aStep);

  } else if (aStep->GetTrack()->GetTrackStatus() == fStopAndKill) {
    DumpHit(aStep);
  }

  return true;

}

void 
TRKSD02::EndOfEvent(G4HCofThisEvent*HCE)
{
  if(HCID<0)
    { HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]); }
  HCE->AddHitsCollection( HCID, HitCollection );
}

void 
TRKSD02::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
}


void TRKSD02::DrawAll()
{
} 

void TRKSD02::PrintAll()
{
} 

void TRKSD02::DumpHit(G4Step* aStep)
{
  // If currentCopyNumber==0 there is nothing to dump
  if(currentCopyNumber == 0) return;

  // It keeps the hit only if the total deposited energy
  // is greater than the given Threshold.
  if( DepositedEnergy < Threshold && Control::TrackingPhysicsListELossOn == true ) return;
  
  G4ThreeVector MiddlePoint = 
    ( EntryPoint + ExitPoint ) / 2.;
  
  G4ThreeVector MeanMomentum = 
    ( ExitMomentum + EntryMomentum ) / 2.;

  //PK: fix for delta electrons: all particles causing hits
  // have to be saved in the LCIO file
   UserTrackInformation* theUserTrackInformation =
    (UserTrackInformation*) (aStep->GetTrack()->GetUserInformation());
  
 
   if(theUserTrackInformation) {
     
     theUserTrackInformation->GetTheTrackSummary()->SetToBeSaved();
   }

  HitCollection->
    insert(new TRKHit (currentCellID0,
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
	 << "====>DumpHit, currentCopyNumber = " << currentCopyNumber
   << ", currentCellID0 = " << currentCellID0
   << ", MiddlePoint(0) = " << MiddlePoint(0)
	 << ", MiddlePoint(1) = " << MiddlePoint(1)
	 << ", EntryPoint = " << EntryPoint
	 << ", ExitPoint = " << ExitPoint
	 << ", StepLength = " << StepLength
	 << G4endl;
  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif

  this->Clear();
  
}

void TRKSD02::StartNewHit(G4int copynumber, G4int CellID0,
			  G4ThreeVector theEntryPoint,
			  G4ThreeVector theEntryMomentum,
			  G4int thePDG,
			  G4int theTrackID)
{      
  currentCopyNumber = copynumber;
  currentCellID0 = CellID0;
  currentPID = theTrackID,
  currentPDG = thePDG;
  EntryPoint = ExitPoint = theEntryPoint;
  EntryMomentum = ExitMomentum = theEntryMomentum;
  DepositedEnergy = 0.;
  HitTime = 0.;
  StepLength = 0.;

#ifdef DEBUGTRKSD
  G4cout << GetName()
  << "====>StartHit, currentCopyNumber = " << currentCopyNumber
  << ", currentCellID0 = " << currentCellID0
  << ", EntryPoint = " << EntryPoint
  << ", ExitPoint = " << ExitPoint
  << ", StepLength = " << StepLength
  << G4endl;
#endif

}

void TRKSD02::Clear()
{
  currentCopyNumber = 0;
  currentCellID0 = 0;
  currentPID = -1; 
  DepositedEnergy = 0;
  HitTime = 0. ;
  StepLength  = 0. ;
}

void TRKSD02::UpdateHit(G4Step *aStep)
{
  DepositedEnergy+=aStep->GetTotalEnergyDeposit();
  HitTime = aStep->GetTrack()->GetGlobalTime() ; 
  ExitPoint=aStep->GetPostStepPoint()->GetPosition();
  ExitMomentum=aStep->GetPostStepPoint()->GetMomentum();
  StepLength += aStep->GetStepLength();

#ifdef DEBUGTRKSD
  G4cout << GetName()
	 << "====>UpdateHit, currentCopyNumber = " << currentCopyNumber
	 << ", EntryPoint = " << EntryPoint
	 << ", ExitPoint = " << ExitPoint
	 << ", StepLength = " << aStep->GetStepLength()
	 << G4endl;
#endif
}
