// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: TPCSD04.cc,v 1.2 2009/05/22 17:03:43 steve Exp $
// $Name: mokka-07-00 $

#include "Control.hh"
#include "TPCSD04.hh"

#include "G4Step.hh"
#include "G4SteppingManager.hh"
#include "G4SDManager.hh"
#include "UserTrackInformation.hh"

TPCSD04::TPCSD04(G4String name, G4double thresholdEnergyDeposit):
VSensitiveDetector(name), 
fThresholdEnergyDeposit(thresholdEnergyDeposit),
fHitCollection(0),
fSpaceHitCollection(0),
fLowPtHitCollection(0),
fHCID(-1),
fSpaceHitCollectionID(-1),
fLowPtHitCollectionID(-1),
CurrentTrackID(-1)
{
  
  
  G4String CollName1=name+"Collection";
  collectionName.insert(CollName1);
  
  G4String CollName2=name+"SpacePointCollection";
  collectionName.insert(CollName2);
  
  G4String CollName3=name+"LowPtCollection";
  collectionName.insert(CollName3);
  
  if( Control::TPCLowPtStepLimit ) {
    G4cout << "TPCSD04: TPCLowPtStepLimit ON" << G4endl;
  }
  else {
    G4cout << "TPCSD04: TPCLowPtStepLimit OFF" << G4endl;
  }
  
  if( Control::TPCLowPtStoreMCPForHits ) {
    G4cout << "TPCSD04: TPCLowPtStoreMCPForHits ON" << G4endl;
  }
  else {
    G4cout << "TPCSD04: TPCLowPtStoreMCPForHits OFF" << G4endl;
  }
  
  G4cout << "TPCSD04: Threshold for Energy Deposit = " << fThresholdEnergyDeposit / eV << " eV" << G4endl;
  G4cout << "TPCSD04: TPCLowPtCut = " << Control::TPCLowPtCut / MeV << " MeV "<< G4endl;
  G4cout << "TPCSD04: TPCLowPt Max hit separation "<< Control::TPCLowPtMaxHitSeparation / mm << " mm" << G4endl;
  
}

void TPCSD04::Initialize(G4HCofThisEvent *)
{
  
  fHitCollection = new TRKHitsCollection(SensitiveDetectorName, collectionName[0]);
  fSpaceHitCollection = new TRKHitsCollection(SensitiveDetectorName, collectionName[1]);
  fLowPtHitCollection = new TRKHitsCollection(SensitiveDetectorName, collectionName[2]);
  
  CumulativeNumSteps=0;
  
  dEInPadRow = 0.0;
  CumulativeEnergyDeposit = 0.0;
  globalTimeAtPadRingCentre=0.0;
  pathLengthInPadRow=0.0;
  CumulativePathLength=0.0;
  CrossingOfPadRingCentre[0]=0.0;
  CrossingOfPadRingCentre[1]=0.0;
  CrossingOfPadRingCentre[2]=0.0;
  MomentumAtPadRingCentre[0]=0.0;
  MomentumAtPadRingCentre[1]=0.0;
  MomentumAtPadRingCentre[2]=0.0;
  
  CumulativeMeanPosition.set(0.0,0.0,0.0);
  CumulativeMeanMomentum.set(0.0,0.0,0.0);
  
  PreviousPostStepPosition.set(0.0,0.0,0.0);
  CurrentPDGEncoding=0;
  CurrentTrackID=-1;
  CurrentGlobalTime=0.0;
  CurrentCopyNumber=0;
  
}

