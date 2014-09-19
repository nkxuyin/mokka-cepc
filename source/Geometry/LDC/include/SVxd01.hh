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
// $Id: SVxd01.hh,v 1.2 2005/12/06 10:35:55 adrian Exp $
// $Name: mokka-07-00 $
//
// F.Gaede, DESY, 09/2005
// Superdriver for the vertex detector that starts from
// a copy of the VXD driver's database
//

#ifndef SVxd01_h
#define SVxd01_h 1

class Database;
#include "VSuperSubDetectorDriver.hh"

class SVxd01 : public VSuperSubDetectorDriver
{
 public:
  
  SVxd01() : VSuperSubDetectorDriver("SVxd01")
  {}
  
  ~SVxd01(){}
  
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);  
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
  
 private:
};

#endif


