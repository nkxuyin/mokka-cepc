
// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: ShowerDecomposition.cc,v 0.0 2011/01/31 M. Ramilli $
// Changed to ShowerrDecomposition and work with updated TBSD_VCellXX.hh
// 2012/10/08, C. Guenter, S. Morozov, and S. Lu
// $Name: mokka-07-00 $

#include "Control.hh"
#include "CGADefs.h"
#include "G4SDManager.hh"
#include "G4TouchableHandle.hh"
#include "G4StepPoint.hh"
#include "G4VPhysicalVolume.hh"
#include "G4NavigationHistory.hh"
#include "CGAGeometryManager.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "G4UnitsTable.hh"
#include <G4VSensitiveDetector.hh>
#include <G4VProcess.hh>
#include <G4ProcessType.hh>
#include <IMPL/LCGenericObjectImpl.h>
#include "ShowerDecomposition.hh"

#include "TBSD_VCell03.hh"
#include "TBSD_VCell04.hh"
#include "TBSD_VCell4d.hh"




#define PI0 111
#define ETA 221
#define EMINUS 11
#define EPLUS -11
#define GAMMA 22
#define NEUTRON 2112
#define PROTON 2212
#define ALPHA 1000020040
#define PIPLUS 211
#define PIMINUS -211


INITPLUGIN(ShowerDecomposition, "ShowerDecomposition")


//#define ShowerDecomposition_DEBUG 1
//#define ShowerDecomposition_STEPPING 1
//#define ShowerDecomposition_HADTRACK 1
//#define ShowerDecomposition_G4LogicalVolume_DEBUG 1
//#define Data_DEBUG 1

