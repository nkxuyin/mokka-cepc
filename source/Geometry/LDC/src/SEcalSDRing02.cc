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
// $Id: SEcalSDRing02.cc,v 1.4 2009/04/28 12:53:19 musat Exp $
// $Name: mokka-07-00 $
//
// 
#include "Control.hh"
#include "SEcalSDRing02.hh"
#include "G4StepPoint.hh"
#include "G4TransportationManager.hh"
#include "G4ParticleDefinition.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "globals.hh"
#include "G4UImanager.hh"
#include <assert.h>

SEcalSDRing02::
SEcalSDRing02(G4double Idim,G4double Jdim,G4double Thickness,
	      G4int Piece,G4String SDname, G4bool id1Flag,G4bool preShower) 
  : SEcalSD02(Idim,Jdim,Thickness,10000,10000,0.,0.,0.,Piece,SDname,id1Flag)
{
  WithPreShower = preShower;
}

SEcalSDRing02::~SEcalSDRing02() 
{
}

G4ThreeVector SEcalSDRing02::GetCellCenter(G4int ,G4int ,G4int pM,
				G4int pI,G4int pJ,G4int pK) {
  assert (pM>=0);
  assert (pI>=0);
  assert (pJ>=0);
  assert (pK>0);
  assert(pM<MAX_MODULES);
  assert (pK<=MAX_LAYERS);

  // builds the cell center coodinates for the I,J
  G4ThreeVector localCellCenter;
  localCellCenter[0]=Layers[pK-1]->X0 + pI*CellDim(0) + CellDim(0)/2.;
  localCellCenter[1]=Layers[pK-1]->Y0 + pJ*CellDim(2) + CellDim(2)/2.;
  localCellCenter[2]=Layers[pK-1]->Z0;

  // find out the actual cell center coodinates in the reference module
  G4ThreeVector theCellCenter = localCellCenter;
  
  G4RotationMatrix rot1;
  if(pM == 0)
  {
     assert (ModulesZOffsets[6]!=0);
     theCellCenter[2] += *ModulesZOffsets[6];
 
     // The standard module reference is the Z>0 one.
     // If Z<0, rotate the position to the positive one.
     rot1.rotateY(pi);
  }
  else
  {
     assert (ModulesZOffsets[pM]!=0);
     theCellCenter[2] += *ModulesZOffsets[pM];
  }

  // If Z<0 rot1 keeps the good rotation for the Cell Center.
  theCellCenter  = rot1 * theCellCenter;

  return theCellCenter;

}

G4bool SEcalSDRing02::ProcessHits(G4Step *aStep,G4TouchableHistory*)
{

  // process only if energy>0.
  G4double edep;
  if ((edep = aStep->GetTotalEnergyDeposit())<=0. &&
      aStep->GetTrack()->GetDefinition()->GetParticleType()!="geantino")
    return true;
  
  G4double time = aStep->GetTrack()->GetGlobalTime() ;
  
  // Find the layer id in copy number
  const G4VTouchable *history =aStep->GetPreStepPoint()->GetTouchable();

  G4int theLayer = history->GetVolume(0)->GetCopyNo();

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

  // The hit will deposited in the midle of the step
  G4ThreeVector thePosition = 
    (aStep->GetPreStepPoint()->GetPosition()+
     aStep->GetPostStepPoint()->GetPosition())*0.5;

  G4int theStave = 1;
  G4int theModule = 6;

  // The standard module reference is the Z>0 one.
  // If Z<0, rotate the position to the positive one.
  G4RotationMatrix rot1;
  G4int theSDPiece = 0;
  if(thePosition(2)<0.)
    {
      rot1.rotateY(pi);
      theSDPiece = ECALENDCAPMINUS;
      theModule = 0;
    }
  else
    {
      theSDPiece = ECALENDCAPPLUS;
    }
  thePosition  = rot1 * thePosition;

  // find out local position in the standard module reference
  G4ThreeVector localPosition = thePosition;

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
  
//        G4cout << "======= Dumping info :\n"
// 	     << "\ntheSDPiece = " << theSDPiece
// 	     << ", theStave = " << theStave 
// 	     << ", theModule = " << theModule
// 	     << "\nI = " << I << ", J= " << J << ", K = " << theLayer
// 	     << "\nthePosition = " << thePosition
// 	     << ", localPosition = " << localPosition
// 	     << ", theCellCenter = " << theCellCenter
// 	      << G4endl;
  
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
      Control::Abort("SEcalSDRing02::ProcessHits: Assertion failed (distCenter.mag() > CellDim(0) + CellDim(1))",MOKKA_OTHER_ERRORS);
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
  HitsCollection *CalCollection;
  if(WithPreShower)
    {
      if (theLayer > 1)
	{
	  CalCollection = NormalCalCollection;
	  theLayer --;
	}
      else
	{
	  CalCollection = FirstLayerCalCollection;
	  theSDPiece = -theSDPiece;
	}
    }
  else CalCollection  = NormalCalCollection;

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
