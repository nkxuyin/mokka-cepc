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
// Authors: Tomohiko Tanabe <tomohiko@icepp.s.u-tokyo.ac.jp>
//          Taikan Suehara <suehara@icepp.s.u-tokyo.ac.jp>
//
// $Id: $
// $Name: $

#include "ExtraParticles.hh"

#include "Control.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"

#include <fstream>
#include <sstream>

ExtraParticles::ExtraParticles(const G4String& name)
       : G4VPhysicsConstructor(name)
{
}

ExtraParticles::~ExtraParticles() {
}

bool ExtraParticles::FileExists() {
       std::ifstream pdgFile( Control::PDGFile, std::ifstream::in );
       return pdgFile.is_open();
}

void ExtraParticles::ConstructParticle() {
       if (!Control::PDGFile) return;

       G4ParticleTable *theParticleTable = G4ParticleTable::GetParticleTable();
       std::ifstream pdgFile( Control::PDGFile, std::ifstream::in );

       if (!pdgFile.is_open()) {
               G4cout << "Could not open PDGFile: " << Control::PDGFile << "\n";
               return;
       }

       while ( !pdgFile.eof() ) {
               // read line
               std::string linebuf;
               getline( pdgFile, linebuf );

               // ignore comments
               if (linebuf.substr(0,1) == "#") continue;
               if (linebuf.substr(0,2) == "//") continue;

               // ignore empty lines
               if (linebuf.empty()) continue;

               // parse line
               int pdg;
               std::string name;
               double charge;
               double mass;
               double width;
               double lifetime;

               std::istringstream istr(linebuf);

               istr >> pdg >> name >> charge >> mass >> width >> lifetime;

               // don't add particles that don't fly
               // if (lifetime == 0) continue;

               if(width<0) width = 0;

               // normalize to G4 units
               mass *= GeV;

               if (charge != 0) {
                       charge /= 3.;
               }

               if (lifetime > 0) {
                       lifetime = lifetime*mm/c_light;
               }

               if (width == 0 && lifetime > 0) {
                       width = hbar_Planck/lifetime;
               }

               // don't add if the particle already exists
               G4ParticleDefinition* theParticle = theParticleTable->FindParticle(pdg);
               if (!theParticle) {

                       if (abs(pdg)>80 && abs(pdg)<=100) {
                               // don't add generator-specific particles
                       } else {
                               /*
                               if (pdg==5122) {
                                       G4cout << "Lambda_b0: " << "PDG=" << pdg << ", name=" << name << ", chrg=" << charge
                                               << ", mass=" << mass << ", width=" << width << ", lifetime=" << lifetime << "\n";
                                       G4cout << "debug: mass=" << 5.62 << ", width =" << 1.39e-12/6.582e-16 << ", lifetime=" << 1.39e-12 << "\n";
                               }
                               //*/
                               theParticle = new G4ParticleDefinition(
                                               name,       // name
                                               mass,       // mass
                                               width,      // width
                                               charge,     // charge
                                               0,                                      // 2*spin
                                               0,          // parity
                                               0,          // C-conjugation
                                               0,          // 2*isospin
                                               0,          // 2*isospin3
                                               0,          // G-parity
                                               "extra",    // type
                                               0,          // lepton number
                                               0,          // baryon number
                                               pdg,        // PDG encoding
                                               width==0?true:false,      // stable
                                               lifetime,   // lifetime
                                               NULL,       // decay table
                                               false);      // short lived
                       }
               }
       }

       G4cout << "Loaded extra particles using file: " << Control::PDGFile << G4endl;
}

void ExtraParticles::ConstructProcess() {
       theParticleIterator->reset();
       while((*theParticleIterator)()) {
               G4ParticleDefinition* pdef = theParticleIterator->value();
               G4ProcessManager* pmgr = pdef->GetProcessManager();
               if (pdef->GetParticleType() == "extra") {
                       if (pdef->GetPDGCharge() != 0) {
#if ! G4_VERSION_GE( 940 )
	pmgr->AddProcess(&_scatter, -1,  1, 1); // multiple scattering
	pmgr->AddProcess(&_ionise,  -1,  2, 2); // ionisation
	pmgr->AddProcess(&_decay,   -1, -1, 2); // decay
#else
	pmgr->AddProcess(new G4hMultipleScattering(), -1,  1, 1); // multiple scattering
	pmgr->AddProcess(new G4hIonisation(),  -1,  2, 2); // ionisation
	pmgr->AddProcess(new G4Decay(),   -1, -1, 2); // decay 
#endif

                       } else {

#if ! G4_VERSION_GE( 940 )
//	pmgr->AddProcess(&_scatter, -1,  1, 1); // multiple scattering
	pmgr->AddProcess(&_decay,   -1, -1, 2); // decay
#else
//	pmgr->AddProcess(new G4hMultipleScattering(), -1,  1, 1); // multiple scattering
	pmgr->AddProcess(new G4Decay(),   -1, -1, 2); // decay 
#endif

                       }
               }
       }
}

