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
// $Id: Coil01.hh,v 1.2 2006/02/14 15:28:25 mora Exp $
// $Name:  $
//
// History
// first implementation from Mokka08, 
// P. Mora de Freitas (mora@poly.in2p3.fr), July 2001
//

#ifndef Coil01_h
#define Coil01_h 1

class G4LogicalVolume;
class Database;
class TRKSD00;

#include "Control.hh"
#include "VSubDetectorDriver.hh"
#include "G4Material.hh"

class Coil01 : public VSubDetectorDriver
{

public:
  Coil01() : VSubDetectorDriver("coil01","coil"){}
  ~Coil01() {};
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);
private:
  Database* db;
  TRKSD00 *theCoilSD;

};

#endif


