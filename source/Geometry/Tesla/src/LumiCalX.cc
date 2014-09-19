
/*
$Id: LumiCalX.cc,v 1.5 2008/10/22 18:08:47 bogdan Exp $
*/
#include "Control.hh"
#include "LumiCalX.hh"

#ifdef MOKKA_GEAR
#include "gearimpl/CalorimeterParametersImpl.h"
//#include "gearimpl/LCALParametersImpl.h"
#include "gearimpl/LayerLayoutImpl.h"
#include "MokkaGear.h"
#include <math.h>
#endif

// define some levels of detail for graphical display
#define DM_FULL 3
#define DM_ABSORBERANDSENSITIVE 2
#define DM_WHOLELAYERONLY  1

//create an instance of class which is registered vs. the
//Mokka management system for the drivers 
INSTANTIATE (LumiCalX)

  LumiCalX::~LumiCalX() {/*no op*/}

G4bool LumiCalX::construct(const G4String &aSubDetectorDBName, G4LogicalVolume *WorldLog) 
{

  //Fetch database constants

  //open database (database opening routines provided by a MySQL wrapper
  //implemented in Mokka
  db = new Database(aSubDetectorDBName.data());
  
  //Set the data member WorldLogical
  WorldLogical = WorldLog;
  
  FetchdbEntries();
  BuildElements();
  G4bool cokay = Build_LumiCal();
  
  return cokay;
}

