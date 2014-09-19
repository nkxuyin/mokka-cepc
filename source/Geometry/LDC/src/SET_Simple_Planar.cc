//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for Tesla -      *
//*                                                     *
//* For more information about Mokka, please, go to the *
//*                                                     *
//*  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
//*                                                     *
//* Mokka home page.                                    *
//*                                                     *
//*******************************************************

/*
 * SET_Simple_Planar.cc  Simple Cylinder based SET 
 */

// Feb 7th 2011, Steve Aplin 


#include "Control.hh"
#include "SET_Simple_Planar.hh"
#include "TRKSD02.hh"
#include "G4PVPlacement.hh"
#include "CGAGeometryManager.hh"


#include "G4Cons.hh"
#include "G4Box.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4RotationMatrix.hh"

#include "MySQLWrapper.hh"
#include <assert.h>

#include "globals.hh"
#include "CGADefs.h"
#include "UserInit.hh"

#ifdef MOKKA_GEAR
#include "gearimpl/GearParametersImpl.h"
#include "gearimpl/ZPlanarParametersImpl.h" 
#include "gearimpl/ZPlanarLayerLayoutImpl.h" 
#include "MokkaGear.h"
#include "G4EmCalculator.hh"
#include "G4ParticleTable.hh"
#endif 

#include <UTIL/BitField64.h>
#include <UTIL/ILDConf.h>


INSTANTIATE(SET_Simple_Planar)

G4bool SET_Simple_Planar::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog)

