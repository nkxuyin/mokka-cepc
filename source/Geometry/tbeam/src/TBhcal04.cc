// $Header: /home/flc/cvs/Mokka/source/Geometry/tbeam/src/TBhcal04.cc,v 1.10 2006/09/29 14:07:47 oliver Exp $
// Author Roman Poeschl DESY
// Routines adapted from Jeremy McCormick, NIU
// Updated for CERN test beam geometry by Oliver Wendt DESY

#include "Control.hh"
#include "TBhcal04.hh"

#include "CGADefs.h"
#include "MySQLWrapper.hh"

#include "CGAGeometryManager.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4PVParameterised.hh"
#include "UserInit.hh"

#include <sstream>

#define NO_VIS_HCAL 1

// define some levels of detail for graphical display
#define DM_FULL 3
#define DM_ABSORBERANDSENSITIVE 2
#define DM_WHOLELAYERONLY  1

INSTANTIATE(TBhcal04)

TBhcal04::TBhcal04() : VSubDetectorDriver("TBhcal04","TBhcal"),
		       db(0),
                       _aGeometryEnvironment("","",NULL, NULL),
		       config_angle(0)  
{
}

TBhcal04::~TBhcal04()
{}

//G4bool TBhcal04::construct(const G4String &aSubDetectorDBName,
//			   G4LogicalVolume *WorldLog)
G4bool TBhcal04::ContextualConstruct(const
    CGAGeometryEnvironment &aGeometryEnvironment,
    G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding TBhcal04..." << G4endl;

  //Obtain the pointer to our database via the Environment object
  _aGeometryEnvironment = aGeometryEnvironment;
  db = new Database(_aGeometryEnvironment.GetDBName());

  WorldLogical = WorldLog;    
  
  FetchAll();
  AddMaterial();
  BuildElements();
  Print();

  G4bool cokay = BuildHcal();

  delete db;
  db = 0;

  G4cout << "\nDone building TBhcal04" << G4endl;
  
  return cokay;
}

