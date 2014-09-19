// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: PhysicsListUserLimits.cc,v 1.3 2009/05/13 08:22:57 steve Exp $
// $Name: mokka-07-00 $

#include "PhysicsListUserLimits.hh"

#include "G4UserSpecialCuts.hh"
#include "G4StepLimiter.hh"
#include "TPCStepLimiterLowPt.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"
#include "Control.hh"

void PhysicsListUserLimits::Enable(void)
{
  // cf. Geant 4 HyperNews, Forum "Physics List", Message 129
  // http://geant4-hn.slac.stanford.edu:5090/HyperNews/public/get/phys-list/129.html

  G4UserSpecialCuts *specialCuts = new G4UserSpecialCuts;
  G4StepLimiter     *stepLimiter = new G4StepLimiter;
  TPCStepLimiterLowPt     *tpcStepLimiterLowPt = new TPCStepLimiterLowPt;

  G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
  G4ParticleTable::G4PTblDicIterator *particleIterator = particleTable->GetIterator();
  // make sure you have called "G4RunManager::Initialize()" before

  particleIterator->reset();
  while ((*particleIterator)()) {
  // iterate through all known particles

    G4ParticleDefinition *particleDefinition = particleIterator->value();
    G4ProcessManager *processManager = particleDefinition->GetProcessManager();

    if (processManager && !particleDefinition->IsShortLived() && particleDefinition->GetPDGCharge() != 0) {
    // the process manager should exist, but we don't need to limit short-lived particles or neutrals

      processManager->AddDiscreteProcess(stepLimiter);
      processManager->AddDiscreteProcess(specialCuts);
      // these transportation-related processes are always applicable

      // if TPCStepLimiterLowPt set in steering file
      if( Control::TPCLowPtStepLimit ) {
        processManager->AddDiscreteProcess(tpcStepLimiterLowPt);
      }
    }
  }
}
