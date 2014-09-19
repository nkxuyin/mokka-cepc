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
#include "SDHcalBarrel.hh"
#include "Encoder32Hcal.hh"
#include "G4SDManager.hh"
#include "G4VSolid.hh"

    //#define SDHcalBarrel_DEBUG

SDHcalBarrel::SDHcalBarrel(G4double IdimInt, G4double Jdim, G4double cellThickness,
			   G4int Piece, G4String SDname,
 			   G4double xOffset,
			   G4bool applyBirksLaw)
  : SD(IdimInt, Jdim, cellThickness, Piece, SDname)
{
  //initialize the dimensions of the integer cell
  DimIntCell = SD::CellDim;

  //set counter of HCAL layers to 0
  countHcalLayers = 0;
  
  G4int i;
  for(i = 0; i < MAX_HALF_STAVES; i++) {
    StavesRotationMatrices[i]        = NULL;
    InverseStavesRotationMatrices[i] = NULL;
    StavesPhirots[i]                 = 0;
  }
  for(i = 0; i < MAX_DOUBLE_LAYERS; i++) {
    Layers[i] = 0;
    DimFractCellPerLayer[i] = new G4ThreeVector(0,0,0);
  }

  theEncoder = new Encoder32Hcal();
  theXoffset = xOffset;
  applyBirksLawFlag = applyBirksLaw;
}
//==================================================================
//
//
//==================================================================
void SDHcalBarrel::SetFractCellDimPerLayer(G4int layer_id, G4ThreeVector newFractCellDim)
{
  DimFractCellPerLayer[layer_id-1]->setX(newFractCellDim.x());
  DimFractCellPerLayer[layer_id-1]->setY(newFractCellDim.y());
  DimFractCellPerLayer[layer_id-1]->setZ(newFractCellDim.z());
#ifdef SDHcalBarrel_DEBUG
  G4cout<<"layer_id="<<layer_id<<"  xFract="<<newFractCellDim.x()<<G4endl;
#endif
}
//==================================================================
//
//
//==================================================================
SDHcalBarrel::~SDHcalBarrel()
{
  G4int i;
  for( i = 0; i < MAX_HALF_STAVES; i++) {
    if(StavesRotationMatrices[i] != 0) delete StavesRotationMatrices[i];
    if(InverseStavesRotationMatrices[i] != 0) delete InverseStavesRotationMatrices[i];
  }

  for( i = 0; i < MAX_DOUBLE_LAYERS; i++){
    if(Layers[i] != 0) delete Layers[i];
    if(DimFractCellPerLayer[i] != 0) delete DimFractCellPerLayer[i];
  }

  delete theEncoder;
}

//==================================================================
//
//
//==================================================================
void SDHcalBarrel::SetStaveRotationMatrix(G4int staveNumber, G4double phirot)
{
  stringstream tempString;//temporary string
  tempString << staveNumber;
  if (staveNumber > MAX_HALF_STAVES || staveNumber <= 0)
    G4Exception("\n SDHcalBarrel::SetStaveRotationMatrix(), invalid staveNumber="+ tempString.str());

  if(StavesRotationMatrices [staveNumber-1] !=0) 
    delete StavesRotationMatrices [staveNumber-1];

  if(InverseStavesRotationMatrices [staveNumber-1] !=0) 
    delete  InverseStavesRotationMatrices [staveNumber-1];

  StavesRotationMatrices [staveNumber-1] = new G4RotationMatrix();  
  StavesRotationMatrices [staveNumber-1]->rotateZ(-phirot);
  InverseStavesRotationMatrices [staveNumber-1] = 
    new G4RotationMatrix(StavesRotationMatrices [staveNumber-1]->inverse());

  StavesPhirots[staveNumber-1] = phirot;
#ifdef SDHcalBarrel_DEBUG
  G4cout<<"staveNumber="<<staveNumber<<"  phirot="<<StavesPhirots[staveNumber-1]<<G4endl;
#endif

}
//==================================================================
//
//
//==================================================================
void SDHcalBarrel::AddLayer(G4int layerNumber, G4double X,G4double Y,G4double Z)
{
  stringstream tempString;//temporary string
  tempString << layerNumber;

  if (layerNumber > MAX_DOUBLE_LAYERS || layerNumber <= 0)
    G4Exception("SDHcalBarre::AddLayer(), invalid layer number=" + tempString.str());

  if (Layers[layerNumber-1] != 0)
    G4Exception("SDHcalBarrel::AddLayer(), LayerRef already filled...");

  //build new LayerRef
  Layers[layerNumber-1] = new LayerRef(X,Y,Z);

  //count the number of HCAL layers
  countHcalLayers++;
}


