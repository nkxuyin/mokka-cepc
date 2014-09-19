/*
 * SLcal03 SuperDriver for Mokka 
 *
 * SLcal03.hh - superdriver class header
 * 
 */

#ifndef SLcal03_hh
#define SLcal03_hh 1


class Database;
#include "VSuperSubDetectorDriver.hh"

class SLcal03 : public VSuperSubDetectorDriver
{
 public:
  
  SLcal03();
  ~SLcal03();
    
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
};

#endif

// EOF
