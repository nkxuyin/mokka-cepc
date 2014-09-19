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
// $Id: TrackSummary.cc,v 1.21 2009/04/30 08:12:53 mora Exp $
// $Name: mokka-07-00 $
//
// 

#include "TrackSummary.hh"
#include "Control.hh"
#include "CGAGeometryManager.hh"
#include "UserInit.hh"

#include "G4Track.hh"
#include "G4DecayProducts.hh"
#include "G4VProcess.hh"

G4Allocator<TrackSummary> TrackSummaryAllocator;

TrackSummary::TrackSummary(const G4Track* aTrack,
			   G4bool ToBeSaved,
			   G4bool BackScattering) 
  : parent(NULL), toBeSaved(ToBeSaved), backScattering(BackScattering)
#ifdef LCIO_MODE
    , theMCParticle(NULL)
    , mcParticleIsUpToDate(false)
#endif
{
  charge = aTrack->GetDefinition()->GetPDGCharge();
  mass = aTrack->GetDynamicParticle()->GetMass();

// Geant4 has a strange behavior when filling G4Track in 
// the beginning of Track. So the energy, momentum and vertex 
// information are stored by SteppingAction and filled later by
// TrackSummary::update (see below)

  PDG = aTrack->GetDefinition()->GetPDGEncoding();

  trackID = aTrack->GetTrackID();
  parentTrackID = aTrack->GetParentID();
}

void 
TrackSummary::update (const G4Track* aTrack)
{
  // Turn around to Geant4 strange behavior
  // when filling G4Track in the begin of Track. 
  // Vertex info filled by SteppingAction and used here.

  // AZ: In case of back scattering track, Control
  // has wrong information about vertex

  if(!GetBackScattering()){
    vertex = Control::VertexPosition;
    energy = Control::VertexEnergy;
    momentum = Control::VertexMomentum;
  }
  // update final track information
  endPoint = aTrack->GetPosition();
  trackLength = aTrack->GetTrackLength();
  
  // hepEvtStatus == generator status in LCIO
  hepEvtStatus = 0;
  if(parentTrackID == 0) 
    {
      const G4DecayProducts* thePreAssignedDecayProducts=
	aTrack->GetDynamicParticle()->GetPreAssignedDecayProducts();
      if(thePreAssignedDecayProducts && 
	 thePreAssignedDecayProducts->entries()>0)
	hepEvtStatus = 2;
      else
	hepEvtStatus = 1;
    }

#ifdef LCIO_MODE
  
  // Setting the simulator status as for LCIO, by the moment by hand
  // just adapting the beta LCIO code.
  // Remarks: 
  //   - with Mokka BITVertexIsNotEndpointOfParent is always false 
  //     because the endpoint is set always explicitly.
  //   - BITDecayedInTracker and BITDecayedInCalorimeter not
  //     set, waiting for better understand about these bits.
  //
  _simstatus = 0;

  // endpoint always set explicitly by Mokka.
  _simstatus[ MCParticle::BITEndpoint ] = 1;

  // these folowwing pointers should always be not NULL but
  // with Geant4 we never know...
  //
  const G4Step* theLastStep = aTrack->GetStep();
  G4StepPoint* theLastPostStepPoint = NULL;
  if(theLastStep) 
    theLastPostStepPoint = theLastStep->GetPostStepPoint();
  
  // BITCreatedInSimulation  and BITBackscatter has no sense except 
  // if created by simulator.
  if(hepEvtStatus == 0)
    {
      //fg: BITCreatedInSimulation should only be set if MCParticle has been created
      //    by Mokka/geant4 - now done in BuildMCParticle
      // _simstatus[ BITCreatedInSimulation ] = 1; 
      if(GetBackScattering()) _simstatus[ MCParticle::BITBackscatter ] = 1;
    }

  // BITLeftDetector
  G4bool LeftDetector = false;
  //

  if(theLastPostStepPoint)
    //
    //   Thanks to Predrag Krstonosic (krstonos@ifh.de) we realize that Geant4 never 
    // sets the fWorldBoundary step status even when the particle is leaving the detector 
    // world volume. As a turn around fix while Geant4 does't work correctly we 
    // replaced the test
    //
    // LeftDetector = (theLastPostStepPoint->GetStepStatus() == fWorldBoundary))
    //
    //     by 
    //
    // LeftDetector = (theLastPostStepPoint->GetStepStatus() == fGeomBoundary))
    //
    // in the TrackSummary::update() method. We suppose that normaly a track doens't 
    // finish in a Geometry boundary except if it's leaving the detector world volume,
    // so it should works nice for the moment.
    //
    if((LeftDetector = (theLastPostStepPoint->GetStepStatus() == fGeomBoundary)))
      _simstatus[ MCParticle::BITLeftDetector ] = 1;

  // BITStopped
  if(aTrack->GetKineticEnergy() <= 0.)
    _simstatus[ MCParticle::BITStopped  ] = 1;

  // BITDecayedInTracker
  CGAGeometryManager* theCGAGeometryManager =
    CGAGeometryManager::GetCGAGeometryManager();
  
  G4bool InsideTrackerRegion =
    theCGAGeometryManager->
    GetTrackerRegionRmax() > endPoint.perp() 
    &&
    theCGAGeometryManager->
    GetTrackerRegionZmax() > std::fabs(endPoint.z());
  
  if(InsideTrackerRegion)
    _simstatus[ MCParticle::BITDecayedInTracker ] = 1;

  // BITDecayedInCalorimeter
  if(!InsideTrackerRegion && !LeftDetector)
    _simstatus[ MCParticle::BITDecayedInCalorimeter ] = 1;
  
  // BITVertexIsNotEndpointOfParent
  //
  TrackSummary* myParent = GetMyParent();
  if(myParent)
    if(myParent->GetEndPoint().isNear(GetVertex()))
      _simstatus[ MCParticle::BITVertexIsNotEndpointOfParent ] = 1;  
#endif
}


