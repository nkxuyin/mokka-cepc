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
// $Id: SET00.hh,v 1.1 2003/07/18 09:05:13 musat Exp $
// $Name: mokka-07-00 $
//
#ifndef SET00_h
#define SET00_h 1

class G4LogicalVolume;

#include "VSubDetectorDriver.hh"

class SET00 : public VSubDetectorDriver
{
public:
  SET00() : VSubDetectorDriver("set00") {}
  ~SET00();
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);

private:
  //  SETSD00* theSETSD;
};

#endif


