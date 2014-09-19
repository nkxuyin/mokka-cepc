//Author Roman Poeschl DESY
//Routines partially adapted from Jeremy McCormick, NIU
#include "Control.hh"
#include "TBSD_VCell02.hh"

// sub_detectors
#include "TBhcal02.hh"
#include "TBecal02.hh"
#include "TBcatcher02.hh"

#include "CGADefs.h"
#include "EncoderTB.hh"

//#include "CellHit.hh"


#include "G4SDManager.hh"
#include "G4TouchableHandle.hh"
#include "G4StepPoint.hh"
#include "G4VPhysicalVolume.hh"
#include "G4NavigationHistory.hh"

// set this for ProcessHits debugging (lots of output)
//#define TBSD_DEBUG 1

// better proto: sdname, det_enum, ncell_xz, grid_sizePtr
TBSD_VCell02::TBSD_VCell02(G4String sdname, VSubDetectorDriver* this_detector) :
  VSensitiveDetector(sdname), HCID(-1), SDName(sdname), hitsColl(0)
{
  G4cout << "SD create: " << SDName << G4endl;

  collectionName.insert(sdname);
  origin = G4ThreeVector();
  hitMap = new TBHitMap;

  if (SDName=="hcalSD") {
    moduleID=TBHCAL;
    // CRP get numbers of cells in xz-direction
    //     our prototype is still aligned in y-position (that's why xz and
    //     not xy !!!
    ncell_xz =  ( static_cast<TBhcal02*> (this_detector) )->GetNCells();
    grid_sizePtr =  ( static_cast<TBhcal02*> (this_detector) )->GetGridSize();
  }
  else if (SDName=="ecalSD") {
    moduleID=TBECAL;
    ncell_xz =  ( static_cast<TBecal02*> (this_detector) )->GetNCells();
    grid_sizePtr =  ( static_cast<TBecal02*> (this_detector) )->GetGridSize();
  }
  else if (SDName=="catcherSD") {
    moduleID=TBCATCHER;
    ncell_xz =  ( static_cast<TBcatcher02*> (this_detector) )->GetNCells();
    grid_sizePtr =  ( static_cast<TBcatcher02*> (this_detector) )->GetGridSize();
  }
  else {
    G4cerr << "SDName " << SDName << " is unknown type for module enum." << G4endl;
    Control::Abort("TBSD_VCell02 - Unknown SDName",
		MOKKA_ERROR_BAD_SENSITIVE_DETECTOR_NAME);
  }

  theEncoder = new EncoderTB();
}


TBSD_VCell02::~TBSD_VCell02()
{
  hitMap->clear();
  delete hitMap;
}


void TBSD_VCell02::Initialize(G4HCofThisEvent* HCE)
{
  // G4cout << "SD Init: " << SDName << G4endl;

  hitsColl = new TBHitsCollection(SDName, collectionName[0]);

  if (HCID < 0)
    HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  
  HCE->AddHitsCollection(HCID, hitsColl);

  // G4cout << "HCID: " << HCID << G4endl;
  hitMap->clear();
}


G4bool TBSD_VCell02::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
#ifdef TBSD_DEBUG
  G4cout << "\nTBSD_VCell02::ProcessHits(): " << SDName << G4endl;
#endif

  G4double edep;

  if ((edep=aStep->GetTotalEnergyDeposit())<=0 &&
      aStep->GetTrack()->GetDefinition()->GetParticleType() !="geantino")
    return true;

  const G4StepPoint *preStepPoint = aStep->GetPreStepPoint();
  G4ThreeVector theGlobalPos = preStepPoint->GetPosition();
  
#ifdef TBSD_DEBUG
  G4cout << "Global Hit pos: " << theGlobalPos << G4endl;
  G4cout << "Post Position= " <<
    aStep->GetPostStepPoint()->GetPosition() << G4endl;
#endif

  //Get CellIDs for hit
  G4ThreeVector theLocalPos = theGlobalPos - origin;

  TBSD_VCell02::SetCellCoordinates( theLocalPos[0]/mm, theLocalPos [2]/mm);
  G4int i_x = cellID[0] ;
  // layer #
  G4TouchableHandle theTouchableHandle = preStepPoint->GetTouchableHandle();
  G4int n_lay = theTouchableHandle->GetHistory()->GetVolume(2)->GetCopyNo();
#ifdef TBSD_DEBUG
  G4cout << "Layer: " << n_lay << G4endl;
