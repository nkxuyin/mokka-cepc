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
// $Id: SCoil01.hh,v 1.1 2005/07/27 12:09:28 mora Exp $
// $Name: mokka-07-00 $
//

#ifndef SCoil01_h
#define SCoil01_h 1

class Database;
#include "VSuperSubDetectorDriver.hh"

class SCoil01 : public VSuperSubDetectorDriver
{
 public:
  
  SCoil01() : VSuperSubDetectorDriver("SCoil01")
  {}
  
  ~SCoil01(){}
  
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
  
 private:
};

#endif


