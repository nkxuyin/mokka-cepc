/*
$Id$
$Name$
*/
#include "Control.hh"
#include "LumiCalSD.hh"

#include "Encoder32Fcal.hh"
#include "CalHit.hh"
#include "CGADefs.h"
                                                                                

// comment-in for verbose debugging output from SD
//#define LCSD_DEBUG 1
//#define LCSD_CHECK_POSITION 1

LumiCalSD::LumiCalSD( G4String sdname,
		      G4String LumiCal_Type,
		      G4double CalRhoMin,    
		      G4double CalPhiOffset,    
		      G4double cellDimRho,
		      G4double cellDimPhi,
		      G4int nCellRho,
		      G4int nCellPhi,
		      G4int modID)
  
  :  VSensitiveDetector(sdname), 
     HCID(-1),
     SDName(sdname), 
     hitsColl(0)

{


  G4cout << "SD created <" << SDName << ">" << G4endl;
  G4String CollName = SDName+"Collection";
  VirtualCell = false;
  collectionName.insert(CollName);
  origin = G4ThreeVector();
  hitMap = new LCHitMap;

  // set primary args
  this->LumiCal_Type = LumiCal_Type;
  SetRhoMin(CalRhoMin);
  SetPhiOffset(CalPhiOffset);
  SetRhoCellDim(cellDimRho);
  SetPhiCellDim( cellDimPhi);
  SetNCellRho(nCellRho);
  SetNCellPhi( nCellPhi);
  SetModuleID(modID);

  theEncoder = new Encoder32Fcal();
}

LumiCalSD::LumiCalSD( G4String sdname,
		      G4String LumiCal_Type,
		      G4double CalRhoMin,    
		      G4double CalPhiOffset,    
		      G4double cellDimRho,
		      G4double cellDimPhi,
		      G4int nCellRho,
		      G4int nCellPhi,
		      G4int modID,
		      G4bool virtualcell)
  
  :  VSensitiveDetector(sdname), 
     HCID(-1),
     SDName(sdname), 
     hitsColl(0),
     VirtualCell(virtualcell)

{


  G4cout << "SD created <" << SDName << ">" << G4endl;
  G4String CollName = SDName+"Collection";
  collectionName.insert(CollName);
  origin = G4ThreeVector();
  hitMap = new LCHitMap;

  // set primary args
  this->LumiCal_Type = LumiCal_Type;
  SetRhoMin(CalRhoMin);
  SetPhiOffset(CalPhiOffset);
  SetRhoCellDim(cellDimRho);
  SetPhiCellDim( cellDimPhi);
  SetNCellRho(nCellRho);
  SetNCellPhi( nCellPhi);
  SetModuleID(modID);
  

  theEncoder = new Encoder32Fcal();
}

LumiCalSD::~LumiCalSD()
{
  hitMap->clear();
  delete hitMap;
  delete theEncoder;
}


void LumiCalSD::Initialize(G4HCofThisEvent* HCE)
{
#ifdef LCSD_DEBUG
  G4cout << "SD Init: " << SDName << G4endl;
#endif

  //Create a (G4) hit collection for this event
  hitsColl = new LCHitsCollection(SDName, collectionName[0]);

  if (HCID < 0) {
    HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  }
  
  //Would be sufficient to add the collection at the end of the event
  //Doing it here just suppresses a warning during compilation
  HCE->AddHitsCollection(HCID, hitsColl);

#ifdef LCSD_DEBUG
  G4cout << "HCID: " << HCID << G4endl;
#endif
  
  //Reset the hit map
  hitMap->clear();
}

G4bool LumiCalSD::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
  
#ifdef LCSD_DEBUG
  G4cout << G4endl << "LumiCalSD::ProcessHits(): " << SDName << G4endl;
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
  
#ifdef LCSD_DEBUG
  G4cout << "Global Hit pos " << theGlobalPos << G4endl;
  G4cout << "Post Position " << aStep->GetPostStepPoint()->GetPosition() << G4endl;
  G4cout << "Deposited energy: " << edep << G4endl;  
