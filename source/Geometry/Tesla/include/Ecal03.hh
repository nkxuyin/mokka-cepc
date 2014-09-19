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
// $Id: Ecal03.hh,v 1.3 2007/08/13 08:12:42 kristian Exp $
// $Name: mokka-07-00 $
//
#ifndef Ecal03_h
#define Ecal03_h 1

#include <map>
#include <utility>

class G4LogicalVolume;
class Database;
class G4VSolid;

class SD03;
class ECSD03;

#include "VSubDetectorDriver.hh"

class Ecal03 : public VSubDetectorDriver
{
public:
  Ecal03() : VSubDetectorDriver("ecal03","ecal"), 
	     theBarrelCellSD(0),theBarrelGRSD(0),theEndCapCellSD(0),
	     theEndCapGRSD(0),theMaxStepAllowed(DBL_MAX)
  {}

  ~Ecal03();
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);

  // To reload hits for visualisation
  void LoadEvent(FILE* theSubDetectorEventHitsFileInput);

private:

  // Barrel
  void BarrelStandardModule(G4LogicalVolume*);
  void BarrelWPlate(G4LogicalVolume*);
  void BarrelAlveolusModule(G4LogicalVolume*);
  // EndCaps
  void EndcapStandardModule(G4LogicalVolume*);
  void EndcapWPlate(G4LogicalVolume*);
  void EndcapAlveolusModule(G4LogicalVolume*);
  void EndcapAlveolusPads(G4LogicalVolume*);

  G4VSolid* BuildECShape(G4double L,
			 G4double dim_x,
			 G4double dim_y,
			 G4double dim_z);

  G4LogicalVolume * BuildWafer(char theDetector,
		G4int n_cell_x, G4int n_cell_z);
	  
  void FillWafer(G4LogicalVolume * theWafer, 
		 G4int n_cell_x, G4int n_cell_z,
		 VSensitiveDetector*theSD);

  void FillBarrelPlane(G4LogicalVolume * thePlane,
		G4double dimx, G4double dimz);

  void FillEndcapPlane(G4LogicalVolume * thePlane, G4double L, 
		  G4double dimx, G4double dimy);

  //#####################################################
  // NO MORE AL PLATES FOR THE ECAL ENDCAPS WITH ecal02
  // ####################################################
  //void EndcapAlPlate(G4LogicalVolume*);

  G4LogicalVolume *EnvLogEcalModuleBarrel;
  G4LogicalVolume *EnvLogEcalModuleEndCap;
  G4VisAttributes *VisAttSi;
  std::map<G4String, G4LogicalVolume *> theBarrelWafersMap,theEndcapWafersMap;
  Database* db;
  SD03* theBarrelCellSD;
  SD03* theBarrelGRSD;
  ECSD03* theEndCapCellSD;
  ECSD03* theEndCapGRSD;
  G4double theMaxStepAllowed;
  G4double cell_dim_x, cell_dim_z, si_thickness, guard_ring_size;
  G4double inter_wafer_gap; 
  G4int nmax_cell_x, nmax_cell_z, n_guard_ring_zones;

#ifdef MOKKA_GEAR
  // MokkaGear
  
  struct helpParameters{
    G4double innerRadius;
    G4double outerRadius;
    G4double zMax;
    G4double phi0;
    std::vector<double> layerPos;
    std::vector<double> radiThickness;
    G4int count;
    G4double leastZ;
    G4double mostZ;
  };
  
  helpParameters helpBarrel;
  helpParameters helpEndcap;
  
#endif


};

#endif


