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
//
#ifndef SET01_h
#define SET01_h 1

class G4LogicalVolume;
class Database;
class TRKSD00;

#include "Control.hh"
#include "VSubDetectorDriver.hh"
#include "G4Material.hh"

class SET01 : public VSubDetectorDriver
{
public:
SET01() : VSubDetectorDriver("set01","SET"), 
	    db(0),theSETSD(0)
  {}
  ~SET01();
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);
#ifdef MOKKA_GEAR
  void GearSetup();
#endif

private:
  Database* db;
  std::vector<double> radiusVec;
  std::vector<double> half_zVec;
  std::vector<double> supportRadiusVec;
  G4double sensitive_thickness;
  G4double support_thickness;
  G4Material *SETMat;
  G4Material *SETSupport;
  TRKSD00 *theSETSD;
};

#endif


