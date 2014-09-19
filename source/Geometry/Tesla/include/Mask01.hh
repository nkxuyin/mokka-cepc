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
// $Id: Mask01.hh,v 1.2 2006/02/14 15:28:25 mora Exp $
// $Name: mokka-07-00 $
//
#ifndef Mask01_h
#define Mask01_h 1

class G4LogicalVolume;
class Database;

#include "VSubDetectorDriver.hh"

class Mask01 : public VSubDetectorDriver
{
public:
  Mask01() : VSubDetectorDriver("mask01","mask"), 
	   db(0)
  {}

  ~Mask01() {};
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);
  
private:

  Database* db;
};

#endif


