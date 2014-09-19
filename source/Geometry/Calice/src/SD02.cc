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
// $Id: SD02.cc,v 1.5 2006/03/01 14:13:30 musat Exp $
// $Name: mokka-07-00 $
//
#include "Control.hh"
#include "SD02.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4TransportationManager.hh"
#include "G4ParticleDefinition.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "globals.hh"
#include "G4UImanager.hh"
#include <assert.h>

#include "G4Material.hh"
#include "CGAGeometryManager.hh"
#include "G4VProcess.hh"

#include "G4ThreeVector.hh"

#include "Encoder32.hh"

SD02::SD02(G4double Idim,G4double Jdim,G4double radius, G4double Thickness,G4int Piece,G4String SD02name, G4bool divideFlag) 
  : VSensitiveDetector(SD02name), CalCollection(0),
    CellDim (Idim,Thickness,Jdim),theRadius(radius),SDPiece(Piece),HCID(-1),
    divide(divideFlag)
{
  assert (CellDim(0)>0);
  assert (CellDim(1)>0);
  assert (CellDim(2)>0);

  G4String CollName=SD02name+"Collection";
  collectionName.insert(CollName);

  G4int i;
  for(i=0;i<MAX_STAVES;i++) 
    {
      StavesRotationMatrices[i]=0;
      InverseStavesRotationMatrices[i]=0;
      StavesPhirots[i]=0;
    }
  for(i=0;i<MAX_LAYERS;i++) Layers[i]=0;
  for(i=0;i<MAX_MODULES;i++) ModulesZOffsets[i]=0;
  ModuleRotationMatrix = 0;
  InverseModuleRotationMatrix = 0;

  theEncoder = new Encoder32();
}

SD02::~SD02()
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
  if(ModuleRotationMatrix != 0) delete ModuleRotationMatrix;
  if(InverseModuleRotationMatrix != 0) delete InverseModuleRotationMatrix;
}

void SD02::Initialize(G4HCofThisEvent *)
{
//if(CalCollection!=0) delete CalCollection;
  CalCollection = new HitsCollection
    (SensitiveDetectorName,collectionName[0]); 
}

void SD02::SetModuleZOffset(G4int moduleNumber,G4double Zoff)
{
  assert (moduleNumber<MAX_MODULES && moduleNumber>=0);

  if(ModulesZOffsets[moduleNumber] != 0) 
    {
      delete ModulesZOffsets[moduleNumber];
      ModulesZOffsets[moduleNumber]=0;
    }
  ModulesZOffsets[moduleNumber]  = new G4double(Zoff);
}

void SD02::SetModuleRotationMatrix(G4double thetarot)
{
  if(ModuleRotationMatrix != 0)
	  delete ModuleRotationMatrix;
  if(InverseModuleRotationMatrix != 0)
	  delete InverseModuleRotationMatrix;
  ModuleRotationMatrix = new G4RotationMatrix();
  ModuleRotationMatrix->rotateX(-thetarot);
  InverseModuleRotationMatrix = 
    new G4RotationMatrix(ModuleRotationMatrix->inverse());
}
  
void SD02::SetStaveRotationMatrix(G4int staveNumber, G4double phirot)
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

void SD02::AddLayer(G4int layerNumber, G4double X,G4double Y,G4double Z)
{
  assert (layerNumber<=MAX_LAYERS && layerNumber>0);
  assert (Layers[layerNumber-1] == 0);

  Layers[layerNumber-1] =
    new SD02LayerRef(X,Y,Z);
}

void SD02::AddLayerOffset(G4int layerNumber, G4double X,G4double Y,G4double Z)
{
  assert (layerNumber<=MAX_LAYERS && layerNumber>0);
  assert (Layers[layerNumber-1] != 0);

  Layers[layerNumber-1]->X0 +=X;
  Layers[layerNumber-1]->Y0 +=Y;
  Layers[layerNumber-1]->Z0 +=Z;
}

