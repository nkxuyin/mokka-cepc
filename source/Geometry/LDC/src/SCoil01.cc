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
// $Id: SCoil01.cc,v 1.2 2005/12/13 13:54:34 adrian Exp $
// $Name: mokka-07-00 $
//
// SCoil01.cc
//
// History:  
//

#include "MySQLWrapper.hh"
#include "Control.hh"
#include "SCoil01.hh"

#include "globals.hh"
#include "CGADefs.h"

INSTANTIATE(SCoil01)

G4bool 
SCoil01::PreLoadScriptAction(Database* dbtmp,
			     CGAGeometryEnvironment& theGeometryEnvironment)
{
  G4String query;

  // upload the setup parameters
  query = "CREATE TABLE coil ("
    "inner_radius float default NULL,"
    "outer_radius float default NULL,"
    "half_z float default NULL"
    ") ENGINE=ISAM PACK_KEYS=1;";
  dbtmp->exec(query.data());
  query = "set  @Coil_inner_radius = ";
  query += theGeometryEnvironment.GetParameterAsString("Hcal_R_max");
  query += ";";
  dbtmp->exec(query.data());
  query = "set  @Coil_thickness = ";
  query += theGeometryEnvironment.GetParameterAsString("Coil_thickness");
  query += ";";
  dbtmp->exec(query.data());
  query = "set  @Coil_half_z = ";
  query += theGeometryEnvironment.GetParameterAsString("TPC_Ecal_Hcal_barrel_halfZ");
  query += " + ";
  query += theGeometryEnvironment.GetParameterAsString("Coil_extra_size");
  query += ";";
  dbtmp->exec(query.data());

  query = "INSERT INTO coil VALUES (@Coil_inner_radius,@Coil_inner_radius+@Coil_thickness,@Coil_half_z);";
  dbtmp->exec(query.data());

  return true;
}

G4bool 
SCoil01::PostLoadScriptAction(Database* dbtmp,
			      CGAGeometryEnvironment&)
{
  G4String query;
  // propagates the changes to Yoke, if any.
  dbtmp->exec("select  outer_radius + 350 as radius_limit,  half_z + 350 as z_limit,  half_z + 2250 as endcap_yoke_thickness from coil;");
  dbtmp->getTuple();
  (*Control::globalModelParameters)["Yoke_barrel_inner_radius"] =
    dbtmp->fetchString("radius_limit");

  (*Control::globalModelParameters)["Yoke_Z_start_endcaps"] =
    dbtmp->fetchString("z_limit");

  (*Control::globalModelParameters)["Yoke_Barrel_Half_Z"] =
    dbtmp->fetchString("endcap_yoke_thickness");

  G4cout << "SCoil information: Yoke_barrel_inner_radius = "
	 << dbtmp->fetchString("radius_limit")
	 << "\n                   Yoke_Z_start_endcaps = "
	 << dbtmp->fetchString("z_limit")
	 << "\n                   Yoke_Barrel_Half_Z = "
	 << dbtmp->fetchString("endcap_yoke_thickness")
	 << G4endl;
  return true;    

}
