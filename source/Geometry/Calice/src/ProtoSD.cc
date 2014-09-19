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
// $Id: ProtoSD.cc,v 1.6 2006/03/01 14:13:30 musat Exp $
// $Name: mokka-07-00 $
//
#include "Control.hh"
#include "ProtoSD.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4TransportationManager.hh"
#include "G4ParticleDefinition.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "globals.hh"
#include "G4UImanager.hh"
#include <assert.h>

#include "G4AffineTransform.hh"

#include "Encoder64.hh"
#include "Encoder32.hh"

ProtoSD::ProtoSD(G4double dimX,G4double dimY,G4double dimZ,
		 G4int n_cell_x, G4int n_cell_z,
		 G4int aMultiplicity,G4int aNumberOfElements,
		 G4int aNumberOfTowers,G4String ProtoSDname) 
  : VSensitiveDetector(ProtoSDname), CalCollection(0),
    HCID(-1),CellDim (dimX,dimY,dimZ),theNumberOfTowers(aNumberOfTowers),
    theNCell_x(n_cell_x), theNCell_z(n_cell_z),
    theMultiplicity(aMultiplicity),
    theNumberOfElements(aNumberOfElements)
    
{
  assert (CellDim(0)>0);
  assert (CellDim(1)>0);
  assert (CellDim(2)>0);

  G4String CollName=ProtoSDname+"Collection";
  collectionName.insert(CollName);

  if((CellDim(0) < 10*mm) || (CellDim(2) < 10*mm))
        theEncoder = new Encoder64();
  else
        theEncoder = new Encoder32();

}

ProtoSD::~ProtoSD()
{
}

void ProtoSD::Initialize(G4HCofThisEvent *)
{
  //if(CalCollection!=0) delete CalCollection;
  CalCollection = new HitsCollection
    (SensitiveDetectorName,collectionName[0]); 
}

G4bool ProtoSD::ProcessHits(G4Step *aStep,G4TouchableHistory *)
{

  // process only if energy>0. except geantinos
  G4double edep;
  if ((edep = aStep->GetTotalEnergyDeposit())<=0. &&
      aStep->GetTrack()->GetDefinition()->GetParticleType()!="geantino") 
    return true;

  // hit will deposited in the midle of the step
  G4ThreeVector thePosition = 
    (aStep->GetPreStepPoint()->GetPosition()+
     aStep->GetPostStepPoint()->GetPosition())*0.5;
  
  const G4VTouchable *history =aStep->GetPreStepPoint()->GetTouchable();

  G4AffineTransform theAffineTransformation, theInverseAffineTransformation;

  theAffineTransformation = 
    history->GetHistory()->
    GetTransform(history->GetHistory()->GetDepth());
  theInverseAffineTransformation = theAffineTransformation.Inverse();

  G4ThreeVector theLocalPosition;
  theLocalPosition = theAffineTransformation.
    TransformPoint(thePosition);

  G4int I = static_cast <G4int> ((theLocalPosition(0)+
	     (CellDim(0)*theNCell_x/2))/CellDim(0));
  G4int J = static_cast <G4int> ((theLocalPosition(2)+
	     (CellDim(2)*theNCell_z/2))/CellDim(2));

  G4ThreeVector theLocalCellCenter(double(I*CellDim(0)
				   -(CellDim(0)*theNCell_x/2)+
				   CellDim(0)/2.),
				   0.,  
				   double(J*CellDim(2)
				   -(CellDim(2)*theNCell_z/2)+
				   CellDim(2)/2.));

  G4ThreeVector theCellCenter = 
    theInverseAffineTransformation.
    TransformPoint(theLocalCellCenter);

  assert (theCellCenter(0)-thePosition(0)<=CellDim(0));
  assert (theCellCenter(1)-thePosition(1)<=CellDim(1));
  assert (theCellCenter(2)-thePosition(2)<=CellDim(2));

  
//   G4cout << "thePosition = " << thePosition
// 	 << ", theLocalPosition= " << theLocalPosition 
// 	 << " ==> I = " << I
// 	 << ", J = " << J 
// 	 << "\n, theLocalCellCenter = " << theLocalCellCenter 
// 	 << ", theCellCenter = " << theCellCenter << G4endl;


//   G4int depth = history->GetHistory()->GetDepth();
//   for (G4int idepth = 0; idepth <= depth; idepth++) 
//     {
//       G4cout << "\nlevel " << idepth << ", volume "
// 	     << history->GetVolume(idepth)->GetName()
// 	     << "copy number = " << history->GetVolume(idepth)->GetCopyNo() 
// 	     << G4endl;
//     }

  G4int LayersCopyNumber=history->GetVolume(2)->GetCopyNo();
  G4int Tower = LayersCopyNumber/100;
  G4int Plate = LayersCopyNumber % 100;

  G4int WafferCopyNumber=history->GetVolume(1)->GetCopyNo();
  G4int MJ = abs(WafferCopyNumber) % 10;     // position J de MxM (M = multiplicite)
  G4int MI = (abs(WafferCopyNumber)/10)%10;  // position I de MxM
  G4int GI = (abs(WafferCopyNumber)/100)%10; // position sur le support G10 (0 ou 1)
  G4int EI = (abs(WafferCopyNumber)/1000)%10; // numero de l'element sur le slab

  G4int WI = MI + GI*theMultiplicity + EI * 2 * theMultiplicity;
  G4int WJ = MJ + Tower * theMultiplicity; 

  G4int Layer = WafferCopyNumber>0 ? Plate : Plate-1;

//   G4cout << "Tower = " << Tower 
// 	 << ", Plate = " << Plate
// 	 << ", MI = " << MI
// 	 << ", MJ = " << MJ
// 	 << ", GI = " << GI
// 	 << ", EI = " << EI
// 	 << ", Tower = " << Tower;

//   G4cout << "WI = " << WI
// 	 << ", WJ = " << WJ
// 	 << ", Layer = " << Layer
// 	 << G4endl;


//   // creates a new cell or add the energy to the cell if it already exists

//   assert(Control::primaryId!=-1);

  G4int PDG=
    aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
  G4double time = aStep->GetTrack()->GetGlobalTime() ;

  G4int n_hit = CalCollection->entries();

  cell_ids theCode = 
    theEncoder->encode(WI,WJ,
		   I,J,Layer,0);

  G4bool found=false;
  for(G4int i_hit = 0; i_hit< n_hit; i_hit++)
    if((*CalCollection)[i_hit]->
       testCell(theCode)) {
      (*CalCollection)[i_hit]->
	AddEdep( Control::primaryId, PDG,edep, time/ns );
      found = true;
      break;
    }
  
  if(!found) CalCollection->
	       insert(new CalHit (PROTOMODULE,
				   WI,
				   WJ,
				   I,
				   J,
				   Layer,
				   0,
				   theCellCenter (0),
				   theCellCenter (1),
				   theCellCenter (2),
				   edep,
				   Control::primaryId,
				   PDG,
				   time/ns,
				   theCode));
  return true;
}

void ProtoSD::EndOfEvent(G4HCofThisEvent*HCE)
{
  if(HCID<0)
    { HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]); }
  HCE->AddHitsCollection( HCID, CalCollection );
}

void ProtoSD::clear()
{
} 

void ProtoSD::DrawAll()
{
} 

void ProtoSD::PrintAll()
{
} 

void ProtoSD::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
  CalHit* newHit = new CalHit();
  while (newHit->Load(theSubDetectorEventHitsFileInput))
    {
      CalCollection->insert(newHit);
      newHit = new CalHit();
    }
  delete newHit;
}



