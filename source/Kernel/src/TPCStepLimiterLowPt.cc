//
// $Id: TPCStepLimiterLowPt.cc,v 1.1 2009/05/13 08:22:57 steve Exp $
// $Name: mokka-07-00 $   
// --------------------------------------------------------------
// History
// 
// 06-05-09 Initial version, to limit the step length of very low Pt tracks in the TPC based on G4StepLimiter (Steve Aplin)
// --------------------------------------------------------------

#include "TPCStepLimiterLowPt.hh"

#include "G4Step.hh"
#include "G4UserLimits.hh"
#include "G4VParticleChange.hh"
#include "Control.hh"


////////////////////////////////////
TPCStepLimiterLowPt::TPCStepLimiterLowPt(const G4String& aName)
  : G4VProcess(aName)
{
   if (verboseLevel>1) {
     G4cout << GetProcessName() << " is created "<< G4endl;
   }
}


////////////////////////
TPCStepLimiterLowPt::~TPCStepLimiterLowPt()
{
}


////////////////////////
TPCStepLimiterLowPt::TPCStepLimiterLowPt(TPCStepLimiterLowPt& right)
  : G4VProcess(right)
{
}

 
////////////////
G4double 
  TPCStepLimiterLowPt::PostStepGetPhysicalInteractionLength( 
				       const G4Track& aTrack,
				       G4double, // previousStepSize
				       G4ForceCondition* condition  )
{
  // condition is set to "Not Forced"
  *condition = NotForced;
   G4double proposedStep = DBL_MAX;
   G4UserLimits* pUserLimits =
                 aTrack.GetVolume()->GetLogicalVolume()->GetUserLimits();

   if ( pUserLimits && aTrack.GetDefinition()->GetPDGCharge()!=0 
	&& 
	( aTrack.GetVolume()->GetLogicalVolume()->GetName() =="TPC_upperlayer_log" 
	  || 
	  aTrack.GetVolume()->GetLogicalVolume()->GetName() =="TPC_lowerlayer_log" ) 
	) {

     // max allowed step length
     //
     const G4ThreeVector Momentum = aTrack.GetMomentum();
     float ptSQRD = Momentum[0]*Momentum[0]+Momentum[1]*Momentum[1];
     
     if( ptSQRD < (Control::TPCLowPtCut * Control::TPCLowPtCut)){
       proposedStep = Control::TPCLowPtMaxStepLength ;
     }
       if (proposedStep < 0.) proposedStep = 0.; 
   }
   return proposedStep;
}

///////////////
G4VParticleChange*
  TPCStepLimiterLowPt::PostStepDoIt( const G4Track& aTrack,
                                 const G4Step&  )
// Do Nothing
//
{
   aParticleChange.Initialize(aTrack);
   return &aParticleChange;
}













