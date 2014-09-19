/*
 * TubeX00 SuperDriver for Mokka
 *
 * STubeX00.cc - v1.0 jan. 2007
 *
 * Version history:
 *   1.0 - first implementation
 *
 */
// M.Kapolka - first implementation jan 2007

#include "MySQLWrapper.hh"
#include "Control.hh"
#include "STubeX00.hh"

#include "globals.hh"
#include "CGADefs.h"


INSTANTIATE(STubeX00)

string double2str(double v)
{
	std::ostringstream s ;
	s << v;
	return s.str();
}

G4bool STubeX00::PreLoadScriptAction(Database* dbtmp, CGAGeometryEnvironment& theGeometryEnvironment)
{

  // copy all tables from the original database to the temporary database

  G4String query;
  G4String dbName = theGeometryEnvironment.GetDBName() ;

  query =  "CREATE TABLE parameters SELECT * FROM "+dbName+".parameters;" ;
  dbtmp->exec(query.data());

  G4double crossingAngle = theGeometryEnvironment.GetParameterAsDouble("TUBE_crossing_angle");

  if (!crossingAngle)
    query = "CREATE TABLE tube SELECT * FROM "+dbName+".tube;" ;
  else
    query = "CREATE TABLE tube SELECT * FROM "+dbName+".tube2;" ;
  dbtmp->exec(query.data());

  dbtmp->exec( ("UPDATE parameters SET value = " + double2str(crossingAngle) + " WHERE name = \"crossingAngle\";").data() );

  G4cout << "  STubeX00 : crossing angle is " << crossingAngle << endl;

  //lcal geometry info
  G4double z_begin = theGeometryEnvironment.GetParameterAsDouble("Lcal_z_begin");
  G4double   r_max = theGeometryEnvironment.GetParameterAsDouble("Lcal_outer_radius");
  G4double   r_min = theGeometryEnvironment.GetParameterAsDouble("Lcal_inner_radius");
  G4int    n_layer = theGeometryEnvironment.GetParameterAsInt("Lcal_n_layers");
  G4double tungsten_thickness= theGeometryEnvironment.GetParameterAsDouble("Lcal_tungsten_thickness");
  G4double support_thickness = theGeometryEnvironment.GetParameterAsDouble("Lcal_support_thickness");
  G4double silicon_thickness = theGeometryEnvironment.GetParameterAsDouble("Lcal_silicon_thickness");
  G4double layer_gap = theGeometryEnvironment.GetParameterAsDouble("Lcal_layer_gap");
  G4double z_end = z_begin + (tungsten_thickness + support_thickness + silicon_thickness + layer_gap)*(double)n_layer;

  //min inner radius of lcal to fit in lateral tubes
//  G4double alpha = crossingAngle * 0.5;
  G4double alpha = crossingAngle / 2.* mrad;
  G4double width = z_end - z_begin;

//  G4double r_inner_min = (width + z_begin * cos(alpha)) * tan(crossingAngle);
  G4double r_inner_min = (width + z_begin * cos(alpha )) * tan(2.*alpha);

  G4cout << "      r_inner_min = " << r_inner_min << G4endl;

  //the extra inner/outer radial extents offset due to lcal rotation
  G4double lcal_rot_extra_r = width * sin(alpha);

  G4cout << "      lcal_rot_extra_r = " << lcal_rot_extra_r << G4endl;

  //the extra z-axial extents offset due to lcal rotation
  G4double lcal_rot_extra_z = r_max * sin(alpha);

  G4cout << "      lcal_rot_extra_z = " << lcal_rot_extra_z << G4endl;

  //opening nagle for ftd
  G4double opening_angle = r_max/z_end;

  //save it for SFTD
  actual_opening_angle = opening_angle;

  //save for lcal
  //check if inner radius change is needed
  dbtmp->exec("SELECT * FROM tube WHERE name=\"lumcal_back\";");
  dbtmp->getTuple();
  G4double r = dbtmp->fetchDouble("rInnerStart");

  if ( (r_min - 0.5 - lcal_rot_extra_r) < r)
  {
	r = r_min - 0.5 - lcal_rot_extra_r;
	G4cout << "   ...change r_min of tube (lcal rotation) r = " << r << G4endl;
  }

  if (dbtmp->fetchDouble("zEnd") * tan(alpha) > r)
  {
    r_min = r_inner_min;
    G4cout << "    ...change r_min of lcal (lateral tubes crossAngle) r = " << r_min << G4endl; 
  }

//save for lcal
  actual_lcal_rin = r_min;

  G4double vxd_r = theGeometryEnvironment.GetParameterAsDouble("VXD_inner_radius");
  G4double thickness;

  dbtmp->exec("SELECT * FROM tube WHERE name=\"ip_inner_parallel\";");
  dbtmp->getTuple();
  thickness = dbtmp->fetchDouble("rOuterStart") - dbtmp->fetchDouble("rInnerStart");

  //ip_inner_parallel 0.5mm gap between pipe and vxd
  dbtmp->exec( ("UPDATE tube SET rOuterStart = " + double2str(vxd_r - 0.5) + " WHERE name = \"ip_inner_parallel\";").data() );

  dbtmp->exec( ("UPDATE tube SET rOuterEnd = " + double2str(vxd_r - 0.5) + " WHERE name = \"ip_inner_parallel\";").data() );

  dbtmp->exec( ("UPDATE tube SET rInnerStart = " + double2str(vxd_r - 0.5 - thickness) + " WHERE name = \"ip_inner_parallel\";").data() );

  dbtmp->exec( ("UPDATE tube SET rInnerEnd = " + double2str(vxd_r - 0.5 - thickness) + " WHERE name = \"ip_inner_parallel\";").data() );


  dbtmp->exec("SELECT * FROM tube WHERE name=\"lumcal_front\";");
  dbtmp->getTuple();
  width = dbtmp->fetchDouble("zEnd") - dbtmp->fetchDouble("zStart");
  r = dbtmp->fetchDouble("rInnerStart");

  //ip_outer_bulge
  dbtmp->exec("SELECT * FROM tube WHERE name=\"ip_outer_bulge\";");
  dbtmp->getTuple();
  thickness = dbtmp->fetchDouble("rOuterEnd") - dbtmp->fetchDouble("rInnerEnd");

  dbtmp->exec( ("UPDATE tube SET zEnd = " + double2str(z_begin - width - lcal_rot_extra_z) + " WHERE name = \"ip_outer_bulge\";").data() );

  dbtmp->exec( ("UPDATE tube SET rOuterEnd = " + double2str(r_max) + " WHERE name = \"ip_outer_bulge\";").data() );

  dbtmp->exec( ("UPDATE tube SET rInnerEnd = " + double2str(r_max - thickness) + " WHERE name = \"ip_outer_bulge\";").data() );

  //lumcal_front
  //0.5 cm gap between lcal and tube ?

  dbtmp->exec("SELECT * FROM tube WHERE name=\"lumcal_front\";");
  dbtmp->getTuple();
  r = dbtmp->fetchDouble("rInnerStart");

  dbtmp->exec("SELECT * FROM tube WHERE name=\"lumcal_wall\";");
  dbtmp->getTuple();
  thickness = dbtmp->fetchDouble("rOuterEnd") - dbtmp->fetchDouble("rInnerEnd");

  G4cout << "lumcal_wall thickness = " << thickness << G4endl;

  dbtmp->exec( ("UPDATE tube SET zStart = " + double2str(z_begin - width - lcal_rot_extra_z) + " WHERE name = \"lumcal_front\";").data() );

  dbtmp->exec( ("UPDATE tube SET zEnd = " + double2str(z_begin - lcal_rot_extra_z) + " WHERE name = \"lumcal_front\";").data() );

  dbtmp->exec( ("UPDATE tube SET rOuterStart = " + double2str(r_max) + " WHERE name = \"lumcal_front\";").data() );

  dbtmp->exec( ("UPDATE tube SET rOuterEnd = " + double2str(r_max) + " WHERE name = \"lumcal_front\";").data() );

  if ( (r_min - 0.5 - lcal_rot_extra_r) < r)
  {
    dbtmp->exec( ("UPDATE tube SET rInnerStart = " + double2str(r_min - 0.5 - lcal_rot_extra_r - thickness) + " WHERE name = \"lumcal_front\";").data() );

    dbtmp->exec( ("UPDATE tube SET rInnerEnd = " + double2str(r_min - 0.5 - lcal_rot_extra_r - thickness) + " WHERE name = \"lumcal_front\";").data() );
  }

  //lumcal_wall
  dbtmp->exec("SELECT * FROM tube WHERE name=\"lumcal_wall\";");
  dbtmp->getTuple();
  r = dbtmp->fetchDouble("rOuterStart");

  dbtmp->exec( ("UPDATE tube SET zStart = " + double2str(z_begin - lcal_rot_extra_z) + " WHERE name = \"lumcal_wall\";").data() );

  dbtmp->exec( ("UPDATE tube SET zEnd = " + double2str( z_end + lcal_rot_extra_z) + " WHERE name = \"lumcal_wall\";").data() );

  if ( (r_min - 0.5 - lcal_rot_extra_r) < r)
  {
    dbtmp->exec( ("UPDATE tube SET rOuterStart = " + double2str( r_min - 0.5 - lcal_rot_extra_r ) + " WHERE name = \"lumcal_wall\";").data() );

    dbtmp->exec( ("UPDATE tube SET rOuterEnd = " + double2str( r_min - 0.5 - lcal_rot_extra_r ) + " WHERE name = \"lumcal_wall\";").data() );

    dbtmp->exec( ("UPDATE tube SET rInnerStart = " + double2str(r_min - 0.5 - lcal_rot_extra_r - thickness) + " WHERE name = \"lumcal_wall\";").data() );

    dbtmp->exec( ("UPDATE tube SET rInnerEnd = " + double2str(r_min - 0.5 - lcal_rot_extra_r - thickness) + " WHERE name = \"lumcal_wall\";").data() );
  }

  //lumcal_back
  dbtmp->exec("SELECT * FROM tube WHERE name=\"lumcal_back\";");
  dbtmp->getTuple();
  r = dbtmp->fetchDouble("rInnerStart");

  dbtmp->exec( ("UPDATE tube SET zStart = " + double2str( z_end + lcal_rot_extra_z) + " WHERE name = \"lumcal_back\";").data() );

  if ( (r_min - 0.5 - lcal_rot_extra_r) < r)
  {
    dbtmp->exec( ("UPDATE tube SET rInnerStart = " + double2str(r_min - 0.5 - lcal_rot_extra_r - thickness) + " WHERE name = \"lumcal_back\";").data() );

    dbtmp->exec( ("UPDATE tube SET rInnerEnd = " + double2str(r_min - 0.5 - lcal_rot_extra_r - thickness) + " WHERE name = \"lumcal_back\";").data() );
  }

  return true;
}

