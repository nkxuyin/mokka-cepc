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
// $Id: ECSD03.cc,v 1.8 2007/07/10 16:27:43 mora Exp $
// $Name: mokka-07-00 $
//
// 
#include "Control.hh"
#include "ECSD03.hh"
#include "G4StepPoint.hh"
#include "G4TransportationManager.hh"
#include "G4ParticleDefinition.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "globals.hh"
#include "G4GeometryTolerance.hh"
#include "G4UImanager.hh"
#include <assert.h>

ECSD03::ECSD03(G4double Idim,G4double Jdim,G4double Thickness,
	G4double guard_ring_size, G4double inter_wafer_gap,
	G4int nmax_cell_x, G4int nmax_cell_z, G4int n_guard_ring_zones,
	G4int Piece,G4String SDname,G4bool useID1) 
  : SD03(Idim,Jdim,Thickness,guard_ring_size,inter_wafer_gap,nmax_cell_x,nmax_cell_z,n_guard_ring_zones,Piece,SDname,useID1)
{
}

ECSD03::~ECSD03() 
{
}

G4ThreeVector ECSD03::GetCellCenter(G4int,G4int pS,G4int pM,
				G4int pI,G4int pJ,G4int pK) {
  assert (pS>0);
  assert (pM>=0);
  assert (pI>=0);
  assert (pJ>=0);
  assert (pK>0);
  assert(pS<=MAX_STAVES);
  assert(pM<MAX_MODULES);
  assert (pK<=MAX_LAYERS);

  // builds the cell center coodinates for the I,J
  G4ThreeVector localCellCenter;

  G4int iX = (G4int)((pI+1)/theNMaxCellX);
  G4int iStrip = (pI+1)%theNMaxCellX;
  G4int iY = (G4int)((pJ+1)/theNMaxCellZ);
  G4int iCell = (pJ+1)%theNMaxCellZ;

  G4double xInSiPlane = iX*(theNMaxCellX*CellDim(0) + 
	     2*theGuardRingSize + theInterWaferGap);
  if(iStrip > 0)
	     xInSiPlane += (iStrip-1)*CellDim(0) + CellDim(0)/2. 
		     + theGuardRingSize;
  else if(iStrip == 0)
	     xInSiPlane -= (theInterWaferGap+theGuardRingSize+CellDim(0)/2.);

  G4double yInSiPlane = iY*(theNMaxCellZ*CellDim(2) + 
	     2*theGuardRingSize + theInterWaferGap);
  if(iCell > 0)
	     yInSiPlane += (iCell-1)*CellDim(2) + CellDim(2)/2. 
		     + theGuardRingSize;
  else if(iCell == 0)
	     yInSiPlane -= (theInterWaferGap+theGuardRingSize+CellDim(2)/2.);

  localCellCenter[0]=Layers[pK-1]->X0 + xInSiPlane;
  localCellCenter[1]=Layers[pK-1]->Y0 + yInSiPlane;
  localCellCenter[2]=Layers[pK-1]->Z0;

  assert (InverseStavesRotationMatrices[pS-1]!=0);
  
  // find out the actual cell center coodinates in the reference module
  G4ThreeVector theCellCenter = 
    *InverseStavesRotationMatrices[pS-1] * localCellCenter;
  
  assert (ModulesZOffsets[pM]!=0);
  theCellCenter[2] += *ModulesZOffsets[pM];

  // The standard module reference is the Z>0 one.
  // If Z<0, rotate the position to the positive one.
  G4RotationMatrix rot1;
//  if(pP == ECALENDCAPMINUS)
  if(pM == 0)
    {
      rot1.rotateY(pi);
    }
  // If Z<0 rot1 keeps the good rotation for the Cell Center.
  theCellCenter  = rot1 * theCellCenter;

  return theCellCenter;
}