G4ThreeVector SD02::GetCellCenter(G4int,G4int pS,G4int pM,
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
//  if(pP==ECALBARREL || pP==HCALBARREL) { // barrels
  if(pM >=1 && pM <=5) { //barrels
    
    // builds the cell center coodinates for I,J
    localCellCenter[0]=Layers[pK-1]->X0 + pI*CellDim(0) +CellDim(0)/2.;
    localCellCenter[1]=Layers[pK-1]->Y0;
    localCellCenter[2]=Layers[pK-1]->Z0 + pJ*CellDim(2) + CellDim(2)/2.;
  }
  else {  // endcaps
    
    // builds the cell center coodinates for the calculated I,J
    localCellCenter[0]=Layers[pK-1]->X0 + pI*CellDim(0) + CellDim(0)/2.;
    localCellCenter[1]=Layers[pK-1]->Y0 + pJ*CellDim(2) + CellDim(2)/2.;
    localCellCenter[2]=Layers[pK-1]->Z0;
    // -z endcaps are inverted!
    assert(ModulesZOffsets[pM] != 0);
    if(*ModulesZOffsets[pM]<0) localCellCenter[2]=-localCellCenter[2];
  }

  assert (InverseStavesRotationMatrices[pS-1]!=0);
  
  // find out the actual cell center coodinates
  G4ThreeVector theCellCenter = 
    *InverseStavesRotationMatrices[pS-1] * localCellCenter;
  
  theCellCenter = (*InverseModuleRotationMatrix) * theCellCenter;
  theCellCenter[2] += *ModulesZOffsets[pM];

  return theCellCenter;
}

