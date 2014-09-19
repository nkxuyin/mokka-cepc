/*
 * LumiCal SuperDriver for Mokka
 *
$Id: SLcal03.cc 23 2009-12-17 11:35:56Z bogdan $
 * SLcal01.cc - v1.0 Nov. 2006
 * SLcal02.cc - v1.1 Oct. 2008
 * SLcal03.cc - v1.3 Oct. 2009
 *
 * Version history:
 *   1.0 - first implementation   (M.Kapolka  Nov 2006)
 *   1.1 - dependence on Ecal_endcup_zmax implemented  (B.Pawlik Oct 2008) 
 *         sets Ecal_endcap_zmax if not set - to assure proper build of tubeX01
 *         must be build befor tubeX (!)
 *         asserts LCAL fits to ECAL endcap plug hole
 *   1.3 - new parameters tile gap, and extra size for support and electronics (b.p) 
 *
$LastChangeBy$
 */
#include <assert.h>
#include "MySQLWrapper.hh"
#include "Control.hh"
#include "SLcal03.hh"

#include "globals.hh"
#include "CGADefs.h"



INSTANTIATE(SLcal03)


SLcal03::SLcal03() : VSuperSubDetectorDriver("SLcal03")
{

}

SLcal03::~SLcal03()
{
}

G4bool SLcal03::PreLoadScriptAction(Database* dbtmp,
			     CGAGeometryEnvironment& theEnv)
{
  G4String query;

  G4cout << G4endl <<"SLcal03 - the LumiCal super driver v1.1" << G4endl;
  G4String dbName = theEnv.GetDBName() ;
  G4cout << " Database name : " << dbName << G4endl ;



////////// create the lumical table ////////////////

  query = "CREATE TABLE lumical (\n";
  query += "  type char(80) NOT NULL default '',\n";
  query += "  calo_inner_radius double default NULL,\n";
  query += "  calo_outer_radius double default NULL,\n";
  query += "  calo_extra_size double default NULL,\n";
  query += "  nstrips_theta int(3) default NULL,\n";
  query += "  nstrips_phi int(3) default NULL,\n";
  query += "  n_layers int(3) default NULL,\n";
  query += "  n_tiles int(3) default NULL,\n";
  query += "  z_begin double default NULL,\n";
  query += "  phi_offset double default NULL,\n";
  query += "  sensor_phi_offset double default NULL,\n";
  query += "  tungsten_thickness double default NULL,\n";
  query += "  support_thickness double default NULL,\n";
  query += "  silicon_thickness double default NULL,\n";
  query += "  layer_gap double default NULL,\n";
  query += "  tile_gap double default NULL,\n";
  query += "  crossing_angle double default NULL\n";
  query += ") ENGINE=ISAM PACK_KEYS=1;";
  dbtmp->exec(query.data());



////////////fill in the lumical table with values read from the geometry env/////////////

//preload env values to the db
  query = "set @type = '" + theEnv.GetParameterAsString("Lcal_type") + "',\n";
  query += " @calo_inner_radius = " + theEnv.GetParameterAsString("Lcal_inner_radius") + ",\n";  
  query += " @calo_outer_radius = " + theEnv.GetParameterAsString("Lcal_outer_radius") + ",\n";
  query += " @calo_extra_size = " + theEnv.GetParameterAsString("Lcal_extra_size") + ",\n";
  query += " @nstrips_theta = " + theEnv.GetParameterAsString("Lcal_nstrips_theta") + ",\n";
  query += " @nstrips_phi = " + theEnv.GetParameterAsString("Lcal_nstrips_phi") + ",\n";
  query += " @n_layers = " + theEnv.GetParameterAsString("Lcal_n_layers") + ",\n";
  query += " @n_tiles = " + theEnv.GetParameterAsString("Lcal_n_tiles") + ",\n";
  query += " @z_begin = " + theEnv.GetParameterAsString("Lcal_z_begin") + ",\n";
  query += " @phi_offset = " + theEnv.GetParameterAsString("Lcal_phi_offset") + ",\n";
  query += " @sensor_phi_offset = " + theEnv.GetParameterAsString("Lcal_sensor_phi_offset") + ",\n";
  query += " @tungsten_thickness = " + theEnv.GetParameterAsString("Lcal_tungsten_thickness") + ",\n";
  query += " @support_thickness = " + theEnv.GetParameterAsString("Lcal_support_thickness") + ",\n";
  query += " @silicon_thickness = " + theEnv.GetParameterAsString("Lcal_silicon_thickness") + ",\n";
  query += " @layer_gap = " + theEnv.GetParameterAsString("Lcal_layer_gap") + ",\n";
  query += " @tile_gap = " + theEnv.GetParameterAsString("Lcal_tile_gap") + ",\n";
  query += " @crossing_angle = " + theEnv.GetParameterAsString("ILC_Main_Crossing_Angle") + ";";
  dbtmp->exec(query.data());


  
//  G4cout << "SLcal03 : " << query << G4endl;

//insert'em into the lumical table
  query = "INSERT INTO lumical VALUES (@type, ";
  query += "@calo_inner_radius, @calo_outer_radius, @calo_extra_size, @nstrips_theta, ";
  query += "@nstrips_phi, @n_layers, @n_tiles, @z_begin, @phi_offset, ";
  query += "@sensor_phi_offset, @tungsten_thickness, @support_thickness, ";
  query += "@silicon_thickness, @layer_gap, @tile_gap, @crossing_angle);";
   
  
  dbtmp->exec(query.data());
  // make z_end position of LCAL aligned with ECAL endcup zmax if ECAL was build already;
 
  G4double  Ecal_endcap_zmax = theEnv.GetParameterAsDouble("Ecal_endcap_zmax");
  G4double  Ecal_endcap_zmin = theEnv.GetParameterAsDouble("Ecal_endcap_zmin");
  G4double  Lcal_length = (
                + theEnv.GetParameterAsDouble("Lcal_tungsten_thickness")
                + theEnv.GetParameterAsDouble("Lcal_support_thickness")
                + theEnv.GetParameterAsDouble("Lcal_silicon_thickness")
                + theEnv.GetParameterAsDouble("Lcal_layer_gap")
                          )*theEnv.GetParameterAsDouble("Lcal_n_layers"); 
  G4cout << " Lcal_length " << Lcal_length << G4endl ;
  std::ostringstream oss_lcalL;

  oss_lcalL <<  Lcal_length;
 (*Control::globalModelParameters)["Lcal_z_thickness"] =  oss_lcalL.str(); 

  G4double z_begin = Ecal_endcap_zmax - Lcal_length ;
   std::ostringstream oss_lcal_zmin ;
   oss_lcal_zmin <<  z_begin ; 

 if ( (int)Ecal_endcap_zmin  )  {

   assert ( z_begin >= Ecal_endcap_zmin );
   query = "UPDATE lumical SET z_begin =" + oss_lcal_zmin.str() + ";";
   dbtmp -> exec(query.data());
   (*Control::globalModelParameters)["Lcal_z_begin"] =  oss_lcal_zmin.str();
    G4cout << " Lcal_z_begin updated from " << theEnv.GetParameterAsString("Lcal_z_begin") 
        << " to " <<  oss_lcal_zmin.str() << " [ mm ] " << G4endl;
 }
 else {
    G4cout << " WARNING : Lcal_z_begin updated from " << theEnv.GetParameterAsString("Lcal_z_begin") 
           << " to " <<  oss_lcal_zmin.str() << " [ mm ] "
           << " Make sure You know what You are doing ! " 
           << G4endl;

    (*Control::globalModelParameters)["Lcal_z_begin"] =  oss_lcal_zmin.str();
      std::ostringstream oss_Ecalz ;
      oss_Ecalz <<  z_begin + Lcal_length ; 
    (*Control::globalModelParameters)["Ecal_endcap_zmax"] =  oss_Ecalz.str();

 };

  G4double  Ecal_endcap_plug_rmin = theEnv.GetParameterAsDouble("Ecal_endcap_plug_rmin");
  if ( (int)Ecal_endcap_plug_rmin ) {
    assert( Ecal_endcap_plug_rmin >= (theEnv.GetParameterAsDouble("Lcal_outer_radius")
				      + theEnv.GetParameterAsDouble("Lcal_extra_size")));
  };



//  G4cout << "SLcal03::PreLoadScriptAction() done." << G4endl;
  return true;
}


