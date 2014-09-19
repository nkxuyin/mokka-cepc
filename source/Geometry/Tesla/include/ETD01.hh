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
#ifndef ETD01_h
#define ETD01_h 1

class G4LogicalVolume;
class Database;
class TRKSD00;

#include "Control.hh"
#include "VSubDetectorDriver.hh"
#include "G4Material.hh"

class ETD01 : public VSubDetectorDriver
{
  //public:
  //  ETD01() : VSubDetectorDriver("ETD01","ETD"), 
  //	    db(0),theSETSD(0)
  //  {}
  //  ~ETD01();

public:
  ETD01() : VSubDetectorDriver("etd01","etd"),
          db(0),theETDSD(0)
  {}
  ~ETD01();
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);
#ifdef MOKKA_GEAR
  void GearSetup();
#endif

private:
  Database* db;
    std::vector<double> inner_radiusVec ;
    std::vector<double> outer_radiusVec ;
    std::vector<double> half_zVec ;
    std::vector<double> dzVec ;
  G4double sensitive_thickness;
  G4double support_thickness;
  G4Material *ETDMat;
  TRKSD00 *theETDSD;
};

#endif


