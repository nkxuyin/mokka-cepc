//
// $Id: TPCStepLimiterLowPt.hh

//
// class description
//
// A "process" to be registered to the process manager of each particle,
// in the UserPhysicsList, in order to take into account the MaxAllowedStep
// defined by the steering parameter the G4UserLimits object attached to a logical volume.
//
// ------------------------------------------------------------
//                  12 May. 2009  Steve Aplin
// ------------------------------------------------------------
#ifndef TPCStepLimiterLowPt_h
#define TPCStepLimiterLowPt_h 1

#include "G4ios.hh"
#include "globals.hh"
#include "G4VProcess.hh"

class TPCStepLimiterLowPt : public G4VProcess 
{
  public:  // with description     

     TPCStepLimiterLowPt(const G4String& processName ="StepLimiterLowPt" );

     virtual ~TPCStepLimiterLowPt();

     virtual G4double PostStepGetPhysicalInteractionLength(
                             const G4Track& track,
                             G4double   previousStepSize,
                             G4ForceCondition* condition
                            );

     virtual G4VParticleChange* PostStepDoIt(
                             const G4Track& ,
                             const G4Step& 
                            );
                            
  public:  // without description 
                                 
     //  no operation in  AtRestGPIL
     virtual G4double AtRestGetPhysicalInteractionLength(
                             const G4Track& ,
                             G4ForceCondition* 
                            ){ return -1.0; };
                            
     //  no operation in  AtRestDoIt      
     virtual G4VParticleChange* AtRestDoIt(
                             const G4Track& ,
                             const G4Step&
                            ){return 0;};

     //  no operation in  AlongStepGPIL
     virtual G4double AlongStepGetPhysicalInteractionLength(
                             const G4Track&,
                             G4double  ,
                             G4double  ,
                             G4double& ,
                             G4GPILSelection*
                            ){ return -1.0; };

     //  no operation in  AlongStepDoIt
     virtual G4VParticleChange* AlongStepDoIt(
                             const G4Track& ,
                             const G4Step& 
                            ) {return 0;};

  private:
  
  // hide assignment operator as private 
      TPCStepLimiterLowPt(TPCStepLimiterLowPt&);
      TPCStepLimiterLowPt& operator=(const TPCStepLimiterLowPt& right);

};

#endif










