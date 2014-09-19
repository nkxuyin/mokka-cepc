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
// $Id: TrigSD.cc,v 1.1 2007/02/09 15:57:48 predrag Exp $
// $Name: mokka-07-00 $
//
#include "Control.hh"
#include "TrigSD.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4TransportationManager.hh"
#include "G4ParticleDefinition.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "globals.hh"
#include "G4UImanager.hh"
#include <assert.h>

#include "G4Material.hh"
#include "CGAGeometryManager.hh"
#include "G4VProcess.hh"

#include "G4ThreeVector.hh"
#include "Encoder64.hh"
#include "Encoder32.hh"

TrigSD::TrigSD(G4String SDname) 
  : VSensitiveDetector(SDname), CalCollection(0),HCID(-1)
{
  
  G4String CollName=SDname+"Collection";
  collectionName.insert(CollName);
  theEncoder = new Encoder32();

}

TrigSD::~TrigSD()
{
}

void TrigSD::Initialize(G4HCofThisEvent *)
{
  CalCollection = new HitsCollection
    (SensitiveDetectorName,collectionName[0]); 
}


G4bool TrigSD::ProcessHits(G4Step *aStep,G4TouchableHistory *)
{
  // process only if energy>0. except geantinos
  G4double edep;
  if ((edep = aStep->GetTotalEnergyDeposit())<=0. &&
      aStep->GetTrack()->GetDefinition()->GetParticleType()!="geantino") 
    return true;
  G4double time = aStep->GetTrack()->GetGlobalTime() ;  
  //G4cout<< " test1"<<G4endl;
  // the layer number is the volume copy number
  G4int theLayer ;
 

  // hit will deposited in the midle of the step
  G4ThreeVector thePosition = 
    (aStep->GetPreStepPoint()->GetPosition()+
     aStep->GetPostStepPoint()->GetPosition())*0.5;

  G4int theSDPiece = 0;
  G4int theStave = 1;
  G4int theModule = 0;

  // calculates I,J
  G4int I,J;
  I=0;
  J=0;
G4ThreeVector theCellCenter ;
theCellCenter(0)=0.0;
 theCellCenter(2)=thePosition(2);
   if(thePosition(1)>0.0)
     {
       theCellCenter(1)=655.0;
       theLayer=1;
     }else{
     theCellCenter(1)=-605.0;
     theLayer=2;
     }



  // creates a new cell or add the energy to the cell if it already exists

  G4int PID = 
    Control::GetControl()->GetPIDForCalHit(aStep);
   assert(PID!=-1);

  G4int PDG=
    aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
 
  G4bool found=false;
  G4int n_hit = CalCollection->entries();

  cell_ids theCode = 
    theEncoder->encode(theStave,theModule,
		   I,J,theLayer,0);
  
  for(G4int i_hit = 0; i_hit< n_hit; i_hit++)
    if((*CalCollection)[i_hit]->
       testCell(theCode)) {
      (*CalCollection)[i_hit]->AddEdep(PID,PDG,edep,time);
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
				  PID,
				  PDG,
				  time,
				  theCode));
  return true;
}

void TrigSD::EndOfEvent(G4HCofThisEvent*HCE)
{
  if(HCID<0)
    { HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]); }
  HCE->AddHitsCollection( HCID, CalCollection );
}


void TrigSD::clear()
{
} 

void TrigSD::DrawAll()
{
} 

void TrigSD::PrintAll()
{
} 




