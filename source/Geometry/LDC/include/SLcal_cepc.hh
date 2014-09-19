/*
 * SLcal_cepc SuperDriver for Mokka 
 *
 * SLcal_cepc.hh - superdriver class header
 * 
 */

#ifndef SLcal_cepc_hh
#define SLcal_cepc_hh 1


class Database;
#include "VSuperSubDetectorDriver.hh"

class SLcal_cepc : public VSuperSubDetectorDriver
{
 public:
  
  SLcal_cepc();
  ~SLcal_cepc();
    
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
};

#endif

// EOF