G4bool SD02::ProcessHits(G4Step *aStep,G4TouchableHistory *)
{
  // process only if energy>0. except geantinos
  G4double edep;
  if ((edep = aStep->GetTotalEnergyDeposit())<=0. &&
      aStep->GetTrack()->GetDefinition()->GetParticleType()!="geantino") 
    return true;

  // the layer number is the volume copy number
  G4int theLayer = aStep->GetTrack()->GetVolume()->GetCopyNo();
  assert (theLayer>0);

  // hit will deposited in the midle of the step
  G4ThreeVector thePosition = 
    (aStep->GetPreStepPoint()->GetPosition()+
     aStep->GetPostStepPoint()->GetPosition())*0.5;
  
  // Find out the stave and module id looking for the
  // module copy number and decoding it
  const G4VTouchable *history =aStep->GetPreStepPoint()->GetTouchable();

  G4int depth = history->GetHistory()->GetDepth();
  G4int ModuleCopyNumber = -1;
  for (G4int idepth = 0; idepth <= depth; idepth++) {
    ModuleCopyNumber=history->GetVolume(idepth)->GetCopyNo();
    if (theLayer==ENDCAP_SD_PLATE_FLAG && idepth==1) theLayer=ModuleCopyNumber;
    if(ModuleCopyNumber>100) break;
  }

#ifdef MOKKA_DEBUG
  if (ModuleCopyNumber == 0)
    {
      G4cout << "MOKKA WARNING: ModuleCopyNumber==0 in SD02::ProcessHits, "
	     << "at " << thePosition << ", SDPiece = " << SDPiece << G4endl;

      G4cout << "Volume name in aStep->GetPostStepPoint()GetPhysicalVolume() is "
	     << aStep->GetPostStepPoint()->GetPhysicalVolume()->GetName() << G4endl;

      G4cout << "Volume hierarchy in G4TouchableHistory :\n";
      for (G4int idepth = 0; idepth <= depth; idepth++) {
	G4cout << "level " << idepth << ", volume "
	       << history->GetVolume(idepth)->GetName()
	       << "copy number = " << history->GetVolume(idepth)->GetCopyNo() 
	       << G4endl;
      }
      Control::Abort("SD02::ProcessHits: Assertion failed (ModuleCopyNumber == 0)",MOKKA_OTHER_ERRORS);
    }
#endif
  
  G4int theSDPiece = ModuleCopyNumber/100;

#ifdef MOKKA_DEBUG
  G4int tmp = SDPiece;
  if(tmp==ECALENDCAPMINUS && thePosition(2)>0) tmp+=(ECALENDCAPPLUS-ECALENDCAPMINUS);
  else if(tmp==HCALENDCAPMINUS && thePosition(2)>0) tmp+=(HCALENDCAPPLUS-HCALENDCAPMINUS);
  if (ModuleCopyNumber/100 != tmp)
    Control::Abort("SD02::ProcessHits: Assertion failed (ModuleCopyNumber/100 != tmp)",MOKKA_OTHER_ERRORS);
#endif
  
  G4int theStave = (ModuleCopyNumber-theSDPiece*100)/10;
  G4int theModule = (ModuleCopyNumber-theSDPiece*100)%10;

  assert (StavesRotationMatrices[theStave-1]!=0);

  // find out local position in the standard module reference
  G4ThreeVector theCopyOfThePosition = thePosition;
  theCopyOfThePosition(2) -= *ModulesZOffsets[theModule];
  G4ThreeVector localPosition = 
    *StavesRotationMatrices[theStave-1] * theCopyOfThePosition;

  assert (ModuleRotationMatrix != 0);
  localPosition = (*ModuleRotationMatrix) * localPosition;
  // calculates I,J
  G4int I,J;

  assert (Layers[theLayer-1]!=0);
  assert (ModulesZOffsets[theModule]!=0);
  
  I = static_cast <G4int> ((localPosition(0) - Layers[theLayer-1]->X0)/CellDim(0));
  assert (I>=0);


  if(theSDPiece==ECALBARREL || theSDPiece==HCALBARREL) { // barrels
    J = static_cast <G4int> ((localPosition(2) - 
	 Layers[theLayer-1]->Z0)/CellDim(2));
  }
  else {  // endcaps
    J = static_cast <G4int> ((localPosition(1) - Layers[theLayer-1]->Y0)/CellDim(2));
  }

  assert (J>=0);

  G4ThreeVector localCellCenter, positionInCell;

    localCellCenter[0]=Layers[theLayer-1]->X0 + I*CellDim(0) +CellDim(0)/2.;
    localCellCenter[1]=Layers[theLayer-1]->Y0;
    localCellCenter[2]=Layers[theLayer-1]->Z0 + J*CellDim(2) + CellDim(2)/2.;

  positionInCell = localPosition - localCellCenter;
  if((fabs(positionInCell(0)) < (CellDim(0)/2 - theRadius)) &&
     (fabs(positionInCell(2)) < (CellDim(2)/2 - theRadius)))
          TouchCell(theSDPiece, theStave, theModule, I, J, theLayer,  
			  &thePosition, aStep, edep);
  else if((positionInCell(0) > (CellDim(0)/2 - theRadius)) &&
	  (fabs(positionInCell(2)) < (CellDim(2)/2 - theRadius))){
	  if(divide) edep /= 2;
          TouchCell(theSDPiece, theStave, theModule, I, J, theLayer,  
			  &thePosition, aStep, edep);
	  TouchCell(theSDPiece, theStave, theModule, I+1, J, theLayer,0,
			  aStep, edep);
  }
  else if((positionInCell(0) < -(CellDim(0)/2 - theRadius))&&
	  (fabs(positionInCell(2)) < (CellDim(2)/2 - theRadius))){
	  if(divide) edep /= 2;
          TouchCell(theSDPiece, theStave, theModule, I, J, theLayer,  
			  &thePosition, aStep, edep);
	  TouchCell(theSDPiece, theStave, theModule, I-1, J, theLayer,0,
			  aStep, edep);
  }
  else if((positionInCell(2) > (CellDim(2)/2 - theRadius)) &&
	  (fabs(positionInCell(0)) < (CellDim(0)/2 - theRadius))){
	  if(divide) edep /= 2;
          TouchCell(theSDPiece, theStave, theModule, I, J, theLayer,  
			  &thePosition, aStep, edep);
	  TouchCell(theSDPiece, theStave, theModule, I, J+1, theLayer,0,
			  aStep, edep);
  }
  else if((positionInCell(2) < -(CellDim(2)/2 - theRadius))&&
	  (fabs(positionInCell(0)) < (CellDim(0)/2 - theRadius))){
	  if(divide) edep /= 2;
          TouchCell(theSDPiece, theStave, theModule, I, J, theLayer,  
			  &thePosition, aStep, edep);
	  TouchCell(theSDPiece, theStave, theModule, I, J-1, theLayer,0,
			  aStep, edep);
  }
  else if((positionInCell(0) > (CellDim(0)/2 - theRadius)) &&
	  (positionInCell(2) > (CellDim(2)/2 - theRadius))){
	 if(sqrt((CellDim(0)/2-positionInCell(0))*(CellDim(0)/2-positionInCell(0))+
		 (CellDim(2)/2-positionInCell(2))*(CellDim(2)/2-positionInCell(2)))
	    < theRadius){
	 	if(divide) edep /= 4;
         	TouchCell(theSDPiece, theStave, theModule, I, J, theLayer,  
			  &thePosition, aStep, edep);
       		TouchCell(theSDPiece, theStave, theModule, I+1, J, theLayer,0,
			 aStep, edep);
         	TouchCell(theSDPiece, theStave, theModule, I, J+1, theLayer,0,
			 aStep, edep);
         	TouchCell(theSDPiece, theStave, theModule, I+1, J+1,theLayer,0,
			 aStep, edep);
	 }
	 else {
	 	if(divide) edep /= 3;
         	TouchCell(theSDPiece, theStave, theModule, I, J, theLayer,  
			  &thePosition, aStep, edep);
       		TouchCell(theSDPiece, theStave, theModule, I+1, J, theLayer,0,
			 aStep, edep);
         	TouchCell(theSDPiece, theStave, theModule, I, J+1, theLayer,0,
			 aStep, edep);
	 }
  }
  else if((positionInCell(0) > (CellDim(0)/2 - theRadius)) &&
	  (positionInCell(2) < -(CellDim(2)/2 - theRadius))){
	 if(sqrt((CellDim(0)/2-positionInCell(0))*(CellDim(0)/2-positionInCell(0))+
		 (CellDim(2)/2+positionInCell(2))*(CellDim(2)/2+positionInCell(2)))
	    < theRadius){
	 	if(divide) edep /= 4;
         	TouchCell(theSDPiece, theStave, theModule, I, J, theLayer,  
			  &thePosition, aStep, edep);
         	TouchCell(theSDPiece, theStave, theModule, I+1, J, theLayer,0,
			 aStep, edep);
       		TouchCell(theSDPiece, theStave, theModule, I, J-1, theLayer,0,
			 aStep, edep);
         	TouchCell(theSDPiece, theStave, theModule, I+1, J-1,theLayer,0,
			 aStep, edep);
	 }
	 else {
	 	if(divide) edep /= 3;
         	TouchCell(theSDPiece, theStave, theModule, I, J, theLayer,  
			  &thePosition, aStep, edep);
         	TouchCell(theSDPiece, theStave, theModule, I+1, J, theLayer,0,
			 aStep, edep);
       		TouchCell(theSDPiece, theStave, theModule, I, J-1, theLayer,0,
			 aStep, edep);
	 }
  }
  else if((positionInCell(0) < -(CellDim(0)/2 - theRadius)) &&
	  (positionInCell(2) < -(CellDim(2)/2 - theRadius))){
	 if(sqrt((CellDim(0)/2+positionInCell(0))*(CellDim(0)/2+positionInCell(0))+
		 (CellDim(2)/2+positionInCell(2))*(CellDim(2)/2+positionInCell(2)))
	    < theRadius){
	 	if(divide) edep /= 4;
         	TouchCell(theSDPiece, theStave, theModule, I, J, theLayer,  
			  &thePosition, aStep, edep);
         	TouchCell(theSDPiece, theStave, theModule, I-1, J, theLayer,0,
			 aStep, edep);
       		TouchCell(theSDPiece, theStave, theModule, I, J-1, theLayer,0,
			 aStep, edep);
         	TouchCell(theSDPiece, theStave, theModule, I-1, J-1,theLayer,0,
			 aStep, edep);
	 }
	 else {
	 	if(divide) edep /= 3;
         	TouchCell(theSDPiece, theStave, theModule, I, J, theLayer,  
			  &thePosition, aStep, edep);
         	TouchCell(theSDPiece, theStave, theModule, I-1, J, theLayer,0,
			 aStep, edep);
       		TouchCell(theSDPiece, theStave, theModule, I, J-1, theLayer,0,
			 aStep, edep);
	 }
  }
  else if((positionInCell(0) < -(CellDim(0)/2 - theRadius)) &&
	  (positionInCell(2) > (CellDim(2)/2 - theRadius))){
	 if(sqrt((CellDim(0)/2+positionInCell(0))*(CellDim(0)/2+positionInCell(0))+
		 (CellDim(2)/2-positionInCell(2))*(CellDim(2)/2-positionInCell(2)))
	    < theRadius){
	 	if(divide) edep /= 4;
         	TouchCell(theSDPiece, theStave, theModule, I, J, theLayer,  
			  &thePosition, aStep, edep);
         	TouchCell(theSDPiece, theStave, theModule, I-1, J, theLayer,0,
			 aStep, edep);
         	TouchCell(theSDPiece, theStave, theModule, I, J+1, theLayer,0,
			 aStep, edep);
         	TouchCell(theSDPiece, theStave, theModule, I-1, J+1,theLayer,0,
			 aStep, edep);
	 }
	 else {
	 	if(divide) edep /= 3;
         	TouchCell(theSDPiece, theStave, theModule, I, J, theLayer,  
			  &thePosition, aStep, edep);
         	TouchCell(theSDPiece, theStave, theModule, I-1, J, theLayer,0,
			 aStep, edep);
         	TouchCell(theSDPiece, theStave, theModule, I, J+1, theLayer,0,
			 aStep, edep);
	 }
  }
	  
  return true;
}

