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
// $Id: SEcal05.cc,v 1.1 2009/06/04 11:40:42 musat Exp $
// $Name: mokka-07-00 $
//
// 
//
// SEcal05.cc
//

#include "Control.hh"
#include "MyPlacement.hh"
#include "G4UserLimits.hh"
#include "SEcal05.hh"
#include "CGAGeometryManager.hh"
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
#include "G4PVReplica.hh"
#include "G4GeometryTolerance.hh"
    
#include "CGADefs.h"
#include "UserInit.hh"
    
#ifdef MOKKA_GEAR
#include "gear/CalorimeterParameters.h"
#include "gearimpl/CalorimeterParametersImpl.h"
#include "gearimpl/LayerLayoutImpl.h"
#include "MokkaGear.h"
#endif
    
INSTANTIATE(SEcal05)

#define N_FIBERS_ALVEOLUS 3
#define N_FIBERS_W_STRUCTURE 2

#define VERBOSE 1
#define SECAL_CHECK_OVERLAP 0

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
G4bool SEcal05::ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
				    G4LogicalVolume *theWorld)
{
  EnvLogEcalModuleBarrel = 0;
  EnvLogEcalModuleEndCap = 0;

  theBarrelScintillatorSD = 0;
  theBarrelSiliconSD      = 0;
  theEndCapScintillatorSD = 0;
  theEndCapSiliconSD      = 0;

  G4cout << "\nBuilding Ecal- SEcal05"<< G4endl;

  Ecal_Scale_Endcap = true;
  
  /* Initialize the Geant3 interface*/
  if(Control::DUMPG3) MyPlacement::Init("ECAL","SEcal05");

  /* Initialize the driver*/
  if (!Setup(aGeometryEnvironment)) return false;
  DefineMaterial();
  if (!Build(theWorld))  return false;
  return true;
}

