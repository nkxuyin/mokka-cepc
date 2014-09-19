/*
 * FTD SuperDriver for Mokka 
 *
 * SFtd01.hh - superdriver class header
 * 
 * M.Kapolka 2007
 */

#ifndef SFTD01_hh
#define SFTD01_hh 1


class Database;
#include "VSuperSubDetectorDriver.hh"

class SFtd01 : public VSuperSubDetectorDriver
{
 public:
  
  SFtd01();
  ~SFtd01();
    
  virtual G4bool PreLoadScriptAction(Database* ,
				     CGAGeometryEnvironment&);
  G4bool PostLoadScriptAction(Database* ,
			      CGAGeometryEnvironment&);
};

#endif

// EOF
