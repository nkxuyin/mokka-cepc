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
// $Id: Sit00.hh,v 1.3 2008/07/03 14:35:26 musat Exp $
// $Name: mokka-07-00 $
//
#ifndef Sit00_h
#define Sit00_h 1

class G4LogicalVolume;
class Database;
class TRKSD00;

#include "Control.hh"
#include "VSubDetectorDriver.hh"
#include "G4Material.hh"

class Sit00 : public VSubDetectorDriver
{
public:
  Sit00() : VSubDetectorDriver("sit00","sit"), 
	    db(0),theSITSD(0)
  {}

  ~Sit00();
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);

#ifdef MOKKA_GEAR
  void GearSetup();
#endif 
 
private:
  Database* db;
#ifdef LCIO_MODE
  DoubleVec  inner_radiusVec;
  DoubleVec  half_zVec;
#endif
  G4double thickness;
  G4Material *SITMat;
  TRKSD00 *theSITSD;
};

#endif


