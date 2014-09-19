/*
 * Sit SuperDriver for Mokka
 */
// V.Saveliev for SiLC Collaboration
 
#include <assert.h>
 
#include "MySQLWrapper.hh"
#include "Control.hh"
#include "SSit02.hh"

#include "globals.hh"
#include "CGADefs.h"

INSTANTIATE(SSit02)

SSit02::SSit02() : VSuperSubDetectorDriver("SSit02") {}

SSit02::~SSit02() {}


G4bool SSit02::PreLoadScriptAction(Database* dbtmp,
			     CGAGeometryEnvironment& theGeometryEnvironment)
{
  G4String query;
 
    G4String dbName = theGeometryEnvironment.GetDBName();
      G4cout <<"... SSit02 - dbName: " << dbName << endl;
    G4double Scaling_Factor_r = 1.;
    G4double Scaling_Factor_z = 1.;
  
    G4double VXD_outer_radius = theGeometryEnvironment.GetParameterAsDouble("VXD_outer_radius");
    G4double VXD_length_r5 = theGeometryEnvironment.GetParameterAsDouble("VXD_length_r5");
      G4cout <<"...SSit02" <<"VXD_outer_radius, VXD_length_r5 : "  <<VXD_outer_radius<<" "<< VXD_length_r5 << G4endl;
  
  // Sit table

  query = "CREATE TABLE sit SELECT * FROM "+dbName+".sit;";
  dbtmp->exec(query.data());
  
  G4int numberOfLayers;
  query = "SELECT max(layer_id) as num FROM sit;";
  dbtmp->exec(query.data());

  dbtmp->getTuple();
  numberOfLayers = dbtmp->fetchInt("num");
  G4cout << "SSit02 reading " << numberOfLayers << " layers from database..." << G4endl;
 
  G4int* layer_id = new G4int[numberOfLayers];
  G4double* inner_radious = new G4double[numberOfLayers];
  G4double* half_z = new G4double[numberOfLayers];
  
  query = "SELECT * from sit;";
  dbtmp->exec(query.data());
  
  G4int i;
  for (i=0; i<numberOfLayers; i++)
  {
    dbtmp->getTuple();
    G4int layer_id = dbtmp->fetchInt("layer_id");
    
    assert (layer_id>0);

    inner_radious[layer_id-1] = dbtmp->fetchDouble("inner_radious");
    half_z[layer_id-1] = dbtmp->fetchDouble("half_z");
    
    inner_radious[layer_id-1] = inner_radious[layer_id-1]/Scaling_Factor_r;
      G4cout <<"...SSit02 : updating R_in "<< layer_id <<"to " << inner_radious[layer_id-1] << G4endl;
    half_z[layer_id-1] = half_z[layer_id-1]/Scaling_Factor_z;
      G4cout << "...SSit02 : updating Z_half "<< layer_id <<"to " << half_z[layer_id-1] << G4endl;
 }
  
  for (i=0; i<numberOfLayers; i++)
  {
    std::ostringstream tmpstr;
    tmpstr << "UPDATE sit SET inner_radious=";
    tmpstr << inner_radious[i] << " where layer_id=" << i+1 <<";";

    query = tmpstr.str();
    dbtmp->exec(query.data());
  }
   
  for (i=0; i<numberOfLayers; i++)
  {
    std::ostringstream tmpstr;
  
    tmpstr << "UPDATE sit SET half_z=";
    tmpstr << half_z[i] << " where layer_id=" << i+1 <<";";

    query = tmpstr.str();
    dbtmp->exec(query.data());
  }
  
  delete[] inner_radious;
  delete[] half_z;
     
  return true;
}

G4bool SSit02::PostLoadScriptAction(Database*, CGAGeometryEnvironment& )
{
  return true;    
}

