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
// $Id: SiDBar00.hh,v 1.5 2005/08/03 16:27:03 mora Exp $
// $Name: mokka-07-00 $
//
#ifndef SiDBar00_h
#define SiDBar00_h 1

class G4LogicalVolume;
class Database;
class TRKSD00;

#include "Control.hh"
#include "VSubDetectorDriver.hh"

class SiDBar00 : public VSubDetectorDriver
{
public:
  SiDBar00() : VSubDetectorDriver("SiDBar00","SiDBar"), 
	    db(0),theSiDBarSD(0)
  {}

  ~SiDBar00();
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);
  
private:
  Database* db;
  TRKSD00 *theSiDBarSD;
};

#endif


