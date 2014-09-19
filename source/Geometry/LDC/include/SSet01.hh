/*
 * SET SuperDriver for Mokka 
 *
 * SSet01.hh - superdriver class header
 * 
 */

#ifndef SFTD01_hh
#define SFTD01_hh 1


class Database;
#include "VSuperSubDetectorDriver.hh"

class SSet01 : public VSuperSubDetectorDriver
{
 public:
  
  SSet01();
  ~SSet01();
    
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
};

#endif