// fetch MySQL variables
void TBhcal04::FetchAll()
{

  // config angle from environment object
  config_angle = _aGeometryEnvironment.GetParameterAsDouble ("configuration_angle");
  G4cout << "config_angle <" << config_angle << ">" << G4endl;
  config_angle = config_angle*deg;
  //we are dealing with a simple cell grid 
  //so we only need to know the number of layers,
  //the number of cells in x and z direction and the
  //basic grid size 
  // get layering n
  db->exec("select * from hcal_virt;");
  db->getTuple();
  n_layers = db->fetchInt("n_layers");
  assert(n_layers > 0);
  //number of cells in x
  ncell_xy[0] = db->fetchInt("ncell_x");
  //number of cells in z
  ncell_xy[1] = db->fetchInt("ncell_y");
  assert(ncell_xy[0] >= 0 || ncell_xy[1] >= 0);
  //grid size
  grid_size = db->fetchDouble("grid_size");
  //the place where we'll put the calorimeter 
  //z_place = db->fetchDouble("z_place");
  //the beginning of the Hcal 
  z_begin = db->fetchDouble("z_begin");
  
  
  // set up vector for the sensitive layer pattern
  G4String sensitiveLayerPatternFromSteeringFile = _aGeometryEnvironment.GetParameterAsString("Hcal_layer_pattern");
  
  // set up default layer pattern with the lenghth 'n_layers'
  G4String sensitiveLayerPatternDefault("");
  for (G4int i = 0; i < n_layers; ++i) sensitiveLayerPatternDefault += '1';

  // if steering command /Mokka/init/HCALLayerPattern exists in steering file
  if ( ((G4int)sensitiveLayerPatternFromSteeringFile.length()) != 0 ) {
    
    if (isValidSensitiveLayerPattern(sensitiveLayerPatternFromSteeringFile)) {

      sensitiveLayerPatternVector = getSensitiveLayerPatternVector(sensitiveLayerPatternFromSteeringFile);

    }
    else {

      G4cout << "'/Mokka/init/HCALLayerPattern' parameter is not valid in steering file. ABORTING MOKKA" << G4endl;
      exit(1);

    }

  }
  else {
    
    Database* db2 = new Database(Control::MODELS_DBNAME.data());
    db2->exec("select * from parameters where name='Hcal_layer_pattern';");
    db2->getTuple();
    G4String sensitiveLayerPatternFromDatabase("");
    sensitiveLayerPatternFromDatabase = db2->fetchString("default_value");

    if ( ((G4int)sensitiveLayerPatternFromDatabase.length()) != 0 ) {
      
      if (isValidSensitiveLayerPattern(sensitiveLayerPatternFromDatabase)) {
	
	sensitiveLayerPatternVector = getSensitiveLayerPatternVector(sensitiveLayerPatternFromDatabase);
	
      }
      else {
	
	G4cout << "No valid layer pattern for the HCAL prototype found in database 'parameter' and no steering parameter '/Mokka/init/HCALLayerPattern' set." 
	       << G4endl
	       << "Using a fully equipped HCAL as a default" << G4endl;
	
	sensitiveLayerPatternVector = getSensitiveLayerPatternVector(sensitiveLayerPatternDefault);

      }

    }
    else {
	
      G4cout << "No valid layer pattern for the HCAL prototype found in database 'parameter' and no steering parameter '/Mokka/init/HCALLayerPattern' set." 
	     << G4endl
	     << "Using a fully equipped HCAL as a default" << G4endl;
	
      sensitiveLayerPatternVector = getSensitiveLayerPatternVector(sensitiveLayerPatternDefault);

    }
    
    delete db2;
    db2 = 0;

  }

  //debug
  G4String layers;
  G4String pattern;
  for (G4int i = 0; i < n_layers; ++i) {
    // for debug output
    char dummy[100];
    sprintf(dummy,"%2d ",i);
    layers += dummy;
    sprintf(dummy,"%2d ",sensitiveLayerPatternVector.at(i));
    pattern += dummy;

  }  
  // debug output
  G4cout << G4endl << "HCAL layer numbers and status of sensitive layers (1=sensitive layer installed, 0=air gap)" << G4endl 
	 << layers << G4endl << pattern << G4endl << G4endl;





  // thicknesses
  db->exec("select * from hcal_layer_thickness;");
  db->getTuple();

  poly_hthickness = db->fetchDouble("poly_thickness")/2;
  steel_hthickness = db->fetchDouble("steel_thickness")/2;
  airgap_hthickness = db->fetchDouble("air_gap")/2;
  steel_cassette_hthickness = db->fetchDouble("steel_cassette_thickness")/2.;
  foil_hthickness = db->fetchDouble("foil_thickness")/2.;
  pcb_hthickness = db->fetchDouble("pcb_thickness")/2.;
  cablefibre_mix_hthickness = db->fetchDouble("cablefibre_mix_thickness")/2.;

  //Fetch data for terminating absorber of Hcal
  db->exec("select * from hcal_termlayer;");
  db->getTuple();

  n_layer_term = db->fetchInt("n_layer_term");
  steel_hthickness_term = db->fetchDouble("steel_thickness_term")/2;

}

