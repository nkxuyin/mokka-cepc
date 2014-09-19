//###############################################
//                                              #
//Driver used to simulate the Cerenkov detector #
//            in the CERN06 setup               #
//                                              #
//###############################################

#include "TBcerenkov01.hh"
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

INSTANTIATE(TBcerenkov01)

TBcerenkov01::~TBcerenkov01()
{}

G4bool TBcerenkov01::ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
                G4LogicalVolume *theWorld)
{

 ecal_front_face = aGeometryEnvironment.GetParameterAsDouble("ecal_begin_z");
 // variable config_angle (a double) will be used by method "construct" to place the detector
 config_angle    = aGeometryEnvironment.GetParameterAsDouble("configuration_angle");
 // variables translateX and translateY have default value (0,0) and are used
 // to translate Cerenkov wrt beam axis (user defined parameters)
 translateX      = aGeometryEnvironment.GetParameterAsDouble("TranslateX");
 translateY      = aGeometryEnvironment.GetParameterAsDouble("TranslateY");

 G4cout << "\nBuilding TBcerenkov01..." << G4endl;
 db = new Database(aGeometryEnvironment.GetDBName());
 // fetch db parms
 FetchAll();
  
 G4bool doCeren = CerenConstruct (theWorld,x_place1,y_place1,z_place1,1);

 delete db;
 db = 0;
  
 G4cout << "\nDone building TBcerenkov01" << G4endl;
 return doCeren;

}

G4bool TBcerenkov01::CerenConstruct(G4LogicalVolume *WorldLog, 
				    G4double x_place, G4double y_place, G4double z_place, 
				    G4int iceren)
{

  G4cout << " Building Cerenkov " << iceren << G4endl;
  WorldLogVol = WorldLog;

  // depth to layer
  SetDepthToLayer(1);

  G4cout << " Building Cerenkov elements " << G4endl;
  BuildElements();

  // do build process
  G4bool cokay = BuildCeren(x_place, y_place, z_place);

  // set Sensitive Detector
//  SetSD(iceren);

  return cokay;
}

void TBcerenkov01::FetchAll()
{
  config_angle = config_angle*deg;

  db->exec("select * from ceren_virt;");
  db->getTuple();

  ncell_xy[0] = db->fetchDouble("ceren_dim_x");		// 100 mm
  ncell_xy[1] = db->fetchDouble("ceren_dim_y");		// 100 mm
  assert(ncell_xy[0]>0 && ncell_xy[1]>0);

  z_place1 = db->fetchDouble("z_place1");		// 

// take into account configuration angle and translation
  x_place1 = translateX + (ecal_front_face + z_place1)*sin(config_angle);
  y_place1 = translateY;
  z_place1 = (ecal_front_face + z_place1)*cos(config_angle);
  
  db->exec("select * from layer_thickness;");
  db->getTuple();

  winfront_hthickness = db->fetchDouble("winfront_thickness");  // 180   micron
  winback_hthickness = db->fetchDouble("winback_thickness");    // 180   micron
  ceren_hthickness = db->fetchDouble("ceren_thickness");        // 11000 mm
  gas_pressure = db->fetchDouble("gas_pressure");               // 1.0   atm 

  gas_hthickness = ceren_hthickness - winfront_hthickness - winback_hthickness;
  gas_temperature = STP_Temperature;
  gas_pressure = gas_pressure*atmosphere;

  grid_size = 1;

  Print();
}

void TBcerenkov01::BuildElements()
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

  // Air
  density = 1.29e-3*g/cm3;  
  air = new G4Material(name="Air", density, nel=2);
  air->AddElement(elN, fractionmass=0.7);
  air->AddElement(elO, fractionmass=0.3);

  // Gas in Cerenkov is 100% Helium
  a = 4.003*g/mole;
  density  = (0.179e-3)*g/cm3;
  temperature = gas_temperature;
  pressure    = gas_pressure;
  elHe = new G4Material(name="Helium", z=2., a, density, kStateGas, temperature, pressure);

  // Mylar
  density  = 1.39*g/cm3;
  mylar = new G4Material(name="Mylar", density, nel=3);
  mylar->AddElement(elC, natoms=3);
  mylar->AddElement(elH, natoms=4);
  mylar->AddElement(elO, natoms=2);

  // Cerenkov (half) dimensions 
  ceren_hx = (ncell_xy[0])/2;
  ceren_hy = (ncell_xy[1])/2;
  ceren_hz = ceren_hthickness/2;
  mylarF_hz = winfront_hthickness/2;
  gas_hz = gas_hthickness/2;
  mylarB_hz = winback_hthickness/2;

  // detector
  G4Box *DetectorSolid = new G4Box("DetectorSolid",
				   ceren_hx,
				   ceren_hy,
				   ceren_hz);

  DetectorLogical = new G4LogicalVolume(DetectorSolid,
					air,
					"DetectorLogical",
					0,
					0,
					0);

  G4cout << " Dimension of detector box " << G4endl;
  G4cout << " ceren_hx: " << ceren_hx*2 << " mm " << G4endl;
  G4cout << " ceren_hy: " << ceren_hy*2 << " mm " << G4endl;
  G4cout << " ceren_hz: " << ceren_hz*2 << " mm " << G4endl;

  // Mylar front window
  G4Box *MylarFrontWindow = new G4Box("MylarFrontWindow",
			 	      ceren_hx,
				      ceren_hy,
			              mylarF_hz);
  
  MylarFrontLogical = new G4LogicalVolume(MylarFrontWindow,
				          mylar,
				          "MylarFrontLogical",
				 	  0,
				          0,
				          0);
  
  G4cout << " Dimension of front window " << G4endl;
  G4cout << " ceren_hx: " << ceren_hx*2    << " mm " << G4endl;
  G4cout << " ceren_hy: " << ceren_hy*2    << " mm " << G4endl;
  G4cout << " ceren_hz: " << mylarF_hz*2 << " mm " << G4endl;
  
  // Helium gas
  G4Box *GasSolid = new G4Box("GasSolid",
			      ceren_hx,
			      ceren_hy,
			      gas_hz);
  
  GasLogical = new G4LogicalVolume(GasSolid,
				   elHe,
				   "GasLogical",
				   0,
				   0,
				   0);

  G4cout << " Dimension of gas volume " << G4endl;
  G4cout << " ceren_hx: " << ceren_hx*2 << " mm " << G4endl;
  G4cout << " ceren_hy: " << ceren_hy*2 << " mm " << G4endl;
  G4cout << " ceren_hz: " << ceren_hz*2 << " mm " << G4endl;

  // Mylar back window
  G4Box *MylarBackWindow = new G4Box("MylarBackWindow",
				     ceren_hx,
				     ceren_hy,
				     mylarB_hz);
  
  MylarBackLogical = new G4LogicalVolume(MylarBackWindow,
					 mylar,
					 "MylarBackLogical",
					 0,
					 0,
					 0);
  
  G4cout << " Dimension of back window " << G4endl;
  G4cout << " ceren_hx: " << ceren_hx*2    << " mm " << G4endl;
  G4cout << " ceren_hy: " << ceren_hy*2    << " mm " << G4endl;
  G4cout << " ceren_hz: " << mylarB_hz*2 << " mm " << G4endl;

}

