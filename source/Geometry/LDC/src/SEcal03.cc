//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for ILC   -      *
//*                                                     *
//* For more information about Mokka, visit the         *
//*                                                     *
//*  Mokka.in2p3.fr  Mokka home page.                   *
//*                                                     *
//*******************************************************
//
// $Id: SEcal03.cc,v 1.5 2008/10/23 16:25:44 mora Exp $
// $Name: mokka-07-00 $
//
// 
//
// SEcal03.cc
//

#include "Control.hh"
#include "MyPlacement.hh"
#include "G4UserLimits.hh"
#include "SEcal03.hh"
#include "CGAGeometryManager.hh"
#include "SEcalSD02.hh"
#include "SEcalSDRing02.hh"

#include "globals.hh"
#include "G4Box.hh"
#include "G4Polyhedra.hh"
#include "G4SubtractionSolid.hh"
#include "G4Trd.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
    
#include "CGADefs.h"
    
#ifdef MOKKA_GEAR
#include "gear/CalorimeterParameters.h"
#include "gearimpl/CalorimeterParametersImpl.h"
#include "gearimpl/LayerLayoutImpl.h"
#include "MokkaGear.h"
#endif
    
INSTANTIATE(SEcal03)
  // #define VERBOSE 1

#define N_FIBERS_ALVOULUS 3
#define N_FIBERS_W_STRUCTURE 2

G4bool 
SEcal03::
ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
		    G4LogicalVolume *theWorld)
{
  EnvLogEcalModuleBarrel=0;
  EnvLogEcalModuleEndCap=0;
  theBarrelSD=0;
  theEndCapSD=0;
  G4cout << "\nBuilding Ecal- SEcal03"<< G4endl;
  
  // Initialize the Geant3 interface
  if(Control::DUMPG3) MyPlacement::Init("ECAL","SEcal03");

  // Initialize the driver
  if (!Setup(aGeometryEnvironment)) return false;
  if (!Build(theWorld))  return false;
  return true;
}

