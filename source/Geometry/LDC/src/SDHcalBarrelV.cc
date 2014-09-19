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
#include "SDHcalBarrelV.hh"
#include "Encoder32Hcal.hh"
#include "G4SDManager.hh"
#include "G4VSolid.hh"

//#define SDHcalBarrelV_DEBUG

SDHcalBarrelV::SDHcalBarrelV(G4double IdimInt, G4double Jdim, G4double cellThickness,
			   G4int Piece, G4String SDname,
			   G4bool applyBirksLaw)
  : SD(IdimInt, Jdim, cellThickness, Piece, SDname)
{
  //initialize the dimensions of the integer cell
  DimIntCell = SD::CellDim;

  //set counter of HCAL layers to 0
  countHcalLayers = 0;
  
  G4int i;
  for(i = 0; i < MAX_STAVES; i++) {
    StavesRotationMatrices[i]        = NULL;
    InverseStavesRotationMatrices[i] = NULL;
    StavesPhirots[i]                 = 0;
  }

  for(i = 0; i < MAX_LAYERS; i++) {
    Layers[i] = 0;
  }

  theEncoder = new Encoder32Hcal();

  applyBirksLawFlag = applyBirksLaw;
#ifdef SDHcalBarrelV_DEBUG
  G4cout<<"MAX_STAVES="<<MAX_STAVES <<" MAX_LAYERS ="<<MAX_LAYERS <<G4endl;
#endif
}

//==================================================================
//
//
//==================================================================
SDHcalBarrelV::~SDHcalBarrelV()
{
  G4int i;
  for( i = 0; i < MAX_STAVES; i++) {
    if(StavesRotationMatrices[i] != 0) delete StavesRotationMatrices[i];
    if(InverseStavesRotationMatrices[i] != 0) delete InverseStavesRotationMatrices[i];
  }

  for( i = 0; i < MAX_LAYERS; i++){
    if(Layers[i] != 0) delete Layers[i];
  }
}

//==================================================================
//
//
//==================================================================
void SDHcalBarrelV::SetStaveRotationMatrix(G4int staveNumber, G4double phirot)
{
  stringstream tempString;//temporary string
  tempString << staveNumber;
  if (staveNumber > MAX_STAVES || staveNumber <= 0)
    G4Exception("\n SDHcalBarrelV::SetStaveRotationMatrix(), invalid staveNumber="+ tempString.str());

  if(StavesRotationMatrices [staveNumber-1] !=0) 
    delete StavesRotationMatrices [staveNumber-1];

  if(InverseStavesRotationMatrices [staveNumber-1] !=0) 
    delete  InverseStavesRotationMatrices [staveNumber-1];

  StavesRotationMatrices [staveNumber-1] = new G4RotationMatrix();  
  StavesRotationMatrices [staveNumber-1]->rotateZ(phirot);
  InverseStavesRotationMatrices [staveNumber-1] = 
    new G4RotationMatrix(StavesRotationMatrices [staveNumber-1]->inverse());

  StavesPhirots[staveNumber-1] = phirot;
#ifdef SDHcalBarrelV_DEBUG
  G4cout<<"staveNumber="<<staveNumber<<"  phirot="<<StavesPhirots[staveNumber-1]<<G4endl;
#endif

}
//==================================================================
//
//
//==================================================================
void SDHcalBarrelV::AddLayer(G4int layerNumber, G4double X,G4double Y,G4double Z)
{
  stringstream tempString;//temporary string
  tempString << layerNumber;

#ifdef SDHcalBarrelV_DEBUG
  G4cout<<"AddLayer: "<<layerNumber <<G4endl;
#endif

  if (layerNumber > MAX_LAYERS || layerNumber <= 0)
    G4Exception("SDHcalBarre::AddLayer(), invalid layer number=" + tempString.str());

  if (Layers[layerNumber-1] != 0)
    G4Exception("SDHcalBarrelV::AddLayer(), LayerRef already filled...");

  //build new LayerRef
  Layers[layerNumber-1] = new LayerRef(X,Y,Z);

  //count the number of HCAL layers
  countHcalLayers++;
}