// define calculated vars, materials, solids, LV
void TBhcal04::BuildElements()
{
  // hcal dims
  layer_hthickness =
    poly_hthickness+steel_hthickness+2.0*airgap_hthickness+2.0*steel_cassette_hthickness+
    2.0*foil_hthickness+pcb_hthickness+cablefibre_mix_hthickness;
  G4cout << "Layer Thickness: " << layer_hthickness*2. << " mm" << G4endl;
  //G4cout << "Steel Thickness: " << steel_hthickness*2. << " mm" << G4endl;
    
  //Information needed when hits are processed later on
  SetDepthToLayer(1);
  // materials
  poly = CGAGeometryManager::GetMaterial("polystyrene");
  //steel = CGAGeometryManager::GetMaterial("stainless_steel");
  steel = S235;
  air = CGAGeometryManager::GetMaterial("air");
  foil_3m = TBhcal04::Polystyrole;
  pcb = TBhcal04::PCB;
  cf_mix = TBhcal04::CF_MIX;

  cal_hx = (G4double) (ncell_xy[0] * grid_size*mm)/2.;
  cal_hy = (G4double) (ncell_xy[1] * grid_size*mm)/2.;
  cal_hz = (G4double) (n_layers * layer_hthickness + steel_hthickness);

  //Create the detector step by step
  // whole detector
  //the z-position of the of the calorimeter
  z_place = z_begin + cal_hz;  

  // create and register SD
  SetSD();


  G4Box *DetectorSolid = new G4Box("DetectorSolid",
				   cal_hx,
				   cal_hy,
				   cal_hz);

  DetectorLogical = new G4LogicalVolume(DetectorSolid,
					air,
					"DetectorLogical",
					0,
					0,
					0);

#ifdef NO_VIS_HCAL
  DetectorLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif

  // whole layer
  G4Box *WholeLayerSolid = new G4Box("WholeLayerSolid",
				     cal_hx,
				     cal_hy,
				     layer_hthickness);

  WholeLayerLogical = new G4LogicalVolume(WholeLayerSolid,
					  air,
					  "WholeLayerLogical",
					  0,
					  0,
					  0);
  
  //G4VisAttributes* visWholeLayerLogical = new G4VisAttributes();
  //visWholeLayerLogical->SetColour(0.0,0.0,0.0,0.0);
  //WholeLayerLogical->SetVisAttributes(visWholeLayerLogical);



  // second full layer, but this time with an air gap instead of sensor casette
  G4Box *WholeLayer2Solid = new G4Box("WholeLayer2Solid",
				     cal_hx,
				     cal_hy,
				     layer_hthickness);

  WholeLayer2Logical = new G4LogicalVolume(WholeLayer2Solid,
					  air,
					  "WholeLayer2Logical",
					  0,
					  0,
					  0);
  
  //G4VisAttributes* visWholeLayer2Logical = new G4VisAttributes();
  //visWholeLayer2Logical->SetColour(0.0,0.0,0.0,0.0);
  //WholeLayer2Logical->SetVisAttributes(visWholeLayer2Logical);



#ifdef NO_VIS_HCAL
  //  WholeLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif

  //Absorber Plate
  G4Box *AbsLayerSolid = new G4Box("AbsLayerSolid",
				   cal_hx,
				   cal_hy,
				   steel_hthickness);


  AbsLayerLogical = new G4LogicalVolume(AbsLayerSolid,
					  steel,
					 "AbsLayerLogical",
					  0,
					  0,
					  0);

  //Create a logical volume for the 'terminating' absorber layer of
  //the HCal

  //Absorber Plate
  G4Box *AbsLayerSolid_term = new G4Box("AbsLayerSolid_term",
				   cal_hx,
				   cal_hy,
				   steel_hthickness_term);


  AbsLayerLogical_term = new G4LogicalVolume(AbsLayerSolid_term,
					  steel,
					 "AbsLayerLogical_term",
					  0,
					  0,
					  0);


  int displayMode = UserInit::getInstance()->getInt("DisplayMode") ;
  if( displayMode == 0 )  // if nothing specified display full details
    displayMode = DM_FULL ;

  G4cout << " using display mode - vele of detail : " << displayMode << std::endl ;


#ifdef NO_VIS_HCAL
  AbsLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
  AbsLayerLogical_term->SetVisAttributes(G4VisAttributes::Invisible);
#endif
  if( displayMode < DM_ABSORBERANDSENSITIVE ) {
    AbsLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
    AbsLayerLogical_term->SetVisAttributes(G4VisAttributes::Invisible);
    
  }
  
  //the scintillator housing (made of S235)       

  G4Box *ScinHousSolid = new G4Box("ScinHousSolid",
				   cal_hx,
				   cal_hy,
				   steel_cassette_hthickness);


  ScinHousLogical = new G4LogicalVolume(ScinHousSolid,
					  steel,
					  "ScinHouseLogical",
					  0,
					  0,
					  0);

#ifdef NO_VIS_HCAL
  ScinHousLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif

  if( displayMode < DM_FULL )  
    ScinHousLogical->SetVisAttributes(G4VisAttributes::Invisible);
  
  
  // sensitive layer
  G4Box *WholeSensLayerSolid = new G4Box("WholeSensLayerSolid",
					 cal_hx,
					 cal_hy,
					 poly_hthickness);

  WholeSensLayerLogical = new G4LogicalVolume(WholeSensLayerSolid,
					      poly,
					      "WholeSensLayerLogical",
					      0,
					      0,
					      0);

  WholeSensLayerLogical->SetSensitiveDetector(hcalSD);

#ifdef NO_VIS_HCAL
  WholeSensLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif
  if( displayMode < DM_FULL )  
    WholeSensLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);

  //3M foil
  G4Box *FoilLayerSolid = new G4Box("ALuLayerSolid",
				   cal_hx,
				   cal_hy,
				   foil_hthickness);


  FoilLayerLogical_1 = new G4LogicalVolume(FoilLayerSolid,
					      foil_3m,
					      "FoilLayerLogical_1",
					      0,
					      0,
					      0);
   

  FoilLayerLogical_2 = new G4LogicalVolume(FoilLayerSolid,
					      foil_3m,
					      "FoilLayerLogical_2",
					      0,
					      0,
					      0);
   
