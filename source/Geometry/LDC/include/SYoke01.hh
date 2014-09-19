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
// $Id: SYoke01.hh,v 1.1 2005/07/27 12:09:28 mora Exp $
// $Name: mokka-07-00 $
//

#ifndef SYoke01_h
#define SYoke01_h 1

class Database;
#include "VSuperSubDetectorDriver.hh"

class SYoke01 : public VSuperSubDetectorDriver
{
 public:
  
  SYoke01() : VSuperSubDetectorDriver("SYoke01")
  {}
  
  ~SYoke01(){}
  
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);
  
 private:
};

#endif


