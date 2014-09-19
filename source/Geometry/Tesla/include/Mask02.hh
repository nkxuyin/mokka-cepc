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
// $Id: Mask02.hh,v 1.3 2006/02/14 15:28:25 mora Exp $
// $Name: mokka-07-00 $
//
#ifndef Mask02_h
#define Mask02_h 1

class G4LogicalVolume;
class Database;

#include "VSubDetectorDriver.hh"

class Mask02 : public VSubDetectorDriver
{
public:
  Mask02() : VSubDetectorDriver("mask02","mask"), 
	   db(0)
  {}

  ~Mask02() {};
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);
  
private:

  Database* db;
};

#endif


