#include "Control.hh"
#include "wscalSD.hh"
#include "CGADefs.h"
#include "EncoderTB.hh"

// comment-in for verbose debugging output from SD
//#define WSSD_DEBUG 1

wscalSD::wscalSD( G4String sdname,
		  G4double cellDim,
		  G4int nCellX,
		  G4int nCellY,
		  G4int modID)
  
  :  VSensitiveDetector(sdname), 
     HCID(-1),
     SDName(sdname), 
     hitsColl(0)

{


  G4cout << "SD create <" << SDName << ">" << G4endl;
  collectionName.insert(sdname);
  origin = G4ThreeVector();
  hitMap = new WSHitMap;

  // set primary args
  SetCellDim(cellDim);
  SetNCellXY(nCellX, nCellY);
  SetModuleID(modID);
  
  // DEBUG: print parameters
#ifdef WSSD_DEBUG
  G4cout << "n_cell_x " << ncell_xy[0] << G4endl;
  G4cout << "n_cell_y " << ncell_xy[1] << G4endl;
  G4cout << "cellDim " << cellDim << G4endl;
  // set cal dims derived from above
  G4double xDim = (G4double) ncell_xy[0] * cellDim;
  G4double yDim = (G4double) ncell_xy[1] * cellDim;
  G4cout << "cal xDim " << xDim << G4endl;
  G4cout << "cal yDim " << yDim << G4endl << G4endl;
#endif

  theEncoder = new EncoderTB();
}

wscalSD::~wscalSD()
{
  hitMap->clear();
  delete hitMap;
}


void wscalSD::Initialize(G4HCofThisEvent* HCE)
{
#ifdef WSSD_DEBUG
  G4cout << "SD Init: " << SDName << G4endl;
#endif

  //Create a (G4) hit collection for this event
  hitsColl = new WSHitsCollection(SDName, collectionName[0]);

  if (HCID < 0) {
    HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  }
  
  //Would be sufficient to add the collection at the end of the event
  //Doing it here just suppresses a warning during compilation
  HCE->AddHitsCollection(HCID, hitsColl);

#ifdef WSSD_DEBUG
  G4cout << "HCID: " << HCID << G4endl;
#endif
  
  //Reset the hit map
  hitMap->clear();
}

G4bool wscalSD::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
  
#ifdef WSSD_DEBUG
  G4cout << G4endl << "wscalSD::ProcessHits(): " << SDName << G4endl;
#endif
  
  G4double edep;
  
  if ( (edep=aStep->GetTotalEnergyDeposit()) <= 0 &&
       aStep->GetTrack()->GetDefinition()->GetParticleType() !="geantino") {
    return true;
  }
  
  // get PreStepPoint
  const G4StepPoint *preStepPoint = aStep->GetPreStepPoint();
  
  // get TouchableHandle
  // remark: It is highly recommended to make extensive use of
  // touchables when transforming space coordinates within your
  // geometry (e.g. from local to global coordinates
  G4TouchableHandle theTouchable = preStepPoint->GetTouchableHandle();
  
  // get GlobalHitPosition
  G4ThreeVector theGlobalPos = preStepPoint->GetPosition();
  
#ifdef WSSD_DEBUG
  G4cout << "Global Hit pos " << theGlobalPos << G4endl;
  G4cout << "Post Position " << aStep->GetPostStepPoint()->GetPosition() << G4endl;
  G4cout << "Deposited energy: " << edep << G4endl;  
#endif
  


  // get Layer Number from Touchable
  G4int n_lay = theTouchable->GetHistory()->GetVolume(1)->GetCopyNo();
  //get index i_x from Touchable
  G4int i_x = theTouchable->GetReplicaNumber(); 
  //get index j_y from Touchable
  G4int j_y = theTouchable->GetReplicaNumber(1);

  //Set cell center in x,y,z 
  G4ThreeVector localCellPos(0., 0., 0.);

  // compute GlobalCellPos based on touchable with localCellPos
  G4ThreeVector GlobalCellPos = theTouchable->GetHistory()->GetTopTransform().Inverse().TransformPoint(localCellPos);

