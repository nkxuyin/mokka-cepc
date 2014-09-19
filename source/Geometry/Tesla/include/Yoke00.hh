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
// $Id: Yoke00.hh,v 1.2 2006/02/14 15:28:25 mora Exp $
// $Name: mokka-07-00 $
//
#ifndef Yoke00_h
#define Yoke00_h 1

class G4LogicalVolume;
class Database;

#include "VSubDetectorDriver.hh"

class Yoke00 : public VSubDetectorDriver
{
public:
  Yoke00() : VSubDetectorDriver("yoke00","yoke"){}

  ~Yoke00() {};
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);
private:
};

#endif


