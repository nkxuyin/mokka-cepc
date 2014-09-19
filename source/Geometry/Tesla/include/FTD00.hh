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
// $Id: FTD00.hh,v 1.3 2008/07/03 14:35:26 musat Exp $
// $Name: mokka-07-00 $
//
#ifndef FTD00_h
#define FTD00_h 1

class G4LogicalVolume;
class Database;
class TRKSD00;

#include "Control.hh"
#include "VSubDetectorDriver.hh"
#include "G4Material.hh"

class FTD00 : public VSubDetectorDriver
{
public:
  FTD00() : VSubDetectorDriver("ftd00","ftd"), 
	    db(0),theFTDSD(0)
  {}

  ~FTD00();
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);
#ifdef MOKKA_GEAR    
  void GearSetup();
#endif

private:
  Database* db;
#ifdef LCIO_MODE
  DoubleVec z_positionVec;
  DoubleVec inner_radiusVec;
  DoubleVec outer_radiusVec;
#endif
  G4double Disks_Si_thickness;
  G4double inner_support_length;
  G4double inner_support_thickness;
  G4double outer_support_thickness; 
  G4double  outer_support_length,outer_cylinder_total_thichness,cables_thichness;
  G4double ZStartOuterCylinder,ZstopOuterCylinder;
  G4double ZStartInnerCylinder,ZstopInnerCylinder;
  G4Material *SiMat;
  G4Material *Si872Mat;
  G4Material *KaptonMat;
  G4Material *CuMat;
  G4int LastHeavyLayer;

  TRKSD00 *theFTDSD;
};

#endif