#ifdef NO_VIS_HCAL
  FoilLayerLogical_1->SetVisAttributes(G4VisAttributes::Invisible);
  FoilLayerLogical_2->SetVisAttributes(G4VisAttributes::Invisible);
#endif
  if( displayMode < DM_FULL )  {
    FoilLayerLogical_1->SetVisAttributes(G4VisAttributes::Invisible);
    FoilLayerLogical_2->SetVisAttributes(G4VisAttributes::Invisible);
  }

  //a pcb layer
  G4Box *PCBLayerSolid = new G4Box("PCBLayerSolid",
				   cal_hx,
				   cal_hy,
				   pcb_hthickness);


  PCBLayerLogical = new G4LogicalVolume(PCBLayerSolid,
					      pcb,
					      "PCBLayerLogical",
					      0,
					      0,
					      0);
   
#ifdef NO_VIS_HCAL
  PCBLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif
  if( displayMode < DM_FULL )  
    PCBLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);


  //the cablefibre mixture 

  G4Box *CFmix_LayerSolid = new G4Box("CFmix_LayerSolid",
				      cal_hx,
				      cal_hy,
				      cablefibre_mix_hthickness);


  CFmix_LayerLogical = new G4LogicalVolume(CFmix_LayerSolid,
					      cf_mix,
					      "CFmix_LayerLogical",
					      0,
					      0,
					      0);
   
#ifdef NO_VIS_HCAL
  CFmix_LayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif
  if( displayMode < DM_FULL )  
    CFmix_LayerLogical->SetVisAttributes(G4VisAttributes::Invisible);




  // air gap; used instead of scintillator housing, 3M foil Layer1, sensitive layer, 3M foil Layer2, PCB and Cable-Fibre mixture
  // for simulation of HCAL with half the number of sensor casettes

  G4double airGapThickness = steel_cassette_hthickness + foil_hthickness + poly_hthickness + foil_hthickness + pcb_hthickness + cablefibre_mix_hthickness 
    + steel_cassette_hthickness;

  G4cout << "airGapThickness = " << airGapThickness * 2.0 << " mm" << G4endl;

  G4Box *CFairGap_LayerSolid = new G4Box("CFairGap_LayerSolid",
			                 cal_hx,
			                 cal_hy,
			                 airGapThickness);


  CFairGap_LayerLogical = new G4LogicalVolume(CFairGap_LayerSolid,
					      air,
					      "FairGap_LayerLogical",
					      0,
					      0,
					      0);
   
