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
// $Id: SDHcalEndCapTesla.cc,v 1.4 2008/10/23 11:30:31 angela Exp $
// $Name: mokka-07-00 $
//
#include "Control.hh"
#include "SDHcalEndCapTesla.hh"
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

    //#define SDHcalEndCapTesla_DEBUG

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                            ~
// Destructor                                                 ~
//                                                            ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    SDHcalEndCapTesla::SDHcalEndCapTesla(G4double Idim, G4double Jdim, G4double Thickness, 
					 G4int Piece,G4String SDname,
					 G4bool applyBirksLaw) 
      : SD(Idim,Jdim,Thickness,Piece,SDname)
{
  //Flag: 1=apply Birks law, 0=do not do it
  applyBirksLawFlag = applyBirksLaw;
#ifdef SDHcalEndCapTesla_DEBUG
  G4cout<<"HCAL endcap sensitive detector name: "<<SDname<<G4endl;
#endif

  theEncoder = new Encoder32Hcal();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                            ~
// Constructor                                                ~
//                                                            ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SDHcalEndCapTesla::~SDHcalEndCapTesla() 
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                            ~
// Calculate tile center in the local coordinates system      ~
//                                                            ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
G4ThreeVector SDHcalEndCapTesla::GetLocalCellCenter(G4int pI,G4int pJ,G4int pK) 
{
#ifdef SDHcalEndCapTesla_DEBUG
  G4cout<<"enter SDHcalEndCapTesla::GetLocalCellCenter"<<G4endl;
#endif

  assert (pI >= 0);
  assert (pJ >= 0);
  assert (pK > 0);
  assert (pK <= MAX_LAYERS);

  G4ThreeVector localCellCenter;

  // Builds the cell center coodinates for I,J
  localCellCenter[0] = pI * CellDim(0) + CellDim(0)/2.+ Layers[pK-1]->X0;
  localCellCenter[1] = pJ * CellDim(1) + CellDim(1)/2.+ Layers[pK-1]->Y0;
  localCellCenter[2] = Layers[pK-1]->Z0;

  return localCellCenter;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                            ~
// Process hits                                               ~
//                                                            ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
G4bool SDHcalEndCapTesla::ProcessHits(G4Step *aStep,G4TouchableHistory *)
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

  

#ifdef SDHcalEndCapTesla_DEBUG
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

  //Copy number 1 refers to an endcap stave. Its copy number contains the SD piece number 
  //(i.e. if stave is at Z>0: theSDPiece=6,  else if at Z<0, the SDPiece=4) 
  //and stave number.
  G4int moduleCopyNo1 = theTouchable->GetCopyNumber(1);
  assert (moduleCopyNo1 != 0);  

  G4int theSDPiece = moduleCopyNo1/10;
  G4int theStave   = moduleCopyNo1 - theSDPiece*10;
  assert(theStave <= MAX_STAVES);

  //Identical to theLayer = theTouchable->GetCopyNumber(2);
  G4int theLayer = aStep->GetTrack()->GetVolume()->GetCopyNo();

  if (theLayer <=0 || theLayer > MAX_LAYERS) 
    G4Exception("SDHcalEndCapTesla::ProcessHits() - BAD history, FATAL ERROR");

  //---------------------------------------------------
#ifdef SDHcalEndCapTesla_DEBUG
  G4cout<<"moduleCopyNo1="<<moduleCopyNo1<<" theSDPiece="<<theSDPiece<<" theStave="<<theStave<<G4endl;
  G4cout<<"volume entered: "<<theTouchable->GetVolume()->GetName()<<G4endl;
  G4cout<<"theLayer="<<aStep->GetTrack()->GetVolume()->GetCopyNo()<<G4endl;
  G4cout<<"theStave="<<theStave<<G4endl;
  G4cout<<"theSDPiece="<<theSDPiece<<G4endl;
  G4cout<<"thePosition="<<thePosition<<G4endl;
  G4cout<<"  Layers[theLayer-1]->X0="<<Layers[theLayer-1]->X0<<G4endl;
  G4cout<<"  Layers[theLayer-1]->Y0="<<Layers[theLayer-1]->Y0<<G4endl;
  G4cout<<"  Layers[theLayer-1]->Z0="<<Layers[theLayer-1]->Z0<<G4endl;
#endif
 

  //---------------------------------------------------------
  // Find out local position in the standard module reference
  G4ThreeVector localPosition = theTouchable->GetHistory()->GetTopTransform().TransformPoint(thePosition);
  //-------------------------------------------------------
  // Calculate I,J
  G4int I,J;

  I = static_cast<G4int>(( localPosition(0) - Layers[theLayer-1]->X0)/CellDim(0));
  J = static_cast<G4int>(( localPosition(1) - Layers[theLayer-1]->Y0)/CellDim(1));


#ifdef SDHcalEndCapTesla_DEBUG
  G4cout<<"CellDim: "<<CellDim<<G4endl;
  G4cout<<"localPosition="<<localPosition<<G4endl;
  G4cout<<"I="<<I<<"  J="<<J<<"  theLayer="<<theLayer<<G4endl;
  if (I < 0 || I > 108) G4Exception("SDHcalEndCapTesla::ProcessHits() - invalid cell index I, aborting...");
  if (J < 0 || J > 87) G4Exception("SDHcalEndCapTesla::ProcessHits() - invalid cell index J, aborting...");
#endif

  //Must have I>=0
  if (I < 0) G4Exception("SDHcalEndCapTesla::ProcessHits() - invalid negativ cell index I, aborting...");
  
  //Must have J>=0
  if (J < 0) G4Exception("SDHcalEndCapTesla::ProcessHits() - invalid negativ cell index J, aborting...");
  

  G4AffineTransform transform = theTouchable->GetHistory()->GetTopTransform();
  G4ThreeVector localCellCenter = GetLocalCellCenter(I, J, theLayer);
  
  //--------------------------------------------------------------------
  //Remove hits which are in a tile not fully contained in the endcap 
  //sensitive volume
  //(this may happen since we shift the hit to the center of the tile).
  //For this, check if the upper right corner of the square of a cell
  //is still in the volume.
  G4ThreeVector upperRightCorner(localCellCenter(0) + CellDim(0)/2, 
				 localCellCenter(1) + CellDim(1)/2, 
				 localCellCenter(2));
  G4VSolid *endcapUnion = theTouchable->GetSolid();
  if (endcapUnion->Inside(upperRightCorner) == kOutside) return true;


#ifdef SDHcalEndCapTesla_DEBUG
  G4Box *bottomBox = (G4Box*)endcapUnion->GetConstituentSolid(0);
  G4double box_half_y = bottomBox->GetYHalfLength();
  G4cout<<"localCellCenter: "<<localCellCenter<<G4endl;
  G4cout<<"box_half_y="<<box_half_y<<G4endl;
#endif

  //----------------------------------------------------------------------
  //Now calculate the cell center in the global coordinate system
  transform.Invert();
  G4ThreeVector theCellCenter = transform.TransformPoint(localCellCenter);

  
  //-----------------------------------------------------------------------
  // Create a new cell or add the energy to the cell if it already exists
  G4int PID = Control::GetControl()->GetPIDForCalHit(aStep);
  assert(PID!=-1);

  G4int PDG = aStep->GetTrack()->GetDefinition()->GetPDGEncoding();

#ifdef SDHcalEndCapTesla_DEBUG
  G4cout<<"localCellCenter: "<<localCellCenter<<G4endl;
  G4cout<<"theCellCenter:   "<<theCellCenter<<G4endl;
  G4cout<<"PID="<<PID<<G4endl;
  G4cout<<"PDG="<<PDG<<G4endl;
#endif 

  cell_ids theCode = theEncoder->encode(theStave,theSDPiece, I,J,theLayer,0);
  
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
    insert(new CalHit (theSDPiece, //the detector piece number (0 or 6 for the HCAL endcaps)
		       theStave,   //the stave number
		       theSDPiece, //the module???
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
