/*
 * SMaskX00 SuperDriver for Mokka
 *
 * SMaskX00.hh - superdriver class
 * M.Kapolka 2007
 * 
 */

#ifndef SMaskX00_h
#define SMaskX00_h 1

class Database;
#include "VSuperSubDetectorDriver.hh"

class SMaskX00 : public VSuperSubDetectorDriver
{
 public:
  
  SMaskX00() : VSuperSubDetectorDriver("SMaskX00")  { }
  
  ~SMaskX00(){ }
  
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);  
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
};

#endif