#ifdef NO_VIS_HCAL
  CFairGap_LayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif
  if( displayMode < DM_FULL )  
    CFairGap_LayerLogical->SetVisAttributes(G4VisAttributes::Invisible);








  //Put absorber plate into the layer
  G4double pos_abs = -layer_hthickness+steel_hthickness; 
  G4cout << "Layer Thickness 1: " << layer_hthickness*2. << " mm" << G4endl;
  G4cout << "Steel Thickness 1: " << steel_hthickness*2. << " mm" << G4endl;
  G4cout << "Absorber Plate at: " << pos_abs << " mm" << G4endl;

  new G4PVPlacement(0,
		    G4ThreeVector(0,0,pos_abs),
		    AbsLayerLogical,
		    "AbsLayerPhys",
		    WholeLayerLogical,                                
		    0,
		    0);

  //Put scintillator housing front plate into the layer (after absorber and before scintillator)
  G4double pos_house1 = pos_abs + steel_hthickness
    + 2.0*airgap_hthickness + steel_cassette_hthickness;
  G4cout << "Scintillator Housing Frontplate at: " << pos_house1 << " mm" << G4endl;


  new G4PVPlacement(0,
		    G4ThreeVector(0,0,pos_house1),
		    ScinHousLogical,
		    "ScinHousPhys Front",
		    WholeLayerLogical,                      
		    0,
		    0);

  //Put first 3M foil Layer into the complete layer
  G4double pos_foil_1 = pos_house1 + steel_cassette_hthickness + foil_hthickness; 
  G4cout << "3M Foil 1 at: " << pos_foil_1 << " mm" << G4endl;
  new G4PVPlacement(0,
		    G4ThreeVector(0,0,pos_foil_1),
		    FoilLayerLogical_1,
		    "SensLayerPhys",
		    WholeLayerLogical,                                
		    0,
		    0);


  //Put sensitive part (i.e. scintillator plate) into the layer


  G4double pos_sens = pos_foil_1 + foil_hthickness + poly_hthickness; 
  //G4cout << "Scintillating Tiles at: " << pos_sens << " mm" << G4endl;


  new G4PVPlacement(0,
		    G4ThreeVector(0,0,pos_sens),
		    WholeSensLayerLogical,
		    "SensLayerPhys",
		    WholeLayerLogical,                                
		    0,
		    0);


  //Put alu foil into the layer

  //G4double pos_alu = pos_sens - poly_hthickness - foil_hthickness; 
  G4double pos_foil_2 = pos_sens + poly_hthickness + foil_hthickness; 
  G4cout << "3M Foil 2 at: " << pos_foil_2 << " mm" << G4endl;

  new G4PVPlacement(0,
		    G4ThreeVector(0,0,pos_foil_2),
		    FoilLayerLogical_2,
		    "SensLayerPhys",
		    WholeLayerLogical,                                
		    0,
		    0);

  //Put the PCB into the layer

  G4double pos_pcb = pos_foil_2 + foil_hthickness + pcb_hthickness; 
  G4cout << "PCB at: " << pos_pcb << " mm" << G4endl;

  new G4PVPlacement(0,
		    G4ThreeVector(0,0,pos_pcb),
		    PCBLayerLogical,
		    "SensLayerPhys",
		    WholeLayerLogical,                                
		    0,
		    0);


  //Put the Cable-Fibre mixture into the layer

  G4double pos_cfmix = pos_pcb + pcb_hthickness + cablefibre_mix_hthickness; 
  G4cout << "Cable-Fibre mixture at: " << pos_cfmix << " mm" << G4endl;

  new G4PVPlacement(0,
		    G4ThreeVector(0,0,pos_cfmix),
		    CFmix_LayerLogical,
		    "CFmix_LayerPhys",
		    WholeLayerLogical,                                
		    0,
		    0);



  //Put scintillator housing rear plate into the layer
  G4double pos_house2 = pos_cfmix + cablefibre_mix_hthickness + steel_cassette_hthickness;
  G4cout << "Scintillator Housing Rearplate at: " << pos_house2 << " mm" << G4endl;
  new G4PVPlacement(0,
		    G4ThreeVector(0,0,pos_house2),
		    ScinHousLogical,
		    "ScinHousPhys Rear",
		    WholeLayerLogical,                                
		    0,
		    1);
  



  // create layer with absorber and air gap only. Used for simulation of HCAL with various numbers and positions of sensor casettes

  //Put absorber plate into the layer
  G4double pos_abs2 = -layer_hthickness+steel_hthickness; 
  G4cout << "Creating second kind of layer with absorber and air gap only ..." << G4endl;
  G4cout << "Layer Thickness 1: " << layer_hthickness*2. << " mm" << G4endl;
  G4cout << "Steel Thickness 1: " << steel_hthickness*2. << " mm" << G4endl;
  G4cout << "Absorber Plate at: " << pos_abs << " mm" << G4endl;

  new G4PVPlacement(0,
		    G4ThreeVector(0,0,pos_abs2),
		    AbsLayerLogical,
		    "AbsLayerPhys",
		    WholeLayer2Logical,
		    0,
		    0);

  //Put air gap
  G4double pos_airGap =  pos_abs2 + steel_hthickness + 2.0*airgap_hthickness + airGapThickness;

  G4cout << "AirGap (instead of sensor casette) at : " << pos_airGap << " mm" << G4endl;


  new G4PVPlacement(0,
		    G4ThreeVector(0,0,pos_airGap),
		    CFairGap_LayerLogical,
		    "AirGap (instead of sensor casette)",
		    WholeLayer2Logical,                      
		    0,
		    0);

}

  //We have to place the Hcal layers into the world in order to cope
  //with the various configuration (i.e. impact) angles