#ifdef WSSD_DEBUG
  G4ThreeVector checkPos = GlobalCellPos - theGlobalPos;
  G4cout << "checkPos=GlobalCellPos-theGlobalPos <" << checkPos << ">" << G4endl;
  G4cout << "GlobalCellPos " << GlobalCellPos << G4endl;
  G4cout << "Cell Indices i,j,k: " << i_x << ", " << j_y << ", " << n_lay << G4endl;   
#endif

  // pack i, j, k cell coordinates into a unique, int cellid
  int cellId = 0 ;
  cellId |= (  i_x  << 0  ) ;  
  cellId |= (  j_y  << 8  ) ;  
  cellId |= (  n_lay  << 16  ) ;  


  // get the PDG encoding for the particle
  G4int PDG=aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
  
  // get the global time
  G4double globalTime = aStep->GetTrack()->GetGlobalTime() ;
  
  if (!FindHit(cellId, edep , PDG, globalTime ) && 
      !( (i_x > ncell_xy[0]) || (j_y > ncell_xy[1]) ) ) 
    {
      
#ifdef WSSD_DEBUG
      G4cout << "cellId_i before the Hit: " << (cellId & 0xff) << G4endl; 
      G4cout << "cellId_j before the Hit: " << ( (cellId >> 8) & 0xff)<< G4endl; 
      G4cout << "cellId_k before the Hit: " <<  ( (cellId >> 16) & 0xff) << G4endl; 
#endif
      
      // assert id w/in valid range
      assert((cellId & 0xff)<=90);
      
      // GM: We'll use EncoderTB to get the CellID as it was calculated
      // in class Proto_CellHit
      cell_ids theCode = theEncoder->encode(1,0,
					(cellId & 0xff), // i_x
					((cellId >> 8) & 0xff), // j_y
					n_lay,
					0);

      // create CellHit object from hit class defined for the Prototype
      CalHit *theHit = new CalHit (moduleID,//MODULE(see enum @ Control.hh) | P
                                   1,       //WI (glob index i)             | S
                                   0,       // WJ (glob index j)            | M
                                   (cellId & 0xff),  // I(local index i)    | I
                                   ((cellId >> 8) & 0xff),// J(local index j)|J
                                   n_lay,                // layer           | K
				   0,
                                   GlobalCellPos.x(),    // GlobalCellPosX
                                   GlobalCellPos.y(),    // GlobalCellPosY
                                   GlobalCellPos.z(),    // GlobalCellPosZ
                                   edep,                 // edep
                                   Control::primaryId,   // Primary ID
                                   PDG,                  // PDG encoding
                                   globalTime,           // global time
				   theCode);
      

      //Insert this cellID into the hitmap
      hitMap->insert(WSHitMapPair(cellId, theHit));
    }
  
  
  return true;
  
}



//Check whether we ahve this cell already in our collection and update
//if present
G4bool wscalSD::FindHit(G4int cellId, G4double edep, G4int PDG, G4double globalTime )
{
  WSHitMapIterator iter;
  
  for (iter=hitMap->begin(); iter != hitMap->end(); iter++) {
    if ( ( (iter->first)-cellId) == 0) {
      (iter->second)->AddEdep( Control::primaryId, PDG, edep, globalTime/ns ) ; 
      return true;
    }                  
  } 
  return false;
}



void wscalSD::SetNCellXY(G4int nx, G4int ny) 
{ 
  // set number of cellS IN X,Y direction
  ncell_xy[0] = nx; 
  ncell_xy[1] = ny; 
} 

void wscalSD::SetCellDim(G4double c) 
{ 
  cellDim = c; 
  assert(cellDim > 0);
}


void wscalSD::SetModuleID(G4int m)
{
  moduleID = m;
}


//Pass the hit collection to the other G4 hits collection
//These collections will be transformed into the LCIO Collection you
//find in the output file
void wscalSD::EndOfEvent(G4HCofThisEvent* HCE)
{
  WSHitMapIterator iter;

#ifdef WSSD_DEBUG
  G4cout << "End of Event " << G4endl;
#endif
  for ( iter=hitMap->begin(); 
        iter != hitMap->end(); 
        ++iter) {
    hitsColl->insert(iter->second); 
  }
 
  hitMap->clear(); 


  HCE->AddHitsCollection(HCID, hitsColl);    
}
