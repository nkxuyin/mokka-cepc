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
// $Id: Hcal03.hh,v 1.3 2007/12/20 11:37:21 kristian Exp $
// $Name: mokka-07-00 $
//

#ifndef Hcal03_h
#define Hcal03_h 1

class G4LogicalVolume;
class Database;
class G4VSolid;

class SD;
class HECSD;
class G4UserLimits;

#include "VSubDetectorDriver.hh"

class G4Polyhedra;
class G4Material;

class Hcal03 : public VSubDetectorDriver
{
public:

  Hcal03() : VSubDetectorDriver("hcal03","hcal"),
	     theBarrilRegSD(0),theBarrilEndSD(0),
	     theENDCAPEndSD(0),g10_thickness(0), 
	     glass_thickness(0), gas_thickness(0), 
	     spacer_thickness(0), spacer_gap(0),
	     theMaxStepAllowed(DBL_MAX)
  {}
  ~Hcal03();
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);

  
private:
  G4LogicalVolume *EnvLogHcalModuleBarrel;
  G4LogicalVolume *EnvLogHcalModuleEndCap;
  G4String SensitiveModel;
  Database* db;

  // To reload hits for visualisation
  void LoadEvent(FILE* theSubDetectorEventHitsFileInput);

  // Barrel
  void Barrel(G4LogicalVolume*);
  void BarrelRegularModules(G4LogicalVolume*);
  void BarrelEndModules(G4LogicalVolume*);
  void BarrelRegularChambers(G4LogicalVolume*, G4double chambers_y_off_correction);
  void BarrelEndChambers(G4LogicalVolume*, G4double chambers_y_off_correction);

  // EndCaps
  void Endcaps(G4LogicalVolume*);
  void EndcapChambers(G4LogicalVolume*);

  // RPC1
  G4LogicalVolume * BuildRPC1Box(G4Box* ChamberSolid, 
				 SD* theSD, 
				 G4int layer_id,
				 G4UserLimits* pULimits);
  G4LogicalVolume * BuildRPC1Polyhedra(G4Polyhedra* ChamberSolid, 
				       SD* theSD,
				       G4double phiStart,
				       G4double phiTotal,
				       G4int numSide,
				       G4int numZPlanes,
				       const G4double zPlane[],
				       const G4double rInner[],
				       const G4double rOuter[],
				       G4UserLimits* pULimits);

  SD* theBarrilRegSD;
  SD* theBarrilEndSD;
  HECSD* theENDCAPEndSD;
  
  G4double  g10_thickness, glass_thickness, 
    gas_thickness, spacer_thickness, 
    spacer_gap;

  G4Material * RadiatorMaterial;

  G4double theMaxStepAllowed;
#ifdef MOKKA_GEAR
  // MokkaGear
    
  struct helpParameters{
    G4double innerRadius ;
    G4double outerRadius ;
    G4double zMax ;
    G4double phi0 ;
    std::vector<double> layerPos ;
    std::vector<double> layerPos2 ;
    std::vector<double> sensThickness ;
    std::vector<double> gapThickness ;
    G4int count ;
    G4double leastZ ;
    G4double mostZ ;
  };
 
  helpParameters helpBarrel;
  helpParameters helpEndcap;

  // parameters

  G4int    intParamEndModuleType ;
  G4double dblParamHalfZ ;
  G4double dblParamLateralStructureThickness ;
  G4double dblParamModulesGap ;
  G4double dblParamStavesGap ;
  G4double dblParamBackPlateThickness ;
  G4double dblParamBarrelMostZ ;
  G4double dblParamDigiTileSize ;

#endif

};

#endif


