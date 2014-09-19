//Author Roman Poeschl DESY
//Routines adapted from Jeremy McCormick, NIU
#include "Control.hh"
#include "TBhcal02.hh"
//#include "TBCellReplication.hh"

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

// #define NO_VIS_HCAL 1


// definne some levels of detail for graphical display
#define DM_FULL 3
#define DM_ABSORBERANDSENSITIVE 2
#define DM_WHOLELAYERONLY  1



INSTANTIATE(TBhcal02)

TBhcal02::~TBhcal02()
{}

G4bool TBhcal02::construct(const G4String &aSubDetectorDBName,
			   G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding TBhcal02..." << G4endl;
  db = new Database(aSubDetectorDBName.data());

  WorldLogical = WorldLog;    
  
  FetchAll();
  AddMaterial();
  BuildElements();
  Print();

  G4bool cokay = BuildHcal();

  delete db;
  db = 0;

  G4cout << "\nDone building TBhcal02" << G4endl;
  
  return cokay;
}

// fetch MySQL variables
void TBhcal02::FetchAll()
{
  
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
  ncell_xz[0] = db->fetchInt("ncell_x");
  //number of cells in z
  ncell_xz[1] = db->fetchInt("ncell_z");
  assert(ncell_xz[0] >= 0 || ncell_xz[1] >= 0);
  //grid size
  grid_size = db->fetchDouble("grid_size");
  //the place where we'll put the calorimeter 
  y_place = db->fetchDouble("y_place");
  

  // thicknesses
  db->exec("select * from layer_thickness;");
  db->getTuple();

  poly_hthickness = db->fetchDouble("poly_thickness")/2;
  steel_hthickness = db->fetchDouble("steel_thickness")/2;
  airgap_hthickness = db->fetchDouble("air_gap")/2;
  steel_cassette_hthickness = db->fetchDouble("steel_cassette_thickness")/2.;
  foil_hthickness = db->fetchDouble("foil_thickness")/2.;
  pcb_hthickness = db->fetchDouble("pcb_thickness")/2.;
  cablefibre_mix_hthickness = db->fetchDouble("cablefibre_mix_thickness")/2.;
}

// define calculated vars, materials, solids, LV
void TBhcal02::BuildElements()
{
  // hcal dims
  layer_hthickness =
    poly_hthickness+steel_hthickness+2.0*airgap_hthickness+2.0*steel_cassette_hthickness+
    foil_hthickness+pcb_hthickness+cablefibre_mix_hthickness;
  G4cout << "Layer Thickness: " << layer_hthickness*2. << " mm" << G4endl;
  G4cout << "Steel Thickness: " << steel_hthickness*2. << " mm" << G4endl;
  
  // create and register SD
  SetSD();
  // materials
  poly = CGAGeometryManager::GetMaterial("polystyrene");
  steel = CGAGeometryManager::GetMaterial("stainless_steel");
  air = CGAGeometryManager::GetMaterial("air");
  alu = CGAGeometryManager::GetMaterial("aluminium");
  pcb = TBhcal02::PCB;


  cal_hx = (G4double) (ncell_xz[0] * grid_size*mm)/2.;
  cal_hz = (G4double) (ncell_xz[1] * grid_size*mm)/2.;
  cal_hy = (G4double) (n_layers * 2.*layer_hthickness)/2.;
  //Create the detector step by step
  // whole detector
  G4cout << "cal_hx =  " << cal_hx << G4endl; 
  G4cout << "cal_hz =  " << cal_hz << G4endl; 
  G4cout << "cal_hy =  " << cal_hy << G4endl; 
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
				     layer_hthickness,
				     cal_hz);

  WholeLayerLogical = new G4LogicalVolume(WholeLayerSolid,
					  air,
					  "WholeLayerLogical",
					  0,
					  0,
					  0);
  
#ifdef NO_VIS_HCAL
  WholeLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif

  //Absorber Plate
  G4Box *AbsLayerSolid = new G4Box("AbsLayerSolid",
				     cal_hx,
				     steel_hthickness,
				     cal_hz);

  AbsLayerLogical = new G4LogicalVolume(AbsLayerSolid,
					  steel,
					 "AbsLayerLogical",
					  0,
					  0,
					  0);


  int displayMode = UserInit::getInstance()->getInt("DisplayMode") ;
  if( displayMode == 0 )  // if nothing specified display full details
    displayMode = DM_FULL ;

  std::cout << " using display mode - vele of detail : " << displayMode << std::endl ;


