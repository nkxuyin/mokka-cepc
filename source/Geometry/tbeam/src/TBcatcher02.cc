#include "TBcatcher02.hh"
#include "TBCellReplication.hh"

#include "Control.hh"
#include "CGADefs.h"
#include "MySQLWrapper.hh"
#include "CGAGeometryManager.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"

INSTANTIATE(TBcatcher02)

TBcatcher02::~TBcatcher02()
{}

G4bool TBcatcher02::construct(const G4String &aSubDetectorDBName,
			      G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding TBcatcher02..." << G4endl;
  db = new Database(aSubDetectorDBName.data());

  WorldLogical = WorldLog;
  
  FetchAll();
  BuildElements();
  Print();

  G4bool cokay = BuildCatcher(); 
  delete db;
  db = 0;

  G4cout << "\nDone building TBcatcher02" << G4endl;
  
  return cokay;
}

void TBcatcher02::FetchAll()
{
  db->exec("select * from catcher_virt;");
  db->getTuple();
  n_layers = db->fetchInt("n_layers");
  y_place = db->fetchDouble("y_place");
  
  grid_size=db->fetchDouble("grid_size");

  ncell_xz[0]=db->fetchInt("ncell_x");
  ncell_xz[1]=db->fetchInt("ncell_z");

  db->exec("select * from layer_thickness;");
  db->getTuple();
  poly_hthickness = db->fetchDouble("poly_thickness")/2;
  steel_hthickness = db->fetchDouble("steel_thickness")/2;
}


void TBcatcher02::BuildElements()
{

  // set sensitive detector
  SetSD();

  // catcher dims
  layer_hthickness = steel_hthickness + poly_hthickness;
  cal_hx = (ncell_xz[0] * grid_size)/2;
  cal_hy = n_layers * layer_hthickness;
  cal_hz = (ncell_xz[1] * grid_size)/2;

  // materials
  poly = CGAGeometryManager::GetMaterial("polystyrene");
  steel = CGAGeometryManager::GetMaterial("stainless_steel");

  // detector
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

  // whole layer
  G4Box *LayerSolid = new G4Box("WholeLayerSolid",
				cal_hx,
				layer_hthickness,
				cal_hz);

  LayerLogical = new G4LogicalVolume(LayerSolid,
				     steel,
				     "LayerLogical",
				     0,
				     0,
				     0);
  // steel
  G4Box *SteelSolid = new G4Box("SteelSolid",
				cal_hx,
				steel_hthickness,
				cal_hz);

  SteelLogical = new G4LogicalVolume(SteelSolid,
				     steel,
				     "SteelLogical",
				     0,
				     0,
				     0);

  // poly
  G4Box *PolySolid = new G4Box("PolySolid",
			       cal_hx,
			       poly_hthickness,
			       cal_hz);

  PolyLogical = new G4LogicalVolume(PolySolid,
				    poly,
				    "PolyLogical",
				    0,
				    0,
				    0);

  PolyLogical->SetSensitiveDetector(catcherSD);
  // cell definitions

  // horizontal (along Z)
  //G4Box *CellHorSolid = new G4Box("CellHorSolid",
  //			  cell_hwidth,
  //			  poly_hthickness,
  //			  cal_hz);
  
  //CellHorLogical = new G4LogicalVolume(CellHorSolid,
  //			       poly,
  //			       "CellHorSolid",
  //			       0,
  //			       0,
  //			       0);

  //CellHorLogical->SetSmartless(100.);
  //CellHorLogical->SetVisAttributes(G4VisAttributes::Invisible);
  //CellHorLogical->SetSensitiveDetector(catcherSD);

  // vertical (along X)
  //G4Box *CellVertSolid = new G4Box("CellVertSolid",
  //				   cal_hx,
  //				   poly_hthickness,
  //				   cell_hwidth);

  //  CellVertLogical = new G4LogicalVolume(CellVertSolid,
  //				poly,
  //				"CellVertSolid",
  //				0,
  //				0,
  //				0);

  // CellVertLogical->SetSmartless(100.);
  //CellVertLogical->SetVisAttributes(G4VisAttributes::Invisible);

  //CellVertLogical->SetSensitiveDetector(catcherSD);

  // vertical layer
  //PolyVertLogical = new G4LogicalVolume(PolySolid,
  //					poly,
  //				"PolyVertLogical");

  // replicate vertical cells
  //TBCellReplication *CellRep = new TBCellReplication(PolyVertLogical, CellVertLogical);
  //CellRep->Replicate();
  //delete CellRep;

  // hor layer
  //PolyHorLogical = new G4LogicalVolume(PolySolid,
//				       poly,
//				       "PolyHorLogical");

  // replicate horizontal cells
  //CellRep = new TBCellReplication(PolyHorLogical, CellHorLogical);
//CellRep->Replicate();
//delete CellRep; CellRep=0;
}


G4bool TBcatcher02::BuildCatcher()
{
  for (G4int i=1; i <= n_layers; i++)
  {
    BuildLayer(DetectorLogical, i);
  }
  
  // G4PVPlacement *DetectorPhys = 
  new G4PVPlacement(0,
		    G4ThreeVector(0,y_place,0),
		    DetectorLogical,
		    "CatcherDetectorPhys",
		    WorldLogical,
		    false,
		    0);
  
  return true;
}


void TBcatcher02::BuildLayer(G4LogicalVolume *DetLog, G4int nlay)
{
  G4cerr << "Building catcher layer <" << nlay << ">." << G4endl;

  G4double lay_y = cal_hy - layer_hthickness;

  if (nlay > 1)
    lay_y -= ((nlay-1) * (layer_hthickness*2));

  G4PVPlacement *LayerPhys = new G4PVPlacement(0,
					       G4ThreeVector(0,lay_y,0),
					       LayerLogical,
					       "LayerPhys",
					       DetLog,
					       0,
					       nlay);


 
  
  // G4PVPlacement *PolyPhys = 
  new G4PVPlacement(0,
		    G4ThreeVector(0,layer_hthickness-poly_hthickness,0),
		    "PolyPhys",
		    PolyLogical,
		    LayerPhys,
		    false,
		    0);
  

  // G4PVPlacement *SteelPhys = 
  new G4PVPlacement(0,
		    G4ThreeVector(0,-poly_hthickness,0),
		    "SteelPhys",
		    SteelLogical,
		    LayerPhys,
		    false,
		    0);
}

void TBcatcher02::SetSD()
{
  catcherSD = new TBSD_VCell02("catcherSD",this);  
  RegisterSensitiveDetector(catcherSD);
}

void TBcatcher02::Print()
{
  G4cout << "TBcatcher info: " << G4endl
	 << "n_layers: " << n_layers << G4endl 
	 << "y_place: " << y_place << G4endl 
	 << "cal_hx: " << cal_hx << G4endl
	 << "cal_hy: " << cal_hy << G4endl
	 << "cal_hz: " << cal_hz << G4endl
    //<< "layer_start: " << layer_config << G4endl
    //<< "cell_hwidth: " << cell_hwidth << G4endl
	 << "poly_hthickness: " << poly_hthickness << G4endl
	 << "steel_hthickness: " << steel_hthickness << G4endl
	 << G4endl;
}