//==================================================================
//
//
//==================================================================
G4ThreeVector SDHcalBarrel::GetCellCenter(G4int, 
					  G4int stave_id, G4int module_id,
					  G4int I, G4int J, G4int K) 
{
  stringstream tempString;//temporary string
  
  //stave_id must always be > 0 and <= MAX_HALF_STAVES
  //stave_id = {1, 2, 3,...16}, i.e. indices for HCAL half staves
  if (stave_id <= 0){
   tempString << stave_id;
   G4Exception("SDHcalBarrel::GetCellCenter(), invalid stave_id=" + tempString.str());
  }
  if (stave_id > MAX_HALF_STAVES)
    G4Exception("SDHcalBarrel::GetCellCenter(), invalid stave_id > MAX_HALF_STAVES");

  //Maximum number of modules for the HCAL: 7
  //module 0 and 6 in the endcaps
  //module 1 to 5 in the barrel;
  //in the current model: the barrel contains only 2 modules
  if (module_id < 0){
    tempString << module_id;
    G4Exception("SDHcalBarrel::GetCellCenter(), invalid module_id=" + tempString.str());
  }
  if (module_id > MAX_MODULES)
    G4Exception("SDHcalBarrel::GetCellCenter(), invalid module_id > MAX_MODULES");
  
  //cell index I must be always >= 0
  if (I < 0) G4Exception("SDHcalBarrel::GetCellCenter(), invalid negativ cell index I..."); 
  //cell index J must be always >= 0
  if (J < 0) G4Exception("SDHcalBarrel::GetCellCenter(), invalid negativ cell index J..."); 
  //cell index K must be >= 1 and <= MAX_DOUBLE_LAYERS
  if (K <= 0 || K > MAX_DOUBLE_LAYERS) 
    G4Exception("SDHcalBarrel::GetCellCenter(), invalid cell index K...");



  //===================================================================
  //Prepare to calculate the position of the cell center, in the local
  //coordinates system: special treatment due to fractional cells at the edges
  //For the first cell: i=0
  //For the last cell: temp/xLayer = 1, so the rest, given by fmod(), is zero !!
  G4double temp = ((I - 1) *DimIntCell(0) + 2 *DimFractCellPerLayer[K-1]->x());
  G4double xlayer = abs(Layers[K-1]->X0);//don't care about the sign (negative or positive)

  G4double tempXoffset = 0; //temporary x-offset
  if (stave_id % 2 != 0) //left layer
    tempXoffset = (Layers[K-1]->X0 - theXoffset);
  else //right layer
    tempXoffset = theXoffset;

  G4ThreeVector localCellCenter;

  if(module_id >=1 && module_id <=5) { //barrel
    if (I == 0) 
      {//cell 0 (first cell)
	localCellCenter[0] = tempXoffset + DimFractCellPerLayer[K-1]->x()/2.;
      }
    else if (I >= 1 && fmod(xlayer, temp) != 0) 
      {//all other cells, except the last one
	localCellCenter[0] = tempXoffset
	  + DimFractCellPerLayer[K-1]->x() + (I-1)*DimIntCell(0) + DimIntCell(0)/2.;
      }
    else if (fmod(xlayer, temp) == 0)
      {//the last cell, which is a fractional one
	localCellCenter[0] = tempXoffset + DimFractCellPerLayer[K-1]->x() 
	  + (I-1)*DimIntCell(0) + DimFractCellPerLayer[K-1]->x()/2;
      }

    //in all cases:
    localCellCenter[1] = Layers[K-1]->Y0;
    localCellCenter[2] = Layers[K-1]->Z0 + J*DimIntCell(2) + DimIntCell(2)/2;

  }
  
  if (localCellCenter[0] <= theXoffset && localCellCenter[0] >= - theXoffset){
    G4Exception("SDHcalBarrel::GetCellCenter(), invalid local x-position...");
  }

#ifdef SDHcalBarrel_DEBUG
  G4cout<<"     stave_id="<<stave_id<<"  module_id="<<module_id <<" I="<<I<<" J="<<J<<" K="<<K
        <<"  localCellCenter="<<localCellCenter<<G4endl;
  G4cout<<"  localCellCenter.x()="<<localCellCenter.x()/mm<<G4endl;
#endif

  if (InverseStavesRotationMatrices[stave_id - 1] == NULL)
    G4Exception("SDHcalBarrel::GetCellCenter(), sorry, empty InverseStavesRotationMatrices...");

  //Find out the actual cell center coodinates (in the global coordinates system):
  G4ThreeVector theCellCenter = *InverseStavesRotationMatrices[stave_id-1] * localCellCenter;
  theCellCenter[2] += *SD::ModulesZOffsets[module_id];

  return theCellCenter;
}

