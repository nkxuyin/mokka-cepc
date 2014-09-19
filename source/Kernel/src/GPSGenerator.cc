// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: GPSGenerator.cc,v 1.2 2007/06/22 14:42:50 musat Exp $
// $Name: mokka-07-00 $

#include "GPSGenerator.hh"

#include "Control.hh"
#include "UserInit.hh"

#include "G4GeneralParticleSource.hh"
#include "G4MuonPlus.hh"
#include "G4Event.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include "G4ParticleDefinition.hh"
#include "G4UIcommand.hh"
#include "Randomize.hh"

GPSGenerator::GPSGenerator(void):VPrimaryGenerator("gps")
{
  fGenerator = new G4GeneralParticleSource();

  G4ParticleDefinition * particle = G4MuonPlus::MuonPlus();
  fGenerator->GetCurrentSource()->SetParticleDefinition(particle);

  fGenerator->GetCurrentSource()->GetPosDist()->SetPosDisType("Point");
  fGenerator->GetCurrentSource()->GetPosDist()->SetCentreCoords(
						G4ThreeVector(0, 0, 0));

  fGenerator->GetCurrentSource()->GetAngDist()->SetAngDistType("planar");
  fGenerator->GetCurrentSource()->GetAngDist()->
		SetParticleMomentumDirection(G4ThreeVector(0, 1, 0));

  fGenerator->GetCurrentSource()->GetEneDist()->SetEnergyDisType("Mono");
  fGenerator->GetCurrentSource()->GetEneDist()->SetMonoEnergy(100 * GeV);
  // If you want /generator/info to show these settings at startup then do:
  // GeneratePrimaryVertex(new G4Event);
}

GPSGenerator::~GPSGenerator(void)
{
  delete fGenerator;
}

void GPSGenerator::GeneratePrimaryVertex(G4Event *evt)
{
  fGenerator->GeneratePrimaryVertex(evt);
}

void GPSGenerator::PrintGeneratorInfo(void)
{
  const G4ParticleDefinition *particleDef = fGenerator->GetParticleDefinition();

  const G4ThreeVector direction = fGenerator->GetParticleMomentumDirection();

  // borrow G4UIcommand::ConvertToString(...) for a minute because it does exactly what we need
  G4cout
    << "Particle:       " << particleDef->GetParticleName()
    << " (m = " << G4UIcommand::ConvertToString(particleDef->GetPDGMass(), "GeV") << ")"
    << G4endl;
  G4cout
    << "Kinetic Energy: " << G4UIcommand::ConvertToString(fGenerator->GetParticleEnergy(), "GeV")
    << G4endl;
  G4cout
    << "Position:       " << G4UIcommand::ConvertToString(fGenerator->GetParticlePosition(), "cm")
    << G4endl;
  G4cout
    << "Direction:      " << G4UIcommand::ConvertToString(direction) // has no unit
    << " (theta = " << G4UIcommand::ConvertToString(direction.getTheta(), "deg")
    << ", phi = " << G4UIcommand::ConvertToString(direction.getPhi(), "deg") << ")"
    << G4endl;
}