#ifdef NO_VIS_HCAL
  AbsLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif
  if( displayMode < DM_ABSORBERANDSENSITIVE )  
    AbsLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
    

  
  //the scintillator housing (made of steel)       

  G4Box *ScinHousSolid = new G4Box("ScinHousSolid",
				     cal_hx,
				     steel_cassette_hthickness,
				     cal_hz);

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
					 poly_hthickness,
					 cal_hz);

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

  //alu foil
  G4Box *AluLayerSolid = new G4Box("ALuLayerSolid",
					 cal_hx,
					 foil_hthickness,
					 cal_hz);

  AluLayerLogical = new G4LogicalVolume(AluLayerSolid,
					      alu,
					      "AluLayerLogical",
					      0,
					      0,
					      0);
   
#ifdef NO_VIS_HCAL
  AluLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif
  if( displayMode < DM_FULL )  
    AluLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);


  //a pcb layer
  G4Box *PCBLayerSolid = new G4Box("PCBLayerSolid",
					 cal_hx,
					 pcb_hthickness,
					 cal_hz);

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


  //the cablefibre mixture (still treated as air)

  G4Box *CFmix_LayerSolid = new G4Box("CFmix_LayerSolid",
					 cal_hx,
					 cablefibre_mix_hthickness,
					 cal_hz);

  CFmix_LayerLogical = new G4LogicalVolume(CFmix_LayerSolid,
					      air,
					      "CFmix_LayerLogical",
					      0,
					      0,
					      0);
   
#ifdef NO_VIS_HCAL
  CFmix_LayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif
  if( displayMode < DM_FULL )  
    CFmix_LayerLogical->SetVisAttributes(G4VisAttributes::Invisible);

  //Put absorber plate into the layer
  G4double pos_abs = (layer_hthickness-steel_hthickness); 
  G4cout << "Layer Thickness 1: " << layer_hthickness*2. << " mm" << G4endl;
  G4cout << "Steel Thickness 1: " << steel_hthickness*2. << " mm" << G4endl;
  G4cout << "Absorber Plate at: " << pos_abs << " mm" << G4endl;

  // G4PVPlacement *AbsLayerPhys = 
  new G4PVPlacement(0,
		    G4ThreeVector(0,pos_abs,0),
		    AbsLayerLogical,
		    "AbsLayerPhys",
		    WholeLayerLogical,                                
		    0,
		    0);

  //Put scintillator housing front plate into the layer (after absorber and before scintillator)
  G4double pos_house1 = pos_abs -steel_hthickness
    -2.0*airgap_hthickness -steel_cassette_hthickness;
  G4cout << "Scintillator Housing Frontplate at: " << pos_house1 << " mm" << G4endl;


  G4PVPlacement *ScinHousPhys = new G4PVPlacement(0,
			             G4ThreeVector(0,pos_house1,0),
						   ScinHousLogical,
						   "ScinHousPhys Front",
                                                   WholeLayerLogical,                                
						   0,
						   0);


  //Put sensitive part (i.e. scintillator plate) into the layer

  G4double pos_sens = pos_house1 - steel_cassette_hthickness - poly_hthickness; 
  G4cout << "Scintillating Tiles at: " << pos_sens << " mm" << G4endl;

  //G4PVPlacement *SensLayerPhys = 
  new G4PVPlacement(0,
		    G4ThreeVector(0,pos_sens,0),
		    WholeSensLayerLogical,
		    "SensLayerPhys",
		    WholeLayerLogical,                                
		    0,
		    0);


  //Put alu foil into the layer

  G4double pos_alu = pos_sens - poly_hthickness - foil_hthickness; 
  G4cout << "Alu Foil at: " << pos_alu << " mm" << G4endl;
  // G4PVPlacement *AluLayerPhys = 
  new G4PVPlacement(0,
		    G4ThreeVector(0,pos_alu,0),
		    AluLayerLogical,
		    "SensLayerPhys",
		    WholeLayerLogical,                                
		    0,
		    0);

  //Put the PCB into the layer

  G4double pos_pcb = pos_alu - foil_hthickness - pcb_hthickness; 
  G4cout << "PCB at: " << pos_pcb << " mm" << G4endl;
  // G4PVPlacement *PCBLayerPhys = 
  new G4PVPlacement(0,
		    G4ThreeVector(0,pos_pcb,0),
		    PCBLayerLogical,
		    "SensLayerPhys",
		    WholeLayerLogical,                                
		    0,
		    0);


  //Put the Cable-Fibre mixture into the layer

  G4double pos_cfmix = pos_pcb - pcb_hthickness - cablefibre_mix_hthickness; 
  G4cout << "Cable-Fibre mixture at: " << pos_cfmix << " mm" << G4endl;
  // G4PVPlacement *CFmix_LayerPhys = 
  new G4PVPlacement(0,
		    G4ThreeVector(0,pos_cfmix,0),
		    CFmix_LayerLogical,
		    "CFmix_LayerPhys",
		    WholeLayerLogical,                                
		    0,
		    0);



  //Put scintillator housing rear plate into the layer
  G4double pos_house2 = pos_cfmix - cablefibre_mix_hthickness -steel_cassette_hthickness;
  G4cout << "Scintillator Housing Rearplate at: " << pos_house2 << " mm" << G4endl;
                  ScinHousPhys = new G4PVPlacement(0,
			             G4ThreeVector(0,pos_house2,0),
						   ScinHousLogical,
						   "ScinHousPhys Rear",
                                                   WholeLayerLogical,                                
						   0,
						   1);


}