G4bool 
SEcal03::Setup(const CGAGeometryEnvironment &theGeometryEnvironment)
{
  // retrieve the setup parameters and initialize the
  // driver
  
  Ecal_Alveolus_Air_Gap =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_Alveolus_Air_Gap");
  Ecal_Slab_shielding =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_Slab_shielding");
  Ecal_Slab_copper_thickness =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_Slab_copper_thickness");
  Ecal_Slab_PCB_thickness =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_Slab_PCB_thickness");
  Ecal_Slab_glue_gap =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_Slab_glue_gap");
  Ecal_Slab_ground_thickness =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_Slab_ground_thickness");
  Ecal_fiber_thickness =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_fiber_thickness");
  Ecal_Si_thickness = 
    theGeometryEnvironment.GetParameterAsDouble("Ecal_Si_thickness");

  Ecal_guard_ring_size =     
    theGeometryEnvironment.GetParameterAsDouble("Ecal_guard_ring_size");

  Ecal_radiator_material= 
    theGeometryEnvironment.GetParameterAsString("Ecal_radiator_material");

  if(Ecal_radiator_material == "tungsten")
    RadiatorMaterial =  CGAGeometryManager::GetMaterial("tungsten_19.3gccm");
  else
    if(Ecal_radiator_material == "lead")
      RadiatorMaterial =  CGAGeometryManager::GetMaterial("lead");
    else Control::Abort("SEcal03: invalid radiator material name. \nIt has to be either tungsten or lead",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
  G4String MaterialWarning = 
    Ecal_radiator_material 
    + " is the radiator material being placed.";
  Control::Log(MaterialWarning.data());

// Sensitive material
  Ecal_sensitive_material= 
    theGeometryEnvironment.GetParameterAsString("Ecal_sensitive_material");

  if(Ecal_sensitive_material == "silicon_2.33gccm")
    SensitiveMaterial =  CGAGeometryManager::GetMaterial("silicon_2.33gccm");
  else
    if(Ecal_sensitive_material == "polystyrene")
      SensitiveMaterial =  CGAGeometryManager::GetMaterial("polystyrene");
    else Control::Abort("SEcal03: invalid sensitive material name. \nIt has to be either silicon_2.33gccm or polystyrene",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
   MaterialWarning = 
    Ecal_sensitive_material 
    + " is the sensitive material being placed.";
  Control::Log(MaterialWarning.data());

  Ecal_inner_radius =
    theGeometryEnvironment.GetParameterAsDouble("TPC_outer_radius") +
    theGeometryEnvironment.GetParameterAsDouble("Ecal_Tpc_gap");
  Ecal_radiator_thickness1 =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_radiator_layers_set1_thickness");
  Ecal_radiator_thickness2 =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_radiator_layers_set2_thickness");
  Ecal_radiator_thickness3 = 
    theGeometryEnvironment.GetParameterAsDouble("Ecal_radiator_layers_set3_thickness");
  Ecal_Barrel_halfZ =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_Barrel_halfZ");

  Ecal_cell_size = 
    theGeometryEnvironment.GetParameterAsDouble("Ecal_cells_size");
  Ecal_cables_gap = 
    theGeometryEnvironment.GetParameterAsDouble("Ecal_cables_gap");
  Ecal_endcap_center_box_size = 
    theGeometryEnvironment.GetParameterAsDouble("Ecal_endcap_center_box_size");
  Lcal_outer_radius =
    theGeometryEnvironment.GetParameterAsDouble("Lcal_outer_radius");

  Ecal_Lcal_ring_gap = 
    theGeometryEnvironment.GetParameterAsDouble("Ecal_Lcal_ring_gap");
  Ecal_EC_Ring_gap = 
    theGeometryEnvironment.GetParameterAsDouble("Ecal_EC_Ring_gap");

  TUBE_crossing_angle = 
    theGeometryEnvironment.GetParameterAsDouble("TUBE_crossing_angle");

  Ecal_endcap_extra_size = 
    theGeometryEnvironment.GetParameterAsDouble("Ecal_endcap_extra_size");
  Ecal_nlayers1 =
    theGeometryEnvironment.GetParameterAsInt("Ecal_nlayers1");
  Ecal_nlayers2 =
    theGeometryEnvironment.GetParameterAsInt("Ecal_nlayers2");
  Ecal_nlayers3 =
    theGeometryEnvironment.GetParameterAsInt("Ecal_nlayers3");
  Ecal_support_thickness =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_support_thickness");
  Ecal_front_face_thickness =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_front_face_thickness");
  Ecal_lateral_face_thickness =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_lateral_face_thickness");
  Ecal_Slab_H_fiber_thickness =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_Slab_H_fiber_thickness");
  Ecal_barrel_number_of_towers =
    theGeometryEnvironment.GetParameterAsInt("Ecal_barrel_number_of_towers");
  
  
  // Calculed parameters.
  // Ecal_total_Slab_thickness = just one Slab side in the H, from 
  // shielding to mass

  total_number_of_layers =
    Ecal_nlayers1 + Ecal_nlayers2 + Ecal_nlayers3;
  if((total_number_of_layers % 2) == 0)
    Control::Abort("SEcal03: total number of layers has to be odd!",
		MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
  
  Ecal_total_Slab_thickness = 
    Ecal_Slab_shielding + 
    Ecal_Slab_copper_thickness + 
    Ecal_Slab_PCB_thickness +
    Ecal_Slab_glue_gap + 
    Ecal_Si_thickness + 
    Ecal_Slab_ground_thickness +
    Ecal_Alveolus_Air_Gap / 2;
#ifdef VERBOSE
  G4cout << " Ecal_total_Slab_thickness = " << Ecal_total_Slab_thickness  << G4endl;
#endif
  
  // In this release the number of modules is fixed to 5
  Ecal_Barrel_module_dim_z = 2 * Ecal_Barrel_halfZ / 5. ;
#ifdef VERBOSE
  G4cout << "Ecal_Barrel_module_dim_z  = " << Ecal_Barrel_module_dim_z  << G4endl;
#endif

  // The alveolus size takes in account the module Z size
  // but also 4 fiber layers for the alveoulus wall, the all
  // divided by the number of towers
  alveolus_dim_z = 
    (Ecal_Barrel_module_dim_z - 2. * Ecal_lateral_face_thickness) /
    Ecal_barrel_number_of_towers - 
    2 * N_FIBERS_ALVOULUS  * Ecal_fiber_thickness  - 
    2 * Ecal_Slab_H_fiber_thickness -
    2 * Ecal_Slab_shielding;

#ifdef VERBOSE
  G4cout << "alveolus_dim_z = " <<  alveolus_dim_z << G4endl;
#endif

  G4int n_total_layers = Ecal_nlayers1 + Ecal_nlayers2 + Ecal_nlayers3;

  module_thickness = 
    Ecal_nlayers1 * Ecal_radiator_thickness1 +
    Ecal_nlayers2 * Ecal_radiator_thickness2 +
    Ecal_nlayers3 * Ecal_radiator_thickness3 +

    int(n_total_layers/2) * // fiber around W struct layers
    (N_FIBERS_W_STRUCTURE * 2 *  Ecal_fiber_thickness) +
    
    (n_total_layers + 1) * // slabs plus fiber around and inside
    (Ecal_total_Slab_thickness +
     (N_FIBERS_ALVOULUS + 1 ) * Ecal_fiber_thickness) +
    
    Ecal_support_thickness + Ecal_front_face_thickness;
  
  //#ifdef VERBOSE
  G4cout << "For information : module_thickness = " << module_thickness  << G4endl;
  //#endif

  Ecal_endcap_rmax = Ecal_inner_radius 
    + module_thickness
    + Ecal_endcap_extra_size;

  // initialize the central box in endcaps
  // Central box become a Tub...

  CenterECTub =
    new G4Tubs ("CenterECTub",
		0.,
		Lcal_outer_radius + Ecal_Lcal_ring_gap,
		module_thickness,
		0.,
		2 * pi);

  // module barrel key parameters
  bottom_dim_x = 2. * tan(pi/8.) * Ecal_inner_radius +
    module_thickness/sin(pi/4.);

  top_dim_x = bottom_dim_x - 2 * module_thickness;

  
  endcap_module_dim_x = Ecal_endcap_center_box_size/2 + Ecal_inner_radius + 
    module_thickness + Ecal_endcap_extra_size;
#ifdef VERBOSE
  G4cout << " bottom_dim_x = " << bottom_dim_x  << G4endl;
  G4cout << " top_dim_x = " << top_dim_x << G4endl;
  G4cout << " endcap_module_dim_x = " <<  endcap_module_dim_x << G4endl;
#endif
  EC_module_z_offset =
    Ecal_Barrel_module_dim_z * 2.5 
    + Ecal_cables_gap
    + module_thickness /2;
  

  G4double centerTubDispl = 
    EC_module_z_offset * tan(TUBE_crossing_angle /2000);
//   G4cout << "centerTubDispl = "
// 	 << centerTubDispl 
// 	 << G4endl;
  FollowLcal = new G4TranslateX3D (centerTubDispl);
  FollowLcalZminus = new G4TranslateX3D (-centerTubDispl);
  
  // parameters for the old Ecal03 driver are kept here just for
  // documentation
  // nmax_cell_x = 6, nmax_cell_z = 6, inter_wafer_gap = 0.15,
  // n_guard_ring_zones = 3
  return true;
}


G4bool SEcal03::Build(G4LogicalVolume* WorldLog)
{

  cell_dim_x = Ecal_cell_size;
  G4double total_Si_dim_z = alveolus_dim_z;

  G4double util_SI_wafer_dim_z = 
    total_Si_dim_z/2 -  2 * Ecal_guard_ring_size;

  cell_dim_z =  util_SI_wafer_dim_z/ 
    floor(util_SI_wafer_dim_z/
	  cell_dim_x);

  N_cells_in_Z = int(util_SI_wafer_dim_z/cell_dim_z);
  N_cells_in_X = N_cells_in_Z;
  
  cell_dim_x = cell_dim_z;
  
  theMaxStepAllowed= std::min(cell_dim_x,cell_dim_z);

#ifdef VERBOSE
  G4cout << " total_Si_dim_z = " << total_Si_dim_z << G4endl;
  G4cout << " util_SI_wafer_dim_z  = " << util_SI_wafer_dim_z << G4endl;
#endif

  G4cout << "With the actual parameters for Ecal you have:\n cell_dim_z = " 
	 << cell_dim_z  
	 << "\n\n****>>>> Warning: \nEcal_cells_size redefined, forcing cell_dim_x to " << cell_dim_x
	 << " \nto insure squared Si cells!\n"
	 << G4endl;

  G4cout << " # of cells in X = " << N_cells_in_X  << G4endl;
  G4cout << " # of cells in Z = " << N_cells_in_Z << G4endl;
  
  G4bool barID1Flag = true, 
    ecID1Flag = true;
  G4double barDimX = bottom_dim_x;
  if((barDimX/cell_dim_x) < 511) 
    barID1Flag = false;
  G4double ecDimX = endcap_module_dim_x;
  if((ecDimX/cell_dim_x) < 511)
    ecID1Flag = false;
  // The cell boundaries does not really exist as G4 volumes. So,
  // to avoid long steps over running  several cells, the 
  // theMaxStepAllowed inside the sensitive material is the
  // pad smaller x or z dimension.
  theMaxStepAllowed= std::min(cell_dim_x,cell_dim_z);
  
  // Sensitive detector for the Ecal barrel

  G4double HWallSize =
    Ecal_Slab_H_fiber_thickness +
    Ecal_Slab_shielding;
  G4double TowerWallSize =
    N_FIBERS_ALVOULUS * Ecal_fiber_thickness;
  theBarrelSD = 
    new SEcalSD02(cell_dim_x,
		  cell_dim_z,
		  Ecal_Si_thickness,
		  N_cells_in_X,
		  N_cells_in_Z,
		  Ecal_guard_ring_size,
		  HWallSize,
		  TowerWallSize,
		  ECALBARREL,
		  "EcalBarrel",
		  barID1Flag,
		  "0110","0101");
  RegisterSensitiveDetector(theBarrelSD);

// Sensitive detector for the +z Ecal endcap
  theEndCapSD = 
    new SEcalSD02 (cell_dim_x,
		   cell_dim_z,
		   Ecal_Si_thickness,
		   N_cells_in_X,
		   N_cells_in_Z,
		   Ecal_guard_ring_size,
		   HWallSize,
		   TowerWallSize,
		   ECALENDCAPMINUS,
		   "EcalEndcap",
		   ecID1Flag,
		   "0110","0101");
  RegisterSensitiveDetector(theEndCapSD);

// Sensitive detector for end cap rings: we use the same
// but with fake parameters, to reuse the code. So we use
// it as just a huge wafer

//
// For that, we adjust the cell size in the ring to have a 
// interger number os cells in the given Si plates
  
  ECRingSiplateSize = Ecal_endcap_center_box_size 
    - 2 * Ecal_EC_Ring_gap
    - 2 * Ecal_lateral_face_thickness;

  G4double cell_size_ring = ECRingSiplateSize
    / floor(ECRingSiplateSize/cell_dim_x);
  
  G4int N_cells = int(ECRingSiplateSize / cell_size_ring);

  G4cout << "\n >> Si sensitive plates in Ecal rings are "
	 << ECRingSiplateSize 
	 << " X "
	 << ECRingSiplateSize
	 << " mm sized"
	 << G4endl;

  G4cout << "\n***** Forcing cell size in Ecal rings to be " << cell_size_ring 
	 << " mm,\n to insure an integer number of cells."
	 << "\nWith these values the Ecal rings will have \n" << N_cells
	 << " X " << N_cells << " identique cells of " << cell_size_ring
	 << " mm as size. *****\n" << G4endl;
  
  theEndCapRingSD = 
    new SEcalSDRing02 (cell_size_ring,
		       cell_size_ring,
		       Ecal_Si_thickness,
		       ECALENDCAPMINUS,
		       "EcalEndcapRing",
		       ecID1Flag);
  RegisterSensitiveDetector(theEndCapRingSD);

  // End cap ring doesn't rotate and has just one stave
  theEndCapRingSD->SetStaveRotationMatrix(1,0.);

  // initialize the Si in Rings
  G4VisAttributes* VisAtt = 
    new G4VisAttributes(G4Colour(0.7,0.7,0.7));
  VisAtt->SetForceSolid(true);
  VisAtt->SetVisibility(true);


  ECRingSiBox = 
    new G4Box ("ECRingSiSolid", 
	       ECRingSiplateSize/ 2.,
	       ECRingSiplateSize/ 2.,
	       Ecal_Si_thickness/2);

  G4SubtractionSolid *ECRingSiSolid =
    new G4SubtractionSolid("ECRingSiSolid", 
			   ECRingSiBox, 
			   CenterECTub,
			   *FollowLcal);

  G4UserLimits* pULimits=
    new G4UserLimits(theMaxStepAllowed);

  ECRingSiLog =
    new G4LogicalVolume(ECRingSiSolid,
			SensitiveMaterial,
			"ECRingSiLog", 
			0, 0, pULimits);  
  ECRingSiLog->SetSensitiveDetector(theEndCapRingSD);
  ECRingSiLog->SetVisAttributes(VisAtt);

  // Z minus -> hole not symetric
  ECRingSiSolid =
    new G4SubtractionSolid("ECRingSiSolidZminus", 
			   ECRingSiBox, 
			   CenterECTub,
			   *FollowLcalZminus);

  ECRingSiLogZminus =
    new G4LogicalVolume(ECRingSiSolid,
			SensitiveMaterial,
			"ECRingSiLog", 
			0, 0, pULimits);  
  ECRingSiLogZminus->SetSensitiveDetector(theEndCapRingSD);
  ECRingSiLogZminus->SetVisAttributes(VisAtt);

  MyPlacement::InsertComment("Building Ecal"); 
//
//---------------------------------------------------- 
// Barrel Standard Module in the air //
//---------------------------------------------------- 
//
  MyPlacement::InsertComment("Building Ecal barrel");
  BarrelStandardModule(WorldLog);


  
//   //----------------------------------------------------
//   // EndCaps in the air
//   //----------------------------------------------------
  
  MyPlacement::InsertComment("Building Ecal endcaps");  
  EC_Initialize();
  G4LogicalVolume* EndCapLogical =
    EndcapStandardModule();

  // EndCap module Placements
  new MyPlacement(0,
		  G4ThreeVector(0.,
				0.,
				EC_module_z_offset),
		  EndCapLogical,
		  "EndCapPhys",
		  WorldLog,
		  false,
		  ECALENDCAPPLUS);
  theEndCapRingSD->SetModuleZOffset(6,EC_module_z_offset);

  FollowLcal = FollowLcalZminus;

  EndCapLogical =
    EndcapStandardModule(true);

  // rotate the endcap module to place it on the -Z side

  // *************
  G4RotationMatrix *rot =  new G4RotationMatrix();
  rot->rotateY(pi);
  new MyPlacement(rot,
		  G4ThreeVector(0.,
				0.,
				-EC_module_z_offset),
		  EndCapLogical,
		  "EndCapPhys",
		  WorldLog,
		  false,
		  ECALENDCAPMINUS);

  // module = 6 to flag as endcap module in SD
  theEndCapSD->
    SetModuleZOffset(6,
 		     EC_module_z_offset);  
  
  theEndCapRingSD->
    SetModuleZOffset(6,
 		     EC_module_z_offset);  


  
#ifdef MOKKA_GEAR
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++
  // +  MOKKA GEAR                                      +
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++

  // get the information that are not yet included
  helpBarrel.phi0 = 0;

  //helpBarrel.zMax = (helpBarrel.mostZ + helpBarrel.leastZ) / 2 ;
  helpBarrel.zMax = std::max( helpBarrel.mostZ , -helpBarrel.leastZ ) ;
  
  // ECAL Barrel
  gear::CalorimeterParametersImpl* barrelParam = 
    new gear::CalorimeterParametersImpl( helpBarrel.innerRadius, 
					 helpBarrel.zMax, 
					 8, 
					 helpBarrel.phi0 );

  // calculate each layer thichness and push its paramaters
  G4double calcThick = 0;
  for (int i=1; i < helpBarrel.count; i++) 
    {
      calcThick = helpBarrel.layerPos[i] - helpBarrel.layerPos[i-1] ;    
      barrelParam->layerLayout().positionLayer
	(0, calcThick, cell_dim_z, cell_dim_x, 
	 helpBarrel.radiThickness[i-1]);
    }
  // well, it's a bit trick, but we have to add the last layer also...
  // just repeat the last calcThick, there is no reason to be not the same!     
  barrelParam->layerLayout().positionLayer
    (0, calcThick, cell_dim_z, cell_dim_x, 
     helpBarrel.radiThickness[helpBarrel.count-1]);
  
  // The same for Ecal Endcap   helpEndcap
  gear::CalorimeterParametersImpl* endcapParam = 
    new gear::CalorimeterParametersImpl( helpEndcap.innerRadius, 
					 helpEndcap.outerRadius, 
					 helpEndcap.leastZ,
					 2, 
					 helpBarrel.phi0 );

  for (int i=1; i < helpEndcap.count; i++) 
    {
      calcThick = helpEndcap.layerPos[i] - helpEndcap.layerPos[i-1] ;    
      endcapParam->layerLayout().positionLayer
	(0, calcThick, cell_dim_z, cell_dim_x, 
	 helpEndcap.radiThickness[i-1]);
    }
  // the last layer...
  endcapParam->layerLayout().positionLayer
    (0, calcThick, cell_dim_z, cell_dim_x, 
     helpEndcap.radiThickness[helpEndcap.count-1]);

  //Ecal Plug
  gear::CalorimeterParametersImpl* plugParam =
    new gear::CalorimeterParametersImpl(helpPlug.innerRadius,
					helpPlug.outerRadius,
					helpPlug.leastZ,
					2,
					helpBarrel.phi0);
  
  for (int i=1; i < helpPlug.count; i++)
    {
      calcThick = helpPlug.layerPos[i] - helpPlug.layerPos[i-1] ;    
      plugParam->layerLayout().positionLayer
	(0, calcThick, cell_dim_z, cell_dim_x, 
	 helpPlug.radiThickness[i-1]);
    }

  // the last layer...
  plugParam->layerLayout().positionLayer
    (0, calcThick, cell_dim_z, cell_dim_x, 
     helpPlug.radiThickness[helpPlug.count-1]);

  
  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setEcalBarrelParameters( barrelParam ) ;
  gearMgr->setEcalEndcapParameters( endcapParam ) ;
  gearMgr->setEcalPlugParameters( plugParam );
#endif

  G4cout << "Ecal done.\n" << G4endl;
  return true;  
}

SEcal03::~SEcal03() 
{
}  

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              BarrelStandardModule                 ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void 
SEcal03::BarrelStandardModule(G4LogicalVolume* MotherLog)
{
  // Attention: on bâtit le module dans la verticale
  // à cause du G4Trd et on le tourne avant de le positioner
  G4Trd * MyTrd = new G4Trd("Barrel_Module",
			    bottom_dim_x / 2, 
			    top_dim_x / 2,
			    Ecal_Barrel_module_dim_z / 2,
			    Ecal_Barrel_module_dim_z / 2,
			    module_thickness/2);
  
  EnvLogEcalModuleBarrel  = 
    new G4LogicalVolume(MyTrd,
			CGAGeometryManager::GetMaterial("g10"),
			"EnvLog", 
			0, 0, 0);
  G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(0.,1.,0));
  VisAtt->SetForceWireframe(true);
  VisAtt->SetDaughtersInvisible(true);
  EnvLogEcalModuleBarrel->SetVisAttributes(VisAtt);

//   //----------------------------------------------------
//   // W_Plate in the Barrel Envelop Ecal 
//   //----------------------------------------------------
//   MyPlacement::InsertComment("Ecal Barrel W Plates");
//   BarrelWPlate(EnvLogEcalModuleBarrel);

  // We count the layers starting from IP and from 1,
  // so odd layers should be inside slabs and
  // even ones on the structure.
  // The structure W layers are here big plans, as the 
  // gap between each W plate is too small to create problems 
  // The even W layers are part of H structure placed inside
  // the alveolus.
  G4double y_floor = 
    Ecal_front_face_thickness +
    N_FIBERS_ALVOULUS * Ecal_fiber_thickness;

  // ATTENTION, TWO LAYERS PER LOOP
  for(G4int layer_id = 1; 
      layer_id < total_number_of_layers+1;
      layer_id+=2)
    {
      // build and place the several Alveolus with 
      // the slabs and the radiator layer inside.
      G4double alveolus_dim_y;

      alveolus_dim_y =
	BuildBarrelAlveolus(layer_id,y_floor,EnvLogEcalModuleBarrel);
#ifdef VERBOSE
      G4cout << "y_floor = " << y_floor
	     << ", layer_id = " << layer_id
	     << ", alveolus_dim_y = " << alveolus_dim_y
	     << G4endl;
#endif
      // update the y_floor
      y_floor += alveolus_dim_y + 
	(N_FIBERS_ALVOULUS + N_FIBERS_W_STRUCTURE) * Ecal_fiber_thickness;
      
      // 
      G4int even_layer = layer_id + 1;
      if(even_layer > Ecal_nlayers1 + Ecal_nlayers2 + Ecal_nlayers3)
	continue;

      // Build and place the structure radiator layer
      // into the module
      G4double W_thick =
	BuildBarrelStructureLayer(even_layer,y_floor,EnvLogEcalModuleBarrel);
#ifdef VERBOSE
      G4cout << "y_floor = " << y_floor
	     << ", even_layer,  = " << even_layer
	     << ", W_thick  = " << W_thick
	     << G4endl;
#endif

      // update the y_floor
      y_floor += W_thick + 
	 (N_FIBERS_ALVOULUS + N_FIBERS_W_STRUCTURE) * Ecal_fiber_thickness;
    }
  
  //***********  Draw pour tests et return 
//   G4RotationMatrix *rot =  new G4RotationMatrix();
//   rot->rotateX(pi*0.5); // on couche le module.
//   new MyPlacement(rot,
// 		  G4ThreeVector(0,
// 				Ecal_inner_radius + 
// 				module_thickness/2,
// 				0),
// 		  EnvLogEcalModuleBarrel,
// 		  "BarrelEcalModule",
// 		  MotherLog,
// 		  false,
// 		  ECALBARREL*100+1*10+
// 			3);
//   theBarrelSD->SetStaveRotationMatrix(1,0.);
//   theBarrelSD->
//     SetModuleZOffset(3,
// 		     0.);
//   return;
  //*********** Draw pour tests et return !!!

    
  // BarrelStandardModule placements
  
  G4double phirot,module_z_offset;
  G4double X,Y;
  X = module_thickness * sin(pi/4.);
  Y = Ecal_inner_radius + module_thickness / 2.;
  theBarrelSD->SetStandardXOffset(X);
#ifdef MOKKA_GEAR
  // set first radius
  helpBarrel.innerRadius = X*X + Y*Y ;
  
  // set last layer position
  helpBarrel.layerPos.push_back( MyTrd->GetZHalfLength() ) ;
#endif
  
  for (G4int stave_id = 1; stave_id < 9 ; stave_id++)
    for (G4int module_id = 1; module_id < 6; module_id++)
      {
	phirot = (stave_id-1) * pi/4;
	module_z_offset =  (2 * module_id-6)*
	  Ecal_Barrel_module_dim_z/2.;
	G4RotationMatrix *rot=new G4RotationMatrix();
	rot->rotateX(pi*0.5); // on couche le module.
	rot->rotateY(phirot); // on tourne selon le stave
	new MyPlacement(rot,
			G4ThreeVector(X*cos(phirot)-Y*sin(phirot),
				      X*sin(phirot)+Y*cos(phirot),
				      module_z_offset),
			EnvLogEcalModuleBarrel,
			"BarrelEcalModule",
			MotherLog,
			false,
			ECALBARREL*100+stave_id*10+
			module_id);

	theBarrelSD->SetStaveRotationMatrix(stave_id,phirot);
	theBarrelSD->
	  SetModuleZOffset(module_id,
			   module_z_offset);
      }

#ifdef MOKKA_GEAR
  // find out most and least extensions in z
  // take offset and add/subtract dimension of trapezoid
  // attention z<->y
  G4double Z = module_z_offset;
  helpBarrel.leastZ = std::min( helpBarrel.leastZ, Z - MyTrd->GetYHalfLength1() );
  helpBarrel.mostZ  = std::max( helpBarrel.mostZ , Z + MyTrd->GetYHalfLength1() );
  
  // get innerRadius as minimun of all occurend inner radius
  // helf heigth of module
  G4double moduleHeigth = MyTrd->GetZHalfLength() ;
  G4double radius = std::sqrt( X*X + Y*Y );
  helpBarrel.innerRadius = std::min( helpBarrel.innerRadius, radius - moduleHeigth );
#endif
  
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              EndcapStandardModule                 ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
G4LogicalVolume* SEcal03::EndcapStandardModule(G4bool Zminus)
{
  // While waiting for more geometric details,
  // build a simple Endcap using a fiber polyhedra
  // and substract the center box

  MyPlacement::InsertComment("Ecal endcaps");

  G4double zPlane[2];
  G4double rInner[2],rOuter[2];


  zPlane[0]=-module_thickness/2;
  zPlane[1]=-zPlane[0];
  
  //  rInner[0]=rInner[1]= Ecal_endcap_center_box_size / 2.;
  // fulfill the central role, to subtract cylinder later
  rInner[0]=rInner[1]= 0.;
  rOuter[0]=rOuter[1]= Ecal_endcap_rmax;
  
  G4Polyhedra *ECPolyHedra=
    new G4Polyhedra("EcalEndCapSolid",
		    pi/8.,
		    2 * pi,
		    8,
		    2,
		    zPlane,
		    rInner,
		    rOuter);
  
  G4SubtractionSolid *EndCapSolid=
    new G4SubtractionSolid("EndCapSolid", 
			   ECPolyHedra, 
			   CenterECTub,
			   *FollowLcal);

  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(0,1,0));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  VisAtt->SetDaughtersInvisible(true);
   
  G4LogicalVolume* EndCapLogical =
    new G4LogicalVolume(EndCapSolid,
			CGAGeometryManager::GetMaterial("g10"),
			"EndCapLog",
			0, 0, 0);
  EndCapLogical->SetVisAttributes(VisAtt);

#ifdef MOKKA_GEAR
  // retrieve ending value for layersposition
  G4double lastLayerPos = module_thickness/2  ;
#endif

  //----------------------------------------------------
  // Radiator plates in the EndCap structure also as
  // polyhedra, and radiator plates in the slab of EndCap 
  // Rings as box less Tub
  //-------------------------------------------------------
  
  MyPlacement::InsertComment("Ecal endcaps W plates");
  G4LogicalVolume* EndCapRadiatorL1 = NULL;  
  G4LogicalVolume* EndCapRadiatorL2 = NULL;  
  G4LogicalVolume* EndCapRadiatorL3 = NULL;
  EndCapRingSlabRadiatorL1 = NULL;
  EndCapRingSlabRadiatorL2 = NULL;
  EndCapRingSlabRadiatorL3 = NULL;


  // Build the standard radiator plates to be placed
  // inside the module structure.
  EndcapRadiatorPlates(EndCapRadiatorL1,
		       EndCapRadiatorL2,
		       EndCapRadiatorL3,
		       EndCapRingSlabRadiatorL1,
		       EndCapRingSlabRadiatorL2,
		       EndCapRingSlabRadiatorL3);


  //-------------------------------------------------------
  // Radiator and towers placements inside the Endcap module
  //-------------------------------------------------------

  // We count the layers starting from IP and from 1,
  // so odd layers should be inside slabs and
  // even ones on the structure.
  //
  G4double z_floor = 
    - module_thickness/2 +
    Ecal_front_face_thickness + 
    N_FIBERS_ALVOULUS * Ecal_fiber_thickness;

  //
  // ATTENTION, TWO LAYERS PER LOOP AS THERE IS ONE
  // INSIDE THE ALVEOLUS.

  G4LogicalVolume* EndCapRadiator = NULL;
  G4double RadiatorThickness = 0.;
  G4double AlveolusThickness = 0;
  
  // ********************
  //total_number_of_layers = 1;

  G4int layer_id;
  for(layer_id = 1; 
      layer_id <= total_number_of_layers;
      layer_id+=2)
    {
      // place the tower layer for the four modules
      AlveolusThickness = 
	BuildECAlveolus (layer_id,
			 z_floor,
			 EndCapLogical,
			 Zminus);
    	
      // place Si/Rad/Si in ring 
      BuildECRingAlveolus (layer_id,
			   z_floor,
			   EndCapLogical,
			   Zminus);

      // update the z_floor
      z_floor += AlveolusThickness
	+ (N_FIBERS_ALVOULUS + N_FIBERS_W_STRUCTURE) 
	* Ecal_fiber_thickness;

      // place a radiator layer if the number of layers
      // is not complete
      if( layer_id == total_number_of_layers) break;
      if(layer_id < Ecal_nlayers1) 
	{
	  EndCapRadiator=EndCapRadiatorL1;
	  RadiatorThickness=Ecal_radiator_thickness1;
	}
      if(layer_id > Ecal_nlayers1 &&
	 layer_id < Ecal_nlayers1 + Ecal_nlayers2)
	{
	  EndCapRadiator=EndCapRadiatorL2;
	  RadiatorThickness=Ecal_radiator_thickness2;
	}
      if(layer_id > Ecal_nlayers1 + Ecal_nlayers2)
	{
	  EndCapRadiator=EndCapRadiatorL3;
	  RadiatorThickness=Ecal_radiator_thickness3;
	}
      
      new MyPlacement(0,
		      G4ThreeVector(0.,
				    0.,
				    z_floor +
				    RadiatorThickness / 2.),
		      EndCapRadiator,
		      "EndCapPhys",
		      EndCapLogical,
		      false,
		      0);
#ifdef MOKKA_GEAR
      if(!Zminus)
	{
	  // get positions of Layer as the middle of the radiator layer 
	  helpEndcap.layerPos.push_back(z_floor 
					+ RadiatorThickness / 2.) ;
	  helpPlug.layerPos.push_back(z_floor 
				      + RadiatorThickness / 2);
	  
	  // get radiator thickness
	  helpEndcap.radiThickness.
	    push_back(RadiatorThickness) ;
	  helpPlug.radiThickness.push_back(RadiatorThickness);
	  // count layers
	  helpEndcap.count ++ ;
	  helpPlug.count ++;
	}
#endif

      // update the z_floor
      z_floor += RadiatorThickness + 
	(N_FIBERS_ALVOULUS + N_FIBERS_W_STRUCTURE) 
	* Ecal_fiber_thickness;
      
    }
  
#ifdef MOKKA_GEAR
      if(!Zminus)
	{
	  // set ending value for layersposition
	  helpEndcap.layerPos.push_back( lastLayerPos ) ;

	  // getting the outer radius as maximal reached radius
	  helpEndcap.outerRadius = rOuter[0];
	  
	  // getting inner radius as minimal distance
	  helpEndcap.innerRadius = Ecal_endcap_center_box_size / 2.;
	  helpPlug.outerRadius = helpEndcap.innerRadius;
	  helpPlug.innerRadius = Lcal_outer_radius + Ecal_Lcal_ring_gap;
	  
	  // get least z-distance as maximum
	  helpEndcap.zMax = EC_module_z_offset 
	    + module_thickness / 2.;
	  helpPlug.zMax = helpEndcap.zMax;
	  helpEndcap.leastZ = EC_module_z_offset 
	    - module_thickness / 2.;
	  helpPlug.leastZ = helpEndcap.leastZ; 
	  
	  // set phi0 to zero
	  helpEndcap.phi0 = 0. ;
	  helpPlug.phi0 = 0;
	}
#endif
  
  return EndCapLogical;
}

void 
SEcal03::EndcapRadiatorPlates(G4LogicalVolume*& EndCapRadiatorL1,
			      G4LogicalVolume*& EndCapRadiatorL2,
			      G4LogicalVolume*& EndCapRadiatorL3,
			      G4LogicalVolume*& EndCapRingSlabRadiatorL1,
			      G4LogicalVolume*& EndCapRingSlabRadiatorL2,
			      G4LogicalVolume*& EndCapRingSlabRadiatorL3)
{
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(1,0,0));
  //VisAtt->SetForceWireframe(false);
  VisAtt->SetVisibility(true);
  VisAtt->SetForceSolid(true);
  
  // W Logicals
  G4double zPlane[2];
  G4double rInner[2],rOuter[2];
  
  rInner[0]=rInner[1]= 
    //    Ecal_endcap_center_box_size / 2.
    + Ecal_lateral_face_thickness;
  rOuter[0]=rOuter[1]= Ecal_endcap_rmax
    - Ecal_lateral_face_thickness;
  
  if(Ecal_nlayers1 > 0 )
    {
      zPlane[0]=-Ecal_radiator_thickness1 / 2.;
      zPlane[1]=-zPlane[0];
      G4Polyhedra *ECPolyHedraRadL1=
	new G4Polyhedra("EcalECRadL1",
			pi/8.,
			2 * pi,
			8,
			2,
			zPlane,
			rInner,
			rOuter);

      G4SubtractionSolid *EndCapRadL1 =
	new G4SubtractionSolid("EcalECRadL1",
			       ECPolyHedraRadL1, 
			       CenterECTub,
			       *FollowLcal);

      EndCapRadiatorL1 =
	new G4LogicalVolume(EndCapRadL1,
			    RadiatorMaterial,
			    "EndCapLog",
			    0, 0, 0);
      EndCapRadiatorL1->SetVisAttributes(VisAtt);

      // plate for slab in ring
      G4Box *ECRingRadBox1 = 
	new G4Box ("ECRingRadBox1", 
		   (Ecal_endcap_center_box_size - Ecal_lateral_face_thickness)/ 2.,
		   (Ecal_endcap_center_box_size - Ecal_lateral_face_thickness)/ 2.,
		   Ecal_radiator_thickness1/2);

      G4SubtractionSolid *ECRingRadSolid1 =
	new G4SubtractionSolid("ECRingRadSolid1", 
			       ECRingRadBox1, 
			       CenterECTub,
			       *FollowLcal);

      EndCapRingSlabRadiatorL1 =
	new G4LogicalVolume(ECRingRadSolid1,
			    RadiatorMaterial,
			    "EndCapRingSlabRadiatorL1", 
			    0, 0, 0);  
      EndCapRingSlabRadiatorL1->SetVisAttributes(VisAtt);
    }
  if(Ecal_nlayers2 > 0 )
    {
      zPlane[0]=-Ecal_radiator_thickness2 / 2.;
      zPlane[1]=-zPlane[0];
      G4Polyhedra *ECPolyHedraRadL2 =
	new G4Polyhedra("EcalECRadL2",
			pi/8.,
			2 * pi,
			8,
			2,
			zPlane,
			rInner,
			rOuter);

      G4SubtractionSolid *EndCapRadL2 =
	new G4SubtractionSolid("EcalECRadL2",
			       ECPolyHedraRadL2, 
			       CenterECTub,
			       *FollowLcal);
      
      EndCapRadiatorL2 =
	new G4LogicalVolume(EndCapRadL2,
			    RadiatorMaterial,
			    "EndCapLog",
			    0, 0, 0);
      EndCapRadiatorL2->SetVisAttributes(VisAtt);

      // plate for slab in ring
      G4Box *ECRingRadBox2 = 
	new G4Box ("ECRingRadBox2", 
		   (Ecal_endcap_center_box_size - Ecal_lateral_face_thickness)/ 2.,
		   (Ecal_endcap_center_box_size - Ecal_lateral_face_thickness)/ 2.,
		   Ecal_radiator_thickness2/2);

      G4SubtractionSolid *ECRingRadSolid2 =
	new G4SubtractionSolid("ECRingRadSolid2", 
			       ECRingRadBox2, 
			       CenterECTub,
			       *FollowLcal);

      EndCapRingSlabRadiatorL2 =
	new G4LogicalVolume(ECRingRadSolid2,
			    RadiatorMaterial,
			    "EndCapRingSlabRadiatorL2", 
			    0, 0, 0);  
      EndCapRingSlabRadiatorL2->SetVisAttributes(VisAtt);


    }
  if(Ecal_nlayers3 > 0 )
    {
      zPlane[0]=-Ecal_radiator_thickness3 / 2.;
      zPlane[1]=-zPlane[0];
      G4Polyhedra *ECPolyHedraRadL3 =
	new G4Polyhedra("EcalECRadL3",
			pi/8.,
			2 * pi,
			8,
			2,
			zPlane,
			rInner,
			rOuter);

      G4SubtractionSolid *EndCapRadL3 =
	new G4SubtractionSolid("EcalECRadL3",
			       ECPolyHedraRadL3, 
			       CenterECTub,
			       *FollowLcal);
      
      EndCapRadiatorL3 =
	new G4LogicalVolume(EndCapRadL3,
			    RadiatorMaterial,
			    "EndCapLog",
			    0, 0, 0);
      EndCapRadiatorL3->SetVisAttributes(VisAtt);

      // plate for slab in ring
      G4Box *ECRingRadBox3 = 
	new G4Box ("ECRingRadBox3", 
		   (Ecal_endcap_center_box_size - Ecal_lateral_face_thickness)/ 2.,
		   (Ecal_endcap_center_box_size - Ecal_lateral_face_thickness)/ 2.,
		   Ecal_radiator_thickness3/2);

      G4SubtractionSolid *ECRingRadSolid3 =
	new G4SubtractionSolid("ECRingRadSolid3", 
			       ECRingRadBox3, 
			       CenterECTub,
			       *FollowLcal);

      EndCapRingSlabRadiatorL3 =
	new G4LogicalVolume(ECRingRadSolid3,
			    RadiatorMaterial,
			    "EndCapRingSlabRadiatorL3", 
			    0, 0, 0);  
      EndCapRingSlabRadiatorL3->SetVisAttributes(VisAtt);
    }
}

void 
SEcal03::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
  // In this complex subdetector the best is to read
  // all hits in the one of the sensitive detectors,
  // just for visualisation, so just for your eyes...
  theBarrelSD->LoadEvent(theSubDetectorEventHitsFileInput);
}