/*********************************************************************************************/
/*                                                                                           */
/* retrieve the setup parameters and initialize the driver                                   */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
G4bool SEcal05::Setup(const CGAGeometryEnvironment &theGeometryEnvironment)
{
  Ecal_Alveolus_Air_Gap      = theGeometryEnvironment.GetParameterAsDouble("Ecal_Alveolus_Air_Gap");
  Ecal_Slab_shielding        = theGeometryEnvironment.GetParameterAsDouble("Ecal_Slab_shielding");
  Ecal_Slab_copper_thickness = theGeometryEnvironment.GetParameterAsDouble("Ecal_Slab_copper_thickness");
  Ecal_Slab_PCB_thickness    = theGeometryEnvironment.GetParameterAsDouble("Ecal_Slab_PCB_thickness");
  Ecal_Slab_glue_gap         = theGeometryEnvironment.GetParameterAsDouble("Ecal_Slab_glue_gap");
  Ecal_Slab_ground_thickness = theGeometryEnvironment.GetParameterAsDouble("Ecal_Slab_ground_thickness");
  Ecal_fiber_thickness       = theGeometryEnvironment.GetParameterAsDouble("Ecal_fiber_thickness");
  Ecal_Si_thickness          = theGeometryEnvironment.GetParameterAsDouble("Ecal_Si_thickness");
  Ecal_guard_ring_size       = theGeometryEnvironment.GetParameterAsDouble("Ecal_guard_ring_size");

  Ecal_radiator_material = theGeometryEnvironment.GetParameterAsString("Ecal_radiator_material");

  if(Ecal_radiator_material == "tungsten")
    {
      RadiatorMaterial =  CGAGeometryManager::GetMaterial("tungsten_19.3gccm");
    }
  else
    {
      if(Ecal_radiator_material == "lead")
	{
	  RadiatorMaterial =  CGAGeometryManager::GetMaterial("lead");
	}
    else 
      {
	Control::Abort("SEcal05: invalid radiator material name. \nIt has to be either tungsten or lead", MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
      }
    }

  G4String MaterialWarning = Ecal_radiator_material + " is the radiator material being placed.";
  Control::Log(MaterialWarning.data());

  Ecal_inner_radius = theGeometryEnvironment.GetParameterAsDouble("TPC_outer_radius") 
    + theGeometryEnvironment.GetParameterAsDouble("Ecal_Tpc_gap");
  Ecal_radiator_thickness1 = theGeometryEnvironment.GetParameterAsDouble("Ecal_radiator_layers_set1_thickness");
  Ecal_radiator_thickness2 = theGeometryEnvironment.GetParameterAsDouble("Ecal_radiator_layers_set2_thickness");
  Ecal_radiator_thickness3 = theGeometryEnvironment.GetParameterAsDouble("Ecal_radiator_layers_set3_thickness");
  Ecal_Barrel_halfZ        = theGeometryEnvironment.GetParameterAsDouble("Ecal_Barrel_halfZ");

  Ecal_cell_size              = theGeometryEnvironment.GetParameterAsDouble("Ecal_cells_size");
  Ecal_cables_gap             = theGeometryEnvironment.GetParameterAsDouble("Ecal_cables_gap");
  Ecal_endcap_center_box_size = theGeometryEnvironment.GetParameterAsDouble("Ecal_endcap_center_box_size");
  Lcal_outer_radius           = theGeometryEnvironment.GetParameterAsDouble("Lcal_outer_radius");

  Ecal_Lcal_ring_gap = theGeometryEnvironment.GetParameterAsDouble("Ecal_Lcal_ring_gap");
  Ecal_EC_Ring_gap   = theGeometryEnvironment.GetParameterAsDouble("Ecal_EC_Ring_gap");

  TUBE_crossing_angle = theGeometryEnvironment.GetParameterAsDouble("TUBE_crossing_angle");

  Ecal_endcap_extra_size       = theGeometryEnvironment.GetParameterAsDouble("Ecal_endcap_extra_size");
  Ecal_nlayers1                = theGeometryEnvironment.GetParameterAsInt("Ecal_nlayers1");
  Ecal_nlayers2                = theGeometryEnvironment.GetParameterAsInt("Ecal_nlayers2");
  Ecal_nlayers3                = theGeometryEnvironment.GetParameterAsInt("Ecal_nlayers3");
  Ecal_support_thickness       = theGeometryEnvironment.GetParameterAsDouble("Ecal_support_thickness");
  Ecal_front_face_thickness    = theGeometryEnvironment.GetParameterAsDouble("Ecal_front_face_thickness");
  Ecal_lateral_face_thickness  = theGeometryEnvironment.GetParameterAsDouble("Ecal_lateral_face_thickness");
  Ecal_Slab_H_fiber_thickness  = theGeometryEnvironment.GetParameterAsDouble("Ecal_Slab_H_fiber_thickness");
  Ecal_barrel_number_of_towers = theGeometryEnvironment.GetParameterAsInt("Ecal_barrel_number_of_towers");
  
  
  /* Calculed parameters.
     Ecal_total_SiSlab_thickness = just one slab side in the H, from shielding to mass*/

  total_number_of_layers = Ecal_nlayers1 + Ecal_nlayers2 + Ecal_nlayers3;
  if((total_number_of_layers % 2) == 0)
    {
      Control::Abort("SEcal05: total number of layers has to be odd!", MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
    }
  
  /*------------------------------------------------------------------------------
    Angela Lucaci: add scintillator cell sizes varying per layer */
    inputEcal_Sc_cellDim1_string = UserInit::getInstance()->getString("Ecal_Sc_cellDim1_string");
    inputEcal_Sc_cellDim2_string = UserInit::getInstance()->getString("Ecal_Sc_cellDim2_string");
    this->FillCellDimVector(inputEcal_Sc_cellDim1_string, inputEcal_Sc_cellDim1_vector);
    this->FillCellDimVector(inputEcal_Sc_cellDim2_string, inputEcal_Sc_cellDim2_vector);

    if (!inputEcal_Sc_cellDim1_string.empty() && !inputEcal_Sc_cellDim2_string.empty()) useScintillatorTilesOfVaryingSize = true;
    else useScintillatorTilesOfVaryingSize = false;
    /*------------------------------------------------------------------------------*/

  Ecal_total_SiSlab_thickness = 
    Ecal_Slab_shielding + 
    Ecal_Slab_copper_thickness + 
    Ecal_Slab_PCB_thickness +
    Ecal_Slab_glue_gap + 
    Ecal_Si_thickness + 
    Ecal_Slab_ground_thickness +
    Ecal_Alveolus_Air_Gap / 2;

#ifdef VERBOSE
  G4cout << " Ecal_total_SiSlab_thickness = " << Ecal_total_SiSlab_thickness  << G4endl;
#endif
  

  Ecal_Slab_Sc_PCB_thickness  = theGeometryEnvironment.GetParameterAsDouble("Ecal_Slab_Sc_PCB_thickness");
  Ecal_Sc_thickness           = theGeometryEnvironment.GetParameterAsDouble("Ecal_Sc_thickness");
  Ecal_Sc_reflector_thickness = theGeometryEnvironment.GetParameterAsDouble("Ecal_Sc_reflector_thickness");
  Ecal_MPPC_size              = theGeometryEnvironment.GetParameterAsDouble("Ecal_MPPC_size");
  Ecal_Sc_MPPC_breadth        = theGeometryEnvironment.GetParameterAsDouble("Ecal_Sc_MPPC_breadth");

  Ecal_Sc_N_strips_across_module = theGeometryEnvironment.GetParameterAsInt("Ecal_Sc_N_strips_across_module");
  if(Ecal_Sc_N_strips_across_module == 0)
    {
      Control::Abort("SEcal05: Parameter \"Ecal_Sc_N_strips_across_module\" must be different from zero", MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
    }

  Ecal_Sc_number_of_virtual_cells = theGeometryEnvironment.GetParameterAsInt("Ecal_Sc_number_of_virtual_cells");
  if(Ecal_Sc_number_of_virtual_cells == 0)
    {
      Control::Abort("SEcal05: Parameter \"Ecal_Sc_number_of_virtual_cells\" must be different from zero", MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
    }
  
  Ecal_Barrel_Sc_Si_Mix = theGeometryEnvironment.GetParameterAsString("Ecal_Sc_Si_mix");
  Ecal_EndCap_Sc_Si_Mix = Ecal_Barrel_Sc_Si_Mix;

  Ecal_total_ScSlab_thickness = 
    Ecal_Slab_shielding + 
    Ecal_Slab_copper_thickness + 
    Ecal_Slab_Sc_PCB_thickness +
    Ecal_Sc_thickness + 
    Ecal_Sc_reflector_thickness * 2 +
    Ecal_Alveolus_Air_Gap / 2;
#ifdef VERBOSE
  G4cout << " Ecal_total_ScSlab_thickness = " << Ecal_total_ScSlab_thickness  << G4endl;
#endif
  
    if(Ecal_Barrel_Sc_Si_Mix.size() != (unsigned int)((total_number_of_layers + 1)/2))
      {
	Control::Abort("SEcal05: The size of Ecal_Barrel_Sc_Si_Mix is not consistent with the total number of layers",
		       MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
      }

    Number_of_Si_Layers_in_Barrel = 0;
    Number_of_Sc_Layers_in_Barrel = 0;

    for(unsigned int i_char = 0; i_char < Ecal_Barrel_Sc_Si_Mix.size(); i_char++)
      {
	switch(Ecal_Barrel_Sc_Si_Mix[i_char]) 
	  {
	  case ECAL_SI_LAYERS:
	    Number_of_Si_Layers_in_Barrel += 2;
	    break;
      
	  case ECAL_SC_LAYER_1_2_ALONG_X:
      
	  case ECAL_SC_LAYER_1_2_ALONG_Z:
      
	  case ECAL_SC_LAYER_1_ALONG_X_LAYER_2_ALONG_Z:
      
	  case ECAL_SC_LAYER_1_ALONG_Z_LAYER_2_ALONG_X:
	    Number_of_Sc_Layers_in_Barrel += 2;
	    break;
      
	  case ECAL_MIX_LAYER_1_SI_LAYER_2_ALONG_X:
      
	  case ECAL_MIX_LAYER_1_SI_LAYER_2_ALONG_Z:
      
	  case ECAL_MIX_LAYER_1_ALONG_X_LAYER_2_SI:
      
	  case ECAL_MIX_LAYER_1_ALONG_Z_LAYER_2_SI:
	    Number_of_Si_Layers_in_Barrel++;
	    Number_of_Sc_Layers_in_Barrel++;
	    break;
	    
	  default:
	    G4cout << i_char << " " << Ecal_Barrel_Sc_Si_Mix[i_char] << G4endl;
	    Control::Abort("SEcal05 a: The Ecal_Barrel_Sc_Si_Mix parameter should contain only 0,1,2,3,4,5,6,7,9",
			   MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
	  };
      }
    
    Number_of_Si_Layers_in_EC = 0;
    Number_of_Sc_Layers_in_EC = 0;
    EC_Scint_Along_X = false;
    EC_Scint_Along_Z = false;

    for(unsigned int i_char = 0; i_char < Ecal_EndCap_Sc_Si_Mix.size(); i_char++)
      {
	switch(Ecal_EndCap_Sc_Si_Mix[i_char]) 
	  {
	  case ECAL_SI_LAYERS:
	    Number_of_Si_Layers_in_EC += 2;
	    break;
     
	  case ECAL_SC_LAYER_1_2_ALONG_X:
	    EC_Scint_Along_X = true;
	    Number_of_Sc_Layers_in_EC += 2;
	    break;
      
	  case ECAL_SC_LAYER_1_2_ALONG_Z:
	    EC_Scint_Along_Z = true;
	    Number_of_Sc_Layers_in_EC += 2;
	    break;

	  case ECAL_SC_LAYER_1_ALONG_X_LAYER_2_ALONG_Z:
	    EC_Scint_Along_X = true;
	    EC_Scint_Along_Z = true;
	    Number_of_Sc_Layers_in_EC += 2;
	    break;
      
	  case ECAL_SC_LAYER_1_ALONG_Z_LAYER_2_ALONG_X:
	    EC_Scint_Along_X = true;
	    EC_Scint_Along_Z = true;
	    Number_of_Sc_Layers_in_EC += 2;
	    break;

	  case ECAL_MIX_LAYER_1_SI_LAYER_2_ALONG_X:
	    EC_Scint_Along_X = true;
	    Number_of_Sc_Layers_in_EC++;
	    Number_of_Si_Layers_in_EC++;
	    break;

	  case ECAL_MIX_LAYER_1_SI_LAYER_2_ALONG_Z:
	    EC_Scint_Along_Z = true;
	    Number_of_Sc_Layers_in_EC++;
	    Number_of_Si_Layers_in_EC++;
	    break;
	    
	  case ECAL_MIX_LAYER_1_ALONG_X_LAYER_2_SI:
	    EC_Scint_Along_X = true;
	    Number_of_Sc_Layers_in_EC++;
	    Number_of_Si_Layers_in_EC++;
	    break;
	    
	  case ECAL_MIX_LAYER_1_ALONG_Z_LAYER_2_SI:
	    EC_Scint_Along_Z = true;
	    Number_of_Sc_Layers_in_EC++;
	    Number_of_Si_Layers_in_EC++;
	    break;
	    
	  default:
	    G4cout << i_char << " " << Ecal_EndCap_Sc_Si_Mix[i_char] << G4endl;
	    Control::Abort("SEcal05 b: The Ecal_Barrel_Sc_Si_Mix parameter should contain only 0's, 1's, 2's, 3's or 4's",
			   MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
	  };
      }
    
    
#ifdef VERBOSE
    G4cout << " Ecal total number of Silicon layers = " << Number_of_Si_Layers_in_Barrel  << G4endl;
    G4cout << " Ecal total number of Scintillator layers = " << Number_of_Sc_Layers_in_Barrel  << G4endl;
#endif
  
    /* In this release the number of modules is fixed to 5*/
  Ecal_Barrel_module_dim_z = 2 * Ecal_Barrel_halfZ / 5. ;
#ifdef VERBOSE
  G4cout << "Ecal_Barrel_module_dim_z  = " << Ecal_Barrel_module_dim_z  << G4endl;
#endif

  /* The alveolus size takes in account the module Z size
     but also 4 fiber layers for the alveoulus wall, the all
     divided by the number of towers*/
  alveolus_dim_z = (Ecal_Barrel_module_dim_z - 2. * Ecal_lateral_face_thickness) / Ecal_barrel_number_of_towers - 
    2 * N_FIBERS_ALVEOLUS  * Ecal_fiber_thickness  - 
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
    
    int(n_total_layers/2) * /* fiber around W struct layers*/
    (N_FIBERS_W_STRUCTURE * 2 *  Ecal_fiber_thickness) +
    
    Number_of_Si_Layers_in_Barrel * /* Silicon slabs plus fiber around and inside*/
    (Ecal_total_SiSlab_thickness +
     (N_FIBERS_ALVEOLUS + 1 ) * Ecal_fiber_thickness) +
    
    Number_of_Sc_Layers_in_Barrel * /* Scintillator slabs plus fiber around and inside*/
    (Ecal_total_ScSlab_thickness +
     (N_FIBERS_ALVEOLUS + 1 ) * Ecal_fiber_thickness) +
    
    Ecal_support_thickness + Ecal_front_face_thickness;
  
  G4cout << "For information : module_thickness = " << module_thickness  << G4endl;

  Ecal_endcap_rmax = Ecal_inner_radius + module_thickness + Ecal_endcap_extra_size;
  EC_y_botton      = Ecal_endcap_center_box_size / 2 + Ecal_lateral_face_thickness;
  EC_y_top         = Ecal_inner_radius + module_thickness + Ecal_endcap_extra_size - Ecal_lateral_face_thickness;

  G4double EC_y_height = EC_y_top - EC_y_botton;

  if (Ecal_Scale_Endcap) 
    { 
      /* vary width of alveolus in endcap to fill space*/
      EC_n_alveolus = int((EC_y_height/alveolus_dim_z) + 0.5); /* make endcap alvs similar in size to barrel ones*/
      EC_alveolus_width = EC_y_height/EC_n_alveolus 
	- 2 * N_FIBERS_ALVEOLUS * Ecal_fiber_thickness 
	- 2 * Ecal_Slab_H_fiber_thickness
	- 2 * Ecal_Slab_shielding;
      
      G4cout << "************************************" << G4endl;
      G4cout << "WARNING redefining width of endcap alveolus to fill available space !!!!" << G4endl;
      G4cout << "new endcap width = " << EC_alveolus_width << " number of alveolii = " << EC_n_alveolus << G4endl;
      G4cout << "for information, barrel alveolar width = " << alveolus_dim_z << G4endl;
      G4cout << "************************************" << G4endl;
      
    } 
  else 
    { 
      /* fixed alveolus width in endcap (same as in barrel)*/    
      EC_alveolus_width = alveolus_dim_z;
      EC_n_alveolus = int(EC_y_height/alveolus_dim_z);
      
      G4cout << "************************************" << G4endl;
      G4cout << "using identical alveolar width in endcap as in barrel: may lose some space at edge of endcap " << G4endl;
      G4cout << "endcap width = " << EC_alveolus_width << " number of alveolii = " << EC_n_alveolus << G4endl;
      G4cout << "************************************" << G4endl;      
    }


  /* initialize the central box in endcaps
     Central box become a Tub...*/

  CenterECTub = new G4Tubs ("CenterECTub",
			    0.,
			    Lcal_outer_radius + Ecal_Lcal_ring_gap,
			    module_thickness,
			    0.,
			    2 * pi);
  
  /* module barrel key parameters*/
  bottom_dim_x        = 2. * tan(pi/8.) * Ecal_inner_radius + module_thickness/sin(pi/4.);
  top_dim_x           = bottom_dim_x - 2 * module_thickness;
  endcap_module_dim_x = Ecal_endcap_center_box_size/2 + Ecal_inner_radius + module_thickness + Ecal_endcap_extra_size;

#ifdef VERBOSE
  G4cout << " bottom_dim_x = " << bottom_dim_x  << G4endl;
  G4cout << " top_dim_x = " << top_dim_x << G4endl;
  G4cout << " endcap_module_dim_x = " <<  endcap_module_dim_x << G4endl;
#endif

  EC_module_z_offset = Ecal_Barrel_module_dim_z * 2.5 + Ecal_cables_gap + module_thickness /2;  

  G4double centerTubDispl = EC_module_z_offset * tan(TUBE_crossing_angle /2000);
  FollowLcal              = new G4TranslateX3D (centerTubDispl);
  FollowLcalZminus        = new G4TranslateX3D (-centerTubDispl);
  
  /* Parameters for the old Ecal03 driver are kept here just for documentation:
     nmax_cell_x = 6, nmax_cell_z = 6, inter_wafer_gap = 0.15,
     n_guard_ring_zones = 3*/
  return true;
}


/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
G4bool SEcal05::Build(G4LogicalVolume* WorldLog)
{
  cell_dim_x = Ecal_cell_size;
  G4double total_Si_dim_z      = alveolus_dim_z;
  G4double util_SI_wafer_dim_z = total_Si_dim_z/2 -  2 * Ecal_guard_ring_size;

  //cell_dim_z =  util_SI_wafer_dim_z/ floor(util_SI_wafer_dim_z/cell_dim_x);
  //N_cells_in_Z = int(util_SI_wafer_dim_z/cell_dim_z);

  //Angela Lucaci: confused about the lines above,
  //I think the lines below give the same, but are easier to read:
  N_cells_in_Z = (G4int)floor(util_SI_wafer_dim_z/cell_dim_x);
  cell_dim_z = util_SI_wafer_dim_z/N_cells_in_Z;

  N_cells_in_X = N_cells_in_Z;
  cell_dim_x = cell_dim_z;
  theMaxStepAllowed = std::min(cell_dim_x, cell_dim_z);

#ifdef VERBOSE
  G4cout<<"\n\n====================================== Build"<<G4endl;
  G4cout<<" total_Si_dim_z:      " << total_Si_dim_z << G4endl;
  G4cout<<" util_SI_wafer_dim_z: " << util_SI_wafer_dim_z << G4endl;
  G4cout<<" cell_dim_x: "<<cell_dim_x<<G4endl;
  G4cout<<" cell_dim_z: "<<cell_dim_z<<G4endl;
  G4cout<<" Ecal_guard_ring_size: "<<Ecal_guard_ring_size<<G4endl;
  G4cout<<" N_cells_in_Z: "<<N_cells_in_Z<<G4endl;
  G4cout<<" N_cells_in_X: "<<N_cells_in_X<<G4endl;
#endif

  G4cout << "With the actual parameters for Ecal you have:\n cell_dim_z = " 
	 << cell_dim_z  
	 << "\n\n****>>>> Warning: \nEcal_cells_size redefined, forcing cell_dim_x to " << cell_dim_x
	 << " \nto insure squared Si cells!\n"
	 << G4endl;

  G4cout << " # of cells in X = " << N_cells_in_X << G4endl;
  G4cout << " # of cells in Z = " << N_cells_in_Z << G4endl<<G4endl;
  
  G4bool barID1Flag = true, ecID1Flag = true;
  G4double barDimX = bottom_dim_x;

  if((barDimX/cell_dim_x) < 511) 
    {
      barID1Flag = false;
    }

  G4double ecDimX = endcap_module_dim_x;

  if((ecDimX/cell_dim_x) < 511)
    {
      ecID1Flag = false;
    }

  /* The cell boundaries does not really exist as G4 volumes. So,
     to avoid long steps over running  several cells, the 
     theMaxStepAllowed inside the sensitive material is the
     pad smaller x or z dimension.*/
  theMaxStepAllowed = std::min(cell_dim_x, cell_dim_z);

  /* Daniel Jeans defines new cell size for endcap*/  
  EC_cell_dim_x = Ecal_cell_size;
  G4double EC_total_Si_dim_z      = EC_alveolus_width;
  G4double EC_util_SI_wafer_dim_z = EC_total_Si_dim_z/2 -  2*Ecal_guard_ring_size;

  //EC_cell_dim_z   =  EC_util_SI_wafer_dim_z/ floor(EC_util_SI_wafer_dim_z/EC_cell_dim_x);
  //EC_N_cells_in_Z = int(EC_util_SI_wafer_dim_z/EC_cell_dim_z);

  //Angela Lucaci: confused about the lines above,
  //I think the lines below give the same, but are easier to read:
  EC_N_cells_in_Z = (G4int)floor(EC_util_SI_wafer_dim_z/EC_cell_dim_x);
  EC_cell_dim_z   = EC_util_SI_wafer_dim_z/EC_N_cells_in_Z;

  EC_N_cells_in_X = EC_N_cells_in_Z;
  EC_cell_dim_x = EC_cell_dim_z;
  EC_theMaxStepAllowed= std::min(EC_cell_dim_x,EC_cell_dim_z);

#ifdef VERBOSE
  G4cout<<"\n EC total_Si_dim_z: " << EC_total_Si_dim_z << G4endl;
  G4cout<<" EC_util_SI_wafer_dim_z: " << EC_util_SI_wafer_dim_z << G4endl;
  G4cout<<" EC_cell_dim_x: "<<EC_cell_dim_x<<G4endl;
  G4cout<<" EC_cell_dim_z: "<<EC_cell_dim_z<<G4endl;
  G4cout<<" EC_N_cells_in_X: "<<EC_N_cells_in_X<<G4endl;
  G4cout<<" EC_N_cells_in_Z: "<<EC_N_cells_in_Z<<G4endl;
#endif

  G4cout << "With the actual parameters for Ecal endcap you have:\n EC_cell_dim_z = " 
	 << EC_cell_dim_z  
	 << "\n\n****>>>> Warning: \nEndCap Ecal_cells_size redefined, forcing cell_dim_x to " << EC_cell_dim_x
	 << " \nto ensure squared Si cells!\n"
	 << G4endl;

  G4cout << " # of cells in X (EC) = " << EC_N_cells_in_X << G4endl;
  G4cout << " # of cells in Z (EC) = " << EC_N_cells_in_Z << G4endl;

    // Adjust requested cell sizes in order to accommodate integral numbers of cells in each dimension
    if (useScintillatorTilesOfVaryingSize)
    {
        const unsigned int layerIndexEnd(total_number_of_layers + 1);

        if ((inputEcal_Sc_cellDim1_vector.size() != inputEcal_Sc_cellDim2_vector.size()) || (inputEcal_Sc_cellDim1_vector.size() != layerIndexEnd))
        {
            Control::Abort("SEcal05: Inconsistent cell dimension vectors", MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
        }

        // For the barrel region
        barrelEcal_Sc_cellDim1_vector.resize(layerIndexEnd);
        barrelEcal_Sc_cellDim2_vector.resize(layerIndexEnd);
        barrelEcal_Sc_N_strips_across_module_vector.resize(layerIndexEnd);
        barrelEcal_Sc_N_strip_containers_along_z_vector.resize(layerIndexEnd);

        for (unsigned int iLayer = 0; iLayer < layerIndexEnd; ++iLayer)
        {
            const double initialDim1(inputEcal_Sc_cellDim1_vector[iLayer]);
            const int nStripContainersAlongZ(2 * static_cast<int>(std::floor((alveolus_dim_z / 2.) / initialDim1)));
            const double correctedDim1(alveolus_dim_z / static_cast<double>(nStripContainersAlongZ));
            barrelEcal_Sc_cellDim1_vector[iLayer] = correctedDim1;
            barrelEcal_Sc_N_strip_containers_along_z_vector[iLayer] = nStripContainersAlongZ;

            const double initialDim2(inputEcal_Sc_cellDim2_vector[iLayer]);
            const int nStripsAcrossModule(static_cast<int>(std::floor(alveolus_dim_z / initialDim2)));
            const double correctedDim2(alveolus_dim_z / static_cast<double>(nStripsAcrossModule));
            barrelEcal_Sc_cellDim2_vector[iLayer] = correctedDim2;
            barrelEcal_Sc_N_strips_across_module_vector[iLayer] = nStripsAcrossModule;

            theMaxStepAllowed = std::min(theMaxStepAllowed, std::min(correctedDim1, correctedDim2));
        }

        // For the endcap region
        endcapEcal_Sc_cellDim1_vector.resize(layerIndexEnd);
        endcapEcal_Sc_cellDim2_vector.resize(layerIndexEnd);
        endcapEcal_Sc_N_strips_across_module_vector.resize(layerIndexEnd);
        endcapEcal_Sc_N_strip_containers_along_z_vector.resize(layerIndexEnd);

        for (unsigned int iLayer = 0; iLayer < layerIndexEnd; ++iLayer)
        {
            const double initialDim1(inputEcal_Sc_cellDim1_vector[iLayer]);
            const int nStripContainersAlongZ(2 * static_cast<int>(std::floor((EC_alveolus_width / 2.) / initialDim1)));
            const double correctedDim1(EC_alveolus_width / static_cast<double>(nStripContainersAlongZ));
            endcapEcal_Sc_cellDim1_vector[iLayer] = correctedDim1;
            endcapEcal_Sc_N_strip_containers_along_z_vector[iLayer] = nStripContainersAlongZ;

            const double initialDim2(inputEcal_Sc_cellDim2_vector[iLayer]);
            const int nStripsAcrossModule(static_cast<int>(std::floor(EC_alveolus_width / initialDim2)));
            const double correctedDim2(EC_alveolus_width / static_cast<double>(nStripsAcrossModule));
            endcapEcal_Sc_cellDim2_vector[iLayer] = correctedDim2;
            endcapEcal_Sc_N_strips_across_module_vector[iLayer] = nStripsAcrossModule;

            EC_theMaxStepAllowed = std::min(EC_theMaxStepAllowed, std::min(correctedDim1, correctedDim2));
        }
    }

  /* Sensitive detector for the Ecal barrel*/
  G4double HWallSize =  Ecal_Slab_H_fiber_thickness + Ecal_Slab_shielding;
  G4double TowerWallSize = N_FIBERS_ALVEOLUS * Ecal_fiber_thickness;

  if(Number_of_Si_Layers_in_Barrel != 0)
    {
     theBarrelSiliconSD = new SEcalSD02(cell_dim_x,
					cell_dim_z,
					Ecal_Si_thickness,
					N_cells_in_X,
					N_cells_in_Z,
					Ecal_guard_ring_size,
					HWallSize,
					TowerWallSize,
					ECALBARREL,
					"EcalBarrelSilicon",
					barID1Flag);
     RegisterSensitiveDetector(theBarrelSiliconSD);
    }

  barrelStripSizeinX = alveolus_dim_z / (2 * N_cells_in_Z);
  barrelStripSizeParallelToZ = alveolus_dim_z / Ecal_Sc_N_strips_across_module;
  virtualCellDim = barrelStripSizeParallelToZ / Ecal_Sc_number_of_virtual_cells;

  EC_StripSizeinX = EC_alveolus_width / (2 * EC_N_cells_in_Z);
  EC_StripSizeParallelToZ = EC_alveolus_width / Ecal_Sc_N_strips_across_module;
  EC_virtualCellDim = EC_StripSizeParallelToZ / Ecal_Sc_number_of_virtual_cells;


    if (Number_of_Sc_Layers_in_Barrel != 0)
    {
#ifdef VERBOSE
      G4cout<<"\n scintillator layers in barrel: "<<G4endl;
      G4cout<<" Number_of_Sc_Layers_in_Barrel:   "<<Number_of_Sc_Layers_in_Barrel<<G4endl;
      G4cout<<" barrelStripSizeinX:              "<<barrelStripSizeinX<<G4endl;
      G4cout<<" barrelStripSizeParallelToZ:      "<<barrelStripSizeParallelToZ<<G4endl;
      G4cout<<" Ecal_Sc_thickness:               "<<Ecal_Sc_thickness<<G4endl;
      G4cout<<" Ecal_Sc_N_strips_across_module:  "<<Ecal_Sc_N_strips_across_module<<G4endl;
      G4cout<<" Ecal_Sc_number_of_virtual_cells: "<<Ecal_Sc_number_of_virtual_cells<<G4endl;
      G4cout<<" Ecal_Sc_reflector_thickness:     "<<Ecal_Sc_reflector_thickness<<G4endl;
      G4cout<<" 2 * N_cells_in_Z:                "<<2 * N_cells_in_Z<<G4endl;
      G4cout<<" Ecal_Barrel_Sc_Si_Mix:           "<<Ecal_Barrel_Sc_Si_Mix<<G4endl;
      G4cout<<" HWallSize:                       "<<HWallSize<<G4endl;
      G4cout<<" TowerWallSize:                   "<<TowerWallSize<<G4endl;
      G4cout<<" ECALBARREL:                      "<<ECALBARREL<<G4endl;
      G4cout<<" barID1Flag:                      "<<barID1Flag<<G4endl;
      G4cout<<"\n alveolus_dim_z:        "<<alveolus_dim_z<<G4endl;
      G4cout<<" EC_alveolus_width:       "<<EC_alveolus_width<<G4endl;
      G4cout<<" EC_StripSizeinX:         "<<EC_StripSizeinX<<G4endl;
      G4cout<<" EC_StripSizeParallelToZ: "<<EC_StripSizeParallelToZ<<G4endl;
#endif

      theBarrelScintillatorSD = new SEcalSD04(barrelStripSizeinX,
					      barrelStripSizeParallelToZ,
					      Ecal_Sc_thickness,
					      Ecal_Sc_N_strips_across_module,
					      Ecal_Sc_number_of_virtual_cells,
					      Ecal_Sc_reflector_thickness,
					      2 * N_cells_in_Z,
					      Ecal_Barrel_Sc_Si_Mix,
					      HWallSize,
					      TowerWallSize,
					      ECALBARREL,
					      "EcalBarrelScintillator",
					      barID1Flag);

        if (useScintillatorTilesOfVaryingSize)
        {
            theBarrelScintillatorSD->FillScintillatorCellSizesVectors(barrelEcal_Sc_cellDim1_vector, barrelEcal_Sc_cellDim2_vector,
                barrelEcal_Sc_N_strips_across_module_vector, barrelEcal_Sc_N_strip_containers_along_z_vector);
        }

        RegisterSensitiveDetector(theBarrelScintillatorSD);
    }

    if (Number_of_Sc_Layers_in_EC != 0)
    {
      theEndCapScintillatorSD = new SEcalSD04(EC_StripSizeinX,
					      EC_StripSizeParallelToZ,
					      Ecal_Sc_thickness,
					      Ecal_Sc_N_strips_across_module,
					      Ecal_Sc_number_of_virtual_cells,
					      Ecal_Sc_reflector_thickness,
					      2 * EC_N_cells_in_Z,
					      Ecal_EndCap_Sc_Si_Mix,
					      HWallSize,
					      TowerWallSize,
					      ECALENDCAPMINUS,
					      "EcalEndcapScintillator",
					      ecID1Flag);
        if (useScintillatorTilesOfVaryingSize)
        {
            theEndCapScintillatorSD->FillScintillatorCellSizesVectors(endcapEcal_Sc_cellDim1_vector, endcapEcal_Sc_cellDim2_vector,
                endcapEcal_Sc_N_strips_across_module_vector, endcapEcal_Sc_N_strip_containers_along_z_vector);
        }

        RegisterSensitiveDetector(theEndCapScintillatorSD);
    }

    if (Number_of_Si_Layers_in_EC != 0)
    {
      /*Sensitive detector for the +z Ecal endcap*/
      theEndCapSiliconSD = new SEcalSD02 (EC_cell_dim_x,
					  EC_cell_dim_z,
					  Ecal_Si_thickness,
					  EC_N_cells_in_X,
					  EC_N_cells_in_Z,
					  Ecal_guard_ring_size,
					  HWallSize,
					  TowerWallSize,
					  ECALENDCAPMINUS,
					  "EcalEndcapSilicon",
					  ecID1Flag);
      RegisterSensitiveDetector(theEndCapSiliconSD);
    }
  
  /* Sensitive detector for end cap rings: we use the same
     but with fake parameters, to reuse the code. So we use
     it as just a huge wafer.

     For that, we adjust the cell size in the ring to have a 
     interger number os cells in the given Si plates*/
  
  ECRingSiplateSize = Ecal_endcap_center_box_size 
    - 2 * Ecal_EC_Ring_gap
    - 2 * Ecal_lateral_face_thickness;

  G4double cell_size_ring = ECRingSiplateSize / floor(ECRingSiplateSize/cell_dim_x);
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
  
  theEndCapRingSD = new SEcalSDRing02 (cell_size_ring,
				       cell_size_ring,
				       Ecal_Si_thickness,
				       ECALENDCAPMINUS,
				       "EcalEndcapRing",
				       ecID1Flag);
  RegisterSensitiveDetector(theEndCapRingSD);
  
  /* End cap ring doesn't rotate and has just one stave*/
  theEndCapRingSD->SetStaveRotationMatrix(1,0.);

  /* initialize the Si in Rings*/
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::Blue());
  VisAtt->SetForceSolid(false);
  VisAtt->SetVisibility(true);
  
  G4Tubs *CenterECTubForSi = new G4Tubs ("CenterECTubForSi",
					 0.,
					 Lcal_outer_radius + Ecal_Lcal_ring_gap
					 +G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
					 module_thickness,
					 0.,
					 2 * pi);
  
  ECRingSiBox = new G4Box ("ECRingSiSolid", 
			   ECRingSiplateSize/ 2. - G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
			   ECRingSiplateSize/ 2. - G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
			   Ecal_Si_thickness/2. - G4GeometryTolerance::GetInstance()->GetSurfaceTolerance());
  
  G4SubtractionSolid *ECRingSiSolid = new G4SubtractionSolid("ECRingSiSolid", 
							     ECRingSiBox, 
							     CenterECTubForSi,
							     *FollowLcal);
  
  G4UserLimits *pULimits = new G4UserLimits(theMaxStepAllowed);

  ECRingSiLog = new G4LogicalVolume(ECRingSiSolid,
				    CGAGeometryManager::
				    GetMaterial("silicon_2.33gccm"),
				    "ECRingSiLog", 
				    0, 0, pULimits);  
  ECRingSiLog->SetSensitiveDetector(theEndCapRingSD);
  ECRingSiLog->SetVisAttributes(VisAtt);

  /* Z minus -> hole not symetric*/
  ECRingSiSolid = new G4SubtractionSolid("ECRingSiSolidZminus", 
					 ECRingSiBox, 
					 CenterECTubForSi,
					 *FollowLcalZminus);
  
  ECRingSiLogZminus = new G4LogicalVolume(ECRingSiSolid,
					  CGAGeometryManager::
					  GetMaterial("silicon_2.33gccm"),
					  "ECRingSiLog", 
					  0, 0, pULimits);  
  ECRingSiLogZminus->SetSensitiveDetector(theEndCapRingSD);
  ECRingSiLogZminus->SetVisAttributes(VisAtt);
  
  MyPlacement::InsertComment("Building Ecal"); 

  /*---------------------------------------------------- 
    Barrel Standard Module in the air 
    ----------------------------------------------------*/
  MyPlacement::InsertComment("Building Ecal barrel");
  BarrelStandardModule(WorldLog);

  /*----------------------------------------------------
    EndCaps in the air
 ----------------------------------------------------*/
  MyPlacement::InsertComment("Building Ecal endcaps");  

  if(Number_of_Si_Layers_in_EC != 0) EC_Initialize(EC_SiliconTowerSlabs, '0');
  
  if (!useScintillatorTilesOfVaryingSize)
    {
      if(EC_Scint_Along_X) EC_Initialize(EC_ScintillatorParallelToXTowerSlabs, 'x');
      if(EC_Scint_Along_Z) EC_Initialize(EC_ScintillatorParallelToZTowerSlabs, 'z');
    }

  G4LogicalVolume *EndCapLogical = EndcapStandardModule();
  new MyPlacement(0,
		  G4ThreeVector(0., 0., EC_module_z_offset),
		  EndCapLogical,
		  "EndCapPhys+Z",
		  WorldLog,
		  false,
		  ECALENDCAPPLUS);
  theEndCapRingSD->SetModuleZOffset(6,EC_module_z_offset);

  FollowLcal = FollowLcalZminus;

  EndCapLogical = EndcapStandardModule(true);

  /* rotate the endcap module to place it on the -Z side*/
  G4RotationMatrix *rot =  new G4RotationMatrix();
  rot->rotateY(pi);
  new MyPlacement(rot,
		  G4ThreeVector(0., 0., -EC_module_z_offset),
		  EndCapLogical,
		  "EndCapPhys-Z",
		  WorldLog,
		  false,
		  ECALENDCAPMINUS);
  
  /* module = 6 to flag as endcap module in SD*/
 if(Number_of_Si_Layers_in_EC != 0) theEndCapSiliconSD->SetModuleZOffset(6, EC_module_z_offset);  
 if(Number_of_Sc_Layers_in_EC != 0) theEndCapScintillatorSD->SetModuleZOffset(6,EC_module_z_offset);

 theEndCapRingSD->SetModuleZOffset(6, EC_module_z_offset);  
  

  
#ifdef MOKKA_GEAR
  G4double cell_dim_1 = 0, cell_dim_2 = 0;
  char layerCode;

  /* get the information that are not yet included*/
  helpBarrel.phi0 = 0;
  helpBarrel.zMax = std::max( helpBarrel.mostZ , -helpBarrel.leastZ ) ;
  
  /* ECAL Barrel*/
  bool isBarrel=true;
  gear::CalorimeterParametersImpl *barrelParam = new gear::CalorimeterParametersImpl( helpBarrel.innerRadius, 
										      helpBarrel.zMax, 
										      8, 
										      helpBarrel.phi0 );
  
  /* calculate each layer thichness and push its paramaters*/
  G4double calcThick = 0;
  for (int i = 1; i < helpBarrel.count; i++) 
    {
      calcThick = helpBarrel.layerPos[i] - helpBarrel.layerPos[i-1] ;    
      layerCode = Ecal_Barrel_Sc_Si_Mix[(unsigned int)((i)/2)];      
      getCellDims(cell_dim_1, cell_dim_2, i, layerCode, isBarrel);
      barrelParam->layerLayout().positionLayer(0, calcThick, cell_dim_1, cell_dim_2, helpBarrel.radiThickness[i-1]);

#ifdef VERBOSE
      G4cout<<"\n GEAR barrel:"<<G4endl;
      G4cout<<" i="<<i<<" calcThick="<<calcThick<<" cell_dim_1="<<cell_dim_1<<" cell_dim_2="<<cell_dim_2
	    <<" radiThickness="<<helpBarrel.radiThickness[i-1]<<G4endl;
#endif
    }
  
  /* well, it's a bit trick, but we have to add the last layer also...
     just repeat the last calcThick, there is no reason to be not the same!*/
  layerCode = Ecal_Barrel_Sc_Si_Mix[(unsigned int)((helpBarrel.count)/2)];      
  getCellDims(cell_dim_1, cell_dim_2, helpBarrel.count, layerCode, isBarrel);
  barrelParam->layerLayout().positionLayer(0, calcThick, cell_dim_1, cell_dim_2, helpBarrel.radiThickness[helpBarrel.count-1]);
  
  /* The same for Ecal Endcap   helpEndcap*/
  isBarrel = false;
  gear::CalorimeterParametersImpl *endcapParam = new gear::CalorimeterParametersImpl( helpEndcap.innerRadius, 
										      helpEndcap.outerRadius, 
										      helpEndcap.leastZ,
										      2, 
										      helpBarrel.phi0 );

  for (int i = 1; i < helpEndcap.count; i++) 
    {
      calcThick = helpEndcap.layerPos[i] - helpEndcap.layerPos[i-1] ;    
      layerCode = Ecal_EndCap_Sc_Si_Mix[(unsigned int)((i)/2)];      
      getCellDims(cell_dim_1, cell_dim_2, i, layerCode, isBarrel);
      endcapParam->layerLayout().positionLayer(0, calcThick, cell_dim_1, cell_dim_2, helpEndcap.radiThickness[i-1]);

#ifdef VERBOSE
      G4cout<<"\n GEAR endcap:"<<G4endl;
      G4cout<<" i="<<i<<" calcThick="<<calcThick<<" cell_dim_1="<<cell_dim_1<<" cell_dim_2="<<cell_dim_2
	    <<" radiThickness="<<helpEndcap.radiThickness[i-1]<<G4endl;
#endif
    }

  /* the last layer...*/
  layerCode = Ecal_EndCap_Sc_Si_Mix[(unsigned int)((helpEndcap.count)/2)];      
  getCellDims(cell_dim_1, cell_dim_2, helpEndcap.count, layerCode, isBarrel);
  endcapParam->layerLayout().positionLayer(0, calcThick, cell_dim_1, cell_dim_2, helpEndcap.radiThickness[helpEndcap.count-1]);
  
  /*Ecal Plug*/
  gear::CalorimeterParametersImpl *plugParam = new gear::CalorimeterParametersImpl(helpPlug.innerRadius,
										   helpPlug.outerRadius,
										   helpPlug.leastZ,
										   2,
										   helpBarrel.phi0);
  
  for (int i = 1; i < helpPlug.count; i++)
    {
      calcThick = helpPlug.layerPos[i] - helpPlug.layerPos[i-1] ;    
      plugParam->layerLayout().positionLayer(0, calcThick, cell_dim_z, cell_dim_x, helpPlug.radiThickness[i-1]);
    }
  
  /* the last layer...*/
  plugParam->layerLayout().positionLayer(0, calcThick, cell_dim_z, cell_dim_x, helpPlug.radiThickness[helpPlug.count-1]);

  gear::GearMgr *gearMgr = MokkaGear::getMgr() ;
  gearMgr->setEcalBarrelParameters( barrelParam ) ;
  gearMgr->setEcalEndcapParameters( endcapParam ) ;
  gearMgr->setEcalPlugParameters( plugParam );
#endif

  G4cout << "Ecal done.\n" << G4endl;
  return true;  
}

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
SEcal05::~SEcal05() 
{
}  

/*********************************************************************************************/
/*                                                                                           */
/*      BarrelStandardModule                                                                 */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
void SEcal05::BarrelStandardModule(G4LogicalVolume* MotherLog)
{
#ifdef VERBOSE
  G4cout<<"\n\n\n ================== BarrelStandardModule"<<G4endl;
#endif

  /* Attention: on b??tit le module dans la verticale
     ?? cause du G4Trd et on le tourne avant de le positioner*/
  G4Trd * MyTrd = new G4Trd("Barrel_Module",
			    bottom_dim_x / 2, 
			    top_dim_x / 2,
			    Ecal_Barrel_module_dim_z / 2,
			    Ecal_Barrel_module_dim_z / 2,
			    module_thickness/2);
  
  EnvLogEcalModuleBarrel  = new G4LogicalVolume(MyTrd,
						CGAGeometryManager::GetMaterial("g10"),
						"EnvLog", 
						0, 0, 0);
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::Green());
  VisAtt->SetForceSolid(false);
  VisAtt->SetForceWireframe(true);
  VisAtt->SetVisibility(true);
  VisAtt->SetDaughtersInvisible(false);
  EnvLogEcalModuleBarrel->SetVisAttributes(VisAtt);

  /*We count the layers starting from IP and from 1,
    so odd layers should be inside slabs and
    even ones on the structure.
    The structure W layers are here big plans, as the 
    gap between each W plate is too small to create problems 
    The even W layers are part of H structure placed inside
    the alveolus.*/

  G4double y_floor = Ecal_front_face_thickness + N_FIBERS_ALVEOLUS * Ecal_fiber_thickness;

  /* ATTENTION, TWO LAYERS PER LOOP*/
  for(G4int layer_id = 1; layer_id < total_number_of_layers+1; layer_id+=2)
    {
      /* build and place the several Alveolus with 
	 the slabs and the radiator layer inside.*/
      G4double alveolus_dim_y = BuildBarrelAlveolus(layer_id, y_floor, EnvLogEcalModuleBarrel);

#ifdef VERBOSE
      G4cout<<" BarrelStandardModule: loop over layer pair "<<layer_id<<G4endl;
      G4cout << "y_floor = " << y_floor
	     << ", layer_id = " << layer_id
	     << ", alveolus_dim_y = " << alveolus_dim_y
	     << G4endl;
#endif
      /* update the y_floor*/
      y_floor += alveolus_dim_y + (N_FIBERS_ALVEOLUS + N_FIBERS_W_STRUCTURE) * Ecal_fiber_thickness;
      
     
      G4int even_layer = layer_id + 1;
      if(even_layer > Ecal_nlayers1 + Ecal_nlayers2 + Ecal_nlayers3)
	continue;

      /* Build and place the structure radiator layer into the module*/
      G4double W_thick = BuildBarrelStructureLayer(even_layer, y_floor, EnvLogEcalModuleBarrel);
#ifdef VERBOSE
      G4cout << "y_floor= " << y_floor
	     << ", even_layer=" << even_layer
	     << ", W_thick=" << W_thick
	     << G4endl;
#endif

      /*update the y_floor*/
      y_floor += W_thick + (N_FIBERS_ALVEOLUS + N_FIBERS_W_STRUCTURE) * Ecal_fiber_thickness;
    }


  /* BarrelStandardModule placements*/  
  G4double phirot = 0, module_z_offset = 0;
  G4double X = module_thickness * sin(pi/4.);
  G4double Y = Ecal_inner_radius + module_thickness / 2.;

  if(Number_of_Si_Layers_in_Barrel != 0) theBarrelSiliconSD->SetStandardXOffset(X);
  if(Number_of_Sc_Layers_in_Barrel != 0) theBarrelScintillatorSD->SetStandardXOffset(X);

#ifdef MOKKA_GEAR
  /* set first radius*/
  helpBarrel.innerRadius = X*X + Y*Y ;
  
  /* set last layer position*/
  helpBarrel.layerPos.push_back( MyTrd->GetZHalfLength() ) ;
#endif

  for (G4int stave_id = 1; stave_id < 9 ; stave_id++)
    {
      for (G4int module_id = 1; module_id < 6; module_id++)
	{
	  phirot = (stave_id-1) * pi/4;
	  module_z_offset =  (2 * module_id-6)* Ecal_Barrel_module_dim_z/2.;

	  G4RotationMatrix *rot = new G4RotationMatrix();
	  rot->rotateX(pi*0.5); /* on couche le module.*/
	  rot->rotateY(phirot); /* on tourne selon le stave*/
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
	  
	  if(Number_of_Si_Layers_in_Barrel != 0)
	    {
	      theBarrelSiliconSD->SetStaveRotationMatrix(stave_id, phirot);
	      theBarrelSiliconSD->SetModuleZOffset(module_id, module_z_offset);
	    }

	  if(Number_of_Sc_Layers_in_Barrel != 0)
	     {
		theBarrelScintillatorSD->SetStaveRotationMatrix(stave_id, phirot);
		theBarrelScintillatorSD->SetModuleZOffset(module_id, module_z_offset);
	     }
	}
    }

#ifdef MOKKA_GEAR
  /* find out most and least extensions in z
     take offset and add/subtract dimension of trapezoid
     attention z<->y*/
  G4double Z = module_z_offset;
  helpBarrel.leastZ = std::min( helpBarrel.leastZ, Z - MyTrd->GetYHalfLength1() );
  helpBarrel.mostZ  = std::max( helpBarrel.mostZ , Z + MyTrd->GetYHalfLength1() );
  
  /* get innerRadius as minimun of all occurent inner radius
     helf heigth of module*/
  G4double moduleHeigth = MyTrd->GetZHalfLength() ;
  G4double radius = std::sqrt( X*X + Y*Y );
  helpBarrel.innerRadius = std::min( helpBarrel.innerRadius, radius - moduleHeigth );
#endif

}


/*********************************************************************************************/
/*                                                                                           */
/*     EndcapStandardModule                                                                  */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
G4LogicalVolume* SEcal05::EndcapStandardModule(G4bool Zminus)
{
  /* While waiting for more geometric details,
     build a simple Endcap using a fiber polyhedra
     and substract the center box*/
  
  MyPlacement::InsertComment("Ecal endcaps");

  G4double zPlane[2];
  G4double rInner[2],rOuter[2];
  zPlane[0] = -module_thickness/2;
  zPlane[1] = -zPlane[0];
  
  rInner[0] = rInner[1] = 0.;
  rOuter[0] = rOuter[1] = Ecal_endcap_rmax;
  
  G4Polyhedra *ECPolyHedra = new G4Polyhedra("EcalEndCapSolid",
					     pi/8.,
					     2 * pi,
					     8,
					     2,
					     zPlane,
					     rInner,
					     rOuter);
  
  G4SubtractionSolid *EndCapSolid = new G4SubtractionSolid("EndCapSolid", 
							   ECPolyHedra, 
							   CenterECTub,
							   *FollowLcal);
  
   
  G4LogicalVolume *EndCapLogical = new G4LogicalVolume(EndCapSolid,
						       CGAGeometryManager::GetMaterial("g10"),
						       "EndCapLog",
						       0, 0, 0);
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::Green());
  VisAtt->SetForceWireframe(true);
  VisAtt->SetForceSolid(false);
  VisAtt->SetDaughtersInvisible(false);
  EndCapLogical->SetVisAttributes(VisAtt);
  
#ifdef MOKKA_GEAR
  /* retrieve ending value for layers position*/
  G4double lastLayerPos = module_thickness/2  ;
#endif

  /*----------------------------------------------------
    Radiator plates in the EndCap structure also as
    polyhedra, and radiator plates in the slab of EndCap 
    Rings as box less Tub
    -------------------------------------------------------*/  
  MyPlacement::InsertComment("Ecal endcaps W plates");
  G4LogicalVolume *EndCapRadiatorL1 = NULL;  
  G4LogicalVolume *EndCapRadiatorL2 = NULL;  
  G4LogicalVolume *EndCapRadiatorL3 = NULL;
  EndCapRingSlabRadiatorL1 = NULL;
  EndCapRingSlabRadiatorL2 = NULL;
  EndCapRingSlabRadiatorL3 = NULL;

  /* Build the standard radiator plates to be placed inside the module structure.*/
  EndcapRadiatorPlates(EndCapRadiatorL1,
		       EndCapRadiatorL2,
		       EndCapRadiatorL3,
		       EndCapRingSlabRadiatorL1,
		       EndCapRingSlabRadiatorL2,
		       EndCapRingSlabRadiatorL3);


  /*-------------------------------------------------------
    Radiator and towers placements inside the Endcap module
    -------------------------------------------------------*/
  /* We count the layers starting from IP and from 1,
     so odd layers should be inside slabs and
     even ones on the structure.*/

  G4double z_floor = - module_thickness/2 +  Ecal_front_face_thickness + N_FIBERS_ALVEOLUS * Ecal_fiber_thickness;

  
  /* ATTENTION, TWO LAYERS PER LOOP AS THERE IS ONE INSIDE THE ALVEOLUS */

  G4LogicalVolume *EndCapRadiator = NULL;
  G4double RadiatorThickness = 0.;
  G4double AlveolusThickness = 0;
  
  G4int layer_id;
  for(layer_id = 1; layer_id <= total_number_of_layers; layer_id+=2)
    {
      /* place the tower layer for the four modules*/
      AlveolusThickness = BuildECAlveolus (layer_id, z_floor, EndCapLogical, Zminus);
      
      /* place Si/Rad/Si in ring */
      BuildECRingAlveolus (layer_id, z_floor, EndCapLogical, Zminus);

      /* update the z_floor*/
      z_floor += AlveolusThickness + (N_FIBERS_ALVEOLUS + N_FIBERS_W_STRUCTURE) * Ecal_fiber_thickness;

      /* place a radiator layer if the number of layers is not complete*/
      if( layer_id == total_number_of_layers) break;

      if(layer_id < Ecal_nlayers1) 
	{
	  EndCapRadiator    = EndCapRadiatorL1;
	  RadiatorThickness = Ecal_radiator_thickness1;
	}

      if(layer_id > Ecal_nlayers1 && layer_id < Ecal_nlayers1 + Ecal_nlayers2)
	{
	  EndCapRadiator    = EndCapRadiatorL2;
	  RadiatorThickness = Ecal_radiator_thickness2;
	}

      if(layer_id > Ecal_nlayers1 + Ecal_nlayers2)
	{
	  EndCapRadiator    = EndCapRadiatorL3;
	  RadiatorThickness = Ecal_radiator_thickness3;
	}
      
      new MyPlacement(0,
		      G4ThreeVector(0., 0., z_floor + RadiatorThickness / 2.),
		      EndCapRadiator,
		      "EndCapRadiatorPhys",
		      EndCapLogical,
		      false,
		      0);
#ifdef MOKKA_GEAR
      if(!Zminus)
	{
	  /* get positions of Layer as the middle of the radiator layer */
	  helpEndcap.layerPos.push_back(z_floor + RadiatorThickness / 2.) ;
	  helpPlug.layerPos.push_back(z_floor + RadiatorThickness / 2);
	  
	  /* get radiator thickness*/
	  helpEndcap.radiThickness.push_back(RadiatorThickness) ;
	  helpPlug.radiThickness.push_back(RadiatorThickness);
	  helpEndcap.count ++ ;
	  helpPlug.count ++;
	}
#endif

      /* update the z_floor*/
      z_floor += RadiatorThickness + (N_FIBERS_ALVEOLUS + N_FIBERS_W_STRUCTURE) * Ecal_fiber_thickness;
      
    }
  
#ifdef MOKKA_GEAR
      if(!Zminus)
	{
	  /* set ending value for layer's position*/
	  helpEndcap.layerPos.push_back( lastLayerPos ) ;

	  /* getting the outer radius as maximal reached radius*/
	  helpEndcap.outerRadius = rOuter[0];
	  
	  /* getting inner radius as minimal distance*/
	  helpEndcap.innerRadius = Ecal_endcap_center_box_size / 2.;
	  helpPlug.outerRadius = helpEndcap.innerRadius;
	  helpPlug.innerRadius = Lcal_outer_radius + Ecal_Lcal_ring_gap;
	  
	  /* get least z-distance as maximum*/
	  helpEndcap.zMax   = EC_module_z_offset + module_thickness / 2.;
	  helpPlug.zMax     = helpEndcap.zMax;
	  helpEndcap.leastZ = EC_module_z_offset - module_thickness / 2.;
	  helpPlug.leastZ   = helpEndcap.leastZ; 
	  
	  /* set phi0 to zero*/
	  helpEndcap.phi0 = 0. ;
	  helpPlug.phi0 = 0;
	}
#endif

  return EndCapLogical;
}

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
void SEcal05::EndcapRadiatorPlates(G4LogicalVolume *&EndCapRadiatorL1,
				   G4LogicalVolume *&EndCapRadiatorL2,
				   G4LogicalVolume *&EndCapRadiatorL3,
				   G4LogicalVolume *&EndCapRingSlabRadiatorL1,
				   G4LogicalVolume *&EndCapRingSlabRadiatorL2,
				   G4LogicalVolume *&EndCapRingSlabRadiatorL3)
{
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::Red());
  //VisAtt->SetForceWireframe(false);
  VisAtt->SetVisibility(true);
  VisAtt->SetForceSolid(false);
  
  /* W Logicals*/
  G4double zPlane[2];
  G4double rInner[2],rOuter[2];
  
  rInner[0] = rInner[1] =  + Ecal_lateral_face_thickness;
  rOuter[0] = rOuter[1] = Ecal_endcap_rmax  - Ecal_lateral_face_thickness;
  
  G4Tubs *CenterECTubForRadiator = new G4Tubs ("CenterECTubForRadiator",
					       0.,
					       Lcal_outer_radius + Ecal_Lcal_ring_gap
					       + G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
					       module_thickness,
					       0.,
					       2 * pi);
  

  if(Ecal_nlayers1 > 0 )
    {
      zPlane[0] = -Ecal_radiator_thickness1 / 2.;
      zPlane[1] = -zPlane[0];

      G4Polyhedra *ECPolyHedraRadL1    = new G4Polyhedra("EcalECRadL1", pi/8., 2 * pi, 8, 2, zPlane, rInner, rOuter);
      G4SubtractionSolid *EndCapRadL1 = new G4SubtractionSolid("EcalECRadL1", ECPolyHedraRadL1, CenterECTubForRadiator, *FollowLcal);

      EndCapRadiatorL1 = new G4LogicalVolume(EndCapRadL1, RadiatorMaterial, "EndCapLog", 0, 0, 0);
      EndCapRadiatorL1->SetVisAttributes(VisAtt);

      /* plate for slab in ring*/
      G4Box *ECRingRadBox1 = new G4Box ("ECRingRadBox1", 
					(Ecal_endcap_center_box_size - Ecal_lateral_face_thickness)/ 2.,
					(Ecal_endcap_center_box_size - Ecal_lateral_face_thickness)/ 2.,
					Ecal_radiator_thickness1/2);
      
      G4SubtractionSolid *ECRingRadSolid1 = new G4SubtractionSolid("ECRingRadSolid1", 
								   ECRingRadBox1, 
								   CenterECTubForRadiator,
								   *FollowLcal);
      
      EndCapRingSlabRadiatorL1 = new G4LogicalVolume(ECRingRadSolid1,
						     RadiatorMaterial,
						     "EndCapRingSlabRadiatorL1", 
						     0, 0, 0);  
      EndCapRingSlabRadiatorL1->SetVisAttributes(VisAtt);
    }

  if(Ecal_nlayers2 > 0 )
    {
      zPlane[0] = -Ecal_radiator_thickness2 / 2.;
      zPlane[1] = -zPlane[0];
      G4Polyhedra *ECPolyHedraRadL2   = new G4Polyhedra("EcalECRadL2", pi/8., 2 * pi, 8, 2, zPlane, rInner, rOuter);
      G4SubtractionSolid *EndCapRadL2 = new G4SubtractionSolid("EcalECRadL2", ECPolyHedraRadL2, CenterECTubForRadiator, *FollowLcal);
      
      EndCapRadiatorL2 = new G4LogicalVolume(EndCapRadL2, RadiatorMaterial, "EndCapLog", 0, 0, 0);
      EndCapRadiatorL2->SetVisAttributes(VisAtt);

      /* plate for slab in ring*/
      G4Box *ECRingRadBox2 = new G4Box ("ECRingRadBox2", 
					(Ecal_endcap_center_box_size - Ecal_lateral_face_thickness)/ 2.,
					(Ecal_endcap_center_box_size - Ecal_lateral_face_thickness)/ 2.,
					Ecal_radiator_thickness2/2);
      
      G4SubtractionSolid *ECRingRadSolid2 = new G4SubtractionSolid("ECRingRadSolid2", 
								   ECRingRadBox2, 
								   CenterECTubForRadiator,
								   *FollowLcal);

      EndCapRingSlabRadiatorL2 = new G4LogicalVolume(ECRingRadSolid2,
						     RadiatorMaterial,
						     "EndCapRingSlabRadiatorL2", 
						     0, 0, 0);  
      EndCapRingSlabRadiatorL2->SetVisAttributes(VisAtt);
      

    }
  
  if(Ecal_nlayers3 > 0 )
    {
      zPlane[0] = -Ecal_radiator_thickness3 / 2.;
      zPlane[1] = -zPlane[0];

      G4Polyhedra *ECPolyHedraRadL3   = new G4Polyhedra("EcalECRadL3", pi/8., 2 * pi, 8, 2, zPlane, rInner, rOuter);
      G4SubtractionSolid *EndCapRadL3 = new G4SubtractionSolid("EcalECRadL3", ECPolyHedraRadL3, CenterECTubForRadiator, *FollowLcal);
      
      EndCapRadiatorL3 = new G4LogicalVolume(EndCapRadL3, RadiatorMaterial, "EndCapLog", 0, 0, 0);
      EndCapRadiatorL3->SetVisAttributes(VisAtt);

      /* plate for slab in ring*/
      G4Box *ECRingRadBox3 = new G4Box ("ECRingRadBox3", 
					(Ecal_endcap_center_box_size - Ecal_lateral_face_thickness)/ 2.,
					(Ecal_endcap_center_box_size - Ecal_lateral_face_thickness)/ 2.,
					Ecal_radiator_thickness3/2);
      
      G4SubtractionSolid *ECRingRadSolid3 = new G4SubtractionSolid("ECRingRadSolid3", ECRingRadBox3, CenterECTubForRadiator, *FollowLcal);

      EndCapRingSlabRadiatorL3 = new G4LogicalVolume(ECRingRadSolid3, RadiatorMaterial, "EndCapRingSlabRadiatorL3", 0, 0, 0);  
      EndCapRingSlabRadiatorL3->SetVisAttributes(VisAtt);
    }

}

/*********************************************************************************************/
/*                                                                                           */
/* In this complex subdetector the best is to read all hits in the one of the sensitive      */
/* detectors, just for visualisation, so just for your eyes...                                */
/*                                                                                           */
/*********************************************************************************************/
void SEcal05::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
  theBarrelSiliconSD->LoadEvent(theSubDetectorEventHitsFileInput);
}

/*********************************************************************************************/
/*                                                                                           */
/* BuildBarrelAlveolus build the Slabs, the radiator plate and place it inside the           */
/* standard Module, in several towers                                                        */
/*                                                                                           */
/*********************************************************************************************/
G4double SEcal05::BuildBarrelAlveolus(G4int layer_id, G4double module_y_floor, G4LogicalVolume* EnvLogEcalModuleBarrel)
{
  G4double alveolus_dim_y = 0;
  G4double W_thick = GiveMeRadiatorThickness(layer_id);

  G4double Slab_thickness        = Ecal_total_SiSlab_thickness;
  G4double first_slab_thickness  = Ecal_total_SiSlab_thickness;
  G4double second_slab_thickness = Ecal_total_SiSlab_thickness;

  if(Ecal_Barrel_Sc_Si_Mix[(unsigned int)((layer_id-1)/2)] != ECAL_SI_LAYERS) 
    {
      /* Daniel Jeans*/
    if(Ecal_Barrel_Sc_Si_Mix[(unsigned int)((layer_id-1)/2)] == ECAL_MIX_LAYER_1_SI_LAYER_2_ALONG_X 
       || Ecal_Barrel_Sc_Si_Mix[(unsigned int)((layer_id-1)/2)] == ECAL_MIX_LAYER_1_SI_LAYER_2_ALONG_Z ) 
      {
	first_slab_thickness  = Ecal_total_SiSlab_thickness;
	second_slab_thickness = Ecal_total_ScSlab_thickness;
      } 
    else if(Ecal_Barrel_Sc_Si_Mix[(unsigned int)((layer_id-1)/2)] == ECAL_MIX_LAYER_1_ALONG_X_LAYER_2_SI 
	    || Ecal_Barrel_Sc_Si_Mix[(unsigned int)((layer_id-1)/2)] == ECAL_MIX_LAYER_1_ALONG_Z_LAYER_2_SI ) 
      {
	first_slab_thickness  = Ecal_total_ScSlab_thickness;
	second_slab_thickness = Ecal_total_SiSlab_thickness;
      } 
    else 
      {  
	first_slab_thickness  = Ecal_total_ScSlab_thickness;
	second_slab_thickness = Ecal_total_ScSlab_thickness;
      }
    }

  Slab_thickness = (first_slab_thickness + second_slab_thickness)/2.;

  alveolus_dim_y = first_slab_thickness + second_slab_thickness + W_thick + 2 * Ecal_fiber_thickness;
  
  G4double alveolus_dim_x = bottom_dim_x - 2 * (module_y_floor+alveolus_dim_y);
  
#ifdef VERBOSE
  G4cout << "alveolus_dim_x = " << alveolus_dim_x << G4endl;
#endif

  /* To simplify we place each slab and the radiator
     layer directly into the fiber module.*/
  
  /* Build a slab:*/
  
  G4LogicalVolume *FirstSlabLog = 0;
  G4LogicalVolume *SecondSlabLog = 0;

  char layerCode = Ecal_Barrel_Sc_Si_Mix[(unsigned int)((layer_id-1)/2)];

  char FirstSlabDir='0', SecondSlabDir='0';

  /* code cleaned up by Daniel Jeans,
     to deal with extra configs more elegantly*/

  switch (layerCode) 
    {
    case ECAL_SI_LAYERS:
      FirstSlabDir  = '0';
      SecondSlabDir = '0';
      break;

    case ECAL_SC_LAYER_1_2_ALONG_X:
      FirstSlabDir  = 'x';
      SecondSlabDir = 'x';
      break;
      
    case ECAL_SC_LAYER_1_2_ALONG_Z:
      FirstSlabDir  = 'z';
      SecondSlabDir = 'z';
      break;

    case ECAL_SC_LAYER_1_ALONG_X_LAYER_2_ALONG_Z:
      FirstSlabDir  = 'x';
      SecondSlabDir = 'z';
      break;
      
    case ECAL_SC_LAYER_1_ALONG_Z_LAYER_2_ALONG_X:
      FirstSlabDir  = 'z';
      SecondSlabDir = 'x';
      break;

    case ECAL_MIX_LAYER_1_SI_LAYER_2_ALONG_X:
      FirstSlabDir  = '0';
      SecondSlabDir = 'x';
      break;

    case ECAL_MIX_LAYER_1_SI_LAYER_2_ALONG_Z:
      FirstSlabDir  = '0';
      SecondSlabDir = 'z';
      break;
      
    case ECAL_MIX_LAYER_1_ALONG_X_LAYER_2_SI:
      FirstSlabDir  = 'x';
      SecondSlabDir = '0';
      break;

    case ECAL_MIX_LAYER_1_ALONG_Z_LAYER_2_SI:
      FirstSlabDir  = 'z';
      SecondSlabDir = '0';
      break;
      
    default:
      G4cout << layerCode << G4endl;
      Control::Abort("SEcal05 c: The Ecal_Barrel_Sc_Si_Mix parameter should contain only 0,1,2,3,4,5,6,7,8",
		     MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
      break;
    }
  
  switch (FirstSlabDir) 
    {
    case '0':
      FirstSlabLog = BuildSiliconSlab(alveolus_dim_x,
				      Ecal_total_SiSlab_thickness,
				      alveolus_dim_z,
				      theBarrelSiliconSD);
      break;
    case 'x':
    case 'z':
    if (useScintillatorTilesOfVaryingSize)
    {
        barrelStripSizeinX = barrelEcal_Sc_cellDim1_vector[layer_id - 1];
        barrelStripSizeParallelToZ = barrelEcal_Sc_cellDim2_vector[layer_id - 1];
        Ecal_Sc_N_strips_across_module = barrelEcal_Sc_N_strips_across_module_vector[layer_id - 1];
        N_cells_in_Z = barrelEcal_Sc_N_strip_containers_along_z_vector[layer_id - 1] / 2;
        virtualCellDim = barrelStripSizeParallelToZ / Ecal_Sc_number_of_virtual_cells;
    }
#ifdef VERBOSE
      G4cout<<"\n before BuildScintillatorSlab in z: barrelStripSizeinX="<<barrelStripSizeinX
            <<" barrelStripSizeParallelToZ="<<barrelStripSizeParallelToZ<<G4endl;
#endif

      FirstSlabLog = BuildScintillatorSlab(alveolus_dim_x,
					   Ecal_total_ScSlab_thickness,
					   alveolus_dim_z,
					   FirstSlabDir,
					   theBarrelScintillatorSD,
					   barrelStripSizeinX,
					   barrelStripSizeParallelToZ);
      break;
    default:
      Control::Abort("SEcal05: cannot determine first layer orientation...",
		     MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
      break;
    }
  
  switch (SecondSlabDir) 
    {
    case '0':
      SecondSlabLog = BuildSiliconSlab(alveolus_dim_x,
				       Ecal_total_SiSlab_thickness,
				       alveolus_dim_z,
				       theBarrelSiliconSD);
      break;
    case 'x':
    case 'z':
    if (useScintillatorTilesOfVaryingSize)
    {
        barrelStripSizeinX = barrelEcal_Sc_cellDim1_vector[layer_id];
        barrelStripSizeParallelToZ = barrelEcal_Sc_cellDim2_vector[layer_id];
        Ecal_Sc_N_strips_across_module = barrelEcal_Sc_N_strips_across_module_vector[layer_id];
        N_cells_in_Z = barrelEcal_Sc_N_strip_containers_along_z_vector[layer_id] / 2;
        virtualCellDim = barrelStripSizeParallelToZ / Ecal_Sc_number_of_virtual_cells;
    }
#ifdef VERBOSE
      G4cout<<"\n before BuildScintillatorSlab second in z: barrelStripSizeinX="
            <<barrelStripSizeinX<<" barrelStripSizeParallelToZ="<<barrelStripSizeParallelToZ<<G4endl;
#endif
      SecondSlabLog = BuildScintillatorSlab(alveolus_dim_x,
					    Ecal_total_ScSlab_thickness,
					    alveolus_dim_z,
					    SecondSlabDir,
					    theBarrelScintillatorSD,
					    barrelStripSizeinX,
					    barrelStripSizeParallelToZ);
      break;
    default:
      Control::Abort("SEcal05: cannot determine first layer orientation...",
		     MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
      break;
    }
  

 #ifdef VERBOSE
  G4cout<<"\n--------------------- BuildBarrelAlveolus"<<G4endl;
  G4cout<<" pair layer_id: "<<layer_id<<G4endl;
  G4cout<<" barrelStripSizeinX:         "<<barrelStripSizeinX<<G4endl;
  G4cout<<" barrelStripSizeParallelToZ: "<<barrelStripSizeParallelToZ<<G4endl;
#endif


 
  /* Place the Slab and radiator inside the H,
     here directly into the module fiber as the
     H structure is also built in fiber.*/
  G4double z_tower_center =  
    - Ecal_Barrel_module_dim_z /2
    + Ecal_lateral_face_thickness +
    Ecal_fiber_thickness * N_FIBERS_ALVEOLUS +
    Ecal_Slab_shielding + 
    Ecal_Slab_H_fiber_thickness +
    alveolus_dim_z /2;  

  for (G4int i_tower = Ecal_barrel_number_of_towers; i_tower > 0; i_tower--)
    {
      G4double y_floor = module_y_floor;
      G4double x_off = 0; /* to be calculed*/

      G4double y_off = -module_thickness/2 + y_floor + first_slab_thickness/2.;

      /* First Slab*/
      G4RotationMatrix *rot=new G4RotationMatrix();
      rot->rotateX(pi); 
      new MyPlacement(rot,
		      G4ThreeVector(x_off,
				    z_tower_center, /* Y<->Z !*/
				    y_off),
		      FirstSlabLog,
		      "FirstSlab",
		      EnvLogEcalModuleBarrel,
		      false,
		      i_tower * 1000 + layer_id);
 
      if (i_tower == Ecal_barrel_number_of_towers)
	{
          if(FirstSlabDir == '0') /*Silicon Slab*/
	    {
	      theBarrelSiliconSD->AddLayer(layer_id,
					   x_off - ((G4Box *)FirstSlabLog->GetSolid())->GetXHalfLength(),
					   Ecal_inner_radius + module_thickness/2 + y_off - Si_Slab_Y_offset,
					   z_tower_center - ((G4Box *)FirstSlabLog->GetSolid())->GetYHalfLength());
	    }
          else /* Scintillator Slab : pass the FirstSlabDir to the SD!*/
	    {
	      theBarrelScintillatorSD->AddLayer(layer_id,
						x_off - ((G4Box *)FirstSlabLog->GetSolid())->GetXHalfLength(),
						Ecal_inner_radius + module_thickness/2 + y_off - Sc_Slab_Y_offset,
						z_tower_center - ((G4Box *)FirstSlabLog->GetSolid())->GetYHalfLength());
	    }
	}

      y_floor += first_slab_thickness + Ecal_fiber_thickness;
      
      /* Radiator layer "inside" alveolus*/
      G4LogicalVolume *WLog = BuildRadiatorPlate(alveolus_dim_x, W_thick, alveolus_dim_z);
      new MyPlacement(0,
		      G4ThreeVector(0,
				    z_tower_center, /* Y<->Z !*/
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
	  /* first layer in Slab
	     get middle of radiator layer as layer pos*/
	  helpBarrel.layerPos.push_back(y_floor + W_thick/ 2);
	  
	  /* get radiator thickness as W-Plate Thickness*/
	  helpBarrel.radiThickness.push_back(W_thick);
	  helpBarrel.count ++ ;
	}
#endif

      
      y_floor +=  W_thick + Ecal_fiber_thickness;
      
      y_off = -module_thickness/2 + y_floor + second_slab_thickness/ 2;

      /* Second Slab
	 The second slab starts from bottom to up*/     
      new MyPlacement(0,
		      G4ThreeVector(0,
				    z_tower_center, /* Y<->Z !*/
				    y_off),
		      SecondSlabLog,
		      "SecondSlab",
		      EnvLogEcalModuleBarrel,
		      false,
		      i_tower * 1000 + layer_id + 1);

      if (i_tower == Ecal_barrel_number_of_towers)
	{
	  if(SecondSlabDir == '0') /*Silicon Slab*/
	    {
	      theBarrelSiliconSD->AddLayer(layer_id + 1,
					   x_off - ((G4Box *)SecondSlabLog->GetSolid())->GetXHalfLength(),
					   Ecal_inner_radius + module_thickness/2 + y_off + Si_Slab_Y_offset, 
					   z_tower_center - ((G4Box *)SecondSlabLog->GetSolid())->GetYHalfLength());
	    }
          else /* Scintillator Slab : pass the SecondSlabDir to the SD!*/
	    {
	      theBarrelScintillatorSD->AddLayer(layer_id + 1,
						x_off - ((G4Box *)SecondSlabLog->GetSolid())->GetXHalfLength(),
						Ecal_inner_radius + module_thickness/2 + y_off + Sc_Slab_Y_offset,
						z_tower_center - ((G4Box *)SecondSlabLog->GetSolid())->GetYHalfLength());
	    }
	}
      
      z_tower_center += alveolus_dim_z + 
	2. * Ecal_fiber_thickness * N_FIBERS_ALVEOLUS +
	2. * Ecal_Slab_H_fiber_thickness +
	2. * Ecal_Slab_shielding;

    }

  return alveolus_dim_y;
}

/*********************************************************************************************/
/*                                                                                           */
/*  GiveMeRadiatorThickness returns the radiator layer thickness for a given                 */
/*  layer index                                                                              */
/*                                                                                           */
/*********************************************************************************************/
G4double SEcal05::GiveMeRadiatorThickness(G4int layer_id)
{
  G4double W_thick = Ecal_radiator_thickness1;

  if(layer_id > Ecal_nlayers1 && layer_id <= Ecal_nlayers1 + Ecal_nlayers2)
    {
      W_thick = Ecal_radiator_thickness2;
    }

  if(layer_id > Ecal_nlayers1 + Ecal_nlayers2 &&  layer_id <= Ecal_nlayers1 + Ecal_nlayers2 + Ecal_nlayers3)
    {
      W_thick = Ecal_radiator_thickness3;
    }

  return W_thick;
}

/*********************************************************************************************/
/*                                                                                           */
/* Build and places the radiator embedded into the fiber structure                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
G4double SEcal05::BuildBarrelStructureLayer(G4int layer_id, G4double y_floor, G4LogicalVolume *EnvLogEcalModuleBarrel)
{
  G4double radiator_dim_y = GiveMeRadiatorThickness(layer_id);
  G4double radiator_dim_x = bottom_dim_x - 2 * (y_floor+radiator_dim_y);

#ifdef VERBOSE
  G4cout << "radiator_dim_x = " << radiator_dim_x << G4endl;
#endif  

  G4double radiator_dim_z =
    Ecal_Barrel_module_dim_z -
    2 * Ecal_lateral_face_thickness -
    2 * N_FIBERS_W_STRUCTURE * Ecal_fiber_thickness;

  G4LogicalVolume *RadLog = BuildRadiatorPlate(radiator_dim_x,radiator_dim_y,radiator_dim_z);

  new MyPlacement(0,
		  G4ThreeVector(0,
				0,
				-module_thickness/2 + y_floor + radiator_dim_y/2),
		  RadLog,
		  "RadiatorStruct",
		  EnvLogEcalModuleBarrel,
		  false,0);
#ifdef MOKKA_GEAR
  /* get middle of radiator layer as layer pos*/
  helpBarrel.layerPos.push_back(y_floor + radiator_dim_y/2);
  
  /* get radiator thickness as W-Plate Thickness*/
  helpBarrel.radiThickness.push_back(radiator_dim_y);
  helpBarrel.count ++ ;
#endif
  
  return radiator_dim_y;
}
/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
G4LogicalVolume* SEcal05::BuildRadiatorPlate(G4double radiator_dim_x, G4double radiator_dim_y, G4double radiator_dim_z)
{
  radiator_dim_x -= 8*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance());
  radiator_dim_y -= 8*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance());
  radiator_dim_z -= 8*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance());
  
  /* Radiator solid*/
  G4Box *BoxSolid = new G4Box("RadSolid", 
			      radiator_dim_x/2,  /*hx*/
			      radiator_dim_z/2,  /*hz attention!*/
			      radiator_dim_y/2); /*hy attention!*/

  /* Radiator Logical*/
  G4LogicalVolume *RadLog = new G4LogicalVolume(BoxSolid, RadiatorMaterial, "RadLogical", 0, 0, 0);  
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::Cyan());
  VisAtt->SetForceSolid(false);
  VisAtt->SetVisibility(false);
  RadLog->SetVisAttributes(VisAtt);
  return RadLog;
}

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
G4LogicalVolume *SEcal05::BuildSiliconSlab(G4double slab_dim_x, G4double slab_dim_y, G4double slab_dim_z, SEcalSD02 * theSD)
{
  /*Try to avoid false overlaps:*/
  slab_dim_x -= 2*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance());
  slab_dim_y -= 2*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance());
  slab_dim_z -= 2*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance());

  /* Slab solid*/
  G4Box *BoxSolid = new G4Box("SlabSolid", 
			      slab_dim_x/2,  /*hx*/
			      slab_dim_z/2,  /*hz attention!*/
			      slab_dim_y/2); /*hy attention!*/

  /* Slab Logical*/
  G4LogicalVolume *SlabLog = new G4LogicalVolume(BoxSolid, CGAGeometryManager::GetMaterial("air"), "SlabLogical", 0, 0, 0);  
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::White());
  VisAtt->SetForceSolid(false);
  VisAtt->SetVisibility(true);
  VisAtt->SetDaughtersInvisible(false);
  SlabLog->SetVisAttributes(VisAtt);

  G4double y_slab_floor = - slab_dim_y /2;

  /* Ground plate*/
  G4Box *GroundSolid = new G4Box("GroundSolid", 
				 slab_dim_x/2 - G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
				 slab_dim_z/2 - G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
				 Ecal_Slab_ground_thickness/2 - G4GeometryTolerance::GetInstance()->GetSurfaceTolerance());
  G4LogicalVolume *GroundLog = new G4LogicalVolume(GroundSolid, groundMix, "GroundLogical", 0, 0, 0);
  G4VisAttributes *GroundVisAtt = new G4VisAttributes(G4Colour::Blue());
  GroundVisAtt->SetForceWireframe(true);
  GroundVisAtt->SetVisibility(true);
  GroundLog->SetVisAttributes(GroundVisAtt);

  new MyPlacement(0,
		  G4ThreeVector(0, 0, y_slab_floor + Ecal_Slab_ground_thickness/2),
		  GroundLog,
		  "Ground",
		  SlabLog,
		  false,0);
  
  y_slab_floor += Ecal_Slab_ground_thickness;

  /* Si layer
     we place a big plane of Si and inside it the
     Si wafers, to simplify the gard ring placements*/
  G4Box *CommonSiSolid = new G4Box("CommonSiSolid", 
				   slab_dim_x/2 - G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
				   slab_dim_z/2 - G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
				   Ecal_Si_thickness/2 - G4GeometryTolerance::GetInstance()->GetSurfaceTolerance());
  
  G4LogicalVolume *CommonSiLog = new G4LogicalVolume(CommonSiSolid, CGAGeometryManager::GetMaterial("silicon_2.33gccm"),"CommonSiLog",  0, 0, 0);  
  G4VisAttributes *CommonSiVisAtt = new G4VisAttributes(G4Colour::Yellow());
  CommonSiVisAtt->SetForceWireframe(true);
  CommonSiVisAtt->SetVisibility(true);
  CommonSiLog->SetVisAttributes(CommonSiVisAtt);

  Si_Slab_Y_offset = y_slab_floor + Ecal_Si_thickness/2;

  new MyPlacement(0, G4ThreeVector(0, 0, Si_Slab_Y_offset), CommonSiLog, "CommonSi", SlabLog, false, 0);
  
  /* Then we place the Si wafers inside the big Si plane*/
  
  /* User limits to limit the step to stay inside the cell*/
  G4UserLimits *pULimits = new G4UserLimits(theMaxStepAllowed);

  /* As the same method builds both barrel and end cap
     slabs, place the wafers along the biggest axe*/


  /* Barrel*/
  if (slab_dim_z < slab_dim_x) 
    {
      /* Normal squared wafers*/
      G4double wafer_dim_x = N_cells_in_X * cell_dim_x;
      G4double wafer_dim_z = N_cells_in_Z * cell_dim_z;
      G4Box *WaferSiSolid = new G4Box("WaferSiSolid", 
				      wafer_dim_x/2 - G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
				      wafer_dim_z/2 - G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
				      Ecal_Si_thickness/2 - 2*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()));
      
      G4LogicalVolume *WaferSiLog = new G4LogicalVolume(WaferSiSolid,
							CGAGeometryManager::
							GetMaterial("silicon_2.33gccm"),
							"WaferSiLog", 
							0, 0, pULimits);  
      WaferSiLog->SetSensitiveDetector(theSD);

      G4VisAttributes *WaferSiVisAtt = new G4VisAttributes(G4Colour::Magenta());
      WaferSiVisAtt->SetForceSolid(false);
      WaferSiVisAtt->SetVisibility(true);
      WaferSiLog->SetVisAttributes(WaferSiVisAtt);

      G4double real_wafer_size_x = wafer_dim_x + 2 * Ecal_guard_ring_size;
      G4int n_wafers_x           = int(floor(slab_dim_x / real_wafer_size_x));      
      G4double wafer_pos_x       = -slab_dim_x/2 + Ecal_guard_ring_size + wafer_dim_x /2 ;
 
     G4int n_wafer_x;
      for (n_wafer_x = 1; n_wafer_x < n_wafers_x + 1; n_wafer_x++)
	{
	  G4double wafer_pos_z = -slab_dim_z/2 + Ecal_guard_ring_size + wafer_dim_z /2;
	  for (G4int n_wafer_z = 1; n_wafer_z < 3; n_wafer_z++)
	    {
	      new MyPlacement(0,
			      G4ThreeVector(wafer_pos_x, wafer_pos_z, 0),
			      WaferSiLog,
			      "WaferSi",
			      CommonSiLog,
			      false,
			      n_wafer_x*1000 + n_wafer_z);

	      wafer_pos_z += wafer_dim_z + 2 * Ecal_guard_ring_size;
	    }

	  wafer_pos_x += wafer_dim_x + 2 * Ecal_guard_ring_size;
	}
      
      /* Magic wafers to complete the slab...
	 (wafers with variable number of cells just
	 to complete the slab. in reality we think that
	 we'll have just a few models of special wafers for that.*/
 
     G4double resting_dim_x = slab_dim_x - (wafer_dim_x + 2 * Ecal_guard_ring_size) * n_wafers_x;
      
     if(resting_dim_x > (cell_dim_x + 2 * Ecal_guard_ring_size))
	{
	  G4int N_cells_x_remaining = int(floor((resting_dim_x - 2 * Ecal_guard_ring_size)/cell_dim_x));
	  wafer_dim_x = N_cells_x_remaining * cell_dim_x;
	  
	  WaferSiSolid = new G4Box("WaferSiSolid", 
				   wafer_dim_x/2 - G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
				   wafer_dim_z/2 - G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
				   Ecal_Si_thickness/2 - 2*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()));
	  
	  WaferSiLog = new G4LogicalVolume(WaferSiSolid,
					   CGAGeometryManager::
					   GetMaterial("silicon_2.33gccm"),
					   "WaferSiLog", 
					   0, 0, pULimits);  
	  WaferSiLog->SetVisAttributes(WaferSiVisAtt);
	  WaferSiLog->SetSensitiveDetector(theSD);
	  
	  wafer_pos_x =  -slab_dim_x/2 + n_wafers_x * real_wafer_size_x + (wafer_dim_x + 2 * Ecal_guard_ring_size)/2;
	  real_wafer_size_x = wafer_dim_x + 2 * Ecal_guard_ring_size;
	  
	  G4double wafer_pos_z =  -slab_dim_z/2 + Ecal_guard_ring_size + wafer_dim_z /2;

	  for (G4int n_wafer_z = 1; n_wafer_z < 3; n_wafer_z++)
	    {
	      new MyPlacement(0,
			      G4ThreeVector(wafer_pos_x, wafer_pos_z, 0),
			      WaferSiLog,
			      "WaferSi",
			      CommonSiLog,
			      false,
			      n_wafer_x*1000 + n_wafer_z);
	      wafer_pos_z += wafer_dim_z + 2 * Ecal_guard_ring_size;
	    }
	}
    }
  else
    {
      /* Endcaps*/

      /* redefine slightly for endcap*/
      float useful_wafer_dim_endcap = slab_dim_x/2 - 2.*Ecal_guard_ring_size;

      int N_cells_in_X_endcap = int(useful_wafer_dim_endcap/cell_dim_x);
      float cell_dim_x_endcap = useful_wafer_dim_endcap/N_cells_in_X_endcap;

      /* Normal squared wafers*/
      G4double wafer_dim_x = N_cells_in_X_endcap*cell_dim_x_endcap;
      G4double wafer_dim_z = wafer_dim_x;

      G4Box *WaferSiSolid = new G4Box("WaferSiSolid", 
				      wafer_dim_x/2 - G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
				      wafer_dim_z/2 - G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
				      Ecal_Si_thickness/2 - 2*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()));
      
      G4LogicalVolume *WaferSiLog = new G4LogicalVolume(WaferSiSolid,
							CGAGeometryManager::
							GetMaterial("silicon_2.33gccm"),
							"WaferSiLog", 
							0, 0, pULimits);  
      WaferSiLog->SetSensitiveDetector(theSD);
      G4VisAttributes *WaferSiVisAtt = new G4VisAttributes(G4Colour::Green());
      WaferSiVisAtt->SetForceSolid(false);
      WaferSiVisAtt->SetVisibility(true);
      WaferSiLog->SetVisAttributes(WaferSiVisAtt);

      G4double real_wafer_size_x = wafer_dim_z + 2 * Ecal_guard_ring_size;
      
      G4int n_wafers_x = int(floor(slab_dim_z / real_wafer_size_x));
      
      G4double wafer_pos_x = -slab_dim_z/2 + Ecal_guard_ring_size + wafer_dim_z /2;

      G4int n_wafer_x;
      for (n_wafer_x = 1; n_wafer_x < n_wafers_x + 1; n_wafer_x++)
	{
	  G4double wafer_pos_z = -slab_dim_x/2 + Ecal_guard_ring_size + wafer_dim_x /2;

	  for (G4int n_wafer_z = 1; n_wafer_z < 3; n_wafer_z++)
	    {
	      new MyPlacement(0,
			      G4ThreeVector(wafer_pos_z, wafer_pos_x, 0),
			      WaferSiLog,
			      "WaferSi",
			      CommonSiLog,
			      false,
			      n_wafer_x*1000 + n_wafer_z);

	      wafer_pos_z += wafer_dim_x + 2 * Ecal_guard_ring_size;
	    }

	  wafer_pos_x += wafer_dim_z + 2 * Ecal_guard_ring_size;
	}
      
      /* Magic wafers to complete the slab...
	 (wafers with variable number of cells just
	 to complete the slab. in reality we think that
	 we'll have just a few models of special wafers for that.*/

      G4double resting_dim_x = slab_dim_z - (wafer_dim_z + 2 * Ecal_guard_ring_size) * n_wafers_x;
      
      if(resting_dim_x > (cell_dim_z + 2 * Ecal_guard_ring_size))
	{
	  G4int N_cells_x_remaining = int(floor((resting_dim_x - 2 * Ecal_guard_ring_size) /cell_dim_z));
	  wafer_dim_x = N_cells_x_remaining * cell_dim_z;
	  
	  WaferSiSolid = new G4Box("WaferSiSolid", 
				   wafer_dim_z/2 - G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
				   wafer_dim_x/2 - G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
				   Ecal_Si_thickness/2 - 2*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()));
	  
	  WaferSiLog = new G4LogicalVolume(WaferSiSolid,
					   CGAGeometryManager::
					   GetMaterial("silicon_2.33gccm"),
					   "WaferSiLog", 
					   0, 0, pULimits);  
	  WaferSiLog->SetVisAttributes(WaferSiVisAtt);
	  WaferSiLog->SetSensitiveDetector(theSD);
	  
	  wafer_pos_x = -slab_dim_z/2 + n_wafers_x * real_wafer_size_x + (wafer_dim_x + 2 * Ecal_guard_ring_size)/2;
	  
	  real_wafer_size_x = wafer_dim_x + 2 * Ecal_guard_ring_size;
	  
	  G4double wafer_pos_z = -slab_dim_x/2 + Ecal_guard_ring_size + wafer_dim_z /2;

	  for (G4int n_wafer_z = 1; n_wafer_z < 3; n_wafer_z++)
	    {
	      new MyPlacement(0,
			      G4ThreeVector(wafer_pos_z, wafer_pos_x, 0),
			      WaferSiLog,
			      "WaferSi",
			      CommonSiLog,
			      false,
			      n_wafer_x*1000 + n_wafer_z);

	      wafer_pos_z += wafer_dim_z + 2 * Ecal_guard_ring_size;
	    }
	} 

    }
  
  /* Glue space as just a air gap, we don't care about a few points of glue...*/
  
  y_slab_floor += Ecal_Si_thickness + Ecal_Slab_glue_gap;

  
  /* The PCB layer, the copper and the shielding are
     placed as a big G10 layer, as the copper and the
     shielding ones are very tiny.*/
 
  G4double PCBCuShield_thickness = Ecal_Slab_PCB_thickness +  Ecal_Slab_copper_thickness + Ecal_Slab_shielding;
  
  G4Box *PCBCuShieldSolid = new G4Box("PCBCuShieldSolid", 
				      slab_dim_x/2 - 2*( G4GeometryTolerance::GetInstance()->GetSurfaceTolerance() ),
				      slab_dim_z/2 - 2*( G4GeometryTolerance::GetInstance()->GetSurfaceTolerance() ),
				      PCBCuShield_thickness/2 - 2*( G4GeometryTolerance::GetInstance()->GetSurfaceTolerance() ) );
  G4LogicalVolume *PCBCuShieldLog = new G4LogicalVolume(PCBCuShieldSolid, siPCBMix, "PCBCuShieldLogical", 0, 0, 0);
  G4VisAttributes *PCBCuShieldVisAtt = new G4VisAttributes(G4Colour::Green());
  PCBCuShieldVisAtt->SetForceWireframe(true);
  PCBCuShieldVisAtt->SetVisibility(false);
  PCBCuShieldLog->SetVisAttributes(PCBCuShieldVisAtt);

  new MyPlacement(0,
		  G4ThreeVector(0, 0, y_slab_floor + PCBCuShield_thickness/2),
		  PCBCuShieldLog,
		  "PCBCuShield",
		  SlabLog,
		  false,0);
  return SlabLog;
}
/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
void SEcal05::BuildECRingAlveolus (G4int layer_id, G4double z_floor, G4LogicalVolume *ECLog, G4bool Zminus)
{
  G4LogicalVolume *SiLog = ECRingSiLog;
  if(Zminus) SiLog = ECRingSiLogZminus;
  
  /* place Si 1 (in z_floor + Ecal_total_SiSlab_thickness / 2)*/
  new MyPlacement(0,
		  G4ThreeVector (0, 0, z_floor + Ecal_total_SiSlab_thickness / 2),
		  SiLog,
		  "SlabSiECRing",
		  ECLog,
		  false,
		  layer_id, SECAL_CHECK_OVERLAP);
  
  /* set layer in SD*/
  if(!Zminus)
    {
      theEndCapRingSD->AddLayer(layer_id,
				- ECRingSiBox->GetXHalfLength(),
				- ECRingSiBox->GetYHalfLength(),
				z_floor + Ecal_total_SiSlab_thickness / 2);
    }
  
  z_floor += Ecal_total_SiSlab_thickness + Ecal_fiber_thickness;

  /* place Rad (in z_floor + Ecal_total_SiSlab_thickness + N X fiber + RadThick / 2)*/

  G4LogicalVolume *radiatorLog = NULL;
  G4double RadiatorThickness = 0.;

  if(layer_id <= Ecal_nlayers1) 
    {
      radiatorLog       = EndCapRingSlabRadiatorL1;
      RadiatorThickness = Ecal_radiator_thickness1;
    }

  if(layer_id > Ecal_nlayers1 && layer_id <= Ecal_nlayers1 + Ecal_nlayers2)
    {
      radiatorLog       = EndCapRingSlabRadiatorL2;
      RadiatorThickness = Ecal_radiator_thickness2;
    }
  
  if(layer_id > Ecal_nlayers1 + Ecal_nlayers2 && layer_id <= Ecal_nlayers1 + Ecal_nlayers2 + Ecal_nlayers3)
    {
      radiatorLog       = EndCapRingSlabRadiatorL3;
      RadiatorThickness = Ecal_radiator_thickness3;
    }
  
  new MyPlacement(0,
		  G4ThreeVector (0, 0, z_floor + RadiatorThickness / 2),
		  radiatorLog,
		  "SlabRadECRing",
		  ECLog,
		  false,
		  layer_id, SECAL_CHECK_OVERLAP);

  z_floor += RadiatorThickness + Ecal_fiber_thickness;;

  /* place Si 2 (in z_floor + Ecal_total_SiSlab_thickness +
     N X fiber + RadThick +  N X fiber + Ecal_total_SiSlab_thickness / 2)*/
  new MyPlacement(0,
		  G4ThreeVector (0, 0, z_floor + Ecal_total_SiSlab_thickness / 2),
		  SiLog,
		  "SlabSiECRing",
		  ECLog,
		  false,
		  layer_id + 1, SECAL_CHECK_OVERLAP);

  if(!Zminus)
    {
      theEndCapRingSD->AddLayer(layer_id+1,
				- ECRingSiBox->GetXHalfLength(),
				- ECRingSiBox->GetYHalfLength(),
				z_floor + Ecal_total_SiSlab_thickness / 2);
    }
}

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
G4double SEcal05::BuildECAlveolus (G4int layer_id, G4double z_floor, G4LogicalVolume *ECLog, G4bool Zminus)
{
  if (useScintillatorTilesOfVaryingSize)
    {
      if(EC_Scint_Along_X) 
	{
	  EC_Initialize(EC_ScintillatorParallelToXTowerSlabs_first, 'x', layer_id);
	  EC_Initialize(EC_ScintillatorParallelToXTowerSlabs_second, 'x', layer_id + 1);
	}
      if(EC_Scint_Along_Z) 
	{
	  EC_Initialize(EC_ScintillatorParallelToZTowerSlabs_first, 'z', layer_id);
	  EC_Initialize(EC_ScintillatorParallelToZTowerSlabs_second, 'z', layer_id + 1);
	}
    }
  
#ifdef VERBOSE
  G4cout<<"\n\n ----------------------------------- BuildECAlveolus"<<G4endl;
  G4cout<<" layer_id="<<layer_id<<G4endl;
#endif

  std::vector<G4LogicalVolume*> *EC_FirstLayerTowerSlabs = &EC_SiliconTowerSlabs;
  std::vector<G4LogicalVolume*> *EC_SecondLayerTowerSlabs = &EC_SiliconTowerSlabs;

#ifdef VERBOSE
  G4cout<<" EC_FirstLayerTowerSlabs->size(): "<<EC_FirstLayerTowerSlabs->size()<<G4endl;
  G4cout<<" EC_SecondLayerTowerSlabs->size(): "<<EC_SecondLayerTowerSlabs->size()<<G4endl;
#endif

  G4double FirstLayerSlab_Y_offset   = Si_Slab_Y_offset;
  G4double SecondLayerSlab_Y_offset  = Si_Slab_Y_offset;

  G4double FirstLayerSlab_thickness  = Ecal_total_SiSlab_thickness;
  G4double SecondLayerSlab_thickness = Ecal_total_SiSlab_thickness;

  SEcalSD02 *theFirstLayerEndCapSD  = theEndCapSiliconSD;
  SEcalSD02 *theSecondLayerEndCapSD = theEndCapSiliconSD;

  char layerCode = Ecal_EndCap_Sc_Si_Mix[(unsigned int)((layer_id-1)/2)];

  if( layerCode == ECAL_SI_LAYERS ) 
    {
      /* default values set above are for Si*/
    }
  else if( layerCode == ECAL_SC_LAYER_1_2_ALONG_X) 
    {
      FirstLayerSlab_Y_offset  = Sc_Slab_Y_offset;
      SecondLayerSlab_Y_offset = Sc_Slab_Y_offset;
	
      FirstLayerSlab_thickness  = Ecal_total_ScSlab_thickness;
      SecondLayerSlab_thickness = Ecal_total_ScSlab_thickness;
   
      if (!useScintillatorTilesOfVaryingSize)
	{
	  EC_FirstLayerTowerSlabs  = &EC_ScintillatorParallelToXTowerSlabs;
	  EC_SecondLayerTowerSlabs = &EC_ScintillatorParallelToXTowerSlabs;
	}
      else
	{
	  EC_FirstLayerTowerSlabs  = &EC_ScintillatorParallelToXTowerSlabs_first;
	  EC_SecondLayerTowerSlabs = &EC_ScintillatorParallelToXTowerSlabs_second;
	}

      theFirstLayerEndCapSD  = theEndCapScintillatorSD;
      theSecondLayerEndCapSD = theEndCapScintillatorSD;
    }
    else if( layerCode == ECAL_SC_LAYER_1_2_ALONG_Z) 
      {
	FirstLayerSlab_Y_offset  = Sc_Slab_Y_offset;
	SecondLayerSlab_Y_offset = Sc_Slab_Y_offset;

	FirstLayerSlab_thickness  = Ecal_total_ScSlab_thickness;
	SecondLayerSlab_thickness = Ecal_total_ScSlab_thickness;
  
	if (!useScintillatorTilesOfVaryingSize)
	{
	  EC_FirstLayerTowerSlabs  = &EC_ScintillatorParallelToZTowerSlabs;
	  EC_SecondLayerTowerSlabs = &EC_ScintillatorParallelToZTowerSlabs;
	}
	else
	  {
	    EC_FirstLayerTowerSlabs  = &EC_ScintillatorParallelToZTowerSlabs_first;
	    EC_SecondLayerTowerSlabs = &EC_ScintillatorParallelToZTowerSlabs_second;
	  }
	theFirstLayerEndCapSD  = theEndCapScintillatorSD;
	theSecondLayerEndCapSD = theEndCapScintillatorSD;
    }
    else if(layerCode == ECAL_SC_LAYER_1_ALONG_X_LAYER_2_ALONG_Z) 
      {
	FirstLayerSlab_Y_offset  = Sc_Slab_Y_offset;
	SecondLayerSlab_Y_offset = Sc_Slab_Y_offset;

	FirstLayerSlab_thickness  = Ecal_total_ScSlab_thickness;
	SecondLayerSlab_thickness = Ecal_total_ScSlab_thickness;

	if (!useScintillatorTilesOfVaryingSize)
	{
	  EC_FirstLayerTowerSlabs  = &EC_ScintillatorParallelToXTowerSlabs;
	  EC_SecondLayerTowerSlabs = &EC_ScintillatorParallelToZTowerSlabs;
	}
	else
	  {
	    EC_FirstLayerTowerSlabs  = &EC_ScintillatorParallelToXTowerSlabs_first;
	    EC_SecondLayerTowerSlabs = &EC_ScintillatorParallelToZTowerSlabs_second;
	  }

	theFirstLayerEndCapSD = theEndCapScintillatorSD;
	theSecondLayerEndCapSD = theEndCapScintillatorSD;
    }
    else if(layerCode == ECAL_SC_LAYER_1_ALONG_Z_LAYER_2_ALONG_X) 
      {
	FirstLayerSlab_Y_offset  = Sc_Slab_Y_offset;
	SecondLayerSlab_Y_offset = Sc_Slab_Y_offset;

	FirstLayerSlab_thickness  = Ecal_total_ScSlab_thickness;
	SecondLayerSlab_thickness = Ecal_total_ScSlab_thickness;
  
	if (!useScintillatorTilesOfVaryingSize)
	{
	  EC_FirstLayerTowerSlabs  = &EC_ScintillatorParallelToZTowerSlabs;
	  EC_SecondLayerTowerSlabs = &EC_ScintillatorParallelToXTowerSlabs;
	}
	else
	  {
	    EC_FirstLayerTowerSlabs  = &EC_ScintillatorParallelToZTowerSlabs_first;
	    EC_SecondLayerTowerSlabs = &EC_ScintillatorParallelToXTowerSlabs_second;
	  }

	theFirstLayerEndCapSD  = theEndCapScintillatorSD;
	theSecondLayerEndCapSD = theEndCapScintillatorSD;
    }
    else if (layerCode == ECAL_MIX_LAYER_1_SI_LAYER_2_ALONG_X) 
      {
	FirstLayerSlab_Y_offset = Si_Slab_Y_offset;
	SecondLayerSlab_Y_offset = Sc_Slab_Y_offset;
	FirstLayerSlab_thickness = Ecal_total_SiSlab_thickness;
	SecondLayerSlab_thickness = Ecal_total_ScSlab_thickness;
	EC_FirstLayerTowerSlabs = &EC_SiliconTowerSlabs;

	if (!useScintillatorTilesOfVaryingSize)
	  {
	    EC_SecondLayerTowerSlabs = &EC_ScintillatorParallelToXTowerSlabs;
	  }
	else
	  {
	    EC_SecondLayerTowerSlabs = &EC_ScintillatorParallelToXTowerSlabs_second;
	  }

	theSecondLayerEndCapSD = theEndCapScintillatorSD;
    }
    else if (layerCode == ECAL_MIX_LAYER_1_SI_LAYER_2_ALONG_Z) 
      {
	FirstLayerSlab_Y_offset = Si_Slab_Y_offset;
	SecondLayerSlab_Y_offset = Sc_Slab_Y_offset;
	FirstLayerSlab_thickness = Ecal_total_SiSlab_thickness;
	SecondLayerSlab_thickness = Ecal_total_ScSlab_thickness;
	EC_FirstLayerTowerSlabs = &EC_SiliconTowerSlabs;

	if (!useScintillatorTilesOfVaryingSize)
	  {
	    EC_SecondLayerTowerSlabs = &EC_ScintillatorParallelToZTowerSlabs;
	  }
	else
	  {
	    EC_SecondLayerTowerSlabs = &EC_ScintillatorParallelToZTowerSlabs_second;
	  }

	theSecondLayerEndCapSD = theEndCapScintillatorSD;
      }
    else if (layerCode == ECAL_MIX_LAYER_1_ALONG_X_LAYER_2_SI) 
      {
	FirstLayerSlab_Y_offset = Sc_Slab_Y_offset;
	SecondLayerSlab_Y_offset = Si_Slab_Y_offset;
	FirstLayerSlab_thickness = Ecal_total_ScSlab_thickness;
	SecondLayerSlab_thickness = Ecal_total_SiSlab_thickness;

	if (!useScintillatorTilesOfVaryingSize)
	  {
	    EC_FirstLayerTowerSlabs = &EC_ScintillatorParallelToXTowerSlabs;
	  }
	else
	  {
	    EC_FirstLayerTowerSlabs = &EC_ScintillatorParallelToXTowerSlabs_first;
	  }

	EC_SecondLayerTowerSlabs = &EC_SiliconTowerSlabs;
	theFirstLayerEndCapSD = theEndCapScintillatorSD;
      }
    else if (layerCode == ECAL_MIX_LAYER_1_ALONG_Z_LAYER_2_SI) 
      {
	FirstLayerSlab_Y_offset = Sc_Slab_Y_offset;
	SecondLayerSlab_Y_offset = Si_Slab_Y_offset;
	FirstLayerSlab_thickness = Ecal_total_ScSlab_thickness;
	SecondLayerSlab_thickness = Ecal_total_SiSlab_thickness;

	if (!useScintillatorTilesOfVaryingSize)
	  {
	    EC_FirstLayerTowerSlabs = &EC_ScintillatorParallelToZTowerSlabs;
	  }
	else
	  {
	    EC_FirstLayerTowerSlabs = &EC_ScintillatorParallelToZTowerSlabs_first;
	  }

	EC_SecondLayerTowerSlabs = &EC_SiliconTowerSlabs;
	theFirstLayerEndCapSD = theEndCapScintillatorSD;
      }
    else 
      {
	G4cout << layerCode << G4endl;
	Control::Abort("SEcal05 d: The Ecal_Barrel_Sc_Si_Mix parameter should contain only 0,1,2,3,4,5,6,7,9",
		       MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
      }
  
  G4double AlveolusThickness = 0;
  G4double W_thick = GiveMeRadiatorThickness(layer_id);

  G4LogicalVolume *radiatorLog = NULL;
  
  AlveolusThickness = FirstLayerSlab_thickness + SecondLayerSlab_thickness +  W_thick + 2 * Ecal_fiber_thickness;
  
  G4RotationMatrix rotOdd;
  rotOdd.rotateZ(pi);
  rotOdd.rotateX(pi);

  G4RotationMatrix rotEven;
  rotEven.rotateZ(pi);

  G4RotationMatrix *rot;
  G4int limit_staves;
  limit_staves = 4;

  for (G4int i_stave = 1; i_stave <= limit_staves; i_stave ++)
    {
      G4double angle_module = pi/2 * ( i_stave - 1 );

      if(layer_id == 1) 
	{
	  if(Number_of_Si_Layers_in_EC != 0)theEndCapSiliconSD->SetStaveRotationMatrix(i_stave,angle_module);
	  if(Number_of_Sc_Layers_in_EC != 0)theEndCapScintillatorSD->SetStaveRotationMatrix(i_stave,angle_module);
      }

      for (unsigned int i_tower = 0; i_tower < EC_FirstLayerTowerSlabs->size(); i_tower++)
	{
	  /* first slab*/
	  G4ThreeVector pos = EC_TowerXYCenters[i_tower];
	  pos[2] = z_floor + FirstLayerSlab_thickness /2;
	  rot = new G4RotationMatrix(rotOdd);
	  rot->rotateZ(angle_module);
	  pos.rotateZ(angle_module);

	  std::stringstream temporaryString; 
	  temporaryString << i_stave*100000 + (i_tower+1) * 1000 + layer_id;

	  new MyPlacement(rot, pos, (*EC_FirstLayerTowerSlabs)[i_tower], G4String("FirstSlabEC_") + G4String(temporaryString.str()), ECLog,
			  false, i_stave*100000 + (i_tower+1) * 1000 + layer_id);
      
	  if (i_stave == 1 && i_tower == 0 && !Zminus) 
	    {
	      theFirstLayerEndCapSD->AddLayer(layer_id,
					      - pos[0] - ((G4Box *)(*EC_FirstLayerTowerSlabs)[i_tower]->GetSolid())->GetXHalfLength(),
					      pos[2] - FirstLayerSlab_Y_offset,
					      pos[1] - ((G4Box *)(*EC_FirstLayerTowerSlabs)[i_tower]->GetSolid())->GetYHalfLength());
	  }

	  /* radiator inside alveolus*/
	  if(layer_id <= Ecal_nlayers1) 
	    {
	      radiatorLog = EC_TowerR1[i_tower];
	    }

	  if(layer_id > Ecal_nlayers1 && layer_id <= Ecal_nlayers1 + Ecal_nlayers2)
	    {
	      radiatorLog = EC_TowerR2[i_tower];
	    }

	  if(layer_id > Ecal_nlayers1 + Ecal_nlayers2 && layer_id <= Ecal_nlayers1 + Ecal_nlayers2 + Ecal_nlayers3)
	    {
	      radiatorLog = EC_TowerR3[i_tower];
	    }

	  /* reinitialise pos as it's now rotated for the slab*/
	  pos = EC_TowerXYCenters[i_tower];
	  pos[2] = z_floor + FirstLayerSlab_thickness /2;
	  /* update pos to take in account slab + fiber dim*/
	  pos[2] += FirstLayerSlab_thickness / 2 + Ecal_fiber_thickness + W_thick/2;
	  
	  rot = new G4RotationMatrix(rotEven);
	  rot->rotateZ(angle_module);
	  pos.rotateZ(angle_module);

	  new MyPlacement(rot, pos, radiatorLog, "RadEC", ECLog, false, 0);

#ifdef MOKKA_GEAR
	  if(!Zminus)
	    {
	      if (i_stave == 1 && i_tower == 0) 
		{
		  /* get positions of Layer as the middle of the radiator layer */
		  helpEndcap.layerPos.push_back(pos[2]);
		  helpPlug.layerPos.push_back(pos[2]);
		  
		  /* get radiator thickness*/
		  helpEndcap.radiThickness.push_back(W_thick);
		  helpPlug.radiThickness.push_back(W_thick);
		  
		  helpEndcap.count ++ ;
		  helpPlug.count ++ ;
		}
	    }
#endif
	  
	  /* second slab*/
	  pos[2] +=  W_thick/2 + Ecal_fiber_thickness + SecondLayerSlab_thickness /2;

	  rot = new G4RotationMatrix(rotOdd);
	  rot->rotateZ(angle_module);
	  rot->rotateY(pi);

	  std::stringstream temporaryString2; 
	  temporaryString2 << i_stave*100000 + (i_tower+1) * 1000 + layer_id + 1;
	  new MyPlacement(rot, pos, (*EC_SecondLayerTowerSlabs)[i_tower], G4String("SecondSlabEC_") + G4String(temporaryString2.str()), ECLog,
			  false, i_stave*100000 + (i_tower+1) * 1000 + layer_id+1, SECAL_CHECK_OVERLAP);

	  if (i_stave == 1 && i_tower == 0 && !Zminus) 
	    {
	      theSecondLayerEndCapSD->AddLayer(layer_id + 1, 
					       - pos[0] - ((G4Box *)(*EC_SecondLayerTowerSlabs)[i_tower]->GetSolid())->GetXHalfLength(),
					       pos[2] + SecondLayerSlab_Y_offset, 
					       pos[1] - ((G4Box *)(*EC_SecondLayerTowerSlabs)[i_tower]->GetSolid())->GetYHalfLength());
	      
	    }
	}
    }
  
  return AlveolusThickness;
}

/*********************************************************************************************/
/*                                                                                           */
/* EC_Initialize() builds the Slabs and the radiator plates for the several towers           */
/* to be placed latter into each module                                                      */
/*                                                                                           */
/*********************************************************************************************/
G4bool SEcal05::EC_Initialize(std::vector<G4LogicalVolume*>& EC_TowerSlabs,
			      char direction, G4int layer)
{  
  static G4bool EC_TowerXYCentersFlag = true;

  /* Take care:
  
  To turn the direction of slabs in the end caps
  we interchanged X<->Y. It's not clean, but it
  works...*/

  G4double y_middle = (Ecal_inner_radius + module_thickness + Ecal_endcap_extra_size) * tan(pi/8)  - Ecal_lateral_face_thickness;
  G4double x_left   = -(Ecal_inner_radius + module_thickness + Ecal_endcap_extra_size)  + Ecal_lateral_face_thickness;
  G4double x_middle = -y_middle;

  G4double x_right = + Ecal_endcap_center_box_size / 2 - Ecal_lateral_face_thickness;
  
  fiber_inter_alveolus = 2 * (Ecal_Slab_H_fiber_thickness + Ecal_Slab_shielding + N_FIBERS_ALVEOLUS * Ecal_fiber_thickness);
  
  EC_alveolus_dim_y = EC_alveolus_width; /* Daniel Jeans: this is adjusted width*/

  G4double EC_alveolus_x_left = 0;
  G4double EC_alveolus_dim_x  = 0 ;
  G4double alv_upper_y        = 0;

  G4double inc = (x_middle -x_left ) / (EC_y_top - y_middle);

  G4int EC_Number_of_towers = 0;
  G4double last_dim_x       = 0;
  G4double y_floor          = EC_y_botton;

#ifdef VERBOSE
  G4cout<<"\n\n-------------------- EC_Initialize"<<G4endl;
  G4cout<<" EC_n_alveolus="<<EC_n_alveolus<<G4endl;
  G4cout<<" direction="<<direction<<G4endl;
  G4cout<<" EC_TowerSlabs.size="<<EC_TowerSlabs.size()<<G4endl;
#endif

  if (EC_TowerSlabs.size() > 0) EC_TowerSlabs.clear();

  for (int i = 0; i < EC_n_alveolus; i++)
    {
      alv_upper_y = y_floor + EC_alveolus_dim_y;

      if( alv_upper_y <= y_middle )
	{
	  EC_alveolus_dim_x = x_right- x_left;
	}
      else
	{
	  EC_alveolus_x_left = (alv_upper_y - y_middle) * inc + x_left;
	  EC_alveolus_dim_x = x_right - EC_alveolus_x_left;
	}

      /* We use the same method able to create the Barrel
	 Slabs, so we have to rotate it later when placing 
	 into the EC modules.
	 
	 We use the same method able to create the Barrel
	 radiator plates between slabs, but with the good
	 dimensions to avoid to rotate it later when placing 
	 into the EC modules.
	 
	 While the towers have the same shape use the same 
	 logical volumes and parameters.*/

      if(last_dim_x != EC_alveolus_dim_x )
	{
	  if(direction == '0') 
	    {
	      EC_TowerSlabs.push_back(BuildSiliconSlab(EC_alveolus_dim_y,
						       Ecal_total_SiSlab_thickness,
						       EC_alveolus_dim_x,
						       theEndCapSiliconSD));
	    } 
	  else 
	    {
#ifdef VERBOSE
	      G4cout<<"\n\n This is endcaps before BuildScintillatorSlab: EC_StripSizeinX="<<EC_StripSizeinX
		    <<" EC_StripSizeParallelToZ="<<EC_StripSizeParallelToZ
		    <<" EC_Number_of_towers="<<EC_Number_of_towers
		    <<" i="<<i
		    <<G4endl;
#endif
	      
            if (useScintillatorTilesOfVaryingSize)
            {
                EC_StripSizeinX = endcapEcal_Sc_cellDim1_vector[layer - 1];
                EC_StripSizeParallelToZ = endcapEcal_Sc_cellDim2_vector[layer - 1];
                Ecal_Sc_N_strips_across_module = endcapEcal_Sc_N_strips_across_module_vector[layer - 1];
                EC_N_cells_in_Z = endcapEcal_Sc_N_strip_containers_along_z_vector[layer - 1] / 2;
                N_cells_in_Z = EC_N_cells_in_Z; // ATTN: It is N_cells_in_Z, not EC_N_cells_in_Z that is used in BuildSensitiveVolume
                EC_virtualCellDim = EC_StripSizeParallelToZ / Ecal_Sc_number_of_virtual_cells;
            }

#ifdef VERBOSE
	      G4cout<<"\n  new EC_StripSizeinX="<<EC_StripSizeinX
		    <<" EC_StripSizeParallelToZ="<<EC_StripSizeParallelToZ
		    <<" layer="<<layer
		    <<G4endl;
#endif
	      
	      EC_TowerSlabs.push_back(BuildScintillatorSlab(EC_alveolus_dim_y,
							    Ecal_total_ScSlab_thickness,
							    EC_alveolus_dim_x,
							    direction,
							    theEndCapScintillatorSD,
							    EC_StripSizeinX,
							    EC_StripSizeParallelToZ));
	    }
	  
	  if( Ecal_nlayers1 > 0 )
	    {
	      EC_TowerR1.push_back(BuildRadiatorPlate(EC_alveolus_dim_y,
						      Ecal_radiator_thickness1,
						      EC_alveolus_dim_x));
	    }
	  if( Ecal_nlayers2 > 0 )
	    {
	      EC_TowerR2.push_back(BuildRadiatorPlate(EC_alveolus_dim_y,
						      Ecal_radiator_thickness2,
						      EC_alveolus_dim_x));
	    }
	  if( Ecal_nlayers3 > 0 )
	    {
	      EC_TowerR3.push_back(BuildRadiatorPlate(EC_alveolus_dim_y,
						      Ecal_radiator_thickness3,
						      EC_alveolus_dim_x));
	    }
	  
	  last_dim_x = EC_alveolus_dim_x;
	}
      else
	{
	  EC_TowerSlabs.push_back(EC_TowerSlabs[EC_Number_of_towers-1]);

	  if( Ecal_nlayers1 > 0 ) EC_TowerR1.push_back(EC_TowerR1[EC_Number_of_towers-1]);
	  if( Ecal_nlayers2 > 0 ) EC_TowerR2.push_back(EC_TowerR2[EC_Number_of_towers-1]);
	  if( Ecal_nlayers3 > 0 ) EC_TowerR3.push_back(EC_TowerR3[EC_Number_of_towers-1]);
	}

      if(EC_TowerXYCentersFlag)
	{
	  EC_TowerXYCenters.push_back(G4ThreeVector(-(y_floor + EC_alveolus_dim_y/2),
						    -(-EC_alveolus_dim_x/2 + x_right),
						    0));
	}
      
      EC_Number_of_towers++;
 
      /* Update y_floor*/
      y_floor += EC_alveolus_dim_y + fiber_inter_alveolus;
    }

  G4cout << "\nFor information: Ecal endcap modules have "
	 << EC_Number_of_towers
	 << " towers,\n" 
	 << EC_y_top - y_floor 
	 << " mm are lost (no enough space left for more one tower)."
	 << G4endl;

  EC_TowerXYCentersFlag = false;
  return true;  
}
/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
G4bool SEcal05::PostConstructAction(CGAGeometryEnvironment& theGeometryEnvironment)
{
  /*propagates the actual Ecal outer radius and endcap zmax to 
    avoid overlaps by the Hcal, if any */
  std::ostringstream oss;  
  oss << Ecal_inner_radius + module_thickness;
  (*Control::globalModelParameters)["Ecal_outer_radius"] = oss.str();
  
  std::ostringstream oss2;
  oss2 << EC_module_z_offset + module_thickness / 2;
  (*Control::globalModelParameters)["Ecal_endcap_zmax"] = oss2.str();


  /* propagates the actual Ecal endcap zmin and outer radius 
     to aline the Hcal rings, if any*/
  std::ostringstream oss_endcap1;  
  oss_endcap1 << EC_module_z_offset - module_thickness / 2;
  (*Control::globalModelParameters)["Ecal_endcap_zmin"] = oss_endcap1.str();

  /* Modifies Lcal_z_begin, as the LCal01 driver use its own parameter */
  std::ostringstream oss_Lcal;
  oss_Lcal << EC_module_z_offset - module_thickness / 2;
  (*Control::globalModelParameters)["Lcal_z_begin"] = oss_Lcal.str();

  /* propagates the actual inner radius of Ecal plug needed by LCAL to avoid overlap*/
  std::ostringstream oss_plugR;
  oss_plugR <<  theGeometryEnvironment.GetParameterAsDouble("Ecal_Lcal_ring_gap") 
              + theGeometryEnvironment.GetParameterAsDouble("Lcal_outer_radius");
  (*Control::globalModelParameters)["Ecal_endcap_plug_rmin"] = oss_plugR.str();

  std::ostringstream oss_endcap2;  
  oss_endcap2 << Ecal_endcap_rmax ;
  (*Control::globalModelParameters)["Ecal_endcap_outer_radius"] = oss_endcap2.str();

  
    /* The Ecal driver has the responsability to change the tracker region parameters.*/
  (*Control::globalModelParameters)["tracker_region_rmax"] = theGeometryEnvironment.GetParameterAsString("TPC_outer_radius");
  
  std::ostringstream oss3;
  oss3 << EC_module_z_offset  - module_thickness / 2;
  (*Control::globalModelParameters)["tracker_region_zmax"] =  oss3.str();
  
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
  (*Control::globalModelParameters)["TPC_Ecal_Hcal_barrel_halfZ"] = oss4.str();

  return true;
}

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
void SEcal05::DefineMaterial(void) 
{
  G4String name;
  G4int nel;
  
  G4double cuTracksThickness = 0.1 * mm;
  G4double g10Thickness = Ecal_Slab_PCB_thickness - cuTracksThickness;
  G4double cuThickness  = Ecal_Slab_copper_thickness + cuTracksThickness  + Ecal_Slab_shielding;
  
  G4double g10Volumefraction = g10Thickness / (g10Thickness + cuThickness);
    
  G4Material *theG10Mat = CGAGeometryManager::GetMaterial("g10");
  
  G4double g10Density = (theG10Mat->GetDensity())/(g/cm3);
  
  G4Material *theCuMat = CGAGeometryManager::GetMaterial("copper");

  G4double cuDensity = (theCuMat->GetDensity())/(g/cm3);
	
  G4double pcbMixDensity = (g10Density * g10Volumefraction + cuDensity *(1 - g10Volumefraction)) * g/cm3;
  
  siPCBMix = new G4Material(name="siPCBMix", pcbMixDensity, nel=2);

  G4double g10FractionMass = g10Density * g10Thickness / (g10Density * g10Thickness + cuDensity * cuThickness);

  siPCBMix->AddMaterial(theG10Mat, g10FractionMass);
  siPCBMix->AddMaterial(theCuMat, 1 - g10FractionMass);
  
  G4cout << "siPCBMix: g10FractionMass = " << g10FractionMass << G4endl;
  G4cout << "siPCBMix: cuFractionMass = " << 
    cuDensity * cuThickness/ 
    (g10Density * g10Thickness + cuDensity * cuThickness)
	 << G4endl;
  
  G4cout << "siPCBMix->GetDensity() = "
	 << (siPCBMix->GetDensity())/(g/cm3)   << " g/cm3\n";
  G4cout << "siPCBMix->GetRadlen() = "
	 << siPCBMix->GetRadlen() /cm   << " cm\n";
  
  
  G4double cuGroundOrHVThickness     = Ecal_Slab_ground_thickness / 2.;
  G4double kaptonGroundOrHVThickness = Ecal_Slab_ground_thickness / 2.;
  
  G4Material *theKaptonMat = CGAGeometryManager::GetMaterial("kapton");
  
  G4double kaptonDensity = (theKaptonMat->GetDensity())/(g/cm3);
  
  G4double kaptonVolumefraction = kaptonGroundOrHVThickness / (kaptonGroundOrHVThickness + cuGroundOrHVThickness);

  G4double groundMixDensity = (kaptonDensity * kaptonVolumefraction + cuDensity *(1 - kaptonVolumefraction)) * g/cm3;

  groundMix = new G4Material(name="GroundOrHVMix", groundMixDensity, nel=2);

  G4double kaptonFractionMass = kaptonDensity * kaptonGroundOrHVThickness / (kaptonDensity * kaptonGroundOrHVThickness 
									     + cuDensity * cuGroundOrHVThickness);

  groundMix->AddMaterial(theKaptonMat, kaptonFractionMass);
  groundMix->AddMaterial(theCuMat, 1 - kaptonFractionMass);
  
  G4cout << "groundMix: kaptonFractionMass = " << kaptonFractionMass 
	 << G4endl;
  G4cout << "groundMix: cuFractionMass = " << 
    cuDensity * cuGroundOrHVThickness / 
    (kaptonDensity * kaptonGroundOrHVThickness 
     + cuDensity * cuGroundOrHVThickness)
	 << G4endl;
  
  G4cout << "groundMix->GetDensity() = "
	 << (groundMix->GetDensity())/(g/cm3)   << " g/cm3\n";
  G4cout << "groundMix->GetRadlen() = "
	 << groundMix->GetRadlen() /cm   << " cm\n";
  
  G4double ScCuTracksThickness = 0;
  G4double ScG10Thickness = Ecal_Slab_Sc_PCB_thickness - ScCuTracksThickness;
  G4double ScCuThickness  = Ecal_Slab_copper_thickness + ScCuTracksThickness + Ecal_Slab_shielding;

  G4double ScG10Volumefraction = ScG10Thickness / (ScG10Thickness + ScCuThickness);
  G4double ScPCBMixDensity = (g10Density * ScG10Volumefraction + cuDensity *(1 - ScG10Volumefraction) ) * g/cm3;
  
  scPCBMix = new G4Material(name="scPCBMix", ScPCBMixDensity, nel=2);
  
  G4double ScG10FractionMass = g10Density * ScG10Thickness /
    (g10Density * ScG10Thickness + cuDensity * ScCuThickness);
  
  scPCBMix->AddMaterial(theG10Mat, ScG10FractionMass);
  scPCBMix->AddMaterial(theCuMat, 1 - ScG10FractionMass);
  
  G4cout << "scPCBMix: ScG10FractionMass = " << ScG10FractionMass 
	 << G4endl;
  G4cout << "scPCBMix: ScCuFractionMass = " << 
    cuDensity * ScCuThickness/ 
    (g10Density*ScG10Thickness + cuDensity*ScCuThickness)
	 << G4endl;
  
  G4cout << "scPCBMix->GetDensity() = "
	 << (scPCBMix->GetDensity())/(g/cm3)   << " g/cm3\n";
  G4cout << "scPCBMix->GetRadlen() = "
	 << scPCBMix->GetRadlen() /cm   << " cm\n";
}


/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
G4LogicalVolume *SEcal05::BuildScintillatorSlab(G4double slab_dim_x,
						G4double slab_dim_y,
						G4double slab_dim_z,
						char direction,
						SEcalSD04 * theSD,
						G4double theStripSizeinX,
						G4double theStripSizeParallelToZ)
{

#ifdef VERBOSE
  G4cout<<"\n------------------- BuildScintillatorSlab"<<G4endl;
  G4cout<<" theStripSizeinX:         "<<theStripSizeinX<<G4endl;
  G4cout<<" theStripSizeParallelToZ: "<<theStripSizeParallelToZ<<G4endl;
#endif

  /* Slab solid*/
  G4Box *BoxSolid = new G4Box("SlabSolid", 
			      slab_dim_x/2,  /*hx*/
			      slab_dim_z/2,  /*hz attention!*/
			      slab_dim_y/2); /*hy attention!*/

  /* Slab Logical*/
  G4LogicalVolume *SlabLog = new G4LogicalVolume(BoxSolid, CGAGeometryManager::GetMaterial("air"), "SlabLogical", 0, 0, 0);  
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::White());
  VisAtt->SetForceWireframe(true);
  VisAtt->SetForceSolid(false);/*do not set it to true, otherwise you don't see the daughters*/
  VisAtt->SetVisibility(true);
  VisAtt->SetDaughtersInvisible(true);
  SlabLog->SetVisAttributes(VisAtt);

  G4double y_slab_floor = - slab_dim_y /2;

  G4double sensThickness = Ecal_Sc_thickness + 2*Ecal_Sc_reflector_thickness;
  Sc_Slab_Y_offset = y_slab_floor + sensThickness/2;

  G4LogicalVolume *sensitiveLog = 0;
  G4RotationMatrix *rotateSensVol = 0;

  /*IF EndCap Slab:*/
  if(slab_dim_z > slab_dim_x) 
    {
      rotateSensVol = new G4RotationMatrix();
      rotateSensVol->rotateZ(-pi/2.);
      
      sensitiveLog = BuildSensitiveVolume(slab_dim_z,
					  slab_dim_x,
					  direction, theSD,
					  theStripSizeinX,
					  theStripSizeParallelToZ);
      
    }
  else
    {
      sensitiveLog = BuildSensitiveVolume(slab_dim_x,
					  slab_dim_z,
					  direction, theSD,
					  theStripSizeinX,
					  theStripSizeParallelToZ);
    }
  
  G4VisAttributes *SensVisAtt = new G4VisAttributes(G4Colour::Yellow());
  SensVisAtt->SetForceWireframe(true);
  SensVisAtt->SetVisibility(true);
  sensitiveLog->SetVisAttributes(SensVisAtt);

  new MyPlacement(rotateSensVol,
		  G4ThreeVector(0, 0, Sc_Slab_Y_offset),
		  sensitiveLog,
		  "SensitiveVolume",
		  SlabLog,
		  false,0);
  
  y_slab_floor += sensThickness;

  /* As the same method builds both barrel and end cap
     slabs, place the wafers along the biggest axe

     The PCB layer, the copper and the shielding are
     placed as a big layer, as the copper and the
     shielding ones are very tiny*/

  G4double ScPCBCuShield_thickness =
    Ecal_Slab_Sc_PCB_thickness +
    Ecal_Slab_copper_thickness +
    Ecal_Slab_shielding;
  
  G4Box *ScPCBCuShieldSolid = new G4Box("ScPCBCuShieldSolid", 
					slab_dim_x/2,
					slab_dim_z/2,
					ScPCBCuShield_thickness/2);
  G4LogicalVolume *ScPCBCuShieldLog = new G4LogicalVolume(ScPCBCuShieldSolid,
							  scPCBMix,
							  "ScPCBCuShieldLogical", 
							  0, 0, 0);
  G4VisAttributes *ScPCBCuShieldVisAtt = new G4VisAttributes(G4Colour::Green());
  ScPCBCuShieldVisAtt->SetForceWireframe(true);
  ScPCBCuShieldVisAtt->SetVisibility(true);
  ScPCBCuShieldLog->SetVisAttributes(ScPCBCuShieldVisAtt);

  new MyPlacement(0,
		  G4ThreeVector(0, 0, y_slab_floor+ScPCBCuShield_thickness/2),
		  ScPCBCuShieldLog,
		  "ScPCBCuShield",
		  SlabLog,
		  false,0);
  return SlabLog;
}
 