G4bool TBcerenkov01::BuildCeren(G4double x_place, G4double y_place, G4double z_place)
{
  
  G4cout << " Building Cerenkov structure " << G4endl;

  G4cout << " x_place of full Cerenkov detector " << x_place << G4endl;
  G4cout << " y_place of full Cerenkov detector " << y_place << G4endl;
  G4cout << " z_place of full Cerenkov detector " << z_place << G4endl;
  
  translateCeren = G4ThreeVector(x_place, y_place, z_place);
  G4RotationMatrix rotateCeren;
  rotateCeren.rotateY(config_angle);
  transformCeren = new G4Transform3D(rotateCeren, translateCeren);

  new G4PVPlacement(*transformCeren,
	            DetectorLogical,
		    "DetectorPhys",
		    WorldLogVol,
		    0,
		    0);  

// Place gas part of the detector in the middle of detector box
  new G4PVPlacement(0,
	    	    G4ThreeVector(0.,0.,0.),
		    GasLogical,
		    "GasPhys",
		    DetectorLogical,
		    false,
		    0);
  G4VisAttributes *gasColour = new G4VisAttributes(G4Colour(1.,0.,1.));
  gasColour->SetVisibility(true);
  GasLogical->SetVisAttributes(gasColour);
 
  G4double wfront_z = winfront_hthickness/2 - ceren_hthickness/2;
  G4cout << " Front window Z position (centre, wrt detector box): " << wfront_z << " mm " << G4endl;

  new G4PVPlacement(0,
		    G4ThreeVector(0.,0.,wfront_z),
		    MylarFrontLogical,
		    "WinFrontPhys",
		    DetectorLogical,
		    false,
		    0);
  G4VisAttributes *winColour = new G4VisAttributes(G4Colour(0.,1.,0.));
  winColour->SetVisibility(true);
  MylarFrontLogical->SetVisAttributes(winColour);

  G4double wback_z = ceren_hthickness/2 - winback_hthickness/2;
  G4cout << " Back window Z position (centre, wrt detector box): " << wback_z << " mm " << G4endl;

  new G4PVPlacement(0,
		    G4ThreeVector(0.,0.,wback_z),
		    MylarBackLogical,
		    "WinBackPhys",
		    DetectorLogical,
		    false,
		    0);
  MylarBackLogical->SetVisAttributes(winColour);

  return true;
}

void TBcerenkov01::SetSD(G4int iceren)
{

  G4String base = "cerenSD";
  stringstream s;
  s << base << iceren;
  
  G4cout <<" Sensitive detector " << s.str() << G4endl;
  TBSD_Dch01 *cerenSD;
  cerenSD = new TBSD_Dch01(s.str(), 99999.);

  // set active layer
  GasLogical->SetSensitiveDetector(cerenSD);

  // register
  RegisterSensitiveDetector(cerenSD);
}

void TBcerenkov01::SetDepthToLayer(G4int i) {
  depthToLayer = i;
  G4cout <<" DepthToLayer in Cerenkov: " << depthToLayer << G4endl;
}

void TBcerenkov01::Print()
{
  G4cout << "\nTBcerenkov01 information: " << G4endl
	 << " z_place1: "            << z_place1            << " mm " << G4endl
	 << " layer_hthickness: "    << ceren_hthickness    << " mm " << G4endl
	 << " winfront_hthickness: " << winfront_hthickness << " mm " << G4endl
	 << " winback_hthickness: "  << winback_hthickness  << " mm " << G4endl
	 << " gas_hthickness: "      << gas_hthickness      << " mm " << G4endl
	 << " gas_pressure: "        << gas_pressure        << " mm " << G4endl
	 << " gas_temperature: "     << gas_temperature     << " mm " << G4endl
	 << G4endl;  
}
