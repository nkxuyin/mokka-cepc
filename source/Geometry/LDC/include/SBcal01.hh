/*
 * SBcal01 SuperDriver for Mokka 
 *
 * SBcal01.hh - superdriver class header
 * 
 * A.Hartin Oct 2008 - adapted from Lcal
 */

#ifndef SBcal01_hh
#define SBcal01_hh 1


class Database;
#include "VSuperSubDetectorDriver.hh"

class SBcal01 : public VSuperSubDetectorDriver
{
 public:
  
  SBcal01();
  ~SBcal01();
    
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
};

#endif

// EOF
