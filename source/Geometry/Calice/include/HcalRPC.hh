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
// $Id: HcalRPC.hh,v 1.1 2004/07/23 15:09:08 musat Exp $
// $Name: mokka-07-00 $
//

#ifndef HcalRPC_h
#define HcalRPC_h 1

class G4LogicalVolume;
class Database;
class G4VSolid;

class SD02;
//class HECSD;
class G4UserLimits;

#include "VSubDetectorDriver.hh"

class G4Polyhedra;
class G4Material;

class HcalRPC : public VSubDetectorDriver
{
public:

  HcalRPC() : VSubDetectorDriver("hcalrpc","hcal"),
	     theBarrilRegSD(0),//theBarrilEndSD(0),
	     //theENDCAPEndSD(0),
  	     g10_thickness(0), 
	     glass_thickness(0), gas_thickness(0), 
	     spacer_thickness(0), spacer_gap(0),
	     theMaxStepAllowed(DBL_MAX)
  {}
  ~HcalRPC();
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);

  
private:
  G4LogicalVolume *EnvLogHcalModuleBarrel;
  //G4LogicalVolume *EnvLogHcalModuleEndCap;
  G4String SensitiveModel;
  Database* db;

  // To reload hits for visualisation
  void LoadEvent(FILE* theSubDetectorEventHitsFileInput);

  // Barrel
  void Barrel(G4LogicalVolume*);
  void BarrelRegularModules(G4LogicalVolume*);
  //void BarrelEndModules(G4LogicalVolume*);
  void BarrelRegularChambers(G4LogicalVolume*, G4double chambers_y_off_correction);
  //void BarrelEndChambers(G4LogicalVolume*, G4double chambers_y_off_correction);

  // EndCaps
  //void Endcaps(G4LogicalVolume*);
  //void EndcapChambers(G4LogicalVolume*);

  // RPC1
  G4LogicalVolume * BuildRPC1Box(G4Box* ChamberSolid, 
				 SD02* theSD, 
				 G4int layer_id,
				 G4UserLimits* pULimits);
  /*
  G4LogicalVolume * BuildRPC1Polyhedra(G4Polyhedra* ChamberSolid, 
				       SD02* theSD,
				       G4double phiStart,
				       G4double phiTotal,
				       G4int numSide,
				       G4int numZPlanes,
				       const G4double zPlane[],
				       const G4double rInner[],
				       const G4double rOuter[],
				       G4UserLimits* pULimits);
  */
  SD02* theBarrilRegSD;
  //SD02* theBarrilEndSD;
  //HECSD* theENDCAPEndSD;
  
  G4double  g10_thickness, glass_thickness, 
    gas_thickness, spacer_thickness, 
    spacer_gap, thetarot;

  G4Material * RadiatorMaterial;

  G4double theMaxStepAllowed;
};

#endif


