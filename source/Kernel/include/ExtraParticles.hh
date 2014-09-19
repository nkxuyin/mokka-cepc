// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// Authors: Tomohiko Tanabe <tomohiko@icepp.s.u-tokyo.ac.jp>
//          Taikan Suehara <suehara@icepp.s.u-tokyo.ac.jp>
//
// $Id: $
// $Name: $

#ifndef ExtraParticles_hh
#define ExtraParticles_hh 1

// geant4
#include "G4VPhysicsConstructor.hh"
#include "G4ProcessManager.hh"
#include "G4Decay.hh"
#include "G4hIonisation.hh"
#include "G4hMultipleScattering.hh"

#include "Control.hh"

class ExtraParticles : public G4VPhysicsConstructor
{
       public:
               ExtraParticles(const G4String& name = "ExtraParticles");
               virtual ~ExtraParticles();
               void ConstructParticle();
               void ConstructProcess();
               static bool FileExists();
       private:
#if ! G4_VERSION_GE( 940 )
	G4Decay _decay; 	 
	G4hIonisation _ionise; 	 
	G4hMultipleScattering _scatter;
#endif
};

#endif