G4bool ECSD03::ProcessHits(G4Step *aStep,G4TouchableHistory*)
{
  if(aStep->GetStepLength() <= 
	G4GeometryTolerance::GetInstance()->GetSurfaceTolerance())
        return true;

  // process only if energy>0.
  G4double edep;
  if ((edep = aStep->GetTotalEnergyDeposit())<=0. &&
      aStep->GetTrack()->GetDefinition()->GetParticleType()!="geantino")
    return true;
  
  G4double time = aStep->GetTrack()->GetGlobalTime() ;

  const G4VTouchable *theTouchable =aStep->GetPreStepPoint()->GetTouchable();

  G4int depth = theTouchable->GetHistory()->GetDepth();
 
  G4ThreeVector origin;

  G4AffineTransform theAffineTransformation, theInverseAffineTransformation;

  theAffineTransformation = theTouchable->GetHistory()->GetTopTransform();
//		          ->GetTransform(depth);
  theInverseAffineTransformation = theAffineTransformation.Inverse();

  G4ThreeVector theCellCenter;

  G4int theSDPiece, theStave, theModule, I, J, theLayer, zone = 0;

  // The hit will deposited in the midle of the step
  G4ThreeVector thePosition = 
    (aStep->GetPreStepPoint()->GetPosition()+
     aStep->GetPostStepPoint()->GetPosition())*0.5;

  if(theTouchable->GetHistory()->GetVolume(depth)->IsReplicated()) {
	if(!GetCellIndices(theTouchable, 
#ifdef MOKKA_DEBUG
			thePosition, 
#endif
			theSDPiece, theStave, 
			theModule, I, J, theLayer))
	  return false;

	theCellCenter =
		theInverseAffineTransformation.TransformPoint(origin);
  }
  else {
	GetNearestCell(theTouchable, thePosition, theSDPiece, theStave, 
			theModule, I, J, theLayer, zone);

	if(zone < 0)
		return false;

	theCellCenter = GetCellCenter(theSDPiece, theStave, theModule, I, J, 
			theLayer);
  }
  
#ifdef MOKKA_DEBUG
  // thePosition  = rot1 * thePosition;
  G4ThreeVector distCenter = thePosition - theCellCenter;
  if (distCenter.mag() > (CellDim(0) + CellDim(2)) / 2.)
    {
      G4cout << "======= ASSERT WILL CRASH :\n"
	     << "\ntheSDPiece = " << theSDPiece
	     << ", theStave = " << theStave 
	     << ", theModule = " << theModule
	     << "\nI = " << I << ", J= " << J << ", K = " << theLayer
	     << "\nthePosition = " << thePosition
	     << ", theCellCenter = " << theCellCenter
	     << ", distCenter.mag() = " << distCenter.mag() << G4endl;
      Control::Abort("ECSD03::ProcessHits: Assertion failed (distCenter.mag() > (CellDim(0) + CellDim(2)) / 2.)",MOKKA_OTHER_ERRORS);
    }
#endif

  // creates a new cell or add the energy to the cell if it already exists
  G4int PID = 
    Control::GetControl()->GetPIDForCalHit(aStep);
  assert(PID!=-1);

  G4int PDG=
    aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
  
  cell_ids theCode = 
    theEncoder->encode(theStave,theModule,
		   I,J,theLayer,zone);
  
  G4bool found=false;
  G4int n_hit = CalCollection->entries();

  for(G4int i_hit = 0; i_hit< n_hit; i_hit++)
    if((*CalCollection)[i_hit]->testCell(theCode)) {
 	    (*CalCollection)[i_hit]->AddEdep(PID,PDG,edep, time ); 
	    found = true;
	    break;
    }
  
  if(!found) {
	if(zone == 0) // the hit is inside a cell
		CalCollection->
			insert(new CalHit(theSDPiece, 
				  theStave, 
				  theModule, 
				  I, 
				  J, 
				  theLayer,
				  zone,
				  theCellCenter (0),
				  theCellCenter (1),
				  theCellCenter (2),
				  edep, PID, PDG, time, theCode));
 	else {        // the hit is in the guard-ring
		G4int newPieceID = GRENDCAPPLUS;
		if(theSDPiece == ECALENDCAPMINUS)
			newPieceID = GRENDCAPMINUS;
		CalCollection->
			insert(new CalHit(newPieceID, 
				  theStave, 
				  theModule, 
				  I, 
				  J, 
				  theLayer,
				  zone,
				  theCellCenter (0),
				  theCellCenter (1),
				  theCellCenter (2),
				  edep, PID, PDG, time, theCode));
	}
  }
  
  return true;
}

