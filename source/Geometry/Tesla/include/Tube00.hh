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
// $Id: Tube00.hh,v 1.3 2007/08/13 08:12:42 kristian Exp $
// $Name: mokka-07-00 $
//
#ifndef Tube00_h
#define Tube00_h 1

class G4LogicalVolume;
class Database;

#include "VSubDetectorDriver.hh"
#include "G4Material.hh"

class Tube00 : public VSubDetectorDriver
{
public:
  Tube00() : VSubDetectorDriver("tube00","tube"), 
	   db(0)
  {}

  ~Tube00() {};
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);

#ifdef MOKKA_GEAR
  void GearSetup();
#endif

private:
  Database* db;
  G4double central_half_z,central_inner_radious,
    central_thickness;
  G4Material *BeamMaterial;
};

#endif


