#include "Control.hh"
#include "TBhcal01.hh"
#include "TBCellReplication.hh"

#include "CGADefs.h"
#include "MySQLWrapper.hh"

#include "CGAGeometryManager.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4PVParameterised.hh"

#include <sstream>

// #define NO_VIS_HCAL 1

INSTANTIATE(TBhcal01)

TBhcal01::~TBhcal01()
{}

G4bool TBhcal01::construct(const G4String &aSubDetectorDBName,
			   G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding TBhcal01..." << G4endl;
  db = new Database(aSubDetectorDBName.data());

  WorldLogical = WorldLog;    
  
  FetchAll();
  BuildElements();
  Print();

  G4bool cokay = BuildHcal();

  delete db;
  db = 0;

  G4cout << "\nDone building TBhcal01" << G4endl;
  
  return cokay;
}

// fetch MySQL variables
void TBhcal01::FetchAll()
{
  // get layering n
  db->exec("select * from hcal;");
  db->getTuple();
  n_layers = db->fetchInt("n_layers");
  assert(n_layers > 0);

  n_complex_layers = db->fetchInt("n_complex");
  assert(n_complex_layers >= 0);

  y_place = db->fetchDouble("y_place");
  
  // inner cells
  db->exec("select * from layer_inner;");
  db->getTuple();

  icell_dim_hx = db->fetchDouble("cell_dim_x")/2;
  icell_dim_hz = db->fetchDouble("cell_dim_z")/2;

  n_icell_x = db->fetchInt("n_cell_x");
  n_icell_z = db->fetchInt("n_cell_z");

  // outer cells
  db->exec("select * from layer_outer;");
  db->getTuple();

  ocell_dim_hx = db->fetchDouble("cell_dim_x")/2;
  ocell_dim_hz = db->fetchDouble("cell_dim_z")/2;
  n_ocell_x = db->fetchInt("n_cell_x");
  n_ocell_z = db->fetchInt("n_cell_z");

  // bordering cells = fixed
  bcell_dim_hx = ocell_dim_hx*2;
  bcell_dim_hz = ocell_dim_hz*2;

  // thicknesses
  db->exec("select * from layer_thickness;");
  db->getTuple();

  poly_hthickness = db->fetchDouble("poly_thickness")/2;
  steel_hthickness = db->fetchDouble("steel_thickness")/2;
}