G4double
SEcal03::
BuildBarrelAlveolus(G4int layer_id,
		    G4double module_y_floor,
		    G4LogicalVolume* EnvLogEcalModuleBarrel)
{
  // BuildBarrelAlveolus build the Slabs, the 
  // radiator plate and place it inside the Barrel
  // standard Module, in several towers.
  G4double alveolus_dim_y;
  G4double W_thick =
    GiveMeRadiatorThickness(layer_id);
  alveolus_dim_y =
    2 * Ecal_total_Slab_thickness +
    W_thick + 
    2 * Ecal_fiber_thickness;
  
  G4double alveolus_dim_x =
    bottom_dim_x - 2 * (module_y_floor+alveolus_dim_y);
  
#ifdef VERBOSE
  G4cout << "alveolus_dim_x = " << alveolus_dim_x << G4endl;
#endif
  
  // To simplify we place each slab and the radiator
  // layer directly into the fiber module.
  //
  // Build a slab:
  //
  G4LogicalVolume* SlabLog =
    BuildSlab(alveolus_dim_x,
	      Ecal_total_Slab_thickness,
	      alveolus_dim_z,
	      theBarrelSD);
  
  // Place the Slab and radiator inside the H,
  // here directly into the module fiber as the
  // H structure is also built in fiber.
  G4double z_tower_center =  
    - Ecal_Barrel_module_dim_z /2
    + Ecal_lateral_face_thickness +
    Ecal_fiber_thickness * N_FIBERS_ALVOULUS +
    Ecal_Slab_shielding + 
    Ecal_Slab_H_fiber_thickness +
    alveolus_dim_z /2;  
//   for (G4int i_tower = 1;
//        i_tower < Ecal_barrel_number_of_towers + 1;
//        i_tower++)
  for (G4int i_tower = Ecal_barrel_number_of_towers;
       i_tower > 0;
       i_tower--)
    {
      G4double y_floor = module_y_floor;
      G4double x_off = 0; // to be calculed
      G4double y_off = 
	-module_thickness/2 +
	y_floor +
	Ecal_total_Slab_thickness/ 2;
      // First Slab
      G4RotationMatrix *rot=new G4RotationMatrix();
      rot->rotateX(pi); 
      new MyPlacement(rot,
		      G4ThreeVector(x_off,
				    z_tower_center, // Y<->Z !
				    y_off),
		      SlabLog,
		      "Slab",
		      EnvLogEcalModuleBarrel,
		      false,
		      i_tower * 1000 + layer_id);
      //      if(i_tower == 1)
      if (i_tower == Ecal_barrel_number_of_towers)
	{
	  theBarrelSD->
	    AddLayer(layer_id,
		     x_off - 
		     ((G4Box *)SlabLog->GetSolid())->GetXHalfLength(),
		     Ecal_inner_radius + module_thickness/2 +
		     y_off - Si_Slab_Y_offset,
		     z_tower_center - 
		     ((G4Box *)SlabLog->GetSolid())->GetYHalfLength());
	}

      y_floor += Ecal_total_Slab_thickness +
	Ecal_fiber_thickness;
      
      // Radiator layer "inside" alveolus
      G4LogicalVolume* WLog =
	BuildRadiatorPlate(alveolus_dim_x,
			   W_thick,
			   alveolus_dim_z);
      new MyPlacement(0,
		      G4ThreeVector(0,
				    z_tower_center, // Y<->Z !
				    -module_thickness/2 +
				    y_floor +
				    W_thick/ 2),
		      WLog,
		      "RadiatorSlab",
		      EnvLogEcalModuleBarrel,
		      false,
		      0);
#ifdef MOKKA_GEAR
      if (i_tower == Ecal_barrel_number_of_towers)
	{
	  // first layer in Slab
	  // get middle of radiator layer as layer pos
	  
	  helpBarrel.layerPos.
	    push_back(y_floor + W_thick/ 2);
	  
	  // get radiator thickness as W-Plate Thickness
	  helpBarrel.radiThickness.push_back(W_thick);
	  // count layers
	  helpBarrel.count ++ ;
	}
#endif

      
      y_floor +=  W_thick + Ecal_fiber_thickness;
      
      y_off = 
	-module_thickness/2 +
	y_floor +
	Ecal_total_Slab_thickness/ 2;

      // Second Slab
      // The second slab starts from bottom to up
      
      new MyPlacement(0,
		      G4ThreeVector(0,
				    z_tower_center, // Y<->Z !
				    y_off),
		      SlabLog,
		      "Slab",
		      EnvLogEcalModuleBarrel,
		      false,
		      i_tower * 1000 + layer_id + 1);
      //      if(i_tower == 1)
      if (i_tower == Ecal_barrel_number_of_towers)
	{
	  theBarrelSD->
	    AddLayer(layer_id + 1,
		     x_off - 
		     ((G4Box *)SlabLog->GetSolid())->GetXHalfLength(),
		     Ecal_inner_radius + module_thickness/2 +
		     y_off + Si_Slab_Y_offset,
		     z_tower_center - 
		     ((G4Box *)SlabLog->GetSolid())->GetYHalfLength());
	}

      z_tower_center += alveolus_dim_z + 
	2. * Ecal_fiber_thickness * N_FIBERS_ALVOULUS +
	2. * Ecal_Slab_H_fiber_thickness +
	2. * Ecal_Slab_shielding;

    }

  return alveolus_dim_y;
}