#endif
  

  // get LumiCal sub-module number
  //fg: start numbering from one - as expected by Encoder32Fcal::encode
  G4int i_side = 1 + theTouchable->GetHistory()->GetVolume(1)->GetCopyNo();
  // get Layer Number from Touchable
  G4int n_lay = theTouchable->GetHistory()->GetVolume(2)->GetCopyNo();
  //get strip index from Touchable
  G4int i_phi = 0;
  G4int i_rho = 0;
  if ( !VirtualCell ){
    if( LumiCal_Type == "strip"){
      if(n_lay%2 == 0){
	i_phi = theTouchable->GetReplicaNumber();
      }else{
	i_rho = theTouchable->GetReplicaNumber();
      }
    }else{
      i_rho = theTouchable->GetReplicaNumber();
      i_phi = theTouchable->GetReplicaNumber(1);
    }
  }else{
    const G4StepPoint *postStepPoint = aStep->GetPostStepPoint();
    G4ThreeVector GlobalHitPos = ( theGlobalPos + (postStepPoint->GetPosition())) / 2.;
    G4ThreeVector LocalHitPos = theTouchable->GetHistory()->GetTopTransform().TransformPoint(GlobalHitPos) ;
    G4double rho = LocalHitPos.getRho();
    G4double phi = LocalHitPos.getPhi();
    phi = ( phi < 0. ) ? phi + 2.* M_PI : phi; 
    i_phi = (G4int)floor ( phi / cellDimPhi );
    // we take abs() to insure tiny negative get 0 instead -1 
    i_rho   = (G4int)floor(std::abs( rho - CalRhoMin ) / cellDimRho );
  }

  // pack i, j, k cell coordinates into a unique, int cellid
  int cellId = 0 ;
  cellId |= (  i_rho   << 0  ) ;  
  cellId |= (  i_phi   << 8  ) ;  
  cellId |= (  n_lay   << 16 ) ;  
  cellId |= (  i_side  << 24 ) ; 



#ifdef LCSD_DEBUG
  //  G4cout << "GlobalCellPos " << GlobalCellPos << G4endl;
  G4cout << "Cell ID : " << cellId << G4endl;   
  G4cout << "Cell Indices Theta,Phi,layer: " << i_rho << ", " << i_phi << ", " << n_lay << G4endl;   
#endif


  // get the PDG encoding for the particle
  G4int PDG=aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
  
  // get the global time
  G4double globalTime = aStep->GetTrack()->GetGlobalTime() ;
  
  if (!FindHit(cellId, edep , PDG, globalTime ) && 
      !( (i_rho > NstripRho) || (i_phi > NstripPhi) ) )  {
    
    
    //FG: ---- now  we need to compute the global position of the cell center in x,y,z
    
    
    // FG: the local coordinate system of the cell replica is the same as for the disk 
    //     containing the the pad - rotated by phi of the phi replica, i.e.
    //     the origin is at the center of the disk
    //     hence the position of the cell center in its coordinate system is
    //     given by:
    G4ThreeVector localCellPos(  CalRhoMin+((G4double)i_rho+0.5000)*cellDimRho, 0. , 0. );
    
    if( VirtualCell ) {
      // B.P. In this case local system is the plane
      localCellPos.setPhi( ( G4double(i_phi) + 0.5000 ) *cellDimPhi );
      
    } else { 
      // B.P here the local system is single cell  
      localCellPos.setPhi(0.5000 * cellDimPhi);
    }
    
    // compute GlobalCellPos based on touchable with localCellPos
    G4ThreeVector GlobalCellPos = theTouchable->GetHistory()->GetTopTransform().Inverse().TransformPoint(localCellPos);
    
    //FG: don't set cell position in cylindrical system    
    //   GlobalCellPos.setX( CalRhoMin+((G4double)i_rho+0.5000)*cellDimRho );  
    //   GlobalCellPos.setY( PhiOffset+((G4double)i_phi+0.5000)*cellDimPhi ); 
    
  


#ifdef LCSD_CHECK_POSITION
    // check distance of true step point to cell center in local coordinate frame 
    // (this is where dphi and drho are defined
    
    // transform the global step position into the cell's coordinate system
    G4ThreeVector localstepPos = theTouchable->GetHistory()->GetTopTransform().TransformPoint(theGlobalPos) ;
    
    // check for delta phi and delta rho -  allow for tiny rounding errors:
    if(   std::abs( localCellPos.phi() - localstepPos.phi() ) > cellDimPhi*1.000001/2. )
      std::cout << " warning dphi too large : cell : " << localCellPos.phi() 
		<< " step " <<  localstepPos.phi() 
		<< " ----- \t" << ( localCellPos.phi() - localstepPos.phi() ) / cellDimPhi * 2.
		<< std::endl ;
    
    if(   std::abs( localCellPos.rho() - localstepPos.rho() ) > cellDimRho*1.000001/2. )
      std::cout << " warning drho too large : cell : " << localCellPos.rho() 
		<< " step " <<  localstepPos.rho() 
		<< " ----- \t" << ( localCellPos.rho() - localstepPos.rho() ) / cellDimRho * 2.
		<< std::endl ;
#endif
    
#ifdef LCSD_DEBUG
      int MaxCellId = 0;
      MaxCellId |= (NstripRho << 0);
      MaxCellId |= (NstripPhi << 8);
      MaxCellId |= (30 << 16);
      MaxCellId |= (2 << 24);
      G4cout << "MaxCellId : " <<  (MaxCellId & 0xff)  << G4endl;
      G4cout << "cellId : " <<  cellId << G4endl;
      G4cout << "cellId  before the Hit (side): " <<  ( (cellId >> 24) & 0xff) << G4endl; 
      G4cout << "cellId  before the Hit (layer#): " << ( (cellId >> 16) & 0xff)<< G4endl; 
      G4cout << "cellId  before the Hit (phi-cell#): " << ( (cellId >> 8) & 0xff) << G4endl; 
      G4cout << "cellId  before the Hit (rho-cell#): " << (cellId & 0xff) << G4endl; 
#endif
      
      // assert id w/in valid range
      assert((cellId & 0xff)<=120);
      
      // GM: we now use Encoder32Fcal, that has another order than the cellId
      // set by Bogdan
      cell_ids theCode = theEncoder->encode(i_side,i_rho,i_phi,n_lay,0,0);

      // create CellHit object from hit class defined for the Prototype
      CalHit *theHit = new CalHit (
			moduleID, // P
			i_side,   // S
			0,        // M
			i_rho,    // I
			i_phi,   // J
			n_lay,    // K
			0, 			  // GRZone
			GlobalCellPos.x(),       // GlobalCellPosRho
			GlobalCellPos.y(),       // GlobalCellPosPhi
			GlobalCellPos.z(),       // GlobalCellPosZ 
			edep,                    // edep
			Control::primaryId,      // Primary ID
			PDG,                     // PDG encoding
			globalTime, 		 // global time
			theCode);		 // CellID's
      

      //Insert this cellID into the hitmap
      hitMap->insert(LCHitMapPair(cellId, theHit));
    }
  
  
  return true;
  
}



