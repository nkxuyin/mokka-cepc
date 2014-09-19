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
// $Id: SEcalSD02.cc,v 1.7 2009/06/04 11:40:42 musat Exp $
// $Name: mokka-07-00 $
//
#include "Control.hh"
#include "SEcalSD02.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4ParticleDefinition.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "globals.hh"
#include <assert.h>

#include "CGAGeometryManager.hh"
#include "G4VProcess.hh"

#include "G4ThreeVector.hh"

#include "Encoder64.hh"
#include "Encoder32.hh"

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
SEcalSD02::
SEcalSD02(G4double Idim, G4double Jdim, G4double Thickness,
	  G4int the_n_cells_i, G4int the_n_cells_j,
	  G4double theGuardRingSize, G4double theHWallSize,
	  G4double theTowerWallSize,
	  G4int Piece, G4String SDname, G4bool id1Flag,
	  G4String theBarrelSlabMode, G4String theECSlabMod) 
  : VSensitiveDetector(SDname), NormalCalCollection(0),
    FirstLayerCalCollection(0),
    CellDim (Idim,Thickness,Jdim),
    n_cells_i(the_n_cells_i), n_cells_j(the_n_cells_j),
    GuardRingSize(theGuardRingSize), HWallSize(theHWallSize),
    TowerWallSize(theTowerWallSize),
    SDPiece(Piece),HCID1(-1),HCID2(-1),
    BarrelSlabMode(theBarrelSlabMode),ECSlabMode(theECSlabMod)
{
  assert (CellDim(0)>0);
  assert (CellDim(1)>0);
  assert (CellDim(2)>0);

  StandardXOffset = 0;

  if(BarrelSlabMode != "0110") 
    Control::Abort("SEcalSD02: invalid BarrelSlabMode!",MOKKA_OTHER_ERRORS);

  if(ECSlabMode != "0110" && ECSlabMode != "0101" ) 
    Control::Abort("SEcalSD02: invalid ECSlabMode!",MOKKA_OTHER_ERRORS);

  G4String CollName1=SDname+"Collection";
  collectionName.insert(CollName1);

  G4String CollName2=SDname+"PreShowerCollection";
  collectionName.insert(CollName2);

  G4int i;
  for(i=0;i<MAX_STAVES;i++) 
    {
      StavesRotationMatrices[i]=0;
      InverseStavesRotationMatrices[i]=0;
      StavesPhirots[i]=0;
    }
  for(i=0;i<MAX_LAYERS;i++) Layers[i]=0;
  for(i=0;i<MAX_MODULES;i++) ModulesZOffsets[i]=0;

  if(id1Flag)
        theEncoder = new Encoder64();
  else
        theEncoder = new Encoder32();
  total_wafer_size_x = 
    n_cells_i * Idim +
    2 * theGuardRingSize;
  total_wafer_size_z = 
    n_cells_j * Jdim +
    2 * theGuardRingSize;
  total_tower_size_z =  2 *
    (total_wafer_size_z + TowerWallSize + HWallSize);
}

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
SEcalSD02::~SEcalSD02()
{
  G4int i;
  for(i=0;i<MAX_STAVES;i++) {
    if(StavesRotationMatrices[i]!=0) delete StavesRotationMatrices[i];
    if(InverseStavesRotationMatrices[i]!=0) delete InverseStavesRotationMatrices[i];
  }
  for(i=0;i<MAX_LAYERS;i++) 
    if(Layers[i]!=0) delete Layers[i];
  for(i=0;i<MAX_MODULES;i++) 
    if(ModulesZOffsets[i]!=0) delete ModulesZOffsets[i];
  for(i=0;i<MAX_STAVES;i++) 
    if(StavesPhirots[i]!=0) delete StavesPhirots [i];

  delete theEncoder;
}

void SEcalSD02::Initialize(G4HCofThisEvent *)
{
//if(CalCollection!=0) delete CalCollection;
  NormalCalCollection = new HitsCollection
    (SensitiveDetectorName,collectionName[0]); 
  FirstLayerCalCollection = new HitsCollection
    (SensitiveDetectorName,collectionName[1]); 
}

