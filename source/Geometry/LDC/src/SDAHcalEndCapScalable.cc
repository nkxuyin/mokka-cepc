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
// $Id: SDAHcalEndCapScalable.cc,2011.12.05 S.Lu $
// 
//
#include "Control.hh"
#include "SDAHcalEndCapScalable.hh"
#include "G4StepPoint.hh"
#include "G4TransportationManager.hh"
#include "G4ParticleDefinition.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "globals.hh"
#include "G4UImanager.hh"
#include "G4VisExtent.hh"
#include "G4Box.hh"
#include "Encoder32Hcal.hh"
#include <assert.h>

//#define SDAHcalEndCapScalable_DEBUG

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                            ~
// Destructor                                                 ~
//                                                            ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    SDAHcalEndCapScalable::SDAHcalEndCapScalable(G4double Idim, G4double Jdim, G4double Thickness, 
				 G4int Piece,G4String SDname,
				 G4bool applyBirksLaw) 
      : SD(Idim,Jdim,Thickness,Piece,SDname)
{
  //Flag: 1=apply Birks law, 0=do not do it
  applyBirksLawFlag = applyBirksLaw;
#ifdef SDAHcalEndCapScalable_DEBUG
  G4cout<<"HCAL endcap sensitive detector name: "<<SDname<<G4endl;
#endif
        int i;
	for(i=0;i<SCALABLE_MAX_ENDCAP_MODULES_NUMBER;i++) ModulesYOffsets[i]=0;
	theEncoder = new Encoder32Hcal();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                            ~
// Constructor                                                ~
//                                                            ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SDAHcalEndCapScalable::~SDAHcalEndCapScalable() 
{
	for(int i=0;i<SCALABLE_MAX_ENDCAP_MODULES_NUMBER;i++) 
		if(ModulesYOffsets[i]!=0) delete ModulesYOffsets[i];

        delete theEncoder;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                            ~
// Calculate tile center in the local coordinates system      ~
//                                                            ~
// overwrite GetLocalCellCenter with module information       ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
G4ThreeVector SDAHcalEndCapScalable::GetLocalCellCenter(G4int pI,G4int pJ,G4int pK, G4int pM) 
{
#ifdef SDAHcalEndCapScalable_DEBUG
  G4cout<<"enter SDAHcalEndCapScalable::GetLocalCellCenter I:J:K:M"<<G4endl;
#endif

  assert (pI >= 0);
  assert (pJ >= 0);
  assert (pK > 0);
  assert (pK <= MAX_LAYERS);
  assert (pM >= 0);
  assert (pM <= SCALABLE_MAX_ENDCAP_MODULES_NUMBER);

  G4ThreeVector localCellCenter;

  // Builds the cell center coodinates for I,J
  localCellCenter[0] = pI * CellDim(0) + CellDim(0)/2.+ Layers[pK-1]->X0;
  localCellCenter[1] = pJ * CellDim(1) + CellDim(1)/2.+ *ModulesYOffsets[pM];
  localCellCenter[2] = Layers[pK-1]->Z0;
	
  return localCellCenter;
}


void SDAHcalEndCapScalable::SetModuleYOffset(G4int moduleNumber,G4double Yoff)
{
	assert (moduleNumber<SCALABLE_MAX_ENDCAP_MODULES_NUMBER && moduleNumber>=0);
	
	if(ModulesYOffsets[moduleNumber] != 0) 
	  {
		delete ModulesYOffsets[moduleNumber];
		ModulesYOffsets[moduleNumber]=0;
	  }
	ModulesYOffsets[moduleNumber]  = new G4double(Yoff);
}



void SDAHcalEndCapScalable::SetmaxScalableEndcapModuleNumber(G4int maxEndCapModuleNumber)
{
  assert ( maxEndCapModuleNumber > 0 && assert <100);
  assert ( maxEndCapModuleNumber%2 == 0);

  maxScalableEndcapModuleNumber = maxEndCapModuleNumber;

}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                            ~
// Process hits                                               ~
//                                                            ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
G4bool SDAHcalEndCapScalable::ProcessHits(G4Step *aStep,G4TouchableHistory *)
{
  // process only if energy>0 if not geantinos
  G4double edep = aStep->GetTotalEnergyDeposit();
  if (edep <=0. &&
      aStep->GetTrack()->GetDefinition()->GetParticleType()!="geantino")
    return true;

  //-----------------------------------------------------
  // The hit will deposited in the middle of the step
  G4ThreeVector thePosition = 
    (aStep->GetPreStepPoint()->GetPosition()+
     aStep->GetPostStepPoint()->GetPosition())*0.5;

  

#ifdef SDAHcalEndCapScalable_DEBUG
  G4cout<<"\n\n\n thePosition="<<thePosition<<G4endl;
  G4TouchableHandle myTouchable = aStep->GetPreStepPoint()->GetTouchableHandle();
  G4AffineTransform mytransform = myTouchable->GetHistory()->GetTopTransform();
  G4ThreeVector mylocalPosition = mytransform.TransformPoint(thePosition);
  G4cout<<"mylocalPosition: "<<mylocalPosition<<G4endl;

  mytransform.Invert();
  G4ThreeVector myglobalPosition = mytransform.TransformPoint(mylocalPosition);
  G4cout<<"myglobalPosition: "<<myglobalPosition<<G4endl;
#endif  
  G4double time = aStep->GetTrack()->GetGlobalTime() ;
  
  //----------------------------------------------------
  // Find out the stave and module id looking for the
  // module copy number and decoding it

  G4TouchableHandle theTouchable = aStep->GetPreStepPoint()->GetTouchableHandle();

	
  //Identical to theLayer = theTouchable->GetCopyNumber(2);
	//the layer number is the volume copy number
  G4int theLayer = aStep->GetTrack()->GetVolume()->GetCopyNo();

  if (theLayer <=0 || theLayer > MAX_LAYERS) 
    G4Exception("SDAHcalEndCapScalable::ProcessHits() - BAD history, FATAL ERROR");

	// Find out the stave and module id looking for the
	// module copy number and decoding it
	const G4VTouchable *history =aStep->GetPreStepPoint()->GetTouchable();
	
	G4int depth = history->GetHistory()->GetDepth();
	G4int ModuleCopyNumber = -1;
	for (G4int idepth = 0; idepth <= depth; idepth++) {
		ModuleCopyNumber=history->GetVolume(idepth)->GetCopyNo();
		if(ModuleCopyNumber>100) break;
	}

	G4int theStave   = ModuleCopyNumber/1000;
	G4int theModule  = (ModuleCopyNumber - theStave*1000)/10;
	G4int theSDPiece = (ModuleCopyNumber - theStave*1000) % 10;
	
  //---------------------------------------------------
#ifdef SDAHcalEndCapScalable_DEBUG
  G4cout<<"ModuleCopyNumber="<<ModuleCopyNumber <<G4endl;	
  G4cout<<"volume entered: "<<theTouchable->GetVolume()->GetName()<<G4endl;
  G4cout<<"theModule="<<theModule<<G4endl;
  G4cout<<"theLayer="<<aStep->GetTrack()->GetVolume()->GetCopyNo()<<G4endl;
  G4cout<<"theStave="<<theStave<<G4endl;
  G4cout<<"theSDPiece="<<theSDPiece<<G4endl;
  G4cout<<"thePosition="<<thePosition<<G4endl;
  G4cout<<"  Layers[theLayer-1]->X0="<<Layers[theLayer-1]->X0<<G4endl;
  G4cout<<"  Layers[theLayer-1]->Y0="<<Layers[theLayer-1]->Y0<<G4endl;
  G4cout<<"  Layers[theLayer-1]->Z0="<<Layers[theLayer-1]->Z0<<G4endl;
	for (int i= 0; i<16; i++) {
		G4cout << "ModulesYOffsets["<<i<<"] = "<<*ModulesYOffsets[i]<<G4endl;
	}

#endif
 

  //---------------------------------------------------------
  // Find out local position in the standard module reference
  G4ThreeVector localPosition = theTouchable->GetHistory()->GetTopTransform().TransformPoint(thePosition);
  //-------------------------------------------------------

  // Calculate I,J
  G4int I,J;

  I = static_cast<G4int>(( localPosition(0) - Layers[theLayer-1]->X0)/CellDim(0));
  J = static_cast<G4int>(( localPosition(1) - *ModulesYOffsets[theModule])/CellDim(1));


#ifdef SDAHcalEndCapScalable_DEBUG
  G4cout<<"CellDim: "<<CellDim<<G4endl;
  G4cout<<"localPosition="<<localPosition<<G4endl;
  G4cout<<"I="<<I<<"  J="<<J<<"  theLayer="<<theLayer<<G4endl;
  if (I < 0 || I > 11) G4Exception("SDAHcalEndCapScalable::ProcessHits() - invalid cell index I, aborting...");
  if (J < 0 || J > 511) G4Exception("SDAHcalEndCapScalable::ProcessHits() - invalid cell index J, aborting...");
#endif

  //Must have I>=0
  if (I < 0) G4Exception("SDAHcalEndCapScalable::ProcessHits() - invalid negativ cell index I, aborting...");
  
  //Must have J>=0
  if (J < 0) G4Exception("SDAHcalEndCapScalable::ProcessHits() - invalid negativ cell index J, aborting...");
  

  G4AffineTransform transform = theTouchable->GetHistory()->GetTopTransform();

  G4ThreeVector localCellCenter = GetLocalCellCenter(I, J, theLayer, theModule);

  //----------------------------------------------------------------------
  //Now calculate the cell center in the global coordinate system
  transform.Invert();
  G4ThreeVector theCellCenter = transform.TransformPoint(localCellCenter);

  
  //-----------------------------------------------------------------------
  // Create a new cell or add the energy to the cell if it already exists
  G4int PID = Control::GetControl()->GetPIDForCalHit(aStep);
  assert(PID!=-1);

  G4int PDG = aStep->GetTrack()->GetDefinition()->GetPDGEncoding();

#ifdef SDAHcalEndCapScalable_DEBUG
  G4cout<<"localCellCenter: "<<localCellCenter<<G4endl;
  G4cout<<"theCellCenter:   "<<theCellCenter<<G4endl;
  G4cout<<"PID="<<PID<<G4endl;
  G4cout<<"PDG="<<PDG<<G4endl;
#endif 

  //-----------------------------------------------------------------------
  // Prepare the final module, I, J number for AHCAL end cap geometry
  // before encode the cell ID.
  
  // count I from module 0 to module 15 along x-axis
  // endCapID 0: I [0,11], endCapID 1: I [12, 23] ... endCapID 15: [180, 191]
  //I [0,191]
  I = theModule * 12 + I;
  
  // there is the center hole. count all J from y = 0.0.
  // the J start from 12, [12, 96], other J: [0, MAX_itself]
  if( theModule == maxScalableEndcapModuleNumber/2-1 
      || theModule == maxScalableEndcapModuleNumber/2 ) J += 12;
  
  //There is only two module for end cap 0, and 6,
  //And 1 to 5 are for Barrel.
  if(theCellCenter[2]>0) theModule = 0; //z >0;
  if(theCellCenter[2]<0) theModule = 6; //z <0
  
  //Stave: 1: up part, stave 2: down part
  //Module: 0: +z, 6: -z (endCapID [1,16])
  //I: [0,191]
  //J: [0,96]
  //K-1:[0,47] theLayer: [1,48]
  cell_ids theCode = theEncoder->encode(theStave,theModule, I,J,theLayer,0);
  
  //------------------------------------------------------------------------
  //Birks law:
  if (applyBirksLawFlag == true) {
  	G4double attenuatedEnergy = SD::BirkAttenuation(aStep);
	edep = attenuatedEnergy; 
  }

  //------------------------------------------------------------------------
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
    insert(new CalHit (theSDPiece, //the detector piece number (0 or 6 for the HCAL endcaps)???
		       theStave,   //the stave number
		       theModule, //the module
		       I,          //the I,J cell coordinates in the cells matrix 
		       J,          //
		       theLayer,   //the Sensitive (scintillator) layer number (>= 1)
		       0,
		       theCellCenter (0),//the position of the cell center in world coordinates
		       theCellCenter (1),
		       theCellCenter (2),
		       edep,         //the total energy deposited in the cell by the PID particle and  its secondaries;
		       PID,          //the PID of the primary particle id in the Pythia file;
		       PDG,          //the PDG (particle type).
		       time,
		       theCode));
  return true;
}
