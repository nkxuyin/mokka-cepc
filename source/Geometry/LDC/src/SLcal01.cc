/*
 * LumiCal SuperDriver for Mokka
 *
 * SLcal01.cc - v1.0 Nov. 2006
 *
 * Version history:
 *   1.0 - first implementation
 *
 */
 // M.Kapolka - first implementation nov 2006

#include "MySQLWrapper.hh"
#include "Control.hh"
#include "SLcal01.hh"

#include "globals.hh"
#include "CGADefs.h"



INSTANTIATE(SLcal01)


SLcal01::SLcal01() : VSuperSubDetectorDriver("SLcal01")
{

}

SLcal01::~SLcal01()
{
}

G4bool SLcal01::PreLoadScriptAction(Database* dbtmp,
			     CGAGeometryEnvironment& theGeometryEnvironment)
{
  G4String query;

  G4cout << G4endl <<"SLcal01 - the LumiCal super driver v1.0" << G4endl;

////////// create the lumical table ////////////////

  query = "CREATE TABLE lumical (\n";
  query += "  type char(80) NOT NULL default '',\n";
  query += "  calo_inner_radius double default NULL,\n";
  query += "  calo_outer_radius double default NULL,\n";
  query += "  nstrips_theta int(3) default NULL,\n";
  query += "  nstrips_phi int(3) default NULL,\n";
  query += "  n_layers int(3) default NULL,\n";
  query += "  z_begin double default NULL,\n";
  query += "  phi_offset double default NULL,\n";
  query += "  tungsten_thickness double default NULL,\n";
  query += "  support_thickness double default NULL,\n";
  query += "  silicon_thickness double default NULL,\n";
  query += "  layer_gap double default NULL,\n";
  query += "  crossing_angle double default NULL\n";
  query += ") ENGINE=ISAM PACK_KEYS=1;";
  dbtmp->exec(query.data());

  //G4cout << "SLcal01 : temp table created " << G4endl;
////////////fill in the lumical table with values read from the geometry env/////////////

//preload env values to the db
  query = "set @type = '" + theGeometryEnvironment.GetParameterAsString("Lcal_type") + "',\n";
  query += " @calo_inner_radius = " + theGeometryEnvironment.GetParameterAsString("Lcal_inner_radius") + ",\n";  
  query += " @calo_outer_radius = " + theGeometryEnvironment.GetParameterAsString("Lcal_outer_radius") + ",\n";
  query += " @nstrips_theta = " + theGeometryEnvironment.GetParameterAsString("Lcal_nstrips_theta") + ",\n";
  query += " @nstrips_phi = " + theGeometryEnvironment.GetParameterAsString("Lcal_nstrips_phi") + ",\n";
  query += " @n_layers = " + theGeometryEnvironment.GetParameterAsString("Lcal_n_layers") + ",\n";
  query += " @z_begin = " + theGeometryEnvironment.GetParameterAsString("Lcal_z_begin") + ",\n";
  query += " @phi_offset = " + theGeometryEnvironment.GetParameterAsString("Lcal_phi_offset") + ",\n";
  query += " @tungsten_thickness = " + theGeometryEnvironment.GetParameterAsString("Lcal_tungsten_thickness") + ",\n";
  query += " @support_thickness = " + theGeometryEnvironment.GetParameterAsString("Lcal_support_thickness") + ",\n";
  query += " @silicon_thickness = " + theGeometryEnvironment.GetParameterAsString("Lcal_silicon_thickness") + ",\n";
  query += " @layer_gap = " + theGeometryEnvironment.GetParameterAsString("Lcal_layer_gap") + ",\n";
  query += " @crossing_angle = " + theGeometryEnvironment.GetParameterAsString("TUBE_crossing_angle") + ";";
  dbtmp->exec(query.data());
  
//  G4cout << "SLcal01 : " << query << G4endl;

//insert'em into the lumical table
  query = "INSERT INTO lumical VALUES (@type, ";
  query += "@calo_inner_radius, @calo_outer_radius, @nstrips_theta, ";
  query += "@nstrips_phi, @n_layers, @z_begin, @phi_offset, ";
  query += "@tungsten_thickness, @support_thickness, ";
  query += "@silicon_thickness, @layer_gap, @crossing_angle);";
   
  
  dbtmp->exec(query.data());


//  G4cout << "SLcal01::PreLoadScriptAction() done." << G4endl;
  return true;
}


G4bool SLcal01::PostLoadScriptAction(Database*  dbtmp,
			     CGAGeometryEnvironment& )
{

  //show what actually has been set
  dbtmp->exec("SELECT * FROM lumical;");
  dbtmp->getTuple();

  G4cout << "SLcal01 has set the following values :" << G4endl 
   	 << "    type               = " << dbtmp->fetchString("type") << G4endl
 	 << "    calo_inner_radius  = " << dbtmp->fetchDouble("calo_inner_radius") << G4endl
 	 << "    calo_outer_radius  = " << dbtmp->fetchDouble("calo_outer_radius") << G4endl
 	 << "    nstrips_theta      = " << dbtmp->fetchInt("nstrips_theta") << G4endl
 	 << "    nstrips_phi        = " << dbtmp->fetchInt("nstrips_phi") << G4endl
 	 << "    n_layers           = " << dbtmp->fetchInt("n_layers") << G4endl
 	 << "    z_begin            = " << dbtmp->fetchDouble("z_begin") << G4endl
 	 << "    phi_offset         = " << dbtmp->fetchDouble("phi_offset") << G4endl
 	 << "    tungsten_thickness = " << dbtmp->fetchDouble("tungsten_thickness") << G4endl
 	 << "    support_thickness  = " << dbtmp->fetchDouble("support_thickness") << G4endl
 	 << "    silicon_thickness  = " << dbtmp->fetchDouble("silicon_thickness") << G4endl
 	 << "    layer_gap          = " << dbtmp->fetchDouble("layer_gap") << G4endl
 	 << "    crossing_angle     = " << dbtmp->fetchDouble("crossing_angle") << G4endl
	 << G4endl;

  return true;    
}

