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
// $Id: SVxd03.hh,v 1.1 2008/02/26 09:52:23 damien Exp $
// $Name: mokka-07-00 $
//
// F.Gaede, DESY, 09/2005
// Superdriver for the vertex detector that starts from
// a copy of the VXD driver's database
//

#ifndef SVxd03_h
#define SVxd03_h 1

class Database;
#include "VSuperSubDetectorDriver.hh"

class SVxd03 : public VSuperSubDetectorDriver
{
 public:
  
  SVxd03() : VSuperSubDetectorDriver("SVxd03")
  {}
  
  ~SVxd03(){}
  
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);  
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
  
 private:
};

#endif