//==================================================================
//
//
//==================================================================
G4ThreeVector SDHcalBarrelV::GetCellCenter(G4int, 
					  G4int stave_id, G4int module_id,
					  G4int I, G4int J, G4int K) 
{
  stringstream tempString;//temporary string
  
  //stave_id must always be > 0 and <= MAX_STAVES
  //stave_id = {1, 2, 3,...8}, i.e. indices for HCAL staves
  if (stave_id <= 0){
   tempString << stave_id;
   G4Exception("SDHcalBarrelV::GetCellCenter(), invalid stave_id=" + tempString.str());
  }
  if (stave_id > MAX_STAVES)
    G4Exception("SDHcalBarrelV::GetCellCenter(), invalid stave_id > MAX_STAVES");

  //Maximum number of modules for the HCAL: 7
  //module 0 and 6 in the endcaps
  //module 1 to 5 in the barrel;
  //in the Tesla model: the barrel contains only 2 modules
  //in the videau model: the barrel contains 5 modules
  if (module_id < 0){
    tempString << module_id;
    G4Exception("SDHcalBarrelV::GetCellCenter(), invalid module_id=" + tempString.str());
  }
  if (module_id > MAX_MODULES)
    G4Exception("SDHcalBarrelV::GetCellCenter(), invalid module_id > MAX_MODULES");
  
  //cell index I must be always >= 0
  if (I < 0) G4Exception("SDHcalBarrelV::GetCellCenter(), invalid negativ cell index I..."); 
  //cell index J must be always >= 0
  if (J < 0) G4Exception("SDHcalBarrelV::GetCellCenter(), invalid negativ cell index J..."); 
  //cell index K must be >= 1 and <= MAX_LAYERS
  if (K <= 0 || K > MAX_LAYERS) 
    G4Exception("SDHcalBarrelV::GetCellCenter(), invalid cell index K...");



  //===================================================================
  //Prepare to calculate the position of the cell center, in the local
  //coordinates system:
  //For the first cell: i=0

  G4ThreeVector localCellCenter;

  if(module_id >=1 && module_id <=5) { //barrel

	  //in all cases:
	localCellCenter[0] = Layers[K-1]->X0;
    localCellCenter[1] = Layers[K-1]->Y0 + I*DimIntCell(0) + DimIntCell(0)/2;
    localCellCenter[2] = - Layers[K-1]->Z0 + J*DimIntCell(2) + DimIntCell(2)/2;

  }

#ifdef SDHcalBarrelV_DEBUG
  G4cout<<"     stave_id="<<stave_id<<"  module_id="<<module_id <<" I="<<I<<" J="<<J<<" K="<<K
        <<"  localCellCenter="<<localCellCenter<<G4endl;
  G4cout<<"  localCellCenter.x()="<<localCellCenter.x()/mm<<G4endl;
  G4cout<<" *SD::ModulesZOffsets[module_id] "<<*SD::ModulesZOffsets[module_id] <<G4endl;
#endif

  if (InverseStavesRotationMatrices[stave_id-1] == NULL)
  G4Exception("SDHcalBarrelV::GetCellCenter(), sorry, empty InverseStavesRotationMatrices...");

  //Find out the actual cell center coodinates (in the global coordinates system):
  G4ThreeVector theCellCenter = *InverseStavesRotationMatrices[stave_id-1] * localCellCenter;
  theCellCenter[2] += *SD::ModulesZOffsets[module_id];

  return theCellCenter;
}

//==================================================================
//
//
//==================================================================
void SDHcalBarrelV::DecodeStaveModuleID(G4int moduleCopyNumber, 
				       G4int &theSDPiece, G4int &theStave, G4int &theModule)
{
  theStave   = moduleCopyNumber/1000;
  theModule  = (moduleCopyNumber - theStave*1000)/100;
  theSDPiece = (moduleCopyNumber - theStave*1000) % 100;
}


