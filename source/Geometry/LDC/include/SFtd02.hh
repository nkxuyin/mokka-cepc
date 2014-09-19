/*
 * FTD SuperDriver for Mokka 
 *
 */

#ifndef SFTD02_hh
#define SFTD02_hh 1


class Database;
#include "VSuperSubDetectorDriver.hh"

class SFtd02 : public VSuperSubDetectorDriver
{
 public:
  
  SFtd02();
  ~SFtd02();
    
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
};

#endif
