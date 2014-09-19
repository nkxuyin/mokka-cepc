//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for ILC          *
//*                                                     *
//* For more information about Mokka, please, go to the *
//*                                                     *
//*  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
//*                                                     *
//* Mokka home page.                                    *
//*                                                     *
//*******************************************************
//
// $Id: SField01.cc,v 1.3 2005/12/13 13:54:34 adrian Exp $
// $Name: mokka-07-00 $
//
// SField01.cc
//
// History:  
//

#include "MySQLWrapper.hh"
#include "Control.hh"
#include "SField01.hh"

#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"

//AS
#include "G4PropagatorInField.hh"
//Used for displaying Numbers with best units
#include "G4UnitsTable.hh"
//AS END

#include "globals.hh"
#include "CGADefs.h"

#include <sstream>

INSTANTIATE(SField01)

G4bool 
SField01::PreLoadScriptAction(Database* dbtmp,
			     CGAGeometryEnvironment& theGeometryEnvironment)
{
  G4String query;

  // upload the setup parameters
  query =  "CREATE TABLE field_map (\n";
  query += "  zmin float default NULL,\n";
  query += "  zmax float default NULL,\n";
  query += "  rmin float default NULL,\n";
  query += "  rmax float default NULL,\n";
  query += "  mag_field_x float default NULL,\n";
  query += "  mag_field_y float default NULL,\n";
  query += "  mag_field_z float default NULL\n";
  query += ") ENGINE=ISAM PACK_KEYS=1;";
  dbtmp->exec(query.data());

  // insert field regions
  G4String FieldZMax = 
    theGeometryEnvironment.GetParameterAsString("TPC_Ecal_Hcal_barrel_halfZ") + " + " + 
    theGeometryEnvironment.GetParameterAsString("Coil_extra_size");
  G4String FieldRMiddle = 
    theGeometryEnvironment.GetParameterAsString("Hcal_R_max") + " + " +
    theGeometryEnvironment.GetParameterAsString("Coil_thickness") + " / 2.";
  G4String FieldRMax = 
    theGeometryEnvironment.GetParameterAsString("Yoke_barrel_inner_radius") + " + " +
    theGeometryEnvironment.GetParameterAsString("Yoke_thickness");
  
  // inside the coil :
  query = "INSERT INTO field_map VALUES (\n";
  query += "0.00, \n"; // zmin
  query +=  FieldZMax + ", \n"; // zmax
  query += "0.00, "; // rmin
  query += FieldRMiddle + ", \n"; // rmax
  query += "0., 0.,";
  query += theGeometryEnvironment.GetParameterAsString("Field_nominal_value") + ");"; // central field mag
  dbtmp->exec(query.data());

//Fixing bug reported by AS:
G4double Yoke_inner_radius = 
	theGeometryEnvironment.GetParameterAsDouble("Yoke_barrel_inner_radius");

G4double Yoke_outer_radius = Yoke_inner_radius + 
	theGeometryEnvironment.GetParameterAsDouble("Yoke_thickness");

G4double Coil_middle_radius =
	theGeometryEnvironment.GetParameterAsDouble("Hcal_R_max") + 
	theGeometryEnvironment.GetParameterAsDouble("Coil_thickness") / 2.;

G4double BField_inner = 
	theGeometryEnvironment.GetParameterAsDouble("Field_nominal_value");

G4double BField_in_yoke = 
	BField_inner * Coil_middle_radius * Coil_middle_radius /
	(Yoke_outer_radius * Yoke_outer_radius - 
		Yoke_inner_radius * Yoke_inner_radius);

G4double Yoke_BField_cutoff = 
	theGeometryEnvironment.GetParameterAsDouble("Yoke_BField_cutoff");

if(BField_in_yoke > Yoke_BField_cutoff) 
	BField_in_yoke = Yoke_BField_cutoff;

BField_in_yoke = -BField_in_yoke;

G4cout << "The magnetic field vector inside the Yoke volume is (0., 0.," << 
	BField_in_yoke << ")" << G4endl;

std::ostringstream BField_in_yoke_as_string;
BField_in_yoke_as_string << BField_in_yoke;

  // outside the coil :
  query = "INSERT INTO field_map VALUES (\n";
  query += "0.00, \n"; // zmin
  query += FieldZMax + ", \n"; // zmax
  query += theGeometryEnvironment.GetParameterAsString("Yoke_barrel_inner_radius") + ", \n"; // rmin
  query += FieldRMax + ",\n"; // rmax
  query += "0., 0.,";
  query += BField_in_yoke_as_string.str() + ");"; // yoke return field
  dbtmp->exec(query.data());


/*
  // outside the coil :
  query = "INSERT INTO field_map VALUES (\n";
  query += "0.00, \n"; // zmin
  query += FieldZMax + ", \n"; // zmax
  query += FieldRMiddle + ", \n"; // rmin
  query += FieldRMax + ",\n"; // rmax
  query += "0., 0.,";
  query += " - (4.0 / (" + FieldRMiddle + 
    ") * " + theGeometryEnvironment.GetParameterAsString("Yoke_thickness") +
    "));"; // yoke return field
  dbtmp->exec(query.data());
*/

  return true;
}

G4bool 
SField01::PostLoadScriptAction(Database* ,
			     CGAGeometryEnvironment& env)
{
//Inspired from AS: Limit the length of the steps in the region
G4double FieldPropagator_LargestAcceptableStep =
        env.GetParameterAsDouble("FieldPropagator_LargestAcceptableStep") * m;

if(FieldPropagator_LargestAcceptableStep > 0)
{
  G4PropagatorInField* propMgr = G4TransportationManager::GetTransportationManager()->GetPropagatorInField();

  //Largest acceptable Step is 1km by default
  G4cout << "Largest acceptable Step was " << G4BestUnit(propMgr->GetLargestAcceptableStep(),"Length") << G4endl;
  propMgr->SetLargestAcceptableStep( FieldPropagator_LargestAcceptableStep );  
  G4cout << "Largest acceptable Step is  " << G4BestUnit(propMgr->GetLargestAcceptableStep(),"Length") << G4endl;
}
//AS END
  return true;

}

