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
// $Id: Coil00.hh,v 1.2 2006/02/14 15:28:25 mora Exp $
// $Name: mokka-07-00 $
//
// History
// first implementation from Mokka08, 
// P. Mora de Freitas (mora@poly.in2p3.fr), July 2001
//

#ifndef Coil00_h
#define Coil00_h 1

class G4LogicalVolume;
class Database;

#include "VSubDetectorDriver.hh"

class Coil00 : public VSubDetectorDriver
{
public:
  Coil00() : VSubDetectorDriver("coil00","coil"){}

  ~Coil00() {};
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);
private:
};

#endif


