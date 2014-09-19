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
// $Id: STube01.cc,v 1.0 
// $Name: mokka-07-00 $
//
// STube01.cc
//
// History:  
//- first implementation D.GRANDJEAN october 05

#include "MySQLWrapper.hh"
#include "Control.hh"
#include "STube01.hh"
//#include <math.h>
//#include <string.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <sstream>

#include "globals.hh"
#include "CGADefs.h"

INSTANTIATE(STube01)

G4bool 
STube01::PreLoadScriptAction(Database* dbtmp,
			    CGAGeometryEnvironment& theGeometryEnvironment)
{

  // copy all tables from the original database to the temporary database

  G4String query;
  G4String dbName = theGeometryEnvironment.GetDBName() ;

  query =  "CREATE TABLE central_tube SELECT * FROM "+dbName+".central_tube;" ;
  dbtmp->exec(query.data());

  query = "CREATE TABLE lateral_tubes SELECT * FROM "+dbName+".lateral_tubes;" ;
  dbtmp->exec(query.data());


  // scale the  beampipe according to 'global' parameters:

  // here we modifie the inner radius and the thickness of the central part of the beampipe
  // The inner radius of the beampipe is allways at 0.5 mm of the first layer of the VXD.
  // 


  query = "update central_tube set inner_radious = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_inner_radius") + " - " ;
  query +=  theGeometryEnvironment.GetParameterAsString("TUBE_central_thickness") ;
  query +=" - 0.5 ;";
  dbtmp->exec(query.data());

  query = "update central_tube set thickness = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("TUBE_central_thickness") + "  ;" ;
  dbtmp->exec(query.data());



  return true;
}

G4bool 
STube01::PostLoadScriptAction(Database*  dbtmp,
			     CGAGeometryEnvironment& )
{

//   G4String query;
//   //
//   // The TUBE driver has the responsability to change the VXD  parameters.
//   //

  dbtmp->exec("select * from central_tube ;");
  dbtmp->getTuple();

  G4cout << "STube01 PostLoadScriptAction set radius of the central tube  " 
 	 << dbtmp->fetchDouble("inner_radious") 
	 << G4endl;

  G4cout << "STube01 PostLoadScriptAction set thickness of the central tube  " 
 	 << dbtmp->fetchDouble("thickness") 
	 << G4endl;



  return true;    
}
