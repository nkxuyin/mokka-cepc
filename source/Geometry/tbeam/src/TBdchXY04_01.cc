//#####################################
//                                    #
// Driver used to simulate the drift  #
// chambers in the 2007 CERN setup    #
// (new origin + chambers are now sub #
// divided in 2 halves. Hits in each  #
// half are simulated only in one     #
// direction (X or Y)                 #
//#####################################

#include "TBdchXY04_01.hh"
// #include "TBSD_VCell02.hh"
//#include "TBCellReplication.hh"

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

#include <sstream>

INSTANTIATE(TBdchXY04_01)
  
  TBdchXY04_01::~TBdchXY04_01()
{}

G4bool TBdchXY04_01::ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
					 G4LogicalVolume *theWorld)
{
//  ecal_front_face = aGeometryEnvironment.GetParameterAsDouble("ecal_begin_z");
  ecal_front_face =0.0;// GM 11.04.07
  // variable config_angle (a double) will be used by method "construct" to place the detector
  config_angle    = 0.0;
  // variables translateX and translateY have default value (0,0) and are used
  // to translate DCH and Sci wrt beam axis (user defined parameters)
  translateX      = 0.0*mm;
  translateY      = 0.0*mm;
  
  G4cout << "\nBuilding TBdchXY04_01..." << G4endl;
  db = new Database(aGeometryEnvironment.GetDBName());
  // fetch db parms
  FetchAll();
  
  // set Sensitive Detector
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
  
  G4cout << "\nDone building TBdchXY04_01" << G4endl;

  return doDch;
  
}

G4bool TBdchXY04_01::DchConstruct(G4LogicalVolume *WorldLog, 
				  G4double x_place, G4double y_place, G4double z_place, 
				  G4int idch)
{
  
  G4cout << " Building Drift Chamber " << idch << G4endl;
  WorldLogVol = WorldLog;
  
  // depth to layer
  SetDepthToLayer(1);
  
  G4cout << " Building DCH elements " << G4endl;
  BuildElements();
  
  // do build process
  G4bool cokay = false;
  
  if (idch%2 == 0) cokay = BuildDchX(x_place, y_place, z_place, idch);
  if (idch%2 == 1) cokay = BuildDchY(x_place, y_place, z_place, idch);
  
  return cokay;
}

void TBdchXY04_01::FetchAll()
{
  config_angle = config_angle*deg;
  
  db->exec("select * from dch_virt;");
  db->getTuple();
  
  ncell_xy[0] = db->fetchDouble("dch_dim_x");		// 100 mm
  ncell_xy[1] = db->fetchDouble("dch_dim_y");		// 100 mm
  assert(ncell_xy[0]>0 && ncell_xy[1]>0);
  
  z_placeX1 = db->fetchDouble("z_placeX1");		// -  43.5mm 
  z_placeX2 = db->fetchDouble("z_placeX2");		// - 713.5mm 
  z_placeX3 = db->fetchDouble("z_placeX3");		// -2586.5mm 
  
  z_placeY1 = db->fetchDouble("z_placeY1");		// -  14.5mm 
  z_placeY2 = db->fetchDouble("z_placeY2");		// - 684.5mm 
  z_placeY3 = db->fetchDouble("z_placeY3");		// -2557.5mm 
  
  // take into account configuration angle and translation
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
  
  db->exec("select * from layer_thickness;");
  db->getTuple();
  
  winback_hthickness = db->fetchDouble("winback_thickness");    // 20 micron
  winfront_hthickness = db->fetchDouble("winfront_thickness");  // 20 micron
  dch_hthickness = db->fetchDouble("dch_thickness");            // 29 mm
  gas_hthickness = dch_hthickness - winfront_hthickness;
  
  gas_temperature = STP_Temperature;
  gas_pressure = db->fetchDouble("gas_pressure");               // 1.5mbar
  gas_pressure = gas_pressure*1.0e-3*bar;
  
  grid_size = 1;
  
  Print();
}