//Check whether we ahve this cell already in our collection and update
//if present
G4bool LumiCalSD::FindHit(G4int cellId, G4double edep, G4int PDG, G4double globalTime )
{

  //fg: use find method of map !
  LCHitMapIterator iter = hitMap->find( cellId ) ;

  if( iter != hitMap->end() ) {

    (iter->second)->AddEdep( Control::primaryId, PDG, edep, globalTime/ns ) ; 

    return true ;
  }

  return false ;

//   LCHitMapIterator iter;
//   for (iter=hitMap->begin(); iter != hitMap->end(); iter++) {
//     if ( ( (iter->first)-cellId) == 0) {
//       (iter->second)->AddEdep( Control::primaryId, PDG, edep, globalTime/ns ) ; 
//       return true;
//     }                  
//   } 
//   return false;
}

void LumiCalSD::SetRhoMin(G4double rmin){
  CalRhoMin = rmin;
}

void LumiCalSD::SetPhiOffset(G4double phi0){
  PhiOffset = phi0;
}

void LumiCalSD::SetNCellRho(G4int nx ) 
{ 
  // set number of cellS in kRho direction
  NstripRho = nx; 
  assert(nx > 0 );
} 
void LumiCalSD::SetNCellPhi( G4int ny) 
{ 
  // set number of cellS in  kPhi  direction
  NstripPhi = ny; 
  assert(ny > 0 );
} 

void LumiCalSD::SetRhoCellDim(G4double c1) 
{ 
  cellDimRho = c1; 
  assert(cellDimRho > 0 );
}
void LumiCalSD::SetPhiCellDim(G4double c1) 
{ 
  cellDimPhi = c1; 
  assert(cellDimPhi > 0 );
}


void LumiCalSD::SetModuleID(G4int m)
{
  moduleID = m;
}


//Pass the hit collection to the other G4 hits collection
//These collections will be transformed into the LCIO Collection you
//find in the output file
void LumiCalSD::EndOfEvent(G4HCofThisEvent* HCE)
{
  LCHitMapIterator iter;

#ifdef LCSD_DEBUG
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