G4bool ECSD03::GetCellIndices(const G4VTouchable * theTouchable,
#ifdef MOKKA_DEBUG
		G4ThreeVector& thePosition, 
#endif
		G4int& theSDPiece, G4int& theStave, 
		G4int& theModule, G4int& I, G4int& J, G4int& theLayer) {

  G4int depth = theTouchable->GetHistory()->GetDepth();

  theLayer = theTouchable->GetHistory()->GetVolume(2)->GetCopyNo();

  if( theLayer<=0 || theLayer > MAX_LAYERS ) {
    G4cout << "theLayer = " << theLayer ;
    G4cout << ", history->GetHistory()->GetDepth()= "
	   << depth << G4endl;
    for (G4int ii = 0; ii <= depth; ii++) 
      G4cout << "volname(" << ii << ")= "
	     << theTouchable->GetHistory()->GetVolume(ii)->GetName()
	     << G4endl;
    G4cout << "Mokka warning: BAD history->GetHistory()->GetVolume(2)->GetCopyNo(), skipping the hit"
	   << G4endl;
    return false;
  }
  
  assert (theLayer>0);
  
  // Find out the stave and module id looking for the
  // module copy number and decoding it

  G4int ModuleCopyNumber=-1;
  for (G4int idepth = 0; idepth <= depth; idepth++) {
    ModuleCopyNumber=theTouchable->GetHistory()->GetVolume(idepth)->GetCopyNo();
   if(ModuleCopyNumber>100) break;
  }

#ifdef MOKKA_DEBUG
  if (ModuleCopyNumber == 0) 
    {
      G4cout << "MOKKA WARNING: ModuleCopyNumber==0 in ECSD03::GetCellIndices, "
	     << "at " << thePosition << ", SDPiece = " << SDPiece << G4endl;

      G4cout << "Volume hierarchy in G4TouchableHistory :\n";
      for (G4int idepth = 0; idepth <= depth; idepth++) {
	G4cout << "level " << idepth << ", volume "
	       << theTouchable->GetHistory()->GetVolume(idepth)->GetName()
	       << " ,copy number = " << theTouchable->GetHistory()->GetVolume(idepth)->GetCopyNo() 
	       << G4endl;
      }
      Control::Abort("ECSD03::GetCellIndices: Assertion failed (ModuleCopyNumber == 0)",MOKKA_OTHER_ERRORS);
    }
#endif

  theSDPiece = ModuleCopyNumber/100;

#ifdef MOKKA_DEBUG
  G4int tmp = SDPiece;
  if(thePosition(2)>0) tmp+=(ECALENDCAPPLUS - ECALENDCAPMINUS);
  if (ModuleCopyNumber / 100 != tmp)
    Control::Abort("ECSD03::GetCellIndices: Assertion failed (ModuleCopyNumber / 100 != tmp)",MOKKA_OTHER_ERRORS);
#endif
  
  theStave = (ModuleCopyNumber-theSDPiece*100)/10;
  theModule = (ModuleCopyNumber-theSDPiece*100)%10;

  G4int jCell = theTouchable->GetReplicaNumber();
  G4int iCell = theTouchable->GetReplicaNumber(1);

  G4int fullWaferCopyNo = theTouchable->GetHistory()->GetVolume(depth-3)
	  		->GetCopyNo();

  G4int iX = fullWaferCopyNo/100000;
  G4int iZ = (fullWaferCopyNo - iX*100000)/1000;

  I = iX*theNMaxCellX + iCell;
  J = iZ*theNMaxCellZ + jCell;

  return true;
}

