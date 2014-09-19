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
// $Id: SYoke01.cc,v 1.2 2005/12/13 13:54:34 adrian Exp $
// $Name: mokka-07-00 $
//
// SYoke01.cc
//
// History:  
//

#include "MySQLWrapper.hh"
#include "Control.hh"
#include "SYoke01.hh"

#include "globals.hh"
#include "CGADefs.h"

INSTANTIATE(SYoke01)

G4bool 
SYoke01::PreLoadScriptAction(Database* dbtmp,
			     CGAGeometryEnvironment& theGeometryEnvironment)
{
  G4String query;

  // upload the setup parameters
  query = "CREATE TABLE yoke (\n";
  query += "  outer_radius float default NULL,\n";
  query += "  barrel_inner_radius float default NULL,\n";
  query += "  endcap_inner_radius float default NULL,\n";
  query += "  barrel_half_z float default NULL,\n";
  query += "  ye1_outer_radius float default NULL,\n";
  query += "  ye1_half_z float default NULL,\n";
  query += "  ye1_z_center float default NULL\n";
  query += ") ENGINE=ISAM PACK_KEYS=1;";
  dbtmp->exec(query.data());


  query = "set @barrel_inner_radius = " + theGeometryEnvironment.GetParameterAsString("Yoke_barrel_inner_radius") + ",\n";
  query += " @endcap_inner_radius = " + theGeometryEnvironment.GetParameterAsString("Yoke_endcap_inner_radius") + ",\n";
  query += " @barrel_half_z = " + theGeometryEnvironment.GetParameterAsString("Yoke_Barrel_Half_Z") + ",\n";
  query += " @Z_start_endcaps = " + theGeometryEnvironment.GetParameterAsString("Yoke_Z_start_endcaps") + ";";
  //  G4cout << query << G4endl;
  dbtmp->exec(query.data());

  query = "set  @outer_radius = @barrel_inner_radius + ";
  query += theGeometryEnvironment.GetParameterAsString("Yoke_thickness") + ",\n";
  query += " @ye1_outer_radius = @barrel_inner_radius ,\n";
  query += " @ye1_half_z = 20, @ye1_z_center = @Z_start_endcaps - 10;";
  dbtmp->exec(query.data());

  
  query = "INSERT INTO yoke VALUES (@outer_radius,@barrel_inner_radius,@endcap_inner_radius,@barrel_half_z,@ye1_outer_radius,@ye1_half_z,@ye1_z_center);";
  dbtmp->exec(query.data());

  return true;
}
