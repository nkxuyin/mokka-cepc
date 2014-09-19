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
// $Id: Hodoscope00.hh,v 1.1 2003/07/18 09:04:58 musat Exp $
// $Name: mokka-07-00 $
//
#ifndef Hodoscope00_h
#define Hodoscope00_h 1

class G4LogicalVolume;
class Database;
class G4Material;
class HodoscopeSD00;

#include "VSubDetectorDriver.hh"

class Hodoscope00 : public VSubDetectorDriver
{
public:
  Hodoscope00() : VSubDetectorDriver("hodoscope00","hodoscope"),
	     theHodoscopeSD(0) {}
  
  ~Hodoscope00();

  G4bool construct(const G4String &aSubDetectorDBName,
			    G4LogicalVolume *WorldLog);

private:
  
  G4Material* Mix;
  HodoscopeSD00 *theHodoscopeSD;
};

#endif


