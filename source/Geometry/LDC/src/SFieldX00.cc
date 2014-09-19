/*
 * SFieldX00 SuperDriver for Mokka
 *
 * SFieldX00.cc - v1.0 feb 2007
 *
 * Version history:
 *   1.0 - first implementation
 *
 */
// M.Kapolka - first implementation feb 2007

#include "MySQLWrapper.hh"
#include "Control.hh"
#include "SFieldX00.hh"
#include "globals.hh"
#include "CGADefs.h"

INSTANTIATE(SFieldX00)

extern string double2str(double v); //in STubeX00

G4bool SFieldX00::PreLoadScriptAction(Database* dbtmp, CGAGeometryEnvironment& theGeometryEnvironment)
{

  G4String query;
  G4String dbName = theGeometryEnvironment.GetDBName() ;
  vector<G4String> tableNames;
  G4double crossingAngle = theGeometryEnvironment.GetParameterAsDouble("TUBE_crossing_angle");

  G4cout << "  SFieldX00 : crossing angle is " << crossingAngle << endl;

   // copy all tables from the original database to the temporary database
  query =  "CREATE TABLE parameters SELECT * FROM "+dbName+".parameters;" ;
  dbtmp->exec(query.data());

  if (!crossingAngle)
  {
  	query =  "CREATE TABLE magnetic SELECT * FROM "+dbName+".magnetic;" ;
        G4cout << "...using magnetic table \"magnetic\"" << endl;
  }
  else if (crossingAngle == 0.02)
  {
  	query =  "CREATE TABLE magnetic SELECT * FROM "+dbName+".magnetic2;" ;
        G4cout << "...using magnetic table \"magnetic2\"" << endl;
  }
  else
  {
	query =  "CREATE TABLE magnetic SELECT * FROM "+dbName+".magnetic14;" ;
        G4cout << "...using magnetic table \"magnetic14\"" << endl;
  }
  dbtmp->exec(query.data());

  //copy map tables (if any)
  query =  "SELECT * FROM magnetic;" ;
  dbtmp->exec(query.data());

  while( dbtmp->getTuple() )
  {
    int ft = dbtmp->fetchInt("fieldType");

    if (ft == 5 || ft == 6)
    {
       G4String str = dbtmp->fetchString("fieldData");

       tableNames.push_back( str );
    }
  }

  for (int i = 0; i < (int)tableNames.size(); i++)
  {
    G4String tableName = tableNames[i];

    query =  "CREATE TABLE "+tableName+" SELECT * FROM "+dbName+"."+tableName+";" ;
    dbtmp->exec(query.data());

    //get Bx at z=0 from map and move to magnetic.fieldValue
    if (!crossingAngle)
    {
      query =  "SELECT * FROM "+tableName+" WHERE z = 0;";
      dbtmp->exec(query.data());
      dbtmp->getTuple();
      double fld = dbtmp->fetchDouble("Bz");

      query =  "UPDATE magnetic SET fieldValue = "+double2str(fld)+" WHERE fieldData = \"" + tableName +"\";";
      dbtmp->exec(query.data());

      cout << "   SFieldX00 - moved Bx = " << fld << " @z = 0 from map to fieldData" << endl;
    }
  }

  // check for crossingAngle compliance / field types
  if (!crossingAngle)
  {
      query =  "UPDATE magnetic SET fieldType = 3 WHERE fieldType = 5;";
      dbtmp->exec(query.data());

      cout << "   SFieldX00 - changing field types from Solenoid-Map to Solenoid..." << endl;
  }

  // update crossingAngle in db
  query =  "UPDATE parameters SET value = " + double2str(crossingAngle) + " WHERE name = \"crossingAngle\";";
  dbtmp->exec(query.data());

  tableNames.clear();

  return true;
}

G4bool SFieldX00::PostLoadScriptAction(Database*  dbtmp, CGAGeometryEnvironment& )
{
  (void)dbtmp;

  return true;    
}
