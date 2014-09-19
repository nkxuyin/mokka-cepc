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
// $Id: TPC01.hh,v 1.2 2007/08/13 08:12:42 kristian Exp $
// $Name: mokka-07-00 $
//
#ifndef TPC01_h
#define TPC01_h 1

class G4LogicalVolume;
class Database;
class TRKSD00;

#include "VSubDetectorDriver.hh"
#include "G4Material.hh"
class TPC01 : public VSubDetectorDriver
{
public:
  TPC01() : VSubDetectorDriver("tpc01","tpc"),
	    theTPCSD(0)  {}
  ~TPC01();
  
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
  //  G4PVPlacement *Phys;
  

  TRKSD00* theTPCSD;
  TRKSD00* theFCHSD;
};

#endif


