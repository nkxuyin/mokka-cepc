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
// $Id: TPCSD00.cc,v 1.2 2003/07/25 13:10:31 mora Exp $
// $Name: mokka-07-00 $
//
#include "Control.hh"
#include "TPCSD00.hh"

#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "globals.hh"
#include <assert.h>


TPCSD00::TPCSD00(G4String TPCSD00name) 
  : VSensitiveDetector(TPCSD00name),HCID(-1),
    lastCylinder(-10),lastPID(-10),     
    CalCollection(0)
{
  G4String CollName=TPCSD00name+"Collection";
  collectionName.insert(CollName);
}

void TPCSD00::Initialize(G4HCofThisEvent *)
{
  CalCollection = new TPCHitsCollection
    (SensitiveDetectorName,collectionName[0]);
  lastCylinder=lastPID=-10;
}

G4bool TPCSD00::ProcessHits(G4Step *aStep,G4TouchableHistory *)
{
  // In this first release we keep just the first step point
  // in the layer boundary with P and PID.
  // pseudo TPC is type 1, FCH type 2.
  // It's not really a Hit!
  // September 2002: we keep also the particle PDG

  // Just for particles with more than 10 MeV
  if(aStep->GetPreStepPoint()->GetKineticEnergy() 
     < 10 * MeV) return true;

  // and just for charged particles
  if(aStep->GetTrack()->GetDefinition()->GetPDGCharge()==0) return true;

  // klayer number
  G4int K=aStep->GetPreStepPoint()->
    GetPhysicalVolume()->GetCopyNo();

  // One hit per cylinder
  if(lastCylinder==K && 
     lastPID==Control::primaryId) return true;
  else 
    {
      lastCylinder=K;
      lastPID=Control::primaryId;
    }
  
  // pseudo hit will deposited on the layer boundary
  G4ThreeVector theCellCenter = 
    aStep->GetPreStepPoint()->GetPosition();
    
  CalCollection->
    insert(new TPCHit (0,
		       K,
		       theCellCenter (0),
		       theCellCenter (1),
		       theCellCenter (2),
		       aStep->GetPostStepPoint()->GetMomentum()(0),
		       aStep->GetPostStepPoint()->GetMomentum()(1),
		       aStep->GetPostStepPoint()->GetMomentum()(2),
		       Control::primaryId,
		       aStep->GetTrack()->GetDefinition()->GetPDGEncoding()));
  return true;
}

void 
TPCSD00::EndOfEvent(G4HCofThisEvent*HCE)
{
  if(HCID<0)
    { HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]); }
  HCE->AddHitsCollection( HCID, CalCollection );
}

void 
TPCSD00::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
  TPCHit* newHit = new TPCHit();
  while (newHit->Load(theSubDetectorEventHitsFileInput))
    {
      CalCollection->insert(newHit);
      newHit = new TPCHit();
    }
  delete newHit;
}


void TPCSD00::clear()
{
} 

void TPCSD00::DrawAll()
{
} 

void TPCSD00::PrintAll()
{
} 




