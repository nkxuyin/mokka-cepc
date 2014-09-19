//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for the Linear   *
//*   collider detector studies.                        *
//*                                                     *
//* For more information about Mokka, please, go to the *
//*                                                     *
//*  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
//*                                                     *
//* Mokka home page.                                    *
//*                                                     *
//*******************************************************
//
// $Id: VSuperSubDetectorDriver.hh,v 1.3 2005/08/09 07:31:07 mora Exp $
// $Name: mokka-07-00 $
//
#ifndef VSuperSubDetectorDriver_h
#define VSuperSubDetectorDriver_h 1

#include "globals.hh"
#include "CGAGeometryEnvironment.hh"

class Database;

class VSuperSubDetectorDriver
{
public:
  VSuperSubDetectorDriver(const G4String &aSuperDriverName,
			  G4bool runMysqlScript = false,
			  G4String scriptName = "");
  
  virtual ~VSuperSubDetectorDriver() {};
  
  virtual G4bool IsApplicable(const G4String &aSuperDriverName) const
  { return (theDriverName == aSuperDriverName); }
  
  
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&)
  {return true;}
  
  virtual G4bool PostLoadScriptAction(Database* ,
				      CGAGeometryEnvironment&) 
  {return true;}
  
  virtual G4bool CheckParameters(CGAGeometryEnvironment&) 
  {return true;}

  G4String GetName() const 
  { return theDriverName; }

  G4bool NeedRunMysqlScript() const 
  { return _runMysqlScript; }

  G4String GetStringName () const
  {
    if(_scriptName == "") return theDriverName;
    else return _scriptName;
  }

private:

  G4String theDriverName;
  G4bool _runMysqlScript;
  G4String _scriptName;
};

#endif


