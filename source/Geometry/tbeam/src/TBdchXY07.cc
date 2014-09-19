//############################################################################
//                                                                           #
// Driver used to simulate the drift chambers in the June 2011 CERN setup    #
//############################################################################
#include "TBdchXY07.hh"
#include "Control.hh"
#include "CGADefs.h"
#include "MySQLWrapper.hh"
#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4Element.hh"
#include "G4ElementTable.hh"
#include "G4Isotope.hh"
#include "CGAGeometryManager.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4UnitsTable.hh"
#include <sstream>

INSTANTIATE(TBdchXY07)
  
/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
  TBdchXY07::~TBdchXY07()
{}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
G4bool TBdchXY07::ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
					 G4LogicalVolume *theWorld)
{
  checkForOverlappingVolumes = false;  

  /* variable config_angle (a double) will be used by method "construct" to place the detector*/
  config_angle    = 0.0;
  
  /* variables translateX and translateY have default value (0,0) and are used
     to translate DCH and Sci wrt beam axis (user defined parameters) */
  translateX      = 0.0*mm;
  translateY      = 0.0*mm;
  
  G4cout << "\nBuilding TBdchXY07..." << G4endl;
  db = new Database(aGeometryEnvironment.GetDBName());

  /* fetch db parms*/
  FetchAll();

  /* depth to layer*/
  SetDepthToLayer(1);

  SetSD(0);

  G4bool doDchX1 = DchConstruct (theWorld,x_placeX1,y_placeX1,z_placeX1,2);
  G4bool doDchY1 = DchConstruct (theWorld,x_placeY1,y_placeY1,z_placeY1,3);
  G4bool doDch1 = false;
  if (doDchX1 && doDchY1) doDch1 = true;

  G4bool doDchX2 = DchConstruct (theWorld,x_placeX2,y_placeX2,z_placeX2,4);
  G4bool doDchY2 = DchConstruct (theWorld,x_placeY2,y_placeY2,z_placeY2,5);
  G4bool doDch2 = false;
  if (doDchX2 && doDchY2) doDch2 = true;

  G4bool doDchX3 = DchConstruct (theWorld,x_placeX3,y_placeX3,z_placeX3,6);
  G4bool doDchY3 = DchConstruct (theWorld,x_placeY3,y_placeY3,z_placeY3,7);
  G4bool doDch3 = false;
  if (doDchX3 && doDchY3) doDch3 = true;

  delete db;
  db = 0;
  
  G4bool doDch = false;
  if (doDch1 && doDch2 && doDch3) doDch = true;
  
  G4cout << "\nDone building TBdchXY07" << G4endl;

  return doDch;
  
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
G4bool TBdchXY07::DchConstruct(G4LogicalVolume *WorldLog, 
				  G4double x_place, G4double y_place, G4double z_place, 
				  G4int idchamber)
{
  G4cout << "\n Building Drift Chamber " << idchamber << G4endl;
  WorldLogVol = WorldLog;
  
  
  G4cout << " Building DCH elements " << G4endl;
  BuildElements();
  
  G4bool cokay = false;
  
  if (idchamber % 2 == 0) cokay = BuildDchX(x_place, y_place, z_place, idchamber);
  if (idchamber % 2 == 1) cokay = BuildDchY(x_place, y_place, z_place, idchamber);
  
  return cokay;
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
void TBdchXY07::FetchAll()
{
  config_angle = config_angle*deg;
  
  db->exec("select * from wch_cernJune2011_virt;");
  db->getTuple();
  
  wirechamber_X_dimension = db->fetchDouble("wirechamber_X_dimension");	
  wirechamber_Y_dimension = db->fetchDouble("wirechamber_Y_dimension");	
  assert(wirechamber_X_dimension>0 && wirechamber_Y_dimension>0);
  
  z_placeX1 = db->fetchDouble("wirechamber_z_placeX1");	
  z_placeX2 = db->fetchDouble("wirechamber_z_placeX2");	
  z_placeX3 = db->fetchDouble("wirechamber_z_placeX3");	
  z_placeY1 = db->fetchDouble("wirechamber_z_placeY1");	
  z_placeY2 = db->fetchDouble("wirechamber_z_placeY2");	
  z_placeY3 = db->fetchDouble("wirechamber_z_placeY3");	
  gas_pressure = db->fetchDouble("wirechamber_gas_pressure");
  gas_pressure = gas_pressure*1.0e-3*bar; 
  gas_temperature = STP_Temperature;
  grid_size = 1;

  /* take into account configuration angle and translation*/
  x_placeX1 = translateX;
  y_placeX1 = translateY;
  
  x_placeX2 = translateX;
  y_placeX2 = translateY;
  
  x_placeX3 = translateX;
  y_placeX3 = translateY;
  
  x_placeY1 = translateX;
  y_placeY1 = translateY;
  
  x_placeY2 = translateX;
  y_placeY2 = translateY;
  
  x_placeY3 = translateX;
  y_placeY3 = translateY;
  
  db->exec("select * from wch_cernJune2011_layerthickness_virt;");
  db->getTuple();
  
  wirechamber_kapton_windowback_hthickness  = db->fetchDouble("wirechamber_kapton_windowback_hthickness");
  wirechamber_kapton_windowfront_hthickness = db->fetchDouble("wirechamber_kapton_windowfront_hthickness");
  wirechamber_thickness                     = db->fetchDouble("wirechamber_thickness");
  gas_thickness = (wirechamber_thickness - wirechamber_kapton_windowback_hthickness - wirechamber_kapton_windowfront_hthickness);
 
  db->exec("select * from wch_cernJune2011_materials_virt;");
  db->getTuple();
  Wire_chambers_CO2_density      = db->fetchDouble("Wire_chambers_CO2_density")*g/cm3;
  Wire_chambers_ArCO2_density    = db->fetchDouble("Wire_chambers_ArCON2_density")*g/cm3;
  G4cout << " Wire chambers materials " << G4endl;
  G4cout << "Wire_chambers_CO2_density " << G4BestUnit(Wire_chambers_CO2_density,"Volumic Mass") << G4endl;
  G4cout << "Wire_chambers_ArCO2_density " << G4BestUnit(Wire_chambers_ArCO2_density,"Volumic Mass") << G4endl;

  Print();
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
void TBdchXY07::BuildElements()
{
  /* materials*/
  G4String name, symbol;
  G4int nel, natoms;
  G4double a, z, temperature, pressure;

  /* Carbon*/
  a = 12.01*g/mole;
  elC  = new G4Element(name="Carbon"  ,symbol="C" , z= 6., a);
  /* Oxigen*/
  a = 16.00*g/mole;
  elO = new G4Element(name="Oxigen",   symbol="O",  z=8.,  a);
  
  /* Air, Argon and Kapton*/
  air    = CGAGeometryManager::GetMaterial("air");
  kapton = CGAGeometryManager::GetMaterial("kapton"); 
  argon  = CGAGeometryManager::GetMaterial("argon"); 

  /* CO2*/
  CO2 = new G4Material(name="CO2", Wire_chambers_CO2_density, nel=2, kStateGas);
  CO2->AddElement(elC, natoms=1);   
  CO2->AddElement(elO, natoms=2);   
  
  /* Gas mixture in DCH is 50% Ar and 50% CO2*/
  temperature = gas_temperature;
  pressure    = gas_pressure;
  gas_mix = new G4Material(name="mixture", Wire_chambers_ArCO2_density, nel=2, kStateGas, temperature, pressure);
  gas_mix->AddMaterial(argon, 50*perCent);
  gas_mix->AddMaterial(CO2,  50*perCent);

  /* dch (half) dimensions */
  dch_hx = wirechamber_X_dimension/2;
  dch_hy = wirechamber_Y_dimension/2;
  dch_hz = wirechamber_thickness/2;
  kaptonF_hz = wirechamber_kapton_windowfront_hthickness/2;
  kaptonB_hz = wirechamber_kapton_windowback_hthickness/2;
  gas_hz = gas_thickness/2;
  
  /* detector*/
  G4Box *DetectorSolid = new G4Box("DetectorSolid", dch_hx,dch_hy,dch_hz);
  DetectorLogical = new G4LogicalVolume(DetectorSolid,air,"DetectorLogical",0,0,0);
  
  G4cout << "\n Dimension of whole detector box " << G4endl;
  G4cout << " dch_hx: " << dch_hx*2 << " mm " << G4endl;
  G4cout << " dch_hy: " << dch_hy*2 << " mm " << G4endl;
  G4cout << " dch_hz: " << dch_hz*2 << " mm " << G4endl;
  
  /* Kapton front window*/
  G4Box *KaptonFrontWindow = new G4Box("KaptonFrontWindow",dch_hx,dch_hy,kaptonF_hz);
  KaptonFrontLogical = new G4LogicalVolume(KaptonFrontWindow,kapton,"KaptonFrontLogical",0,0,0);
  
  G4cout << "\n Dimension of front window " << G4endl;
  G4cout << " dch_hx: " << dch_hx*2    << " mm " << G4endl;
  G4cout << " dch_hy: " << dch_hy*2    << " mm " << G4endl;
  G4cout << " dch_hz: " << kaptonF_hz*2 << " mm " << G4endl;
  
  /* Ar-Ethane gas mixture*/
  G4Box *GasSolid_X = new G4Box("GasSolid_X",dch_hx,dch_hy,gas_hz/2);
  GasLogical_X = new G4LogicalVolume(GasSolid_X,gas_mix,"GasLogical_X",0,0,0);

  G4Box *GasSolid_Y = new G4Box("GasSolid_Y",dch_hx,dch_hy,gas_hz/2);
  GasLogical_Y = new G4LogicalVolume(GasSolid_Y,gas_mix,"GasLogical_Y",0,0,0);
  
  G4cout << "\n Dimension of gas volume " << G4endl;
  G4cout << " dch_hx: " << dch_hx*2 << " mm " << G4endl;
  G4cout << " dch_hy: " << dch_hy*2 << " mm " << G4endl;
  G4cout << " dch_hz: " << gas_hz << " mm " << G4endl;
  
  /* Kapton back window*/
  G4Box *KaptonBackWindow = new G4Box("KaptonBackWindow",dch_hx,dch_hy,kaptonB_hz); 
  KaptonBackLogical = new G4LogicalVolume(KaptonBackWindow,kapton,"KaptonBackLogical",0,0,0);
  
  G4cout << "\n Dimension of back window " << G4endl;
  G4cout << " dch_hx: " << dch_hx*2    << " mm " << G4endl;
  G4cout << " dch_hy: " << dch_hy*2    << " mm " << G4endl;
  G4cout << " dch_hz: " << kaptonB_hz*2 << " mm " << G4endl;
  
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
G4bool TBdchXY07::BuildDchY(G4double x_place, G4double y_place, G4double z_place, G4int idch)
{
  G4cout << "\n\n Building DchY "<< idch <<" structure " << G4endl;
  
  G4cout << " x_place of DchY detector " << x_place << G4endl;
  G4cout << " y_place of DchY detector " << y_place << G4endl;
  G4cout << " z_place of DchY detector " << z_place << G4endl;

  G4int pCopyNo = idch;
  
  translateDch = G4ThreeVector(x_place, y_place, z_place);
  G4RotationMatrix rotateDch;
  rotateDch.rotateY(config_angle);
  transformDch = new G4Transform3D(rotateDch, translateDch);
  
  new G4PVPlacement(*transformDch,DetectorLogical,"DetectorPhys",WorldLogVol,0,pCopyNo);

  
  G4double wfront_z = - dch_hz + kaptonF_hz;
  G4cout<<" DchY: place KaptonFront at z="<<wfront_z<<G4endl;
  
  /* Place kapton window*/
  new G4PVPlacement(0,G4ThreeVector(0.,0.,wfront_z),
		    KaptonFrontLogical,
		    "WinFrontPhys",
		    DetectorLogical,
		    false,
		    pCopyNo,
		    checkForOverlappingVolumes);  
  G4VisAttributes *winColour = new G4VisAttributes(G4Colour::Blue());
  winColour->SetVisibility(true);
  KaptonFrontLogical->SetVisAttributes(winColour);

  
  /* Place gas part of the detector in the middle of detector box*/
  G4double gas_z = wfront_z + kaptonF_hz + gas_hz/2;
  G4cout<<" DchY: place gas at z="<<gas_z<<G4endl;
  new G4PVPlacement(0,G4ThreeVector(0.,0.,gas_z),
		    GasLogical_Y,"GasPhys",
		    DetectorLogical,
		    false,
		    pCopyNo,
		    checkForOverlappingVolumes);

  G4VisAttributes *gasColour = new G4VisAttributes(G4Colour::Magenta());
  gasColour->SetVisibility(true);
  GasLogical_Y->SetVisAttributes(gasColour);

  /* set active layer*/
  GasLogical_Y->SetSensitiveDetector(dchSD);
  
  return true;
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
G4bool TBdchXY07::BuildDchX(G4double x_place, G4double y_place, G4double z_place, G4int idch)
{                                     
  G4cout << "\n Building DchX "<<idch<<" structure " << G4endl;
  
  G4cout << " x_place of DchX detector " << x_place << G4endl;
  G4cout << " y_place of DchX detector " << y_place << G4endl;
  G4cout << " z_place of DchX detector " << z_place << G4endl;
  
  G4int pCopyNo = idch;

  translateDch = G4ThreeVector(x_place, y_place, z_place);
  G4RotationMatrix rotateDch;
  rotateDch.rotateY(config_angle);
  transformDch = new G4Transform3D(rotateDch, translateDch);
  
  new G4PVPlacement(*transformDch,DetectorLogical,"DetectorPhys",WorldLogVol,0,pCopyNo);

  /* Place gas part of the detector in the middle of detector box*/
  G4cout<<" DchX init gas_hz="<<gas_hz<<" dch_hz="<<dch_hz<<" kaptonF_hz="<<kaptonF_hz<<G4endl;
  G4double gas_z = - dch_hz + kaptonF_hz + kaptonF_hz + gas_hz;
  G4cout<<" DchX place gas at z="<<gas_z<<G4endl;

  new G4PVPlacement(0,G4ThreeVector(0.,0.,gas_z),
		    GasLogical_X,"GasPhys",
		    DetectorLogical,
		    false,
		    pCopyNo,
		    checkForOverlappingVolumes);

  G4VisAttributes *gasColour = new G4VisAttributes(G4Colour::Magenta());
  gasColour->SetVisibility(true);
  GasLogical_X->SetVisAttributes(gasColour);

  /* set active layer*/
  GasLogical_X->SetSensitiveDetector(dchSD);
    
  G4double wback_z = - dch_hz + 2*kaptonF_hz + 2*gas_hz + kaptonB_hz;
  G4cout << " DchX place KaptonBack at z= " << wback_z << " mm " << G4endl;
  
  new G4PVPlacement(0,G4ThreeVector(0.,0.,wback_z),
		    KaptonBackLogical,
		    "WinBackPhys",
		    DetectorLogical,
		    false,
		    pCopyNo,
		    checkForOverlappingVolumes);  

  G4VisAttributes *winColour = new G4VisAttributes(G4Colour::Blue());
  winColour->SetVisibility(true);
  KaptonBackLogical->SetVisAttributes(winColour);
  return true;
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
void TBdchXY07::SetSD(G4int idch)
{
  G4String base = "dchSDxy";
  stringstream s;
  s << base << idch;
  
  G4cout <<" Sensitive detector " << s.str() << G4endl;
  dchSD = new TBSD_Dch01(s.str(), 0.000000000001);
  
  RegisterSensitiveDetector(dchSD);
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
void TBdchXY07::SetDepthToLayer(G4int i) 
{
  depthToLayer = i;
  G4cout <<" DepthToLayer in Dch: " << depthToLayer << G4endl;
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
void TBdchXY07::Print()
{
  G4cout << "\nTBdchXY07 information: " << G4endl
	 << " z_placeX1: "           << z_placeX1           << " mm " << G4endl
	 << " z_placeX2: "           << z_placeX2           << " mm " << G4endl
	 << " z_placeX3: "           << z_placeX3           << " mm " << G4endl
	 << " z_placeY1: "           << z_placeY1           << " mm " << G4endl
	 << " z_placeY2: "           << z_placeY2           << " mm " << G4endl
	 << " z_placeY3: "           << z_placeY3           << " mm " << G4endl
	 << " layer_hthickness: "    << wirechamber_thickness    << " mm " << G4endl
	 << " winfront_hthickness: " << wirechamber_kapton_windowfront_hthickness << " mm " << G4endl
	 << " winback_hthickness: "  << wirechamber_kapton_windowback_hthickness  << " mm " << G4endl
	 << " gas_thickness: "       << gas_thickness       << " mm " << G4endl
	 << " gas_pressure: "        << gas_pressure        << " mm " << G4endl
	 << " gas_temperature: "     << gas_temperature     << G4endl
	 << G4endl;  
}