void SEcalSD02::SetModuleZOffset(G4int moduleNumber,G4double Zoff)
{
  assert (moduleNumber<MAX_MODULES && moduleNumber>=0);

  if(ModulesZOffsets[moduleNumber] != 0) 
    {
      delete ModulesZOffsets[moduleNumber];
      ModulesZOffsets[moduleNumber]=0;
    }
  ModulesZOffsets[moduleNumber]  = new G4double(Zoff);

}

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
void SEcalSD02::SetStaveRotationMatrix(G4int staveNumber, G4double phirot)
{

  assert (staveNumber<=MAX_STAVES && staveNumber>0);

  if(StavesRotationMatrices [staveNumber-1] !=0) 
    delete StavesRotationMatrices [staveNumber-1];

  if(InverseStavesRotationMatrices [staveNumber-1] !=0) 
    delete  InverseStavesRotationMatrices [staveNumber-1];

  if(StavesPhirots [staveNumber-1] !=0) 
    {
      delete  StavesPhirots [staveNumber-1];
      StavesPhirots [staveNumber-1] = 0;
    }

  StavesPhirots [staveNumber-1] = new G4double(phirot);

  StavesRotationMatrices [staveNumber-1] = new G4RotationMatrix();  
  StavesRotationMatrices [staveNumber-1]->rotateZ(-phirot);
  InverseStavesRotationMatrices [staveNumber-1] = 
    new G4RotationMatrix(StavesRotationMatrices [staveNumber-1]->inverse());
}

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
void SEcalSD02::AddLayer(G4int layerNumber, G4double X,G4double Y,G4double Z)
{
  assert (layerNumber<=MAX_LAYERS && layerNumber>0);
  assert (Layers[layerNumber-1] == 0);

  Layers[layerNumber-1] =
    new LayerRef(X,Y,Z);
}

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
void SEcalSD02::AddLayerOffset(G4int layerNumber, G4double X,G4double Y,G4double Z)
{
  assert (layerNumber<=MAX_LAYERS && layerNumber>0);
  assert (Layers[layerNumber-1] != 0);

  Layers[layerNumber-1]->X0 +=X;
  Layers[layerNumber-1]->Y0 +=Y;
  Layers[layerNumber-1]->Z0 +=Z;
}

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
G4ThreeVector SEcalSD02::GetCellCenter(G4int ,G4int pS,G4int pM,
				       G4int pI,G4int pJ,G4int pK) 
{
//  assert (pP >=ECALENDCAPMINUS);
//  assert (pP <= ECALENDCAPPLUS);
  assert (pS>0);
  assert (pM>=0);
  assert (pI>=0);
  assert (pJ>=0);
  assert (pK>0);
  assert(pS<=MAX_STAVES);
  assert(pM<MAX_MODULES);
  assert (pK<=MAX_LAYERS);

  G4ThreeVector localCellCenter;
  if((pM != 6) && (pM != 0))
    {
      // Barrel
      G4double X0 = Layers[pK-1]->X0;
      
      G4int N_wafers_before_x =
	pI / n_cells_i;
      // builds the cell center coodinates for I,J
      localCellCenter[0]= 
	X0 
	+ StandardXOffset
	+ N_wafers_before_x * total_wafer_size_x 
	+ GuardRingSize +
	(pI % n_cells_i) * CellDim(0) + 
	CellDim(0)/2.;
      
      
      localCellCenter[1]=Layers[pK-1]->Y0;
      
      G4int N_wafers_before_z =
	( pJ ) / n_cells_j;
      
      G4int N_cells_before_z =
	N_wafers_before_z * n_cells_j;
      
      G4int N_towers_before_z =
	static_cast 
	<G4int> (N_wafers_before_z / 2);
      
      localCellCenter[2]=
	Layers[pK-1]->Z0 
	+ N_wafers_before_z * total_wafer_size_z
	+ N_towers_before_z * 2 * ( TowerWallSize + HWallSize )
	+ GuardRingSize
	+ (pJ - N_cells_before_z ) * CellDim(2) 
	+ CellDim(2)/2.;
    }
  else
    {
      // EndCaps
      G4double X0 = Layers[pK-1]->X0;
      
      G4int N_wafers_before_x =
	pI / n_cells_i;

      G4int N_towers_before_x =
	static_cast 
	<G4int> (N_wafers_before_x / 2);

      // builds the cell center coodinates for I,J
      localCellCenter[0]= 
	X0 
	// + StandardXOffset
	+ N_wafers_before_x * total_wafer_size_x
	+ N_towers_before_x * 2 * ( TowerWallSize + HWallSize ) 
	+ GuardRingSize +
	(pI % n_cells_i) * CellDim(0) + 
	CellDim(0)/2.;
      
      localCellCenter[1]=Layers[pK-1]->Y0;
      
      G4int N_wafers_before_z =
	( pJ ) / n_cells_j;
      
      G4int N_cells_before_z =
	N_wafers_before_z * n_cells_j;
      
      localCellCenter[2]=
	Layers[pK-1]->Z0 
	+ N_wafers_before_z * total_wafer_size_z
	+ GuardRingSize
	+ (pJ - N_cells_before_z ) * CellDim(2) 
	+ CellDim(2)/2.;

      // X grows against +X in end caps
      localCellCenter.setX(-localCellCenter[0]);
      // Y <-> Z in endcaps
      G4double temp = localCellCenter[1];
      localCellCenter.setY(localCellCenter[2]);
      localCellCenter.setZ(temp);
    }
  
  // find out the actual cell center coodinates
  assert (InverseStavesRotationMatrices[pS-1]!=0);
  G4ThreeVector theCellCenter = 
    *InverseStavesRotationMatrices[pS-1] * localCellCenter;
  
  //if(pP == ECALENDCAPMINUS)
  if(pM == 0)
    {
      theCellCenter[2] += *ModulesZOffsets[6];
      G4RotationMatrix rot1;
      rot1.rotateY(pi);
      theCellCenter = rot1 * theCellCenter;
    }
  else
      theCellCenter[2] += *ModulesZOffsets[pM];
  

  return theCellCenter;
}

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
G4bool SEcalSD02::ProcessHits(G4Step *aStep,G4TouchableHistory *)
{
  // process only if energy>0. except geantinos
  G4double edep;
  if ((edep = aStep->GetTotalEnergyDeposit())<=0. &&
      aStep->GetTrack()->GetDefinition()->GetParticleType()!="geantino") 
    return true;
  G4double time = aStep->GetTrack()->GetGlobalTime() ;  

  // hit will deposited in the midle of the step
  G4ThreeVector thePosition = 
    (aStep->GetPreStepPoint()->GetPosition()+
     aStep->GetPostStepPoint()->GetPosition())*0.5;
  
  // Find out the stave and module id looking for the
  // module copy number and decoding it
  const G4VTouchable *history =aStep->GetPreStepPoint()->GetTouchable();
  G4int copyNumber;

  // The wafer indexes are in the copy number
  G4int n_wafer_x,n_wafer_z;
  copyNumber = history->GetVolume(0)->GetCopyNo();
  n_wafer_x = copyNumber / 1000;
  n_wafer_z = copyNumber % 1000;

  // Stave (if end cap), Tower and layer
  copyNumber = history->GetVolume(2)->GetCopyNo();
  G4int theStave = 0;
  G4int n_tower, theLayer;
  theStave = copyNumber / 100000;
  n_tower = (copyNumber - theStave*100000) /1000;
  theLayer = copyNumber % 1000;
  
  // if theStave > 0 => endcap!
  // SDPiece, Stave and module
  copyNumber = history->GetVolume(3)->GetCopyNo();
  G4int theSDPiece = 0;
  G4int theModule = 6; // Default endcap module
  if(theStave == 0)
    {
      theSDPiece = copyNumber/100;
      theStave = (copyNumber - theSDPiece*100)/10;
      theModule = (copyNumber - theSDPiece*100)%10;
    }
  else
    {
      theSDPiece =  copyNumber;
    }

  assert (StavesPhirots[theStave-1]!=0);
  assert (StavesRotationMatrices[theStave-1]!=0);


  // find out local position in the standard module reference
  G4ThreeVector localPosition;

  if(theSDPiece == ECALBARREL)
    {
      // barrel
      localPosition = 
	*StavesRotationMatrices[theStave-1] * thePosition;
      localPosition [0] -= StandardXOffset;
    }
  else
    {
      // endcaps
      // For the endcaps, the end cap reference is the Z>0 one.
      // So if Z<0, rotate the position to the positive one.
      G4ThreeVector tmpPosition = thePosition;
      if(tmpPosition(2)<0.)
	{
	  G4RotationMatrix rot1;
	  rot1.rotateY(pi);
	  tmpPosition  = rot1 * tmpPosition;
	}
      // find out local position in the standard module reference
      localPosition = 
	*StavesRotationMatrices[theStave-1] * tmpPosition;
      
    }


  // calculates I,J
  G4int I=0,J=0;

  assert (theLayer>0);

  assert (Layers[theLayer-1]!=0);
  assert (ModulesZOffsets[theModule]!=0);

  //**************************
  //prints for debug 
//       G4cout << "\ntheSDPiece = " << theSDPiece
// 	     << ", theStave = " << theStave 
// 	     << ", theModule = " << theModule
// 	     << "\nthePosition = " << thePosition
// 	     << ", localPosition = " << localPosition
// 	     << G4endl;
//   G4cout << "theLayer = "
// 	 << theLayer
// 	 <<", Layers[theLayer-1]->X0 = "
// 	 << Layers[theLayer-1]->X0
// 	 << ", Layers[theLayer-1]->Y0 = "
// 	 << Layers[theLayer-1]->Y0
// 	 << ", Layers[theLayer-1]->Z0 = "
// 	 << Layers[theLayer-1]->Z0
// 	 << G4endl;
  //**************************

  if(theSDPiece==ECALBARREL)
    {
      // BARREL
      // I = X direction
      G4double x_starting_wafer =
	( n_wafer_x - 1 ) * total_wafer_size_x
	+ GuardRingSize;
      
      G4double x_in_wafer =
	localPosition(0) - 
	Layers[theLayer-1]->X0
	- x_starting_wafer;
      
      I = static_cast 
	<G4int> ( x_in_wafer/
		  CellDim(0)) +
	(n_wafer_x - 1) * n_cells_i;
   
      // displacement from the Z0 offset
      G4double z_size_before_tower =
	(n_tower -1) * total_tower_size_z;
      // The Z0 offset starts at alveolus boundary
      G4double z_starting_wafer =
	z_size_before_tower + 
	GuardRingSize;
      // depending on the layer number the Si is inversed,
      // so fixes n_wafer_z depending on the layer number
      if ( theLayer % 2 == 0 )
	n_wafer_z = (n_wafer_z==1)?2:1;
      
      // wafer z offset
      z_starting_wafer +=
	(n_wafer_z-1) *
	total_wafer_size_z;
      
      // z in local wafer z offset
      G4double z_in_wafer =
	localPosition(2) - 
	*ModulesZOffsets[theModule] - 
	Layers[theLayer-1]->Z0 - 
	z_starting_wafer;
      
      J = static_cast 
	<G4int> (z_in_wafer/
		 CellDim(2)) + 
	(n_tower -1) * 2* n_cells_j +
	(n_wafer_z - 1) * n_cells_j;
    }
  else
    {
      // END CAPS
      // After turning the slabs in the end caps,it become really
      // confuse. What we call below "z" is in fact "x", and what
      // we call below "z" is in fact "y". We keep it as it's because
      // we reuse the old code already tested, which is already
      // an adaptation from the barrel code. So, to complicate a bit
      // more things, Y<->Z in end caps. But it works fine...
 
      if(theSDPiece==ECALENDCAPMINUS)
		theModule = 0;

      // displacement from the X0 offset
      G4double z_size_before_tower =
	(n_tower -1) * total_tower_size_z;
      // The X0 offset starts at alveolus boundary
      G4double z_starting_wafer =
	z_size_before_tower + 
	GuardRingSize;

      // depending on the layer number the Si is inversed,
      // so fixes n_wafer_z depending on the layer number
      // (not more true after turning the slabs)
      if ( ECSlabMode == "0110" && theLayer % 2 == 0 )
	n_wafer_z = (n_wafer_z==1)?2:1;
      
      // wafer x offset
      z_starting_wafer +=
	(n_wafer_z-1) *
	total_wafer_size_z;
      
      // x in local wafer x offset
      G4double z_in_wafer =
	- localPosition(0) - 
	Layers[theLayer-1]->X0 - 
	z_starting_wafer;
      
      // I !
      I = static_cast 
	<G4int> (z_in_wafer/
		 CellDim(2)) + 
	(n_tower -1) * 2* n_cells_j +
	(n_wafer_z - 1) * n_cells_j;

      // J
      // below, "x_" means in fact "y_"
      G4double x_starting_wafer =
	( n_wafer_x - 1 ) * total_wafer_size_x
	+ GuardRingSize;
      
      G4double x_in_wafer =
	localPosition(1) - 
	Layers[theLayer-1]->Z0
	- x_starting_wafer;
      
      J = static_cast 
	<G4int> ( x_in_wafer/
		  CellDim(0)) +
	(n_wafer_x - 1) * n_cells_i;

    }


  //  if(theSDPiece==ECALBARREL || theSDPiece==HCALBARREL)
  assert (theSDPiece==ECALENDCAPMINUS || 
	  theSDPiece==ECALENDCAPPLUS ||
	  theSDPiece==ECALBARREL);
  
  assert (I>=0);
  assert (J>=0);
  //  G4cout << "I= " << I << ", J = " << J << G4endl;
  
  // find out the actual cell center coodinates
  G4ThreeVector theCellCenter = 
    GetCellCenter(theSDPiece, theStave, theModule, I, J, theLayer);
  
#ifdef MOKKA_DEBUG
  // test if the cell center is not so far way the current
  // step point.
  G4ThreeVector distCenter = thePosition - theCellCenter;
  if (distCenter.mag() > CellDim.mag()*sqrt(2.))
    {
      G4cout << "======= ASSERT WILL CRASH :\n"
	     << "\ntheSDPiece = " << theSDPiece
	     << ", theStave = " << theStave 
	     << ", theModule = " << theModule
	     << "\nI = " << I << ", J= " << J << ", K = " << theLayer
	     << "\nthePosition = " << thePosition
	     << ", localPosition = " << localPosition
	     << ", \ntheCellCenter = " << theCellCenter
	     << ", distCenter = " << distCenter 
	     << ", distCenter.mag() = "
	     << distCenter.mag() 
	     << ", CellDim.mag() *sqrt(2.0)= "
	     << CellDim.mag()*sqrt(2.0)
	     << G4endl;
      Control::Abort("SEcalSD02::ProcessHits: Assertion failed (distCenter.mag() > CellDim.mag())",MOKKA_OTHER_ERRORS);
    }
#endif

  // creates a new cell or add< the energy to the cell if it already exists
  
  G4int PID = 
    Control::GetControl()->GetPIDForCalHit(aStep);
  assert(PID!=-1);

  G4int PDG=
    aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
  
  G4bool found=false;
  HitsCollection *CalCollection;
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

  G4int n_hit = CalCollection->entries();

  cell_ids theCode = 
    theEncoder->encode(theStave,theModule,
		   I,J,theLayer,0);
  
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

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
void SEcalSD02::EndOfEvent(G4HCofThisEvent*HCE)
{
  if(HCID1 < 0) HCID1 = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  HCE->AddHitsCollection( HCID1, NormalCalCollection );
  if(HCID2 <0) HCID2 = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[1]);
  HCE->AddHitsCollection( HCID2, FirstLayerCalCollection );
}

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
void SEcalSD02::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
  CalHit* newHit = new CalHit();
  while (newHit->Load(theSubDetectorEventHitsFileInput))
    {
      NormalCalCollection->insert(newHit);
      newHit = new CalHit();
    }
  delete newHit;
}

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
void SEcalSD02::clear()
{
} 

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
void SEcalSD02::DrawAll()
{
} 

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
void SEcalSD02::PrintAll()
{
} 




