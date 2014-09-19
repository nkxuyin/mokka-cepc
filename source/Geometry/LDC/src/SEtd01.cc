/*
 * ETD SuperDriver for Mokka
 */
// V.Saveliev for SiLC Collaboration
 
#include <assert.h>
 
#include "MySQLWrapper.hh"
#include "Control.hh"
#include "SEtd01.hh"

#include "globals.hh"
#include "CGADefs.h"

INSTANTIATE(SEtd01)

SEtd01::SEtd01() : VSuperSubDetectorDriver("SEtd01") {}

SEtd01::~SEtd01() {}

G4bool SEtd01::PreLoadScriptAction(Database* dbtmp,
			     CGAGeometryEnvironment& theGeometryEnvironment)
{
  G4String query;
 
    G4double Scaling_Factor_r = 1.;
    G4double Scaling_Factor_z = 1.;

    G4String dbName = theGeometryEnvironment.GetDBName();  
      G4cout <<"...SEtd01 dbName: " << dbName << G4endl; 
    G4double TPC_inner_radius = theGeometryEnvironment.GetParameterAsDouble("TPC_inner_radius");
    G4double TPC_outer_radius = theGeometryEnvironment.GetParameterAsDouble("TPC_outer_radius");
    //    G4double TPC_Ecal_Hcal_barrel_halfZ = theGeometryEnvironment.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");
    G4double Ecal_endcap_zmin = theGeometryEnvironment.GetParameterAsDouble("Ecal_endcap_zmin");
      G4cout <<"...SEtd01 TPC_Rin, TPC_Rout, Ecal_endcap_zmin : " 
      << TPC_inner_radius <<" "<< TPC_outer_radius <<" "<< Ecal_endcap_zmin <<G4endl;
  
  query = "CREATE TABLE ETD SELECT * FROM "+dbName+".ETD;";
  dbtmp->exec(query.data());
  
  G4int numberOfLayers;
  query = "SELECT max(layer_id) as num FROM ETD;";
  dbtmp->exec(query.data());

  dbtmp->getTuple();
  numberOfLayers = dbtmp->fetchInt("num");
  G4cout << "SEtd01 reading " << numberOfLayers << " layers from database..." << G4endl;
  
  G4int* layer_id = new G4int[numberOfLayers];
  G4double* inner_radious = new G4double[numberOfLayers];
  G4double* half_z = new G4double[numberOfLayers];
  
  query = "SELECT * from ETD;";
  dbtmp->exec(query.data());
  
  G4int i;
  for (i=0; i<numberOfLayers; i++)
  {
    dbtmp->getTuple();
      G4int layer_id = dbtmp->fetchInt("layer_id");
    assert (layer_id>0);

    inner_radious[layer_id-1] = dbtmp->fetchDouble("inner_radious");
    half_z[layer_id-1] = dbtmp->fetchDouble("half_z");
    
    inner_radious[layer_id-1] = TPC_inner_radius; 
      G4cout << "...SEtd01 : updating inner radius "<< layer_id <<" to " << inner_radious[layer_id-1] << G4endl;
      /*
      outer_radious[layer_id-1] = TPC_outer_radius;  
      G4cout << "...SEtd01 : updating inner radius "<< layer_id <<" to " << outer_radious[layer_id-1] << G4endl; 
      */

    half_z[layer_id-1] = Ecal_endcap_zmin - 20*mm; 
      G4cout << "...SEtd01 : updating z position "<< layer_id <<" to " << half_z[layer_id-1] << G4endl;
 }
  
  for (i=0; i<numberOfLayers; i++)
  {
    std::ostringstream tmpstr;
    tmpstr << "UPDATE ETD SET inner_radious=";
    tmpstr << inner_radious[i] << " where layer_id =" << i+1 <<";";
    query = tmpstr.str();
    dbtmp->exec(query.data());
  }
  /*
  for (i=0; i<numberOfLayers; i++) 
    { 
      std::ostringstream tmpstr; 
      tmpstr << "UPDATE ETD SET outer_radious="; 
      tmpstr << inner_radious[i] << " where layer_id =" << i+1 <<";"; 
      query = tmpstr.str(); 
      dbtmp->exec(query.data()); 
    } 
  */ 

  for (i=0; i<numberOfLayers; i++)
  {
    std::ostringstream tmpstr;    
    tmpstr << "UPDATE ETD SET half_z=";
    tmpstr << half_z[i] << " where layer_id =" << i+1 <<";";
    query = tmpstr.str();
    dbtmp->exec(query.data());
  }
  
  delete[] inner_radious;
  //  delete[] outer_radious; 
  delete[] half_z;
     
  return true;
}

G4bool SEtd01::PostLoadScriptAction(Database*, CGAGeometryEnvironment& )
{
  return true;    
}

