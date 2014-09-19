/*
 * ETD SuperDriver for Mokka 
 *
 * SEtd01.hh - superdriver class header
 * 
 */

#ifndef SETD01_hh
#define SETD01_hh 1


class Database;
#include "VSuperSubDetectorDriver.hh"

class SEtd01 : public VSuperSubDetectorDriver
{
 public:
  
  SEtd01();
  ~SEtd01();
    
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
};

#endif