//==================================================================
//
//
//==================================================================
void SDHcalBarrel::DecodeStaveModuleID(G4int moduleCopyNumber, 
				       G4int &theSDPiece, G4int &theStave, G4int &theModule)
{
  theSDPiece = moduleCopyNumber/100;
  theStave  = (moduleCopyNumber - theSDPiece*100)/10;
  theModule = (moduleCopyNumber - theSDPiece*100) % 10;
}

//==================================================================
//
//
//==================================================================
void SDHcalBarrel::RedefineLayerID(G4int oldLayer, G4int countHcalLayers, G4int &newLayer)
{
  if ( (oldLayer < (countHcalLayers/2))
       || (oldLayer > (countHcalLayers/2) && oldLayer < countHcalLayers)
       )
    newLayer = oldLayer % (countHcalLayers/2);
  else if (oldLayer == (countHcalLayers/2)) newLayer = oldLayer;
  else if (oldLayer == countHcalLayers) newLayer = countHcalLayers/2;
}
//==================================================================
//
//
//==================================================================
G4bool SDHcalBarrel::ProcessHits(G4Step *aStep, G4TouchableHistory *)
{
  //process only if energy>0, except neutrinos
  G4double energyDeposition = aStep->GetTotalEnergyDeposit();
  if (energyDeposition <= 0 
      && aStep->GetTrack()->GetDefinition()->GetParticleType() != "geantino")
    return true;
  //----------------------------------------------------------
  G4double time = aStep->GetTrack()->GetGlobalTime();
  
  //the layer number is the volume copy number
  //this is the physical layer number, i.e. <= 2*MAX_LAYERS
  G4int theLayer = aStep->GetTrack()->GetVolume()->GetCopyNo();

  //hit will be deposited in the middle of the step
  //(dependence of the step width) =>  will be global position of the hit
  //(position in the world coordinate system)
  G4ThreeVector thePosition = ((aStep->GetPreStepPoint()->GetPosition())
			       + (aStep->GetPostStepPoint()->GetPosition())) * 0.5;

#ifdef SDHcalBarrel_DEBUG
  G4cout<<"\n\n\n SDHcalBarrel::ProcessHits()  theLayer="<<theLayer<<G4endl;
  G4cout<<"         thePosition="<<thePosition<<G4endl;
  G4cout<<"         current volume="<<(aStep->GetPreStepPoint()->GetPhysicalVolume()->GetName()) <<G4endl;
  G4cout<<"         current material="<<(aStep->GetPreStepPoint()->GetMaterial()->GetName())<<G4endl;
  //aStep->GetPreStepPoint()->GetPhysicalVolume()->GetMotherLogical()->GetSolid()->DumpInfo();
#endif

  //----------------------------------------------------------
  //find out the stave and module id looking for the module copy number
  //and decoding it;
  //(the encoding was done in SHca03::BarrelRegularModules(), MyPlacement(HCALBARREL...))
  G4TouchableHandle theTouchable = aStep->GetPreStepPoint()->GetTouchableHandle();
  G4int moduleCopyNumber = theTouchable->GetReplicaNumber(2);

  //---------------------------------------
  //Decode the moduleCopyNumber to find out the stave and module id's
  G4int theSDPiece, thePhysicalStave, theModule;
  this->DecodeStaveModuleID(moduleCopyNumber, theSDPiece, thePhysicalStave, theModule);
  //------------------------------------------------------------
#ifdef SDHcalBarrel_DEBUG
  G4cout<<"  -->moduleCopyNumber="<<moduleCopyNumber<<endl;
  G4cout<<"     thePhysicalStave="<<thePhysicalStave<<"  theModule="<<theModule<<endl;
  G4cout<<"     countHcalLayers="<<countHcalLayers<<G4endl;
#endif
  //------------------------------------------------------------
  //need theStave to indicate if the hit is in the right or the left side
  //of a physical stave
  G4int theStave;
  if (theLayer <= (countHcalLayers/2)) theStave = 2*thePhysicalStave - 1;
  else theStave = 2*thePhysicalStave;

#ifdef SDHcalBarrel_DEBUG
  G4cout<<"  theStave="<<theStave<<G4endl;
  G4cout<<"  phirot="<<StavesPhirots[theStave-1]<<G4endl;
#endif

  //find out local position in the standard module reference
  if (StavesRotationMatrices[theStave-1] == NULL)
    G4Exception("SDHcalBarrel::ProcessHits(), sorry, empty StavesRotationMatrices...");
  G4ThreeVector localPosition = *StavesRotationMatrices[theStave-1] * thePosition;
#ifdef SDHcalBarrel_DEBUG
  G4cout<<"   localPosition="<<localPosition<<endl;
#endif

  //----------------------------------------
  //calculate I, J
  if (Layers[theLayer-1] == NULL)
    G4Exception("SDHcalBarrel::ProcessHits(), sorry, no layer coordinates in Layer[theLayer-1]...");
  if (SD::ModulesZOffsets[theModule] == NULL)
    G4Exception("SDHcalBarrel::ProcessHits(), sorry, no modules z offsets saved...");
  
#ifdef SDHcalBarrel_DEBUG
  G4cout<<"  Layers[theLayer-1]->X0="<<Layers[theLayer-1]->X0<<G4endl;
  G4cout<<"  Layers[theLayer-1]->Y0="<<Layers[theLayer-1]->Y0<<G4endl;
  G4cout<<"  Layers[theLayer-1]->Z0="<<Layers[theLayer-1]->Z0<<G4endl;
  G4cout<<"  DimIntCell="<<DimIntCell<<G4endl;
  G4cout<<"  DimFractCellPerLayer[theLayer]->x()="<<DimFractCellPerLayer[theLayer]->x()<<G4endl;
  G4cout<<"  DimFractCellPerLayer[theLayer]->y()="<<DimFractCellPerLayer[theLayer]->y()<<G4endl;
  G4cout<<"  DimFractCellPerLayer[theLayer]->z()="<<DimFractCellPerLayer[theLayer]->z()<<G4endl;
#endif

  G4int I, J;
  if (theStave % 2 != 0)
    {//left layer
      I = static_cast<G4int>((localPosition(0) - Layers[theLayer-1]->X0 + theXoffset)/DimIntCell(0));
    }
  else //right layer
    {
      I = static_cast<G4int>((localPosition(0) -  theXoffset)/DimIntCell(0));
    }
  //---------------------------------------------------------------------
  //Must have I>=0
  if (I < 0) G4Exception("SDHcalBarrel::ProcessHits() - invalid negativ cell index I, aborting...");

  J = static_cast<G4int>( (localPosition(2) - *SD::ModulesZOffsets[theModule]
			   - Layers[theLayer-1]->Z0) / DimIntCell(2) );

  if (J < 0) G4Exception("SDHcalBarrel::ProcessHits() - invalid negativ cell index J, aborting...");

#ifdef SDHcalBarrel_DEBUG
  G4cout<<"  I="<<I<<"  J="<<J<<G4endl;
#endif
  //------------------------------------------
  //find out the actual cell coordinates
  G4ThreeVector theCellCenter = this->GetCellCenter(theSDPiece, theStave, theModule, I, J, theLayer);
  //-------------------------------------------
  //the PID of the primary particle id in the Pythia file;
  G4int PID = Control::GetControl()->GetPIDForCalHit(aStep);
  //Must have PID != -1
  if (PID == -1) G4Exception("SDHcalBarrel::ProcessHits() - invalid PID=-1, aborting..."); 

  G4int PDG    = aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
  G4bool found = false;
  G4int n_hit  = CalCollection->entries();

#ifdef SDHcalBarrel_DEBUG
  G4cout<<"  theCellCenter="<<theCellCenter<<G4endl;
  G4cout<<"  PID="<<PID<<G4endl;
  G4cout<<"  PDG="<<PDG<<"  n_hit="<<n_hit<<G4endl;
#endif

  //redefine layer id
  G4int newLayer;
  this->RedefineLayerID(theLayer, countHcalLayers, newLayer);
#ifdef SDHcalBarrel_DEBUG
  G4cout<<"  newLayer = "<<newLayer<<G4endl;
#endif

  //cell_ids is a structure containing ID0 and ID1 of the cell (see CGADefs.h)
  cell_ids theCode = theEncoder->encode(theStave, theModule, I, J, newLayer, 0);
  
  //-----------------------------------------------------------------------------
  if (applyBirksLawFlag == true) {
  	G4double attenuatedEnergy = SD::BirkAttenuation(aStep);
	energyDeposition = attenuatedEnergy;
#ifdef SDHcalBarrel_DEBUG
  G4cout <<"   applyBirksLawFlag: "<<applyBirksLawFlag<<G4endl;
  G4cout << "  engyDeposition: " << energyDeposition/keV << " keV"
         << "  response after Birk: "  << attenuatedEnergy/keV << " keV"
	 << G4endl;
#endif
  }

  //-----------------------------------------------------------------------------

  //create a new cell or add the energy to the cell, if it already exists
  for (G4int i_hit = 0; i_hit < n_hit; i_hit++){
    if ((*CalCollection)[i_hit]->testCell(theCode)){
      (*CalCollection)[i_hit]->AddEdep(PID, PDG, energyDeposition, time);
      found = true;
      break;
    }
  }//end loop over i_hit
  
  if (!found) CalCollection->insert(new CalHit(theSDPiece,  //the detector piece number (5 for Hcal barrel)
					       theStave,    //the stave number
					       theModule,   //the module number in stave
					       I,           //the I,J cell coordinates in the cells matrix 
					       J,           //
					       newLayer,    //the Sensitive (scintillator) layer number (>= 1)
					       0,
					       theCellCenter(0),//the position of the cell center in world coordinates
					       theCellCenter(1),
					       theCellCenter(2),
					       energyDeposition,//the total energy deposited in the cell by the PID particle and  its secondaries;
					       PID,             //the PID of the primary particle id in the Pythia file;
					       PDG,             //the PDG (particle type).
					       time,
					       theCode
					       ));
  return true;
}

