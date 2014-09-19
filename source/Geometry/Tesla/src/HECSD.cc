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
// $Id: HECSD.cc,v 1.10 2006/03/01 14:13:31 musat Exp $
// $Name: mokka-07-00 $
//
#include "Control.hh"
#include "HECSD.hh"
#include "G4StepPoint.hh"
#include "G4TransportationManager.hh"
#include "G4ParticleDefinition.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "globals.hh"
#include "G4UImanager.hh"
#include <assert.h>

HECSD::HECSD(G4double Idim,G4double Jdim,G4double Thickness,G4int Piece,G4String SDname) 
  : SD(Idim,Jdim,Thickness,Piece,SDname)
{
  // enregistre 4 staves bidons
  SetStaveRotationMatrix(1, pi/2.);
  SetStaveRotationMatrix(2, pi);
  SetStaveRotationMatrix(3, 3*pi/2.);
  SetStaveRotationMatrix(4, 0.);
}

HECSD::~HECSD() 
{
}

G4ThreeVector HECSD::GetCellCenter(G4int,G4int pS,G4int pM,
				G4int pI,G4int pJ,G4int pK) {

  assert (pS>0);
  assert (pM>=0);
  assert (pI>=0);
  assert (pJ>=0);
  assert (pK>0);
  assert(pS<=MAX_STAVES);
  assert(pM<MAX_MODULES);
  assert (pK<=MAX_LAYERS);

  G4ThreeVector localCellCenter;

  // builds the cell center coodinates for I,J
  localCellCenter[0]=pI*CellDim(0) + CellDim(0)/2.;
  localCellCenter[1]=pJ*CellDim(2) + CellDim(2)/2.;
  //localCellCenter[2]=Layers[theLayer-1]->Z0 + CellDim(1)/2;
  localCellCenter[2]=Layers[pK-1]->Z0;

  assert (InverseStavesRotationMatrices[pS-1]!=0);
  
  // find out the actual cell center coodinates in the reference module
  G4ThreeVector theCellCenter = 
    *InverseStavesRotationMatrices[pS-1] * localCellCenter;
  
  assert (ModulesZOffsets[pM]!=0);

  theCellCenter[2] += *ModulesZOffsets[pM];
  theCellCenter[2] = -1.0 * theCellCenter(2);

  // The standard module reference is the Z>0 one.
  // If Z<0, rotate the position to the positive one.
  G4RotationMatrix rot1;
//  if(pP == HCALENDCAPPLUS)
  if(pM == 6)
  {
        rot1.rotateY(pi);
  }
  // If Z<0 rot1 keeps the good rotation for the Cell Center.
  theCellCenter  = rot1 * theCellCenter;

  return theCellCenter;
}

G4bool HECSD::ProcessHits(G4Step *aStep,G4TouchableHistory *)
{

  // CENTERRADIUS should be read from the database!!!
  const double CENTERRADIUS = 300;

  // The hit will deposited in the midle of the step
  G4ThreeVector thePosition = 
    (aStep->GetPreStepPoint()->GetPosition()+
     aStep->GetPostStepPoint()->GetPosition())*0.5;

  // be sure the step is outside de central box
  if(fabs(thePosition(0))<CENTERRADIUS &&
     fabs(thePosition(1))<CENTERRADIUS) return true;

  // process only if energy>0 if not geantinos
  G4double edep;
  if ((edep = aStep->GetTotalEnergyDeposit())<=0. &&
      aStep->GetTrack()->GetDefinition()->GetParticleType()!="geantino")
    return true;

  G4double time = aStep->GetTrack()->GetGlobalTime() ;
  
  // Find out the stave and module id looking for the
  // module copy number and decoding it

  const G4VTouchable *history =aStep->GetPreStepPoint()->GetTouchable();

  //G4int theLayer = history->GetVolume(0)->GetCopyNo();

  G4int depth = history->GetHistory()->GetDepth();
  G4int theLayer = 0;
  G4int idepth;
  for (idepth = 0; idepth <= depth; idepth++) {
    theLayer=history->GetVolume(idepth)->GetCopyNo();
    if(theLayer>0) break;
  }
  
  if( theLayer<=0 || theLayer > MAX_LAYERS ) {
    G4cout << "theLayer = " << theLayer ;
    G4cout << ", history->GetHistory()->GetDepth()= "
	   << history->GetHistory()->GetDepth()
	   << G4endl;
    for (G4int ii = 0; ii <= history->GetHistory()->GetDepth(); ii++) 
      G4cout << "volname(" << ii << ")= "
	     << history->GetVolume(ii)->GetName()
	     << G4endl;
    G4cout << "Mokka HECSD: BAD history->GetVolume(0)->GetCopyNo(), FATAL ERROR"
	   << G4endl;
    exit(1);
    return true;
  }
  

  //G4int ModuleCopyNumber;
  //ModuleCopyNumber=history->GetVolume(1)->GetCopyNo();

  depth = history->GetHistory()->GetDepth();
  G4int ModuleCopyNumber=0;

  for (idepth = 0; idepth <= depth; idepth++) {
    ModuleCopyNumber=history->GetVolume(idepth)->GetCopyNo();
    if(ModuleCopyNumber>100) break;
  }
  
  assert (ModuleCopyNumber!=0);  

  G4int theSDPiece = ModuleCopyNumber/100;

#ifdef MOKKA_DEBUG
  G4int tmp = SDPiece;
  if (thePosition(2) > 0) tmp += (HCALENDCAPPLUS - HCALENDCAPMINUS);
  if (theSDPiece != tmp)
    Control::Abort("HECSD::ProcessHits: Assertion failed (theSDPiece != tmp)",MOKKA_OTHER_ERRORS);
#endif

  G4int theModule = (ModuleCopyNumber-theSDPiece*100)%10;  

  // The standard module reference is the Z>0 one.
  // If Z<0, rotate the position to the positive one.
  G4RotationMatrix rot1;
  if(thePosition(2)>0.)
    {
      rot1.rotateY(pi);
    }
  thePosition  = rot1 * thePosition;

  //  theStave est bidon!
  G4int theStave=0;
  if(thePosition(0)<0 && thePosition(1)>=0) theStave=1;
  if(thePosition(0)<0 && thePosition(1)<0) theStave=2;
  if(thePosition(0)>=0 && thePosition(1)<0) theStave=3;
  if(thePosition(0)>=0 && thePosition(1)>=0) theStave=4;

  // find out local position in the standard module reference
  assert (StavesRotationMatrices[theStave-1]!=0);
  G4ThreeVector localPosition = 
    *StavesRotationMatrices[theStave-1] * thePosition;

  // calculates I,J
  G4int I,J;

  I= static_cast <G4int> (localPosition(0)/CellDim(0));
  J= static_cast <G4int> (localPosition(1)/CellDim(2));

  G4ThreeVector theCellCenter = 
    		GetCellCenter(theSDPiece, theStave, theModule, I, J, theLayer);
  
//   G4cout << "theCellCenter[2] = " << theCellCenter[2] 
// 	 << " thePosition = " << thePosition << G4endl;
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
	     << "\nthePosition= " << thePosition
	     << ", localPosition = " << localPosition
	     << ", theCellCenter = " << theCellCenter
	     << ", distCenter.mag() = " << distCenter.mag() << G4endl;
      Control::Abort("HECSD::ProcessHits: Assertion failed (distCenter.mag() > CellDim(0) + CellDim(1))",MOKKA_OTHER_ERRORS);
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
