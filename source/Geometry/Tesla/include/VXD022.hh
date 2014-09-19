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
// $Id: VXD022.hh,v 1.3 2008/10/06 12:58:35 musat Exp $
// $Name: mokka-07-00 $
//
// author D.Grandjean, IRes, 09/2005
//
#ifndef VXD022_h
#define VXD022_h 1

class G4LogicalVolume;
class Database;
class TRKSiSD00;

#include "Control.hh"
#include "VSubDetectorDriver.hh"
#include "G4Material.hh"

class VXD022 : public VSubDetectorDriver
{
public:
  VXD022() : VSubDetectorDriver("vxd022","vxd"), 
	    db(0),theVXDSD(0),theVXDElectSD(0)
  {}

  ~VXD022();
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);
 
#ifdef MOKKA_GEAR
  void GearSetup();
#endif
 
private:
  Database* db;
#ifdef LCIO_MODE
  DoubleVec ladder_gapVec;
  DoubleVec StripLineFinalZ_Vec;
#endif
  G4double electronics_structure_thickness;
  G4double end_electronics_half_z; 
  G4double strip_lines_thickness;
  G4double strip_final_beampipe_radious;
  G4double support_endplate_inner_radious;
  G4double rAlu;
  G4double drAlu;
  G4double rInner;
  G4double aluEndcapZ;
  G4double aluHalfZ;
  G4Material *aluMaterial; 
  G4Material *activeMaterial;
  G4Material *supportMaterial;
  G4Material *stripMaterial;
  TRKSiSD00 *theVXDSD;
  TRKSiSD00 *theVXDElectSD;

#ifdef MOKKA_GEAR

  struct helpLayer {
    G4double distance ;
    G4double offset ;
    G4double thickness ;
    G4double length ;
    G4double width ;
    G4double radLength ;
  };

#endif

};

#endif


