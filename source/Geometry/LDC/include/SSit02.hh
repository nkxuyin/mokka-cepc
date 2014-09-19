/*
 * SIT SuperDriver for Mokka 
 *
 * SSit02.hh - superdriver class header
 * 
 */

#ifndef SSIT02_hh
#define SSIT02_hh 1


class Database;
#include "VSuperSubDetectorDriver.hh"

class SSit02 : public VSuperSubDetectorDriver
{
 public:
  
  SSit02();
  ~SSit02();
    
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
};

#endif