//----------------------------------------------------------------------------//
void ShowerDecomposition::Init(void)
//----------------------------------------------------------------------------//
{
  //G4EmSaturation implements Birks' law
  emSaturation = new G4EmSaturation();
  
  _detectorModel = 0; 

   G4String dModel = Control::DETECTOR_MODEL.data();
   if (dModel == "TBhcal4d") {
     _detectorModel = 2; //TBSD_VCell4d, TBhcal4d
   }else if (dModel == "TBCern2010") {
     _detectorModel = 1; //TBSD_VCell04, TBCern2010
   }else{
     _detectorModel = 0; //TBSD_VCell03, TBCern0807_p0709
   }
}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void ShowerDecomposition::Exit(void)
//----------------------------------------------------------------------------//
{
}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void ShowerDecomposition::BeginOfRunAction(const G4Run *run)
//----------------------------------------------------------------------------//
{


}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void ShowerDecomposition::EndOfRunAction(const G4Run *run)
//----------------------------------------------------------------------------//
{
}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void ShowerDecomposition::BeginOfEventAction(const G4Event *evt){
  //----------------------------------------------------------------------------//
  //at the begin of the event i just
  //clear all the dummy vectors and maps

  _trackIDs.clear();
  _hitsEnergy.clear();
  _hitsTime.clear();
  _hitsPDG.clear();
  _hitsParentPDG.clear();
  _hitsGrandParentPDG.clear();

  _hitsPosX.clear();
  _hitsPosY.clear();
  _hitsPosZ.clear();

  _hitsPosI.clear();
  _hitsPosJ.clear();
  _hitsPosK.clear();

  _isFEM.clear();
  _isNelastic.clear();
  _isNcapture.clear();
  _isNinelastic.clear();
  _hasNeutronAncestor.clear();

  _map_IDG4Track.clear();
  _map_TrackPDG.clear();
  _map_DaughtParent.clear();

}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void ShowerDecomposition::EndOfEventAction(const G4Event *evt)
//----------------------------------------------------------------------------//
{
  //at the end of the event I store the data saved in the vectors in a collection of LCGenericObjects
    Control* c = Control::GetControl();

    _nProtonsFromFirstInelastic  = nHadronsFromFirstInelastic( PROTON  );
    _nNeutronsFromFirstInelastic = nHadronsFromFirstInelastic( NEUTRON );   
    _nPionsFromFirstInelastic     = nHadronsFromFirstInelastic( PIPLUS ) + nHadronsFromFirstInelastic( PIMINUS ); 

    //set event parameters
#ifdef LCIO_MODE
    if(c->lcWrt && c->lcEvt) 
      {    
	G4int _nHits = (G4int) _hitsEnergy.size();

    //creating a collectino of LCIOGenericObjects
        LCCollection *col = new LCCollectionVec( LCIO::LCGENERICOBJECT );
        for( int i=0; i<_nHits; i++) {//loop on the number of hits (if everthng works fine, all vector should have the same number of elements)
	  //new LCGenericObject
            LCGenericObjectImpl *o = new IMPL::LCGenericObjectImpl;

	    //fill the LCGenericObject with the elements from the vectors:
	    o->setDoubleVal( 0, (double) _hitsEnergy[i]         ); // hit energy deposition
	    o->setDoubleVal( 1, (double) _hitsTime[i]           ); // hit time

	    o->setFloatVal(  0, (float)  _hitsPosX[i]           );
	    o->setFloatVal(  1, (float)  _hitsPosY[i]           );
	    o->setFloatVal(  2, (float)  _hitsPosZ[i]           );

	    o->setIntVal(    0, (int)    _trackIDs[i]           ); // track ID
            o->setIntVal(    1, (int)    _hitsPDG[i]            ); // hit particle PDG
	    o->setIntVal(    2, (int)    _hitsParentPDG[i]      ); // hit parent particle PDG
	    o->setIntVal(    3, (int)    _hitsGrandParentPDG[i] ); // hit grandparent particle PDG
	    o->setIntVal(    4, (int)    _hitsPosI[i]           );
	    o->setIntVal(    5, (int)    _hitsPosJ[i]           );
	    o->setIntVal(    6, (int)    _hitsPosK[i]           );  
	    o->setIntVal(    7, (int)    _isFEM[i]              ); // tag for FEM component
	    o->setIntVal(    8, (int)    _isNelastic[i]         ); // tag for neutron component
	    o->setIntVal(    9, (int)    _isNcapture[i]         ); // tag for neutron component
	    o->setIntVal(   10, (int)    _hasNeutronAncestor[i] ); // tag if particle that does energy deposition has a neutron ancestor 
	    o->setIntVal(   11, (int)    _isNelasticProton[i]   ); // tag for neutron eleastic scattering with hydrogen

	    //o->setIntVal( 10, (int)  _isNinelastic[i] );//tag for neutron component
	    //	    o->setIntVal( 0, (int) _hitsProcessSubType[i] );
            col->addElement( o );
        }

        c->lcEvt->addCollection( col, "ShowerDecomposition" );

	// store also the number of neutrons, protons and pions coming out of the first hard interaction
	c->lcEvt->parameters().setValue( "nProtonsFromFirstInelastic",  _nProtonsFromFirstInelastic  );
	c->lcEvt->parameters().setValue( "nNeutronsFromFirstInelastic", _nNeutronsFromFirstInelastic );
	c->lcEvt->parameters().setValue( "nPionsFromFirstInelastic",    _nPionsFromFirstInelastic    );
      }
#endif

    for(G4int i = 0; i<10; i++){
      _map_IDG4Track[i].PrintParameters();
    }

}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void ShowerDecomposition::PreUserTrackingAction(const G4Track *trk)
//----------------------------------------------------------------------------//
{
  G4int trkID = trk->GetTrackID();
  const G4ParticleDefinition* def = trk->GetDefinition();
  G4int pdgCode=def->GetPDGEncoding();

if(_map_TrackPDG.find(trkID)==_map_TrackPDG.end()){//fill the map

    _map_TrackPDG[trkID]=pdgCode;

  }

    //============== Fill in the G4Track detail information =========



      IDG4TrackMapping _map_data;

      G4int _map_trkID = trk->GetTrackID();
      G4int _map_parentID;
       if( trk->GetTrackID() > 0 ) _map_parentID = trk->GetParentID();
       else _map_parentID = 0;
      G4int _map_pdgCode = trk->GetDefinition()->GetPDGEncoding();
      G4double _map_timeStamp = trk->GetGlobalTime() / ns;// Time since the event in which the track belongs is created.
      G4double _map_kineticEnergy =  trk->GetKineticEnergy() / GeV;
      G4double _map_totalEnergy = trk->GetTotalEnergy() / GeV;

      _map_data.SetTrackID(_map_trkID);
      _map_data.SetParentID(_map_parentID);
      _map_data.SetPDGcode(_map_pdgCode);
      _map_data.SetTimeStamp(_map_timeStamp);
      _map_data.SetKineticEnergy(_map_kineticEnergy);
      _map_data.SetTotalEnergy(_map_totalEnergy);

      _map_data.SetParticleName( trk->GetDefinition()->GetParticleName() );

      G4String lvname = trk->GetVolume()->GetLogicalVolume()->GetName();
      size_t found=  lvname.find_first_of("0123456789"); 
      G4String LogicalVName = lvname.substr(0,found);

      _map_data.SetLogicalVolumeName( LogicalVName );

      bool IsHcalWholeSensLayerLogical = false;
      if ( LogicalVName == "HcalWholeSensLayerLogical")
      IsHcalWholeSensLayerLogical = true;

      _map_data.SetHcalWholeSensLayerLogical( IsHcalWholeSensLayerLogical );

      if( trk->GetTrackID() > 0 ) {      
	if ( trk->GetParentID() ==0 ) _map_data.SetProcessName("ParticleGun");
	else _map_data.SetProcessName( trk->GetCreatorProcess()->GetProcessName() ); // not for first particle
      }
      else{
	_map_data.SetProcessName("");
      }
	_map_IDG4Track[_map_trkID] = _map_data;



      if( trk->GetTrackID()  == 1 && //Force to fill trk "0" with ParticleGun
	  trk->GetParentID() == 0 )
	{
	  _map_data.SetTrackID(0);
	  _map_data.SetParentID(0);
	  _map_data.SetHcalWholeSensLayerLogical(false);
	  _map_IDG4Track[0] = _map_data;
	}


    //============== End: Fill in the G4Track detail information =========


}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void ShowerDecomposition::PostUserTrackingAction(const G4Track *trk)
//----------------------------------------------------------------------------//
{
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//

//inherited from A. Kaplan plugin, never used ...
bool ShowerDecomposition::inEcal( const G4Track *&trk )
//----------------------------------------------------------------------------//
{
    G4VPhysicalVolume  *pv = trk->GetVolume();
    G4LogicalVolume   *mlv = pv->GetMotherLogical();
    
    if( mlv ) {
        G4String mlvname = mlv->GetName();
        if( (mlvname.index("CarbonFiber") == 0) ) {
#ifdef ShowerDecomposition_DEBUG
            G4cout<<" is in ECAL"<<G4endl;
#endif
            return true;
        }
        if( (mlvname.index("StructureLogical") == 0) ) {
#ifdef ShowerDecomposition_DEBUG
            G4cout<<" is in ECAL"<<G4endl;
#endif
            return true;
        }
        if( (mlvname.index("Air") == 0) && mlvname.contains("") ) {
#ifdef ShowerDecomposition_DEBUG
            G4cout<<" is in ECAL"<<G4endl;
#endif
            return true;
        }
    }

#ifdef ShowerDecomposition_DEBUG
    G4cout<<" is not in ECAL"<<G4endl;
#endif
    return false;
}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//

//inherited from A. Kaplan plugin ... never used ...
bool ShowerDecomposition::inTcmt( const G4Track *&trk )
//----------------------------------------------------------------------------//
{
    G4VPhysicalVolume  *pv = trk->GetVolume();
    G4String pvname = pv->GetName();

    if( pvname.index("pv_")==0 ) {
#ifdef ShowerDecomposition_DEBUG
        G4cout<<" is in TCMT"<<G4endl;
#endif
        return true;
    }
    if( pvname.index("Catcher")==0 ) {
#ifdef ShowerDecomposition_DEBUG
        G4cout<<" is in TCMT"<<G4endl;
#endif
        return true;
    }
        
#ifdef ShowerDecomposition_DEBUG
    G4cout<<" is not in TCMT"<<G4endl;
#endif

    return false;
}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//

//checks if the track is passing by the hcal SD
bool ShowerDecomposition::inHcalSD( const G4Track *&trk )
//----------------------------------------------------------------------------//
{
    G4VPhysicalVolume  *pv = trk->GetVolume();
    G4LogicalVolume    *lv = pv->GetLogicalVolume();
    G4LogicalVolume   *mlv = pv->GetMotherLogical();

    
    G4String vname = lv->GetName();

#ifdef ShowerDecomposition_G4LogicalVolume_DEBUG
    G4cout<<" G4LogicalVolume name: " << vname <<G4endl;
#endif

    if( (vname.index("HcalWholeSensLayerLogical") == 0) && (vname(25)!='_') ) {
#ifdef ShowerDecomposition_G4LogicalVolume_DEBUG
        G4cout<<" is in HCAL"<<G4endl;
#endif
        return true;
    }


#ifdef ShowerDecomposition_G4LogicalVolume_DEBUG
    G4cout<<" is not in HCAL"<<G4endl;
#endif

    return false;
}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
//contains all the processes of stornig energy and other data ...
void ShowerDecomposition::UserSteppingAction(const G4Step *step)
//----------------------------------------------------------------------------//
{

  const G4Track *trk = step->GetTrack();
  G4int trkID = trk->GetTrackID();

  G4int parentID = 0;
  G4int GrandparentID = 0;

  if(trkID !=0){
    parentID = trk->GetParentID();

    if(_map_DaughtParent.find(trkID)==_map_DaughtParent.end()){

      _map_DaughtParent[trkID]=parentID;
    }

    if(parentID !=0){
      GrandparentID = _map_DaughtParent.find(parentID)->second;
    }

  }



  G4double edep = 0.;
  edep = step->GetTotalEnergyDeposit();
  if (edep <= 0) return;//if energy deposited is zero, exit

  G4StepPoint *step1 = step->GetPreStepPoint(); 
  G4StepPoint *step2 = step->GetPostStepPoint(); 
  G4TouchableHandle touch1 = step1->GetTouchableHandle(); 

  

  //check energy has been deposited in hcal sensitive detector:
  //warning - this check has been implemented by cross-checking the name of the G4 volume!
  //it may give problems with geometry drivers older than TBhcal07 (iron ahcal) 
  if(inHcalSD(trk)/*to be fully implemented? */){

#ifdef ShowerDecomposition_G4LogicalVolume_DEBUG 
    G4cout<<" ....... ==> save it ...."<<G4endl;
#endif

    G4double time = step->GetTrack()->GetGlobalTime()/ns;
    G4ThreeVector hitPos = (step1->GetPosition()+step2->GetPosition())*0.5;

    G4VPhysicalVolume  *pv = trk->GetVolume();
    G4LogicalVolume    *lv = pv->GetLogicalVolume();
    G4VSensitiveDetector *sdV = lv->GetSensitiveDetector(); //hcalSD

#ifdef ShowerDecomposition_DEBUG
  G4cout << "***** before TBSD_VCell4d *sd " << G4endl;
#endif


    G4double gridDim;
    G4int depthToLayer;

    G4double zBeginDetector;
    G4double hcalTimeCut;
    G4int applyBirksLaw;
    
    G4int ncell_xy[2];

   
    if( _detectorModel == 2){
      
      TBSD_VCell4d *sd = dynamic_cast<TBSD_VCell4d*>(sdV); 
      
      gridDim = sd->GetGridDim();
      depthToLayer = sd->GetDepthToLayer();
      
      zBeginDetector = sd->GetZBeginDetector();
      hcalTimeCut = sd->GetHcalTimeCut();
      applyBirksLaw = sd->GetApplyBirksLaw();
      
      ncell_xy[0] = sd->GetNCellXY()[0];
      ncell_xy[1] = sd->GetNCellXY()[1]; 
      
      
    } else if( _detectorModel == 1){

      TBSD_VCell04 *sd = dynamic_cast<TBSD_VCell04*>(sdV);
      
      gridDim = sd->GetGridDim();
      depthToLayer = sd->GetDepthToLayer();
      
      zBeginDetector = sd->GetZBeginDetector();
      hcalTimeCut = sd->GetHcalTimeCut();
      applyBirksLaw = sd->GetApplyBirksLaw();
      
      ncell_xy[0] = sd->GetNCellXY()[0];
      ncell_xy[1] = sd->GetNCellXY()[1]; 
 
    }else{

      TBSD_VCell03 *sd = dynamic_cast<TBSD_VCell03*>(sdV);
      
      gridDim = sd->GetGridDim();
      depthToLayer = sd->GetDepthToLayer();
      
      zBeginDetector = sd->GetZBeginDetector();
      hcalTimeCut = sd->GetHcalTimeCut();
      applyBirksLaw = sd->GetApplyBirksLaw();
      
      ncell_xy[0] = sd->GetNCellXY()[0];
      ncell_xy[1] = sd->GetNCellXY()[1]; 
      
    }

  /***********************************************************/
    if (applyBirksLaw != 0)
      {
	G4double attenuatedEnergy = this->GetBirksAttenuatedEnergy(step);
	edep = attenuatedEnergy;
      }
 /***********************************************************/

    G4double visEdep = edep;


  /******************************************************************/
  if (hcalTimeCut > 0 && zBeginDetector != 0)
    {
      G4EventManager* pEventManager= G4EventManager::GetEventManager();
      const G4Event *event = pEventManager->GetConstCurrentEvent();
      //position of primary vertex (i.e. of particle gun...)
      G4double zPrimaryGenerator = event->GetPrimaryVertex()->GetZ0();
      
      G4double timeOutsideDetector = (zBeginDetector - zPrimaryGenerator)/c_light;
      G4double globalTime = step->GetTrack()->GetGlobalTime();
      G4double timeInsideDetector = (globalTime - timeOutsideDetector)*ns;

      //do not accept hits in HCAL which are later than Hcal_time_cut
      if (timeInsideDetector > hcalTimeCut) return;
    }
  /******************************************************************/


     
    // face dimension
    G4double xDim, yDim;
    xDim = (G4double) ncell_xy[0] * gridDim;
    yDim = (G4double) ncell_xy[1] * gridDim;

 // DEBUG: print parameters
#ifdef ShowerDecomposition_DEBUG
  G4cout << "n_cell_x " << ncell_xy[0] << G4endl;
  G4cout << "n_cell_y " << ncell_xy[1] << G4endl;
  G4cout << "gridDim " << gridDim << G4endl;
  G4cout << "depthToLayer " << depthToLayer << G4endl;
  G4cout << "cal xDim " << xDim << G4endl;
  G4cout << "cal yDim " << yDim << G4endl << G4endl;
#endif

  // get Layer Number from Touchable based on depthToLayer
  G4int n_lay = touch1->GetHistory()->GetVolume(depthToLayer)->GetCopyNo();

#ifdef ShowerDecomposition_DEBUG
  G4cout << "Layer: " << n_lay << G4endl;
#endif

  // origin point from transformations = 0, 0, 0
  G4ThreeVector origin;
  origin = G4ThreeVector();

  // get global Volume center of active layer
  G4ThreeVector GlobalVolumeCenter = touch1->GetHistory()->GetTopTransform().Inverse().TransformPoint(origin);  

  // get GlobalHitPosition
  G4ThreeVector theGlobalPos = step1->GetPosition();

  // get local hit position using touchable with theGlobalPos
  G4ThreeVector LocalHitPos = touch1->GetHistory()->GetTopTransform().TransformPoint(theGlobalPos);

  // compute "natural" x and y bins of the local hit
  G4int xbin = int(floor(LocalHitPos.x() / gridDim ));
  G4int ybin = int(floor(LocalHitPos.y() / gridDim ));

#ifdef ShowerDecomposition_DEBUG
  G4cout << "xbin " << xbin << G4endl;
  G4cout << "ybin " << ybin << G4endl;
#endif

  // compute x and y local coord. from natural bins
  G4double localXPos = (double(xbin) + .5) * gridDim;
  G4double localYPos = (double(ybin) + .5) * gridDim;

  // set localCellPos from coord. calc
  G4ThreeVector localCellPos(localXPos, localYPos, 0.);

  // compute GlobalCellPos based on touchable with localCellPos
  G4ThreeVector GlobalCellPos = touch1->GetHistory()->GetTopTransform().Inverse().TransformPoint(localCellPos);

  // DEBUG: result should be within gridSize
#ifdef ShowerDecomposition_DEBUG
  G4ThreeVector checkPos = GlobalCellPos - theGlobalPos;
  G4cout << "checkPos=GlobalCellPos-theGlobalPos <" << checkPos << ">" << G4endl;
#endif

#ifdef ShowerDecomposition_DEBUG
  G4cout << "GlobalCellPos " << GlobalCellPos << G4endl;
  G4cout << "GlobalVolumeCenter " << GlobalVolumeCenter << G4endl;
  G4cout << "LocalHitPos " << LocalHitPos << G4endl;
#endif

  // use the local coords to calculate the i, j cell IDs
  // SetCellID( localXPos, localYPos);
  // CRP Calculate i_x, j_y position of virtual cell from the
  //     location of the edep and the geometrical parameters
  //     stored in the db, ncell_xz and the grid size

  G4int cellI =  (G4int) floor( ncell_xy[0] * ( localXPos  + xDim / 2. )
                              /  xDim ) ;   
  G4int cellJ =  (G4int) floor( ncell_xy[1] * ( localYPos  + yDim / 2. ) 
                              /  yDim ) ;
    
#ifdef ShowerDecomposition_DEBUG
  if ( (cellI > 89) || (cellJ > 89)) G4cout << "WARNING" << G4endl;
  G4cout << "localPosX = " << localXPos << G4endl; 
  G4cout << "xDim = " << xDim << G4endl; 
  G4cout << "cellID[0] = " << cellI << G4endl; 
  G4cout << "localPosY = " << localYPos << G4endl; 
  G4cout << "yDim = " << yDim << G4endl; 
  G4cout << "cellID[1] = " << cellJ << G4endl; 
#endif

    //store quantities:
    _trackIDs.push_back(trkID);//track ID of the particle depositing energy
    _hitsEnergy.push_back(visEdep/GeV);//deposited energy
    _hitsPosX.push_back(hitPos[0]);//position X
    _hitsPosY.push_back(hitPos[1]);//position Y
    _hitsPosZ.push_back(hitPos[2]);//position Z
    _hitsTime.push_back(time);//time from beginning of event

    _hitsPosI.push_back(cellI + 1);//position I
    _hitsPosJ.push_back(cellJ + 1);//position J
    _hitsPosK.push_back(n_lay);//position K

    G4int trckPDG = _map_TrackPDG.find(trkID)->second;// finding track PDG
    _hitsPDG.push_back(trckPDG);//track PDG

    
    //initializing quantities for the following loop
    G4int looptrkID = trkID;
    G4int loopPtrkID = 0;
    
  
    G4int isFEM(0); //initializing default value for flag 
    G4int isNel(0);
    G4int isNcap(0);
    G4int isNelProton(0);
    G4int hasNeutronAncestor(0);

    G4int particleName;
    G4int parentName;
    G4String processName;

    looptrkID = trkID;
    loopPtrkID = 0;

    G4int oriTrkPDG = _map_IDG4Track.find(trkID)->second.GetPDGcode();

    G4int isEmCandidate(0); // help variable for the "break" statement in the loop
    if(oriTrkPDG == EPLUS || oriTrkPDG ==EMINUS || oriTrkPDG==GAMMA) isEmCandidate = 1; 

    while(looptrkID > 0){

      loopPtrkID   = _map_IDG4Track.find(looptrkID)  ->second.GetParentID();    // get parent track 
      particleName = _map_IDG4Track.find(looptrkID)  ->second.GetPDGcode();     // get particle PDG code
      parentName   = _map_IDG4Track.find(loopPtrkID) ->second.GetPDGcode();     // get parent particle PDG code
      processName  = _map_IDG4Track.find(looptrkID)  ->second.GetProcessName(); // get process creating current track
      

      if(isEmCandidate == 1) {
	if( parentName == PI0 || parentName == ETA ){
	  if(trckPDG == EPLUS || trckPDG ==EMINUS || trckPDG==GAMMA){//check if energy has been deposited by e+/e-	
  
	    isFEM = 1;
	    break;
	  }
	}
      }
   
      else if(parentName==NEUTRON) {

	if(processName=="hadElastic")                         isNel  = 1;
	if(processName=="nCapture")                           isNcap = 1;
	if(processName=="hadElastic" && particleName==PROTON) isNelProton = 1;

	hasNeutronAncestor = 1;
	break;
      }
      looptrkID = loopPtrkID;  

    }
    _isFEM              .push_back( isFEM              ); // store EM flag
    _isNelastic         .push_back( isNel              ); // store neutron flags
    _isNcapture         .push_back( isNcap             );
    _isNelasticProton   .push_back( isNelProton        );
    _hasNeutronAncestor .push_back( hasNeutronAncestor );



    //storing parent and grandparent PDG codes

    if(trkID !=0){//check if the track is primary (no parent track ...)
      _hitsParentPDG.push_back(_map_TrackPDG.find(parentID)->second);

      //       _hitsProcessSubType.push_back(_map_TrackSubProcess.find(trkID)->second);
    }else{
      _hitsParentPDG.push_back(_map_TrackPDG.find(0)->second);//if particle is primary, parent PDG is primary PDG
      //       _hitsProcessSubType.push_back(0);
    }

    if(parentID !=0){

      _hitsGrandParentPDG.push_back(_map_TrackPDG.find(GrandparentID)->second);

    }else{
      _hitsGrandParentPDG.push_back(_map_TrackPDG.find(0)->second );
    }

  }




  //=========== Use data information ==========
  
    G4int currentParentID = -1;
    G4int lastParentID = -1;

    if( step->GetTrack() > 0 ) {

	G4int tID = step->GetTrack()->GetTrackID();
	G4int pID = step->GetTrack()->GetParentID();

#ifdef Data_DEBUG
	G4cout <<" --test --> trackID: "
	       << tID
	       <<" -- parentID: "
	       << pID
	       << G4endl;
#endif

	if ( pID > 0 ) {
	  G4int grandParentID =  _map_IDG4Track[parentID].GetParentID();
	    
	  if ( _map_IDG4Track[grandParentID].GetPDGcode() == 2112 //grand  parent is Neutron
	       && step->GetTrack()->GetDefinition()->GetPDGEncoding() == 22 //11   //This step is electron e-
	       && _map_IDG4Track[tID].IsHcalWholeSensLayerLogical() == 1 ) //This step, the e- track is in Sensitive logical volume
	    {
	      currentParentID = grandParentID;
	      
	      if( currentParentID != lastParentID)
		{
		  lastParentID = currentParentID;
		  G4cout <<"\n -------------- Next Neutron ---------------"<<G4endl;
		  _map_IDG4Track[currentParentID].PrintParameters();
		  G4cout <<" ------- Sub Sub Tracks from this Neutron ------"<<G4endl;
		}
		
	      _map_IDG4Track[pID].PrintParameters();
	      _map_IDG4Track[tID].PrintParameters();
	  
	      /*	  
	      unsigned i=0;
	      for( i=_lastindex; i<children.size(); ++i ) {
		G4Track* c = children[i];
		const G4VProcess* p = c->GetCreatorProcess(); 
		
		G4cout <<"  trk "<<c->GetTrackID()
		       <<", pdgcode=="<<c->GetDefinition()->GetPDGEncoding()
		       <<", E_tot=="<<c->GetTotalEnergy()/GeV
		       <<", cProc=="<<p->GetProcessName()
		       <<", type=="<<p->GetProcessType()
		       <<", sub=="<<p->GetProcessSubType()
		       <<", z=="<<c->GetPosition().z() / mm<<" mm"
		       <<G4endl; 

	      }
	      */
	    }
	}
    }
  
    //=========== End: Use data information ==========



}
//----------------------------------------------------------------------------//

G4double ShowerDecomposition::GetBirksAttenuatedEnergy(const G4Step* aStep)
{
  G4double energyDeposition = aStep->GetTotalEnergyDeposit();
  G4double length = aStep->GetStepLength();
  G4double niel = 0.; //aStep->GetNonIonisingEnergyDeposit(); //FIXME
  //G4double niel = aStep->GetNonIonisingEnergyDeposit(); //FIXME
  //G4cout<<"\n\n niel="<<niel<<G4endl;
  const G4Track* track = aStep->GetTrack();
  const G4ParticleDefinition* particle = track->GetDefinition();
  const G4MaterialCutsCouple* couple = track->GetMaterialCutsCouple();

  G4double engyVis = emSaturation->VisibleEnergyDeposition(particle,
                                                           couple,
                                                           length,
                                                           energyDeposition,
                                                           niel);
  return engyVis;
}


G4int ShowerDecomposition::nHadronsFromFirstInelastic( const G4int hadronPDG ) {

  // create temp vector to store primary particles daughters
  std::vector<G4int> primDaughtersTrkID;
  primDaughtersTrkID.resize(0);

  G4int trkID_MCPart = 0; // gun particle always has trkID 0

  // find the daughters of the primary (gun) particle and save them to vector
   for( std::map<G4int, G4int>::const_iterator it = _map_DaughtParent.begin();
  it != _map_DaughtParent.end(); it++) {

    // check if daughters come from inelastic scattering
    if( _map_IDG4Track.find( it->second )->second.GetProcessName() == "hadInelastic" ) {

      // check if inelastic scattering happens in absorber material
      G4String lvname = _map_IDG4Track.find( it->second )->second.GetLogicalVolumeName();
      size_t found=  lvname.find_first_of("0123456789"); 
      G4String LogicalVName = lvname.substr(0,found);
      if( LogicalVName == "HcalAbsLayerLogical" ) {

	// check if is the particle ID we are looking for
	if( it->second == trkID_MCPart) {
	  
	  primDaughtersTrkID.push_back( it->first );
	}
      }
    }
  }
  
  // count the daughters with the asked PDG ID and give out the number
  G4int number(0);
  for( uint i(0); i < primDaughtersTrkID.size(); i++) {
    
    G4int daughtersPDG = _map_TrackPDG.find( primDaughtersTrkID.at(i) ) ->second;

    if( daughtersPDG == hadronPDG) number++;    
  }

  return number;

}
