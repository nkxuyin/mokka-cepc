#include "Control.hh"
#include "wscal.hh"




// define some levels of detail for graphical display
#define DM_FULL 3
#define DM_ABSORBERANDSENSITIVE 2
#define DM_WHOLELAYERONLY  1

//create an instance of class which is registered vs. the
//Mokka management system for the drivers 
INSTANTIATE (wscal)

  wscal::~wscal() {/*no op*/}

G4bool wscal::construct(const G4String &aSubDetectorDBName,
			G4LogicalVolume *WorldLog) {

  //Fetch database constants

  //open database (database opening routines provided by a MySQL wrapper
  //implemented in Mokka
  db = new Database(aSubDetectorDBName.data());
  
  //Set the datamember WorldLogical
  WorldLogical = WorldLog;
  
  FetchdbEntries();
  AddMaterial();
  BuildElements();
  G4bool cokay = Build_wscal();
  
  return cokay;
}

void wscal::FetchdbEntries(){
  //fetch geometry constants (might be more elegant to do this in a
  //separate routine)
  //provide access to database table block
  db->exec("select * from wscal;");
  db->getTuple();
  //number of cells in x
  ncell_xy[0] = db->fetchInt("ncell_x");
  //number of cells in y
  ncell_xy[1] = db->fetchInt("ncell_y");
  assert(ncell_xy[0] >= 0 || ncell_xy[1] >= 0);
  n_layers = db->fetchInt("n_layers");
  assert(n_layers > 0);
  //size of a cell
  cell_size = db->fetchDouble("cell_size");
  //beginning of calorimeter
  z_begin = db->fetchDouble("z_begin");
  //material thicknesses
  db->exec("select * from wscal_layer_thickness;");
  db->getTuple();
  poly_hthickness = db->fetchDouble("poly_thickness")/2;
  steel_hthickness = db->fetchDouble("steel_thickness")/2;
  //Layer dimensions
  layer_hthickness = poly_hthickness + steel_hthickness;   
  
  cal_hx = (G4double) (ncell_xy[0] * cell_size*mm)/2.;
  cal_hy = (G4double) (ncell_xy[1] * cell_size*mm)/2.;
  cal_hz = (G4double) (n_layers * layer_hthickness);
  
}