void 
TrackSummary::SetParentToBeSaved()
{
#ifdef LCIO_MODE
   if( !  (theMCParticle && mcParticleIsUpToDate)  ) 
     BuildMCParticle();
   else
     // In LCIO mode, if the theMCParticle exists we have already
     // passed by here.
     return;
#endif
  TrackSummary* myParent =
    GetMyParent();  
  if(myParent)
    {

      // std::cout << " **** TrackSummary::SetParentToBeSaved for trackID: " << myParent->GetTrackID() << " this trackid: " << GetTrackID() << std::endl ;

      myParent->toBeSaved=true;
      myParent->SetParentToBeSaved();
#ifdef LCIO_MODE
      if(Control::lcWrt)
	{  

	  MCParticle* theParentMCParticle  = myParent->GetMCParticle()  ;
	  
	  // fg: need to check if particle already has this parent assigned ...
	  // this will be fixed in a future (>1.6) LCIO version
	  const MCParticleVec& parents = theMCParticle->getParents() ;

	  if(  std::find(  parents.begin(), parents.end(),  
			   theParentMCParticle ) == parents.end() ) {

	    // fix B.Vormwald:  only add parent if particlke created in simulation 
	    if (theMCParticle->getGeneratorStatus()== 0){
	      theMCParticle->addParent( theParentMCParticle );
	    }
	  }
// 	  else{
// 	    std::cout << "  >>>>> parent already exists in MCParticle ! " << std::endl;
// 	  }

	}
#endif      
    }
}

TrackSummary* 
TrackSummary::GetMyParent() const
{
  TrackSummary* aTrackSummary = NULL;
  if(parentTrackID == 0) return aTrackSummary;
  
  TrackSummaryVector* trackedtracks =
    Control::GetControl()->GetTrackedtracks();

  G4int itr;
  for(itr=trackedtracks->size()-1;itr>=0;itr--)
    {
      aTrackSummary=(*trackedtracks)[itr];
      if(aTrackSummary && 
	 aTrackSummary->GetTrackID()==parentTrackID)
	break;
    }
  return aTrackSummary;
}

#ifdef LCIO_MODE
void
TrackSummary::BuildMCParticle() 
{

  //  static bool writeCompleteHepEvt =  UserInit::getInstance()->getBool("WriteCompleteHepEvt") ;

  
//   std::cout << " TrackSummary::BuildMCParticle " 
// 	    <<  " pdg: "       << GetPDG() 
// 	    <<  " gen stat: "  << GetHepEvtStatus() 
// 	    <<  " sim stat: "  << GetSimulatorStatus() 
// 	    <<  " trackID: "   << GetTrackID() 
// 	    <<  " isUpdated: " << isUpdate
// 	    << std::endl ;


  // if we have an MCParticle already we just need to update it 
  // otherwise we need to create one ...
  if( theMCParticle == 0 ) { 

    theMCParticle = new MCParticleImpl;
    
    _simstatus[ MCParticle::BITCreatedInSimulation ] = 1 ;    // created in simulation

    if( Control::LCIOWriteCompleteHepEvt ){ // fg: enforce particles created by the simulator to have generator status 0 !!! 

      theMCParticle->setGeneratorStatus( 0 ); 

    } else {
      
      theMCParticle->setGeneratorStatus(GetHepEvtStatus());
    }

    theMCParticle->setPDG(GetPDG());
  }

  theMCParticle->setCharge( GetCharge() );

  //
  double endpoint[3];
  endpoint[0]=GetEndPoint()(0);
  endpoint[1]=GetEndPoint()(1);
  endpoint[2]=GetEndPoint()(2);

  theMCParticle->setEndpoint(endpoint);
  //  theMCParticle->setEnergy(GetEnergy());

  theMCParticle->setMass( mass  / GeV);

  theMCParticle->setSimulatorStatus(GetSimulatorStatus());
  
  float p[3];
  G4ThreeVector theMomentum = 
    GetMomentum();
  p[0]=theMomentum(0) / GeV;
  p[1]=theMomentum(1) / GeV;
  p[2]=theMomentum(2) / GeV;
  theMCParticle->setMomentum(p);
  //
  double vertex[3];
  vertex[0]=GetVertex()(0);
  vertex[1]=GetVertex()(1);
  vertex[2]=GetVertex()(2);
  theMCParticle->setVertex(vertex);



  mcParticleIsUpToDate = true ;
}
#endif      