G4bool SLcal03::PostLoadScriptAction(Database*  dbtmp,
			     CGAGeometryEnvironment& )
{
   // export updated parameters to whom it concerns
  

  //show what actually has been set
  dbtmp->exec("SELECT * FROM lumical;");
  dbtmp->getTuple();

  G4cout << "SLcal03 has set the following values :" << G4endl 
  	 << "    crossing_angle     = " << dbtmp->fetchDouble("crossing_angle") << G4endl
  	 << "    type               = " << dbtmp->fetchString("type") << G4endl
 	 << "    z_begin            = " << dbtmp->fetchDouble("z_begin") << G4endl
 	 << "    calo_inner_radius  = " << dbtmp->fetchDouble("calo_inner_radius") << G4endl
 	 << "    calo_outer_radius  = " << dbtmp->fetchDouble("calo_outer_radius") << G4endl
 	 << "    calo_extra_size    = " << dbtmp->fetchDouble("calo_extra_size") << G4endl
	 << "    n_layers           = " << dbtmp->fetchInt("n_layers") << G4endl
	 << "    n_tiles            = " << dbtmp->fetchInt("n_tiles") << G4endl
  	 << "    nstrips_theta      = " << dbtmp->fetchInt("nstrips_theta") << G4endl
 	 << "    nstrips_phi        = " << dbtmp->fetchInt("nstrips_phi") << G4endl
 	 << "    phi_offset         = " << dbtmp->fetchDouble("phi_offset") << G4endl
 	 << "    sensor_phi_offset  = " << dbtmp->fetchDouble("sensor_phi_offset") << G4endl
 	 << "    tungsten_thickness = " << dbtmp->fetchDouble("tungsten_thickness") << G4endl
 	 << "    support_thickness  = " << dbtmp->fetchDouble("support_thickness") << G4endl
 	 << "    silicon_thickness  = " << dbtmp->fetchDouble("silicon_thickness") << G4endl
 	 << "    layer_gap          = " << dbtmp->fetchDouble("layer_gap") << G4endl
 	 << "    tile_gap           = " << dbtmp->fetchDouble("tile_gap") << G4endl
	 << G4endl;
  
  return true;    
}