void TBhcal04::PlaceLayer(G4LogicalVolume *WorldLog, G4int nlay)
{

  G4cerr << "Building hcal layer <" << nlay << ">." << G4endl;
  //Calculate z-position of layer
  //G4double lay_z = cal_hz - layer_hthickness;
  //G4double lay_z = z_begin + layer_hthickness;
  //G4double lay_z = -cal_hz + layer_hthickness;  
  
  G4double lay_z = z_begin + layer_hthickness;
  //Potential off set in x-due to gap between Ecal and Hcal  
  x_offset = z_begin*tan(config_angle);

  if (nlay > 1)


    //lay_z -= ((nlay-1) * (layer_hthickness*2));
    lay_z += ((nlay-1) * (layer_hthickness*2));


  G4double lay_x = x_offset + 2.*layer_hthickness* ( (G4double) (nlay-1))*tan(config_angle);
  // G4cout << "x Position of layer: " << lay_x << G4endl;
  // G4cout << "z Position of Layer: " << lay_z << G4endl;

  std::stringstream slay;
  slay << nlay;


  // alter sensor casette and air gap
  if ( sensitiveLayerPatternVector.at(nlay-1) == 0 ){
    // place an air gap
    new G4PVPlacement(0,
		      G4ThreeVector(lay_x,0,lay_z),
		      WholeLayer2Logical,
		      G4String("WholeLayerPhys2") + G4String(slay.str()),
		      WorldLog,
		      0,
		      nlay);
    G4cout << "air gap" << G4endl << G4endl;

  }
  else {
    //place layer into logical volume reserved for the HCAL
    // G4PVPlacement *WholeLayerPhys = 
    new G4PVPlacement(0,
		      G4ThreeVector(lay_x,0,lay_z),
		      WholeLayerLogical,
		      G4String("WholeLayerPhys") + G4String(slay.str()),
		      WorldLog,
		      0,
		      nlay);
    G4cout << "sensitive layer" << G4endl << G4endl;

  }




  if (nlay == n_layers) {

    
    for (int nterm = 0; nterm < n_layer_term; nterm++) {
      if (nterm == 0 ) lay_z = lay_z + layer_hthickness + steel_hthickness_term;
      if (nterm > 0 ) lay_z += 2*steel_hthickness_term;
      lay_x = x_offset + lay_z*tan(config_angle);
      G4cout << "Placing Terminating Absorber Number: " << nterm+1 << G4endl;
      //G4cout << "x Position of Terminating Absorber: " << lay_x << G4endl;
      //G4cout << "z Position of Terminating Absorber: " << lay_z << G4endl;
      
      std::stringstream sterm;
      sterm << nterm;
      
      new G4PVPlacement(0,
			G4ThreeVector(lay_x,0,lay_z),
			AbsLayerLogical_term,
			G4String("Terminating Absorber") + G4String(sterm.str()),
			WorldLog,
			0,
			nterm+1);
      
    }
    
  }

  
}


