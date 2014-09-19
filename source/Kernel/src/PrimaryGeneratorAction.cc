// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: PrimaryGeneratorAction.cc,v 1.17 2008/07/03 14:35:26 musat Exp $
// $Name: mokka-07-00 $

#include "PrimaryGeneratorAction.hh"
#include "PrimaryGeneratorMessenger.hh"

#include "ParticleGunGenerator.hh"
#include "InputFileGenerator.hh"
#include "GPSGenerator.hh"
#include "Control.hh"

#include "G4Event.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include "G4ParticleDefinition.hh"


#ifdef LCIO_MODE
#include "HepLCIOInterfaceNew.hh"
#include "EVENT/LCCollection.h"
#include "IMPL/MCParticleImpl.h"

#endif


PrimaryGeneratorAction::PrimaryGeneratorAction(void)
{
  fMessenger = new PrimaryGeneratorMessenger(this);
  fPrimaryGenerator = new ParticleGunGenerator();
}

PrimaryGeneratorAction::~PrimaryGeneratorAction(void)
{
  if (fPrimaryGenerator) delete fPrimaryGenerator;
  delete fMessenger;
}

void PrimaryGeneratorAction::SetGeneratorWithName(G4String generatorName)
{
  if (fPrimaryGenerator) // First dispose of the old generator 
	delete fPrimaryGenerator; 

  VPrimaryGenerator *newGenerator = 0;

  if(generatorName == "particleGun") 
	newGenerator = new ParticleGunGenerator();
  else if(generatorName == "gps") 
	newGenerator = new GPSGenerator();
  else {// Generator from input file 
	newGenerator = new InputFileGenerator(generatorName);
  }
  fPrimaryGenerator = newGenerator; 
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event *evt)
{
  if (fPrimaryGenerator)
    fPrimaryGenerator->GeneratePrimaryVertex(evt); 

  // the Lorentz boost is now applied already to the MCParticles in HepLCIOInterfaceNew 
  // else we have to apply the old lorentz boost
 
  if ( ! fPrimaryGenerator->AppliesLorentzTransform() ) {
    
    ApplyLorentzTransformation(evt);
  }
}

void PrimaryGeneratorAction::ApplyLorentzTransformation(G4Event *evt)
{

  //std::cout << " **************************  ApplyLorentzTransformation: applying Lorentz boost  ! **************************** " << std::endl ;

  const G4double alpha = Control::LorentzTransformationAngle;
  if (alpha == 0) return; // nothing to do

  // parameters of the Lorentz transformation matrix
  const G4double gamma = sqrt(1 + sqr(tan(alpha)));
  const G4double betagamma = tan(alpha);

  // scan through all vertices and all valid primary particles
  for (int iVertex = 0; iVertex < evt->GetNumberOfPrimaryVertex(); iVertex++) {
    G4PrimaryVertex *vertex = evt->GetPrimaryVertex(iVertex);
    for (int iParticle = 0; iParticle < vertex->GetNumberOfParticle(); iParticle++) {
      G4PrimaryParticle *particle = vertex->GetPrimary(iParticle);
      // does the particle have a valid particle definition attached?
      if (particle->GetG4code()) {

        // before the transformation
        const G4double px = particle->GetPx();
        const G4double py = particle->GetPy();
        const G4double pz = particle->GetPz();

//         const G4double m  = particle->GetG4code()->GetPDGMass();
	//FG: take generator mass ( not necessarily euqal to PDG mass ) 
	const G4double m  = particle->GetMass() ;

        // after the transformation (boost in x-direction)
        const G4double pxPrime = betagamma * sqrt(sqr(px) + sqr(py) + sqr(pz) + sqr(m)) + gamma * px;

        // py and pz remain the same, E changes implicitly with px
        particle->SetMomentum(pxPrime, py, pz);
      }
    }
    // the position of the vertex is not transformed here

  }

  //FG: now we need to apply the same transformation to the MCParticles:

  
#ifdef LCIO_MODE
  LCCollection* col = Control::lcMCVec ;

  if( col !=0 ) { // if particle gun is used no collection exists ...

    int nMCP = col->getNumberOfElements() ;
    
    for(int i=0; i < nMCP ; ++i ) {
      
      MCParticleImpl* mcp = dynamic_cast<MCParticleImpl*>( col->getElementAt(i) ) ;
      
      const double* p = mcp->getMomentum() ;
      
      // before the transformation
      double pPrime[3] ;
      
      const G4double m  = mcp->getMass() ;
      
      // after the transformation (boost in x-direction)
      pPrime[0] = betagamma * sqrt(sqr(p[0] ) + sqr(p[1] ) + sqr( p[2]) + sqr(m) ) + gamma * p[0] ;
      
      pPrime[1] = p[1] ;
      pPrime[2] = p[2] ;
      
      // py and pz remain the same, E changes implicitly with px
      mcp->setMomentum( pPrime );
      
    
    }
  }
#endif
}
