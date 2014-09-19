//Author Roman Poeschl DESY
//Routines partially adapted from Jeremy McCormick, NIU
#include "Control.hh"
//#include "TBSDVCellscecal01.hh"
#include "TBSDVCellscecal01.hh"
// sub_detectors
#include "TBhcal02.hh"
//120227.1833#include "TBecal02.hh"
#include "TBscecal01.hh"
#include "TBcatcher02.hh"

#include "CGADefs.h"
#include "EncoderTBscecal01.hh"

//#include "CellHit.hh"


#include "G4SDManager.hh"
#include "G4TouchableHandle.hh"
#include "G4StepPoint.hh"
#include "G4VPhysicalVolume.hh"
#include "G4NavigationHistory.hh"

// set this for ProcessHits debugging (lots of output)
///#define TBSD_DEBUG 1

// better proto: sdname, det_enum, ncell_xz, grid_sizePtr
TBSDVCellscecal01::TBSDVCellscecal01(G4String sdname, VSubDetectorDriver* this_detector) :
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
    ncell_xz =  ( static_cast<TBscecal01*> (this_detector) )->GetNCells();
    grid_sizePtr =  ( static_cast<TBscecal01*> (this_detector) )->GetGridSize();
#if TBSD_DEBUG
    G4cout << "ncell_xz = " << ncell_xz << "grid_sizePtr = " << grid_sizePtr << G4endl; 
#endif
  }
  else if (SDName=="catcherSD") {
    moduleID=TBCATCHER;
    ncell_xz =  ( static_cast<TBcatcher02*> (this_detector) )->GetNCells();
    grid_sizePtr =  ( static_cast<TBcatcher02*> (this_detector) )->GetGridSize();
  }
  else {
    G4cerr << "SDName " << SDName << " is unknown type for module enum." << G4endl;
    Control::Abort("TBSDVCellscecal01 - Unknown SDName",
		MOKKA_ERROR_BAD_SENSITIVE_DETECTOR_NAME);
  }

  theEncoder = new EncoderTBscecal01();
}


TBSDVCellscecal01::~TBSDVCellscecal01()
{
  hitMap->clear();
  delete hitMap;
}


void TBSDVCellscecal01::Initialize(G4HCofThisEvent* HCE)
{
  // G4cout << "SD Init: " << SDName << G4endl;

  hitsColl = new TBHitsCollection(SDName, collectionName[0]);

  if (HCID < 0)
    HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  
  HCE->AddHitsCollection(HCID, hitsColl);

  // G4cout << "HCID: " << HCID << G4endl;
  hitMap->clear();
}


G4bool TBSDVCellscecal01::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
#if TBSD_DEBUG
  G4cout << "\nTBSDVCellscecal01::ProcessHits(): " << SDName << G4endl;
#endif

  G4double edep;

  if ((edep=aStep->GetTotalEnergyDeposit())<=0 &&
      aStep->GetTrack()->GetDefinition()->GetParticleType() !="geantino")
    return true;

  const G4StepPoint *preStepPoint = aStep->GetPreStepPoint();
  G4ThreeVector theGlobalPos = preStepPoint->GetPosition();
  
#if TBSD_DEBUG
  G4cout << "Global Hit pos: " << theGlobalPos << G4endl;
  G4cout << "Post Position= " <<
    aStep->GetPostStepPoint()->GetPosition() << G4endl;
#endif
  //Get CellIDs for hit
  G4ThreeVector theLocalPos = theGlobalPos - origin;

#if TBSD_DEBUG
  G4cout 
  << " theLocalPos[0] = " << theLocalPos[0] 
  << " theLocalPos[1] = " << theLocalPos[1]
  << " theLocalPos[2] = " << theLocalPos[2]
  << G4endl;
#endif

  G4TouchableHandle theTouchableHandle = preStepPoint->GetTouchableHandle();
  G4int n_lay = theTouchableHandle->GetHistory()->GetVolume(2)->GetCopyNo();
  TBSDVCellscecal01::SetCellCoordinates( theLocalPos[0]/mm, theLocalPos [1]/mm, n_lay);

//  TBSDVCellscecal01::SetCellCoordinates( theLocalPos[0]/mm, theLocalPos [1]/mm);
  G4int i_x = cellID[0] ;
  // layer #
