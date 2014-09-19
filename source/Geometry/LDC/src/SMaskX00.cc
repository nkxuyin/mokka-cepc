/*
 * MaskX00 SuperDriver for Mokka
 *
 * SMaskX00.cc - v1.0 march 2007
 *
 * Version history:
 *   1.0 - first implementation
 *   1.1 - lhcal placing after lcal with a 5cm gap
 *
 */
// M.Kapolka - first implementation feb 2007

#include "MySQLWrapper.hh"
#include "Control.hh"
#include "SMaskX00.hh"

#include "globals.hh"
#include "CGADefs.h"

#define CALO_GAP 50.0 
// gap between lcal and lhcal in mm


INSTANTIATE(SMaskX00)

extern string double2str(double v);

G4bool SMaskX00::PreLoadScriptAction(Database* dbtmp, CGAGeometryEnvironment& theGeometryEnvironment)
{

  G4String query;
  G4String dbName = theGeometryEnvironment.GetDBName() ;
  vector <G4String> components;

  //lcal geometry info
  G4double z_begin = theGeometryEnvironment.GetParameterAsDouble("Lcal_z_begin");
  G4int    n_layer = theGeometryEnvironment.GetParameterAsInt("Lcal_n_layers");
  G4double tungsten_thickness= theGeometryEnvironment.GetParameterAsDouble("Lcal_tungsten_thickness");
  G4double support_thickness = theGeometryEnvironment.GetParameterAsDouble("Lcal_support_thickness");
  G4double silicon_thickness = theGeometryEnvironment.GetParameterAsDouble("Lcal_silicon_thickness");
  G4double layer_gap = theGeometryEnvironment.GetParameterAsDouble("Lcal_layer_gap");
  G4double z_end = z_begin + (tungsten_thickness + support_thickness + silicon_thickness + layer_gap)*(double)n_layer;

  G4cout << "  ...Lcal_z_end is " << z_end << G4endl;

  query =  "CREATE TABLE _parameters SELECT * FROM "+dbName+"._parameters;" ;
  dbtmp->exec(query.data());

  query =  "CREATE TABLE _components SELECT * FROM "+dbName+"._components;" ;
  dbtmp->exec(query.data());

  G4double crossingAngle = theGeometryEnvironment.GetParameterAsDouble("TUBE_crossing_angle");

  dbtmp->exec( ("UPDATE _parameters SET value = " + double2str(crossingAngle) + " WHERE name = \"crossingAngle\";").data() );

  G4cout << "  SMaskX00 : crossing angle is " << crossingAngle << endl;

  dbtmp->exec("SELECT * FROM _components;");
  while (dbtmp->getTuple())
  {
    G4String name = dbtmp->fetchString("name");
    components.push_back(name);
  }

  for (int i = 0; i < (int)components.size(); i++)
  {
    query = "CREATE TABLE " + components[i] + " SELECT * FROM " + dbName + "." + components[i];
    if (crossingAngle) query += "X";
    query += ";";
    
    dbtmp->exec( query.data() );

    if (components[i] == "lhcal")
    {

      G4cout << "    checking LHCAL..." << G4endl;

      query = "SELECT * FROM lhcal;";
      dbtmp->exec( query.data() );
      dbtmp->getTuple();

      G4double zstart = dbtmp->fetchDouble("zStart");
      G4double zend = dbtmp->fetchDouble("zEnd");
      G4double diff = zend - zstart;

      G4cout << "   ...start = " << zstart << "  end = " << zend << G4endl;

      if (diff > 0) // move lhcal
      {
        G4cout << "   ...moving LHCAL from " << zstart;

        zstart += diff;
        zend += diff;

        G4cout << " to " << zstart << G4endl;

        query = "UPDATE lhcal SET zStart=" + double2str(z_end + CALO_GAP)+";";
        dbtmp->exec( query.data() );

        query = "UPDATE lhcal SET zEnd=" + double2str(z_end + CALO_GAP + diff)+";";
        dbtmp->exec( query.data() );
      }

    }
  }

  components.clear();

  return true;
}

G4bool SMaskX00::PostLoadScriptAction(Database*  dbtmp, CGAGeometryEnvironment& )
{
  (void)dbtmp;

  return true;    
}
