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
// $Id: SCoil02.cc,v 1.1 2008/10/05 18:33:39 frank Exp $
// $Name: mokka-07-00 $
//
// SCoil02.cc
//
// History:  
// F.Gaede, DESY:  based on SCoil01, with added parameters for the 
//                 clearance to the yoke:
//     Hcal_Coil_additional_gap  : adjust the actual gap in r (Hcal_R_max allready defines a gap)
//     Coil_Yoke_radial_clearance :  -> defines inner r of yoke
//     Coil_Yoke_lateral_clearance : -> defines zEndcap of yoke

#include "MySQLWrapper.hh"
#include "Control.hh"
#include "SCoil02.hh"

#include "globals.hh"
#include "CGADefs.h"


INSTANTIATE(SCoil02)

G4bool 
SCoil02::PreLoadScriptAction(Database* dbtmp,
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
  query += " + ";
  query += theGeometryEnvironment.GetParameterAsString("Hcal_Coil_additional_gap");
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
SCoil02::PostLoadScriptAction(Database* dbtmp,
			      CGAGeometryEnvironment& theGeometryEnvironment)
{
  G4String query;
  // propagates the changes to Yoke, if any.
  query = "set  @Coil_Yoke_radial_clearance = ";
  query += theGeometryEnvironment.GetParameterAsString("Coil_Yoke_radial_clearance");
  query += ";";
  dbtmp->exec(query.data());
  query = "set  @Coil_Yoke_lateral_clearance = ";
  query += theGeometryEnvironment.GetParameterAsString("Coil_Yoke_lateral_clearance");
  query += ";";
  dbtmp->exec(query.data());

  dbtmp->exec("select  outer_radius + @Coil_Yoke_radial_clearance  as radius_limit,  half_z + @Coil_Yoke_lateral_clearance as z_limit  from coil;");
  dbtmp->getTuple();
  (*Control::globalModelParameters)["Yoke_barrel_inner_radius"] =
    dbtmp->fetchString("radius_limit");

  (*Control::globalModelParameters)["Yoke_Z_start_endcaps"] =
    dbtmp->fetchString("z_limit");

  G4cout << "SCoil information: Yoke_barrel_inner_radius = "
	 << dbtmp->fetchString("radius_limit")
	 << "\n                   Yoke_Z_start_endcaps = "
	 << dbtmp->fetchString("z_limit")
	 << G4endl;
  return true;    

}
