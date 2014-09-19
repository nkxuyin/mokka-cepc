// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: PrimaryGeneratorMessenger.cc,v 1.7 2007/06/22 14:42:50 musat Exp $
// $Name: mokka-07-00 $

#include "PrimaryGeneratorMessenger.hh"
#include "PrimaryGeneratorAction.hh"
#include "VPrimaryGenerator.hh"

#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithoutParameter.hh"

PrimaryGeneratorMessenger::PrimaryGeneratorMessenger(PrimaryGeneratorAction *primaryGenerator)
{
  fPrimaryGenerator = primaryGenerator;

  fGeneratorDir = new G4UIdirectory("/generator/");
  fGeneratorDir->SetGuidance("Primary generator control commands.");
  
  fGeneratorNameCmd = new G4UIcmdWithAString("/generator/generator", this);
  fGeneratorNameCmd->SetGuidance("Select primary generator.");
  fGeneratorNameCmd->SetGuidance("Available generators: \"particleGun\", \"gps\" or a filename with suffix");
  fGeneratorNameCmd->SetGuidance("  \".HEPEvt\" - ASCII file in reduced HEPEvt common block format");
  fGeneratorNameCmd->SetGuidance("  \".stdhep\" - Binary file in HEPEvt common block format");
  fGeneratorNameCmd->SetGuidance("  \".pairs\"  - ASCII file in Guinea Pig \"pairs.dat\" format");
  /* ...insert any other interfaces here... */
  fGeneratorNameCmd->SetParameterName("generator", true);
  fGeneratorNameCmd->SetDefaultValue("particleGun");

  fGeneratorInfoCmd = new G4UIcmdWithoutParameter("/generator/info", this);
  fGeneratorInfoCmd->SetGuidance("Print some information about the next shot.");

}

PrimaryGeneratorMessenger::~PrimaryGeneratorMessenger(void)
{
  delete fGeneratorNameCmd;
  delete fGeneratorDir;
  delete fGeneratorInfoCmd;
}

void PrimaryGeneratorMessenger::SetNewValue(G4UIcommand *command, G4String newValue)
{
  if(command == fGeneratorNameCmd)       
		fPrimaryGenerator->SetGeneratorWithName(newValue);
  else if(command == fGeneratorInfoCmd)
		fPrimaryGenerator->GetPrimaryGenerator()->PrintGeneratorInfo();
}

G4String PrimaryGeneratorMessenger::GetCurrentValue(G4UIcommand *command)
{
  if(command == fGeneratorNameCmd)       
	return fPrimaryGenerator->GetPrimaryGenerator()->GetGeneratorName();
  return G4String(); // nothing
}
