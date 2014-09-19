/*
 * FieldX00 SuperDriver for Mokka 
 *
 * SFieldX00.hh - superdriver class header
 * 
 * M.Kapolka 2007
 */

#ifndef STubeX00_h
#define STubeX00_h 1

class Database;
#include "VSuperSubDetectorDriver.hh"

class SFieldX00 : public VSuperSubDetectorDriver
{
 public:
  
  SFieldX00() : VSuperSubDetectorDriver("SFieldX00")  { }
  
  ~SFieldX00(){ }
  
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);  
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
 private:
};

#endif