void LumiCalX::FetchdbEntries(){
  //fetch geometry constants 
  //provide access to database table block
  //
  db->exec("select * from lumical;");
  db->getTuple();

  //LumiCal dimensions
  cal_innerradius =db->fetchDouble("calo_inner_radius") ;   
  cal_outerradius =db->fetchDouble("calo_outer_radius") ;   
  assert(cal_innerradius > 0. && cal_innerradius < cal_outerradius);
 //number of strips in R direction
  ncell_theta = db->fetchInt("nstrips_theta");
  //number of cells in phi angle
  ncell_phi = db->fetchInt("nstrips_phi");
  assert(ncell_phi >= 1 && ncell_theta >= 1);
  n_layers = db->fetchInt("n_layers");
  assert(n_layers > 0);
  //beginning of calorimeter
  z_begin = db->fetchDouble("z_begin");
  //material thicknesses
  support_hthickness = db->fetchDouble("support_thickness")/2;
  tungsten_hthickness = db->fetchDouble("tungsten_thickness")/2;
  silicon_hthickness = db->fetchDouble("silicon_thickness")/2;
  layer_gap =db->fetchDouble("layer_gap") ;
  // beam crossing angle
  bx_angle = db->fetchDouble("crossing_angle")/2. * mrad; 
  //
  // calculate basic dimension   
  layer_hz = tungsten_hthickness+support_hthickness+silicon_hthickness;
  cal_hz = (G4double) (n_layers * (layer_hz+layer_gap/2.))*mm;
  cal_sphi = 0.*deg;
  cal_ephi = 360.*deg;
  thetastrip_dr = (G4double) ((cal_outerradius-cal_innerradius)/ncell_theta)*mm;
  phistrip_dphi = (G4double) (cal_ephi/ncell_phi);
  //
  /*
  G4cout << " LumiCal Zstart : "<< z_begin << " mm " << G4endl;
  G4cout << " LumiCal Zend   : "<< z_begin+2.*cal_hz << " mm " << G4endl;
  G4cout << " LumiCal Rmin   : "<< cal_innerradius << " mm" << G4endl;
  G4cout << " LumiCal Rmax   : "<< cal_outerradius << " mm" << G4endl;
  G4cout << " LumiCal length : "<< 2.*cal_hz << " mm" << G4endl;
  G4cout << " LumiCal #of rings     : "<< ncell_theta<< G4endl;
  G4cout << " LumiCal #of sectors   : "<< ncell_phi<< G4endl;
  G4cout << " Theta cell size : "<< thetastrip_dr<< " mm" << G4endl;
  G4cout << " Phi cell size   : "<< phistrip_dphi<< " rad" << G4endl;
  G4cout << " Phi angle from  : "<< cal_sphi<< " to "<< cal_ephi<<" rad" << G4endl;
  */

#ifdef MOKKA_GEAR
  
//   gear::LCALParametersImpl* lcalParams = new gear::LCALParametersImpl( 
// 	theGeometryEnvironment.GetParameterAsDouble("Lcal_inner_radius"), 
// 	theGeometryEnvironment.GetParameterAsDouble("Lcal_outer_radius"),
// 	theGeometryEnvironment.GetParameterAsDouble("Lcal_z_begin"),
// 	theGeometryEnvironment.GetParameterAsDouble("Lcal_phi_offset"),
// 	theGeometryEnvironment.GetParameterAsDouble("TUBE_crossing_angle"));

//   cout << "gear::LCALParametersImpl->getBeamCrossingAngle() = " << lcalParams->getBeamCrossingAngle() << endl;
//   cout << "gear::LCALParametersImpl->getPhi0() = " << lcalParams->getPhi0() << endl;

//fg: switched to normal CalorimeterParameters for LCAL
  gear::CalorimeterParametersImpl * lcalParams = 
    new gear::CalorimeterParametersImpl(cal_innerradius,
					cal_outerradius,
					z_begin,
					1, 
					db->fetchInt("phi_offset") );

  //fg: change this back to 14 mrad
  double bx_angle_mrad =  db->fetchDouble("crossing_angle") ; 
  lcalParams->setDoubleVal("beam_crossing_angle", bx_angle_mrad  ) ;

  cout << "gear: lcalParameters-  BeamCrossingAngle = " << lcalParams->getDoubleVal("beam_crossing_angle" ) << endl;
  cout << "gear: lcalParameters Phi0 = " << lcalParams->getPhi0() << endl;

  G4double l_thickness = tungsten_hthickness*2 + support_hthickness*2 +
    silicon_hthickness*2 + layer_gap;

  G4double a_thickness = tungsten_hthickness*2;

  G4double cellsize0 = (cal_outerradius - cal_innerradius) / ncell_theta;

  G4double cellsize1 = M_PI * 2.0 / ncell_phi;

  for (int i = 0; i < n_layers; i++)
     lcalParams->layerLayout().addLayer(l_thickness, cellsize0, cellsize1, a_thickness);
  
  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setLcalParameters( lcalParams ) ;

#endif

 
}


void LumiCalX::BuildElements() {


  // create and register SD
  SetSD();


  //Set displayMode 
  //  int displayMode = UserInit::getInstance()->getInt("DisplayMode") ;
  // if( displayMode == 0 )  // if nothing specified display full details
    G4int displayMode = DM_FULL ;
  
  G4cout << " using display mode : " << displayMode << G4endl ;
  
  //Create logical volumes 

  //an air tube which will hold all layers
  // whole LumiCal  
  air = CGAGeometryManager::GetMaterial("air");
  G4Tubs *WholeLumiCalSolid = new G4Tubs("WholeLumiCalSolid",
					cal_innerradius,
					cal_outerradius,
					cal_hz,
					cal_sphi, cal_ephi);

  WholeLumiCalLogical = new G4LogicalVolume( WholeLumiCalSolid,
					     air,
					     "WholeLumiCalLogical",
					     0,
					     0,
					     0);

  //an air box which will hold the layer components   
  //whole theta layer
  G4Tubs *LayerSolid = new G4Tubs("LayerSolid",
				      cal_innerradius,
				      cal_outerradius,
				      layer_hz,
				      cal_sphi, cal_ephi);
  
  LayerLogical = new G4LogicalVolume(LayerSolid,
                                          air,
                                          "LayerLogical",
                                          0,
                                          0,
                                          0);
  
#ifdef NO_VIS_LUMICAL
  WholeLumiCalLogical->SetVisAttributes(G4VisAttributes::Invisible);
  LayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#else
  G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(1.,1.,0));
  VisAtt->SetDaughtersInvisible(true);
  //VisAtt->SetForceSolid(true);
  WholeLumiCalLogical->SetVisAttributes(VisAtt);
