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
// $Id: STpc01.cc,v 1.4 2005/12/13 13:54:34 adrian Exp $
// $Name: mokka-07-00 $
//
// STpc01.cc
//
// History:  
//

#include "MySQLWrapper.hh"
#include "Control.hh"
#include "STpc01.hh"

#include "globals.hh"
#include "CGADefs.h"

INSTANTIATE(STpc01)

G4bool 
STpc01::PreLoadScriptAction(Database* dbtmp,
			    CGAGeometryEnvironment& theGeometryEnvironment)
{
  G4String query;
  
  // 
  // create the table
  query = "CREATE TABLE tpc (";
  query += "  id tinyint(4) NOT NULL default '0',\n";
  query += "    inner_radius float default NULL,\n";
  query += "    outer_radius float default NULL,\n";
  query += "    z_half_length float default NULL,\n";
  query += "    inner_wall_thickness float default NULL,\n";
  query += "    outer_wall_thickness float default NULL,\n";
  query += "    number_of_layers int(4) default NULL,\n";
  query += "    endplate_thickness float default NULL,\n";
  query += "    endplate_kapton_percent float default NULL,\n";
  query += "    endplate_cu_percent float default NULL,\n";
  query += "    endplate_air_percent float default NULL,\n";
  query += "    fch_inner_radius float default NULL,\n";
  query += "    fch_outer_radius float default NULL,\n";
  query += "    fch_thickness float default NULL,\n";
  query += "    inner_sensitive_radius float default NULL,\n";
  query += "    outer_sensitive_radius float default NULL,\n";
  query += "    PRIMARY KEY  (id)\n";
  query += "  ) ENGINE=ISAM PACK_KEYS=1;";
  dbtmp->exec(query.data());

  // download the main parameters
  query = "INSERT INTO tpc VALUES (0,\n";
  query += theGeometryEnvironment.GetParameterAsString("TPC_inner_radius") + ", \n";
  query += theGeometryEnvironment.GetParameterAsString("TPC_outer_radius") + ", \n";
  query += theGeometryEnvironment.GetParameterAsString("TPC_Ecal_Hcal_barrel_halfZ") + " - ";
  query += theGeometryEnvironment.GetParameterAsString("TPC_electronics_backend_thickness") + ", \n";
  query += theGeometryEnvironment.GetParameterAsString("TPC_inner_wall_thickness") + ", \n";
  query += theGeometryEnvironment.GetParameterAsString("TPC_outer_wall_thickness") + ", \n";
  query += "0., \n"; // number_of_layers become calculated
  query += theGeometryEnvironment.GetParameterAsString("TPC_electronics_backend_thickness") + ", \n";
  query += theGeometryEnvironment.GetParameterAsString("TPC_electronics_kapton_percent") + ", \n";
  query += theGeometryEnvironment.GetParameterAsString("TPC_electronics_cu_percent") + ", \n";
  query += theGeometryEnvironment.GetParameterAsString("TPC_electronics_air_percent") + ", \n";
  query += "0., \n"; // fch_inner_radius become calculated
  query += "0., \n"; // fch_outer_radius become calculated
  query += theGeometryEnvironment.GetParameterAsString("FCH_thickness") +", \n";
  query += "0., \n"; // inner_sensitive_radius become calculated
  query += "0.);";   // outer_sensitive_radius become calculated
  //  G4cout << query << G4endl;
  dbtmp->exec(query.data());
  
  // calculate the functionnal parameters (the constants came from tpc04 database).
  query = "update tpc set ";
  query += "inner_sensitive_radius = inner_radius + 66.,\n";
  query += "outer_sensitive_radius = outer_radius - 64.,\n"; 
  query += "fch_inner_radius  = inner_radius,\n";
  query += "fch_outer_radius  = outer_radius - 90.;";
  dbtmp->exec(query.data());
  query = "update tpc set ";
  query += "number_of_layers = FLOOR((outer_sensitive_radius - inner_sensitive_radius + 1) / 6.2);";
  dbtmp->exec(query.data());  

  return true;
}
G4bool 
STpc01::PostLoadScriptAction(Database*  dbtmp,
			     CGAGeometryEnvironment& )
{
  G4String query;
  //
  // The TPC driver has the responsability to change the tracker region parameters.
  //
  dbtmp->exec("select outer_radius, number_of_layers, z_half_length as tracker_region_zmax from tpc;");
  dbtmp->getTuple();
  (*Control::globalModelParameters)["tracker_region_rmax"] =
    dbtmp->fetchString("outer_radius");
  (*Control::globalModelParameters)["tracker_region_zmax"] =
    dbtmp->fetchString("tracker_region_zmax");

  G4cout << "STpc01 information : number_of_layers = " 
	 << dbtmp->fetchInt("number_of_layers") 
	 << G4endl;

  return true;    
}