{
  
  
  G4PVPlacement *Phys ;
  G4VisAttributes *VisAtt ;
  
  
  db = new Database(env.GetDBName());

  // *********************
  //  Read and Store the Extended Reconstruction Parameters which are passed directly through to gear. Note others may be added below
  db->exec("select * from extended_reconstruction_parameters;");
  db->getTuple();
  
  _e_r_p.strip_width_mm  = db->fetchDouble("strip_width_mm")  *mm;
  _e_r_p.strip_length_mm = db->fetchDouble("strip_length_mm") *mm;
  _e_r_p.strip_pitch_mm  = db->fetchDouble("strip_pitch_mm")  *mm;
  _e_r_p.strip_angle_deg = db->fetchDouble("strip_angle_deg") *deg;
  
  // *********************

    
  //... db common_parameters
  db->exec("select * from global;");
  db->getTuple();
  
  // Sensitive Thickness  
  _sensitive_thickness = db->fetchDouble("sensitive_thickness") *mm;
  // Support Thickness
  _support_thickness = db->fetchDouble("support_thickness") *mm;  
  // Sensor Length
  _sensor_length = db->fetchDouble("sensor_length") *mm;
  _e_r_p.sensor_length_mm = _sensor_length;
  
  G4Material* air = CGAGeometryManager::GetMaterial("air");  
  _sensitiveMat = CGAGeometryManager::GetMaterial(db->fetchString("sensitive_mat"));  
  _supportMat = CGAGeometryManager::GetMaterial(db->fetchString("support_mat"));  
  
  
  // setup the encoder 
  UTIL::BitField64 encoder( ILDCellID0::encoder_string ) ; 
  
  encoder.reset() ;  // reset to 0
  
  encoder[ILDCellID0::subdet] = ILDDetID::NOTUSED ;
  encoder[ILDCellID0::side] = 0 ;
  encoder[ILDCellID0::layer]  = 0 ;
  encoder[ILDCellID0::module] = 0 ;
  encoder[ILDCellID0::sensor] = 0 ;
  int cellID0 = encoder.lowWord() ;
  
  //... The SET Sensitive detector
  double sensitive_threshold_KeV = db->fetchDouble("sensitive_threshold_KeV") * keV ;
  
  _theSETSD = 
  new TRKSD02("SET",
              _sensitive_thickness * mm 
              * sensitive_threshold_KeV ,
              10.0 * MeV);
  
  RegisterSensitiveDetector(_theSETSD);

  
  const G4double TPC_outer_radius = env.GetParameterAsDouble("TPC_outer_radius");
  const G4double TPC_Ecal_Hcal_barrel_halfZ = env.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");
//  const G4double Ecal_Tpc_gap = env.GetParameterAsDouble("Ecal_Tpc_gap");
  //  const G4double ECal_min_r = TPC_outer_radius + Ecal_Tpc_gap;

  
  db->exec("select * from set_layers;");
  db->getTuple();
  
  do 
      {      
        
        G4int layer_id = db->fetchInt("layer_id");
        
        G4double coverage_of_TPC_Ecal_Hcal_barrel(0);
        G4double half_z(0);
        G4double sensitive_distance_from_tpc(0);
        G4double sensitive_radius(0);
        G4double sensitive_inner_radius(0);
        G4double support_radius(0);
        G4int    n_ladders(0) ;
        G4double ladder_width(0) ;
        G4double ladder_clearance(0) ;
        G4int    faces_IP(0) ;

      
        sensitive_distance_from_tpc = db->fetchDouble("sensitive_distance_from_tpc") *mm;
        coverage_of_TPC_Ecal_Hcal_barrel = db->fetchDouble("coverage_of_TPC_Ecal_Hcal_barrel") *mm;

        
        G4double max_half_z = coverage_of_TPC_Ecal_Hcal_barrel * TPC_Ecal_Hcal_barrel_halfZ;
        
        int number_of_sensors_per_half = floor(max_half_z/_sensor_length);
        
        half_z = _sensor_length * number_of_sensors_per_half;
        
        sensitive_radius = TPC_outer_radius + sensitive_distance_from_tpc;
        
        n_ladders        = db->fetchInt("n_ladders") ;
        faces_IP         = db->fetchInt("faces_IP") ;
        ladder_clearance = db->fetchDouble("ladder_clearance") *mm;

        
        const G4double ladder_dphi = ( twopi / n_ladders ) * rad;

        sensitive_inner_radius = sensitive_radius - 0.5 * _sensitive_thickness;
        ladder_width = 2*(tan(ladder_dphi*0.5)*sensitive_inner_radius - ladder_clearance) ;
        
        if( faces_IP == 1 ){ // support is on the outside 
          support_radius = sensitive_radius + (0.5 * _sensitive_thickness) ;
          ladder_width = 2*(tan(ladder_dphi*0.5)*sensitive_inner_radius - ladder_clearance) ;
        }
        else{ // support is on the inside
          support_radius = sensitive_radius - (0.5 * _sensitive_thickness) - _support_thickness;
          ladder_width = 2*(tan(ladder_dphi*0.5)*support_radius - ladder_clearance) ;
        }

        
        
        SET_Layer layer_geom ;
        
        layer_geom.n_ladders = n_ladders;
        layer_geom.half_z = half_z ;
        layer_geom.sensor_length = _sensor_length;
        layer_geom.n_sensors_per_ladder = number_of_sensors_per_half * 2.0;
        layer_geom.sensitive_inner_radius = sensitive_radius - 0.5 * _sensitive_thickness;
        layer_geom.support_inner_radius = support_radius;
        layer_geom.ladder_width = ladder_width ;
        layer_geom.ladder_dphi = ladder_dphi;
        
        _SET_Layers.push_back(layer_geom) ;
        

        std::cout << "SET_Simple_Planar: Layer:" << layer_id
        << "\t half length = " << half_z
        << "\t sensor length = " << layer_geom.sensor_length
        << "\t n sensors per ladder = " << layer_geom.n_sensors_per_ladder
        << "\t min r sensitive = " << layer_geom.sensitive_inner_radius
        << "\t min r support = " << layer_geom.support_inner_radius
        << "\t n ladders = " << n_ladders
        << "\t ladder width = " << ladder_width
        << "\t ladder clearance = " << ladder_clearance
        << "\t ladder dphi = " << ladder_dphi
        << "\t sensitive mat = " << _sensitiveMat->GetName()
        << "\t support mat = " << _supportMat->GetName()
        << "\t faces_IP = " << faces_IP
        << std::endl;
        
        
        //************************************************************************************************
        
        // Geometric Values Established. Start Creating Volumes.
        
        std::stringstream name_base;
        
        name_base << "SET";
 
        std::stringstream name_enum;
        name_enum << layer_id;
        
        // create an enclosing ladder volume that will be placed in the world volume for every ladder
        
        G4Box *setLadderSolid
        = new G4Box(name_base.str()+"_LadderSolid_"+name_enum.str(),
                    ( _sensitive_thickness + _support_thickness ) / 2.0 ,
                    layer_geom.ladder_width / 2.0,
                    layer_geom.half_z);
        
        
        G4LogicalVolume *setLadderLogical 
        = new G4LogicalVolume(setLadderSolid,
                              air, 
                              name_base.str()+"_LadderLogical_"+name_enum.str() );
        
        
        // now create an envelope volume to represent the sensitive area, which will be divided up into individual sensors         
        
        G4Box *setSenEnvelopeSolid
        = new G4Box(name_base.str()+"_SenEnvelopeSolid_"+name_enum.str(),
                    ( _sensitive_thickness ) / 2.0 ,
                    layer_geom.ladder_width  / 2.0,
                    layer_geom.half_z);
        
        G4LogicalVolume *setSenEnvelopeLogical 
        = new G4LogicalVolume(setSenEnvelopeSolid,
                              _sensitiveMat, 
                              name_base.str()+"_SenEnvelopeLogical_"+name_enum.str());
        
        
        
        // create the sensor volumes and place them in the senstive envelope volume 
        
        G4Box *setSenSolid
        = new G4Box(name_base.str()+"_SenSolid_"+name_enum.str(),
                    ( _sensitive_thickness ) / 2.0 ,
                    layer_geom.ladder_width  / 2.0,
                    (layer_geom.sensor_length / 2.0) - 1.e-06*mm ); // added tolerance to avoid false overlap detection
        
        G4LogicalVolume *setSenLogical 
        = new G4LogicalVolume(setSenSolid,
                              _sensitiveMat, 
                              name_base.str()+"_SenLogical_"+name_enum.str());
        
        setSenLogical->SetSensitiveDetector(_theSETSD);
        
        
        for (int isensor=0; isensor < layer_geom.n_sensors_per_ladder ; ++isensor) {
          
          
          encoder.reset() ;  // reset to 0
          encoder[ILDCellID0::subdet] = ILDDetID::NOTUSED ;
          encoder[ILDCellID0::sensor] =  isensor+1;
          cellID0 = encoder.lowWord() ;
          
          double xpos = 0.0;
          double ypos = 0.0;
          double zpos = -layer_geom.half_z + (0.5*layer_geom.sensor_length) + (isensor*layer_geom.sensor_length) ;
          
          Phys =
          new G4PVPlacement(0,
                            G4ThreeVector(xpos, ypos, zpos),
                            setSenLogical,
                            name_base.str()+"_SenPhys_"+name_enum.str(),
                            setSenEnvelopeLogical,
                            false,
                            cellID0,
                            false);
          
        }
        
        
        
        
        
        VisAtt = new G4VisAttributes(G4Colour(0.,0.,0.75)); 
        //        VisAtt->SetForceWireframe(false);
        //VisAtt->SetForceSolid(true); 
        
        setSenLogical->SetVisAttributes(VisAtt);
        
        encoder.reset() ;  // reset to 0
        encoder[ILDCellID0::subdet] = ILDDetID::NOTUSED ;
        encoder[ILDCellID0::layer]  = layer_id ;
        cellID0 = encoder.lowWord() ;
        
        Phys =
        new G4PVPlacement(0,
                          G4ThreeVector( (-( _sensitive_thickness + _support_thickness ) / 2.0 + (_sensitive_thickness / 2.0) ), 
                                        0., 
                                        0.),
                          setSenEnvelopeLogical,
                          name_base.str()+"_SenEnvelopePhys_"+name_enum.str(),
                          setLadderLogical,
                          false,
                          cellID0,
                          false);
        
        
        
        // create support volume which will be placed in the enclosing ladder volume together with the senstive envelope volume
        
        G4Box *setSupSolid
        = new G4Box(name_base.str()+"_SupSolid_"+name_enum.str(),
                    ( _support_thickness ) / 2.0 ,
                    layer_geom.ladder_width / 2.0,
                    layer_geom.half_z);
        
        G4LogicalVolume *setSupLogical 
        = new G4LogicalVolume(setSupSolid,
                              _supportMat, 
                              name_base.str()+"_SupLogical_"+name_enum.str());
        
        
        VisAtt = new G4VisAttributes(G4Colour(1.,0.,0.0)); 
        //        VisAtt->SetForceWireframe(false);
        
        setSupLogical->SetVisAttributes(VisAtt);
        
        
        Phys =
        new G4PVPlacement(0,
                          G4ThreeVector( (-( _sensitive_thickness + _support_thickness ) / 2.0 + _sensitive_thickness + (_support_thickness / 2.0)   ), 
                                        0., 
                                        0.),
                          setSupLogical,
                          name_base.str()+"_SupPhys_"+name_enum.str(),
                          setLadderLogical,
                          false,
                          0,
                          false);
        
        
        for( int i = 0 ; i < n_ladders ; ++i ){
          
          std::stringstream ladder_enum;
          
          ladder_enum << i;
          
          G4RotationMatrix *rot = new G4RotationMatrix();
          rot->rotateZ( i * -ladder_dphi );
          
          // rotate by 180 degrees around z if facing away from the IP
          if( faces_IP == 0 ) rot->rotateZ( 180 * deg );
          
          encoder[ILDCellID0::subdet] = ILDDetID::SET ;
          encoder[ILDCellID0::layer]  = layer_id ;
          encoder[ILDCellID0::module] = i + 1 ;
          cellID0 = encoder.lowWord() ;  
          
          float dr = ( ( _sensitive_thickness + _support_thickness ) / 2.0 ) - ( _sensitive_thickness / 2.0 ) ;
          if( faces_IP == 0 ) dr = -dr;
          
          Phys =
          new G4PVPlacement(rot,
                            G4ThreeVector( (sensitive_radius+dr) * cos(i * ladder_dphi), 
                                          (sensitive_radius+dr) * sin(i * ladder_dphi), 
                                          0.),
                            setLadderLogical,
                            name_base.str()+"_LadderPhys_"+name_enum.str()+"_"+ladder_enum.str(),
                            worldLog,
                            false,
                            cellID0,
                            false);
          
        }
        

      } while(db->getTuple()!=NULL);
  
  // Closes Database connection
  delete db;
  db = 0;
  G4cout << "SET_Simple_Planar done.\n" << G4endl;
  
  return true;
}

