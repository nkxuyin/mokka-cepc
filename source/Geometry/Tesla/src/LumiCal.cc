#include "Control.hh"
#include "LumiCal.hh"

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
INSTANTIATE (LumiCal)

  LumiCal::~LumiCal() {/*no op*/}

G4bool LumiCal::construct(const G4String &aSubDetectorDBName,
			G4LogicalVolume *WorldLog) {

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

void LumiCal::FetchdbEntries(){
  //fetch geometry constants (might be more elegant to do this in a
  //separate routine)
  //provide access to database table block
  //
  db->exec("select * from lumical;");
  db->getTuple();
  //LumiCal type
  LumiCal_Type = db->fetchString("type");
  Strip_Type = "strip"; 
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
  //
  // calculate basic dimension   
  layer_hz = tungsten_hthickness+support_hthickness+silicon_hthickness;
  cal_hz = (G4double) (n_layers * (layer_hz+layer_gap/2.))*mm;
  cal_sphi = 0.*deg;
  cal_ephi = 360.*deg;
  thetastrip_dr = (G4double) ((cal_outerradius-cal_innerradius)/ncell_theta)*mm;
  phistrip_dphi = (G4double) (cal_ephi/ncell_phi);
  //
  G4cout << " LumiCal Rmin : "<< cal_innerradius << " mm" << G4endl;
  G4cout << " LumiCal Rmax : "<< cal_outerradius << " mm" << G4endl;
  G4cout << " LumiCal length : "<< 2.*cal_hz << " mm" << G4endl;
  G4cout << " LumiCal #of Theta strips : "<< ncell_theta<< G4endl;
  G4cout << " LumiCal #of Phi strips   : "<< ncell_phi<< G4endl;
  G4cout << " Theta cell size : "<< thetastrip_dr<< " mm" << G4endl;
  G4cout << " Phi cell size : "<< phistrip_dphi<< " rad" << G4endl;
  G4cout << " Phi angle size from : "<< cal_sphi<< " to "<< cal_ephi<<" rad" << G4endl;

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
  lcalParams->setDoubleVal("beam_crossing_angle",   0.  ) ;

  cout << "gear: lcalParameters-  BeamCrossingAngle = " << lcalParams->getDoubleVal("beam_crossing_angle" ) ;
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


void LumiCal::BuildElements() {


  // create and register SD
  SetSD();


  //Set displayMode 
  int displayMode = UserInit::getInstance()->getInt("DisplayMode") ;
  if( displayMode == 0 )  // if nothing specified display full details
    displayMode = DM_FULL ;
  
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
  G4Tubs *ThetaLayerSolid = new G4Tubs("ThetaLayerSolid",
				      cal_innerradius,
				      cal_outerradius,
				      layer_hz,
				      cal_sphi, cal_ephi);
  
  ThetaLayerLogical = new G4LogicalVolume(ThetaLayerSolid,
                                          air,
                                          "ThetaLayerLogical",
                                          0,
                                          0,
                                          0);
  if( LumiCal_Type == Strip_Type ){
  // phi layer
  
  G4Tubs *PhiLayerSolid = new G4Tubs("PhiLayerSolid",
				      cal_innerradius,
				      cal_outerradius,
				      layer_hz,
				      cal_sphi, cal_ephi);
  
  PhiLayerLogical = new G4LogicalVolume(PhiLayerSolid,
                                          air,
                                          "PhiLayerLogical",
                                          0,
                                          0,
                                          0);
  }
  
#ifdef NO_VIS_LUMICAL
  WholeLumiCalLogical->SetVisAttributes(G4VisAttributes::Invisible);
  ThetaLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
  if( LumiCal_Type == Strip_Type )
    PhiLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#else
  G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(1.,1.,0));
  VisAtt->SetDaughtersInvisible(true);
  VisAtt->SetForceSolid(true);
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
  poly = CGAGeometryManager::GetMaterial("polystyrene");
  G4Tubs *SupportLayerSolid = new G4Tubs("SupportLayerSolid",
				       cal_innerradius,
				       cal_outerradius,
				       support_hthickness,
				       cal_sphi, cal_ephi);
  
  SupportLayerLogical = new G4LogicalVolume(SupportLayerSolid,
					    poly,
					    "SupportLayerLogical",
					    0,
					    0,
					    0);
  
  if( displayMode < DM_ABSORBERANDSENSITIVE )  
    SupportLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#ifdef NO_VIS_LUMICAL
  SupportLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif
  
  // a volume of sensitive material, here THETA silicon strip
  G4VisAttributes* visSenseSilicon = new G4VisAttributes(G4Colour(12.5,6,0));

  silicon = CGAGeometryManager::GetMaterial("silicon_2.33gccm");

   if( LumiCal_Type == Strip_Type ){
 G4Tubs *ThetaStripSolid = new G4Tubs("ThetaStripSolid",
				       cal_innerradius,
				      (cal_innerradius+thetastrip_dr),
				       silicon_hthickness,
				       cal_sphi, cal_ephi);
  
  ThetaStripLogical = new G4LogicalVolume(ThetaStripSolid,
					 silicon,
					 "ThetaStripLogical",
					 0,
					 0,
					 0);
   }else{
     // pad version create a cell dr-dphi
     //

     G4Tubs *ThetaStripSolid = new G4Tubs("ThetaStripSolid",
					  cal_innerradius,
					  (cal_innerradius+thetastrip_dr),
					  silicon_hthickness,
					  cal_sphi, phistrip_dphi);
     ThetaStripLogical = new G4LogicalVolume(ThetaStripSolid,
					     silicon,
					     "ThetaStripLogical",
					     0,
					     0,
					     0);
   }
  //whole silicon (helper) plane
  G4Tubs *SenseThetaSolid = new G4Tubs("SenseThetaSolid",
				       cal_innerradius,
				       cal_outerradius,
				       silicon_hthickness,
				       cal_sphi, cal_ephi);
  
  SenseThetaLogical = new G4LogicalVolume(SenseThetaSolid,
					  ThetaStripLogical->GetMaterial(),
					 "SenseThetaLogical",
					 0,
					 0,
					 0);
  
  //  here PHI silicon strip
  G4Tubs *PhiStripSolid = new G4Tubs("PhiStripSolid",
				       cal_innerradius,
				       cal_outerradius,
				       silicon_hthickness,
				       cal_sphi, phistrip_dphi);
  
  PhiStripLogical = new G4LogicalVolume(PhiStripSolid,
					 silicon,
					 "PhiStripLogical",
					 0,
					 0,
					 0);
  if( LumiCal_Type == Strip_Type ){
  //whole silicon (helper) PHI plane
  G4Tubs *SensePhiSolid = new G4Tubs("SensePhiSolid",
				       cal_innerradius,
				       cal_outerradius,
				       silicon_hthickness,
				       cal_sphi, cal_ephi);
  
  SensePhiLogical = new G4LogicalVolume(SensePhiSolid,
					PhiStripLogical->GetMaterial(),
					 "SensePhiLogical",
					 0,
					 0,
					 0);
  SensePhiLogical->SetVisAttributes(G4VisAttributes::Invisible);
  }

 
  visSenseSilicon->SetForceSolid(true);
  //visSenseSilicon->SetForceWireframe(true);
  SenseThetaLogical->SetVisAttributes(G4VisAttributes::Invisible);
  ThetaStripLogical->SetVisAttributes(visSenseSilicon);
  PhiStripLogical->SetVisAttributes(visSenseSilicon);
  if( displayMode < DM_FULL ){
    ThetaStripLogical->SetVisAttributes(G4VisAttributes::Invisible);
    PhiStripLogical->SetVisAttributes(G4VisAttributes::Invisible);
  }
#ifdef NO_VIS_LUMICAL
    ThetaStripLogical->SetVisAttributes(G4VisAttributes::Invisible);
    PhiStripLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif
  
  //Replicate strips within a theta and phi plane 
    
  
  if( LumiCal_Type == Strip_Type ){
    new G4PVDivision("ThetaStripReplica",
		     ThetaStripLogical,
		     SenseThetaLogical,
		     kRho,
		     ncell_theta,
		     thetastrip_dr,
		     0.);
    new G4PVReplica("PhiStripReplica",
		    PhiStripLogical,
		    SensePhiLogical,
		    kPhi,
		    ncell_phi,
		    phistrip_dphi,
		    0);
  } else {
    // padded version
    new G4PVDivision("ThetaCellReplica",
		    ThetaStripLogical,
		    PhiStripLogical,
		    kRho,
		    ncell_theta,
		    thetastrip_dr,
		    0);
    new G4PVDivision("PhiCellReplica",
		     PhiStripLogical,
		     SenseThetaLogical,
		     kPhi,
		     ncell_phi,
		     phistrip_dphi,
		     0);
  }

   //Declare the silcon strip  to be sensitive 
  ThetaStripLogical->SetSensitiveDetector(theLumiCalSD);
  if( LumiCal_Type == Strip_Type ) PhiStripLogical->SetSensitiveDetector(theLumiCalSD);
 
  
  //Put the layer together
  //Put absorber plate into the layer
  G4double pos_abs = -layer_hz+tungsten_hthickness; 
  G4cout << "Layer Thickness 1: " << layer_hz*2. << " mm" << G4endl;
  G4cout << "Tungsten Thickness 1: " << tungsten_hthickness*2. << " mm" << G4endl;
  G4cout << "Absorber Plate at: " << pos_abs << " mm" << G4endl;
  
  new G4PVPlacement(0,
                    G4ThreeVector(0,0,pos_abs),
                    AbsLayerLogical,
                    "Absorber",
                    ThetaLayerLogical,                                
                    0,
                    0);

  if( LumiCal_Type == Strip_Type ){
  new G4PVPlacement(0,
                    G4ThreeVector(0,0,pos_abs),
                    AbsLayerLogical,
                    "Absorber",
                    PhiLayerLogical,                                
                    0,
                    0);
  }
  
  //Put support part(s) into the layer
  G4double pos_supp = pos_abs + tungsten_hthickness + support_hthickness; 
  G4cout << "Support plate at: " << pos_supp << " mm" << G4endl;
  
  
  new G4PVPlacement(0,
                    G4ThreeVector(0,0,pos_supp),
                    SupportLayerLogical,
                    "SupportPlate",
                    ThetaLayerLogical,                                
                    0,
                    0);
  
  if( LumiCal_Type == Strip_Type ){
  new G4PVPlacement(0,
                    G4ThreeVector(0,0,pos_supp),
                    SupportLayerLogical,
                    "SupportPlate",
                    PhiLayerLogical,                                
                    0,
                    0);
  }
  //Put silicon detectors into the layer
  G4double pos_sens = pos_supp + support_hthickness + silicon_hthickness; 
  G4cout << "Silicon detector plate at: " << pos_sens << " mm" << G4endl;
  
  
  new G4PVPlacement(0,
                    G4ThreeVector(0,0,pos_sens),
                    SenseThetaLogical,
                    "SiliconDetector",
                    ThetaLayerLogical,                                
                    0,
                    0);
  
   if( LumiCal_Type == Strip_Type ){
     new G4PVPlacement(0,
		       G4ThreeVector(0,0,pos_sens),
		       SensePhiLogical,
		       "SiliconDetector",
		       PhiLayerLogical,                                
		       0,
		       0);
   }

  
  
}



G4bool LumiCal::Build_LumiCal() {

  //position of first layer
  G4double lay_z = -cal_hz + layer_hz;
  //Put the layers into the LumiCal
  for (int nLay = 1; nLay < n_layers+1; nLay++) {
    
    std::stringstream slay;
    slay << nLay;
    
    
    //place layers into the LumiCal sub-module 
    //    G4cout << G4String("Layer") + G4String(slay.str())  <<" z= " << lay_z << G4endl;
    if((nLay%2 == 0) && (LumiCal_Type == Strip_Type)) {
    new G4PVPlacement(0,
		      G4ThreeVector(0, 0, lay_z),
		      PhiLayerLogical,
		      G4String("Layer") + G4String(slay.str()),
		      WholeLumiCalLogical,
		      0,
		      nLay);
    }else{
     new G4PVPlacement(0,
		      G4ThreeVector(0, 0, lay_z),
		      ThetaLayerLogical,
		      G4String("Layer") + G4String(slay.str()),
		      WholeLumiCalLogical,
		      0,
		      nLay);
   }
    lay_z += (layer_hz*2.0+layer_gap); 
    
  }
  //place two LumiCal sub-modules into the world
  G4double z_center = z_begin+cal_hz;
  G4RotationMatrix* yrot180deg = new G4RotationMatrix();
  yrot180deg-> rotateY(180.*deg);
     new G4PVPlacement(yrot180deg,
		       G4ThreeVector(0, 0, -z_center),
		       WholeLumiCalLogical,
		       "LumiCalN",
		       WorldLogical,
		       0,
		       1);
     new G4PVPlacement(0,
		       G4ThreeVector(0, 0, z_center),
		       WholeLumiCalLogical,
		       "LumiCalP",
		       WorldLogical,
		       0,
		       2);
 
    delete db;
    db = 0;
    G4cout <<" LumiCal done.\n" << G4endl;

  return true;
}



void LumiCal::SetSD() {
  
  //create an instance of a sensitive detector class
  //the actual registration of all sensitive
  //detectors to G4 is done via a Mokka interface,
  //so we create an instance of our sens. detector class 
  //and make this
  //known to Mokka 
  theLumiCalSD = new LumiCalSD("LumiCal",
			       LumiCal_Type,
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
