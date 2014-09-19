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
// $Id: SDHcalSD01.cc,v 1.8 2009/02/09 12:25:57 musat Exp $
// $Name: mokka-07-00 $
//
#include "Control.hh"
#include "SDHcalSD01.hh"
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

#include "Encoder64.hh"
#include "Encoder32.hh"

SDHcalSD01::SDHcalSD01(G4double Idim,G4double Jdim,G4double Thickness,
 G4int Piece,G4String SDname,G4double PadSeparation,G4bool id1Flag): VSensitiveDetector(SDname), CalCollection(0),CellDim (Thickness,Idim,Jdim),SDPiece(Piece),PadSpacing(PadSeparation),HCID(-1)
{
  assert (CellDim(0)>0);
  assert (CellDim(1)>0);
  assert (CellDim(2)>0);

  //PadSpacing=PadSeparation;

  G4String CollName=SDname+"Collection";
  collectionName.insert(CollName);

  G4int i;
  for(i=0;i<DHCAL_MAX_STAVES;i++) 
    {
      StavesRotationMatrices[i]=0;
      InverseStavesRotationMatrices[i]=0;
      StavesPhirots[i]=0;
    }
  for(i=0;i<DHCAL_MAX_LAYERS;i++) {
	Layers[i]=0;
  }
  for(i=0;i<DHCAL_MAX_MODULES;i++) ModulesZOffsets[i]=0;

  if(id1Flag)
        theEncoder = new Encoder64();
  else
        theEncoder = new Encoder32();

}

SDHcalSD01::~SDHcalSD01()
{
  G4int i;
  for(i=0;i<DHCAL_MAX_STAVES;i++) {
    if(StavesRotationMatrices[i]!=0) delete StavesRotationMatrices[i];
    if(InverseStavesRotationMatrices[i]!=0) delete InverseStavesRotationMatrices[i];
  }
  for(i=0;i<DHCAL_MAX_LAYERS;i++) 
    if(Layers[i]!=0) delete Layers[i];
  for(i=0;i<DHCAL_MAX_MODULES;i++) 
    if(ModulesZOffsets[i]!=0) delete ModulesZOffsets[i];
  for(i=0;i<DHCAL_MAX_STAVES;i++) 
    if(StavesPhirots[i]!=0) delete StavesPhirots [i];
}

void SDHcalSD01::Initialize(G4HCofThisEvent *)
{
//if(CalCollection!=0) delete CalCollection;
  CalCollection = new HitsCollection
    (SensitiveDetectorName,collectionName[0]); 
}

void SDHcalSD01::SetModuleZOffset(G4int moduleNumber,G4double Zoff)
{
  assert (moduleNumber<DHCAL_MAX_MODULES && moduleNumber>=0);

  if(ModulesZOffsets[moduleNumber] != 0) 
    {
      delete ModulesZOffsets[moduleNumber];
      ModulesZOffsets[moduleNumber]=0;
    }
  ModulesZOffsets[moduleNumber]  = new G4double(Zoff);

}

void SDHcalSD01::SetStaveRotationMatrix(G4int staveNumber, 
	G4RotationMatrix* rotInv)
{

  assert (staveNumber<=DHCAL_MAX_STAVES && staveNumber>0);

  if(InverseStavesRotationMatrices [staveNumber-1] !=0) 
    delete  InverseStavesRotationMatrices [staveNumber-1];

  if(StavesPhirots [staveNumber-1] !=0) 
    {
      delete  StavesPhirots [staveNumber-1];
      StavesPhirots [staveNumber-1] = 0;
    }

//  StavesPhirots [staveNumber-1] = new G4double(phirot);
  StavesRotationMatrices [staveNumber-1] =
	new G4RotationMatrix(rotInv->inverse());
  InverseStavesRotationMatrices [staveNumber-1] = rotInv;
}

void SDHcalSD01::AddLayer(G4int layerNumber, G4double X,G4double Y,G4double hY, G4double hZ)
{

  assert (layerNumber<=DHCAL_MAX_LAYERS && layerNumber>0);
  assert (Layers[layerNumber-1] == 0);

  Layers[layerNumber-1] =
    new DhcalLayerRef(X,Y,hY,hZ);
}

G4ThreeVector SDHcalSD01::GetCellCenter(G4int,G4int pS,G4int pM,
			G4int pI,G4int pJ,G4int pK) {

  assert (pS>0);
  assert (pM>=0 && pM<=5);
  assert (pI>=0);
  assert (pJ>=0);
  assert (pK>0);
  assert(pS<=DHCAL_MAX_STAVES);
  assert(pM<DHCAL_MAX_MODULES);
  assert (pK<=DHCAL_MAX_LAYERS);

  G4ThreeVector localCellCenter;
    
  // builds the cell center coordinates for I,J
  localCellCenter[0]=Layers[pK-1]->X0;
  localCellCenter[1]=Layers[pK-1]->hZ0 - *ModulesZOffsets[pM] 
		- pJ*(CellDim(2)+PadSpacing) - CellDim(2)/2. - PadSpacing;
  localCellCenter[2]=-Layers[pK-1]->hY0 + pI*(CellDim(1)+PadSpacing) +
		CellDim(1)/2. + PadSpacing + Layers[pK-1]->Y0 ;
  
  assert (InverseStavesRotationMatrices[pS-1]!=0);
  
  // find out the actual cell center coordinates
  G4ThreeVector theCellCenter = 
    *InverseStavesRotationMatrices[pS-1] * localCellCenter;
  
  return theCellCenter;
}