// GiveMeRadiatorThickness returns the radiator
// layer thickness for a given layer index
G4double 
SEcal03::GiveMeRadiatorThickness(G4int layer_id)
{
  G4double W_thick = Ecal_radiator_thickness1;
  if(layer_id > Ecal_nlayers1 &&
     layer_id <= Ecal_nlayers1 + Ecal_nlayers2)
    W_thick = Ecal_radiator_thickness2;
  if(layer_id > Ecal_nlayers1 + Ecal_nlayers2 && 
     layer_id <= Ecal_nlayers1 + Ecal_nlayers2 + Ecal_nlayers3)
    W_thick = Ecal_radiator_thickness3;
  return W_thick;
}

// Build and places the radiator embedded into the
// fiber structure
G4double
SEcal03::
BuildBarrelStructureLayer(G4int layer_id,
			  G4double y_floor,
			  G4LogicalVolume* EnvLogEcalModuleBarrel)
{
  G4double radiator_dim_y =
    GiveMeRadiatorThickness(layer_id);

  G4double radiator_dim_x =
    bottom_dim_x - 2 * (y_floor+radiator_dim_y);
#ifdef VERBOSE
  G4cout << "radiator_dim_x = " << radiator_dim_x << G4endl;
#endif  
  G4double radiator_dim_z =
    Ecal_Barrel_module_dim_z -
    2 * Ecal_lateral_face_thickness -
    2 * N_FIBERS_W_STRUCTURE * Ecal_fiber_thickness;

  G4LogicalVolume* RadLog =
    BuildRadiatorPlate(radiator_dim_x,radiator_dim_y,radiator_dim_z);

  new MyPlacement(0,
		  G4ThreeVector(0,
				0,
				-module_thickness/2+
				y_floor +
				radiator_dim_y/2),
		  RadLog,
		  "RadiatorStruct",
		  EnvLogEcalModuleBarrel,
		  false,0);
#ifdef MOKKA_GEAR
	  // get middle of radiator layer as layer pos
	  helpBarrel.layerPos.
	    push_back(y_floor 
		      + radiator_dim_y/2);
	  
	  // get radiator thickness as W-Plate Thickness
	  helpBarrel.radiThickness.
	    push_back(radiator_dim_y);
	  // count layers
	  helpBarrel.count ++ ;
#endif
  
  return radiator_dim_y;
}
G4LogicalVolume* 
SEcal03::BuildRadiatorPlate(G4double radiator_dim_x,
			    G4double radiator_dim_y,
			    G4double radiator_dim_z)
{
  // Radiator solid
  G4LogicalVolume * RadLog;
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(1.,0,0));
  VisAtt->SetForceSolid(false);
  VisAtt->SetVisibility(false);
  G4Box* BoxSolid = 
    new G4Box("RadSolid", 
	      radiator_dim_x/2,  //hx
	      radiator_dim_z/2,  //hz attention!
	      radiator_dim_y/2); //hy attention!

  // Radiator Logical
  RadLog = 
    new G4LogicalVolume(BoxSolid,
			RadiatorMaterial,
			"RadLogical", 
			0, 0, 0);  
  RadLog->SetVisAttributes(VisAtt);
  return RadLog;
}

