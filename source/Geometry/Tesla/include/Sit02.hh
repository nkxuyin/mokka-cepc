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
// $Id: Sit02.hh,v 1.4 2008/03/18 17:49:09 steve Exp $
// $Name: mokka-07-00 $
//
#ifndef Sit02_h
#define Sit02_h 1

class G4LogicalVolume;
class Database;
class TRKSD00;

#include "Control.hh"
#include "VSubDetectorDriver.hh"
#include "G4Material.hh"

class Sit02 : public VSubDetectorDriver
{
public:
  Sit02() : VSubDetectorDriver("sit02","sit"), 
	    db(0),theSITSD(0)
  {}

  ~Sit02();
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);
#ifdef MOKKA_GEAR
  void GearSetup();
#endif

private:
  Database* db;
  std::vector<double> inner_radiusVec ;
  std::vector<double> half_zVec ;
  std::vector<double>  support_radiusVec;
  std::vector<double>  support_half_zVec;
  G4double sensitive_thickness;
  G4double support_thickness;
  G4Material *SITMat;
  G4Material *SupportMat;
  TRKSD00 *theSITSD;
};

#endif