G4bool SDHcalSD01::ProcessHits(G4Step *aStep,G4TouchableHistory *)
{
  // process only if energy>0. except geantinos
  G4double edep;
  if ((edep = aStep->GetTotalEnergyDeposit())<=0. &&
      aStep->GetTrack()->GetDefinition()->GetParticleType()!="geantino") 
    return true;
  G4double time = aStep->GetTrack()->GetGlobalTime() ;  

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
    if(ModuleCopyNumber>100) break;
  }

#ifdef MOKKA_DEBUG
  if (ModuleCopyNumber == 0) {
      G4cout <<"MOKKA WARNING: ModuleCopyNumber==0 in SDHcalSD01::ProcessHits, "
	     << "at " << thePosition << ", SDPiece = " << SDPiece << G4endl;
      G4cout <<"Volume name in aStep->GetPostStepPoint()GetPhysicalVolume() is "
	     << aStep->GetPostStepPoint()->GetPhysicalVolume()->GetName() 
		<< G4endl;

      G4cout << "Volume hierarchy in G4TouchableHistory :\n";
      for (G4int idepth = 0; idepth <= depth; idepth++) {
	G4cout << "level " << idepth << ", volume "
	       << history->GetVolume(idepth)->GetName()
	       << "copy number = " << history->GetVolume(idepth)->GetCopyNo() 
	       << G4endl;
      }
      Control::Abort("SDHcalSD01::ProcessHits: Assertion failed (ModuleCopyNumber == 0)",MOKKA_OTHER_ERRORS);
    }
#endif
  G4int theSDPiece = ModuleCopyNumber%100;

#ifdef MOKKA_DEBUG
  G4int tmp = SDPiece;
  if(tmp==ECALENDCAPMINUS && thePosition(2)>0) tmp+=(ECALENDCAPPLUS-ECALENDCAPMINUS);
  else if(tmp==HCALENDCAPMINUS && thePosition(2)>0) tmp+=(HCALENDCAPPLUS-HCALENDCAPMINUS);
  if (ModuleCopyNumber%100 != tmp)
    Control::Abort("SDHcalSD01::ProcessHits: Assertion failed (ModuleCopyNumber/100 != tmp)",MOKKA_OTHER_ERRORS);
#endif
  
  G4int theStave = (G4int)((ModuleCopyNumber-theSDPiece)/1000);
  G4int theModule = (G4int)(((ModuleCopyNumber-theSDPiece)%1000)/100);

  assert (StavesRotationMatrices[theStave-1]!=0);

  // find out local position in the standard module reference
  G4ThreeVector localPosition = 
    *StavesRotationMatrices[theStave-1] * thePosition;

  // calculates I,J
  G4int I,J;

  assert (Layers[theLayer-1]!=0);
  assert (ModulesZOffsets[theModule]!=0);

  localPosition(0)=localPosition(0) - Layers[theLayer-1]->X0;
  localPosition(1)=localPosition(1) + *ModulesZOffsets[theModule];
  localPosition(2)=localPosition(2) - Layers[theLayer-1]->Y0;

#ifdef MOKKA_DEBUG
  G4double diff0 = fabs(localPosition(2))-Layers[theLayer-1]->hY0;

  if(diff0 > 1E-04){

      G4cout << "\ntheSDPiece = " << theSDPiece
	     << ", theStave = " << theStave 
	     << ", theModule = " << theModule
	     << ", K = " << theLayer
	     << "\nthePosition = " << thePosition
	     << ", localPosition = " << localPosition
	     << "Layers[theLayer-1]->Y0 = " << Layers[theLayer-1]->Y0
	     << "Layers[theLayer-1]->hY0 = " << Layers[theLayer-1]->hY0
	     << G4endl;

      Control::Abort("SDHcalSD01::ProcessHits: Assertion failed:  (fabs(localPosition(2)) >  Layers[theLayer-1]->hY0)",MOKKA_OTHER_ERRORS);
  }

  diff0 = fabs(localPosition(1))-Layers[theLayer-1]->hZ0;
  if(diff0 > 1E-04) {

      G4cout << "\ntheSDPiece = " << theSDPiece
	     << ", theStave = " << theStave 
	     << ", theModule = " << theModule
	     << ", K = " << theLayer
	     << "\nthePosition = " << thePosition
	     << ", localPosition = " << localPosition
	     << "*ModulesZOffsets[theModule] = " << *ModulesZOffsets[theModule]
	     << "Layers[theLayer-1]->hZ0 = " << Layers[theLayer-1]->hZ0
	     << G4endl;


      Control::Abort("SDHcalSD01::ProcessHits: Assertion failed:  (fabs(localPosition(1)) >  Layers[theLayer-1]->hZ0)",MOKKA_OTHER_ERRORS);
  }

  G4AffineTransform transf = history->GetHistory()->GetTransform(depth);
  G4AffineTransform invTransf = transf.Inverse();
  G4ThreeVector theNewPosition =
                  invTransf.TransformPoint(localPosition);
 
  diff0 =((G4ThreeVector)(theNewPosition - thePosition)).mag();
  if (diff0 > 1E-09)
    {
      G4cout << "======= ASSERT WILL CRASH :\n"
	     << "\ntheSDPiece = " << theSDPiece
	     << ", theStave = " << theStave 
	     << ", theModule = " << theModule
	     << ", K = " << theLayer
	     << "\nthePosition = " << thePosition
	     << ", localPosition = " << localPosition
	     << ", thePosition from Inverse Transform = " << theNewPosition
	     << ", dist.mag() = " << diff0 << G4endl;
      Control::Abort("SDHcalSD01::ProcessHits: Assertion failed (dist between hit position one way and back > 1E-09",MOKKA_OTHER_ERRORS);
    }

