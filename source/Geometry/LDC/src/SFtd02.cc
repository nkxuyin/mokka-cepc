/*
 * FTD SuperDriver for Mokka
 *
 * V.Saveliev for SiLC Collaboration
 *
 */
 
#include <assert.h>
 
#include "MySQLWrapper.hh"
#include "Control.hh"
#include "SFtd02.hh"
#include "STube01.hh"

#include "globals.hh"
#include "CGADefs.h"

INSTANTIATE(SFtd02)

SFtd02::SFtd02() : VSuperSubDetectorDriver("SFtd02") {}

SFtd02::~SFtd02() {}

G4bool SFtd02::PreLoadScriptAction(Database* dbtmp,
			     CGAGeometryEnvironment& theGeometryEnvironment)
{
  G4String query;
 
  G4String dbName = theGeometryEnvironment.GetDBName();
    G4cout <<"...SFtd02 dbName: " << dbName << endl; 

  G4double tangent = theGeometryEnvironment.GetParameterAsDouble("TUBE_opening_angle");
    G4cout <<"...SFtd02 - got " << tangent <<" as the opening angle from STube01" << G4endl;
    
    //  G4double TPC_inner_radius = theGeometryEnvironment.GetParameterAsDouble("TPC_inner_radius");
  G4double TPC_Ecal_Hcal_barrel_halfZ = theGeometryEnvironment.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");
    G4cout <<"...SFtd02 TPC_Rin, TPC_Ecal_Hcal_barrel_halfZ : " <<TPC_Ecal_Hcal_barrel_halfZ <<G4endl;
 
  query = "CREATE TABLE common_parameters SELECT * FROM "+dbName+".common_parameters;";
   dbtmp->exec(query.data());
  query = "CREATE TABLE disk SELECT * FROM "+dbName+".disk;";
   dbtmp->exec(query.data());
  
  G4int numberOfDisks;
  query = "SELECT max(disk_number) as num FROM disk;";
  dbtmp->exec(query.data());
  dbtmp->getTuple();
  numberOfDisks = dbtmp->fetchInt("num");
  
  G4cout << "   SFtd02 reading " << numberOfDisks << " disks from database..." << G4endl;
  
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
    if(i==6) z_begin[disknum-1] = TPC_Ecal_Hcal_barrel_halfZ*0.85;
    if(i==5) z_begin[disknum-1] = TPC_Ecal_Hcal_barrel_halfZ*0.7;
    if(i==4) z_begin[disknum-1] = TPC_Ecal_Hcal_barrel_halfZ*0.55;
    if(i==3) z_begin[disknum-1] = TPC_Ecal_Hcal_barrel_halfZ*0.4;
    if(i==2) z_begin[disknum-1] = TPC_Ecal_Hcal_barrel_halfZ*0.23;
    if(i==1) z_begin[disknum-1] = TPC_Ecal_Hcal_barrel_halfZ*0.16;
    if(i==0) z_begin[disknum-1] = TPC_Ecal_Hcal_barrel_halfZ*0.1;

    in_r[disknum-1] = dbtmp->fetchDouble("inner_radious");
    out_r[disknum-1] = dbtmp->fetchDouble("outer_radious");
    
    if (in_r[disknum-1] < (z_begin[disknum-1]+supportLength)*tangent - supportThickness)
	{
	G4cout << "...SFtd02 : updating R_in for disk "<< disknum<<" from " << in_r[disknum-1] << " to ";
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

  for (i=0; i<numberOfDisks; i++)
  {
    std::ostringstream tmpstr;
    tmpstr << "UPDATE disk SET z_position=";
    tmpstr << z_begin[i] << " where disk_number=" << i+1 <<";";
    query = tmpstr.str();
    dbtmp->exec(query.data());
    G4cout <<"...SFtd02 : updating z_position to " <<z_begin[i] << G4endl;
  }
  
  delete[] in_r;
  delete[] out_r;
  delete[] z_begin;
     
  return true;
}

G4bool SFtd02::PostLoadScriptAction(Database*, CGAGeometryEnvironment& )
{
  return true;    
}

