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
// $Id: HodoscopeSD00.cc,v 1.4 2006/03/01 14:13:30 musat Exp $
// $Name: mokka-07-00 $
//
#include "Control.hh"
#include "HodoscopeSD00.hh"

#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "globals.hh"
#include <assert.h>

#include "Encoder32.hh"

HodoscopeSD00::HodoscopeSD00(G4String HodoscopeSD00name) 
  : VSensitiveDetector(HodoscopeSD00name), HCID(-1),  CalCollection(0)
{
  G4String CollName=HodoscopeSD00name+"Collection";
  collectionName.insert(CollName);
 
  theEncoder = new Encoder32();
}

void HodoscopeSD00::Initialize(G4HCofThisEvent*)
{
  CalCollection = new HitsCollection
    (SensitiveDetectorName,collectionName[0]);
}

G4bool HodoscopeSD00::ProcessHits(G4Step *aStep,G4TouchableHistory*)
{
  // process only if energy>0 if not geantinos
  G4double edep;
  if ((edep = aStep->GetTotalEnergyDeposit())<=0. &&
      aStep->GetTrack()->GetDefinition()->GetParticleType()!="geantino")
    return true;
  
  // and just for charged particles
  //if(aStep->GetTrack()->GetDefinition()->GetPDGCharge()==0) return true;


  // k = number of fiber core
  G4int K=aStep->GetPreStepPoint()->
    GetPhysicalVolume()->GetCopyNo();

  const G4VTouchable *history =aStep->GetPreStepPoint()->GetTouchable();
  G4int LayersCopyNumber=history->GetVolume(1)->GetCopyNo();

//   G4cout << "K = " << K
// 	 << "\nLayersCopyNumber = " << LayersCopyNumber << G4endl;

//   G4cout << "Position = " << aStep->GetPreStepPoint()->GetPosition() << G4endl;

  // pseudo hit will deposited on the layer boundary
  G4ThreeVector theCellCenter = 
    aStep->GetPreStepPoint()->GetPosition();
    

  G4bool found=false;
  G4int n_hit = CalCollection->entries();

  G4int theSDPiece = HODOSCOPE;
  G4int theStave = LayersCopyNumber;
  G4int theModule = K;
  G4int I = 1;
  G4int J = 1;
  G4int theLayer = 1;

  G4double time = aStep->GetTrack()->GetGlobalTime() ;

  G4int PDG=
    aStep->GetTrack()->GetDefinition()->GetPDGEncoding();

  cell_ids theCode =
    theEncoder->encode(theStave,theModule,I,J,theLayer,0);
                                                                                
  for(G4int i_hit = 0; i_hit< n_hit; i_hit++)
    if((*CalCollection)[i_hit]->testCell(theCode)) {
      (*CalCollection)[i_hit]->AddEdep(Control::primaryId,PDG,edep,time);
      found = true;
      break;
    }
  
  if(!found) CalCollection->
	       insert(new CalHit (theSDPiece,
				  theStave,
				  theModule,
				  I,
				  J,
				  theLayer,
				  0,
				  theCellCenter (0),
				  theCellCenter (1),
				  theCellCenter (2),
				  edep,
				  Control::primaryId,
				  PDG,
				  time,
				  theCode));

  return true;
}

void HodoscopeSD00::EndOfEvent(G4HCofThisEvent*HCE)
{
  if(HCID<0)
    { HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]); }
  HCE->AddHitsCollection( HCID, CalCollection );
}

void HodoscopeSD00::clear()
{
} 

void HodoscopeSD00::DrawAll()
{
} 

void HodoscopeSD00::PrintAll()
{
} 




