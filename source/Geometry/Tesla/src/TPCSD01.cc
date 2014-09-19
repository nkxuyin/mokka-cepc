// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: TPCSD01.cc,v 1.3 2006/03/15 15:50:48 mora Exp $
// $Name: mokka-07-00 $

#include "Control.hh"
#include "TPCSD01.hh"

#include "G4Step.hh"
#include "G4SDManager.hh"
#include "UserTrackInformation.hh"

TPCSD01::TPCSD01(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy):
  VSensitiveDetector(name), fThresholdEnergyDeposit(thresholdEnergyDeposit),
  fThresholdKineticEnergy(thresholdKineticEnergy), fHitCollection(0), fHCID(-1)
{
  collectionName.insert(name + "Collection"); // data member of the base class "G4VSensitiveDetector"
}

void TPCSD01::Initialize(G4HCofThisEvent *)
{
  fHitCollection = new TRKHitsCollection(SensitiveDetectorName, collectionName[0]);
}

G4bool TPCSD01::ProcessHits(G4Step *step, G4TouchableHistory *)
{
  if (step->GetPreStepPoint()->GetKineticEnergy() < fThresholdKineticEnergy) return true;
  if (step->GetTotalEnergyDeposit()               < fThresholdEnergyDeposit) return true;

  const G4ThreeVector meanPosition = (step->GetPreStepPoint()->GetPosition() + step->GetPostStepPoint()->GetPosition()) / 2;
  const G4ThreeVector meanMomentum = (step->GetPreStepPoint()->GetMomentum() + step->GetPostStepPoint()->GetMomentum()) / 2;

  fHitCollection->
    insert(new TRKHit(0, // ignore cell ID or pad row
		      meanPosition[0], meanPosition[1], meanPosition[2],
		      meanMomentum[0], meanMomentum[1], meanMomentum[2],
		      Control::primaryId, 
		      step->GetTrack()->GetDefinition()->GetPDGEncoding(),
		      step->GetTotalEnergyDeposit(), 
		      step->GetTrack()->GetGlobalTime(),
		      step->GetStepLength()));
  
  // fix for delta electrons: all particles causing hits have to be saved in the LCIO file -- PK
  UserTrackInformation *info = (UserTrackInformation*)(step->GetTrack()->GetUserInformation());
  if (info) info->GetTheTrackSummary()->SetToBeSaved();

  return true;
}

void TPCSD01::EndOfEvent(G4HCofThisEvent *eventHC)
{
  if (fHCID < 0)
    fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  eventHC->AddHitsCollection(fHCID, fHitCollection);
}

void TPCSD01::LoadEvent(FILE *)
{
  Control::Log("TPCSD01: ASCII files are deprecated.");
}