/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
G4LogicalVolume* SEcal05::BuildSensitiveVolume(G4double slab_dim_x,
	        			       G4double slab_dim_y,
					       char direction,
					       SEcalSD04 *theSD,
			                       G4double theStripSizeinX,
                   			       G4double theStripSizeParallelToZ)
{
#ifdef VERBOSE
  G4cout<<"\n--------------------- BuildSensitiveVolume"<<G4endl;
  G4cout<<" slab_dim_x: "<<slab_dim_x<<G4endl;
  G4cout<<" slab_dim_y: "<<slab_dim_y<<G4endl;
  G4cout<<" theStripSizeinX:            "<<theStripSizeinX<<G4endl;
  G4cout<<" theStripSizeParallelToZ:    "<<theStripSizeParallelToZ<<G4endl;
#endif

  G4double sensThickness = Ecal_Sc_thickness + 2*Ecal_Sc_reflector_thickness;
  G4Box *SensSolid = new G4Box("SensSolid", 
			       slab_dim_x/2 - ( G4GeometryTolerance::GetInstance()->GetSurfaceTolerance() ),
			       slab_dim_y/2 - ( G4GeometryTolerance::GetInstance()->GetSurfaceTolerance() ),
			       sensThickness/2); 
  
  /* sens Logical: should be made of the reflector material; 
     the scintillator should be placed inside it*/

  G4LogicalVolume *SensLog = new G4LogicalVolume(SensSolid,
						 CGAGeometryManager::GetMaterial("air"),
						 "SensLogical", 
						 0, 0, 0);  
  
  /*For strips alligned along Z, the X size of strips must be 
    the same as the Z size of strips alligned along X:
    we must have twice the number of cells in a Silicon wafer in Z*/

  G4LogicalVolume *nominalStripLogical = BuildStripVolume(theStripSizeinX - 6*( G4GeometryTolerance::GetInstance()->GetSurfaceTolerance() ),
							  theStripSizeParallelToZ - 6*( G4GeometryTolerance::GetInstance()->GetSurfaceTolerance() ), 
							  sensThickness, theSD);
  
  G4VisAttributes *VisAttAir = new G4VisAttributes(G4Colour(1.,1.,1.));
  VisAttAir->SetForceWireframe(true);
  VisAttAir->SetVisibility(false);
  VisAttAir->SetDaughtersInvisible(true);


  if(direction == 'z')
  {
    G4Box *stripContainerBox  = new G4Box("ScZStripContainerSolid",
					  theStripSizeinX/2 - 2*( G4GeometryTolerance::GetInstance()->GetSurfaceTolerance() ) ,
					  slab_dim_y/2 - 2*( G4GeometryTolerance::GetInstance()->GetSurfaceTolerance() ) ,
					  sensThickness/2);

    G4LogicalVolume *stripContainerLogical = new G4LogicalVolume(stripContainerBox,
								 CGAGeometryManager::GetMaterial("air"),
								 "ScZStripContainerLogical", 0, 0, 0);

    stripContainerLogical->SetVisAttributes(VisAttAir);

    G4double yOff = 0;
    for(int i_strip = 0; i_strip < Ecal_Sc_N_strips_across_module; i_strip++)
      {
	yOff = -slab_dim_y/2 + theStripSizeParallelToZ/2 + i_strip * theStripSizeParallelToZ;
	
	new MyPlacement(0, G4ThreeVector(0, yOff, 0),
                        nominalStripLogical, "ScStripsAlongZ", stripContainerLogical, false, i_strip);

      }

    int nStripContainersInX = int(slab_dim_x / theStripSizeinX);
    G4double xOffSet = -slab_dim_x/2 + theStripSizeinX/2;
	
    for(int i_container = 0; i_container < nStripContainersInX; i_container++) 
      {
       	new MyPlacement(0, G4ThreeVector(xOffSet, 0, 0),
                        stripContainerLogical, "ZStripContainerPhysical", SensLog, false,
                        10000 + i_container + 1);

	xOffSet += theStripSizeinX; 
	}
  }/*end if direction == 'z'*/
  
  else if(direction == 'x')
    {
      G4Box *stripContainerBox  = new G4Box("ScXStripContainerSolid",
					    slab_dim_x/2 - 2*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()),
					    theStripSizeinX/2 - 2*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()),
					    sensThickness/2);
      
      G4LogicalVolume *stripContainerLogical = new G4LogicalVolume(stripContainerBox, CGAGeometryManager::GetMaterial("air"),
								   "ScXStripContainerLogical", 0, 0, 0);
      
      stripContainerLogical->SetVisAttributes(VisAttAir);
      
      int nNominalStripsInX = int(slab_dim_x / theStripSizeParallelToZ);
      G4double sizeOfLastStrip = slab_dim_x - nNominalStripsInX * theStripSizeParallelToZ;
      
      if(sizeOfLastStrip <= theStripSizeParallelToZ/2) 
	{
	  nNominalStripsInX--;
	  sizeOfLastStrip += theStripSizeParallelToZ;
	}
      
      G4LogicalVolume *lastStripLogical = BuildStripVolume(theStripSizeinX - 6*( G4GeometryTolerance::GetInstance()->GetSurfaceTolerance() ) ,
							   sizeOfLastStrip - 6*( G4GeometryTolerance::GetInstance()->GetSurfaceTolerance() ) ,
							   sensThickness, theSD);
      
      G4RotationMatrix *rot = new G4RotationMatrix();
      rot->rotateZ(pi/2.);
      
      G4double xOffSet = 0;
      
      for(int i_strip = 0; i_strip < nNominalStripsInX; i_strip++) 
	{
	  xOffSet = -slab_dim_x/2 + theStripSizeParallelToZ/2 + i_strip * theStripSizeParallelToZ;
	  
	  new MyPlacement(rot, G4ThreeVector(xOffSet, 0, 0),
			  nominalStripLogical,
			  "XNominalStripPhysical",
			  stripContainerLogical,
			  false,
			  i_strip+1);
	  
	}
      
      xOffSet += (theStripSizeParallelToZ/2 + sizeOfLastStrip/2);
      
      new MyPlacement(rot, G4ThreeVector(xOffSet, 0, 0),
		      lastStripLogical,
		      "XLastStripPhysical",
		      stripContainerLogical,
		      false,
		      nNominalStripsInX+1);
      
      G4double yOffSet = 0;
  
      for(int i_cont = 0; i_cont < 2 * N_cells_in_Z; i_cont++) 
	{
	  yOffSet = -slab_dim_y/2 + theStripSizeinX/2 + i_cont * theStripSizeinX;

// #ifdef VERBOSE
// 	  G4cout<<" i_cont="<<i_cont<<G4endl;
// 	  G4cout<<" yOffSet="<<yOffSet<<G4endl;
// 	  G4cout<<" N_cells_in_Z="<<N_cells_in_Z<<" slab_dim_y="<<slab_dim_y
// 		<<" theStripSizeinX="<<theStripSizeinX<<G4endl;
// #endif
	  
	  std::stringstream temporaryString; 
	  temporaryString << i_cont;
	  new MyPlacement(0, G4ThreeVector(0, yOffSet, 0),
			  stripContainerLogical,
			  G4String("ScXStripContainersAlongZ_") + G4String(temporaryString.str()),
			  SensLog,
			  false,
			  i_cont);
	}
    }/*end if direction == 'x'*/
  else
    { 
      Control::Abort("SEcal05: The direction of the scintillator strips can only be 'x' or 'z'", MOKKA_OTHER_ERRORS);
    }

  return SensLog;
}