G4bool TPCSD04::ProcessHits(G4Step *step, G4TouchableHistory *)
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
  
  
  if (fabs(step->GetTrack()->GetDefinition()->GetPDGCharge()) < 0.01) return true;
  
  const G4ThreeVector PrePosition = step->GetPreStepPoint()->GetPosition();
  const G4ThreeVector PostPosition = step->GetPostStepPoint()->GetPosition();
  const G4ThreeVector Momentum = step->GetPostStepPoint()->GetMomentum();
  
  float ptSQRD = Momentum[0]*Momentum[0]+Momentum[1]*Momentum[1];
  
  if( ptSQRD >= (Control::TPCLowPtCut*Control::TPCLowPtCut) ){
    // Step finishes at a geometric boundry

    if(step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) {
      // step within the same pair of upper and lower pad ring halves
    
      if(touchPre->GetCopyNumber()==touchPost->GetCopyNumber()) {
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
      else if(!(CrossingOfPadRingCentre[0]==0.0 && CrossingOfPadRingCentre[1]==0.0 && CrossingOfPadRingCentre[2]==0.0)) {
        // the above IF statment is to catch the case where the particle "appears" in this pad-row half volume and 
        // leaves with out crossing the pad-ring centre, as mentioned above
        
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
        
        G4double dE = step->GetTotalEnergyDeposit()+dEInPadRow;
        
        if ( dE > fThresholdEnergyDeposit || Control::TrackingPhysicsListELossOn == false ) {
          

          // needed for delta electrons: all particles causing hits have to be saved in the LCIO file
          UserTrackInformation *info = (UserTrackInformation*)(step->GetTrack()->GetUserInformation());
          if (info) info->GetTheTrackSummary()->SetToBeSaved();

          fHitCollection->
          insert(new TRKHit(touchPre->GetCopyNumber(), 
                            CrossingOfPadRingCentre[0],CrossingOfPadRingCentre[1],CrossingOfPadRingCentre[2],
                            MomentumAtPadRingCentre[0],
                            MomentumAtPadRingCentre[1],
                            MomentumAtPadRingCentre[2],
                            info->GetTheTrackSummary()->GetTrackID(),
                            step->GetTrack()->GetDefinition()->GetPDGEncoding(),
                            dE, 
                            globalTimeAtPadRingCentre,
                            step->GetStepLength()+pathLengthInPadRow));

          
        }
        
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
    else if(step->GetPostStepPoint()->GetStepStatus() != fGeomBoundary) {
      //the step is not limited by the step length 
      
      if(step->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName()!="StepLimiter"){
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
      else {
        
        
        G4double position_x = PostPosition[0];
        G4double position_y = PostPosition[1];
        G4double position_z = PostPosition[2];
        
        G4ThreeVector PreMomentum = step->GetPreStepPoint()->GetMomentum();
        G4ThreeVector PostMomentum = step->GetPostStepPoint()->GetMomentum();
        
        G4double momentum_x = PostMomentum[0];
        G4double momentum_y = PostMomentum[1];
        G4double momentum_z = PostMomentum[2];
        
        
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

        // needed for delta electrons: all particles causing hits have to be saved in the LCIO file
        UserTrackInformation *info = (UserTrackInformation*)(step->GetTrack()->GetUserInformation());
        if (info) info->GetTheTrackSummary()->SetToBeSaved();
        
        fSpaceHitCollection->
	      insert(new TRKHit(touchPre->GetCopyNumber(), 
                          position_x,position_y,position_z,
                          momentum_x,momentum_y,momentum_z,
                          info->GetTheTrackSummary()->GetTrackID(), 
                          step->GetTrack()->GetDefinition()->GetPDGEncoding(),
                          0.0, // dE set to ZERO 
                          step->GetTrack()->GetGlobalTime(),
                          step->GetStepLength()));

        // add dE and pathlegth and return
        dEInPadRow += step->GetTotalEnergyDeposit();
        pathLengthInPadRow += step->GetStepLength();
        return true;
      }
    }
  }
  
  else if (Control::TPCLowPtStepLimit) { // low pt tracks will be treated differently as their step length is limited by the special low pt steplimiter
    
    if ( ( PreviousPostStepPosition - step->GetPreStepPoint()->GetPosition() ).mag() > 1.0e-6 * mm ) {
      
      // This step does not continue the previous path. Deposit the energy and begin a new Pt hit.
      
      if (CumulativeEnergyDeposit > fThresholdEnergyDeposit) {
        DepositLowPtHit();
      }
      
      else {
        // reset the cumulative variables if the hit has not been deposited.
        // The previous track has ended and the cumulated energy left at the end 
        // was not enough to ionize
        //G4cout << "reset due to new track , discarding " << CumulativeEnergyDeposit / eV << " eV" << std::endl;
        ResetCumulativeVariables();
      }

    }

    else {
      //G4cout << "continuing track" << endl;
    }
    
    CumulateLowPtStep(step);  

    
    // check whether to deposit the hit
    if( ( CumulativePathLength > Control::TPCLowPtMaxHitSeparation )  ) {
      
      // hit is deposited because the step limit is reached and there is enough energy
      // to ionize
      
      if ( CumulativeEnergyDeposit > fThresholdEnergyDeposit) {
        DepositLowPtHit();
      }
      //else {
      //G4cout << "not deposited, energy is " << CumulativeEnergyDeposit/eV << " eV" << std::endl;
      //}
    }
    else { // even if the step lenth has not been reached the hit might
           // be deposited because the particle track ends

      if ( step->GetPostStepPoint()->GetKineticEnergy() == 0 ) {
        
            // only deposit the hit if the energy is high enough
        if (CumulativeEnergyDeposit > fThresholdEnergyDeposit) {
          
          DepositLowPtHit();
        }
        
        else { // energy is not enoug to ionize.
               // However, the track has ended and the energy is discarded and not added to the next step
               //G4cout << "reset due to end of track, discarding " << CumulativeEnergyDeposit/eV << " eV" << std::endl;
          ResetCumulativeVariables();
        }
      }
    }
    
    PreviousPostStepPosition = step->GetPostStepPoint()->GetPosition();    
    
    return true;
    
  }

  return true;

}

void TPCSD04::EndOfEvent(G4HCofThisEvent *eventHC)
{
  
  // There might be one low Pt hit which has not been added to the collection.
  if (CumulativeEnergyDeposit > fThresholdEnergyDeposit) {
    DepositLowPtHit(); 
  }  
  
  if (fHCID < 0) {
    fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  }
  
  eventHC->AddHitsCollection(fHCID, fHitCollection);
  
 
  if (fSpaceHitCollectionID < 0) {
    fSpaceHitCollectionID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[1]);
  }
  
  eventHC->AddHitsCollection(fSpaceHitCollectionID, fSpaceHitCollection);
  
  if (fLowPtHitCollectionID < 0) {
    fLowPtHitCollectionID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[2]);
  }
  
  eventHC->AddHitsCollection(fLowPtHitCollectionID, fLowPtHitCollection);
  
}

void TPCSD04::LoadEvent(FILE *)
{
  Control::Log("TPCSD04: ASCII files are deprecated.");
}

void TPCSD04::DepositLowPtHit()
{
  fLowPtHitCollection->
  insert(new TRKHit(CurrentCopyNumber, 
                    CumulativeMeanPosition[0], CumulativeMeanPosition[1], CumulativeMeanPosition[2],
                    CumulativeMeanMomentum[0], CumulativeMeanMomentum[1], CumulativeMeanMomentum[2],
                    CurrentTrackID, 
                    CurrentPDGEncoding,
                    CumulativeEnergyDeposit, 
                    CurrentGlobalTime,
                    CumulativePathLength));
  
  // reset the cumulative variables after positioning the hit
  ResetCumulativeVariables();
}

void TPCSD04::ResetCumulativeVariables()
{
  CumulativeMeanPosition.set(0.0,0.0,0.0);
  CumulativeMeanMomentum.set(0.0,0.0,0.0);
  CumulativeNumSteps = 0;
  CumulativeEnergyDeposit = 0;
  CumulativePathLength = 0;
}

void TPCSD04::CumulateLowPtStep(G4Step *step)
{
  
  const G4ThreeVector meanPosition = (step->GetPreStepPoint()->GetPosition() + step->GetPostStepPoint()->GetPosition()) / 2;
  const G4ThreeVector meanMomentum = (step->GetPreStepPoint()->GetMomentum() + step->GetPostStepPoint()->GetMomentum()) / 2;
  
  ++CumulativeNumSteps;    
  CumulativeMeanPosition = ( (CumulativeMeanPosition*(CumulativeNumSteps-1)) + meanPosition ) / CumulativeNumSteps;
  CumulativeMeanMomentum = ( (CumulativeMeanMomentum*(CumulativeNumSteps-1)) + meanMomentum ) / CumulativeNumSteps;
  CumulativeEnergyDeposit += step->GetTotalEnergyDeposit();
  CumulativePathLength += step->GetStepLength();
  CurrentPDGEncoding = step->GetTrack()->GetDefinition()->GetPDGEncoding();
  CurrentTrackID = step->GetTrack()->GetTrackID();
  CurrentGlobalTime = step->GetTrack()->GetGlobalTime();
  CurrentCopyNumber = step->GetPreStepPoint()->GetTouchableHandle()->GetCopyNumber();
  
  // needed for delta electrons: all particles causing hits have to be saved in the LCIO file 
  if ( CumulativeEnergyDeposit > fThresholdEnergyDeposit ) {
    // This low Pt hit will eventually be saved, so set the flag to store the particle
    
    // writing the MC Particles can be turned on or off for the lowPt particles
    if(Control::TPCLowPtStoreMCPForHits) {
      UserTrackInformation *info = (UserTrackInformation*)(step->GetTrack()->GetUserInformation());
      if (info) info->GetTheTrackSummary()->SetToBeSaved();
    }
  }
}