#endif

  J = static_cast <G4int> ((Layers[theLayer-1]->hZ0 -localPosition(1))
		/(CellDim(2)+PadSpacing));

  G4double delta1 = (Layers[theLayer-1]->hZ0 -localPosition(1)) -
	J*(CellDim(2)+PadSpacing);
  if(delta1 >= PadSpacing) 
	;// Keep the value of J
  else if((delta1 == 0) && (J != 0))
	J--;
  else
	return true;

//  I = static_cast <G4int> ((-localPosition(2) + Layers[theLayer-1]->hY0)
  I = static_cast <G4int> ((localPosition(2) + Layers[theLayer-1]->hY0)
		/(CellDim(1)+PadSpacing));

  delta1 = (localPosition(2) + Layers[theLayer-1]->hY0) -
	I*(CellDim(1)+PadSpacing);
  if(delta1 >= PadSpacing) 
	;// Keep the value of I
  else if((delta1 == 0) && (I != 0))
	I--;
  else
	return true;

  // find out the actual cell center coodinates
  G4ThreeVector theCellCenter = 
		GetCellCenter(theSDPiece, theStave, theModule, I, J, theLayer);
  
#ifdef MOKKA_DEBUG
  G4ThreeVector distCenter = thePosition - theCellCenter;
  G4double diagonal = sqrt(CellDim(2)*CellDim(2) + CellDim(1)*CellDim(1)
				 + CellDim(0)*CellDim(0))/2.;
  if ((distCenter.mag() - diagonal) > 1E-04)
    {
      G4cout << "======= ASSERT WILL CRASH :\n"
	     << "\ntheSDPiece = " << theSDPiece
	     << ", theStave = " << theStave 
	     << ", theModule = " << theModule
	     << "\nI = " << I << ", J= " << J << ", K = " << theLayer
	     << "\nthePosition = " << thePosition
	     << ", localPosition = " << localPosition
	     << ", theCellCenter = " << theCellCenter
	     << ", distCenter.mag() = " << distCenter.mag() << G4endl;
      Control::Abort("SDHcalSD01::ProcessHits: Assertion failed (distCenter.mag() > diagonal",MOKKA_OTHER_ERRORS);
    }
#endif

  // creates a new cell or add the energy to the cell if it already exists

  G4int PID = 
    Control::GetControl()->GetPIDForCalHit(aStep);
  assert(PID!=-1);

  G4int PDG=
    aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
  
  G4bool found=false;
  G4int n_hit = CalCollection->entries();

  cell_ids theCode = 
    theEncoder->encode(theStave,theModule + 1,
		   I,J,theLayer,0);
  
  float * sp = 0;
  if(Control::LCIODetailedShowerMode) {
  	sp = new float[3];
  	sp[0] = thePosition[0];
  	sp[1] = thePosition[1];
  	sp[2] = thePosition[2];
  }

  for(G4int i_hit = 0; i_hit< n_hit; i_hit++)
    if((*CalCollection)[i_hit]->
       testCell(theCode)) {
      (*CalCollection)[i_hit]->AddEdep(PID,PDG,edep,time,sp);
      found = true;
      break;
    }
  
  if(!found) CalCollection->
	       insert(new CalHit (theSDPiece,
				  theStave,
				  theModule + 1,
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
				  theCode,sp));
  return true;
}

void SDHcalSD01::EndOfEvent(G4HCofThisEvent*HCE)
{
  if(HCID<0)
    { HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]); }
  HCE->AddHitsCollection( HCID, CalCollection );
}

void SDHcalSD01::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
  CalHit* newHit = new CalHit();
  while (newHit->Load(theSubDetectorEventHitsFileInput))
    {
      CalCollection->insert(newHit);
      newHit = new CalHit();
    }
  delete newHit;
}

void SDHcalSD01::clear()
{
} 

void SDHcalSD01::DrawAll()
{
} 

void SDHcalSD01::PrintAll()
{
} 