#ifdef MOKKA_GEAR

void SET_Simple_Planar::GearSetup()
{
  
  
  G4double sensitive_RadLen = _sensitiveMat->GetRadlen();
  G4double support_RadLen  = _supportMat->GetRadlen();
  
  // get gear manager
  MokkaGear* gearMgr = MokkaGear::getMgr() ;
  
  gear::ZPlanarParametersImpl* setParams = 
  new gear::ZPlanarParametersImpl(1,      // type CCD
                                  0.0 ,   // shell inner radius
                                  0.0 ,   // shell outer radius
                                  0.0 ,   // shell half length
                                  0.0 ,   // shell gap
                                  0.0 ) ; // shell rad length
  

  // Add the extra geomteric information not included in the ZPlanarParametersImpl API
  
  setParams->setDoubleVal("sensor_length_mm",_e_r_p.sensor_length_mm / mm);
  
  // Add the extended_reconstruction_parameters

  setParams->setDoubleVal("strip_width_mm",_e_r_p.strip_width_mm / mm);
  setParams->setDoubleVal("strip_length_mm",_e_r_p.strip_length_mm / mm);
  setParams->setDoubleVal("strip_pitch_mm",_e_r_p.strip_pitch_mm / mm);
  setParams->setDoubleVal("strip_angle_deg",_e_r_p.strip_angle_deg / deg);

  
  int nLayers = _SET_Layers.size() ;
  
  std::vector<int> n_sensors_per_ladder;

  
  // add all layers
  for( int i = 0 ; i < nLayers; i++ ) {
    
    double sen_distance = _SET_Layers[i].sensitive_inner_radius ;
    double sup_distance = _SET_Layers[i].support_inner_radius ;
    
     n_sensors_per_ladder.push_back(_SET_Layers[i].n_sensors_per_ladder);
    
    setParams->addLayer( _SET_Layers[i].n_ladders , 
                        0.0 , // phi0
                        sup_distance  / mm,
                        0.0,  // offset  
                        _support_thickness / mm,
                        _SET_Layers[i].half_z / mm,
                        _SET_Layers[i].ladder_width / mm, 
                        support_RadLen / mm,
                        sen_distance / mm, 
                        0.0, // offset 
                        _sensitive_thickness / mm, 
                        _SET_Layers[i].half_z / mm, 
                        _SET_Layers[i].ladder_width / mm, 
                        sensitive_RadLen / mm) ;

  }

  gearMgr->setSETParameters( setParams ) ;
  setParams->setIntVals("n_sensors_per_ladder",n_sensors_per_ladder);

  
  //  gear::GearParametersImpl* gearParameters = new gear::GearParametersImpl;
  //
  //
  //  gearParameters -> setDoubleVals( "SETSensitiveLayerInnerRadius" , _sensitive_inner_radiusVec ) ;
  //  gearParameters -> setDoubleVals( "SETLayerHalfLength" , _half_zVec ) ;
  //  gearParameters -> setDoubleVal ( "SETSensitiveLayerThickness" , _sensitive_thickness ) ;
  //  gearParameters -> setDoubleVal ( "SETSupportLayerThickness" , _support_thickness ) ;
  //  gearParameters -> setDoubleVal ( "SETSensitiveLayer_dEdx" , sensitive_dEdx ) ;
  //  gearParameters -> setDoubleVal ( "SETSensitiveLayer_RadLen" , sensitive_RadLen ) ;
  //  gearParameters -> setDoubleVal ( "SETSupportLayer_dEdx" , support_dEdx ) ;
  //  gearParameters -> setDoubleVal ( "SETSupportLayer_RadLen" , support_RadLen ) ;
  //
  //  // Write gearParameters to GearMgr
  //  // Parameters for SET_Simple_Planar
  //  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  //  gearMgr->setGearParameters("SET_Simple_Planar", gearParameters ) ;
}

#endif