G4bool TBhcal04::BuildHcal()
{
  //We have to place the Hcal layers into the world in order to cope
  //with the various configuration (i.e. impact) angles

  for (G4int i = 1; i<=n_layers; i++)
  {
    // G4cout << "Placing layer : " << i << G4endl;
    // PlaceLayer(DetectorLogical, i);
       PlaceLayer(WorldLogical, i);
  }

  
  return true;
}


void TBhcal04::SetSD()
{
  // create SD 
  hcalSD = new TBSD_VCell03("hcalSD",
			    GetGridSize(),
			    ncell_xy[0],
			    ncell_xy[1],
			    GetDepthToLayer(),
			    TBHCAL);

  // register
  RegisterSensitiveDetector(hcalSD);
}

void TBhcal04::Print()
{
  G4cout << "\nTBhcal04 parameters: " << G4endl
	 << "n_layers: " << n_layers << G4endl
    //	 << "layer_hthickness: " << layer_hthickness << G4endl
         << "grid size: " << grid_size << G4endl
         << "z_place Hcal: " << z_place << G4endl 
         << "ncell in x: " << ncell_xy[0] << G4endl
         << "ncell in y: " << ncell_xy[1] << G4endl
	 << "cal_hx: " << cal_hx << G4endl
	 << "cal_hy: " << cal_hy << G4endl
	 << "cal_hz: " << cal_hz << G4endl
         << "Configuration Angle: " << config_angle/deg << G4endl
	 << G4endl;       
}