G4LogicalVolume* 
SEcal03::BuildSlab(G4double slab_dim_x,
		   G4double slab_dim_y,
		   G4double slab_dim_z,
		   SEcalSD02 * theSD)
{
  // Slab solid
  G4LogicalVolume * SlabLog;
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(1.,1.,1.));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  VisAtt->SetVisibility(false);
  VisAtt->SetDaughtersInvisible(true);
  G4Box* BoxSolid = 
    new G4Box("SlabSolid", 
	      slab_dim_x/2,  //hx
	      slab_dim_z/2,  //hz attention!
	      slab_dim_y/2); //hy attention!

  // Slab Logical
  SlabLog = 
    new G4LogicalVolume(BoxSolid,
			CGAGeometryManager::
			GetMaterial("air"),
			"SlabLogical", 
			0, 0, 0);  
  SlabLog->SetVisAttributes(VisAtt);
  G4double y_slab_floor =
    - slab_dim_y /2;

  // Ground plate
  G4Box* GroundSolid = 
    new G4Box("GroundSolid", 
	      slab_dim_x/2,
	      slab_dim_z/2,
	      Ecal_Slab_ground_thickness/2);
  G4LogicalVolume* GroundLog = 
    new G4LogicalVolume(GroundSolid,
			CGAGeometryManager::GetMaterial("copper"),
			"GroundLogical", 
			0, 0, 0);
  G4VisAttributes* GroundVisAtt = 
    new G4VisAttributes(G4Colour(0,0,1));
  GroundVisAtt->SetForceWireframe(true);
  GroundVisAtt->SetVisibility(false);
  GroundLog->SetVisAttributes(GroundVisAtt);
  new MyPlacement(0,
		  G4ThreeVector(0,
				0,
				y_slab_floor +
				Ecal_Slab_ground_thickness/2),
		  GroundLog,
		  "Ground",
		  SlabLog,
		  false,0);
  
  y_slab_floor+=Ecal_Slab_ground_thickness;

  // Si layer
  // we place a big plane of Si and inside it the
  // Si wafers, to simplify the gard ring placements
  G4Box* CommonSiSolid = 
    new G4Box("CommonSiSolid", 
	      slab_dim_x/2,
	      slab_dim_z/2,
	      Ecal_Si_thickness/2);
  
  G4LogicalVolume* CommonSiLog =
    new G4LogicalVolume(CommonSiSolid,
			SensitiveMaterial,
			"CommonSiLog", 
			0, 0, 0);  
  G4VisAttributes* CommonSiVisAtt = 
    new G4VisAttributes(G4Colour(0.5,0.5,0.5));
  CommonSiVisAtt->SetForceWireframe(true);
  CommonSiVisAtt->SetVisibility(false);
  CommonSiLog  ->SetVisAttributes(CommonSiVisAtt);
  Si_Slab_Y_offset = 
    y_slab_floor +
    Ecal_Si_thickness/2;

  new MyPlacement(0,
		  G4ThreeVector(0,
				0,
				Si_Slab_Y_offset),
		  CommonSiLog,
		  "CommonSi",
		  SlabLog,
		  false,0);
  //
  // Then we place the Si wafers inside the big Si plane
  //
  // User limits to limit the step to stay inside the cell
  G4UserLimits* pULimits=
    new G4UserLimits(theMaxStepAllowed);



  // Normal squared wafers
  G4double wafer_dim_x = 
    N_cells_in_X * cell_dim_x;
  G4double wafer_dim_z = 
    N_cells_in_Z * cell_dim_z;
  G4Box* WaferSiSolid = 
    new G4Box("WaferSiSolid", 
	      wafer_dim_x/2,
	      wafer_dim_z/2,
	      Ecal_Si_thickness/2);

  G4LogicalVolume* WaferSiLog =
    new G4LogicalVolume(WaferSiSolid,
			SensitiveMaterial,
			"WaferSiLog", 
			0, 0, pULimits);  
  WaferSiLog->SetSensitiveDetector(theSD);
  G4VisAttributes* WaferSiVisAtt = 
    new G4VisAttributes(G4Colour(0.7,0.7,0.7));
  WaferSiVisAtt->SetForceSolid(true);
  WaferSiVisAtt->SetVisibility(true);
  WaferSiLog  ->SetVisAttributes(WaferSiVisAtt);


  // As the same method builds both barrel and end cap
  // slabs, place the wafers along the biggest axe
  if (slab_dim_z < slab_dim_x) 
    {
      // Barrel
      G4double real_wafer_size_x =
	wafer_dim_x + 2 * Ecal_guard_ring_size;
      
      G4int n_wafers_x =
	int(floor(slab_dim_x / real_wafer_size_x));
      
      G4double wafer_pos_x =
	-slab_dim_x/2 + 
	Ecal_guard_ring_size +
	wafer_dim_x /2 ;
      G4int n_wafer_x;
      for (n_wafer_x = 1;
	   n_wafer_x < n_wafers_x + 1;
	   n_wafer_x++)
	{
	  G4double wafer_pos_z =
	    -slab_dim_z/2 + 
	    Ecal_guard_ring_size +
	    wafer_dim_z /2;
	  for (G4int n_wafer_z = 1;
	       n_wafer_z < 3;
	       n_wafer_z++)
	    {
	      new MyPlacement(0,
			      G4ThreeVector(wafer_pos_x,
					    wafer_pos_z,
					    0),
			      WaferSiLog,
			      "WaferSi",
			      CommonSiLog,
			      false,
			      n_wafer_x*1000 + n_wafer_z);
	      wafer_pos_z +=
		wafer_dim_z +
		2 * Ecal_guard_ring_size;
	    }
	  wafer_pos_x += 
	    wafer_dim_x +
	    2 * Ecal_guard_ring_size;
	}
      
      // Magic wafers to complete the slab...
      // (wafers with variable number of cells just
      // to complete the slab. in reality we think that
      // we'll have just a few models of special wafers
      // for that.
      G4double resting_dim_x =
	slab_dim_x - 
	(wafer_dim_x + 2 * Ecal_guard_ring_size) * 
	n_wafers_x;
      
      if(resting_dim_x >
	 (cell_dim_x + 2 * Ecal_guard_ring_size))
	{
	  G4int N_cells_x_remaining =
	    int(floor((resting_dim_x - 
		       2 * Ecal_guard_ring_size)
		      /cell_dim_x));
	  
	  wafer_dim_x =
	    N_cells_x_remaining *
	    cell_dim_x;
	  
	  WaferSiSolid = 
	    new G4Box("WaferSiSolid", 
		      wafer_dim_x/2,
		      wafer_dim_z/2,
		      Ecal_Si_thickness/2);
	  
	  WaferSiLog =
	    new G4LogicalVolume(WaferSiSolid,
				SensitiveMaterial,
				"WaferSiLog", 
				0, 0, pULimits);  
	  WaferSiLog->SetVisAttributes(WaferSiVisAtt);
	  WaferSiLog->SetSensitiveDetector(theSD);
	  
	  wafer_pos_x =
	    -slab_dim_x/2 +
	    n_wafers_x * real_wafer_size_x +
	    (wafer_dim_x + 2 * Ecal_guard_ring_size)/2;
	  
	  real_wafer_size_x =
	    wafer_dim_x + 2 * Ecal_guard_ring_size;
	  
	  G4double wafer_pos_z =
	    -slab_dim_z/2 + 
	    Ecal_guard_ring_size +
	    wafer_dim_z /2;
	  for (G4int n_wafer_z = 1;
	       n_wafer_z < 3;
	       n_wafer_z++)
	    {
	      new MyPlacement(0,
			      G4ThreeVector(wafer_pos_x,
					    wafer_pos_z,
					    0),
			      WaferSiLog,
			      "WaferSi",
			      CommonSiLog,
			      false,
			      n_wafer_x*1000 + n_wafer_z);
	      wafer_pos_z +=
		wafer_dim_z +
		2 * Ecal_guard_ring_size;
	    }
	}
    }
  else
    {
      // End caps
      G4double real_wafer_size_x =
	wafer_dim_z + 2 * Ecal_guard_ring_size;
      
      G4int n_wafers_x =
	int(floor(slab_dim_z / real_wafer_size_x));
      
      G4double wafer_pos_x =
	-slab_dim_z/2 + 
	Ecal_guard_ring_size +
	wafer_dim_z /2 ;
      G4int n_wafer_x;
      for (n_wafer_x = 1;
	   n_wafer_x < n_wafers_x + 1;
	   n_wafer_x++)
	{
	  G4double wafer_pos_z =
	    -slab_dim_x/2 + 
	    Ecal_guard_ring_size +
	    wafer_dim_x /2;
	  for (G4int n_wafer_z = 1;
	       n_wafer_z < 3;
	       n_wafer_z++)
	    {
	      new MyPlacement(0,
			      G4ThreeVector(wafer_pos_z,
					    wafer_pos_x,
					    0),
			      WaferSiLog,
			      "WaferSi",
			      CommonSiLog,
			      false,
			      n_wafer_x*1000 + n_wafer_z);
	      wafer_pos_z +=
		wafer_dim_x +
		2 * Ecal_guard_ring_size;
	    }
	  wafer_pos_x += 
	    wafer_dim_z +
	    2 * Ecal_guard_ring_size;
	}
      
      // Magic wafers to complete the slab...
      // (wafers with variable number of cells just
      // to complete the slab. in reality we think that
      // we'll have just a few models of special wafers
      // for that.
      G4double resting_dim_x =
	slab_dim_z - 
	(wafer_dim_z + 2 * Ecal_guard_ring_size) * 
	n_wafers_x;
      
      if(resting_dim_x >
	 (cell_dim_z + 2 * Ecal_guard_ring_size))
	{
	  G4int N_cells_x_remaining =
	    int(floor((resting_dim_x - 
		       2 * Ecal_guard_ring_size)
		      /cell_dim_z));
	  wafer_dim_x =
	    N_cells_x_remaining *
	    cell_dim_z;
	  
	  WaferSiSolid = 
	    new G4Box("WaferSiSolid", 
		      wafer_dim_z/2,
		      wafer_dim_x/2,
		      Ecal_Si_thickness/2);
	  
	  WaferSiLog =
	    new G4LogicalVolume(WaferSiSolid,
				SensitiveMaterial,
				"WaferSiLog", 
				0, 0, pULimits);  
	  WaferSiLog->SetVisAttributes(WaferSiVisAtt);
	  WaferSiLog->SetSensitiveDetector(theSD);
	  
	  wafer_pos_x =
	    -slab_dim_z/2 +
	    n_wafers_x * real_wafer_size_x +
	    (wafer_dim_x + 2 * Ecal_guard_ring_size)/2;
	  
	  real_wafer_size_x =
	    wafer_dim_x + 2 * Ecal_guard_ring_size;
	  
	  G4double wafer_pos_z =
	    -slab_dim_x/2 + 
	    Ecal_guard_ring_size +
	    wafer_dim_z /2;
	  for (G4int n_wafer_z = 1;
	       n_wafer_z < 3;
	       n_wafer_z++)
	    {
	      new MyPlacement(0,
			      G4ThreeVector(wafer_pos_z,
					    wafer_pos_x,
					    0),
			      WaferSiLog,
			      "WaferSi",
			      CommonSiLog,
			      false,
			      n_wafer_x*1000 + n_wafer_z);
	      wafer_pos_z +=
		wafer_dim_z +
		2 * Ecal_guard_ring_size;
	    }
	}
    }
  //
  // Glue space as just a air gap, we don't care about
  // a few points of glue...
  //
  y_slab_floor += 
    Ecal_Si_thickness +
    Ecal_Slab_glue_gap;

  //
  // The PCB layer, the copper and the shielding are
  // placed as a big G10 layer, as the copper and the
  // shielding ones are very tiny.
  //
  G4double PCBCuShield_thickness =
    Ecal_Slab_PCB_thickness +
    Ecal_Slab_copper_thickness +
    Ecal_Slab_shielding;
  
  G4Box* PCBCuShieldSolid = 
    new G4Box("PCBCuShieldSolid", 
	      slab_dim_x/2,
	      slab_dim_z/2,
	      PCBCuShield_thickness/2);
  G4LogicalVolume* PCBCuShieldLog = 
    new G4LogicalVolume(PCBCuShieldSolid,
			CGAGeometryManager::GetMaterial("G10"),
			"PCBCuShieldLogical", 
			0, 0, 0);
  G4VisAttributes* PCBCuShieldVisAtt = 
    new G4VisAttributes(G4Colour(0.5,0.5,0));
  PCBCuShieldVisAtt->SetForceWireframe(true);
  PCBCuShieldVisAtt->SetVisibility(false);
  PCBCuShieldLog->SetVisAttributes(PCBCuShieldVisAtt);
  new MyPlacement(0,
		  G4ThreeVector(0,
				0,
				y_slab_floor +
				PCBCuShield_thickness/2),
		  PCBCuShieldLog,
		  "PCBCuShield",
		  SlabLog,
		  false,0);
  return SlabLog;
}
void
SEcal03::BuildECRingAlveolus (G4int layer_id,
			      G4double z_floor,
			      G4LogicalVolume* ECLog,
			      G4bool Zminus)
{
  G4LogicalVolume *SiLog = ECRingSiLog;
  if(Zminus) SiLog = ECRingSiLogZminus;
  
  // place Si 1 (in z_floor + Ecal_total_Slab_thickness / 2)
  new MyPlacement(0,
		  G4ThreeVector (0,
				 0,
				 z_floor + Ecal_total_Slab_thickness / 2),
		  SiLog,
		  "SlabSiECRing",
		  ECLog,
		  false,
		  layer_id);

  // set layer in SD
  if(!Zminus)
    theEndCapRingSD->
      AddLayer(layer_id,
	       - ECRingSiBox->GetXHalfLength(),
	       - ECRingSiBox->GetYHalfLength(),
	       z_floor + Ecal_total_Slab_thickness / 2);

  z_floor += Ecal_total_Slab_thickness + Ecal_fiber_thickness;

  // place Rad (in z_floor + Ecal_total_Slab_thickness +
  // N X fiber + RadThick / 2)

  G4LogicalVolume *radiatorLog = NULL;
  G4double RadiatorThickness = 0.;
  if(layer_id <= Ecal_nlayers1) 
    {
      radiatorLog=EndCapRingSlabRadiatorL1;
      RadiatorThickness = Ecal_radiator_thickness1;
    }
  if(layer_id > Ecal_nlayers1 &&
     layer_id <= Ecal_nlayers1 + Ecal_nlayers2)
    {
      radiatorLog=EndCapRingSlabRadiatorL2;
      RadiatorThickness = Ecal_radiator_thickness1;
    }
  if(layer_id > Ecal_nlayers1 + Ecal_nlayers2 && 
     layer_id <= Ecal_nlayers1 + Ecal_nlayers2 + Ecal_nlayers3)
    {
      radiatorLog=EndCapRingSlabRadiatorL3;
      RadiatorThickness = Ecal_radiator_thickness1;
    }
  
  new MyPlacement(0,
		  G4ThreeVector (0,
				 0,
				 z_floor + RadiatorThickness / 2),
		  radiatorLog,
		  "SlabRadECRing",
		  ECLog,
		  false,
		  0);
  z_floor += RadiatorThickness + Ecal_fiber_thickness;;

  // place Si 2 (in z_floor + Ecal_total_Slab_thickness +
  // N X fiber + RadThick +  N X fiber + Ecal_total_Slab_thickness / 2)
  new MyPlacement(0,
		  G4ThreeVector (0,
				 0,
				 z_floor + Ecal_total_Slab_thickness / 2),
		  SiLog,
		  "SlabSiECRing",
		  ECLog,
		  false,
		  layer_id + 1);

  if(!Zminus)
    theEndCapRingSD->
      AddLayer(layer_id+1,
	       - ECRingSiBox->GetXHalfLength(),
	       - ECRingSiBox->GetYHalfLength(),
	       z_floor + Ecal_total_Slab_thickness / 2);
}

