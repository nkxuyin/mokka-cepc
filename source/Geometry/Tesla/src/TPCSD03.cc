// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: TPCSD03.cc,v 1.2 2009/05/22 17:03:43 steve Exp $
// $Name: mokka-07-00 $

#include "Control.hh"
#include "TPCSD03.hh"

#include "G4Step.hh"
#include "G4SteppingManager.hh"
#include "G4SDManager.hh"
#include "UserTrackInformation.hh"

TPCSD03::TPCSD03(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy):
  VSensitiveDetector(name), fThresholdEnergyDeposit(thresholdEnergyDeposit),
  fThresholdKineticEnergy(thresholdKineticEnergy), 
  fHitCollection(0),
  fSpaceHitCollection(0),
  fLowPtHitCollection(0),
  fHCID(-1),
  fSpaceHitCollectionID(-1),
  fLowPtHitCollectionID(-1)
{
 

  G4String CollName1=name+"Collection";
  collectionName.insert(CollName1);

  G4String CollName2=name+"SpacePointCollection";
  collectionName.insert(CollName2);

  G4String CollName3=name+"LowPtCollection";
  collectionName.insert(CollName3);

}

void TPCSD03::Initialize(G4HCofThisEvent *)
{
  
  fHitCollection = new TRKHitsCollection(SensitiveDetectorName, collectionName[0]);
  fSpaceHitCollection = new TRKHitsCollection(SensitiveDetectorName, collectionName[1]);
  fLowPtHitCollection = new TRKHitsCollection(SensitiveDetectorName, collectionName[2]);

  CumulitiveNumSteps=0;

  dEInPadRow = 0.0;
  CumulitiveEnergyDeposit = 0.0;
  globalTimeAtPadRingCentre=0.0;
  pathLengthInPadRow=0.0;
  CumulitivePathLength=0.0;
  CrossingOfPadRingCentre[0]=0.0;
  CrossingOfPadRingCentre[1]=0.0;
  CrossingOfPadRingCentre[2]=0.0;
  MomentumAtPadRingCentre[0]=0.0;
  MomentumAtPadRingCentre[1]=0.0;
  MomentumAtPadRingCentre[2]=0.0;

  CumulitiveMeanPosition.set(0.0,0.0,0.0);
  CumulitiveMeanMomentum.set(0.0,0.0,0.0);

}

