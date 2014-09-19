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
// $Id: VSuperSubDetectorDriver.cc,v 1.2 2005/08/09 07:31:07 mora Exp $
// $Name: mokka-07-00 $
//
//
// VSuperSubDetectorDriver.cc
//
// History:  
// - first implementation P. Mora de Freitas (apr 01)
//
#include "Control.hh"
#include "CGAGeometryManager.hh"
#include "VSuperSubDetectorDriver.hh"

VSuperSubDetectorDriver::VSuperSubDetectorDriver(const G4String &aSuperSubDriverName,
						 G4bool runMysqlScript,
						 G4String scriptName)
  : _runMysqlScript(runMysqlScript), _scriptName(scriptName)
{
  if(aSuperSubDriverName == "") 
    {
      Control::Abort("VSuperSubDetectorDriver has to have a valid name!",
	MOKKA_OTHER_ERRORS);
    }
  theDriverName=aSuperSubDriverName;
  CGAGeometryManager::
    GetCGAGeometryManager()->RegisterGeometryDriver(this);
}
