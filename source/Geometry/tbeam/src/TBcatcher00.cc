#include "Control.hh"
#include "TBcatcher00.hh"
#include "TBCellReplication.hh"

#include "CGADefs.h"
#include "MySQLWrapper.hh"
#include "CGAGeometryManager.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"

INSTANTIATE(TBcatcher00)

TBcatcher00::~TBcatcher00()
{}

G4bool TBcatcher00::construct(const G4String &aSubDetectorDBName,
			      G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding TBcatcher00..." << G4endl;
  db = new Database(aSubDetectorDBName.data());

  WorldLogical = WorldLog;
  
  FetchAll();
  BuildElements();
  Print();

  G4bool cokay = BuildCatcher();
  delete db;
  db = 0;

  G4cout << "\nDone building TBcatcher00" << G4endl;
  
  return cokay;
}

void TBcatcher00::FetchAll()
{
  db->exec("select * from catcher;");
  db->getTuple();
  n_layers = db->fetchInt("n_layers");
  y_place = db->fetchDouble("y_place");
  
  G4String s_start = db->fetchString("layer_start");
  s_start.toLower();

  G4cout << s_start << G4endl;

  assert((s_start=="x") || (s_start=="z"));
  
  if (s_start=='x')
    layer_config = AlongX;
  else 
    layer_config = AlongZ;

  db->exec("select * from layer;");
  db->getTuple();
  cell_hwidth = db->fetchDouble("cell_width")/2;
  n_cell = db->fetchInt("n_cell");

  db->exec("select * from layer_thickness;");
  db->getTuple();
  poly_hthickness = db->fetchDouble("poly_thickness")/2;
  steel_hthickness = db->fetchDouble("steel_thickness")/2;
}


void TBcatcher00::BuildElements()
{
  // set sensitive detector
  SetSD();

  // catcher dims
  layer_hthickness = steel_hthickness + poly_hthickness;
  cal_hx = cell_hwidth * n_cell;
  cal_hy = n_layers * layer_hthickness;
  cal_hz = cell_hwidth * n_cell;

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
  // cell definitions

  // horizontal (along Z)
  G4Box *CellHorSolid = new G4Box("CellHorSolid",
				  cell_hwidth,
				  poly_hthickness,
				  cal_hz);
  
  CellHorLogical = new G4LogicalVolume(CellHorSolid,
				       poly,
				       "CellHorSolid",
				       0,
				       0,
				       0);

  CellHorLogical->SetSmartless(100.);
  CellHorLogical->SetVisAttributes(G4VisAttributes::Invisible);
  CellHorLogical->SetSensitiveDetector(catcherSD);

  // vertical (along X)
  G4Box *CellVertSolid = new G4Box("CellVertSolid",
				   cal_hx,
				   poly_hthickness,
				   cell_hwidth);

  CellVertLogical = new G4LogicalVolume(CellVertSolid,
					poly,
					"CellVertSolid",
					0,
					0,
					0);

  // CellVertLogical->SetSmartless(100.);
  CellVertLogical->SetVisAttributes(G4VisAttributes::Invisible);

  CellVertLogical->SetSensitiveDetector(catcherSD);

  // vertical layer
  PolyVertLogical = new G4LogicalVolume(PolySolid,
					poly,
					"PolyVertLogical");

  // replicate vertical cells
  TBCellReplication *CellRep = new TBCellReplication(PolyVertLogical, CellVertLogical);
  CellRep->Replicate();
  delete CellRep;

  // hor layer
  PolyHorLogical = new G4LogicalVolume(PolySolid,
				       poly,
				       "PolyHorLogical");

  // replicate horizontal cells
  CellRep = new TBCellReplication(PolyHorLogical, CellHorLogical);
  CellRep->Replicate();
  delete CellRep; CellRep=0;
}


G4bool TBcatcher00::BuildCatcher()
{
  // G4PVPlacement *DetectorPhys = 
  new G4PVPlacement(0,
		    G4ThreeVector(0,y_place,0),
		    DetectorLogical,
		    "CatcherDetectorPhys",
		    WorldLogical,
		    false,
		    0);
  
  for (G4int i=1; i <= n_layers; i++)
  {
    BuildLayer(DetectorLogical, i);
  }

  return true;
}


void TBcatcher00::BuildLayer(G4LogicalVolume *DetLog, G4int nlay)
{
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


 
  
  G4PVPlacement *PolyPhys = new G4PVPlacement(0,
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

  // place sensitive layer
  G4String lt; 
  layer_start lstart=AlongX;  
  G4int l = nlay%2;
  
  if (layer_config==AlongZ)
    {
      if (!l)
	lstart=AlongX;
      else
	lstart=AlongZ;
    }
  if (layer_config==AlongX)
    {
      if (!l)
	lstart=AlongZ;
      else
	lstart=AlongX;
    }
  
  if (lstart==AlongX)
    G4cout << "Building layer AlongX" << G4endl;
  else
    G4cout << "Building layer AlongZ" << G4endl;
  
  BuildSensitive(PolyPhys, lstart);
}

void TBcatcher00::BuildSensitive(G4VPhysicalVolume *pPV, layer_start ltype)
{
  G4LogicalVolume *pLV;

  if (ltype == AlongX)
    pLV = PolyVertLogical;
  else
    pLV = PolyHorLogical;    

  // G4PVPlacement *CellsPhys = 
  new G4PVPlacement(0,
		    G4ThreeVector(),
		    "CellsPhys",
		    pLV,
		    pPV,
		    false,
		    0);
}

void TBcatcher00::SetSD()
{
  catcherSD = new TBSD("catcherSD");  
  RegisterSensitiveDetector(catcherSD);
}

void TBcatcher00::Print()
{
  G4cout << "TBcatcher info: " << G4endl
	 << "n_layers: " << n_layers << G4endl 
	 << "y_place: " << y_place << G4endl 
	 << "cal_hx: " << cal_hx << G4endl
	 << "cal_hy: " << cal_hy << G4endl
	 << "cal_hz: " << cal_hz << G4endl
	 << "layer_start: " << layer_config << G4endl
	 << "cell_hwidth: " << cell_hwidth << G4endl
	 << "poly_hthickness: " << poly_hthickness << G4endl
	 << "steel_hthickness: " << steel_hthickness << G4endl
	 << G4endl;
}