void TBdchXY04_01::BuildElements()
{
  
  // materials
  G4String name, symbol;
  G4int nel, natoms;
  G4double a, z, density, temperature, pressure;
  G4double fractionmass;
  
  // Hydrogen
  a = 1.01*g/mole;
  elH  = new G4Element(name="Hydrogen",symbol="H" , z= 1., a);
  // Carbon
  a = 12.01*g/mole;
  elC  = new G4Element(name="Carbon"  ,symbol="C" , z= 6., a);
  // Oxigen
  a = 16.00*g/mole;
  elO = new G4Element(name="Oxigen",   symbol="O",  z=8.,  a);
  // Nitrogen
  a = 14.01*g/mole;
  elN = new G4Element(name="Nitrogen", symbol="N",  z=7.,  a);
  
  // Argon
  a = 39.95*g/mole;
  density = 1.78e-3*g/cm3;
  elAr= new G4Material(name="Argon", z=18., a, density);
  
  // Air
  density = 1.29e-3*g/cm3;
  air = new G4Material(name="Air", density, nel=2);
  air->AddElement(elN, fractionmass=0.7);
  air->AddElement(elO, fractionmass=0.3);
  
 // CO2
  density  = 1.98e-3*g/cm3;
  CO2 = new G4Material(name="CO2", density, nel=2, kStateGas);
  CO2->AddElement(elC, natoms=1);   
  CO2->AddElement(elO, natoms=2);   
  
  // Gas mixture in DCH is 50% Ar and 50% CO2
  density  = ((1.78e-3)*0.50 + (1.98e-3)*0.50)*g/cm3;
  temperature = gas_temperature;
  pressure    = gas_pressure;
  gas_mix = new G4Material(name="mixture", density, nel=2, kStateGas, temperature, pressure);
  gas_mix->AddMaterial(elAr, 50*perCent);
  gas_mix->AddMaterial(CO2,  50*perCent);

 // Kapton
  density  = 1.420*g/cm3;
  kapton = new G4Material(name="Kapton", density, nel=4);
  kapton->AddElement(elC, natoms=22);
  kapton->AddElement(elH, natoms=10);
  kapton->AddElement(elN, natoms=2);
  kapton->AddElement(elO, natoms=5);
  
  // dch (half) dimensions 
  dch_hx = (ncell_xy[0])/2;
  dch_hy = (ncell_xy[1])/2;
  dch_hz = dch_hthickness/2;
  kaptonF_hz = winfront_hthickness/2;
  kaptonB_hz = winback_hthickness/2;
  gas_hz = gas_hthickness/2;
  
  // detector
  G4Box *DetectorSolid = new G4Box("DetectorSolid",
				   dch_hx,
				   dch_hy,
				   dch_hz);
  
  DetectorLogical = new G4LogicalVolume(DetectorSolid,
					air,
					"DetectorLogical",
					0,
					0,
					0);
  
  G4cout << " Dimension of detector box " << G4endl;
  G4cout << " dch_hx: " << dch_hx*2 << " mm " << G4endl;
  G4cout << " dch_hy: " << dch_hy*2 << " mm " << G4endl;
  G4cout << " dch_hz: " << dch_hz*2 << " mm " << G4endl;
  
  // Kapton front window
  G4Box *KaptonFrontWindow = new G4Box("KaptonFrontWindow",
			 	      dch_hx,
				      dch_hy,
			              kaptonF_hz);
  
  KaptonFrontLogical = new G4LogicalVolume(KaptonFrontWindow,
				          kapton,
				          "KaptonFrontLogical",
				 	  0,
				          0,
				          0);
  
  G4cout << " Dimension of front window " << G4endl;
  G4cout << " dch_hx: " << dch_hx*2    << " mm " << G4endl;
  G4cout << " dch_hy: " << dch_hy*2    << " mm " << G4endl;
  G4cout << " dch_hz: " << kaptonF_hz*2 << " mm " << G4endl;
  
  // Ar-Ethane gas mixture
  G4Box *GasSolid = new G4Box("GasSolid",
			      dch_hx,
			      dch_hy,
			      gas_hz);
  
  GasLogical = new G4LogicalVolume(GasSolid,
				   gas_mix,
				   "GasLogical",
				   0,
				   0,
				   0);
  
  G4cout << " Dimension of gas volume " << G4endl;
  G4cout << " dch_hx: " << dch_hx*2 << " mm " << G4endl;
  G4cout << " dch_hy: " << dch_hy*2 << " mm " << G4endl;
  G4cout << " dch_hz: " << gas_hz*2 << " mm " << G4endl;
  
  // Kapton back window
  G4Box *KaptonBackWindow = new G4Box("KaptonBackWindow",
				     dch_hx,
				     dch_hy,
				     kaptonB_hz); 
  
  KaptonBackLogical = new G4LogicalVolume(KaptonBackWindow,
					 kapton,
					 "KaptonBackLogical",
					 0,
					 0, 
					 0);
  
  G4cout << " Dimension of back window " << G4endl;
  G4cout << " dch_hx: " << dch_hx*2    << " mm " << G4endl;
  G4cout << " dch_hy: " << dch_hy*2    << " mm " << G4endl;
  G4cout << " dch_hz: " << kaptonB_hz*2 << " mm " << G4endl;
  
}

