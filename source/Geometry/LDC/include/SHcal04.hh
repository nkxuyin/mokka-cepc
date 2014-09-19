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
// $Id: SHcal04.hh,v 1.7 2009/02/05 16:32:15 musat Exp $
// $Name: mokka-07-00 $
//
// F.Gaede: identical to  Hcal03 driver except that an additional gap
//          for the fibres is introduced between scintilaltor and steel

#ifndef SHcal04_h
#define SHcal04_h 1

class G4LogicalVolume;
class G4VPhysicalVolume;

class Database;
class G4VSolid;
class G4Box;

class SDHcalSD01;
class SDHcalECSD01;
class G4UserLimits;

#include "globals.hh"

#include "VSubDetectorDriver.hh"

class G4Polyhedra;
class G4Material;

class SHcal04 : public VSubDetectorDriver
{
public:

  SHcal04() : VSubDetectorDriver("SHcal04","hcal"),
	     theRPC2ECRMixture(0),
             theGraphiteMaterial(0),
             theMylarMaterial(0),
             theAirMaterial(0),
             theG10Material(0),
	     theBarrilRegSD(0),
	     theENDCAPEndSD(0),
	     theMaxStepAllowed(DBL_MAX)
  {}
  ~SHcal04();
  
  G4bool 
  ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
		      G4LogicalVolume *theWorld);

  G4bool 
  PostConstructAction(CGAGeometryEnvironment& );

  
private:
//  G4LogicalVolume *EnvLogHcalModuleBarrel;
//  G4LogicalVolume *EnvLogHcalModuleEndCap;
  G4Box * hcalECChamberInnerHole;
  G4RotationMatrix *rotBox;

  G4String SensitiveModel;
  Database* db;

  // Setup run time parameters
  G4bool Setup(const CGAGeometryEnvironment &);

  // To reload hits for visualisation
  void LoadEvent(FILE* theSubDetectorEventHitsFileInput);

  // Barrel
  void Barrel(G4LogicalVolume*);
  void BarrelVirtualModules(G4LogicalVolume*);
  //void BarrelRegularChambers(G4LogicalVolume*, G4double chambers_y_off_correction);

  // EndCaps
  void Endcaps(G4LogicalVolume*);
  void EndcapRings(G4LogicalVolume*);
  void EndcapChambers(G4LogicalVolume*,SDHcalECSD01*,bool);

  // RPC2
  G4LogicalVolume * BuildRPC2Box(G4Box* ChamberSolid, 
				 SDHcalSD01* theSD, 
				 G4int layerId,
				 G4UserLimits* pULimits);
  G4LogicalVolume * BuildRPC2Polyhedra(G4VSolid* ChamberSolid, 
				       SDHcalECSD01* theSD,
                                       bool rings,
				       G4double phiStart,
				       G4double phiTotal,
				       G4int numSide,
				       G4int numZPlanes,
				       const G4double zPlane[],
				       const G4double rInner[],
				       const G4double rOuter[],
				       G4UserLimits* pULimits);

  void BuildMaterials(void);
  G4Material * theRPC2ECRMixture;
  G4Material * theGraphiteMaterial;
  G4Material * theMylarMaterial;
  G4Material * theAirMaterial;
  G4Material * theG10Material;

  SDHcalSD01* theBarrilRegSD;
  SDHcalECSD01* theENDCAPEndSD;
  SDHcalECSD01* theENDCAPRingSD;
  
  G4Material * RadiatorMaterial;

  G4double theMaxStepAllowed;

  // Run time parameters
  G4double Hcal_radiator_thickness;
  G4String Hcal_radiator_material;
  G4int    Hcal_ring;
  G4double Hcal_radial_ring_inner_gap;
  G4double Hcal_stave_gaps;
  G4double Hcal_modules_gap;
  G4int Hcal_nlayers;
  G4int Hcal_barrel_number_modules;
  G4double hPrime;
  G4double Hcal_chamber_tickness;
  G4double layerThickness;
  G4double Hcal_inner_radius, Hcal_outer_radius;
  G4double Hcal_endcap_cables_gap, Hcal_endcap_ecal_gap;
  G4double Hcal_endcap_rmax;
  G4double TPC_Ecal_Hcal_barrel_halfZ;
  G4double Hcal_normal_dim_z;
  G4double Hcal_top_end_dim_z;
  G4double Hcal_start_z;
  G4double Ecal_endcap_outer_radius;
  G4double Ecal_endcap_zmin, Ecal_endcap_zmax;
  G4double Hcal_lateral_plate_thickness;
  G4double Hcal_cells_size;
  G4double Hcal_endcap_center_box_size;
  G4double  Hcal_total_dim_y;
  G4double Hcal_regular_chamber_dim_z;
  G4double Hcal_cell_dim_x;
  G4double Hcal_cell_dim_z;
 
//EndCap/Ring gas offset
  G4double ECR_RPC_GapPosX;
//Emmanuel's parameters
  G4double RPC_GapPosX;
  G4double d_InnerOctoSize;
  G4double RPC_PCB_Thickness, RPC_mylar_ThicknessAnode;
  G4double RPC_PadSeparation;
  G4double RPC_mylar_ThicknessCathode;
  G4double RPC_Graphite_ThicknessAnode;
  G4double RPC_Graphite_ThicknessCathode;
  G4double RPC_ThinGlass, RPC_Gap_Thickness;
  G4double RPC_ThickGlass, RPC_EdgeWidth;
  G4double RPCGazInletInnerRadius, RPCGazInletOuterRadius;
  G4double RPCGazInletLength, RPC_ChipPackageThickness;
  G4double RPC_Free_Thickness;

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
  helpParameters helpEndcapRing;

  // parameters

  G4int    intParamEndModuleType ;
  G4double dblParamHalfZ ;
  G4double dblParamLateralStructureThickness ;
  G4double dblParamModulesGap ;
  G4double dblParamStavesGap ;
  G4double dblParamBarrelMostZ ;

#endif
};

#endif


