//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for ILC          *
//*                                                     *
//* For more information about Mokka, please, go to the *
//*                                                     *
//*  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
//*                                                     *
//* Mokka home page.                                    *
//*                                                     *
//*******************************************************
//
// $Id: SHcal01.cc,v 1.11 2007/12/20 11:37:21 kristian Exp $
// $Name: mokka-07-00 $
//
// SHcal01.cc
//
// History:  
//

#include "MySQLWrapper.hh"
#include "Control.hh"
#include "SHcal01.hh"

#include "globals.hh"
#include "CGADefs.h"

#ifdef MOKKA_GEAR
#include "MokkaGear.h"
#endif

INSTANTIATE(SHcal01)

G4bool 
SHcal01::PreLoadScriptAction(Database* dbtmp,
			     CGAGeometryEnvironment& theGeometryEnvironment)
{
  G4String query;

  // upload the setup parameters
  query = "set  @fe_dim_y = " + 
    theGeometryEnvironment.GetParameterAsString("Hcal_radiator_thickness") + ";";
  dbtmp->exec(query.data());
  query = "set  @RadiatorMaterial = '" + 
    theGeometryEnvironment.GetParameterAsString("Hcal_radiator_material") + "';";
  dbtmp->exec(query.data());
  query = "set  @SensitiveModel =  '" + 
    theGeometryEnvironment.GetParameterAsString("Hcal_sensitive_model") + "';";
  dbtmp->exec(query.data());
  query = "set  @back_plate_thickness =  " + 
    theGeometryEnvironment.GetParameterAsString("Hcal_back_plate_thickness") + ";";
  dbtmp->exec(query.data());
  query = "set  @staves_gap =  " + 
    theGeometryEnvironment.GetParameterAsString("Hcal_stave_gaps") + ";";
  dbtmp->exec(query.data());
  query = "set  @modules_gap = " + 
    theGeometryEnvironment.GetParameterAsString("Hcal_modules_gap") + ";";
  dbtmp->exec(query.data());
  query = "set  @nlayers = " + 
    theGeometryEnvironment.GetParameterAsString("Hcal_nlayers") + ";";
  dbtmp->exec(query.data());
  query = "set @barrel_end_module_type = " +
    theGeometryEnvironment.GetParameterAsString("Hcal_barrel_end_module_type") + ";";
  dbtmp->exec(query.data());
  query = "set @fiber_gap = " +
    theGeometryEnvironment.GetParameterAsString("Hcal_fiber_gap") + ";";
  dbtmp->exec(query.data());



  // For the moment the chamber thickness cannot be changed because the
  // rpc1 table should follow.
  query = "set  @chamber_tickness = 6.5;";
  dbtmp->exec(query.data());
  query = "set  @inner_radius = " + 
    theGeometryEnvironment.GetParameterAsString("Ecal_outer_radius") + " + " + 
    theGeometryEnvironment.GetParameterAsString("Hcal_Ecal_gap") + ";";
  dbtmp->exec(query.data());
  query = "set  @cables_gap = " + 
    theGeometryEnvironment.GetParameterAsString("Hcal_endcap_cables_gap") + ";";
  dbtmp->exec(query.data());

  G4int end_module_type =
    theGeometryEnvironment.GetParameterAsInt("Hcal_barrel_end_module_type");
  if(end_module_type == 1 )
    {
      query = "set  @normal_dim_z = 2 * " + 
	theGeometryEnvironment.GetParameterAsString("TPC_Ecal_Hcal_barrel_halfZ") + " / 5. - 1.;"; 
      dbtmp->exec(query.data());
      query = "set  @top_end_dim_z = 1180.0000;";
      dbtmp->exec(query.data());
      query = "set @start_z =  2.5*@normal_dim_z + 2*@modules_gap + @cables_gap;";
     dbtmp->exec(query.data());

    }
  else
    {
      // the 140 and the proportions comes from Tesla TDR
      G4double total_z_size = 140 +
	2 * theGeometryEnvironment.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");
      G4double normal_dim_z = total_z_size / 5. * 1080./1120.;
      G4double top_end_dim_z = 
	(total_z_size - 3 * normal_dim_z)/2;
      G4cout << "regular barrel module z size = " << normal_dim_z
	     << ", end barrel z size = " << top_end_dim_z << G4endl;
      char buffer [100] ;
      sprintf(buffer,"set @normal_dim_z = %f;",normal_dim_z);
      dbtmp->exec(buffer);
      sprintf(buffer,"set @top_end_dim_z = %f;",top_end_dim_z);
      dbtmp->exec(buffer);     
      query = "set @start_z =  1.5*@normal_dim_z + @top_end_dim_z + 2*@modules_gap + @cables_gap;";
      dbtmp->exec(query.data());
    }

  //
  // Test start_z against Ecal_endcap_zmax to avoid overlaps
  G4String Ecal_endcap_zmax = theGeometryEnvironment.GetParameterAsString("Ecal_endcap_zmax");
  query = "set  @start_z = if(@start_z + 0.5 > ";
  query += Ecal_endcap_zmax;
  query += ", @start_z , ";
  query += Ecal_endcap_zmax + " + 0.5);";
  dbtmp->exec(query.data());

  query = "set  @lateral_plate_thickness = " + 
    theGeometryEnvironment.GetParameterAsString("Hcal_lateral_structure_thickness") + ";";
  dbtmp->exec(query.data());

  query = "set  @cell_dim_x = " + 
    theGeometryEnvironment.GetParameterAsString("Hcal_cells_size") + ";";
  dbtmp->exec(query.data());
  query = "set  @center_box_size = " + 
    theGeometryEnvironment.GetParameterAsString("Hcal_endcap_center_box_size") + ";";
  dbtmp->exec(query.data());


  // general calculated parameters
  query = "set @total_dim_y = @nlayers*(@fe_dim_y+@chamber_tickness) + @back_plate_thickness;";
  dbtmp->exec(query.data());
  query = "set @module_radius = @inner_radius + @total_dim_y;";
  dbtmp->exec(query.data());


  // y_dim2_for_x becomes calculed!
  //query = "set  @y_dim2_for_x = 213.9;";  
  query = "set  @y_dim2_for_x = (@module_radius - @module_radius*COS(PI()/8));";
  dbtmp->exec(query.data());

  query = "set @y_dim1_for_x = @total_dim_y - @y_dim2_for_x;";
  dbtmp->exec(query.data());
  query = "set @bottom_dim_x = 2.*@inner_radius*TAN(PI()/8.)-@staves_gap;";
  dbtmp->exec(query.data());
  query = "set @midle_dim_x = @bottom_dim_x + 2*@y_dim1_for_x*TAN(PI()/8.);";
  dbtmp->exec(query.data());
  query = "set @top_dim_x = @midle_dim_x-2*@y_dim2_for_x/TAN(PI()/8.);";
  dbtmp->exec(query.data());

  // the  y_dim1_for_z kept as the original value in TDR
  query = "set @y_dim1_for_z = 134.8;";
  dbtmp->exec(query.data());

  query = "set @y_dim2_for_z = (@top_end_dim_z-@normal_dim_z)*TAN(RADIANS(39.28));";
  dbtmp->exec(query.data());
  query = "set @y_dim3_for_z = @total_dim_y - @y_dim1_for_z - @y_dim2_for_z;";
  dbtmp->exec(query.data());
  query = "set @regular_chamber_dim_z = @normal_dim_z - 2*(@lateral_plate_thickness);";
  dbtmp->exec(query.data());
  query = "set @cell_dim_z =  @regular_chamber_dim_z / FLOOR(@regular_chamber_dim_z/@cell_dim_x);";
  dbtmp->exec(query.data());
  
  // table scratch to build the layers (not so beautiful but it works...)
  //
  query = "create table scratch (id TINYINT NOT NULL AUTO_INCREMENT PRIMARY KEY,dummy float);";
  dbtmp->exec(query.data());
  G4int N_Layers = theGeometryEnvironment.GetParameterAsInt("Hcal_nlayers");
  query = "insert scratch set dummy = 0;";
  for (G4int i = 0; i < N_Layers; i++)
    dbtmp->exec(query.data());

#ifdef MOKKA_GEAR
  MokkaGear* mokkaGearMgr = MokkaGear::getMgr() ;

  mokkaGearMgr->tmpParam.setDoubleVal(  "Hcal_digitization_tile_size" , 30. ) ;

  mokkaGearMgr->tmpParam.setDoubleVal(  "TPC_Ecal_Hcal_barrel_halfZ" , theGeometryEnvironment.GetParameterAsDouble(  "TPC_Ecal_Hcal_barrel_halfZ" ) ) ;

  mokkaGearMgr->tmpParam.setDoubleVal( "Hcal_lateral_structure_thickness" , theGeometryEnvironment.GetParameterAsDouble( "Hcal_lateral_structure_thickness" ) ) ;			      

  mokkaGearMgr->tmpParam.setDoubleVal( "Hcal_stave_gaps" , theGeometryEnvironment.GetParameterAsDouble( "Hcal_stave_gaps" ) ) ;

  mokkaGearMgr->tmpParam.setDoubleVal( "Hcal_back_plate_thickness" , theGeometryEnvironment.GetParameterAsDouble( "Hcal_back_plate_thickness" ) ) ;
  mokkaGearMgr->tmpParam.setIntVal( "Hcal_barrel_end_module_type" ,end_module_type  ) ;
#endif

  return true;
}
G4bool 
SHcal01::PostLoadScriptAction(Database*  dbtmp,
			     CGAGeometryEnvironment& )
{
  G4String query;
  // propagates the changes to Coil, if any.
  query = "select  y_dim1_for_x +  y_dim2_for_x as module_dim_y,";
  query += " y_dim1_for_x +  y_dim2_for_x +  inner_radius as Hcal_outer_radius,";
  query += " (y_dim1_for_x +  y_dim2_for_x +  inner_radius)/cos(pi()/16) as calorimeter_region_rmax,";
  query += " module_radius+80 as Hcal_R_max from barrel,barrel_module,endcap_standard_module;";
  dbtmp->exec(query.data());
  dbtmp->getTuple();
  (*Control::globalModelParameters)["Hcal_R_max"] =
    dbtmp->fetchString("Hcal_R_max");
  G4cout << "SHcal information: Hcal_outer_radius = "
	 << dbtmp->fetchString("Hcal_outer_radius")
	 << "\n                   module thickness = "
	 << dbtmp->fetchString("module_dim_y")
	 << "\n                   Hcal_R_max = "
    	 << dbtmp->fetchString("Hcal_R_max");

  
  //
  // The SHcal driver has the responsability to change the calorimeter region parameters.
  //
  G4String calorimeter_region_rmax;
  calorimeter_region_rmax = dbtmp->fetchString("calorimeter_region_rmax");
  
  dbtmp->exec("select abs(endcap_z_offset) + module_dim_z/2. as calorimeter_region_zmax from endcap,endcap_standard_module where endcap_id=1;");
  dbtmp->getTuple();
  (*Control::globalModelParameters)["calorimeter_region_rmax"] =
    calorimeter_region_rmax;
  (*Control::globalModelParameters)["calorimeter_region_zmax"] =
    dbtmp->fetchString("calorimeter_region_zmax");
  
  G4cout << "\n                   calorimeter_region_rmax = "
	 << calorimeter_region_rmax
	 << "\n                   calorimeter_region_zmax = "
	 << dbtmp->fetchString("calorimeter_region_zmax")
	 << G4endl;

  return true;    
}