G4bool STubeX00::PostLoadScriptAction(Database*  dbtmp, CGAGeometryEnvironment& )
{
  G4cout << "STubeX00 set the following geomerty parameters : " << endl;
  dbtmp->exec("SELECT * FROM tube;");
  while (dbtmp->getTuple())
  {
	G4cout << " " << dbtmp->fetchString("name") << " :  rIn1 = " << dbtmp->fetchDouble("rInnerStart");
	G4cout << " rOut1 = " << dbtmp->fetchDouble("rOuterStart") << " rIn2 = " << dbtmp->fetchDouble("rInnerEnd");
	G4cout << " rOut2 = " << dbtmp->fetchDouble("rOuterEnd") << " zStart = " << dbtmp->fetchDouble("zStart");
	G4cout << " zEnd = " << dbtmp->fetchDouble("zEnd") << endl;
  }

//pass the opening angle to others
  (*Control::globalModelParameters)["TUBE_opening_angle"] = double2str(actual_opening_angle);

//pass updated lcal rin
  (*Control::globalModelParameters)["Lcal_inner_radius"] = double2str(actual_lcal_rin);

  G4cout << "STubeX00 PostLoadScriptAction set opening angle "
	 << actual_opening_angle << G4endl;

  G4cout << "STubeX00 PostLoadScriptAction set LCAL rIn "
	 << actual_lcal_rin << G4endl;

  return true;    
}
