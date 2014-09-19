// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: GuineaPigInterface.cc,v 1.4 2009/01/12 16:04:18 frank Exp $
// $Name: mokka-07-00 $

#include "GuineaPigInterface.hh"

#include "G4Event.hh"
#include "G4ThreeVector.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4ParticleDefinition.hh"
#include "Control.hh"

GuineaPigInterface::GuineaPigInterface(G4String inputFilename)
{
  fInputFile.open(inputFilename);
  if (!fInputFile) {
    Control::Abort(G4String("GuineaPigInterface: "
      "Cannot open file \"" + inputFilename + "\".").data(),
		MOKKA_ERROR_CANNOT_OPEN_GUINEA_PIG_INPUT_FILE);
  }
}

GuineaPigInterface::~GuineaPigInterface(void)
{
  if (fInputFile.is_open())
    fInputFile.close();
}

void GuineaPigInterface::GeneratePrimaryVertex(G4Event *event)
{
  G4double energy, betaX, betaY, betaZ, posX, posY, posZ;
  std::stringstream eventNumber;
  eventNumber << "Event " <<  event->GetEventID() + Control::SYNCEVT;
   
  //FG: combine n particles into one event 
  for(int i=0 ; i < Control::PairParticlesPerEvent ; ++i ) {
    
    fInputFile >> energy >> betaX >> betaY >> betaZ >> posX >> posY >> posZ;
    
    // you get this only _after_ you try to read past the EOF
    if (fInputFile.eof())
     if(i==0) 
     {
      Control::RunAborted = true;
      G4Exception("GuineaPigInterface::GeneratePrimaryVertex", 
                  eventNumber.str().c_str(),
        	  RunMustBeAborted, "*** Trying to read past the end of file: no particles for this event");

      return;
     }
     else 
     {
      G4Exception("GuineaPigInterface::GeneratePrimaryVertex", 
        eventNumber.str().c_str(), JustWarning,
	"*** The end of file was reached before reading all PairParticlesPerEvent");
      Control::InputFileGeneratorWarning = "WARNING: GuineaPigInterface:\n*** The end of file was reached before reading all PairParticlesPerEvent";

      return;
     }
    fInputFile.ignore(1024, '\n'); // throw away everything up to the end of the line
    
    G4ParticleDefinition *particleDef = 0;
    if (energy >= 0) particleDef = G4Electron::Electron();
    else             particleDef = G4Positron::Positron();
    // positrons are marked with negative energies
    energy = fabs(energy) * GeV; // remove the marker and use Geant units
    
    const G4double momX = betaX * energy;
    const G4double momY = betaY * energy;
    const G4double momZ = betaZ * energy;
  
    posX *= nanometer; // use Geant units
    posY *= nanometer;
    posZ *= nanometer;
    
    G4PrimaryParticle *primaryParticle = new G4PrimaryParticle(particleDef, momX, momY, momZ);
    primaryParticle->SetMass(particleDef->GetPDGMass());
    primaryParticle->SetCharge(particleDef->GetPDGCharge());
  
    G4PrimaryVertex *primaryVertex = new G4PrimaryVertex(posX, posY, posZ, 0);
    // Guinea Pig provides no time information, so assume time zero
    primaryVertex->SetPrimary(primaryParticle); // vertex with only one particle
    event->AddPrimaryVertex(primaryVertex); // event with only one vertex


    //fg debug:
    //    G4cout <<  " pair particle (px,py,pz): " 
    // 	   << momX << ", " << momY << ", " << momZ << ", " 
    // 	   << G4endl;

  }

}
