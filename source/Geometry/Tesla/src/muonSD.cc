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
// $Id: muonSD.cc,v 1.4 2008/10/21 15:09:58 musat Exp $
// $Name: mokka-07-00 $
//
#include "Control.hh"
#include "muonSD.hh"
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

#include "Encoder64Muon.hh"
#include "Encoder32.hh"

muonSD::muonSD(G4double Idim,G4double Jdim,G4double Thickness,G4int Piece,G4String SDname, G4bool id1Flag) 
  : VSensitiveDetector(SDname), CalCollection(0),
    CellDim (Idim,Thickness,Jdim),SDPiece(Piece),HCID(-1)
{
  assert (CellDim(0)>0);
  assert (CellDim(1)>0);
  assert (CellDim(2)>0);

  G4String CollName=SDname+"Collection";
  collectionName.insert(CollName);

  G4int i;
  for(i=0;i<MAX_STAVES_MUON;i++) 
    {
      StavesRotationMatrices[i]=0;
      InverseStavesRotationMatrices[i]=0;
      StavesPhirots[i]=0;
    }
  for(i=0;i<MAX_LAYERS_MUON;i++) {Layers[i]=0; Layers2[i]=0;}
  for(i=0;i<MAX_MODULES;i++) ModulesZOffsets[i]=0;

  if(id1Flag)
        theEncoder = new Encoder64Muon();
  else
        theEncoder = new Encoder32();

}

muonSD::~muonSD()
{
  G4int i;
  for(i=0;i<MAX_STAVES_MUON;i++) {
    if(StavesRotationMatrices[i]!=0) delete StavesRotationMatrices[i];
    if(InverseStavesRotationMatrices[i]!=0) 
                              delete InverseStavesRotationMatrices[i];
  }

  for(i=0;i<MAX_LAYERS_MUON;i++) {
    if(Layers[i]!=0) delete Layers[i];
    if(Layers2[i]!=0) delete Layers2[i];
  }

  for(i=0;i<MAX_MODULES;i++) 
    if(ModulesZOffsets[i]!=0) delete ModulesZOffsets[i];
  for(i=0;i<MAX_STAVES_MUON;i++) 
    if(StavesPhirots[i]!=0) delete StavesPhirots [i];

  if(CalCollection!=0) delete CalCollection;

  delete theEncoder;
}

void muonSD::Initialize(G4HCofThisEvent *)
{
//if(CalCollection!=0) delete CalCollection;
  CalCollection = new HitsCollection
    (SensitiveDetectorName,collectionName[0]); 
}

void muonSD::SetModuleZOffset(G4int moduleNumber,G4double Zoff)
{
  assert (moduleNumber<MAX_MODULES && moduleNumber>=0);

  if(ModulesZOffsets[moduleNumber] != 0) 
    {
      delete ModulesZOffsets[moduleNumber];
      ModulesZOffsets[moduleNumber]=0;
    }
  ModulesZOffsets[moduleNumber]  = new G4double(Zoff);
}