//==================================================================
//
//
//==================================================================
G4bool SDHcalBarrelV::ProcessHits(G4Step *aStep, G4TouchableHistory *)
{
  //process only if energy>0, except neutrinos
  G4double energyDeposition = aStep->GetTotalEnergyDeposit();
  if (energyDeposition <= 0 
      && aStep->GetTrack()->GetDefinition()->GetParticleType() != "geantino")
    return true;
  //----------------------------------------------------------
  G4double time = aStep->GetTrack()->GetGlobalTime();
  
  //the layer number is the volume copy number
  G4int theLayer = aStep->GetTrack()->GetVolume()->GetCopyNo();

  //hit will be deposited in the middle of the step
  //(dependence of the step width) =>  will be global position of the hit
  //(position in the world coordinate system)
  G4ThreeVector thePosition = ((aStep->GetPreStepPoint()->GetPosition())
			       + (aStep->GetPostStepPoint()->GetPosition())) * 0.5;

#ifdef SDHcalBarrelV_DEBUG
  G4cout<<"\n\n\n SDHcalBarrelV::ProcessHits()  theLayer="<<theLayer<<G4endl;
  G4cout<<"         thePosition="<<thePosition<<G4endl;
  G4cout<<"         current volume="<<(aStep->GetPreStepPoint()->GetPhysicalVolume()->GetName()) <<G4endl;
  G4cout<<"         current material="<<(aStep->GetPreStepPoint()->GetMaterial()->GetName())<<G4endl;
  //aStep->GetPreStepPoint()->GetPhysicalVolume()->GetMotherLogical()->GetSolid()->DumpInfo();
#endif

  //----------------------------------------------------------
	// Find out the stave and module id looking for the
	// module copy number and decoding it
	const G4VTouchable *history =aStep->GetPreStepPoint()->GetTouchable();
	
	G4int depth = history->GetHistory()->GetDepth();
	G4int moduleCopyNumber = -1;
	for (G4int idepth = 0; idepth <= depth; idepth++) {
		moduleCopyNumber=history->GetVolume(idepth)->GetCopyNo();
		//if (theLayer==ENDCAP_SD_PLATE_FLAG && idepth==1) theLayer=moduleCopyNumber;
		if(moduleCopyNumber>100) break;
	}
	
  //---------------------------------------
  //Decode the moduleCopyNumber to find out the stave and module id's
  G4int theSDPiece, thePhysicalStave, theModule;
  this->DecodeStaveModuleID(moduleCopyNumber, theSDPiece, thePhysicalStave, theModule);
  //------------------------------------------------------------
#ifdef SDHcalBarrelV_DEBUG
  G4cout<<"  -->moduleCopyNumber="<<moduleCopyNumber<<endl;
  G4cout<<"     thePhysicalStave="<<thePhysicalStave<<"  theModule="<<theModule<<endl;
  G4cout<<"     countHcalLayers="<<countHcalLayers<<G4endl;
#endif
  //------------------------------------------------------------
  //need theStave to indicate if the hit is in the right or the left side
  //of a physical stave
  G4int theStave;
  theStave = thePhysicalStave;
	
#ifdef SDHcalBarrelV_DEBUG
  G4cout<<"  theStave="<<theStave<<G4endl;
  G4cout<<"  phirot="<<StavesPhirots[theStave-1]<<G4endl;
#endif

  //find out local position in the standard module reference
  if (StavesRotationMatrices[theStave-1] == NULL)
    G4Exception("SDHcalBarrelV::ProcessHits(), sorry, empty StavesRotationMatrices...");
  G4ThreeVector localPosition = *StavesRotationMatrices[theStave-1] * thePosition;
#ifdef SDHcalBarrelV_DEBUG
  G4cout<<"   localPosition="<<localPosition<<endl;
#endif

  //----------------------------------------
  //calculate I, J
  if (Layers[theLayer-1] == NULL)
    G4Exception("SDHcalBarrelV::ProcessHits(), sorry, no layer coordinates in Layer[theLayer-1]...");
  if (SD::ModulesZOffsets[theModule] == NULL)
    G4Exception("SDHcalBarrelV::ProcessHits(), sorry, no modules z offsets saved...");
  
#ifdef SDHcalBarrelV_DEBUG
  G4cout<<"  Layers[theLayer-1]->X0="<<Layers[theLayer-1]->X0<<G4endl;
  G4cout<<"  Layers[theLayer-1]->Y0="<<Layers[theLayer-1]->Y0<<G4endl;
  G4cout<<"  Layers[theLayer-1]->Z0="<<Layers[theLayer-1]->Z0<<G4endl;
  G4cout<<"  DimIntCell="<<DimIntCell<<G4endl;
#endif

  G4int I, J;
	
  I = static_cast<G4int>((- Layers[theLayer-1]->Y0 + localPosition(1))/DimIntCell(0));
  //---------------------------------------------------------------------
  //Must have I>=0
  if (I < 0) G4Exception("SDHcalBarrelV::ProcessHits() - invalid negativ cell index I, aborting...");

  J = static_cast<G4int>( (localPosition(2) - *SD::ModulesZOffsets[theModule]
			   + Layers[theLayer-1]->Z0) / DimIntCell(2) );

  if (J < 0) G4Exception("SDHcalBarrelV::ProcessHits() - invalid negativ cell index J, aborting...");

#ifdef SDHcalBarrelV_DEBUG
  G4cout<<"  I="<<I<<"  J="<<J<<G4endl;
#endif
  //------------------------------------------
  //find out the actual cell coordinates
  G4ThreeVector theCellCenter = this->GetCellCenter(theSDPiece, theStave, theModule, I, J, theLayer);
  //-------------------------------------------
  //the PID of the primary particle id in the Pythia file;
  G4int PID = Control::GetControl()->GetPIDForCalHit(aStep);
  //Must have PID != -1
  if (PID == -1) G4Exception("SDHcalBarrelV::ProcessHits() - invalid PID=-1, aborting..."); 

  G4int PDG    = aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
  G4bool found = false;
  G4int n_hit  = CalCollection->entries();

#ifdef SDHcalBarrelV_DEBUG
  G4cout<<"  theCellCenter="<<theCellCenter<<G4endl;
  G4cout<<"  PID="<<PID<<G4endl;
  G4cout<<"  PDG="<<PDG<<"  n_hit="<<n_hit<<G4endl;
#endif

  //cell_ids is a structure containing ID0 and ID1 of the cell (see CGADefs.h)
  cell_ids theCode = theEncoder->encode(theStave, theModule, I, J, theLayer, 0);
  
  //-----------------------------------------------------------------------------
  if (applyBirksLawFlag == true) {
  	G4double attenuatedEnergy = SD::BirkAttenuation(aStep);
	energyDeposition = attenuatedEnergy;
#ifdef SDHcalBarrelV_DEBUG
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
					       theLayer,    //the Sensitive (scintillator) layer number (>= 1)
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