G4double 
SEcal03::BuildECAlveolus (G4int layer_id,
			  G4double z_floor,
			  G4LogicalVolume* ECLog,
			    G4bool Zminus)
{
  G4double AlveolusThickness = 0;
  G4double W_thick =
    GiveMeRadiatorThickness(layer_id);

  G4LogicalVolume* radiatorLog = NULL;
  
  AlveolusThickness =
    2 * Ecal_total_Slab_thickness +
    W_thick + 
    2 * Ecal_fiber_thickness;
  
  G4RotationMatrix rotOdd;
  rotOdd.rotateZ(pi);
  rotOdd.rotateX(pi);

  G4RotationMatrix rotEven;
  rotEven.rotateZ(pi);

  G4RotationMatrix *rot;
  G4int limit_staves;
  // **********************
  limit_staves = 4;
  //limit_staves = 1;
  for (G4int i_stave = 1;
       i_stave <= limit_staves;
       i_stave ++)
    {
      G4double angle_module = pi/2 * ( i_stave - 1 );
      if(layer_id==1)
	theEndCapSD->SetStaveRotationMatrix(i_stave,angle_module);

      for (unsigned int i_tower=0;
	   i_tower < EC_TowerSlabs.size();
	   i_tower++)
	{
	  // first slab
	  G4ThreeVector pos = EC_TowerXYCenters[i_tower];
	  pos[2] = z_floor + Ecal_total_Slab_thickness /2;
	  rot = new G4RotationMatrix(rotOdd);
	  rot->rotateZ(angle_module);
	  pos.rotateZ(angle_module);

	  // ******************
	  new MyPlacement(rot,
			  pos,
			  EC_TowerSlabs[i_tower],
			  "SlabEC",
			  ECLog,
			  false,
			  i_stave*100000 +
			  (i_tower+1) * 1000 + 
			  layer_id);
      
	  if (i_stave == 1  
	      && i_tower == 0
	      && !Zminus) 
	    theEndCapSD->
	      AddLayer(layer_id,
		       - pos[0]
		       - ((G4Box *)EC_TowerSlabs[i_tower]
			  ->GetSolid())->GetXHalfLength(),
		       pos[2]
		       - Si_Slab_Y_offset,
		       pos[1]
		       - ((G4Box *)EC_TowerSlabs[i_tower]
			  ->GetSolid())->GetYHalfLength());

	  // radiator inside alveolus
	  if(layer_id <= Ecal_nlayers1) 
	    {
	      radiatorLog=EC_TowerR1[i_tower];
	    }
	  if(layer_id > Ecal_nlayers1 &&
	     layer_id <= Ecal_nlayers1 + Ecal_nlayers2)
	    {
	      radiatorLog=EC_TowerR2[i_tower];
	    }
	  if(layer_id > Ecal_nlayers1 + Ecal_nlayers2 && 
	     layer_id <= Ecal_nlayers1 + Ecal_nlayers2 + Ecal_nlayers3)
	    {
	      radiatorLog=EC_TowerR3[i_tower];
	    }

	  // reinitialise pos as it's now rotated for the slab.
	  pos = EC_TowerXYCenters[i_tower];
	  pos[2] = z_floor + Ecal_total_Slab_thickness /2;
	  // update pos to take in account slab + fiber dim
	  pos[2] += 
	    Ecal_total_Slab_thickness / 2 
	    + Ecal_fiber_thickness 
	    + W_thick/2;
	  
	  rot = new G4RotationMatrix(rotEven);
	  rot->rotateZ(angle_module);
	  pos.rotateZ(angle_module);

	  new MyPlacement(rot,
			  pos,
			  radiatorLog,
			  "RadEC",
			  ECLog,
			  false,0);
#ifdef MOKKA_GEAR
	  if(!Zminus)
	    {
	      if (i_stave == 1  
		  && i_tower == 0) 
		{
		  // get positions of Layer as the middle of the radiator layer 
		  helpEndcap.layerPos.push_back(pos[2]) ;
		  helpPlug.layerPos.push_back(pos[2]) ;
		  
		  // get radiator thickness
		  helpEndcap.radiThickness.
		    push_back(W_thick);
		  helpPlug.radiThickness.
		    push_back(W_thick);
		  
		  // count layers
		  helpEndcap.count ++ ;
		  helpPlug.count ++ ;
		}
	    }
#endif
	  
	  // second slab
	  pos[2] +=  W_thick/2
	    + Ecal_fiber_thickness
	    + Ecal_total_Slab_thickness /2;

	  //rot = new G4RotationMatrix(rotEven);
	  rot = new G4RotationMatrix(rotOdd);
	  rot->rotateX(pi);
	  rot->rotateZ(angle_module);

	  if(i_stave % 2 == 1 )
	    rot->rotateX(pi);

	  // ***************
	  new MyPlacement(rot,
			  pos,
			  EC_TowerSlabs[i_tower],
			  "SlabEC",
			  ECLog,
			  false,
			  i_stave*100000 +
			  (i_tower+1) * 1000 + 
			  layer_id+1);

	  if (i_stave == 1  
	      && i_tower == 0
	      && !Zminus) 
	    theEndCapSD->
	      AddLayer(layer_id+1,
		       - pos[0]
		       - ((G4Box *)EC_TowerSlabs[i_tower]
			  ->GetSolid())->GetXHalfLength(),
		       pos[2]
		       + Si_Slab_Y_offset,
		       pos[1]
		       - ((G4Box *)EC_TowerSlabs[i_tower]
			  ->GetSolid())->GetYHalfLength());
	}
    }
  
  return AlveolusThickness;
}

