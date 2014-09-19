// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: InputFileGenerator.cc,v 1.4 2007/08/31 16:02:45 mora Exp $
// $Name: mokka-07-00 $

#include "InputFileGenerator.hh"

#include "G4HEPEvtInterface.hh"
#include "lHEPEvtInterface.hh"
#include "HepLCIOInterface.hh"
#include "HepLCIOInterfaceNew.hh"
#include "GuineaPigInterface.hh"
#include "Control.hh"
#include "UserInit.hh"

#include "G4Event.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include "G4ParticleDefinition.hh"
#include "G4UIcommand.hh"
#include "Randomize.hh"

#include <fstream>

InputFileGenerator::InputFileGenerator(G4String fileName): 
		VPrimaryGenerator(fileName)
{ 
  fGenerator = 0;
  SetupFileGenerator(fileName);
}

void InputFileGenerator::SetupFileGenerator(G4String generatorName)
{
    if (fGenerator) delete fGenerator; // dispose of the old generator

    G4VPrimaryGenerator *newGenerator = 0;

    // does the file exist?
    std::ifstream generatorFile(generatorName); // simply try and open it!
    if (!generatorFile) {
      G4cout << "Could not open \"" << generatorName << "\", please choose another file." << G4endl;
      return; // nothing has really happened yet, we can still exit here
    } else {
	generatorFile.close(); // one of the interfaces will open it again
    }

    // now we know the file exists, but what kind of file is it?
    const size_t dotPosition = generatorName.rfind("."); // find the last dot (suffix separator)
    const G4String nameSuffix = (dotPosition != std::string::npos) ? // found a dot?
      generatorName(dotPosition, generatorName.length() - dotPosition) : // yes: get suffix
      G4String(); // no: don't get anything


    //static bool writeCompleteHepEvt =  UserInit::getInstance()->getBool("WriteCompleteHepEvt") ;


#ifdef LCIO_MODE

    if ( Control::LCIOWriteCompleteHepEvt ){

      if( Control::USE_OLD_HEPLCIO  ){

	if (nameSuffix == ".stdhep") 
	  newGenerator =  new HepLCIOInterface( generatorName ) ;
	else if (nameSuffix == ".HEPEvt") 
	  newGenerator =  new HepLCIOInterface( generatorName , HEPEvt )  ;
	else if (nameSuffix == ".hepevt") 
	  newGenerator = new HepLCIOInterface( generatorName, hepevt ) ;

      } else {

	if (nameSuffix == ".stdhep") 
	  newGenerator =  new HepLCIOInterfaceNew( generatorName ) ;
	else if (nameSuffix == ".HEPEvt") 
 	  newGenerator =   new HepLCIOInterfaceNew( generatorName , HepLCIOInterfaceNew::HEPEvt ) ;
	else if (nameSuffix == ".hepevt") 
	  newGenerator =   new HepLCIOInterfaceNew( generatorName, HepLCIOInterfaceNew::hepevt ) ;
      }
    }
#endif
    if(!newGenerator)
      {
	if (nameSuffix == ".HEPEvt") newGenerator = new G4HEPEvtInterface(generatorName);
	else if (nameSuffix == ".stdhep") newGenerator = new lHEPEvtInterface(generatorName);
	else if (nameSuffix == ".pairs")  newGenerator = new GuineaPigInterface(generatorName);
	/* ...insert any other interfaces here... */
	else { // filename has an unknown suffix (or no suffix at all)
	  Control::Abort("InputFileGenerator:The generator has to be filename\n"
	    "with suffix \".HEPEvt\", \".stdhep\", or \".pairs\".",
		MOKKA_ERROR_BAD_GENERATOR_FILENAME);
	}
      }
    // now we know that the new generator is really valid
  fGenerator = newGenerator; 
  SetGeneratorName(generatorName);

  // skip events in the generator file
  if ((Control::RESTART && Control::OutDirName.length() > 0) 
      || Control::SKIP) 
    {
      G4cout << "Skipping first " << Control::SYNCEVT << " events..." << G4endl;
      for (G4int i = 0; i < Control::SYNCEVT; i++) {
	G4Event *evt = new G4Event(); // just a dummy
	fGenerator->GeneratePrimaryVertex(evt);
	delete evt;

#ifdef LCIO_MODE
	// When skipping events, take care to delete
	// MCParticles from the LCIO2Geant4Map to avoid 
	// memory leak
	HepLCIOInterfaceNew::LCIO2Geant4Map::iterator i_map = 
	  HepLCIOInterfaceNew::Map.begin();
	for (;i_map != HepLCIOInterfaceNew::Map.end();i_map++)
	  {
	    delete i_map->first;
	  }
#endif	
      }
    }
}


bool InputFileGenerator::AppliesLorentzTransform() {
  //FG: the HepLCIOInterfaceNew is the only one that appliers the Lorentz boost (to the MCParticle list )
  return  dynamic_cast< HepLCIOInterfaceNew*>( fGenerator ) != 0 ; 
}


InputFileGenerator::~InputFileGenerator(void)
{
  if (fGenerator) delete fGenerator;
}

void InputFileGenerator::GeneratePrimaryVertex(G4Event *evt)
{
  if (fGenerator)
    fGenerator->GeneratePrimaryVertex(evt); // read event from a file
}

void InputFileGenerator::PrintGeneratorInfo(void)
{
  G4cout << "InputFileGenerator: generator file name is: " << GetGeneratorName()
		 << G4endl;
  // Additional info to be added
}