#endif
  
  
  
  //logical volumes for the absorber and the scintillator to be filled 
  //into the logical volume of the layer as defined above
  
  //Absorber Plate made of Tungsten
  G4Tubs *AbsLayerSolid = new G4Tubs("AbsLayerSolid",
				    cal_innerradius,
				    cal_outerradius,
				    tungsten_hthickness,
				    cal_sphi, cal_ephi);
  

  AbsLayerLogical = new G4LogicalVolume(AbsLayerSolid,
					CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
					"AbsLayerLogical",
					0,
					0,
					0);
  
  
  if( displayMode < DM_ABSORBERANDSENSITIVE ) {
    AbsLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
  }
#ifdef NO_VIS_LUMICAL
  AbsLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif

  // ceramic support plate for the sillicon detector
  // 
  poly = CGAGeometryManager::GetMaterial("polystyrene");
  G4Tubs *SupportLayerSolid = new G4Tubs("SupportLayerSolid", cal_innerradius, cal_outerradius, support_hthickness, cal_sphi, cal_ephi);
        SupportLayerLogical = new G4LogicalVolume(SupportLayerSolid, poly,"SupportLayerLogical", 0,0,0);
  
  if( displayMode < DM_ABSORBERANDSENSITIVE )  
    SupportLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#ifdef NO_VIS_LUMICAL
  SupportLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif
  
  // a volume of sensitive material, here THETA silicon strip
  G4VisAttributes* visSenseSilicon = new G4VisAttributes(G4Colour(12.5,6,0));

  silicon = CGAGeometryManager::GetMaterial("silicon_2.33gccm");
 

      // create a cell dr-dphi
      //

  G4Tubs *CellSolid = new G4Tubs("CellSolid",cal_innerradius,(cal_innerradius+thetastrip_dr), silicon_hthickness,cal_sphi, phistrip_dphi);
        CellLogical = new G4LogicalVolume(CellSolid,silicon,"CellLogical",0 , 0, 0);
 
	//  here PHI silicon sector
	//
  G4Tubs *SectorSolid = new G4Tubs("SectorSolid",cal_innerradius,cal_outerradius,silicon_hthickness, cal_sphi, phistrip_dphi);
        SectorLogical = new G4LogicalVolume(SectorSolid,silicon, "SectorLogical",0, 0, 0);

	//whole silicon (helper) plane
	//
  G4Tubs *SensorSolid = new G4Tubs("SensorSolid", cal_innerradius,cal_outerradius,silicon_hthickness, cal_sphi, cal_ephi); 
        SensorLogical = new G4LogicalVolume(SensorSolid,silicon, "SensorLogical", 0, 0, 0);
  

 
  visSenseSilicon->SetForceSolid(true);
  //visSenseSilicon->SetForceWireframe(true);
  CellLogical->SetVisAttributes(G4VisAttributes::Invisible);
  SectorLogical->SetVisAttributes(visSenseSilicon);
  if( displayMode < DM_FULL ){
    CellLogical->SetVisAttributes(G4VisAttributes::Invisible);
    SectorLogical->SetVisAttributes(G4VisAttributes::Invisible);
  }
