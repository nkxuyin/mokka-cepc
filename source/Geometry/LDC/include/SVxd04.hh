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
// $Id: SVxd04.hh,v 1.1 2008/02/26 09:52:23 damien Exp $
// $Name: mokka-07-00 $
//
// F.Gaede, DESY, 09/2005
// Superdriver for the vertex detector that starts from
// a copy of the VXD driver's database
//

#ifndef SVxd04_h
#define SVxd04_h 1

class Database;
#include "VSuperSubDetectorDriver.hh"

class SVxd04 : public VSuperSubDetectorDriver
{
 public:
  
  SVxd04() : VSuperSubDetectorDriver("SVxd04")
  {}
  
  ~SVxd04(){}
  
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);  
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
  
 private:
};

#endif


