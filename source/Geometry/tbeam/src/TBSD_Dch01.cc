#include "TBSD_Dch01.hh"
#include "TRKHit.hh"

#include "G4SDManager.hh"
#include "G4TouchableHandle.hh"
#include "G4StepPoint.hh"
#include "G4VPhysicalVolume.hh"
#include "G4NavigationHistory.hh"

// comment-in for verbose debugging output from SD
//#define TBSD_DEBUG 1

TBSD_Dch01::TBSD_Dch01(G4String sdname, G4double eThreshold)

  :  VSensitiveDetector(sdname), 
     HCID(-1),
     SDName(sdname), 
     hitsColl(0)
{

  G4cout << "SD create <" << SDName << ">" << G4endl;

  collectionName.insert(sdname);

  hits_eThreshold = eThreshold;

  origin = G4ThreeVector();

}

TBSD_Dch01::~TBSD_Dch01()
{
}


void TBSD_Dch01::Initialize(G4HCofThisEvent* HCE)
{
#ifdef TBSD_DEBUG
  G4cout << "SD Init: " << SDName << G4endl;
#endif

  hitsColl = new TBHitsCollection(SDName, collectionName[0]);

  if (HCID < 0) {
    HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  }
  
  HCE->AddHitsCollection(HCID, hitsColl);

#ifdef TBSD_DEBUG
  G4cout << "HCID: " << HCID << G4endl;
#endif

}

G4bool TBSD_Dch01::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
#ifdef TBSD_DEBUG
  G4cout << "TBSD_Dch01::ProcessHits(): " << SDName << G4endl;
#endif

  G4Track* aTrack = aStep->GetTrack();
  if(aTrack ->GetDefinition()->GetPDGCharge() == 0.0 ) return false;
  G4double edep = aStep->GetTotalEnergyDeposit();
  G4double pTime = aTrack->GetGlobalTime();

  // get GlobalHitPosition
  G4ThreeVector theHitPos = aStep->GetPreStepPoint()->GetPosition();
  G4ThreeVector theHitMom = aTrack->GetDynamicParticle()->GetMomentum();

  const G4VTouchable *history =aStep->GetPreStepPoint()->GetTouchable();
  G4int copyNumber = history->GetVolume(0)->GetCopyNo();
//  G4int module_number = copyNumber / 1000;
//  G4int layer_number = copyNumber - module_number; 

  G4int pLayer   = copyNumber;
  G4double xHit  = theHitPos.x();
  G4double yHit  = theHitPos.y();
  G4double zHit  = theHitPos.z();
  G4double PxHit = theHitMom.x();
  G4double PyHit = theHitMom.y();
  G4double PzHit = theHitMom.z();
  G4int    PID   = aTrack->GetTrackID();
  G4int    PDG   = aTrack->GetDefinition()->GetPDGEncoding();
	  
  if(edep < hits_eThreshold) {
    return false;
  } 

#ifdef TBSD_DEBUG
  G4cout << "Deposited Energy " << edep << G4endl;
  G4cout << "Global Time " << pTime << G4endl;
  G4cout << "Global Hit pos " << theHitPos << G4endl;
  G4cout << "Track Momentum " << theHitMom << G4endl;
  G4cout << "Trk Id " << PID << G4endl;
  G4cout << "PDG Id " << PDG << G4endl;
#endif

  TRKHit* theHit = 
    new TRKHit(pLayer, 
	       xHit, yHit, zHit, 
	       PxHit, PyHit, PzHit, 
	       PID, PDG, edep, pTime,
	       aStep->GetStepLength());  
  hitsColl->insert( theHit ); 

  return true;
}

void TBSD_Dch01::EndOfEvent(G4HCofThisEvent* HCE)
{
  HCE->AddHitsCollection(HCID, hitsColl);    
}