void TBhcal04::AddMaterial() {
  //CRP This is the place to add new materials to the simulation.
  //    Since the element definitions made in the CGA Geometry manager is 
  //    not publically available we have to redefine them here
  G4double a, z, density, fractionmass;
  G4String name, symbol;
  G4int nel,natoms;

  //Hydrogen 
  a = 1.001*g/mole;
  G4Element* elH  = new G4Element(name="Hydrogen"  ,symbol="H" , z= 1., a);

  // Carbon
  a = 12.01*g/mole;
  G4Element* elC  = new G4Element(name="Carbon"  ,symbol="C" , z= 6., a);

  // Oxygen
  a = 16.*g/mole;
  G4Element* elO = new G4Element(name="Oxygen",   symbol="O",  z=8.,  a);

 // Chlorine
  a = 35.45*g/mole;
  G4Element* elCl = new G4Element(name="Chlorine",   symbol="Cl",  z=17.,  a);

  //Bromine
  a = 79.905*g/mole;
  G4Element* elBr  = new G4Element(name="Bromine",symbol="Br" , z= 35., a);

  G4cout << "New Materials for TBhcal03 implementation" << G4endl;

 // PCB (Printed Circuit Board) Material FR4
 //Composition and density found under 
 //http://pcba10.ba.infn.it/temp/ddd/ma/materials/list.html
  density = 1.025 *g/cm3;
  PCB = new G4Material(name="PCB", density, nel=5);
  PCB->AddMaterial(CGAGeometryManager::GetMaterial("silicon_2.33gccm"), fractionmass=0.180774);
  PCB->AddElement(elO, fractionmass=0.405633);
  PCB->AddMaterial(CGAGeometryManager::GetMaterial("graphite"), fractionmass=0.278042);
  PCB->AddElement(elH, fractionmass=0.0684428);
  PCB->AddElement(elBr, fractionmass=0.0671091);
  G4cout << "PCB->GetRadlen() = " << PCB->GetRadlen() /mm   << " mm\n";

 //The steel we are going to use in the Hcal: Material S235 (old name St37)
 //Numbers found under 
 //http://n.ethz.ch/student/zwickers/ download/fs_pe_grundlagen_cyrill.pdf 
  density = 7.85*g/cm3;
  S235 = new G4Material(name="S235", density, nel=2);
  S235->AddMaterial(CGAGeometryManager::GetMaterial("iron"), fractionmass=0.998);
  S235->AddMaterial(CGAGeometryManager::GetMaterial("graphite"), fractionmass=0.002);
  G4cout << "S235->GetRadlen() = " << S235->GetRadlen() /mm   << " mm\n";

  //Now, we're composing the cable-fibre mix
  //We start with PVC the main component of the coax cables
  //Numbers from http://www.elpac.de/Kunststoffkleinteile/Kleines_Kunststoff-Know-_How/PVC-P/pvc-p.html
  //and 
  density = 1.35 *g/cm3;
  G4Material* PVC = new G4Material(name="PCB", density, nel=3);
  PVC->AddElement(elH, natoms=3);
  PVC->AddElement(elC, natoms=2);
  PVC->AddElement(elCl, natoms=1);
  G4cout << "PVC->GetRadlen() = " << PVC->GetRadlen() /mm   << " mm\n";
  
  //...and continuing with Polystyrole, an approximation for the
  //Scintillating fibres and for the 3M foils 
  //Numbers from http://de.wikipedia.org/wiki/Polystyrol
  //the structural formula for the Styrene Polymer is C6H5CH=CH2
  //The difference to Styropor definition in CGAGeometryManager
  //comes since we do not have the material in a foamed form
  density = 1.065 *g/cm3;
  Polystyrole = new G4Material(name="Polystyrole", density, nel=2);
  Polystyrole->AddElement(elH, natoms=8);
  Polystyrole->AddElement(elC, natoms=8);
  G4cout << "Polystyrole->GetRadlen() = " << Polystyrole->GetRadlen() /mm   << " mm\n";

  //Now we define the material cf_mix
  //We assume the following:
  //a) a layer has a volume of V_total = 90x90x0.15 cm^3 = 1215 cm^3(last number is
  //   longitudinal space reserved for cable fibre mix)
  //b) coax cable has diameter of 0.12 cm 
  //   fibre has diameter of 0.5 cm
  //   The cables are on average 45 cm long 
  //   => V_coax = PI*(0.06 cm)^2*45 cm = 0.510 cm^3
  //   => V_fibre = PI*(0.025 cm)^2*45 cm = 0.088 cm^3
  // ...The rest is occupied by air 
  //    V_air = V_total - V_coax - V_fibre = 1214.402
  //  There is one coax. cable and one fibre per tile
  //  and we have on average 185 tiles per layer
  //=> Total mass of coax cable (fibre), m = density*V
  //   m_coax = (1.35*0.510)*185 = 127.37 g
  //   m_fibre = (1.065*0.088)*185 = 17.33 g
  //   ... and
  //  m_air = 1214.402*1.29e-03 = 1.45 g
  // total density = (m_air + m_coax + m_fibre)/1215. = 0.120 g/cm^3 
  density = 0.120*g/cm3; 
  CF_MIX = new G4Material(name="Cable Fibre Mix", density, nel=3);
  CF_MIX->AddMaterial(CGAGeometryManager::GetMaterial("air"), fractionmass = 0.009);
  CF_MIX->AddMaterial(PVC, fractionmass = 0.872);
  CF_MIX->AddMaterial(Polystyrole, fractionmass = 0.119);
  G4cout << "CF_MIX->GetRadlen() = " << CF_MIX->GetRadlen() /mm   << " mm\n";
}





G4bool TBhcal04::isValidSensitiveLayerPattern(G4String sensitiveLayerPattern) {

  if ( ((G4int)sensitiveLayerPattern.length()) != n_layers ) return false;
  
  G4bool valid = true;

  for(G4int i = 0; i < n_layers; ++i) {
    
    if ( (sensitiveLayerPattern.data()[i]!='0') && (sensitiveLayerPattern.data()[i]!='1') ) {
      
      valid = false;
      break;
      
    }

  }

  return valid;
  
}

std::vector<G4int> TBhcal04::getSensitiveLayerPatternVector(G4String sensitiveLayerPattern) {

  
  std::vector<G4int> patternVector;

  for(G4int i = 0; i < n_layers; ++i) {

    G4String digit(sensitiveLayerPattern.data()[i]);
    patternVector.push_back(std::atoi(digit.data()));

  }

  return patternVector;

}