G4bool TPCSD03::ProcessHits(G4Step *step, G4TouchableHistory *)
{

  // FIXME: 
  // (i) in the following algorithm if a particle "appears" within a pad-ring half and 
  // leaves without passing through the middle of the pad-ring it will not create a hit in 
  // this ring
  // (ii) a particle that crosses the boundry between two pad-ring halves will have the hit 
  // placed on this surface at the last crossing point, and will be assinged the total energy 
  // deposited in the whole pad-ring. This is a possible source of bias for the hit

  G4TouchableHandle touchPost = step->GetPostStepPoint()->GetTouchableHandle(); 
  G4TouchableHandle touchPre = step->GetPreStepPoint()->GetTouchableHandle(); 

  //  if (step->GetPreStepPoint()->GetKineticEnergy() < fThresholdKineticEnergy) return true;
  if (step->GetTrack()->GetDefinition()->GetPDGCharge()==0) return true;
  
  const G4ThreeVector PrePosition = step->GetPreStepPoint()->GetPosition();
  const G4ThreeVector PostPosition = step->GetPostStepPoint()->GetPosition();
  const G4ThreeVector Momentum = step->GetPostStepPoint()->GetMomentum();
  
  float ptSQRD = Momentum[0]*Momentum[0]+Momentum[1]*Momentum[1];
  
  if(ptSQRD >= (Control::TPCLowPtCut*Control::TPCLowPtCut)*MeV){
    // Step finishes at a geometric boundry
    if(step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary)
      {
	// step within the same pair of upper and lower pad ring halves
	if(touchPre->GetCopyNumber()==touchPost->GetCopyNumber())
	  {
	    //this step must have ended on the boundry between these two pad ring halfs 
	    //record the tracks coordinates at this position 
	    //and return
	    
	    CrossingOfPadRingCentre = PostPosition;
	    MomentumAtPadRingCentre = Momentum;  
	    dEInPadRow += step->GetTotalEnergyDeposit();
	    globalTimeAtPadRingCentre = step->GetTrack()->GetGlobalTime();
	    pathLengthInPadRow += step->GetStepLength();
	  
	    //	    G4cout << "step must have ended on the boundry between these two pad ring halfs" << G4endl;
	    //	    G4cout << "CrossingOfPadRingCentre = "   
	    //		   << sqrt( CrossingOfPadRingCentre[0]*CrossingOfPadRingCentre[0]
	    //			    +CrossingOfPadRingCentre[1]*CrossingOfPadRingCentre[1] )
	    //		   << G4endl;
	    
	    return true;
	  }
	else if(!(CrossingOfPadRingCentre[0]==0.0 && CrossingOfPadRingCentre[1] ==0.0 && CrossingOfPadRingCentre[2]==0.0))
	  // the above IF statment is to catch the case where the particle "appears" in this pad-row half volume and 
	  // leaves with out crossing the pad-ring centre, as mentioned above
	  {
	    //it is leaving the pad ring couplet
	    //write out a hit
	    //make sure particle is added to MC list
	    //and return
	    //	    G4cout << "step must be leaving the pad ring couplet" << G4endl;
	    //	    G4cout << "write out hit at " 
	    //		   << sqrt( CrossingOfPadRingCentre[0]*CrossingOfPadRingCentre[0]
	    //			    +CrossingOfPadRingCentre[1]*CrossingOfPadRingCentre[1] )
	    //		   << " " << "dEdx = " << step->GetTotalEnergyDeposit()+dEInPadRow 
	    //		   << " " << "step length = " << step->GetStepLength()+pathLengthInPadRow  
	    //		   << G4endl;
	  
	    //	    if ( (step->GetTotalEnergyDeposit()+dEInPadRow) > fThresholdEnergyDeposit) 
	  
	    fHitCollection->
	      insert(new TRKHit(touchPre->GetCopyNumber(), 
				CrossingOfPadRingCentre[0],CrossingOfPadRingCentre[1],CrossingOfPadRingCentre[2],
				MomentumAtPadRingCentre[0]/GeV,
				MomentumAtPadRingCentre[1]/GeV,
				MomentumAtPadRingCentre[2]/GeV,
				Control::primaryId, 
				step->GetTrack()->GetDefinition()->GetPDGEncoding(),
				step->GetTotalEnergyDeposit()+dEInPadRow, 
				globalTimeAtPadRingCentre,
				step->GetStepLength()+pathLengthInPadRow));
	  
	    // fix for delta electrons: all particles causing hits have to be saved in the LCIO file -- PK
	    UserTrackInformation *info = (UserTrackInformation*)(step->GetTrack()->GetUserInformation());
	    if (info) info->GetTheTrackSummary()->SetToBeSaved();
	    // zero cumulative variables 
	    dEInPadRow = 0.0;
	    globalTimeAtPadRingCentre=0.0;
	    pathLengthInPadRow=0.0;
	    CrossingOfPadRingCentre[0]=0.0;
	    CrossingOfPadRingCentre[1]=0.0;
	    CrossingOfPadRingCentre[2]=0.0;
	    MomentumAtPadRingCentre[0]=0.0;
	    MomentumAtPadRingCentre[1]=0.0;
	    MomentumAtPadRingCentre[2]=0.0;
	    return true;
	  }
      }
  
    //case for which the step remains within geometric volume
    //FIXME: need and another IF case to catch particles which Stop within the padring
    else if(step->GetPostStepPoint()->GetStepStatus() != fGeomBoundary)
      {
	//the step is not limited by the step length 
	if(step->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName()!="StepLimiter")
	  {
	    // if(particle not stoped){ 	    
	    //add the dEdx and return 
	    //	    G4cout << "Step ended by Physics Process: Add dEdx and carry on" << G4endl;
	    dEInPadRow += step->GetTotalEnergyDeposit();
	    pathLengthInPadRow += step->GetStepLength();
	    return true;
	    //}
	    //else{
	    //  write out the hit and clear counters
	    //}
	  }
	else
	  {
	  
	  
	    G4double position_x = PostPosition[0];
	    G4double position_y = PostPosition[1];
	    G4double position_z = PostPosition[2];
	  
	    G4ThreeVector PreMomentum = step->GetPreStepPoint()->GetMomentum();
	    G4ThreeVector PostMomentum = step->GetPostStepPoint()->GetMomentum();
	  
	    G4double momentum_x = PostMomentum[0] /GeV;
	    G4double momentum_y = PostMomentum[1] /GeV;
	    G4double momentum_z = PostMomentum[2] /GeV;
	  
	  
	    //	    G4cout << "step must have been stopped by the step limiter" << G4endl;
	    //	    G4cout << "write out hit at " 
	    //		   << sqrt( position_x*position_x
	    //			    +position_y*position_y )
	    //		   << " " << "dEdx = " << step->GetTotalEnergyDeposit() 
	    //		   << " " << "step length = " << step->GetStepLength()  
	    //		   << G4endl;
	  
	    // write out step limited hit 
	    // these are just space point hits so do not save the dE, which is set to ZERO
	    //	    if ( step->GetTotalEnergyDeposit() > fThresholdEnergyDeposit ) 
	    fSpaceHitCollection->
	      insert(new TRKHit(touchPre->GetCopyNumber(), 
				position_x,position_y,position_z,
				momentum_x,momentum_y,momentum_z,
				Control::primaryId, 
				step->GetTrack()->GetDefinition()->GetPDGEncoding(),
				0.0, // dE set to ZERO 
				step->GetTrack()->GetGlobalTime(),
				step->GetStepLength()));
	  
	    // fix for delta electrons: all particles causing hits have to be saved in the LCIO file -- PK
	    UserTrackInformation *info = (UserTrackInformation*)(step->GetTrack()->GetUserInformation());
	    if (info) info->GetTheTrackSummary()->SetToBeSaved();
	    // add dE and pathlegth and return
	    dEInPadRow += step->GetTotalEnergyDeposit();
	    pathLengthInPadRow += step->GetStepLength();
	    return true;
	  }
      }
  }
  
  else{ // low pt tracks will be treated differenly as there step length is limited by the special low pt steplimiter
    
    const G4ThreeVector meanPosition = (step->GetPreStepPoint()->GetPosition() + step->GetPostStepPoint()->GetPosition()) / 2;
    const G4ThreeVector meanMomentum = (step->GetPreStepPoint()->GetMomentum() + step->GetPostStepPoint()->GetMomentum()) / 2;

    ++CumulitiveNumSteps;    
    CumulitiveMeanPosition = ( (CumulitiveMeanPosition*(CumulitiveNumSteps-1)) + meanPosition ) / CumulitiveNumSteps;
    CumulitiveMeanMomentum = ( (CumulitiveMeanMomentum*(CumulitiveNumSteps-1)) + meanMomentum ) / CumulitiveNumSteps;
    CumulitiveEnergyDeposit += step->GetTotalEnergyDeposit();
    CumulitivePathLength += step->GetStepLength();

//    G4cout << "updating Cumulitive counters:" 
//	   << "\n CumulitiveNumSteps      " << CumulitiveNumSteps   
//	   << "\n CumulitiveMeanPosition  " << CumulitiveMeanPosition 
//	   << "\n CumulitiveMeanMomentum  " << CumulitiveMeanMomentum 
//	   << "\n CumulitiveEnergyDeposit " << CumulitiveEnergyDeposit 
//	   << "\n CumulitivePathLength    " << CumulitivePathLength   
//	   << G4endl; 

    if( CumulitivePathLength > Control::TPCLowPtMaxHitSeparation * mm 
        || 
        ( (step->GetPostStepPoint()->GetKineticEnergy() == 0) && (CumulitiveEnergyDeposit > fThresholdEnergyDeposit) ) ) 
      {
	fLowPtHitCollection->
	  insert(new TRKHit(touchPre->GetCopyNumber(), 
			    CumulitiveMeanPosition[0], CumulitiveMeanPosition[1], CumulitiveMeanPosition[2],
			    CumulitiveMeanMomentum[0], CumulitiveMeanMomentum[1], CumulitiveMeanMomentum[2],
			    Control::primaryId, 
			    step->GetTrack()->GetDefinition()->GetPDGEncoding(),
			    CumulitiveEnergyDeposit, 
			    step->GetTrack()->GetGlobalTime(),
			    CumulitivePathLength));
	
	// fix for delta electrons: all particles causing hits have to be saved in the LCIO file -- PK
	if(Control::TPCLowPtStoreMCPForHits){
	  UserTrackInformation *info = (UserTrackInformation*)(step->GetTrack()->GetUserInformation());
	  if (info) info->GetTheTrackSummary()->SetToBeSaved();
	 }
	
      CumulitiveMeanPosition.set(0.0,0.0,0.0);
      CumulitiveMeanMomentum.set(0.0,0.0,0.0);
      CumulitiveNumSteps = 0;
      CumulitiveEnergyDeposit = 0;
      CumulitivePathLength = 0;
    }
    
    return true;
    
  }

  return true;
}


void TPCSD03::EndOfEvent(G4HCofThisEvent *eventHC)
{
  if (fHCID < 0)
    fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  eventHC->AddHitsCollection(fHCID, fHitCollection);
  
  if (fSpaceHitCollectionID < 0)
    fSpaceHitCollectionID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[1]);
  eventHC->AddHitsCollection(fSpaceHitCollectionID, fSpaceHitCollection);

  if (fLowPtHitCollectionID < 0)
    fLowPtHitCollectionID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[2]);
  eventHC->AddHitsCollection(fLowPtHitCollectionID, fLowPtHitCollection);
  
}

void TPCSD03::LoadEvent(FILE *)
{
  Control::Log("TPCSD03: ASCII files are deprecated.");
}
