/*
 * SLcal02 SuperDriver for Mokka 
 *
 * SLcal02.hh - superdriver class header
 * 
 * M.Kapolka 2006
 */

#ifndef SLcal02_hh
#define SLcal02_hh 1


class Database;
#include "VSuperSubDetectorDriver.hh"

class SLcal02 : public VSuperSubDetectorDriver
{
 public:
  
  SLcal02();
  ~SLcal02();
    
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
};

#endif

// EOF
