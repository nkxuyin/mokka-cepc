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
// $Id: STube01.hh,v 1.2 2005/12/06 10:35:55 adrian Exp $
// $Name: mokka-07-00 $
//
// D.Grandjean, IReS, 10/2005
// Superdriver for the beampipe
//

#ifndef STube01_h
#define STube01_h 1

class Database;
#include "VSuperSubDetectorDriver.hh"

class STube01 : public VSuperSubDetectorDriver
{
 public:
  
  STube01() : VSuperSubDetectorDriver("STube01")
  {}
  
  ~STube01(){}
  
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);  
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
  
 private:
};

#endif


