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
// $Id: VSubDetectorDriver.cc,v 1.28 2007/11/23 10:00:45 frank Exp $
// $Name: mokka-07-00 $
//
//
// VSubDetectorDriver.cc
//
// History:  
// - first implementation P. Mora de Freitas (apr 01)
// - updated to implement new GEAR interface -- K.Harder, T.Pinto Jayawardena  2007-07-31
//
#include "Control.hh"
#include "CGAGeometryManager.hh"
#include "VSubDetectorDriver.hh"
#include "VHit.hh"
#include "VSensitiveDetector.hh"

#include "G4SDManager.hh"
#include "G4Event.hh"

#include <errno.h>
#include <stdio.h>

VSubDetectorDriver::VSubDetectorDriver(const G4String &aDriverName, 
				       G4String aBaseFileName)
 : theBaseFileName(aBaseFileName)  //  : theBaseFileName(aBaseFileName),saveHitMomentum(false)
{
  IsConstructed = false;
  if(aDriverName == "") 
    {
      Control::Abort("VSubDetectorDriver has to have a valid name!",
		MOKKA_OTHER_ERRORS);
    }
  theDriverName=aDriverName;
  CGAGeometryManager::
    GetCGAGeometryManager()->RegisterGeometryDriver(this);
}

#ifdef MOKKA_GEAR
void VSubDetectorDriver:: GearSetupIfConstructed()
{
  if( IsConstructed)
    GearSetup();
}
#endif


G4bool 
VSubDetectorDriver::IsApplicable(const G4String &aDriverName) const
{
  return (theDriverName == aDriverName);
}

void 
VSubDetectorDriver::RegisterSensitiveDetector(VSensitiveDetector* aSensitiveDetector)
{
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  SDman->AddNewDetector(aSensitiveDetector);
  aSensitiveDetector->setID(theSensitiveDetectors.size());
  theSensitiveDetectors.push_back(aSensitiveDetector);
  // P.K 
  saveHitMomentum.push_back(false);
}

G4String VSubDetectorDriver::AsciiSubDetectorEventFileName(const G4Event* evt) const
{
  G4String FileName="";
  if(Control::OutDirName.length()==0) return FileName;
  if(theBaseFileName.length()==0)
    {
      G4cout << "Fatal error in " << GetName() 
	     << " detector driver: no base file name given in constructor."
	     << G4endl;
      Control::Abort("Mokka kernel is not able to save data without a base file name.", MOKKA_OTHER_ERRORS);
    }
  
  char buffer[40];
  sprintf(buffer,"%.6d",
	  evt->GetEventID()+Control::SYNCEVT);

  FileName=Control::OutDirName+"/"+theBaseFileName;
  FileName+=buffer;
  FileName+=".hits";
  
  return FileName;
}

