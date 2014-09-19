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
//*    Mokka home page.                                 *
//*                                                     *
//*******************************************************
//
// $Id: CGAInit.cc,v 1.13 2006/06/01 11:17:11 musat Exp $
// $Name: mokka-07-00 $
//
// History
// first implementation for the 
// Mokka Common Geometry Access (CGA) by 
// Gabriel Musat (musat@poly.in2p3.fr), July 2002
//
// see CGA documentation at 
// http://polype.in2p3.fr/geant4/tesla/www/mokka/
//        software/doc/CGADoc/CGAIndex.html
//-------------------------------------------------------

//--------------------------------
// The following include forces
// a special Union Solid release to
// turn around some G4 problems.
//--------------------------------

#include "G4GeometryManager.hh"
//--------------------
// Global control variables
//--------------------
#include "Control.hh"

//--------------------
// Detector:
//--------------------
#include "CGAGeometryManager.hh"

//------------------------------
// Run, Generator etc.. actions:
//------------------------------
#include "PrimaryGeneratorAction.hh"
#include "CGASteppingAction.hh"
#include "TrackingAction.hh"

//----------------------------------
// Physics List
//----------------------------------
#include "PhysicsListFactory.hh"
#include "PhysicsListUserLimits.hh"

#include "DummyPhysicsList.hh"
#include "RunManager.hh"
#include "G4UImanager.hh"
#include "G4ios.hh"
#include <string.h>
#include "CGADefs.h"

#ifdef MOKKA_GEAR
// include MokkaGear to write out xml files for geometry description
#include "gear/GEAR.h"
#include "gear/GearMgr.h"
#include "gearimpl/GearMgrImpl.h"
#include "gearxml/GearXML.h"

#include "MokkaGear.h"
#include "G4StateManager.hh"
#include <ctime>
#include <stdio.h>

using namespace gear ;
GearMgr* cgaGearMgr = 0;

#endif

extern "C" {
	void cgainit_(const char* steering, const char *model, 
			const char* setup, const char *host, 
			const char *user, const char *passwd, int steerlen,
			int modelLen, 
			int setupLen, int hostLen, int userLen, int passwdLen);
}

void endString(char *str, int sLen);
void fillString(char *str, int sLen);

void CGAInit(const char* steer, const char *model, const char* setup, const char *host, const char *user, const char *passwd) {
	cgainit_(steer, model, setup, host, user, passwd, strlen(steer), 
		strlen(model), strlen(setup), strlen(host), strlen(user), 
		strlen(passwd));
}

void cgainit_(const char *steering, const char *Model, const char *Setup, 
		const char *Host, const char *User, const char *Passwd,
		int steerLen, int modelLen, int setupLen, int hostLen, 
		int userLen, int passwdLen) {

  char steer[10000], model[1024], setup[1024], host[1024], user[1024], passwd[1024];

  char *lineToken;

  Control::GetControl();
  Control::Log("Initializing geometry");

  strncpy(steer, steering, steerLen);
  strncpy(model, Model, modelLen);
  strncpy(setup, Setup, setupLen);
  strncpy(host, Host, hostLen);
  strncpy(user, User, userLen);
  strncpy(passwd, Passwd, passwdLen);

  endString(steer, steerLen+1);
  endString(model, modelLen+1);
  endString(setup, setupLen+1);
  endString(host, hostLen+1);
  endString(user, userLen+1);
  endString(passwd, passwdLen+1);

  if(strlen(model) != 0)
	Control::DETECTOR_MODEL=model;
  if(strlen(setup) != 0)
	Control::DETECTOR_SETUP=setup;
  if(strlen(host) != 0)
  	Control::DBHOST=host;
  if(strlen(user) != 0)
  	Control::USER=user;
  if(strlen(passwd) != 0)
  	Control::DBPASSWD=passwd;

  if(strlen(steer) != 0) {
	lineToken = strtok(steer, "\n");
	while(lineToken != NULL) {
		G4UImanager::GetUIpointer()->ApplyCommand(lineToken);
		lineToken=strtok(NULL, "\n");
	}
}

  //-------------------------------
  // Initialization of Run manager
  //-------------------------------
  G4cout << "RunManager construction starting...." << G4endl;

  RunManager * runManager = new RunManager;
  runManager->SetNumberOfEventsToBeStored(1);

  // This works only for Geant 4.4.0 and later
  Control::Log(runManager->GetVersionString());
  // PhysicsList
  // (forces as VISUMODE mode just to create general Physics)
  G4VUserPhysicsList * thePhysicsList = PhysicsListFactory::create(Control::PhysicsListName);
  thePhysicsList->SetDefaultCutValue(Control::RangeCut);
 
  runManager->SetUserInitialization(thePhysicsList);
  
  // Detector geometry
  CGAGeometryManager* CGAGeoMan = CGAGeometryManager::GetCGAGeometryManager();
  runManager->SetUserInitialization(CGAGeoMan);

  // Primary Generator Action
  runManager->SetUserAction(new PrimaryGeneratorAction);

  // tracking action (to control number of steps)
  runManager->SetUserAction(new TrackingAction);
                                                                                
  // Stepping Action (to draw the steps and etc.)
  runManager->SetUserAction(new CGASteppingAction);

  // Initialize Run manager
  runManager->Initialize();

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

  // write out GearXML
  MokkaGear* gearMgr = MokkaGear::getMgr() ;

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

  mokkaParams->setStringVal( "MokkaVersion" ,  tagName )  ;

  for( CGASetupParameters::iterator gnIt = Control::globalModelParameters->begin() ;
       gnIt != Control::globalModelParameters->end() ; gnIt++ ){

    mokkaParams->setStringVal( gnIt->first , gnIt->second ) ;

  }
  
  bool printOK = false;
  std::string tmpCGAGearFileName = "/tmp/tmpCGAGear.xml";

  gearMgr->setFileName( tmpCGAGearFileName );
  printOK = gearMgr -> printXML() ;

  // check success
  if (printOK) {
      G4cout << "\nCGAGear -Status- geometry printed out to tmp xml" << G4endl;

      GearXML gearXML( tmpCGAGearFileName );
      cgaGearMgr = gearXML.createGearMgr() ;

      remove( tmpCGAGearFileName.c_str() ) ;

  }
  else {
      G4cout << "\nCGAGear -Error- while printing out geometry to tmp xml." <<G4endl;
  }

#endif

  G4GeometryManager::GetInstance()->CloseGeometry();


}

