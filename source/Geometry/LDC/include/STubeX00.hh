/*
 * TubeX00 SuperDriver for Mokka
 *
 * STubeX00.hh - superdriver class
 * M.Kapolka 2007
 * 
 */

#ifndef STubeX00_h
#define STubeX00_h 1

class Database;
#include "VSuperSubDetectorDriver.hh"

class STubeX00 : public VSuperSubDetectorDriver
{
 public:
  
  STubeX00() : VSuperSubDetectorDriver("STubeX00")  { }
  
  ~STubeX00(){ }
  
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);  
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
 private:
 
  G4double actual_opening_angle;
  G4double actual_lcal_rin;
};

#endif


