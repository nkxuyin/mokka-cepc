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
// $Id: SField01.hh,v 1.1 2005/07/27 12:09:28 mora Exp $
// $Name: mokka-07-00 $
//

#ifndef SField01_h
#define SField01_h 1

class Database;
#include "VSuperSubDetectorDriver.hh"

class SField01 : public VSuperSubDetectorDriver
{
 public:
  
  SField01() : VSuperSubDetectorDriver("SField01")
  {}
  
  ~SField01(){}
  
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);

  virtual G4bool PostLoadScriptAction(Database* ,
                                      CGAGeometryEnvironment&);

 private:
};

#endif