G4bool SEcal03::EC_Initialize()
{  
  // EC_Initialize() builds the Slabs and the 
  // radiator plates for the several towers 
  // to be placed latter into each module
 
  //=================================================
  //
  // Take care:
  //
  // To turn the direction of slabs in the end caps
  // we interchanged X<->Y. It's not clean, but it
  // works...
  //=================================================

  // Main parameters for the virtual EC module
  EC_y_botton =
    + Ecal_endcap_center_box_size / 2
    + Ecal_lateral_face_thickness;
  
  G4double y_middle =
    (Ecal_inner_radius + module_thickness + Ecal_endcap_extra_size) 
    * tan(pi/8)
    - Ecal_lateral_face_thickness;

  G4double EC_y_top =
    Ecal_inner_radius + module_thickness + Ecal_endcap_extra_size
    - Ecal_lateral_face_thickness;
    
  G4double x_left =
    -(Ecal_inner_radius + module_thickness + Ecal_endcap_extra_size)
    + Ecal_lateral_face_thickness;

  G4double x_middle = -y_middle;

//   G4double x_right =
//       - Ecal_endcap_center_box_size / 2
//     - Ecal_lateral_face_thickness;
  G4double x_right =
    + Ecal_endcap_center_box_size / 2
    - Ecal_lateral_face_thickness;
  
  fiber_inter_alveolus =
    2 * (Ecal_Slab_H_fiber_thickness 
	 + Ecal_Slab_shielding
	 + N_FIBERS_ALVOULUS * Ecal_fiber_thickness);
  
  EC_alveolus_dim_y = alveolus_dim_z;
  G4double EC_alveolus_x_left = 0;
  G4double EC_alveolus_dim_x = 0 ;
  G4double alv_upper_y = 0;
  G4double inc = 
    (x_middle -x_left ) 
    / (EC_y_top - y_middle);

  G4int EC_Number_of_towers = 0;
  G4double last_dim_x = 0;
  G4double y_floor = EC_y_botton;
  while ( ( y_floor + EC_alveolus_dim_y) < EC_y_top )
    {
      alv_upper_y = y_floor + EC_alveolus_dim_y;
      if( alv_upper_y <= y_middle )
	{
	  EC_alveolus_dim_x = x_right- x_left;
	}
      else
	{
	  EC_alveolus_x_left = 
	    (alv_upper_y - y_middle) * inc + x_left;
	  EC_alveolus_dim_x = x_right
	    - EC_alveolus_x_left;
	}

      // We use the same method able to create the Barrel
      // Slabs, so we have to rotate it later when placing 
      // into the EC modules.
      //
      // We use the same method able to create the Barrel
      // radiator plates between slabs, but with the good
      // dimensions to avoid to rotate it later when placing 
      // into the EC modules.

      // While the towers have the same shape use the same 
      // logical volumes and parameters.
      if(last_dim_x != EC_alveolus_dim_x)
	{
	  EC_TowerSlabs.
	    push_back(BuildSlab(EC_alveolus_dim_y,
				Ecal_total_Slab_thickness,
				EC_alveolus_dim_x,
				theEndCapSD));
	  if( Ecal_nlayers1 > 0 )
	    {
	      EC_TowerR1.
		push_back(BuildRadiatorPlate(EC_alveolus_dim_y,
					     Ecal_radiator_thickness1,
					     EC_alveolus_dim_x));
	    }
	  if( Ecal_nlayers2 > 0 )
	    {
	      EC_TowerR2.
		push_back(BuildRadiatorPlate(EC_alveolus_dim_y,
					     Ecal_radiator_thickness2,
					     EC_alveolus_dim_x));
	    }
	  if( Ecal_nlayers3 > 0 )
	    {
	      EC_TowerR3.
		push_back(BuildRadiatorPlate(EC_alveolus_dim_y,
					     Ecal_radiator_thickness3,
					     EC_alveolus_dim_x));
	    }
	  
	  last_dim_x = EC_alveolus_dim_x;
	}
      else
	{
	  EC_TowerSlabs.
	    push_back(EC_TowerSlabs[EC_Number_of_towers-1]);
	  if( Ecal_nlayers1 > 0 )
	    EC_TowerR1.
	      push_back(EC_TowerR1[EC_Number_of_towers-1]);
	  if( Ecal_nlayers2 > 0 )
	    EC_TowerR2.
	      push_back(EC_TowerR2[EC_Number_of_towers-1]);
	  if( Ecal_nlayers3 > 0 )
	    EC_TowerR3.
	      push_back(EC_TowerR3[EC_Number_of_towers-1]);
	}
      EC_TowerXYCenters.
	push_back(G4ThreeVector(-(y_floor + EC_alveolus_dim_y/2),
				-(-EC_alveolus_dim_x/2 + x_right),
				0));
      
      EC_Number_of_towers++;
      // Update y_floor
      y_floor += EC_alveolus_dim_y + fiber_inter_alveolus;
    }
  G4cout << "\nFor information: Ecal endcap modules have "
	 << EC_Number_of_towers
	 << " towers,\n" 
	 << EC_y_top - y_floor 
	 << " mm are lost (no enough space left for more one tower)."
	 << G4endl;

  return true;  
}
G4bool 
SEcal03::PostConstructAction(CGAGeometryEnvironment& theGeometryEnvironment)
{
  // propagates the actual Ecal outer radius and endcap zmax to 
  // avoir overlaps by the Hcal, if any. 
  std::ostringstream oss;  
  oss << Ecal_inner_radius + module_thickness;
  (*Control::globalModelParameters)["Ecal_outer_radius"] = 
    oss.str();
  
  std::ostringstream oss2;
  oss2 << EC_module_z_offset + module_thickness / 2;
  (*Control::globalModelParameters)["Ecal_endcap_zmax"] =
    oss2.str();


  // propagates the actual Ecal endcap zmin and outer radius 
  // to aline the Hcal rings, if any.
  std::ostringstream oss_endcap1;  
  oss_endcap1 << EC_module_z_offset - module_thickness / 2;
  (*Control::globalModelParameters)["Ecal_endcap_zmin"] = 
    oss_endcap1.str();

  // Modifies Lcal_z_begin, as the LCal01 driver use its
  // own parameter.
  std::ostringstream oss_Lcal;
  oss_Lcal << EC_module_z_offset - module_thickness / 2;
  (*Control::globalModelParameters)["Lcal_z_begin"] =
    oss_Lcal.str();
  // propagates the actual inner radius of Ecal plug
  // needed by LCAL to avoid overlap
  std::ostringstream oss_plugR;
  oss_plugR <<  theGeometryEnvironment.GetParameterAsDouble("Ecal_Lcal_ring_gap") 
              + theGeometryEnvironment.GetParameterAsDouble("Lcal_outer_radius");
  (*Control::globalModelParameters)["Ecal_endcap_plug_rmin"] = oss_plugR.str();

  std::ostringstream oss_endcap2;  
  oss_endcap2 << Ecal_endcap_rmax ;/// cos(pi/8);
  (*Control::globalModelParameters)["Ecal_endcap_outer_radius"] = 
    oss_endcap2.str();

  
  //
  // The Ecal driver has the responsability to change the tracker region parameters.
  //
  (*Control::globalModelParameters)["tracker_region_rmax"] =
    theGeometryEnvironment.GetParameterAsString("TPC_outer_radius");
  
  std::ostringstream oss3;
  oss3 << EC_module_z_offset 
    - module_thickness / 2;
  (*Control::globalModelParameters)["tracker_region_zmax"] =
    oss3.str();
  
  G4cout << "SEcal information: Ecal_inner_radius = "
	 << Ecal_inner_radius 
	 << "\n                  Ecal_outer_radius = " 
	 <<  (*Control::globalModelParameters)["Ecal_outer_radius"]
   	 << "\n                  module thickness  = " 
	 << module_thickness
   	 << "\n                  Ecal_endcap_outer_radius = " 
	 << (*Control::globalModelParameters)["Ecal_endcap_outer_radius"]
   	 << "\n                  Ecal_endcap_zmin = " 
	 << (*Control::globalModelParameters)["Ecal_endcap_zmin"]
   	 << "\n                  Ecal_endcap_zmax = " 
	 << (*Control::globalModelParameters)["Ecal_endcap_zmax"]
  	 << "\n                  Size of Si plates in Ecal ring  = "
	 << ECRingSiplateSize
   	 << " mm" << G4endl;

  G4cout << "For backward compatibility, setting TPC_Ecal_Hcal_barrel_halfZ to "
	 << Ecal_Barrel_halfZ
	 << G4endl;
  std::ostringstream oss4;
  oss4 << Ecal_Barrel_halfZ;
  (*Control::globalModelParameters)["TPC_Ecal_Hcal_barrel_halfZ"] =
    oss4.str();

  return true;
}