#ifdef TBSD_DEBUG
  G4cout << "Layer: " << n_lay << G4endl;
#endif
//120228.1235 cellID[0] is determined in SetCellCoordinates.
//  cellID[0] = n_lay;
  G4int n_strip = cellID[1] ;
  // CRP Transform cell coordinates into a unique cellid
  int cellId0 = 0 ;
  int cellId1 = 0 ;
//  int cellId1 = 0 ;
  cellId0 |= (  n_lay  << 0  ) ;  
  //Our prototype is still aligned in y-position !!! 
  cellId0 |= ( n_strip  << 8  ) ;  
  cellId1 |= ( n_strip  << 0  ) ;  
//  cellId |= (  k_z  << 16  ) ;  

  // PDG
  G4int PDG=aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
  G4double time = aStep->GetTrack()->GetGlobalTime() ;

  // FindHit will ++edep if it finds the cell
  if (!FindHit(cellId0, edep , PDG, time )) //fg: hits need pdg and time !
  {
//120228    cellPos[1] = theTouchableHandle->GetHistory()->GetTopTransform().Inverse().TransformPoint(origin) (1);
    cellPos[2] = theTouchableHandle->GetHistory()->GetTopTransform().Inverse().TransformPoint(origin) (2);
#ifdef TBSD_DEBUG
    G4cout << "cellId0 before the Hit: " << (cellId0 & 0xff) << G4endl; 
    G4cout << "cellPos1 before the Hit: " <<  cellPos[1] << G4endl; 
//    G4cout << "cellId_j before the Hit: " <<  ( (cellId >> 16) & 0xff) << G4endl; 
//    assert((cellId & 0xff)<=90);
#endif
    TBSDVCellscecal01::SetCellPosition(cellId0);

    //GM: from now on we'll use EncoderTBscecal01, keeping the order in Proto_CellHit
    cell_ids theCode = theEncoder->encode(0, 0, 0,  
			((cellId0 >> 8) & 0xff),	  // i_x
			n_lay, 
			0);
#ifdef TBSD_DEBUG
    //G4cout << cell_id.
#endif
    // CRP Use the CellHit class defined for the Prototye, i.e. Proto_CellHit 
    //CellHit *theHit = new CellHit (moduleID,                  
    CalHit *theHit = new CalHit(moduleID,//MODULE (see enum @ Control.hh) |P
			       1,        // WI (glob index i)             |S
			       0,        // WJ (glob index j)             |M
			       ((cellId0 >> 8) & 0xff),// I  (local index i) (LAYER)|I
			       n_lay,          // layer                     |J
			       0,// J(local index j) |K
			       0, 
			       cellPos[0],              // cell x   
			       cellPos[1],              // cell y
			       cellPos[2],              // cell z
			       edep,                    // edep
			       Control::primaryId,      // Primary ID
			       PDG, 
			       time,
			       theCode);

#ifdef TBSD_DEBUG
    G4cout << "cellPos[0] = " << cellPos[0] << G4endl;
    G4cout << "cellPos[1] = " << cellPos[1] << G4endl;
    G4cout << "cellPos[2] = " << cellPos[2] << G4endl;
#endif

    hitMap->insert(TBHitMapPair(cellId0, theHit));

    // G4cout << "Adding hit at: " << theGlobalCellCenter << " <" << SDName << ">" << G4endl;
  }

  return true;
}

G4bool TBSDVCellscecal01::FindHit(G4int cellId, G4double edep, G4int PDG, G4double time )
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

void TBSDVCellscecal01::EndOfEvent(G4HCofThisEvent* HCE)
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


