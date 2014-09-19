//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for Tesla -      *
//*                                                     *
//* For more information about Mokka, please, go to the *
//*                                                     *
//*  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
//*                                                     *
//* Mokka home page.                                    *
//*                                                     *
//*******************************************************
//
// $Id: SiDFwd00.hh,v 1.3 2005/08/03 16:27:03 mora Exp $
// $Name: mokka-07-00 $
//

#ifndef SiDFwd00_h
#define SiDFwd00_h 1

class G4LogicalVolume;
class Database;
class TRKSD00;

#include "Control.hh"
#include "VSubDetectorDriver.hh"

class SiDFwd00 : public VSubDetectorDriver

{

public:
  SiDFwd00() : VSubDetectorDriver("SiDFwd00","SiDFwd"), 
	    db(0),theSiDFwd00SD(0)
  {}

  ~SiDFwd00();
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);
  
private:
  Database* db;
  TRKSD00 *theSiDFwd00SD;
};

#endif


