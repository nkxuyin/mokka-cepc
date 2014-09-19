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
// $Id: SCoil02.hh,v 1.1 2008/10/05 18:33:39 frank Exp $
// $Name: mokka-07-00 $
//

#ifndef SCoil02_h
#define SCoil02_h 1

class Database;
#include "VSuperSubDetectorDriver.hh"

class SCoil02 : public VSuperSubDetectorDriver
{
 public:
  
  SCoil02() : VSuperSubDetectorDriver("SCoil02")
  {}
  
  ~SCoil02(){}
  
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
  
 private:
};

#endif


