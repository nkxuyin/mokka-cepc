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
// $Id: SEcal01.cc,v 1.6 2006/07/10 09:07:14 mora Exp $
// $Name: mokka-07-00 $
//
// SEcal01.cc
//
// History:  
//

#include "MySQLWrapper.hh"
#include "Control.hh"
#include "SEcal01.hh"

#include "globals.hh"
#include "CGADefs.h"

INSTANTIATE(SEcal01)

G4bool 
SEcal01::PreLoadScriptAction(Database* dbtmp,
			     CGAGeometryEnvironment& theGeometryEnvironment)
{
  G4String query;

  // upload the setup parameters
  query = "set @fiber_thickness = ";
  query += theGeometryEnvironment.GetParameterAsString("Ecal_fiber_thickness");
  query += ", @si_thickness = ";
  query += theGeometryEnvironment.GetParameterAsString("Ecal_Si_thickness");
  query += ", @alveolus_thickness = ";
  query += theGeometryEnvironment.GetParameterAsString("Ecal_total_alveolus_thickness");
  query += ", @radiator = '";
  query += theGeometryEnvironment.GetParameterAsString("Ecal_radiator_material");
  query += "', @inner_radius = ";
  query += theGeometryEnvironment.GetParameterAsString("TPC_outer_radius") + " + ";
  query += theGeometryEnvironment.GetParameterAsString("Ecal_Tpc_gap");
  query += ", @w_thickness1 = ";
  query += theGeometryEnvironment.GetParameterAsString("Ecal_radiator_layers_set1_thickness");
  query += ", @w_thickness2 = ";
  query += theGeometryEnvironment.GetParameterAsString("Ecal_radiator_layers_set2_thickness");
  query += ", @w_thickness3 = ";
  query += theGeometryEnvironment.GetParameterAsString("Ecal_radiator_layers_set3_thickness");
  query += ", @module_dim_z = 2 * ";
  query += theGeometryEnvironment.GetParameterAsString("TPC_Ecal_Hcal_barrel_halfZ") + " / 5. - 1."; 
  query += ", @cell_size =  ";
  query += theGeometryEnvironment.GetParameterAsString("Ecal_cells_size");
  query += ", @dist_barrel_endcap =  ";
  query += theGeometryEnvironment.GetParameterAsString("Ecal_cables_gap");
  query += ", @al_plates_center_dim_xy =  ";
  query += theGeometryEnvironment.GetParameterAsString("Ecal_endcap_center_box_size");
  query += ";";
  dbtmp->exec(query.data());

  // Calculed parameters, ecal03 special parameters just as constants for the moment, 
  // if in the future one want to run SEcal01 with ecal03, and less interesting parameters
  // left for the moment as just constants.

  query = "set @pcb_thickness = (@alveolus_thickness - @si_thickness)/2.,@staves_gap = 2., @modules_gap = 1.,";
  query += "@nmax_cell_x = 6, @nmax_cell_z = 6, @guard_ring_size = 1.,@inter_wafer_gap = 0.15,@n_guard_ring_zones = 3;";
  dbtmp->exec(query.data());

  query = "create table scratch  (id TINYINT NOT NULL AUTO_INCREMENT PRIMARY KEY,";
  query += "w_thickness FLOAT, w_thick1 FLOAT, w_thick2 FLOAT,";
  query += "w_thick3 FLOAT, module_dim_z FLOAT);";
  dbtmp->exec(query.data());

  G4int N_Layers1 = theGeometryEnvironment.GetParameterAsInt("Ecal_nlayers1");
  query = "insert scratch set w_thickness = @w_thickness1;";
  for (G4int i = 0; i < N_Layers1; i++)
    dbtmp->exec(query.data());
  G4int N_Layers2 = theGeometryEnvironment.GetParameterAsInt("Ecal_nlayers2");
  query = "insert scratch set w_thickness = @w_thickness2;";
  for (G4int i = 0; i < N_Layers2; i++)
    dbtmp->exec(query.data());

  G4int N_Layers3 = theGeometryEnvironment.GetParameterAsInt("Ecal_nlayers3");
  query = "insert scratch set w_thickness = @w_thickness3;";
  for (G4int i = 0; i < N_Layers3; i++)
    dbtmp->exec(query.data());

  query = "set @N_Layers1 = " + theGeometryEnvironment.GetParameterAsString("Ecal_nlayers1")
    + ";";
  dbtmp->exec(query.data());

  query = "set @N_Layers2 = " + theGeometryEnvironment.GetParameterAsString("Ecal_nlayers2")
    + ";";
  dbtmp->exec(query.data());
 
  return true;
}
G4bool 
SEcal01::PostLoadScriptAction(Database*  dbtmp,
			     CGAGeometryEnvironment& theGeometryEnvironment)
{
  // propagates the actual Ecal outer radius to the Hcal, if any. 
  G4String query,Ecal_outer_radius,Ecal_endcap_zmax,
    Ecal_endcap_zmin,module_dim_y;
  
  // Be careful: "Ecal_outer_radius is the outer radius in the middle of
  // the Ecal barrel module, not at corner. It co-works only with SHcal01!!!
  query = "select module_dim_y, inner_radius+module_dim_y";
  query += " as Ecal_outer_radius from barrel_standard_module,barrel;";
  dbtmp->exec(query.data());
  dbtmp->getTuple();

  Ecal_outer_radius = dbtmp->fetchString("Ecal_outer_radius");
  (*Control::globalModelParameters)["Ecal_outer_radius"] = Ecal_outer_radius;
  module_dim_y = dbtmp->fetchString("module_dim_y");

  //
  // sets Ecal_endcap_zmax global parameter to avoid Hcal endcap overlap 
  query = "select  endcap_z_offset+module_dim_z/2 as Ecal_endcap_zmax, ";
  query += " endcap_z_offset-module_dim_z/2 as Ecal_endcap_zmin";
  query += " from endcap,endcap_standard_module where endcap_id =1;";
  dbtmp->exec(query.data());
  dbtmp->getTuple();
  Ecal_endcap_zmax = dbtmp->fetchString("Ecal_endcap_zmax");
  (*Control::globalModelParameters)["Ecal_endcap_zmax"] = Ecal_endcap_zmax;
  Ecal_endcap_zmin = dbtmp->fetchString("Ecal_endcap_zmin");

  //
  // The Ecal driver has the responsability to change the tracker region parameters.
  //
  (*Control::globalModelParameters)["tracker_region_rmax"] =
    theGeometryEnvironment.GetParameterAsString("TPC_outer_radius");

  (*Control::globalModelParameters)["tracker_region_zmax"] =
    Ecal_endcap_zmin;
  
  G4cout << "SEcal information: Ecal_outer_radius = " << Ecal_outer_radius 
	 << "\n                  module thickness  = " << module_dim_y
	 << "\n                  Ecal_endcap_zmax = " << Ecal_endcap_zmax
	 << G4endl;
  return true;
}
