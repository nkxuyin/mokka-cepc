#include "TBSD_VCell.hh"

#include "Control.hh"
#include "CalHit.hh"
#include "CGADefs.h"
#include "Encoder32.hh"

#include "G4SDManager.hh"
#include "G4TouchableHandle.hh"
#include "G4StepPoint.hh"
#include "G4VPhysicalVolume.hh"
#include "G4NavigationHistory.hh"

// set this for ProcessHits step-wise debugging (lots of output)
//#define TBSD_DEBUG 1

TBSD_VCell::TBSD_VCell(G4String sdname, G4double cx, G4double cz, G4double calhx, G4double calhz) :
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
    Control::Abort("TBSD_VCell - Unknown SDName",
		MOKKA_ERROR_BAD_SENSITIVE_DETECTOR_NAME);
  } 

  // cell dims
  // cell_x = cx;
  // cell_z = cz;

  // cal dims
  // cal_hx = calhx;
  // cal_hz = calhz;

  // create readout; SD keeps no dims!
  cellRO = TBCellReadout(cx, cz, calhx, calhz);

  theEncoder = new Encoder32();
}


TBSD_VCell::~TBSD_VCell()
{
  hitMap->clear();
  delete hitMap;
}


void TBSD_VCell::Initialize(G4HCofThisEvent* HCE)
{
  // G4cout << "SD Init: " << SDName << G4endl;

  hitsColl = new TBHitsCollection(SDName, collectionName[0]);

  if (HCID < 0)
    HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  
  HCE->AddHitsCollection(HCID, hitsColl);

  // G4cout << "HCID: " << HCID << G4endl;
  hitMap->clear();
}


G4bool TBSD_VCell::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
#ifdef TBSD_DEBUG
  G4cout << "\nTBSD_VCell::ProcessHits(): " << SDName << G4endl;
#endif

  G4double edep;

  if ((edep=aStep->GetTotalEnergyDeposit())<=0 &&
      aStep->GetTrack()->GetDefinition()->GetParticleType() !="geantino")
    return true;

  const G4StepPoint *preStepPoint = aStep->GetPreStepPoint();
  G4ThreeVector theGlobalPos = preStepPoint->GetPosition();
  
#ifdef TBSD_DEBUG
  G4cout << "Global Hit pos: " << theGlobalPos << G4endl;
#endif

  // cell readout 1x1

  // cell indices ( first neg = -1, first pos = 0 i.e. floor(hit_dim/cell_dim) )
  G4int cnx = cellRO.GetCellNoX(theGlobalPos.x());
  G4int cnz = cellRO.GetCellNoZ(theGlobalPos.z());

#ifdef TBSD_DEBUG
  G4cerr << "cnx: " << cnx << G4endl;
  G4cerr << "cnz: " << cnz << G4endl;
#endif

  // locations
  // shouldn't need local -> global trans as X/Z is correct in this ref frame
  G4double locx = cellRO.GetLocalCenterX(cnx);
  G4double locz = cellRO.GetLocalCenterZ(cnz);

#ifdef TBSD_DEBUG
  G4cerr << "locx: " << locx << G4endl;
  G4cerr << "locz: " << locz << G4endl;
#endif
  // --- end RO ---

  G4TouchableHandle theTouchableHandle = preStepPoint->GetTouchableHandle();

  // global layer center
  G4ThreeVector theGlobalCellCenter = 
    theTouchableHandle->GetHistory()->GetTopTransform().Inverse().TransformPoint(origin); 

  // set vector coords. for lkup
  theGlobalCellCenter.setX(locx);
  theGlobalCellCenter.setZ(locz);

  G4double time = aStep->GetTrack()->GetGlobalTime();

#ifdef TBSD_DEBUG
  G4cout << "Cell center: " << theGlobalCellCenter << G4endl;
#endif

  // layer #
  G4int n_lay = theTouchableHandle->GetHistory()->GetVolume(2)->GetCopyNo();

#ifdef TBSD_DEBUG
  G4cout << "Layer: " << n_lay << G4endl;
#endif

  // PDG
  G4int PDG=aStep->GetTrack()->GetDefinition()->GetPDGEncoding();

  
  // GM: We'll use Encoder32 to get the CellID's
  cell_ids theCode = theEncoder->encode(1,0,0,0,n_lay,0);

  // FindHit will ++edep if it finds the cell
  if (!FindHit(theGlobalCellCenter, edep, PDG, time))
  {
    CalHit *theHit = new CalHit (moduleID,// MODULE (see enum @ Control.hh)
				   1,     // WI (glob index i)
				   0,     // WJ (glob index j)
				   0,     // I (local index i) (USE AS LAYER)
				   0,     // J (local index j) 
				   n_lay, // layer
				   0,     //Guard-Ring zone arg of CalHit
				          //theGlobalCellCenter(0), // cell x
				   locx,
				   theGlobalCellCenter.y(),   // cell y
				   locz,                      // cell z
				   edep,                      // edep
				   Control::primaryId, // Primary ID (correct?)
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

G4bool TBSD_VCell::FindHit(G4ThreeVector cellPos, G4double edep, G4int PDG, G4double time)
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

void TBSD_VCell::EndOfEvent(G4HCofThisEvent* HCE)
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