void SD02::TouchCell(G4int theSDPiece, G4int theStave, G4int theModule,
		G4int I, G4int J, G4int theLayer, G4ThreeVector* thePosition,
		G4Step *aStep, G4double edep){
  G4double time = aStep->GetTrack()->GetGlobalTime() ;  
  // find out the actual cell center coodinates
  G4ThreeVector theCellCenter = 
		GetCellCenter(theSDPiece, theStave, theModule, I, J, theLayer);
  
  if(thePosition != 0) {
  G4ThreeVector distCenter = *thePosition - theCellCenter;
#ifdef MOKKA_DEBUG
  if (distCenter.mag() > CellDim(0) + CellDim(1))
    {
      G4cout << "======= ASSERT WILL CRASH :\n"
	     << "\ntheSDPiece = " << theSDPiece
	     << ", theStave = " << theStave 
	     << ", theModule = " << theModule
	     << "\nI = " << I << ", J= " << J << ", K = " << theLayer
	     << "\nthePosition = " << thePosition
//	     << ", localPosition = " << localPosition
//	     << "localCellCenter = " << localCellCenter
	     << ", theCellCenter = " << theCellCenter
	     << ", distCenter.mag() = " << distCenter.mag() << G4endl;
      Control::Abort("SD02::TouchCell: Assertion failed (distCenter.mag() > CellDim(0) + CellDim(1))",MOKKA_OTHER_ERRORS);
    }
  
#endif
  }

  // creates a new cell or add the energy to the cell if it already exists

  G4int PID = 
    Control::GetControl()->GetPIDForCalHit(aStep);
  assert(PID!=-1);

  G4int PDG=
    aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
  
  G4bool found=false;
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
}

void SD02::EndOfEvent(G4HCofThisEvent*HCE)
{
  if(HCID<0)
    { HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]); }
  HCE->AddHitsCollection( HCID, CalCollection );
}

void SD02::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
  CalHit* newHit = new CalHit();
  while (newHit->Load(theSubDetectorEventHitsFileInput))
    {
      CalCollection->insert(newHit);
      newHit = new CalHit();
    }
  delete newHit;
}

void SD02::clear()
{
} 

void SD02::DrawAll()
{
} 

void SD02::PrintAll()
{
} 




