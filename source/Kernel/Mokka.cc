// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: Mokka.cc 310 2012-03-22 17:03:28Z engels $
// $Name: mokka-07-00 $

//--------------------------
// Global control variables
//--------------------------
#include "Control.hh"

//--------------------
// Detector
//--------------------
#include "CGAGeometryManager.hh"

//------------------------------
// Run, Generator etc. Actions
//------------------------------
#include "PrimaryGeneratorAction.hh"
#include "StackingAction.hh"
#include "TrackingAction.hh"
#include "SteppingAction.hh"
#include "EventAction.hh"
#include "RunAction.hh"

//----------------------------------
// Plugins
//----------------------------------
#include "PluginManager.hh"

//----------------------------------
// Physics List
//----------------------------------
#include "PhysicsListFactory.hh"
#include "PhysicsListUserLimits.hh"
#include "DummyPhysicsList.hh"
#include "ExtraParticles.hh"

// G4UIExecutive only works with geant4 >= 9.5
#if ( ! G4_VERSION_GE( 950 ) )
#undef G4UI_USE
#endif


#ifdef G4UI_USE
#include "G4UIExecutive.hh"
#endif

#ifdef G4UI_USE_GAG
#include "G4UIGAG.hh"
#endif

#ifdef G4UI_USE_XM
#include "G4UIXm.hh"
#endif

#ifdef G4UI_USE_TCSH
#include "G4UItcsh.hh"
#endif

#include "G4UIterminal.hh"
#include "G4UImanager.hh"
#include "RunManager.hh"

#ifdef G4VIS_USE
#include "G4VisExecutive.hh"
#endif

#include "G4ios.hh"

// #ifdef LCIO_MODE
// //fg: include a patched version of the LCStdHepRdr from LCIO
// #include "LCStdHepRdr.icc"
// #endif

#ifdef MOKKA_GEAR
// include MokkaGear to write out xml files for geometry description
#include "MokkaGear.h"
#include "G4StateManager.hh"
#include <ctime>
#include <stdio.h>
#endif

#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"

#if ( ! G4_VERSION_GE( 941 ) ) && ( G4_VERSION_GE( 940 ) )
#include "G4LossTableManager.cc"
#endif

#include <string>