// define calculated vars, materials, solids, LV
void TBhcal01::BuildElements()
{
  // hcal dims
  layer_hthickness = poly_hthickness+steel_hthickness;

  // outer cal dim from center
  ocal_hx = ocell_dim_hx * (G4double)n_ocell_x;
  ocal_hz = ocell_dim_hz * (G4double)n_ocell_z;

  // full cal dim from center
  cal_hx = (ocell_dim_hx * (G4double)n_ocell_x) + (bcell_dim_hx*2);
  cal_hz = (ocell_dim_hz * (G4double)n_ocell_z) + (bcell_dim_hz*2);
  cal_hy = (layer_hthickness) * (G4double)n_layers;

  // create and register SD
  SetSD();

  // materials
  poly = CGAGeometryManager::GetMaterial("polystyrene");
  steel = CGAGeometryManager::GetMaterial("stainless_steel");

  // cell replicator
  // TBCellReplication *CellRep = 0;

  // inner cell
  G4Box *InnerCellSolid = new G4Box("InnerCellSolid",
				    icell_dim_hx,
				    poly_hthickness,
				    icell_dim_hz);

  CellInnerLogical = new G4LogicalVolume(InnerCellSolid,
					 poly,
					 "CellInnerLogical",
					 0,
					 0,
					 0);
  
  // CellInnerLogical->SetSmartless(100.);

#ifdef NO_VIS_HCAL
  CellInnerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif

  CellInnerLogical->SetSensitiveDetector(hcalSD);

  // outer cell
  G4Box *OuterCellSolid = new G4Box("OuterCellSolid",
				    ocell_dim_hx,
				    poly_hthickness,
				    ocell_dim_hz);

  CellOuterLogical = new G4LogicalVolume(OuterCellSolid,
					 poly,
					 "CellOuterLogical",
					 0,
					 0,
					 0);

#ifdef NO_VIS_HCAL
  CellOuterLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif

  CellOuterLogical->SetSensitiveDetector(hcalSD);

  // border cells
  G4Box *BorderCellSolid = new G4Box("BorderCellSolid",
				     bcell_dim_hx,
				     poly_hthickness,
				     bcell_dim_hz);

  CellBorderLogical = new G4LogicalVolume(BorderCellSolid,
					  poly,
					  "CellBorderLogical",
					  0,
					  0,
					  0);

  CellBorderLogical->SetSensitiveDetector(hcalSD);
  
#ifdef NO_VIS_HCAL
  CellBorderLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif

  // G4int bcell_nx = (G4int)(n_ocell_x/2);  
  // G4int bcell_nz = (G4int)(n_ocell_z/2);
  G4Box *BorderSolid = new G4Box("BorderSolid", 
				 ocal_hx + ocell_dim_hx,
				 poly_hthickness,
				 bcell_dim_hz);

  BorderLogical = new G4LogicalVolume(BorderSolid,
				      poly,
				      "BorderLogical",
				      0,
				      0,
				      0);

#ifdef NO_VIS_HCAL
  BorderLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif
			  
  // replicate cells
  this->ReplicateCells(BorderLogical, CellBorderLogical);

  // whole detector
  G4Box *DetectorSolid = new G4Box("DetectorSolid",
				   cal_hx,
				   cal_hy,
				   cal_hz);

  DetectorLogical = new G4LogicalVolume(DetectorSolid,
					steel,
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
					  steel,
					  "WholeLayerLogical",
					  0,
					  0,
					  0);
  
#ifdef NO_VIS_HCAL
  WholeLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif

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

  // redef for "doubled" quad
  WholeSensQuad = new G4LogicalVolume(WholeSensLayerSolid,
				      poly,
				      "WholeSensQuad",
				      0,
				      0,
				      0);

  ReplicateCells(WholeSensQuad, CellOuterLogical);

  // inner sublayer
  G4double inner_hx = icell_dim_hx * n_icell_z;
  G4double inner_hz = icell_dim_hz * n_icell_z;

  G4Box *InnerSensLayerSolid = new G4Box("InnerSensLayerSolid",
					 inner_hx,
					 poly_hthickness,
					 inner_hz);

  InnerSensLayerLogical = new G4LogicalVolume(InnerSensLayerSolid,
					      poly,
					      "InnerSensLayerLogical",
					      0,
					      0,
					      0);					 

  // #ifdef NO_VIS_HCAL
  InnerSensLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
  //#endif

  // cells for inner 
  ReplicateCells(InnerSensLayerLogical, CellInnerLogical);

  // top/bottom (along Z)
  outer_tb_hx = (ocal_hx - inner_hx)/2;
  outer_tb_hz = ((ocal_hz*2) - (ocal_hz - inner_hz))/2;

  G4Box *TopBottomQuadrantSolid = new G4Box("TopBottomQuadrantSolid",      
					    outer_tb_hx,
					    poly_hthickness,
					    outer_tb_hz);

  TopBottomSensLayerLogical = new G4LogicalVolume(TopBottomQuadrantSolid,
						  poly,
						  "TopBottomSensLayerLogical",
						  0,
						  0,
						  0);

#ifdef NO_VIS_HCAL
  TopBottomSensLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif

  this->ReplicateCells(TopBottomSensLayerLogical, CellOuterLogical);

  // side quad (along X)
  outer_side_hz = (ocal_hz - inner_hz)/2;
  outer_side_hx = ((ocal_hx*2) - (ocal_hx - inner_hx))/2;

  // G4cout << cal_hx*2 << G4endl;
  // G4cout << (cal_hx-inner_hx)/2 << G4endl;

  G4Box *SideQuadrantSolid = new G4Box("SideQuadrantSolid",
				       outer_side_hx,
				       poly_hthickness,
				       outer_side_hz); 

  SideSensLayerLogical = new G4LogicalVolume(SideQuadrantSolid,
					    poly,
					    "SideSensLayerLogical",
					     0,
					     0,
					     0);

  // side quadrant cells
  this->ReplicateCells(SideSensLayerLogical, CellOuterLogical);

#ifdef NO_VIS_HCAL
  SideSensLayerLogical->SetVisAttributes(G4VisAttributes::Invisible);
#endif

  // absorber sublayer
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
}

