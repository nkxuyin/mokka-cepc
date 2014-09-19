/*
 * FTD SuperDriver for Mokka
 *
 * SFTD01.cc - v1.0 Dec. 2006
 *
 * Version history:
 *   1.0 - first implementation
 *
 */
// M.Kapolka - first implementation dec 2006
 
#include <assert.h>
 
#include "MySQLWrapper.hh"
#include "Control.hh"
#include "SFtd01.hh"
#include "STube01.hh"

#include "globals.hh"
#include "CGADefs.h"

INSTANTIATE(SFtd01)

SFtd01::SFtd01() : VSuperSubDetectorDriver("SFtd01") {}

SFtd01::~SFtd01() {}


G4bool SFtd01::PreLoadScriptAction(Database* dbtmp,
			     CGAGeometryEnvironment& theGeometryEnvironment)
{
  G4String query;
 
  G4String dbName = theGeometryEnvironment.GetDBName();
  
  G4double tangent = theGeometryEnvironment.GetParameterAsDouble("TUBE_opening_angle");

  G4cout << G4endl <<"SFtd01 - the FTD super driver v1.0" << G4endl;
  
  G4cout <<"   SFtd01 - got " << tangent <<" as the opening angle from STube01" << G4endl;
  
  query = "CREATE TABLE common_parameters SELECT * FROM "+dbName+".common_parameters;";
  dbtmp->exec(query.data());
  
  
  query = "CREATE TABLE disk SELECT * FROM "+dbName+".disk;";
  dbtmp->exec(query.data());
  
  G4int numberOfDisks;
  query = "SELECT max(disk_number) as num FROM disk;";
  dbtmp->exec(query.data());
  dbtmp->getTuple();
  numberOfDisks = dbtmp->fetchInt("num");
  
  G4cout << "   SFtd01 reading " << numberOfDisks << " disks from database..." << G4endl;
  
  G4double* in_r = new G4double[numberOfDisks];
  G4double* out_r = new G4double[numberOfDisks];
  G4double* z_begin = new G4double[numberOfDisks];
  G4double supportThickness;
  G4double supportLength;
  
  query = "SELECT * from common_parameters;";
  dbtmp->exec(query.data());
  dbtmp->getTuple();
  supportThickness = dbtmp->fetchDouble("inner_support_thickness");
  supportLength = dbtmp->fetchDouble("inner_support_length");  
   
  query = "SELECT * from disk;";
  dbtmp->exec(query.data());
  
  G4int i;
  for (i=0; i<numberOfDisks; i++)
  {
    dbtmp->getTuple();
    G4int disknum = dbtmp->fetchInt("disk_number");
    
    assert (disknum>0);

    z_begin[disknum-1] = dbtmp->fetchDouble("z_position");
    in_r[disknum-1] = dbtmp->fetchDouble("inner_radious");
    out_r[disknum-1] = dbtmp->fetchDouble("outer_radious");
    
    if (in_r[disknum-1] < (z_begin[disknum-1]+supportLength)*tangent - supportThickness)
	{
	G4cout << "   SFtd01 : updating inner radius for disk "<< disknum<<" from " << in_r[disknum-1] << " to ";
	in_r[disknum-1] = (z_begin[disknum-1]+supportLength)*tangent + supportThickness;
	G4cout << in_r[disknum-1] << G4endl;
	}
	
    assert( out_r[disknum-1] > in_r[disknum-1] );	
  }
  
  for (i=0; i<numberOfDisks; i++)
  {
    std::ostringstream tmpstr;
    tmpstr << "UPDATE disk SET inner_radious=";
    tmpstr << in_r[i] << " where disk_number=" << i+1 <<";";
    query = tmpstr.str();
    dbtmp->exec(query.data());
  }
  
  delete[] in_r;
  delete[] out_r;
  delete[] z_begin;
     
  return true;
}


G4bool SFtd01::PostLoadScriptAction(Database*, CGAGeometryEnvironment& )
{
  return true;    
}