void 
VSubDetectorDriver::EndOfEventAction(const G4Event* evt)
{
  // If there is none Sensitive Detector for this geometry driver,
  // just returns
  G4int n_SensitiveDetectors=0;
  if((n_SensitiveDetectors=theSensitiveDetectors.size())==0) return;
  
  // Open an Ascii file to save hits if in persistent mode
  FILE *oFile = NULL;
  G4String theAsciiSubDetectorEventFileName = AsciiSubDetectorEventFileName(evt);

  if(theAsciiSubDetectorEventFileName.length()!=0 && !Control::VISUMODE)
    if((oFile=fopen(theAsciiSubDetectorEventFileName,"w"))==NULL) {
      perror(theAsciiSubDetectorEventFileName);
      exit(1);
    }

  G4SDManager * SDman = G4SDManager::GetSDMpointer();
  G4HCofThisEvent * HCE = evt->GetHCofThisEvent();
  VHitsCollection* CHC = NULL;
  G4int CollID;
  G4int ihit;


  // Localize the hits collections registered by this driver,
  // summarize and write it on file


  
  for (G4int i_sd=0;i_sd<n_SensitiveDetectors;i_sd++) {
    for (G4int i_coll=0; 
	 i_coll<theSensitiveDetectors[i_sd]->GetNumberOfCollections();
	 i_coll++)
      {
	if((CollID = SDman->
	    GetCollectionID(theSensitiveDetectors[i_sd]->GetCollectionName(i_coll)))!=-1)
	  {
	    CHC = (VHitsCollection*) (HCE->GetHC(CollID));

#ifdef LCIO_MODE
	    LCCollectionVec* LcColVec= NULL;
#endif


	    //if(CHC == 0) return;
	    if(CHC == 0) continue;
	    
	    if(Control::PrintLevel > 1) 
	      {
		G4cout << theSensitiveDetectors[i_sd]->GetCollectionName(i_coll)
		       << " from the "
		       <<  theSensitiveDetectors[i_sd]->GetName()
		       << " sensitive detector has " 
		       << CHC->entries() << " hits." << G4endl;
	      }
	    if(oFile)
	      for(ihit = 0; ihit< CHC->entries(); ihit++) {
		int iFlag = 0;
		iFlag = (id << 8) |
		  (theSensitiveDetectors[i_sd]->getID());
		(*CHC)[ihit]->setAsciiFlag(iFlag);
		(*CHC)[ihit]->Save(oFile);
	      }
	    
#ifdef LCIO_MODE
	    if(CHC->entries()>0 && Control::lcWrt)
	      {
		if(!LcColVec) 
		  LcColVec= new LCCollectionVec( (*CHC)[0]->GetLCCollectionType());
		// set flag for long format (including position )
		// and PDG 
		int iFlag = 0;
		iFlag = (id << 8) |
		  (theSensitiveDetectors[i_sd]->getID());
		LCFlagImpl chFlag(iFlag) ;
		
		if((*CHC)[0]->GetLCCollectionType() == LCIO::SIMCALORIMETERHIT)
		  {
		    if(Control::LCIOStorePosition)
		      chFlag.setBit( LCIO::CHBIT_LONG ) ;
		    
		    if(theSensitiveDetectors[i_sd]->getEncoder()->getID1Flag())
		      chFlag.setBit( LCIO::CHBIT_ID1 ) ;
		    
		    if( Control::LCIODetailedShowerMode )
#if LCIO_VERSION_GE( 1, 60 )
		      chFlag.setBit( LCIO::CHBIT_STEP ) ;
#else
		      chFlag.setBit( LCIO::CHBIT_PDG ) ;
#endif
		    
		    EVENT::LCParameters & theParam = LcColVec->parameters();
		    
#if LCIO_VERSION_GE( 1, 7 )
		    theParam.setValue(LCIO::CellIDEncoding,
				      theSensitiveDetectors[i_sd]->getEncoder()->getIDString());
#else
		    theParam.setValue( "CellIDEncoding",
				       theSensitiveDetectors[i_sd]->getEncoder()->getIDString());
#endif
		    
		  }
		
#if LCIO_VERSION_GE( 1, 6 )
		//cout << saveHitMomentum[i_sd]<<endl;
		if((*CHC)[0]->GetLCCollectionType() == LCIO::SIMTRACKERHIT
		   &&
		   saveHitMomentum[i_sd])
		  chFlag.setBit( LCIO::THBIT_MOMENTUM  ) ;
#endif
		LcColVec->setFlag( chFlag.getFlag()  ) ;
		
		
		for(ihit = 0; ihit< CHC->entries(); ihit++)
		  (*CHC)[ihit]->basic_Save(LcColVec);
		
	      }


	    if(LcColVec) 
	      try {
		Control::lcEvt->addCollection( LcColVec, 
					       theSensitiveDetectors[i_sd]->GetCollectionName(i_coll)
					       ) ;
	      }
	      catch(IOException& e) {
		
		G4cout << "LCIO error when adding collection to an event:\n" 
		       << e.what() 
		       << G4endl ;
		Control::Abort("Fatal error: LCIO error when adding collection to an event.",MOKKA_ERROR_CANNOT_WRITE_TO_LCIO_EVENT);
	      }
	    LcColVec = NULL;


#endif  // LCIO
	  }
      }
  }
  
  
  if(oFile) fclose(oFile);
}

void 
VSubDetectorDriver::LoadEvent(G4Event* evt)
{
  // If there is none Sensitive Detector for this geometry driver,
  // just returns
  G4int n_SensitiveDetectors=0;
  if((n_SensitiveDetectors=theSensitiveDetectors.size())==0) return;
  
  G4cout << "Loading event for geometry driver " << GetName()
	 << "." << G4endl;

  FILE *iFile = NULL;
  if((iFile=fopen(AsciiSubDetectorEventFileName(evt),"r"))==NULL) {
    perror(AsciiSubDetectorEventFileName(evt));
    return;
  }
  
  LoadEvent(iFile);
  
  if(iFile) fclose (iFile);
}

void VSubDetectorDriver::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
  for (unsigned int i_sd=0;i_sd<theSensitiveDetectors.size();i_sd++)
    theSensitiveDetectors[i_sd]->LoadEvent(theSubDetectorEventHitsFileInput);
}

G4bool 
VSubDetectorDriver::basic_construct(const CGAGeometryEnvironment &aGeometryEnvironment,
				    G4LogicalVolume *theWorld
#ifdef LCIO_MODE
				    , G4String aSubDetectorName
#endif
				    )
{
#ifdef LCIO_MODE  
  SubDetectorName=aSubDetectorName;
#endif
  IsConstructed = true;
  return ContextualConstruct (aGeometryEnvironment,theWorld); 
}