cell_ids ECSD03::GetCellIndex(double X, double Y, double Z,
               int & flag, double xDir, double yDir,
                double zDir) {// cell = 1; ring = 0; else = -1

  G4ThreeVector point(X*mm, Y*mm, Z*mm);
  G4ThreeVector direction(xDir, yDir, zDir);

  G4TouchableHistory * theTouchable = new G4TouchableHistory();
  G4TransportationManager::GetTransportationManager()->
	GetNavigatorForTracking()->
	LocateGlobalPointAndUpdateTouchable(point, direction,
						theTouchable);

  G4int depth = theTouchable->GetHistory()->GetDepth();

  G4int P, S, M, I, J, K, zone=0;
  if(theTouchable->GetHistory()->GetVolume(depth)->IsReplicated()) {
	if(!GetCellIndices(theTouchable, 
#ifdef MOKKA_DEBUG
                        point,
#endif
			P, S, M, I, J, K)) {
	  cell_ids res; res.id0=0; res.id1=0;
	  return res;
	}

	flag = 1;
  }
  else {
	G4int copyNo = theTouchable->GetHistory()->GetVolume(depth)
		->GetCopyNo();
	if(abs(copyNo) > 100000) {
		GetNearestCell(theTouchable, point, P, S, M, I, J, K, zone);
		flag = 0;
	}
	else {
		I = 0; J = 0; S = 0; M = 0; K = 0; flag = -1;
	}
  }

return theEncoder->encode(S,M,I,J,K,zone);
}

