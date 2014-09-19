#include "TBSD.hh"

#include "Control.hh"
#include "CalHit.hh"
#include "CGADefs.h"
#include "Encoder32.hh"

#include "G4SDManager.hh"
#include "G4TouchableHandle.hh"
#include "G4StepPoint.hh"
#include "G4VPhysicalVolume.hh"
#include "G4NavigationHistory.hh"

TBSD::TBSD(G4String sdname) :
  VSensitiveDetector(sdname), HCID(-1), SDName(sdname), hitsColl(0)
{
  G4cout << "SD create: " << SDName << G4endl;

  collectionName.insert(sdname);
  origin = G4ThreeVector();
  hitMap = new TBHitMap;

  if (SDName=="hcalSD")
    moduleID=TBHCAL;
  else if (SDName=="ecalSD")
    moduleID=TBECAL;
  else if (SDName=="catcherSD")
    moduleID=TBCATCHER;
  else
  {
    G4cerr << "SDName " << SDName << " is unknown type for module enum." << G4endl;
    Control::Abort("TBSD - Unknown SDName",
		MOKKA_ERROR_BAD_SENSITIVE_DETECTOR_NAME);
  }
  
  theEncoder = new Encoder32();
}


TBSD::~TBSD()
{
  hitMap->clear();
  delete hitMap;
}


void TBSD::Initialize(G4HCofThisEvent* HCE)
{
  // G4cout << "SD Init: " << SDName << G4endl;

  hitsColl = new TBHitsCollection(SDName, collectionName[0]);

  if (HCID < 0)
    HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  
  HCE->AddHitsCollection(HCID, hitsColl);

  // G4cout << "HCID: " << HCID << G4endl;
  hitMap->clear();
}


G4bool TBSD::ProcessHits(G4Step* aStep, G4TouchableHistory *)
{
  // G4cout << "\nTBSD::ProcessHits(): " << SDName << G4endl;

  G4double edep;

  if ((edep=aStep->GetTotalEnergyDeposit())<=0 &&
      aStep->GetTrack()->GetDefinition()->GetParticleType() !="geantino")
    return true;

  const G4StepPoint *preStepPoint = aStep->GetPreStepPoint();
  G4ThreeVector theGlobalPos = preStepPoint->GetPosition();
  
  // G4cout << "Global Hit pos: " << theGlobalPos << G4endl;

  G4TouchableHandle theTouchableHandle = preStepPoint->GetTouchableHandle();

  G4double time = aStep->GetTrack()->GetGlobalTime();

  // global cell center 
  G4ThreeVector theGlobalCellCenter = 
    theTouchableHandle->GetHistory()->GetTopTransform().Inverse().TransformPoint(origin);  

  // G4cout << "Layer: " << n_lay << G4endl;
  // G4cout << "Cell center: " << theGlobalCellCenter << G4endl;

  // layer #
  G4int n_lay = theTouchableHandle->GetHistory()->GetVolume(2)->GetCopyNo();

  // PDG
  G4int PDG=aStep->GetTrack()->GetDefinition()->GetPDGEncoding();

  // GM: We'll use Encoder32 to get the CellID's
  cell_ids theCode = theEncoder->encode(1, 0, 0, 0, n_lay, 0);
  // FindHit will ++edep if it finds the cell
  if (!FindHit(theGlobalCellCenter, edep, PDG, time))
  {
    CalHit *theHit = new CalHit (moduleID, // MODULE (see enum @ Control.hh)
				   1,      // WI (glob index i)
				   0,      // WJ (glob index j)
				   0,      // I (local index i) (USE AS LAYER)
				   0,      // J (local index j) 
				   n_lay,  // layer
				   0, //Guard-Ring zone arg of CalHit
				   theGlobalCellCenter(0),    // cell x
				   theGlobalCellCenter(1),    // cell y
				   theGlobalCellCenter(2),    // cell z
				   edep,                      // edep
				   Control::primaryId,//Primary ID (correct?)
				   PDG,
				   time,
				   theCode);

    /*
    // dummy values for testing indices
    CellHit *theHit = new CellHit (moduleID,                  // MODULE (see enum @ Control.hh)
				   8,                         // WI (glob index i)
				   7,                         // WJ (glob index j)
				   511,                         // I (local index i) (USE AS LAYER)
				   511,                         // J (local index j) 
				   64,                     // layer
         			   0,
				   theGlobalCellCenter(0),    // cell x
				   theGlobalCellCenter(1),    // cell y
				   theGlobalCellCenter(2),    // cell z
				   edep,                      // edep
				   Control::primaryId,        // Primary ID (correct?)
				   PDG,
				   time);    
    */

    hitMap->insert(TBHitMapPair(theGlobalCellCenter, theHit));

    // G4cout << "Adding hit at: " << theGlobalCellCenter << " <" << SDName << ">" << G4endl;
  }

  return true;
}

G4bool TBSD::FindHit(G4ThreeVector cellPos, G4double edep, G4int PDG, G4double time)
{
  TBHitMapIterator iter;

  for (iter=hitMap->begin(); iter != hitMap->end(); iter++)
  {
    if (iter->first==cellPos)
    {
      // G4cout << "Found hit at: " << cellPos << G4endl;

      (iter->second)->AddEdep(Control::primaryId,PDG,edep,time);
      return true;
    }		       
  } 
  return false;
}

void TBSD::EndOfEvent(G4HCofThisEvent* HCE)
{
  TBHitMapIterator iter;

  for(iter=hitMap->begin(); iter != hitMap->end(); ++iter)
  {
    hitsColl->insert(iter->second); 
  }
 
  hitMap->clear(); 
  // will not delete hits;
  // CellHit deleted with HC

  // add to HCE
  if (HCID<0)
    { HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);}

  HCE->AddHitsCollection(HCID, hitsColl);    
}



