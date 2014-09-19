/*
 * SET SuperDriver for Mokka
 */
// V.Saveliev for SiLC Collaboration
 
#include <assert.h>
 
#include "MySQLWrapper.hh"
#include "Control.hh"
#include "SSet01.hh"

#include "globals.hh"
#include "CGADefs.h"

INSTANTIATE(SSet01)

SSet01::SSet01() : VSuperSubDetectorDriver("SSet01") {}

SSet01::~SSet01() {}


G4bool SSet01::PreLoadScriptAction(Database* dbtmp,
			     CGAGeometryEnvironment& theGeometryEnvironment)
{
  G4String query;
 
    G4double Scaling_Factor_r = 1.;
    G4double Scaling_Factor_z = 1.;

    G4String dbName = theGeometryEnvironment.GetDBName();  
      G4cout << "...SSet01 - dbName: " << dbName << endl; 
 
    G4double TPC_inner_radius = theGeometryEnvironment.GetParameterAsDouble("TPC_inner_radius");
    G4double TPC_outer_radius = theGeometryEnvironment.GetParameterAsDouble("TPC_outer_radius");
    G4double TPC_Ecal_Hcal_barrel_halfZ = theGeometryEnvironment.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");
    //    G4double TPC_ECal_gap = theGeometryEnvironment.GetParameterAsDouble("TPC_ECal_gap");
       G4cout <<"...SSet01, TPC_R_out, TPC_Ecal_Hcal_barrel_halfZ : " 
	      << TPC_outer_radius <<" "<< TPC_Ecal_Hcal_barrel_halfZ << G4endl;

  query = "CREATE TABLE EST SELECT * FROM "+dbName+".EST;";
  dbtmp->exec(query.data());
  
  G4int numberOfLayers;
  query = "SELECT max(layer_id) as num FROM EST;";
  dbtmp->exec(query.data());

  dbtmp->getTuple();
  numberOfLayers = dbtmp->fetchInt("num");
  G4cout << "SSet01 reading " << numberOfLayers << " layers from database..." << G4endl;
  
  G4int* layer_id = new G4int[numberOfLayers];
  G4double* inner_radious = new G4double[numberOfLayers];
  G4double* half_z = new G4double[numberOfLayers];
  
  query = "SELECT * from EST;";
  dbtmp->exec(query.data());
  
  G4int i;
  for (i=0; i<numberOfLayers; i++)
  {
    dbtmp->getTuple();
    G4int layer_id = dbtmp->fetchInt("layer_id");
    
    //    assert (disknum>0);
    assert (layer_id>0);

    inner_radious[layer_id-1] = dbtmp->fetchDouble("inner_radious");
    half_z[layer_id-1] = dbtmp->fetchDouble("half_z");
    
    inner_radious[layer_id-1] = TPC_outer_radius + 7.5*mm +(layer_id-1)*5*mm;
      G4cout << "...SSet01 : updating R_in "<< layer_id <<" " << inner_radious[layer_id-1] << G4endl;
    half_z[layer_id-1] = TPC_Ecal_Hcal_barrel_halfZ; 
      G4cout << "...SSet01 : updating z dimentions "<< layer_id <<" to " << half_z[layer_id-1] <<G4endl;
 }
  
  for (i=0; i<numberOfLayers; i++)
  {
    std::ostringstream tmpstr;
    tmpstr << "UPDATE EST SET inner_radious=";
    tmpstr << inner_radious[i] << " where layer_id =" << i+1 <<";";

    query = tmpstr.str();
    dbtmp->exec(query.data());
  }

  for (i=0; i<numberOfLayers; i++)
  {
    std::ostringstream tmpstr;
    
    tmpstr << "UPDATE EST SET half_z=";
    tmpstr << half_z[i] << " where layer_id =" << i+1 <<";";

    query = tmpstr.str();
    dbtmp->exec(query.data());
  }
  
  delete[] inner_radious;
  delete[] half_z;
     
  return true;
}

G4bool SSet01::PostLoadScriptAction(Database*, CGAGeometryEnvironment& )
{
  return true;    
}

