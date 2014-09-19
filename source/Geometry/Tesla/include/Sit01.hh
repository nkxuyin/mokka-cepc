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
// $Id: Sit01.hh,v 1.6 2008/07/03 14:35:26 musat Exp $
// $Name: mokka-07-00 $
//
#ifndef Sit01_h
#define Sit01_h 1

class G4LogicalVolume;
class Database;
class TRKSD00;

#include "Control.hh"
#include "VSubDetectorDriver.hh"
#include "G4Material.hh"

class Sit01 : public VSubDetectorDriver
{
public:
  Sit01() : VSubDetectorDriver("sit01","sit"), 
	    db(0),theSITSD(0)
  {}

  ~Sit01();
  
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
  DoubleVec  support_radiusVec;
  DoubleVec  support_half_zVec;
#endif
  G4double sensitive_thickness;
  G4double support_thickness;
  G4Material *SITMat;
  G4Material *SupportMat;
  TRKSD00 *theSITSD;
};

#endif