void TBhcal02::PlaceLayer(G4LogicalVolume *DetLog, G4int nlay)
{


  //Calculate y-position of layer
 G4double lay_y = cal_hy - layer_hthickness;
  
  if (nlay > 1)
    lay_y -= ((nlay-1) * (layer_hthickness*2));

  G4cout << "Y Place: " << lay_y << G4endl;

  std::stringstream slay;
  slay << nlay;

  //place layer into logical volume reserved for the HCAL
  // G4PVPlacement *WholeLayerPhys = 
  new G4PVPlacement(0,
		    G4ThreeVector(0,lay_y,0),
		    WholeLayerLogical,
		    G4String("WholeLayerPhys") + G4String(slay.str()),
		    DetLog,
		    0,
		    nlay);

  
}


G4bool TBhcal02::BuildHcal()
{
  //Set the detector into the world
  // G4PVPlacement* DetectorPhys = 
  new G4PVPlacement(0,
		    G4ThreeVector(0,y_place,0),
		    DetectorLogical,
		    "HcalDetectorPhys",
		    WorldLogical,
		    0,
		    0);

  for (G4int i = 1; i<=n_layers; i++)
  {
    G4cout << "Placing layer : " << i << G4endl;
    PlaceLayer(DetectorLogical, i);
  }

  return true;
}


void TBhcal02::SetSD()
{
  G4cerr << "cal_hx: " << cal_hx << G4endl;
  G4cerr << "cal_hz: " << cal_hz << G4endl;
  hcalSD = new TBSD_VCell02("hcalSD", this);
  RegisterSensitiveDetector(hcalSD);
}

void TBhcal02::Print()
{
  G4cout << "\nTBhcal02 parameters: " << G4endl
	 << "n_layers: " << n_layers << G4endl
         << "grid size: " << grid_size << G4endl
         << "ncell in x:" << ncell_xz[0] << G4endl
         << "ncell in z:" << ncell_xz[1] << G4endl
	 << "poly_hthickness: " << poly_hthickness << G4endl
	 << "steel_hthickness: " << steel_hthickness << G4endl
	 << G4endl;       
}


void TBhcal02::AddMaterial() {
  //CRP This is the place to add new materials to the simulation.
  //    Since the element definitions made in the CGA Geometry manager is 
  //    not publically available we have to redefine them here
  G4double a, z, density;
  G4String name, symbol;
  G4int nel,natoms;

 // Chlorine
  a = 35.45*g/mole;
  G4Element* elCl = new G4Element(name="Chlorine",   symbol="Cl",  z=17.,  a);

   // Hydrogen
  a = 1.01*g/mole;
  G4Element* elH  = new G4Element(name="Hydrogen",symbol="H" , z= 1., a);

   // Carbon
  a = 12.01*g/mole;
  G4Element* elC  = new G4Element(name="Carbon"  ,symbol="C" , z= 6., a);

 // PCB i.e Arochlor 1242
  density = 1.35 *g/cm3;
  PCB = new G4Material(name="PCB", density, nel=3);
  PCB->AddElement(elH, natoms=6);
  PCB->AddElement(elC, natoms=12);
  PCB->AddElement(elCl, natoms=4);
  G4cout << "PCB->GetRadlen() = " << PCB->GetRadlen() /mm   << " mm\n";
}



