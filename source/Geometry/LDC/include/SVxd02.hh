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
// $Id: SVxd02.hh,v 1.1 2008/01/28 17:23:32 damien Exp $
// $Name: mokka-07-00 $
//
// F.Gaede, DESY, 09/2005
// Superdriver for the vertex detector that starts from
// a copy of the VXD driver's database
//

#ifndef SVxd02_h
#define SVxd02_h 1

class Database;
#include "VSuperSubDetectorDriver.hh"

class SVxd02 : public VSuperSubDetectorDriver
{
 public:
  
  SVxd02() : VSuperSubDetectorDriver("SVxd02")
  {}
  
  ~SVxd02(){}
  
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);  
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
  
 private:
};

#endif