void TBhcal01::BuildLayer(G4LogicalVolume *DetLog, G4int nlay)
{
  G4bool bcomplex = true;

  if (nlay > (n_complex_layers))
  {
    bcomplex = false;
  }

  G4cout << "Building Complex Layer: " << bcomplex << G4endl;

  G4double lay_y = cal_hy - layer_hthickness;
  
  if (nlay > 1)
    lay_y -= ((nlay-1) * (layer_hthickness*2));

  G4cout << "Y Place: " << lay_y << G4endl;

  std::stringstream slay;
  slay << nlay;

  G4PVPlacement *WholeLayerPhys = new G4PVPlacement(0,
						    G4ThreeVector(0,lay_y,0),
						    WholeLayerLogical,
						    G4String("WholeLayerPhys") + G4String(slay.str()),
						    DetLog,
						    0,
						    nlay);

  
  //G4PVPlacement *AbsLayerPhys = 
  new G4PVPlacement(0,
		    G4ThreeVector(0,-poly_hthickness,0),
		    "AbsLayerPhys",
		    AbsLayerLogical,
		    WholeLayerPhys,
		    0,
		    0);
  
  // G4PVPlacement *SensLayerPhys = 
  new G4PVPlacement(0,
		    G4ThreeVector(0,steel_hthickness,0),
		    "SensLayerPhys",
		    WholeSensLayerLogical,
		    WholeLayerPhys,
		    0,
		    0);

  // use virtual R0 instead in hcal01
  // BuildSensitive(SensLayerPhys, bcomplex);  
}

G4bool TBhcal01::BuildHcal()
{
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
    BuildLayer(DetectorLogical, i);
  }

  return true;
}

void TBhcal01::ReplicateCells(G4LogicalVolume *mLV, G4LogicalVolume *cLV)
{
  TBCellReplication CellRep(mLV, cLV);
  CellRep.Replicate();
}

void TBhcal01::SetSD()
{
  // just do hard-coded 1x1 cm for readout :-) --JM
  G4cerr << "cal_hx: " << cal_hx << G4endl;
  G4cerr << "cal_hz: " << cal_hz << G4endl;
  hcalSD = new TBSD_VCell("hcalSD",1.*cm,1.*cm,cal_hx,cal_hz);
  RegisterSensitiveDetector(hcalSD);
}

void TBhcal01::Print()
{
  G4cout << "\nTBhcal01 parameters: " << G4endl
	 << "n_layers: " << n_layers << G4endl
	 << "n_complex_layers: " << n_complex_layers << G4endl
	 << "icell_dim_hx: " << icell_dim_hx << G4endl
	 << "icell_dim_hz: " << icell_dim_hz << G4endl
	 << "n_icell_x: " << n_icell_x << G4endl
	 << "n_icell_z: " << n_icell_z << G4endl
	 << "ocell_dim_hx: " << ocell_dim_hx << G4endl
	 << "ocell_dim_hz: " << ocell_dim_hz << G4endl
	 << "n_ocell_x: " << n_ocell_x << G4endl
	 << "n_ocell_z: " << n_ocell_z << G4endl
	 << "poly_hthickness: " << poly_hthickness << G4endl
	 << "steel_hthickness: " << steel_hthickness << G4endl
	 << G4endl;       
}
