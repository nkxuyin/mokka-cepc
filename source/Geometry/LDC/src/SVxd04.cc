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
// $Id: SVxd04.cc,v 1.1 february 2008 grandjean 
// $Name: mokka-06-07-patch02 $
//
// SVxd04.cc
//
// History:  
//- first implementation D.GRANDJEAN october 05
// modification february 2008 grandjean
#include "MySQLWrapper.hh"
#include "Control.hh"
#include "SVxd04.hh"

#include "globals.hh"
#include "CGADefs.h"
#include "G4UIcommand.hh" 

INSTANTIATE(SVxd04)
  
  G4bool 
SVxd04::PreLoadScriptAction(Database* dbtmp,
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

  // ------------------------- set the radius of the layer ----------------------------
  // set layer 1 radius to new value 
  query = "update layer set layer_radius = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_inner_radius") + " where id=1 ;" ;
  dbtmp->exec(query.data());

  // set layer 1 radius to new value 
  //   query = "update layer set layer_radius = " ; 
  //   query +=  theGeometryEnvironment.GetParameterAsString("VXD_radius_r1") + " where id=1 ;" ;
  //   dbtmp->exec(query.data());
  
  // set layer 2 radius to new value 
  query = "update layer set layer_radius = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_inner_radius") + " where id=2 ;" ;
  dbtmp->exec(query.data());
  
  // set layer 3 radius to new value 
  query = "update layer set layer_radius = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_radius_r3") + " where id=3 ;" ;
  dbtmp->exec(query.data());
  
  // set layer 4 radius to new value 
  query = "update layer set layer_radius = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_radius_r3") + " where id=4 ;" ;
  dbtmp->exec(query.data());
  
  // set layer 5 radius to new value 
  query = "update layer set layer_radius = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_radius_r5") + " where id=5 ;" ;
  dbtmp->exec(query.data());
  
  // if (strcmp(dbName,"vxd02_6l")==0||strcmp(dbName,"vxd01_dblayer")==0||strcmp(dbName,"vxd01_6l")==0)
  //{
  // set layer 6 radius to new value 
  query = "update layer set layer_radius = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_radius_r5") + " where id=6 ;" ;
  dbtmp->exec(query.data());
  G4cout << "SJA:TAG: " << query << G4endl;
  //}
  
  /*    std::cout << "nom de la base de donnees "<< dbName <<std::endl ; 
	if (strcmp(dbName,"vxd02_6l")==0||strcmp(dbName,"vxd01_dblayer")==0)
	{
	//set layer 6 radius to new value 
	query = "update layer set layer_radius = " ; 
	query +=  theGeometryEnvironment.GetParameterAsString("VXD_outer_radius") + " where id=6 ;" ;
	dbtmp->exec(query.data());
	std::cout << " option 6 couches "<< std::endl ; 
	}
	// if (strcmp(dbName,"VXD02"))
	else
	{
	// set layer 5 radius to new value 
	query = "update layer set layer_radius = " ; 
	query +=  theGeometryEnvironment.GetParameterAsString("VXD_outer_radius") + " where id=5 ;" ;
	dbtmp->exec(query.data());
	std::cout << " option 5 couches "<< std::endl ; 
	}
  */
  
  
  //------------------------ set the width of ladder for each layer ---------------------------
  
  // set layer 1 width to new value 
  query = "update layer set ladder_width = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_width_r1") + " where id=1 ;" ;
  dbtmp->exec(query.data());
  
  // set layer 2 width to new value 
  query = "update layer set ladder_width = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_width_r1") + " where id=2 ;" ;
  dbtmp->exec(query.data());
  
  // set layer 3 width to new value 
  query = "update layer set ladder_width = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_width_r3") + " where id=3 ;" ;
  dbtmp->exec(query.data());
  
  // set layer 4 width to new value 
  query = "update layer set ladder_width = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_width_r3") + " where id=4 ;" ;
  dbtmp->exec(query.data());
  
  // set layer 5 width to new value 
  query = "update layer set ladder_width = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_width_r5") + " where id=5 ;" ;
  dbtmp->exec(query.data());
  
  // if (strcmp(dbName,"vxd02_6l")==0||strcmp(dbName,"vxd01_dblayer")==0||strcmp(dbName,"vxd01_6l")==0)
  // {  // set layer 6 width to new value 
  query = "update layer set ladder_width = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_width_r5") + " where id=6 ;" ;
  dbtmp->exec(query.data());
  //}
  
  //------------------------set the length of the ladder for each layer ----------------------
  
  // set layer 1 length to new value 
  query = "update layer set ladder_length = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_length_r1") + " where id=1 ;" ;
  dbtmp->exec(query.data());
  
  // set layer 2 length to new value 
  query = "update layer set ladder_length = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_length_r1") + " where id=2 ;" ;
  dbtmp->exec(query.data());
  
  // set layer 3 length to new value 
  query = "update layer set ladder_length = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_length_r3") + " where id=3 ;" ;
  dbtmp->exec(query.data());
  
  // set layer 4 length to new value 
  query = "update layer set ladder_length = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_length_r3") + " where id=4 ;" ;
  dbtmp->exec(query.data());
  
  // set layer 5 length to new value 
  query = "update layer set ladder_length = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_length_r5") + " where id=5 ;" ;
  dbtmp->exec(query.data());
  
  // if (strcmp(dbName,"vxd02_6l")==0||strcmp(dbName,"vxd01_dblayer")==0||strcmp(dbName,"vxd01_6l")==0)
  // {  // set layer 6 length to new value 
  query = "update layer set ladder_length = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_length_r5") + " where id=6 ;" ;
  dbtmp->exec(query.data());
  //}
  
  query = "update layers_common_parameters set " ; 
  query +=  "layer_gap = " + theGeometryEnvironment.GetParameterAsString("VXD_layer_gap") + " ;" ; 
  dbtmp->exec(query.data());
  
  
  // set the thickness of the support layer 
  G4String vxd07tmp;
  vxd07tmp = G4UIcommand::ConvertToString(theGeometryEnvironment.GetParameterAsDouble("VXD_foam_spacer_thickness"));
  
  query = "update layers_common_parameters set " ; 
  query +=  "foam_spacer_thickness = " + vxd07tmp + " ;" ;
  dbtmp->exec(query.data());
  
  vxd07tmp = G4UIcommand::ConvertToString(theGeometryEnvironment.GetParameterAsDouble("VXD_flex_cable_thickness"));
  
  query = "update layers_common_parameters set " ; 
  query +=  "flex_cable_thickness = " + vxd07tmp + " ;" ;
  dbtmp->exec(query.data());
  
  vxd07tmp = G4UIcommand::ConvertToString(theGeometryEnvironment.GetParameterAsDouble("VXD_metal_traces_thickness"));
  
  query = "update layers_common_parameters set " ; 
  query +=  "metal_traces_thickness = " + vxd07tmp + " ;" ;
  dbtmp->exec(query.data());
  
  // set the material type of the support layer 
  query = "update layers_common_parameters set " ; 
  query +=  "foam_spacer_material = " + theGeometryEnvironment.GetParameterAsString("VXD_foam_spacer_material") + " ;" ; 
  dbtmp->exec(query.data());

  // set the thickness of the silicon active part 
  query = "update layers_common_parameters set " ; 
  query +=  "active_silicon_thickness = " +theGeometryEnvironment.GetParameterAsString("VXD_active_silicon_thickness") + " ;"  ; 
  dbtmp->exec(query.data());
  
  // set the thickness of the side band electronic  
  query = "update layers_common_parameters set " ; 
  query +=  "side_band_electronics_thickness = " + theGeometryEnvironment.GetParameterAsString("VXD_side_band_electronics_thickness")+ " ;"  ; 
  dbtmp->exec(query.data()); 
  
  // set the width of the side band electronic  
  query = "update layers_common_parameters set " ; 
  query +=  "side_band_electronics_width = " + theGeometryEnvironment.GetParameterAsString("VXD_side_band_electronics_width")+ " ;"  ; 
  dbtmp->exec(query.data()); 
  
  // set the thickness of the ladder end electronic 
  query = "update layers_common_parameters set " ; 
  query +=  "electronics_structure_thickness = " + theGeometryEnvironment.GetParameterAsString("VXD_end_ladd_electronics_thickness")+ " ;"  ; 
  dbtmp->exec(query.data()); 
  
  // set ladder end electronic option 
  query = "update layers_common_parameters set " ; 
  query +=  "end_ladd_electronics_option = " + theGeometryEnvironment.GetParameterAsString("VXD_end_ladd_electronics_option")+ " ;"  ; 
  dbtmp->exec(query.data()); 
  
  // set the side band electronic option
  query = "update layers_common_parameters set " ; 
  query +=  "side_band_electronics_option = " + theGeometryEnvironment.GetParameterAsString("VXD_side_band_electronics_option")+ " ;"  ; 
  dbtmp->exec(query.data()); 
  
  // set ladder end electronic lentgh 
  query = "update layers_common_parameters set " ; 
  query +=  "end_electronics_half_z = " + theGeometryEnvironment.GetParameterAsString("VXD_end_ladd_electronics_half_length")+ " ;"  ; 
  dbtmp->exec(query.data()); 
  
  // set sensitive side band electronic 
  query = "update layers_common_parameters set " ; 
  query +=  "active_side_band_electronics_option = " + theGeometryEnvironment.GetParameterAsString("VXD_active_side_band_electronics_option")+ " ;"  ; 
  dbtmp->exec(query.data()); 
  
  
  //set the cryostat option : 0 = no cryostat, 1= TESLA TDR cryostat
  query = "update cryostat set cryostat_option = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_cryostat_option")+ " ;"  ;
  dbtmp->exec(query.data());
  
  // set the position of beryllium disks around the beampipe for the layer 1 mechanical support after scalling 
  query = "update support_shell set endplate_inner_radius_L1 = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_inner_radius");
  query += " - 0.3 " ;
  query +=";" ;
  dbtmp->exec(query.data());
  
  query = "update support_shell set endplate_outer_radius_L1 = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_inner_radius");
  query += " + " ;
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_layer_gap");  
  query += " + 2.0 " ;
  query +=";" ;
  dbtmp->exec(query.data());
  
  // if (strcmp(dbName,"vxd02_6l")==0||strcmp(dbName,"vxd01_dblayer")==0||strcmp(dbName,"vxd01_6l")==0)
  // { 
  // set the beryllium shell support for the other layers after scalling
  query = "update support_shell set inner_radious = " ; 
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_radius_r5");
  query += " + " ;
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_layer_gap");  
  query += " + 5.0 " ;
  query +=";" ;
  dbtmp->exec(query.data());
  //}
  
  /*
  // if (strcmp(dbName,"vxd02_6l")==0||strcmp(dbName,"vxd01_dblayer")==0||strcmp(dbName,"vxd01_6l")==0)
  // {  // set the cryostat paramters after scalling
  query ="update cryostat set foam_inner_radious = " ;
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_radius_r5") ;
  query += " + " ;
  query +=  theGeometryEnvironment.GetParameterAsString("VXD_layer_gap");  
  query += " + 30.0 " ;
  query +=";" ;
  dbtmp->exec(query.data());
  //}
  */
  
  //set the scalling factor to update the radius and the number of ladders of the layer between 1 and 5 
  
  std::vector<int> layerID ;
  //  float ScaleFactor ;
  std::vector<float> foam_spacer_thickness;
  std::vector<float> flex_cable_thickness;
  std::vector<float> metal_traces_thickness;
  std::vector<float> active_thickness ;	
  std::vector<float> new_radius;
  std::vector<float> old_radius;
  std::vector<int> ladder_number;
  std::vector<float> ladder_width;
  std::vector<float> ladder_length;
  std::vector<float> ladder_length_tmp; 
  
  dbtmp->exec("select * from layers_common_parameters  ;") ;
  dbtmp->getTuple();
  foam_spacer_thickness.push_back(  dbtmp->fetchDouble("foam_spacer_thickness")  ) ;
  flex_cable_thickness.push_back(  dbtmp->fetchDouble("flex_cable_thickness")  ) ;
  metal_traces_thickness.push_back(  dbtmp->fetchDouble("metal_traces_thickness")  ) ;
  active_thickness.push_back(  dbtmp->fetchDouble("active_silicon_thickness")  ) ;
  
  dbtmp->exec("select * from support_shell  ;") ;
  dbtmp->getTuple();
  ladder_length_tmp.push_back(  dbtmp->fetchDouble("half_z") );
  
  query = "select * from layer  ;" ;
  dbtmp->exec(query.data());
  
  
  while( dbtmp->getTuple() != 0 ){
    
    layerID.push_back(  dbtmp->fetchInt("id")  ) ;
    new_radius.push_back(  dbtmp->fetchDouble("layer_radius"))   ;
    ladder_width.push_back(  dbtmp->fetchDouble("ladder_width"))   ;
    ladder_length.push_back(  dbtmp->fetchDouble("ladder_length"))   ;
    ladder_number.push_back(  dbtmp->fetchInt("nb_ladder"))   ;
  }
  
  query = "select * from " +dbName+".layer  ;" ;
  dbtmp->exec(query.data());
  
  while( dbtmp->getTuple() != 0 ){
    
    old_radius.push_back(dbtmp->fetchDouble("layer_radius"))   ;
  }
  
  
  
  //   ScaleFactor = (new_radius[layerID.size()-1]-new_radius[0])/(old_radius[layerID.size()-1]-old_radius[0]) ; 
  //    std::cout << " Scale factor : " << ScaleFactor << std::endl ;
  std::cout << " nombre de couches : " << layerID.size() << std::endl ;
  
  
  for( size_t i = 0  ; i <  layerID.size() ; i++ ){
    
    //     if (new_radius[0]!=old_radius[0] || new_radius[layerID.size()-1]!=old_radius[layerID.size()-1])
    //     {
    
    //     new_radius[i]=new_radius[0]+((old_radius[i]-old_radius[0])*ScaleFactor);

    // in the case of ladders overlapped per superlayer
    //ladder_number[i]=int(floor(pi/atan(ladder_width[i]/(new_radius[i]+foam_spacer_thickness[0]+flex_cable_thickness[0]+metal_traces_thickness[0]+active_thickness[0]))))+1;
    
    // in the case of ladders overlapped per layer
    ladder_number[i]=int(floor(pi/atan(ladder_width[i]/(new_radius[i]+foam_spacer_thickness[0]+flex_cable_thickness[0]+metal_traces_thickness[0]+active_thickness[0]))));

    
    if(ladder_length[i]>ladder_length_tmp[0])
      {
	ladder_length_tmp[0]=ladder_length[i];
      }
    
    //      }
    
    std::cout << " *********************************** " <<std::endl ;
    std::cout << " layer id : " << layerID[i] << std::endl ;
    std::cout << " new_radius : " << new_radius[i] << std::endl ;
    std::cout << " old_radius : " << old_radius[i] << std::endl ;
    std::cout << " ladder width : " << ladder_width[i] << std::endl ;
    std::cout << " ladder length : " << ladder_length[i] << std::endl ;
    std::cout << " foam spacer thickness : " << foam_spacer_thickness[0] << std::endl ;
    std::cout << " flex cable thickness : " << flex_cable_thickness[0] << std::endl ;
    std::cout << " metal traces thickness : " << metal_traces_thickness[0] << std::endl ;
    std::cout << " active thickness : " << active_thickness[0] << std::endl ;
    std::cout << " nb ladder : " << ladder_number[i] << std::endl ;
    std::cout << " longer ladder length : " << ladder_length_tmp[0] << std::endl ;
    
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
      std::ostringstream ladlength ;
      ladlength<<ladder_length_tmp[0] ; 
      
      query ="update support_shell set half_z  = ";
      query +=  ladlength.str() ;
      query +=";" ;
      dbtmp->exec(query.data());       
    
      
      
      
      
  return true;
}

G4bool 
SVxd04::PostLoadScriptAction(Database*  dbtmp,
			     CGAGeometryEnvironment& )
{
  
  //   G4String query;
  //   //
  //   // The VXD driver has the responsability to change the TPC  parameters.
  //   //
  
  dbtmp->exec("select layer_radius from layer where id=1 ;");
  dbtmp->getTuple();
  
  G4cout << "SVxd04 PostLoadScriptAction set radius of first layer to  " 
 	 << dbtmp->fetchDouble("layer_radius") 
	 << G4endl;
  
  dbtmp->exec("select layer_radius from layer where id=6 ;");
  dbtmp->getTuple();
  
  G4cout << "SVxd04 PostLoadScriptAction set radius of last layer to  " 
 	 << dbtmp->fetchDouble("layer_radius") 
	 << G4endl;
  
  dbtmp->exec("select layer_gap from layers_common_parameters ;");
  dbtmp->getTuple();
  
  G4cout << "SVxd04 PostLoadScriptAction the layer gap is   " 
 	 << dbtmp->fetchDouble("layer_gap") 
	 << G4endl;
  
  
  return true;    
}
