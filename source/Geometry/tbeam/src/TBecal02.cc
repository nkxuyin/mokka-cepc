#include "Control.hh"
#include "TBecal02.hh"
// #include "TBSD_VCell02.hh"
//#include "TBCellReplication.hh"

#include "CGADefs.h"
#include "MySQLWrapper.hh"
#include "CGAGeometryManager.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VisAttributes.hh"

INSTANTIATE(TBecal02)

TBecal02::~TBecal02()
{}

G4bool TBecal02::construct(const G4String &aSubDetectorDBName,
			   G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding TBecal02..." << G4endl;
  db = new Database(aSubDetectorDBName.data());

  WorldLogical = WorldLog;

  FetchAll();
  BuildElements();
  Print();
  
  G4bool cokay = BuildEcal();

  delete db;
  db = 0;
  
  G4cout << "\nDone building TBecal02" << G4endl;
  return cokay;
}

void TBecal02::FetchAll()
{
  db->exec("select * from ecal_virt;");
  db->getTuple();

  n_layers = db->fetchInt("n_layers");
  assert(n_layers>0);

  n_layers = db->fetchInt("n_layers");
  y_place = db->fetchDouble("y_place");

  ncell_xz[0] = db->fetchInt("ncell_x");
  ncell_xz[1] = db->fetchInt("ncell_z");
  assert(ncell_xz[0]>0 && ncell_xz[1]>0);
  
  grid_size=db->fetchDouble("grid_size");
  y_place=db->fetchDouble("y_place");

  db->exec("select * from layer_thickness;");
  db->getTuple();

  w_hthickness = db->fetchDouble("w_thickness")/2;
  cu_hthickness = db->fetchDouble("cu_thickness")/2;
  g10_hthickness = db->fetchDouble("g10_thickness")/2;
  si_hthickness = db->fetchDouble("si_thickness")/2;
  air_hthickness = db->fetchDouble("air_thickness")/2;
}

void TBecal02::BuildElements()
{
 
  // set Sensitive Detector, including cell LV
  SetSD();

  
  // ecal totals
  layer_hthickness = w_hthickness + cu_hthickness + g10_hthickness + si_hthickness + air_hthickness;
  cal_hx = (ncell_xz[0] * grid_size)/2;
  cal_hy = n_layers * layer_hthickness;
  cal_hz = (ncell_xz[1] * grid_size)/2;

  G4cerr << "cal_hx: " << cal_hx << G4endl;
  G4cerr << "cal_hy: " << cal_hy << G4endl;
  G4cerr << "cal_hz: " << cal_hz << G4endl;

  // materials
  w =  CGAGeometryManager::GetMaterial("tungsten_19.3gccm");
  cu = CGAGeometryManager::GetMaterial("copper");
  g10 = CGAGeometryManager::GetMaterial("g10");
  si = CGAGeometryManager::GetMaterial("silicon_2.33gccm");
  air = CGAGeometryManager::GetMaterial("air");

  // solids & LV

  // detector
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

  // W
  G4Box *WSolid = new G4Box("WSolid",
			    cal_hx,
			    w_hthickness,
			    cal_hz);

  WLogical = new G4LogicalVolume(WSolid,
				 w,
				 "WLogical",
				 0,
				 0,
				 0);

  // Cu
  G4Box *CuSolid = new G4Box("CuSolid",
			     cal_hx,
			     cu_hthickness,
			     cal_hz); 

  CuLogical = new G4LogicalVolume(CuSolid,
				  cu,
				  "CuLogical",
				  0,
				  0,
				  0);

  // Si
  G4Box *SiSolid = new G4Box("SiSolid", 
			     cal_hx,
			     si_hthickness,
			     cal_hz);

  SiLogical = new G4LogicalVolume(SiSolid,
				  si,
				  "SiLogical",
				  0,
				  0,
				  0);

  SiLogical->SetSensitiveDetector(ecalSD);

  // Si Cells
  //G4Box *CellSolid = new G4Box("SiSolid",
  //			       cell_dim_hx,
  //			       si_hthickness,
  //			       cell_dim_hz);

  //CellLogical = new G4LogicalVolume(CellSolid,
  //				    si,
  //				    "CellLogical",
  //				    0,
  //				    0,
  //				    0);

  //CellLogical->SetVisAttributes(G4VisAttributes::Invisible);
  // CellLogical->SetSmartless(100.); // eh?!

  //CellLogical->SetSensitiveDetector(ecalSD);

  // replicate cells
  //TBCellReplication *CellRep = new TBCellReplication(SiLogical, CellLogical);
  //CellRep->Replicate();
  //delete CellRep; CellRep=0;  

  // G10
  G4Box *G10Solid = new G4Box("G10Solid",
			      cal_hx,
			      g10_hthickness,
			      cal_hz);

  G10Logical = new G4LogicalVolume(G10Solid,
				   g10,
				   "G10Logical",
				   0,
				   0,
				   0);

  // Air
  G4Box *AirSolid = new G4Box("AirSolid",
			      cal_hx,
			      air_hthickness,
			      cal_hz);
  
  AirLogical = new G4LogicalVolume(AirSolid,
				   air,
				   "AirLogical",
				   0,
				   0,
				   0); 
}

