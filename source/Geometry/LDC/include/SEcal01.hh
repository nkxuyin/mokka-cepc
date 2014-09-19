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
// $Id: SEcal01.hh,v 1.4 2006/07/10 09:07:14 mora Exp $
// $Name: mokka-07-00 $
//

#ifndef SEcal01_h
#define SEcal01_h 1

class Database;
#include "VSuperSubDetectorDriver.hh"

class SEcal01 : public VSuperSubDetectorDriver
{
 public:
  
  SEcal01() : VSuperSubDetectorDriver("SEcal01",true,"SEcal01v3")
  {}
  
  ~SEcal01(){}
  
  G4bool PreLoadScriptAction(Database* ,
			     CGAGeometryEnvironment&);
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
  
 private:
};

#endif


