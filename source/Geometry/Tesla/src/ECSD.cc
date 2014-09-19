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
// $Id: ECSD.cc,v 1.12 2006/03/02 16:23:40 musat Exp $
// $Name: mokka-07-00 $
//
// 
#include "Control.hh"
#include "ECSD.hh"
#include "G4StepPoint.hh"
#include "G4TransportationManager.hh"
#include "G4ParticleDefinition.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "globals.hh"
#include "G4UImanager.hh"
#include <assert.h>

ECSD::ECSD(G4double Idim,G4double Jdim,G4double Thickness,G4int Piece,G4String SDname, G4bool id1Flag) 
  : SD(Idim,Jdim,Thickness,Piece,SDname,id1Flag)
{
}

ECSD::~ECSD() 
{
}

G4ThreeVector ECSD::GetCellCenter(G4int,G4int pS,G4int pM,
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
  localCellCenter[0]=Layers[pK-1]->X0 + pI*CellDim(0) + CellDim(0)/2.;
  localCellCenter[1]=Layers[pK-1]->Y0 + pJ*CellDim(2) + CellDim(2)/2.;
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

G4bool ECSD::ProcessHits(G4Step *aStep,G4TouchableHistory*)
{

  // process only if energy>0.
  G4double edep;
  if ((edep = aStep->GetTotalEnergyDeposit())<=0. &&
      aStep->GetTrack()->GetDefinition()->GetParticleType()!="geantino")
    return true;
  
  G4double time = aStep->GetTrack()->GetGlobalTime() ;
  
  // Find out the stave and module id looking for the
  // module copy number and decoding it


//   G4TouchableHistory *history =
//     G4TransportationManager::GetTransportationManager()->
//     GetNavigatorForTracking()->CreateTouchableHistory();
  
  //  G4TouchableHistory *history =aStep->GetPreStepPoint()->GetTouchable()

  const G4VTouchable *history =aStep->GetPreStepPoint()->GetTouchable();

  G4int theLayer = history->GetVolume(1)->GetCopyNo();

  if( theLayer<=0 || theLayer > MAX_LAYERS ) {
    G4cout << "theLayer = " << theLayer ;
    G4cout << ", history->GetHistory()->GetDepth()= "
	   << history->GetHistory()->GetDepth()
	   << G4endl;
    for (G4int ii = 0; ii <= history->GetHistory()->GetDepth(); ii++) 
      G4cout << "volname(" << ii << ")= "
	     << history->GetVolume(ii)->GetName()
	     << G4endl;
    G4cout << "Mokka warning: BAD history->GetVolume(1)->GetCopyNo(), skipping the hit"
	   << G4endl;
    return true;
  }
  
  assert (theLayer>0);

  G4int depth = history->GetHistory()->GetDepth();
  G4int ModuleCopyNumber=0;
  for (G4int idepth = 0; idepth <= depth; idepth++) {
    ModuleCopyNumber=history->GetVolume(idepth)->GetCopyNo();
    if(ModuleCopyNumber>100) break;
  }

  //  delete history;
  
  assert (ModuleCopyNumber!=0);  

  G4int theSDPiece = ModuleCopyNumber/100;

  // The hit will deposited in the midle of the step
  G4ThreeVector thePosition = 
    (aStep->GetPreStepPoint()->GetPosition()+
     aStep->GetPostStepPoint()->GetPosition())*0.5;
  

#ifdef MOKKA_DEBUG
  G4int tmp = SDPiece;
  if(thePosition(2)>0) tmp+=(ECALENDCAPPLUS - ECALENDCAPMINUS);
  if (ModuleCopyNumber / 100 != tmp)
    Control::Abort("ECSD::ProcessHits: Assertion failed (ModuleCopyNumber / 100 != tmp)",MOKKA_OTHER_ERRORS);
#endif
  
  G4int theStave = (ModuleCopyNumber-theSDPiece*100)/10;
  G4int theModule = (ModuleCopyNumber-theSDPiece*100)%10;

  assert (StavesRotationMatrices[theStave-1]!=0);

  // The standard module reference is the Z>0 one.
  // If Z<0, rotate the position to the positive one.
  G4RotationMatrix rot1;
  if(thePosition(2)<0.)
    {
      rot1.rotateY(pi);
    }
  thePosition  = rot1 * thePosition;

  // find out local position in the standard module reference
  G4ThreeVector localPosition = 
    *StavesRotationMatrices[theStave-1] * thePosition;


  // calculates I,J
  G4int I,J;

  assert (Layers[theLayer-1]!=0); 
  
  I = static_cast <G4int> ((localPosition(0) - Layers[theLayer-1]->X0)/CellDim(0));

  assert (I>=0);

  J = static_cast <G4int> ((localPosition(1) - Layers[theLayer-1]->Y0)/CellDim(2));
  
  assert (J>=0);
  
  // find out the actual cell center coodinates in the reference module
  G4ThreeVector theCellCenter = 
  		GetCellCenter(theSDPiece, theStave, theModule, I, J, theLayer);
  
#ifdef MOKKA_DEBUG
  thePosition  = rot1 * thePosition;
  G4ThreeVector distCenter = thePosition - theCellCenter;
  if (distCenter.mag() > CellDim(0) + CellDim(1))
    {
      G4cout << "======= ASSERT WILL CRASH :\n"
	     << "\ntheSDPiece = " << theSDPiece
	     << ", theStave = " << theStave 
	     << ", theModule = " << theModule
	     << "\nI = " << I << ", J= " << J << ", K = " << theLayer
	     << "\nthePosition = " << thePosition
	     << ", localPosition = " << localPosition
//	     << "localCellCenter = " << localCellCenter
	     << ", theCellCenter = " << theCellCenter
	     << ", distCenter.mag() = " << distCenter.mag() << G4endl;
      Control::Abort("ECSD::ProcessHits: Assertion failed (distCenter.mag() > CellDim(0) + CellDim(1))",MOKKA_OTHER_ERRORS);
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
		   I,J,theLayer,0);
  
  G4bool found=false;
  G4int n_hit = CalCollection->entries();
  for(G4int i_hit = 0; i_hit< n_hit; i_hit++)
    if((*CalCollection)[i_hit]->
       testCell(theCode)) 
      {
	(*CalCollection)[i_hit]->AddEdep(PID,PDG,edep, time ); 
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