void muonSD::SetStaveRotationMatrix(G4int staveNumber, G4double phirot)
{

  assert (staveNumber<=MAX_STAVES_MUON && staveNumber>0);

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

void muonSD::AddLayer(G4int layerNumber, G4double X,G4double Y,G4double Z)
{ 
  assert (layerNumber<=MAX_LAYERS_MUON && layerNumber>0);
  assert (Layers[layerNumber-1] == 0);

  if( SDPiece==1)
    Layers[layerNumber-1] = new muLayerRef(X,Y,Z);
  else
    Layers2[layerNumber-1] = new muLayerRef(X,Y,Z);
}

void muonSD::AddLayerOffset(G4int layerNumber, G4double X,G4double Y,G4double Z)
{
  assert (layerNumber<=MAX_LAYERS_MUON && layerNumber>0);
  assert (Layers[layerNumber-1] != 0);

  Layers[layerNumber-1]->X0 +=X;
  Layers[layerNumber-1]->Y0 +=Y;
  Layers[layerNumber-1]->Z0 +=Z;
}

G4ThreeVector muonSD::GetCellCenter(G4int,G4int pS,G4int pM,
			G4int pI,G4int pJ,G4int pK) {

  assert (pS>0);
  assert (pM>=0);
  assert (pI>=0);
  assert (pJ>=0);
  assert (pK>0);
  assert (pS<=MAX_STAVES_MUON);
  assert (pM<MAX_MODULES);
  assert (pK<=MAX_LAYERS_MUON);

  G4ThreeVector localCellCenter;
//  if(pP==ECALBARREL || pP==HCALBARREL) { // barrels
  if(SDPiece==1) { //barrels
    
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
  
  theCellCenter[2] += *ModulesZOffsets[pM];

  return theCellCenter;
}

G4bool muonSD::ProcessHits(G4Step *aStep,G4TouchableHistory *)
{
  // process only if energy>0. except geantinos
  G4double edep;
  if ((edep = aStep->GetTotalEnergyDeposit())<=0. &&
      aStep->GetTrack()->GetDefinition()->GetParticleType()!="geantino") 
    return true;
  G4double time = aStep->GetTrack()->GetGlobalTime() ;  

  // in the copy number is encoded  layer, stove & module number!!
  G4int theCopyNo = aStep->GetTrack()->GetVolume()->GetCopyNo();
 
  // hit will deposited in the midle of the step
    G4ThreeVector thePosition = (aStep->GetPreStepPoint()->GetPosition()+
				 aStep->GetPostStepPoint()->GetPosition())*0.5;
 
    G4int theModule = (int) theCopyNo/100000;
    G4int theStave  = (int) (theCopyNo-100000*theModule)/1000;   
    G4int theLayer  = (int) theCopyNo-100000*theModule-1000*(theStave);
    G4int I;
    G4int J;
    G4ThreeVector theCellCenter;

  if(SDPiece==1) // first piece is barrel , second endcap!
    {
   // find out local position in the standard module reference
    G4ThreeVector localPosition =  *StavesRotationMatrices[theStave-1] * thePosition;

    I = static_cast <G4int> ((localPosition(0) +Layers[theLayer-1]->X0)/CellDim(0));
    J = static_cast <G4int> (fabs(localPosition(2))/CellDim(2));

     // problem sa poslednjom celijom u centru !!
     G4ThreeVector localCellCenter;
     localCellCenter[0]=-Layers[theLayer-1]->X0 + I*CellDim(0) +CellDim(0)/2.;
     localCellCenter[1]=Layers[theLayer-1]->Y0;

     // wanted that the structure starts from center not to have irregularities in the middle!
     double sgn_2;
     if(thePosition[2]>=0) sgn_2=1.0;
     if(thePosition[2]<0) sgn_2=-1.0;
     localCellCenter[2]=(J*CellDim(2)+CellDim(2)/2.0)*sgn_2; 

     theCellCenter = *InverseStavesRotationMatrices[theStave-1] * localCellCenter;
    
    }
  else
    {     
    I=  static_cast <G4int> (fabs(thePosition[0])/CellDim(0));
    J=  static_cast <G4int> (fabs(thePosition[1])/CellDim(2));
    double sgn_0;
    if(thePosition[0]>=0) sgn_0=1.0;
    if(thePosition[0]<0) sgn_0=-1.0;
    double sgn_1;
    if(thePosition[1]>=0) sgn_1=1.0;
    if(thePosition[1]<0) sgn_1=-1.0;
    double sgn_2;
    if(thePosition[2]>=0) sgn_2=1.0;
    if(thePosition[2]<0) sgn_2=-1.0;
    theCellCenter[0]= (I*CellDim(0)+CellDim(0)/2.0)*sgn_0;
    theCellCenter[1]= (J*CellDim(2)+CellDim(2)/2.0)*sgn_1; 
    theCellCenter[2]= sgn_2*Layers2[theLayer-1]->Z0; 
    //cout<<I<<"   "<<J<<"   "<<thePosition[1]<<"   "<<theCellCenter[1]<<endl;
    }

  // creates a new cell or add the energy to the cell if it already exists

  G4int PID = 
    Control::GetControl()->GetPIDForCalHit(aStep);
  assert(PID!=-1);

  G4int PDG=
    aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
  
  G4bool found=false;
  G4int n_hit = CalCollection->entries();

  cell_ids theCode =theEncoder->encode(theStave,theModule,I,J,theLayer,0);
  
  for(G4int i_hit = 0; i_hit< n_hit; i_hit++)
    if((*CalCollection)[i_hit]->
       testCell(theCode)) {
      (*CalCollection)[i_hit]->AddEdep(PID,PDG,edep,time);
      found = true;
      break;
    }
  
  if(!found) CalCollection->
	       insert(new CalHit (SDPiece,
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

void muonSD::EndOfEvent(G4HCofThisEvent*HCE)
{
  if(HCID<0)
    { HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]); }
  HCE->AddHitsCollection( HCID, CalCollection );
}

void muonSD::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
  CalHit* newHit = new CalHit();
  while (newHit->Load(theSubDetectorEventHitsFileInput))
    {
      CalCollection->insert(newHit);
      newHit = new CalHit();
    }
  delete newHit;
}

void muonSD::clear()
{
} 

void muonSD::DrawAll()
{
} 

void muonSD::PrintAll()
{
} 




void muonSD:: SetSymmetry(G4int symmetry)
{
  Symmetry=symmetry;
}
void muonSD:: SetInnerbox(G4double inbox)
{
  Inner_box=inbox;
}