G4bool TBdchXY04_01::BuildDchX(G4double x_place, G4double y_place, G4double z_place, G4int idch)
{
  
  G4cout << " Building Dch structure " << G4endl;
  
  G4cout << " x_place of full Dch detector " << x_place << G4endl;
  G4cout << " y_place of full Dch detector " << y_place << G4endl;
  G4cout << " z_place of full Dch detector " << z_place << G4endl;

  G4int pCopyNo = idch;
  
  translateDch = G4ThreeVector(x_place, y_place, z_place);
  G4RotationMatrix rotateDch;
  rotateDch.rotateY(config_angle);
  transformDch = new G4Transform3D(rotateDch, translateDch);
  
  new G4PVPlacement(*transformDch,
	            DetectorLogical,
		    "DetectorPhys",
		    WorldLogVol,
		    0,
		    pCopyNo);  
  
  // Place gas part of the detector in the middle of detector box
  G4double gas_z = winfront_hthickness/2;
  new G4PVPlacement(0,
	    	    G4ThreeVector(0.,0.,gas_z),
		    GasLogical,
		    "GasPhys",
		    DetectorLogical,
		    false,
		    pCopyNo);
  G4VisAttributes *gasColour = new G4VisAttributes(G4Colour(1.,0.,1.));
  gasColour->SetVisibility(true);
  GasLogical->SetVisAttributes(gasColour);
  // set active layer
  GasLogical->SetSensitiveDetector(dchSD);
  
  G4double wfront_z = winfront_hthickness/2 - dch_hthickness/2;
  G4cout << " Front window Z position (centre, wrt detector box): " << wfront_z << " mm " << G4endl;
  
  // Place kapton window
  new G4PVPlacement(0,
		    G4ThreeVector(0.,0.,wfront_z),
		    KaptonFrontLogical,
		    "WinFrontPhys",
		    DetectorLogical,
		    false,
		    pCopyNo);
  G4VisAttributes *winColour = new G4VisAttributes(G4Colour(0.,1.,0.));
  winColour->SetVisibility(true);
  KaptonFrontLogical->SetVisAttributes(winColour);
  
  return true;
}

G4bool TBdchXY04_01::BuildDchY(G4double x_place, G4double y_place, G4double z_place, G4int idch)
{                                     
  
  G4cout << " Building Dch structure " << G4endl;
  
  G4cout << " x_place of full Dch detector " << x_place << G4endl;
  G4cout << " y_place of full Dch detector " << y_place << G4endl;
  G4cout << " z_place of full Dch detector " << z_place << G4endl;
  
  G4int pCopyNo = idch;

  translateDch = G4ThreeVector(x_place, y_place, z_place);
  G4RotationMatrix rotateDch;
  rotateDch.rotateY(config_angle);
  transformDch = new G4Transform3D(rotateDch, translateDch);
  
  new G4PVPlacement(*transformDch,
                    DetectorLogical,
                    "DetectorPhys",
                    WorldLogVol,
                    0,
                    pCopyNo);
  
  // Place gas part of the detector in the middle of detector box
  G4double gas_z = - winback_hthickness/2;
  new G4PVPlacement(0,
                    G4ThreeVector(0.,0.,gas_z),
                    GasLogical, 
                    "GasPhys",
                    DetectorLogical,
                    false,
                    pCopyNo);
  G4VisAttributes *gasColour = new G4VisAttributes(G4Colour(1.,1.,1.));
  gasColour->SetVisibility(true);
  GasLogical->SetVisAttributes(gasColour);
  // set active layer
  GasLogical->SetSensitiveDetector(dchSD);
  
  G4double wback_z = dch_hthickness/2 - winback_hthickness/2;
  G4cout << " Back window Z position (centre, wrt detector box): " << wback_z << " mm " << G4endl;
  
  new G4PVPlacement(0, 
                    G4ThreeVector(0.,0.,wback_z),
                    KaptonBackLogical,
                    "WinBackPhys",
                    DetectorLogical,
                    false,
                    pCopyNo);
  G4VisAttributes *winColour = new G4VisAttributes(G4Colour(0.,1.,0.));
  winColour->SetVisibility(true);
  KaptonBackLogical->SetVisAttributes(winColour);
  
  return true;
}

void TBdchXY04_01::SetSD(G4int idch)
{
  
  G4String base = "dchSDxy";
  stringstream s;
  s << base << idch;
  
  G4cout <<" Sensitive detector " << s.str() << G4endl;
//  TBSD_Dch01 *dchSD;
  dchSD = new TBSD_Dch01(s.str(), 0.001);
  
  // register
  RegisterSensitiveDetector(dchSD);
}

void TBdchXY04_01::SetDepthToLayer(G4int i) {
  depthToLayer = i;
  G4cout <<" DepthToLayer in Dch: " << depthToLayer << G4endl;
}

void TBdchXY04_01::Print()
{
  G4cout << "\nTBdchXY04_01 information: " << G4endl
	 << " ecal_front_face: "     << ecal_front_face     << " mm " << G4endl
	 << " z_placeX1: "           << z_placeX1           << " mm " << G4endl
	 << " z_placeX2: "           << z_placeX2           << " mm " << G4endl
	 << " z_placeX3: "           << z_placeX3           << " mm " << G4endl
	 << " z_placeY1: "           << z_placeY1           << " mm " << G4endl
	 << " z_placeY2: "           << z_placeY2           << " mm " << G4endl
	 << " z_placeY3: "           << z_placeY3           << " mm " << G4endl
	 << " layer_hthickness: "    << 2*dch_hthickness    << " mm " << G4endl
	 << " winfront_hthickness: " << winfront_hthickness << " mm " << G4endl
	 << " winback_hthickness: "  << winback_hthickness  << " mm " << G4endl
	 << " gas_hthickness: "      << 2*gas_hthickness    << " mm " << G4endl
	 << " gas_pressure: "        << gas_pressure        << " mm " << G4endl
	 << " gas_temperature: "     << gas_temperature     << " mm " << G4endl
	 << G4endl;  
}