void wscal::BuildElements() {


  // create and register SD
  SetSD();


  //Set displayMode 
  int displayMode = UserInit::getInstance()->getInt("DisplayMode") ;
  if( displayMode == 0 )  // if nothing specified display full details
    displayMode = DM_FULL ;
  
  G4cout << " using display mode - vele of detail : " << displayMode << G4endl ;
  
  //Create logical volumes 
  
  //an air box which fill held the layer components   
  //whole layer
  air = CGAGeometryManager::GetMaterial("air");
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
  
#ifdef NO_VIS_WSCAL
  WholeLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif
  
  
  
  //logical volumes for the absorber and the scintillator to be filled 
  //into the logical volume of the layer as defined above
  
  //Absorber Plate made of steel
  steel = S235;
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
  
  
  if( displayMode < DM_ABSORBERANDSENSITIVE ) {
    AbsLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
  }
#ifdef NO_VIS_WSCAL
  AbsLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif

  // a layer of sensitive material 
  poly = CGAGeometryManager::GetMaterial("polystyrene");
  G4Box *SenseLayerSolid = new G4Box("SenseLayerSolid",
				     cal_hx,
				     cal_hy,
				     poly_hthickness);
  
  SenseLayerLogical = new G4LogicalVolume(SenseLayerSolid,
					  poly,
					  "SenseLayerLogical",
					  0,
					  0,
					  0);
  
  if( displayMode < DM_ABSORBERANDSENSITIVE )  
    SenseLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#ifdef NO_VIS_WSCAL
  SenseLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif
  
  // a volume of sensitive material e.g. a tile
  G4Box *SenseCellSolid = new G4Box("SenseCellSolid",
				    cell_size/2.,
				    cell_size/2.,
				    poly_hthickness);
  
  SenseCellLogical = new G4LogicalVolume(SenseCellSolid,
					 poly,
					 "SenseCellLogical",
					 0,
					 0,
					 0);
  
  G4VisAttributes* visSenseCell = new G4VisAttributes();
  visSenseCell->SetColour(0., 1., 0., 1.);
  //For those who want to play around with wisualization attributes
  //visSenseCell->SetForceSolid(true);
  SenseCellLogical->SetVisAttributes(visSenseCell);
  if( displayMode < DM_FULL )  
    SenseCellLogical->SetVisAttributes(G4VisAttributes::Invisible);
#ifdef NO_VIS_WSCAL
  SenseCellLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif
  
  //define a (helper) volume for the rows 
  
  G4Box *RowSolid = new G4Box("RowSolid",
                              0.5*cell_size*ncell_xy[0],
                              0.5*cell_size,
                              poly_hthickness);
  
  RowLogical = new G4LogicalVolume(RowSolid,
                                   SenseCellLogical->GetMaterial(),
                                   "RowLogical",
                                   0,
                                   0,
                                   0);          
  
  RowLogical->SetVisAttributes(G4VisAttributes::Invisible);
  
  //Replicate the cells within a row
  //check whether cells fit into row 
  
  G4double n_whole;
  
  G4double cell_hx = SenseCellSolid->GetXHalfLength();
  G4double row_hx  = RowSolid->GetXHalfLength();
  G4double rem = std::modf(row_hx/cell_hx,&n_whole);
  
  //stop program if given number of cells doesn't fit into the row
  assert( (rem == 0) || (G4int) n_whole == ncell_xy[0]); 
  
  
  new G4PVReplica("CellReplica",
		  SenseCellLogical,
		  RowLogical,
		  kXAxis,
		  ncell_xy[0],
		  cell_size,
		  0);
  
  //Replicate the rows within the layer
  //check whether row of cells fits into height of calo 
  G4double layer_hy = SenseLayerSolid->GetYHalfLength();
  G4double row_hy  = RowSolid->GetYHalfLength();
  rem = std::modf(layer_hy/row_hy,&n_whole);
  //stop program if given number of rows don't fit into the layer
  //in height
  assert( (rem == 0) || (G4int) n_whole == ncell_xy[1]); 
  
  
  new G4PVReplica("LayerReplica",
		  RowLogical,
		  SenseLayerLogical,
		  kYAxis,
		  ncell_xy[1],
		  cell_size,
		  0);

  
  
  if( displayMode < DM_FULL )  
    SenseCellLogical->SetVisAttributes(G4VisAttributes::Invisible);
#ifdef NO_VIS_HCAL
  SenseCellLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif
  
  //Declare the poly layer to be sensitive 
  SenseCellLogical->SetSensitiveDetector(wscal_SD);
  
  //Put the layer together
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
  
  //Put sensitive part(s) into the layer
  G4double pos_sens = pos_abs + steel_hthickness + poly_hthickness; 
  G4cout << "Scintillating Tiles at: " << pos_sens << " mm" << G4endl;
  
  
  new G4PVPlacement(0,
                    G4ThreeVector(0,0,pos_sens),
                    SenseLayerLogical,
                    "SensLayerPhys",
                    WholeLayerLogical,                                
                    0,
                    0);
  
  
}



G4bool wscal::Build_wscal() {

  //position of first layer
  G4double lay_z = z_begin + layer_hthickness;
  //Put the layers into the world
  for (int nLay = 1; nLay < n_layers+1; nLay++) {
    
    std::stringstream slay;
    slay << nLay;
    
    
    //place layers into the world 
    if (nLay > 1) lay_z += layer_hthickness*2.0; 
    G4cout << "Lay_z= " << lay_z << G4endl;
    new G4PVPlacement(0,
		      G4ThreeVector(0, 0, lay_z),
		      WholeLayerLogical,
		      G4String("WholeLayerPhys") + G4String(slay.str()),
		      WorldLogical,
		      0,
		      nLay);
    
  } 
  
  return true;
}



void wscal::SetSD() {
  
  //create an instance of a sensitive detector class
  //the actual registration of all sensitive
  //detectors to G4 is done via a Mokka interface,
  //so we create an instance of our sens. detector class 
  //and make this
  //known to Mokka 
  wscal_SD = new wscalSD("wscal_SD",
			 GetCellSize(),
			 ncell_xy[0],
			 ncell_xy[1],
			 //WSCALO defined in Mokka/source/Kernel/include/Control.hh
			 0);
  
  RegisterSensitiveDetector(wscal_SD);
}


void wscal::AddMaterial() {
  //Variables to declare a material to G4
  G4double density, fractionmass;
  G4String name;
  G4int nel;
 
  //The steel we are going to use in our small calo: Material S235 (old name St37)
  //Numbers found under 
  //http://n.ethz.ch/student/zwickers/ download/fs_pe_grundlagen_cyrill.pdf 
  density = 7.85*g/cm3;
  S235 = new G4Material(name="S235", density, nel=2);
  S235->AddMaterial(CGAGeometryManager::GetMaterial("iron"), fractionmass=0.998);
  S235->AddMaterial(CGAGeometryManager::GetMaterial("graphite"), fractionmass=0.002);
  G4cout << "S235->GetRadlen() = " << S235->GetRadlen() /mm   << " mm\n";
  
  
}