void TBSDVCellscecal01::SetCellCoordinates( G4float x, G4float z, G4int n_lay ){
  // CRP Calculate i_x, k_z position of virtual cell from the
  //     location of the edep and the geometrical parameters
  //     stored in the db, ncell_xz and the grid size

  _nstrip_xz[0] = NCOLOMN;
  _nstrip_xz[1] = NROWSTRIP;

  cellID[0] = n_lay;
  G4double xDim = (G4double) ncell_xz[0] * (*grid_sizePtr);
  G4double zDim = (G4double) ncell_xz[1]* (*grid_sizePtr);
//  if ( n_lay%4 == 1 ) {
//  } else if ( n_lay%4 == 2 ) {
//  } else if ( n_lay%4 == 3 ) {
//  } else {
//  }

  if ( n_lay%2 ==0 ) {
     G4float tempoZ = z;
     z = x;
     x = tempoZ;     
  }   
// nstrip_xz should be imput parameter 
// temporarily go to header file  G4int nstrip_xz[2] = {18,4};

  G4int bfrCombiCellx = (G4int) floor( _nstrip_xz[0] * ( x  + xDim / 2. ) 
                                                                / xDim );
  G4int bfrCombiCellz = (G4int) floor( _nstrip_xz[1] * ( z  + zDim / 2. ) 
                                                                / zDim );
  
  cellID[1] = bfrCombiCellx + bfrCombiCellz * _nstrip_xz[0];
    
#ifdef TBSD_DEBUG
  G4cout << "cellID[0] = " << cellID[0] << G4endl; 
  G4cout << "x = " << x << G4endl; 
  G4cout << "xDim = " << xDim << G4endl; 
  G4cout << "bfrCombiCellx = " << bfrCombiCellx << G4endl;
  G4cout << "z = " << z << G4endl; 
  G4cout << "zDim = " << zDim << G4endl;
  G4cout << "bfrCombiCellz = " << bfrCombiCellz << G4endl;
  G4cout << "cellID[1] = " << cellID[1] << G4endl; 
#endif

}

void TBSDVCellscecal01::SetCellPosition( G4int cellId){
  //CRP Basically inverse of method SetCellCoordinates 
  //    Calculates xyz-location of virtual cell in grid 
  _nstrip_xz[0] = NCOLOMN;   //18 
  _nstrip_xz[1] = NROWSTRIP;  //4


  // if( SDName == "ecalSD" ){
   G4int n_layer = (cellId & 0xff);
#ifdef TBSD_DEBUG
   G4cout << "n_layer_in_SetCellPosition = " << n_layer << G4endl;
#endif

  G4int n_strip = ((cellId >> 8) & 0xff );
  G4int n_rowStrip = (G4int) n_strip /_nstrip_xz[0];
  G4int n_colomnStrip = (G4int) n_strip%_nstrip_xz[0];

  G4double xDim = (G4double) ncell_xz[0]* (*grid_sizePtr);
  G4double yDim = (G4double) ncell_xz[0]* (*grid_sizePtr);
#ifdef TBSD_DEBUG
   G4cout << "n_strip_in_SetCellPosition = " << n_strip << G4endl;
   G4cout << "n_rowStrip_in_SetCellPosition = " << n_rowStrip << G4endl;
   G4cout << "n_colomnStrip_in_SetCellPosition = " << n_colomnStrip << G4endl;
   G4cout << "xDim_in_SetCellPosition = " << xDim << G4endl;
   G4cout << "yDim_in_SetCellPosition = " << yDim << G4endl;
#endif

   G4double finePos =
     (G4double) ( xDim / _nstrip_xz[0] ) *( n_colomnStrip + 0.5 )-xDim/2.;
   G4double unfinePos = 
     (G4double) (yDim / _nstrip_xz[1] ) *( n_rowStrip + 0.5 )-yDim/2.;
   G4int nx = 0;
   G4int ny = 0;
   if ( n_layer%2 == 1 ) {
     cellPos[0] = finePos;
     cellPos[1] = unfinePos;
   } else {
     cellPos[0] = unfinePos;
     cellPos[1] = finePos;
   } 

//     cellPos[2] = ( n_layer - 1 ) * 

  // }else if( SDName == "hcalSD" ){
//    cellPos[0] = (G4double) ( xDim / ncell_xz[0] ) * ( (cellId & 0xff)*1.+0.5)-xDim/2. ;
//    cellPos[2] = (G4double) ( zDim / ncell_xz[1] ) * ( ( (cellId >> 16) & 0xff)*1.+0.5)-zDim/2. ;

#ifdef TBSD_DEBUG
    G4cout << "cellPos[0] = " << cellPos[0] << G4endl; 
    G4cout << "cellPos[1] = " << cellPos[1] << G4endl; 
#endif

//}else if( SDName == "catcherSD" ){

//}
}