/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
G4LogicalVolume *SEcal05::BuildStripVolume(G4double stripSizeinX,
					   G4double stripSizeParallelToZ,
					   G4double sensThickness,
					   SEcalSD04 * theSD) 
{
  G4Box *stripBox  = new G4Box("ReflectorStripSolid",
			       stripSizeinX/2,
			       stripSizeParallelToZ/2,
			       sensThickness/2);
  
  G4LogicalVolume *stripLogical = new G4LogicalVolume(stripBox, CGAGeometryManager::GetMaterial("mylar"),
						      "ReflectorStripLogical", 0, 0, 0);
  G4VisAttributes *VisAttReflector = new G4VisAttributes(G4Colour::Yellow());
  VisAttReflector->SetForceWireframe(true);
  stripLogical->SetVisAttributes(VisAttReflector);

  /*now place the Scintillator Strip in the reflector strip*/
  G4Box *scintillatorBox  = new G4Box("ScintillatorStripSolid",
				      stripSizeinX/2 - Ecal_Sc_reflector_thickness,
				      stripSizeParallelToZ/2 - Ecal_Sc_reflector_thickness,
				      Ecal_Sc_thickness/2);
  
  G4UserLimits *pULimits= 0;
  if(Ecal_Sc_number_of_virtual_cells != 0)
    {
      pULimits = new G4UserLimits(stripSizeParallelToZ/Ecal_Sc_number_of_virtual_cells);
    }

  G4LogicalVolume *scintillatorLogical = new G4LogicalVolume(scintillatorBox,
							     CGAGeometryManager::GetMaterial("polystyrene"),
							     "ScintillatorStripLogical",
							     0, 0, pULimits);
  G4VisAttributes * VisAttScintillator = new G4VisAttributes(G4Colour(0.5,0.5,1.));
  VisAttScintillator->SetForceWireframe(true);
  scintillatorLogical->SetVisAttributes(VisAttScintillator);
  scintillatorLogical->SetSensitiveDetector(theSD);

  new MyPlacement(0, G4ThreeVector(0, 0, 0), scintillatorLogical, "ScintillatorStripPhysical",
		  stripLogical, false, 0);


  /*now place the MPPC inside the Scintillator Strip*/
  G4Box *mppcBox  = new G4Box("MPPCSolid", Ecal_Sc_MPPC_breadth/2, Ecal_MPPC_size/2, Ecal_Sc_thickness/2);

  G4LogicalVolume *mppcLogical = new G4LogicalVolume(mppcBox, CGAGeometryManager::GetMaterial("polystyrene"),
						     "MPPCLogical", 0, 0, 0);
  G4VisAttributes *VisAttMPPC = new G4VisAttributes(G4Colour::Green());
  VisAttMPPC->SetForceWireframe(true);
  mppcLogical->SetVisAttributes(VisAttMPPC);

  G4double mppcZOffset = stripSizeParallelToZ/2 - Ecal_Sc_reflector_thickness - Ecal_MPPC_size/2;

  new MyPlacement(0, G4ThreeVector(0, mppcZOffset, 0), mppcLogical, "MPPCPhysical", scintillatorLogical,
		  false, 0);

  return stripLogical;
}

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
#ifdef MOKKA_GEAR
void SEcal05::getCellDims(G4double & cell_dim_1, G4double & cell_dim_2, int i, char layerCode, bool isBarrel)
{
  if (useScintillatorTilesOfVaryingSize)
    {
      barrelStripSizeinX = barrelEcal_Sc_cellDim1_vector[i];
      virtualCellDim     = barrelEcal_Sc_cellDim2_vector[i] / Ecal_Sc_number_of_virtual_cells;

      EC_StripSizeinX   = endcapEcal_Sc_cellDim1_vector[i];
      EC_virtualCellDim = endcapEcal_Sc_cellDim2_vector[i] / Ecal_Sc_number_of_virtual_cells;      
    }

  /* can be different in barrel and endcap (in new scalable endcap design  - Daniel Jeans)*/
  G4double cell_x = isBarrel ? cell_dim_x : EC_cell_dim_x;
  G4double cell_z = isBarrel ? cell_dim_z : EC_cell_dim_z;
  G4double strip_x = isBarrel ? barrelStripSizeinX : EC_StripSizeinX;
  G4double strip_virt = isBarrel ? virtualCellDim : EC_virtualCellDim;
  
  G4double cell_dim_11 = cell_z;
  G4double cell_dim_12 = cell_x;
  G4double cell_dim_21 = cell_z;
  G4double cell_dim_22 = cell_x;
  
  switch(layerCode) 
    {
    case ECAL_SI_LAYERS:
      break;
    
    case ECAL_SC_LAYER_1_2_ALONG_X:
      cell_dim_11 = strip_x - 2*Ecal_Sc_reflector_thickness;
      cell_dim_12 = strip_virt;
      cell_dim_21 = strip_x - 2*Ecal_Sc_reflector_thickness;
      cell_dim_22 = strip_virt;
      break;
      
    case ECAL_SC_LAYER_1_2_ALONG_Z:
      cell_dim_11 = strip_virt;
      cell_dim_12 = strip_x - 2*Ecal_Sc_reflector_thickness;
      cell_dim_21 = strip_virt;
      cell_dim_22 = strip_x - 2*Ecal_Sc_reflector_thickness;
      break;
      
    case ECAL_SC_LAYER_1_ALONG_X_LAYER_2_ALONG_Z:
      cell_dim_11 = strip_x - 2*Ecal_Sc_reflector_thickness;
      cell_dim_12 = strip_virt;
      cell_dim_21 = strip_virt;
      cell_dim_22 = strip_x - 2*Ecal_Sc_reflector_thickness;
      break;
      
    case ECAL_SC_LAYER_1_ALONG_Z_LAYER_2_ALONG_X:
      cell_dim_11 = strip_virt;
      cell_dim_12 = strip_x - 2*Ecal_Sc_reflector_thickness;
      cell_dim_21 = strip_x - 2*Ecal_Sc_reflector_thickness;
      cell_dim_22 = strip_virt;
      break;
      
    case ECAL_MIX_LAYER_1_SI_LAYER_2_ALONG_X:
      cell_dim_21 = strip_x - 2*Ecal_Sc_reflector_thickness;
      cell_dim_22 = strip_virt;
      break;
      
    case ECAL_MIX_LAYER_1_SI_LAYER_2_ALONG_Z:
      cell_dim_21 = strip_virt;
      cell_dim_22 = strip_x - 2*Ecal_Sc_reflector_thickness;
      break;
      
    case ECAL_MIX_LAYER_1_ALONG_X_LAYER_2_SI:
      cell_dim_11 = strip_x - 2*Ecal_Sc_reflector_thickness;
      cell_dim_12 = strip_virt;
      break;
      
    case ECAL_MIX_LAYER_1_ALONG_Z_LAYER_2_SI:
      cell_dim_11 = strip_virt;
      cell_dim_12 = strip_x - 2*Ecal_Sc_reflector_thickness;
      break;
      
    default:
      G4cout << layerCode << G4endl;
      Control::Abort("SEcal05 e: The Ecal_Barrel_Sc_Si_Mix parameter should contain only 0,1,2,3,4,5,6,7,9", MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
    };

  cell_dim_1 = cell_dim_11;
  cell_dim_2 = cell_dim_12;
  
  if(i%2 == 0) 
    {
      cell_dim_1 = cell_dim_21;
      cell_dim_2 = cell_dim_22;
    }
  
}
#endif

/*********************************************************************************************/
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*                                                                                           */
/*********************************************************************************************/
void SEcal05::FillCellDimVector(const std::string &Ecal_Sc_cellDim_string, DoubleVector &Ecal_Sc_cellDim_vector)
{
    /*if the user has defined Ecal_Sc_cellDim_string*/
    if (!Ecal_Sc_cellDim_string.empty())
    {
        std::string dummyString = "";
        int countFoundSeparators = 0;

        for (std::string::const_iterator it = Ecal_Sc_cellDim_string.begin(), end = Ecal_Sc_cellDim_string.end(); it != end; ++it)
        {
            if ((*it) != '_')
            {
                dummyString += (*it);
            }
            else
            {
                const double value = atof(dummyString.c_str());
                Ecal_Sc_cellDim_vector.push_back(value);
                dummyString = "";
                countFoundSeparators++;
            }
        }

        /*the last number is not followed by a separator, but we still need it*/
        if (countFoundSeparators == total_number_of_layers) 
        {
            const unsigned found = Ecal_Sc_cellDim_string.find_last_of('_');
            const std::string substring = Ecal_Sc_cellDim_string.substr(found + 1);
            const double value = atof(substring.c_str());
            Ecal_Sc_cellDim_vector.push_back(value);
        }
    }
    else
    {
        for (int iLayer = 0; iLayer < total_number_of_layers + 1; ++iLayer)
        {
            Ecal_Sc_cellDim_vector.push_back(0);
        }
    }

    if (Ecal_Sc_cellDim_vector.size() != static_cast<unsigned int>((total_number_of_layers + 1)))
    {
        Control::Abort("SEcal05: The size of Ecal_Sc_cellDim1_string is not consistent with the total number of layers",
            MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
    }

#ifdef VERBOSE
    G4cout << "\n\n fillCellDimVector: " << G4endl;
    for (unsigned int i = 0 ; i < Ecal_Sc_cellDim_vector.size(); ++i)
    {
        G4cout << "  i " << i << " vec: " << Ecal_Sc_cellDim_vector[i] << G4endl;
    }
#endif
}