#ifdef NO_VIS_LUMICAL
    CellLogical->SetVisAttributes(G4VisAttributes::Invisible);
    SectorLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif
  
  //Replicate strips within a theta and phi plane 
    
    // padded version
    new G4PVDivision("ThetaCellReplica",CellLogical, SectorLogical,kRho, ncell_theta, thetastrip_dr, 0);
    new G4PVDivision("PhiCellReplica"  ,SectorLogical ,SensorLogical,kPhi,   ncell_phi, phistrip_dphi, 0);

   //Declare the silcon strip  to be sensitive 
  CellLogical->SetSensitiveDetector(theLumiCalSD); 

  
  //Assembly the layer 
  //
  //Put absorber plate 
  //
  G4double pos_abs = -layer_hz+tungsten_hthickness; 
  /*G4cout << "Layer Thickness 1: " << layer_hz*2. << " mm" << G4endl;
  G4cout << "Tungsten Thickness 1: " << tungsten_hthickness*2. << " mm" << G4endl;
  G4cout << "Absorber Plate at: " << pos_abs << " mm" << G4endl;
  */
  new G4PVPlacement(0,G4ThreeVector(0,0,pos_abs), AbsLayerLogical,"Absorber", LayerLogical, 0, 0);
  
  //Put support
  G4double pos_supp = pos_abs + tungsten_hthickness + support_hthickness; 
  // G4cout << "Support plate at: " << pos_supp << " mm" << G4endl;
  
  
  new G4PVPlacement(0,G4ThreeVector(0,0,pos_supp),SupportLayerLogical,"SupportPlate",LayerLogical,0, 0);
  
  //Put silicon detector
  G4double pos_sens = pos_supp + support_hthickness + silicon_hthickness; 
  //G4cout << "Silicon detector plate at: " << pos_sens << " mm" << G4endl;
  
  
  new G4PVPlacement(0,G4ThreeVector(0,0,pos_sens),SensorLogical,"SiliconDetector",LayerLogical,0,0);
    
}



G4bool LumiCalX::Build_LumiCal() {

  //position of first layer
  G4double lay_z = -cal_hz + layer_hz;
  //Put the layers into the LumiCal
  for (int nLay = 1; nLay < n_layers+1; nLay++) {
    
    std::stringstream slay;
    slay << nLay;
        
    //place layers into the LumiCal sub-module 
    //    G4cout << G4String("Layer") + G4String(slay.str())  <<" z= " << lay_z << G4endl;
    //
    G4String LayerName = G4String("Layer") + G4String(slay.str());
    new G4PVPlacement(0,  G4ThreeVector(0, 0, lay_z), LayerLogical, LayerName ,WholeLumiCalLogical, 0, nLay);

    lay_z += (layer_hz*2.0+layer_gap); 
    
  }
  //place two LumiCal sub-modules into the world
  //
  G4double z_center = z_begin+cal_hz;
  G4double rotAngle1 = 180.*deg - bx_angle;
  G4double rotAngle2 = bx_angle;
  G4Transform3D transformer1(G4RotationMatrix().rotateY(rotAngle1),G4ThreeVector(0, 0,  z_center).rotateY(rotAngle1));
  G4Transform3D transformer2(G4RotationMatrix().rotateY(rotAngle2),G4ThreeVector(0, 0,  z_center).rotateY(rotAngle2));


     new G4PVPlacement(transformer1,WholeLumiCalLogical,"LumiCalN",WorldLogical, 0, 0);
     new G4PVPlacement(transformer2,WholeLumiCalLogical,"LumiCalP",WorldLogical, 0, 1);
 
    delete db;
    db = 0;
    G4cout <<" LumiCalX done.\n" << G4endl;

  return true;
}



void LumiCalX::SetSD() {
  
  //create an instance of a sensitive detector class
  //the actual registration of all sensitive
  //detectors to G4 is done via a Mokka interface,
  //so we create an instance of our sens. detector class 
  //and make this
  //known to Mokka 
  theLumiCalSD = new LumiCalSD("LumiCal",
			       "Pad",
			       cal_innerradius,
			       cal_sphi,
			       GetRhoCellSize(),
			       GetPhiCellSize(),
			       ncell_theta,
			       ncell_phi,
			       //LUMICALO defined in Mokka/source/Kernel/include/Control.hh
			       0);
  
  RegisterSensitiveDetector(theLumiCalSD);
}