#endif
  G4int k_z = cellID[1] ;
  // CRP Transform cell coordinates into a unique cellid
  int cellId = 0 ;
  cellId |= (  i_x  << 0  ) ;  
  //Our prototype is still aligned in y-position !!! 
  cellId |= (  n_lay  << 8  ) ;  
  cellId |= (  k_z  << 16  ) ;  

  // PDG
  G4int PDG=aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
  G4double time = aStep->GetTrack()->GetGlobalTime() ;

  // FindHit will ++edep if it finds the cell
  if (!FindHit(cellId, edep , PDG, time )) //fg: hits need pdg and time !
  {
    cellPos[1] = theTouchableHandle->GetHistory()->GetTopTransform().Inverse().TransformPoint(origin) (1);
#ifdef TBSD_DEBUG
    G4cout << "cellId_i before the Hit: " << (cellId & 0xff) << G4endl; 
    G4cout << "cellPos1 before the Hit: " <<  cellPos[1] << G4endl; 
    G4cout << "cellId_j before the Hit: " <<  ( (cellId >> 16) & 0xff) << G4endl; 
    assert((cellId & 0xff)<=90);
#endif
    TBSD_VCell02::SetCellPosition(cellId);

    //GM: from now on we'll use EncoderTB, keeping the order in Proto_CellHit
    cell_ids theCode = theEncoder->encode(0, 0, 
			(cellId & 0xff),	  // i_x
			n_lay, 
			((cellId >> 16) & 0xff),  // k_z 
			0);

    // CRP Use the CellHit class defined for the Prototye, i.e. Proto_CellHit 
    //CellHit *theHit = new CellHit (moduleID,                  
    CalHit *theHit = new CalHit(moduleID,//MODULE (see enum @ Control.hh) |P
			       1,        // WI (glob index i)             |S
			       0,        // WJ (glob index j)             |M
			       (cellId & 0xff),// I  (local index i) (LAYER)|I
			       n_lay,          // layer                     |J
			       ( (cellId >> 16) & 0xff),// J(local index j) |K
			       0, 
			       cellPos[0],              // cell x   
			       cellPos[1],              // cell y
			       cellPos[2],              // cell z
			       edep,                    // edep
			       Control::primaryId,      // Primary ID
			       PDG, 
			       time,
			       theCode);


    hitMap->insert(TBHitMapPair(cellId, theHit));

    // G4cout << "Adding hit at: " << theGlobalCellCenter << " <" << SDName << ">" << G4endl;
  }

  return true;
}

G4bool TBSD_VCell02::FindHit(G4int cellId, G4double edep, G4int PDG, G4double time )
{
  TBHitMapIterator iter;

  for (iter=hitMap->begin(); iter != hitMap->end(); iter++)
  {
    if ( ( (iter->first)-cellId) == 0)
    {
      //       G4cout << "Found hit at: " << cellPos << G4endl;
      (iter->second)->AddEdep( Control::primaryId, PDG, edep, time/ns ) ; 
      return true;
    }		       
  } 
  return false;
}

void TBSD_VCell02::EndOfEvent(G4HCofThisEvent* HCE)
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


void TBSD_VCell02::SetCellCoordinates( G4float x, G4float z){
  // CRP Calculate i_x, k_z position of virtual cell from the
  //     location of the edep and the geometrical parameters
  //     stored in the db, ncell_xz and the grid size

  //if( SDName == "ecalSD" ){
    
  //} 
  //else if( SDName == "hcalSD" ){
  G4double xDim = (G4double) ncell_xz[0] * (*grid_sizePtr);
  cellID[0] =  (G4int) floor( ncell_xz[0] * ( x  + xDim / 2. ) 
                              /  xDim ) ;
    
  G4double zDim = (G4double) ncell_xz[1]* (*grid_sizePtr);
  cellID[1] =  (G4int) floor( ncell_xz[1] * ( z  + zDim / 2. ) 
                              /  zDim ) ;
    
#ifdef TBSD_DEBUG
  G4cout << "x = " << x << G4endl; 
  G4cout << "xDim = " << xDim << G4endl; 
  G4cout << "cellID[0] = " << cellID[0] << G4endl; 
  /*G4cout << "z = " << z << G4endl; 
    G4cout << "zDim = " << zDim << G4endl;*/ 
  G4cout << "cellID[1] = " << cellID[1] << G4endl; 
#endif
  //CRP    cellID[2] =  (int) floor( z /  Geometry::HCal()->layerThickness ) ;
  //}
  //else if( SDName == "catcherSD" ) { 
  //}  
}

void TBSD_VCell02::SetCellPosition( G4int cellId){
  //CRP Basically inverse of method SetCellCoordinates 
  //    Calculates xyz-location of virtual cell in grid 

  // if( SDName == "ecalSD" ){


  // }else if( SDName == "hcalSD" ){
    G4double xDim = (G4double) ncell_xz[0]* (*grid_sizePtr);
    G4double zDim = (G4double) ncell_xz[0]* (*grid_sizePtr);
    cellPos[0] = (G4double) ( xDim / ncell_xz[0] ) * ( (cellId & 0xff)*1.+0.5)-xDim/2. ;
    cellPos[2] = (G4double) ( zDim / ncell_xz[1] ) * ( ( (cellId >> 16) & 0xff)*1.+0.5)-zDim/2. ;

#ifdef TBSD_DEBUG
    G4cout << "cellPos[0] = " << cellPos[0] << G4endl; 
    G4cout << "cellPos[2] = " << cellPos[2] << G4endl; 
#endif

//}else if( SDName == "catcherSD" ){

//}
}