int main(int argc, char *argv[]) {

  Control::GetControl()->setup(argc, argv);

  std::string fullURL("$URL: http://llrforge.in2p3.fr/svn/Mokka/tags/mokka-08-03/source/Kernel/Mokka.cc $");
  std::string::size_type pos1, pos2;
  pos1 = fullURL.find("tags"); 
  pos2 = fullURL.find("source");
  std::string MokkaName("void");
  if(pos1 != std::string::npos)
	MokkaName = std::string("tag ") + 
		fullURL.substr(pos1+5,pos2-pos1-6);
  else if(fullURL.find("trunk") != std::string::npos)
	MokkaName = "trunk";
  else if((pos1 = fullURL.find("branches")) != std::string::npos)
	MokkaName = std::string("branch ") + 
		fullURL.substr(pos1+9,pos2-pos1-10);

  Control::MokkaVersionName = MokkaName.c_str();

  Control::Log((std::string("MokkaVersion : ") + MokkaName + "\n$Id: Mokka.cc 310 2012-03-22 17:03:28Z engels $").c_str());
  
  //Control::Log("Mokka tag $Name: mokka-07-00 $, $Id: Mokka.cc 310 2012-03-22 17:03:28Z engels $");

  //----------------
  // User Interface
  //----------------
  G4UImanager *UImanager = G4UImanager::GetUIpointer();

  //----------------
  // Run Manager
  //----------------
  G4cout << "Constructing RunManager..." << G4endl;
  RunManager *runManager = new RunManager;
  runManager->SetNumberOfEventsToBeStored(1);

  // physics list
  G4VUserPhysicsList *thePhysicsList;
  if (Control::VISUMODE) {
    // in VISUMODE mode, just create DummyPhysics
    thePhysicsList = new DummyPhysicsList(1);
  } else {
    thePhysicsList = PhysicsListFactory::create(Control::PhysicsListName);
    thePhysicsList->SetDefaultCutValue(Control::RangeCut);

               // extra particles
               if (ExtraParticles::FileExists()) {
                       G4VModularPhysicsList* modularPhysicsList = dynamic_cast<G4VModularPhysicsList*>(thePhysicsList);
                       if (modularPhysicsList) {
                               modularPhysicsList->RegisterPhysics(new ExtraParticles());
                       } else {
                               G4cout << "dynamic cast to G4VModularPhysicsList failed, not loading ExtraParticles..." << G4endl;
                       }

               } else { 

		 G4cout << " *** Can't load ExtraParticles - file not found: " <<  Control::PDGFile << G4endl ;
	       }
  }
  runManager->SetUserInitialization(thePhysicsList);

  // detector geometry
  CGAGeometryManager* CGAGeoMan = CGAGeometryManager::GetCGAGeometryManager();
  runManager->SetUserInitialization(CGAGeoMan);

  // primary generator action
  runManager->SetUserAction(new PrimaryGeneratorAction);

  // stacking action
  runManager->SetUserAction(new StackingAction);

  // tracking action (to control number of steps)
  runManager->SetUserAction(new TrackingAction);

  // stepping action (to draw the steps etc.)
  runManager->SetUserAction(new SteppingAction);

  // event action (to save data and draw hits)
  runManager->SetUserAction(new EventAction);

  // run action (to save data and draw hits)
  runManager->SetUserAction(new RunAction);

  // initialize run manager
  runManager->Initialize();

  // enable user limits in the physics list
  PhysicsListUserLimits::Enable();

#ifdef MOKKA_GEAR
  // we need to do an early run initialization here
  // in order to have the dE/dx tables filled,
  // because we need dE/dx information for the GEAR output file
  runManager->RunInitialization();
  // switch GEANT back to idle mode as if nothing had happened
  G4StateManager *stateManager = G4StateManager::GetStateManager();
  stateManager->SetNewState(G4State_Idle);
  // tell subdetector drivers to pass their data to GEAR 
  CGAGeoMan->GearSetup();
#endif

  //----------------
  // Visualization
  //----------------

#ifdef G4VIS_USE
  G4cout << "Constructing VisManager..." << G4endl;
  G4VisExecutive *visManager = new G4VisExecutive;
  visManager->Initialize();
#endif

  G4FieldManager* fieldMgr
                = G4TransportationManager::GetTransportationManager()
                ->GetFieldManager();

  Control::G4DefaultDeltaIntersection = fieldMgr->GetDeltaIntersection();
  Control::G4DefaultDeltaOneStep      = fieldMgr->GetDeltaOneStep();

  // initialize active plugins
  PluginManager::getInstance()->Init();

#ifdef MOKKA_GEAR
  // write out GearXML
  MokkaGear* gearMgr = MokkaGear::getMgr() ;
  // assume error
  bool printOK = false ;
  

#ifdef GEAR_MAJOR_VERSION
#if GEAR_VERSION_GE( 0 , 9 )
  gearMgr->setDetectorName(  Control::DETECTOR_MODEL )  ; 
#endif
#endif

  // --- create a Mokka section in the gear file ----------
  gear::GearParametersImpl* mokkaParams =  new gear::GearParametersImpl() ;

  // set detector model

  gearMgr->setGearParameters( "MokkaParameters" , mokkaParams ) ;
  
  mokkaParams->setStringVal( "MokkaModel" , Control::DETECTOR_MODEL )  ; 

  std::string verStr("$Name: mokka-07-00 $") ;
  std::string tagName( verStr , 7 ,  verStr.size()-9 ) ;
  if( tagName.size()==0 ) 
    tagName="HEAD" ;

  mokkaParams->setStringVal( "MokkaVersion" ,  MokkaName )  ; 


  for( CGASetupParameters::iterator gnIt = Control::globalModelParameters->begin() ;
       gnIt != Control::globalModelParameters->end() ; gnIt++ ){
    
    mokkaParams->setStringVal( gnIt->first , gnIt->second ) ;

  }
  // --------   end Mokka section in gear file ----------
  
  // check if file is to be merged
  if( Control::mokkaGearMergeSource == "" ) {
    
    // only print out 
    // set file name
    gearMgr->setFileName( Control::mokkaGearFileName ) ;
    printOK = gearMgr -> printXML() ;

    // check success
    if (printOK) {
      G4cout << "\nMokkaGear -Status- geometry printed out to xml" << G4endl;
    }
    else {
      G4cout << "\nMokkaGear -Error- while printing out geometry to xml." <<G4endl;
    }
  }
  else {
    // write out and merge

    // get filename with Unix timestamp to have it unique
    //G4String tmpFileName = "tmp" << ctime::time() << ".xml" ;
    G4String tmpFileName = "tmp.xml" ;

    // make output to tmp file
    gearMgr->setFileName( tmpFileName ) ;
    printOK = gearMgr -> printXML() ;
    
    // check success
    if (printOK) {
      G4cout << "\nMokkaGear -Status- geometry printed out to xml - before merging" << G4endl;
      
      // merge tmp outputfile with given dominant xmlfile
      G4String dominantFile = Control::mokkaGearMergeSource ;
      G4String targetFile = Control:: mokkaGearFileName ;

      // debug information
      // std::cout << "dominant : " << dominantFile << " target: " << targetFile << std::endl;

      G4bool printOK2 = gearMgr -> mergeXML( tmpFileName, dominantFile, targetFile ) ;
      
      if ( printOK2 ) {
	G4cout << "                   mergin with " << dominantFile << " successfull." << std::endl ;         
	// remove temp file
	remove( tmpFileName ) ;
      }
      else {
	G4cout << "                   error mergin with " << dominantFile << "." << std::endl ;   
      }
    }
    else {
      G4cout << "\nMokkaGear -Error- while printing out geometry to xml." <<G4endl;
    }
  }

#endif

  if (!Control::IntialMacroFile.empty()) {
    G4String command = "/control/execute ";
    UImanager->ApplyCommand(command + Control::IntialMacroFile);
  }
 


  //
  // Start interactif session if not batch
  if(!Control::BATCH_MODE)  {
    
#if defined( G4UI_USE )
    G4UIExecutive *session = new G4UIExecutive(argc,argv);
#elif   defined(G4UI_USE_GAG)
    G4UIsession *session = new G4UIGAG();
#elif defined(G4UI_USE_XM)
    G4UIsession *session = new G4UIXm(argc, argv);
    UImanager->ApplyCommand("/gui/addMenu file File");
    UImanager->ApplyCommand("/gui/addButton file Exit \"exit\"");
#elif defined(G4UI_USE_TCSH)
    G4UIsession *session = new G4UIterminal(new G4UItcsh());
#else
    G4UIsession *session = new G4UIterminal();
#endif
    
    G4cout << "\n"
           << "   Welcome to Mokka, a detailed Geant4 simulation program\n"
           << "   for the International Linear Collider detector studies.\n"
           << "   This Mokka release relies also on:\n"
           << "\n"
           << "    - lStdHep class written by W.G.J. Langeveld (SLAC)\n"
           << "\n"
           << "   For comments and suggestions about Mokka, please send\n"
           << "   an e-mail to Gabriel Musat <musat@poly.in2p3.fr> or\n" 
           << "   Frank Gaede <frank.gaede@desy.de>.\n"
           << G4endl;
    
#ifdef LCIO_MODE
    G4cout
           << "   You built Mokka/LCIO release. Concerning the LCIO project\n"
           << "   and help, please visit the LCIO site http://lcio.desy.de\n"
           << G4endl;
#endif
    
#ifdef MOKKA_GEAR
    G4cout
           << "   You built MokkaGear release. Concerning the GEAR project\n"
           << "   and help, please visit the Gear site http://ilcsoft.desy.de\n"
           << G4endl;
#endif
    
    G4cout
           << "   Thank you for running Mokka and good luck!\n"
           // "   Abandon all hope, ye who enter here!\n"
           << G4endl;
    
    // start the interactive session
    session->SessionStart();
    // when we arrive here, the session has ended
    delete session;
  }
  
  // exit active plugins
  PluginManager::getInstance()->Exit();

#ifdef LCIO_MODE
  if (Control::lcWrt) {
    try {
      Control::lcWrt->close();
    } catch(IOException &e) {
      G4cout << "IO Error while closing the LCIO file:\n" << e.what() << G4endl;
      Control::Abort("Fatal error: IO Error while closing the LCIO file.",
		MOKKA_ERROR_CANNOT_WRITE_TO_LCIO_FILE);
    }
    delete Control::lcWrt;
  }
#endif

  delete runManager;
#ifdef G4VIS_USE
  delete visManager;
#endif
  
  if(Control::RunAborted) 
	return MOKKA_ERROR_STDHEP_FILE_RAN_OUT_OF_EVENTS;

  return EXIT_SUCCESS;
}