void TBecal02::BuildLayer(G4LogicalVolume *DetLog, G4int nlay)
{
  
  G4cout << "Building Ecal layer: " << nlay << G4endl;

  G4double lay_y = cal_hy - layer_hthickness;
 
  if (nlay > 1)
    lay_y -= ((nlay-1) * (layer_hthickness*2));
  
  G4cout << "Y Place: " << lay_y << G4endl;

  G4PVPlacement *WholeLayerPhys = new G4PVPlacement(0,
						    G4ThreeVector(0,lay_y,0),
						    WholeLayerLogical,
						    "WholeLayerPhys",
						    DetLog,
						    false,
						    nlay);

  G4double sub_y = layer_hthickness;
  sub_y -= w_hthickness;

  // G4PVPlacement *WPhys = 
  new G4PVPlacement(0,
		    G4ThreeVector(0,sub_y,0),
		    "WPhys",
		    WLogical,
		    WholeLayerPhys,
		    false,
		    0);

  sub_y -= (w_hthickness + g10_hthickness);

  // G4PVPlacement *G10Phys = 
  new G4PVPlacement(0,
		    G4ThreeVector(0,sub_y,0),
		    "G10Phys",
		    G10Logical,
		    WholeLayerPhys,
		    false,
		    0);
 
  sub_y -= (g10_hthickness + si_hthickness);

  //G4PVPlacement *SiPhys = 
  new G4PVPlacement(0,
		    G4ThreeVector(0,sub_y,0),
		    "SiPhys",
		    SiLogical,
		    WholeLayerPhys,
		    false,
		    0);

  sub_y -= (cu_hthickness + si_hthickness);

  // G4PVPlacement *CuPhys = 
  new G4PVPlacement(0,
		    G4ThreeVector(0,sub_y,0),
		    "CuPhys",
		    CuLogical,
		    WholeLayerPhys,
		    false,
		    0);

  sub_y -= (cu_hthickness + air_hthickness);
  
  // G4PVPlacement *AirPhys = 
  new G4PVPlacement(0,
		    G4ThreeVector(0,sub_y,0),
		    "AirPhys",
		    AirLogical,
		    WholeLayerPhys,
		    false,
		    0);
  
}

G4bool TBecal02::BuildEcal()
{
  
  // G4PVPlacement *DetectorPhys = 
  new G4PVPlacement(0,
		    G4ThreeVector(0,y_place,0),
		    DetectorLogical,
		    "EcalDetectorPhys",
		    WorldLogical,
		    0,
		    0);  
  for (G4int i = 1; i <= n_layers; i++)
  {
    BuildLayer(DetectorLogical, i);
  }

  return true;
}

void TBecal02::SetSD()
{
  ecalSD = new TBSD_VCell02("ecalSD",this);  
  RegisterSensitiveDetector(ecalSD);
}

void TBecal02::Print()
{
  G4cout << "\nTBecal02 information: " << G4endl
	 << "n_layers: " << n_layers << G4endl
	 << "y_place: " << y_place << G4endl
    //<< "cell_dim_hx: " << cell_dim_hx << G4endl
    //<< "cell_dim_hz: " << cell_dim_hz << G4endl
    //<< "n_cell_x: " << n_cell_x << G4endl
    //<< "n_cell_z: " << n_cell_z << G4endl
	 << "w_hthickness: " << w_hthickness << G4endl
	 << "cu_hthickness: " << cu_hthickness << G4endl
	 << "g10_hthickness: " << g10_hthickness << G4endl
	 << "si_hthickness: " << si_hthickness << G4endl
	 << "air_hthickness: " << air_hthickness << G4endl 
	 << G4endl;  
}
