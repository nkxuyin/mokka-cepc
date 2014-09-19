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
// $Id: SHcal01.hh,v 1.4 2006/03/20 13:59:59 mora Exp $
// $Name: mokka-07-00 $
//

#ifndef SHcal01_h
#define SHcal01_h 1

class Database;
#include "VSuperSubDetectorDriver.hh"

class SHcal01 : public VSuperSubDetectorDriver
{
 public:
  
  SHcal01() : VSuperSubDetectorDriver("SHcal01",true,"SHcal01v3")
  {}
  
  ~SHcal01(){}
  
  G4bool PreLoadScriptAction(Database* ,
			     CGAGeometryEnvironment&);
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
  
 private:
};

#endif


