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
// $Id: SVxd01.cc,v 1.0 
// $Name: mokka-07-00 $
//
// SVxd01.cc
//
// History:  
//- first implementation D.GRANDJEAN october 05

#include "MySQLWrapper.hh"
#include "Control.hh"
#include "SVxd01.hh"

#include "globals.hh"
#include "CGADefs.h"

INSTANTIATE(SVxd01)

G4bool 
SVxd01::PreLoadScriptAction(Database* dbtmp,
			    CGAGeometryEnvironment& theGeometryEnvironment)
{

  // copy all tables from the original database to the temporary database

  G4String query;
  G4String dbName = theGeometryEnvironment.GetDBName() ;

  query =  "CREATE TABLE cryostat SELECT * FROM "+dbName+".cryostat;" ;
  dbtmp->exec(query.data());

  query = "CREATE TABLE layer SELECT * FROM "+dbName+".layer;" ;
  dbtmp->exec(query.data());

  query = "CREATE TABLE layers_common_parameters SELECT * FROM "+dbName+".layers_common_parameters;" ;
  dbtmp->exec(query.data());

  query = "CREATE TABLE support_shell SELECT * FROM "+dbName+".support_shell;" ;
  dbtmp->exec(query.data());


  // scale the vxd detector according to 'global' parameters:

  // here we have to scale all layers according to VXD_inner_radius and VXD_outer_radius
  // .....
  // 


  // set layer 1 radius to new value 
  query = "update layer set layer_radius = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_inner_radius") + " where id=1 ;" ;
  dbtmp->exec(query.data());

  // set layer 5 radius to new value 
  query = "update layer set layer_radius = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_outer_radius") + " where id=5 ;" ;
  dbtmp->exec(query.data());

  // set the thickness of the support layer 
  query = "update layers_common_parameters set " ; 
  query +=  "support_structure_thickness = " + theGeometryEnvironment.GetParameterAsString("VXD_support_ladder_thickness") + " ;" ; 
  dbtmp->exec(query.data());

  // set the material type of the support layer 
  query = "update layers_common_parameters set " ; 
  query +=  "ladder_support_material = " + theGeometryEnvironment.GetParameterAsString("VXD_support_ladder_material") + " ;" ; 
  dbtmp->exec(query.data());

  // set the thickness of the silicon active part 
  query = "update layers_common_parameters set " ; 
  query +=  " active_silicon_thickness = " +theGeometryEnvironment.GetParameterAsString("VXD_active_silicon_thickness") + " ;"  ; 
  dbtmp->exec(query.data());

  // set the thickness of the electronic (side band or ladder end) 
  query = "update layers_common_parameters set " ; 
  query +=  " electronics_structure_thickness = " + theGeometryEnvironment.GetParameterAsString("VXD_end_electronics_thickness")+ " ;"  ; 
  dbtmp->exec(query.data()); 
 
  //set the cryostat option : 0 = no cryostat, 1= TESLA TDR cryostat
  query = "update cryostat set cryostat_option = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_cryostat_option")+ " ;"  ;
  dbtmp->exec(query.data());

 // set the position of beryllium disks around the beampipe for the layer 1 mechanical support after scalling 
  query = "update support_shell set endplate_inner_radius_L1 = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_inner_radius");
  query += " - 0.5 " ;
  query +=";" ;
  dbtmp->exec(query.data());

  query = "update support_shell set endplate_outer_radius_L1 = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_inner_radius");
  query += " +1.5 " ;
  query +=";" ;
  dbtmp->exec(query.data());


 // set the beryllium shell support for the other layers after scalling
  query = "update support_shell set inner_radious = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_outer_radius");
  query += " + 5.0 " ;
  query +=";" ;
  dbtmp->exec(query.data());

 // set the cryostat paramters after scalling
  query ="update cryostat set foam_inner_radious = " ;
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_outer_radius") ;
  query += " + 30.0 " ;
  query +=";" ;
  dbtmp->exec(query.data());

    query ="update cryostat set alu_skin_inner_radious = foam_inner_radious + foam_tickness ;" ;
    dbtmp->exec(query.data());       



  //set the scalling factor to update the radius and the number of ladders of the layer between 1 and 5 

  std::vector<int> layerID ;
  float ScaleFactor ;
  std::vector<float> support_thickness;
  std::vector<float> active_thickness ;	
  std::vector<float> new_radius;
  std::vector<float> old_radius;
  std::vector<int> ladder_number;
  std::vector<float> ladder_width;

  dbtmp->exec("select * from layers_common_parameters  ;") ;
  dbtmp->getTuple();
  support_thickness.push_back(  dbtmp->fetchDouble("support_structure_thickness")  ) ;
  active_thickness.push_back(  dbtmp->fetchDouble("active_silicon_thickness")  ) ;
      
 query = "select * from layer  ;" ;
 dbtmp->exec(query.data());
   
  while( dbtmp->getTuple() != 0 ){

    layerID.push_back(  dbtmp->fetchInt("id")  ) ;
    new_radius.push_back(  dbtmp->fetchDouble("layer_radius"))   ;
    ladder_width.push_back(  dbtmp->fetchDouble("ladder_width"))   ;
    ladder_number.push_back(  dbtmp->fetchInt("nb_ladder"))   ;
  }

 query = "select * from " +dbName+".layer  ;" ;
 dbtmp->exec(query.data());
   
  while( dbtmp->getTuple() != 0 ){

    old_radius.push_back(  dbtmp->fetchInt("layer_radius"))   ;
  }



   ScaleFactor = (new_radius[layerID.size()-1]-new_radius[0])/(old_radius[layerID.size()-1]-old_radius[0]) ; 
    std::cout << " Scale factor : " << ScaleFactor << std::endl ;
  

  for( size_t i = 0  ; i <  layerID.size() ; i++ ){
    
     if (new_radius[0]!=old_radius[0] || new_radius[layerID.size()-1]!=old_radius[layerID.size()-1])
     {
    
     new_radius[i]=new_radius[0]+((old_radius[i]-old_radius[0])*ScaleFactor);
     ladder_number[i]=int(floor(pi/atan(ladder_width[i]/(new_radius[i]+support_thickness[0]+active_thickness[0]))))+1;
      }
      
//    std::cout << " *********************************** " <<std::endl ;
//    std::cout << " layer id : " << layerID[i] << std::endl ;
//    std::cout << " new_radius : " << new_radius[i] << std::endl ;
//    std::cout << " old_radius : " << old_radius[i] << std::endl ;
//    std::cout << " nb ladder : " << ladder_number[i] << std::endl ;
    
      std::ostringstream nladder;
      nladder<<ladder_number[i];
      std::ostringstream radlayer;
      radlayer<<new_radius[i];
      std::ostringstream id ;
      id<<i+1;
    
    query ="update layer set nb_ladder = " ;
    query += nladder.str();
    query += " where id =" ;
    query +=  id.str() ;
    query +=";" ;
    dbtmp->exec(query.data()); 
 
    query ="update layer set layer_radius = " ;
    query += radlayer.str();
    query += " where id =" ;
    query +=  id.str() ;
    query +=";" ;
    dbtmp->exec(query.data()); 
   
  
  }





  return true;
}

G4bool 
SVxd01::PostLoadScriptAction(Database*  dbtmp,
			     CGAGeometryEnvironment& )
{

//   G4String query;
//   //
//   // The VXD driver has the responsability to change the TPC  parameters.
//   //

  dbtmp->exec("select layer_radius from layer where id=1 ;");
  dbtmp->getTuple();

  G4cout << "SVxd01 PostLoadScriptAction set radius of first layer to  " 
 	 << dbtmp->fetchDouble("layer_radius") 
	 << G4endl;

  dbtmp->exec("select layer_radius from layer where id=5 ;");
  dbtmp->getTuple();

  G4cout << "SVxd01 PostLoadScriptAction set radius of last layer to  " 
 	 << dbtmp->fetchDouble("layer_radius") 
	 << G4endl;



  return true;    
}
