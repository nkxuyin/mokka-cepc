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
// $Id: TPC00.hh,v 1.2 2007/08/13 08:12:42 kristian Exp $
// $Name: mokka-07-00 $
//
#ifndef TPC00_h
#define TPC00_h 1

class G4LogicalVolume;
class Database;
class TPCSD00;

#include "VSubDetectorDriver.hh"
#include "G4Material.hh"


class TPC00 : public VSubDetectorDriver
{
public:
  TPC00() : VSubDetectorDriver("tpc00","tpc"),
	    theTPCSD(0)  {}
  ~TPC00();
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);

#ifdef MOKKA_GEAR
  void GearSetup();
#endif

private:
  G4double inner_radius;
  G4double outer_radius;
  G4double z_half_length;
  G4double endplate_thickness;
  G4double inner_wall_thickness;
  G4double outer_wall_thickness;
  G4double inner_sensitive;
  G4double outer_sensitive;
  G4double LayerThickness;
  G4int number_of_layers;
  G4double TPCWallProperties_RadLen;
  G4double TPCWallProperties_dEdx;
  G4double TPCGasProperties_RadLen;
  G4double TPCGasProperties_dEdx;
  G4double Ar_IonPotential;
  G4Material *wallMat;
  TPCSD00* theTPCSD;
};

#endif