void ECSD03::GetNearestCell(const G4VTouchable * theTouchable,
	G4ThreeVector thePosition,
	G4int& P, G4int& S, G4int& M, G4int& I, G4int& J, G4int& K,
	G4int & zone) {

  G4int depth = theTouchable->GetHistory()->GetDepth();

  K = theTouchable->GetHistory()->GetVolume(2)->GetCopyNo();
  assert(K>0);

  G4int ModuleCopyNumber = -1;
  for (G4int idepth = 0; idepth <= depth; idepth++) {
	ModuleCopyNumber=theTouchable->GetHistory()->GetVolume(idepth)->
		GetCopyNo();   
	if(ModuleCopyNumber>100) break;
  }

#ifdef MOKKA_DEBUG
  if (ModuleCopyNumber == 0) {
	G4cout << "MOKKA WARNING: ModuleCopyNumber==0 in ECSD03::GetNearestCell, "
	<< "at " << thePosition << ", SDPiece = " << SDPiece << G4endl;
	G4cout << "Volume hierarchy in G4TouchableHistory :\n";
	for (G4int idepth = 0; idepth <= depth; idepth++) {
		G4cout << "level " << idepth << ", volume "
		<< theTouchable->GetHistory()->GetVolume(idepth)->GetName()
		<< " ,copy number = " << theTouchable->GetHistory()->GetVolume(idepth)->GetCopyNo()
		<< G4endl;
	}
        Control::Abort("ECSD03::GetNearestCell: Assertion failed (ModuleCopyNumber == 0)",MOKKA_OTHER_ERRORS);
  }
#endif

  P = ModuleCopyNumber/100;

#ifdef MOKKA_DEBUG
  G4int tmp = SDPiece;
  if(tmp==ECALENDCAPMINUS && thePosition(2)>0) tmp+=(ECALENDCAPPLUS-ECALENDCAPMINUS);
  if (ModuleCopyNumber / 100 != tmp)
    Control::Abort("ECSD03::GetNearestCell: Assertion failed (ModuleCopyNumber / 100 != tmp)",MOKKA_OTHER_ERRORS);
#endif

  S = (ModuleCopyNumber - P*100)/10;
  M = (ModuleCopyNumber - P*100)%10;

  G4int fullWaferCopyNo = theTouchable->GetHistory()->GetVolume(depth)->
	  GetCopyNo();

  G4int iX = fullWaferCopyNo/100000;
  G4int iY = (fullWaferCopyNo - iX*100000)/1000;
  G4int the_n_cell_x = (fullWaferCopyNo - iX*100000 - iY*1000)/100;
  G4int the_n_cell_y = (fullWaferCopyNo - iX*100000 - iY*1000 - 
		  the_n_cell_x*100)/10;

  G4AffineTransform theAffineTransform= theTouchable->GetHistory()
	  ->GetTopTransform();

  G4ThreeVector theLocalPosition = 
		  theAffineTransform.TransformPoint(thePosition);


  G4int iCell, jCell;
  G4int compare;
  G4double mI;
  if(the_n_cell_x%2==0){//even X-cells wafer
	compare = the_n_cell_x;
  	mI = theLocalPosition(0)/CellDim(0);
  }
  else{//odd X-cells wafer
	compare = the_n_cell_x + 1;
	G4double signx = theLocalPosition(0)/fabs(theLocalPosition(0));
	mI = (theLocalPosition(0)+signx*CellDim(0)/2)/CellDim(0);
  }
  G4int mIint = static_cast<G4int>(mI);
  if(mI > 0) {
    if(compare == 2 * mIint)
	iCell = the_n_cell_x - 1;
    else
	iCell = mIint + static_cast<G4int>(the_n_cell_x/2);
  }
  else {
    if(compare == - 2 * mIint)
	iCell = 0;
    else
	iCell = mIint + compare/2 - 1;
  }

  G4double mJ;
  if(the_n_cell_y%2==0){//even Y-cells wafer
	compare = the_n_cell_y;
  	mJ = theLocalPosition(1)/CellDim(2);
  }
  else{//odd Y-cells wafer
	compare = the_n_cell_y + 1;
	G4double signy = theLocalPosition(1)/fabs(theLocalPosition(1));
	mJ = (theLocalPosition(1)+signy*CellDim(2)/2)/CellDim(2);
  }
  G4int mJint = static_cast<G4int>(mJ);
  if(mJ > 0) {
    if(compare == 2 * mJint)
	jCell = the_n_cell_y - 1;
    else
	jCell = mJint + static_cast<G4int>(the_n_cell_y/2);
  }
  else {
    if(compare== - 2 * mJint)
	jCell = 0;
    else
	jCell = mJint + compare/2 - 1;
  }
  
  I = iX*theNMaxCellX + iCell;
  J = iY*theNMaxCellZ + jCell;

  if(theNGuardRingZones == 0)
	  return;

  G4double delta_x = fabs(theLocalPosition(0)) - CellDim(0) * the_n_cell_x / 2;
  G4double delta_y = fabs(theLocalPosition(1)) - CellDim(2) * the_n_cell_y / 2;
  
  G4double zone_width = theGuardRingSize / theNGuardRingZones;

  G4int zone_x = static_cast<G4int>(delta_x / zone_width); 
  G4int zone_y = static_cast<G4int>(delta_y / zone_width); 

  if((delta_x < 0) && (delta_y < 0))
	  zone = -2;

  else if((iCell != 0) && (iCell != (the_n_cell_x-1)))
	  zone = zone_y;

  else if((jCell != 0) && (jCell != (the_n_cell_y-1)))
	  zone = zone_x;
  
  else if((delta_x <= 0) && (delta_y >= 0))
	  zone = zone_y;
  
  else if((delta_y <= 0) && (delta_x >= 0))
	  zone = zone_x;

  else if((zone_x == 0) && (delta_y <= 0))
	  zone = 0;

  else if((zone_y == 0) && (delta_x <= 0))
	  zone = 0;

  else {
  	G4double radius = sqrt(delta_x * delta_x + delta_y * delta_y);

	zone = static_cast<G4int>(radius / zone_width);
  }

  if(zone >= theNGuardRingZones)
	  zone = theNGuardRingZones - 1;

/*
#ifdef MOKKA_DEBUG
  if (zone < 0) {
	  G4cout << "WARNING: ECSD03::GetNearestCell-negative guard ring zone: "
	         << " I: " << I << " J: " << J << " K: " << K << " P: " << P <<
		 " S: " << S << " M: " << M << " fullWaferCopyNo: " << 
		 fullWaferCopyNo << " zone: " << zone << 
		 " zone width: " << zone_width << 
		 " delta_x: " << delta_x << " delta_z: " << delta_z << 
		 " Position: " << thePosition << 
		  " local pos: " << theLocalPosition << G4endl;
          Control::Abort("CSD03::GetNearestCell: Assertion failed (zone < 0)");
  }
#endif
*/
   //GM: 30.01.06 : new coding convention; zone = 0 if hit in cell;
   //                                      zone = 1, ..., theNGuardRingZones
   //                                              if hit in Guard Ring
   zone++;

}
