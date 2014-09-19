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
// $Id: SHcal03.hh,v 1.3 2007/12/19 15:15:38 kristian Exp $
// $Name: mokka-07-00 $
//
// F.Gaede: identical to  Hcal03 driver except that an additional gap
//          for the fibres is introduced between scintilaltor and steel

#ifndef SHcal03_h
#define SHcal03_h 1

class G4LogicalVolume;
class Database;
class G4VSolid;

class SD;
class HECSD;
class G4UserLimits;

#include "VSubDetectorDriver.hh"

class G4Polyhedra;
class G4Material;

class SHcal03 : public VSubDetectorDriver
{
public:

  SHcal03() : VSubDetectorDriver("SHcal03","hcal"),
	     theBarrilRegSD(0),
	     theENDCAPEndSD(0),g10_thickness(0), 
	     glass_thickness(0), gas_thickness(0), 
	     spacer_thickness(0), spacer_gap(0),
	     theMaxStepAllowed(DBL_MAX)
  {}
  ~SHcal03();
  
  G4bool 
  ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
		      G4LogicalVolume *theWorld);

  G4bool 
  PostConstructAction(CGAGeometryEnvironment& );

  
private:
  G4LogicalVolume *EnvLogHcalModuleBarrel;
  G4LogicalVolume *EnvLogHcalModuleEndCap;
  G4String SensitiveModel;
  Database* db;

  // Setup run time parameters
  G4bool Setup(const CGAGeometryEnvironment &);

  // To reload hits for visualisation
  void LoadEvent(FILE* theSubDetectorEventHitsFileInput);

  // Barrel
  void Barrel(G4LogicalVolume*);
  void BarrelRegularModules(G4LogicalVolume*);
  void BarrelRegularChambers(G4LogicalVolume*, G4double chambers_y_off_correction);

  // EndCaps
  void Endcaps(G4LogicalVolume*);
  void EndcapRings(G4LogicalVolume*);
  void EndcapChambers(G4LogicalVolume*,HECSD*,bool);

  // RPC1
  G4LogicalVolume * BuildRPC1Box(G4Box* ChamberSolid, 
				 SD* theSD, 
				 G4int layer_id,
				 G4UserLimits* pULimits);
  G4LogicalVolume * BuildRPC1Polyhedra(G4Polyhedra* ChamberSolid, 
				       SD* theSD,
                                       bool rings,
				       G4double phiStart,
				       G4double phiTotal,
				       G4int numSide,
				       G4int numZPlanes,
				       const G4double zPlane[],
				       const G4double rInner[],
				       const G4double rOuter[],
				       G4UserLimits* pULimits);

  SD* theBarrilRegSD;
  HECSD* theENDCAPEndSD;
  HECSD* theENDCAPRingSD;
  
  G4double  g10_thickness, glass_thickness, 
    gas_thickness, spacer_thickness, 
    spacer_gap;

  G4Material * RadiatorMaterial;

  G4double theMaxStepAllowed;

  // Run time parameters
  G4double Hcal_radiator_thickness;
  G4String Hcal_radiator_material;
  G4int    Hcal_ring;
  G4double Hcal_radial_ring_inner_gap;
  G4String Hcal_sensitive_model;
  G4double Hcal_back_plate_thickness;
  G4double Hcal_stave_gaps;
  G4double Hcal_modules_gap;
  G4int Hcal_nlayers;
  G4int Hcal_barrel_end_module_type;
  G4double Hcal_fiber_gap;
  G4double Hcal_chamber_tickness;
  G4double Hcal_inner_radius;
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
  G4double Hcal_digi_cells_size;
  G4double Hcal_endcap_center_box_size;
  G4double  Hcal_total_dim_y;
  G4double Hcal_module_radius;
  G4double Hcal_y_dim2_for_x;
  G4double Hcal_y_dim1_for_x;
  G4double Hcal_bottom_dim_x;
  G4double Hcal_midle_dim_x;
  G4double Hcal_top_dim_x;
  G4double Hcal_y_dim1_for_z;
  G4double Hcal_y_dim2_for_z;
  G4double Hcal_y_dim3_for_z;
  G4double Hcal_regular_chamber_dim_z;
  G4double Hcal_cell_dim_x;
  G4double Hcal_cell_dim_z;
  G4double Hcal_digi_cell_dim_x;
  G4double Hcal_digi_cell_dim_z;  

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
  G4double dblParamBackPlateThickness ;
  G4double dblParamBarrelMostZ ;

#endif
};

#endif


