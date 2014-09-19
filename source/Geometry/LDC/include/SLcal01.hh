/*
 * SLcal01 SuperDriver for Mokka 
 *
 * SLcal01.hh - superdriver class header
 * 
 * M.Kapolka 2006
 */

#ifndef SLcal01_hh
#define SLcal01_hh 1


class Database;
#include "VSuperSubDetectorDriver.hh"

class SLcal01 : public VSuperSubDetectorDriver
{
 public:
  
  SLcal01();
  ~SLcal01();
    
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
};

#endif

// EOF
