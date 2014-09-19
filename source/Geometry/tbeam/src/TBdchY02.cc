//#####################################
//                                    #
// Driver used to simulate the drift  #
// chambers in the 2006 Desy setup    #
//                                    #
//#####################################

#include "TBdchY02.hh"
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

INSTANTIATE(TBdchY02)

TBdchY02::~TBdchY02()
{}

G4bool TBdchY02::ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
                G4LogicalVolume *theWorld)
{
 ecal_front_face = aGeometryEnvironment.GetParameterAsDouble("ecal_begin_z");
 // variable config_angle (a double) will be used by method "construct" to place the detector
 config_angle    = aGeometryEnvironment.GetParameterAsDouble("configuration_angle");
 // variables translateX and translateY have default value (0,0) and are used
 // to translate DCH and Sci wrt beam axis (user defined parameters)
 translateX      = aGeometryEnvironment.GetParameterAsDouble("TranslateX");
 translateY      = aGeometryEnvironment.GetParameterAsDouble("TranslateY");

 G4cout << "\nBuilding TBdchY02..." << G4endl;
 db = new Database(aGeometryEnvironment.GetDBName());
 // fetch db parms
 FetchAll();
  
 G4bool doDch1 = DchConstruct (theWorld,x_place1,y_place1,z_place1,1);
 G4bool doDch2 = DchConstruct (theWorld,x_place2,y_place2,z_place2,2);
 G4bool doDch3 = DchConstruct (theWorld,x_place3,y_place3,z_place3,3);
 G4bool doDch4 = DchConstruct (theWorld,x_place4,y_place4,z_place4,4);

 delete db;
 db = 0;
  
 G4bool doDch = false;
 if (doDch1 && doDch2 && doDch3 && doDch4) doDch = true;

 G4cout << "\nDone building TBdchY02" << G4endl;
 return doDch;

}

G4bool TBdchY02::DchConstruct(G4LogicalVolume *WorldLog, 
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
  G4bool cokay = BuildDch(x_place, y_place, z_place);

  // set Sensitive Detector
  SetSD(idch);

  return cokay;
}

void TBdchY02::FetchAll()
{
  config_angle = config_angle*deg;

  db->exec("select * from dch_virt;");
  db->getTuple();

  ncell_xy[0] = db->fetchDouble("dch_dim_x");		// 72 mm
  ncell_xy[1] = db->fetchDouble("dch_dim_y");		// 72 mm
  assert(ncell_xy[0]>0 && ncell_xy[1]>0);

  z_place1 = db->fetchDouble("z_placeY1");		//  638mm from ECAL front face
  z_place2 = db->fetchDouble("z_placeY2");		// 1738mm from ECAL front face
  z_place3 = db->fetchDouble("z_placeY3");		// 2648mm from ECAL front face
  z_place4 = db->fetchDouble("z_placeY4");		// 3748mm from ECAL front face

// take into account configuration angle and translation
  x_place1 = translateX + (ecal_front_face + z_place1)*sin(config_angle);
  y_place1 = translateY;
  z_place1 = (ecal_front_face + z_place1)*cos(config_angle);

  x_place2 = translateX + (ecal_front_face + z_place2)*sin(config_angle);
  y_place2 = translateY;
  z_place2 = (ecal_front_face + z_place2)*cos(config_angle);

  x_place3 = translateX + (ecal_front_face + z_place3)*sin(config_angle);
  y_place3 = translateY;
  z_place3 = (ecal_front_face + z_place3)*cos(config_angle);

  x_place4 = translateX + (ecal_front_face + z_place4)*sin(config_angle);
  y_place4 = translateY;
  z_place4 = (ecal_front_face + z_place4)*cos(config_angle);

  db->exec("select * from layer_thickness;");
  db->getTuple();

  winback_hthickness = db->fetchDouble("winback_thickness");    // 20 micron
  dch_hthickness = db->fetchDouble("dch_thickness");            // 44 mm
  gas_hthickness = dch_hthickness - winback_hthickness;

  gas_temperature = STP_Temperature;
  gas_pressure = db->fetchDouble("gas_pressure");               // 1.2 atm (+0.2Kg/cm^2)
  gas_pressure = gas_pressure*atmosphere;

  grid_size = 1;

  Print();
}

void TBdchY02::BuildElements()
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

  // Ethane
  density  = 1.36e-3*g/cm3;
  C2H6 = new G4Material(name="Ethane", density, nel=2, kStateGas);
  C2H6->AddElement(elC, natoms=2);
  C2H6->AddElement(elH, natoms=6);

  // Gas mixture in DCH is 96% Ar and 4% Ethane (C2H6)
  density  = ((1.78e-3)*0.96 + (1.36e-3)*0.04)*g/cm3;
  temperature = gas_temperature;
  pressure    = gas_pressure;
  gas_mix = new G4Material(name="mixture", density, nel=2, kStateGas, temperature, pressure);
  gas_mix->AddMaterial(elAr, 96*perCent);
  gas_mix->AddMaterial(C2H6, 4*perCent);

  // Mylar
  density  = 1.39*g/cm3;
  mylar = new G4Material(name="Mylar", density, nel=3);
  mylar->AddElement(elC, natoms=3);
  mylar->AddElement(elH, natoms=4);
  mylar->AddElement(elO, natoms=2);

  // dch (half) dimensions 
  dch_hx = (ncell_xy[0])/2;
  dch_hy = (ncell_xy[1])/2;
  dch_hz = dch_hthickness/2;
  gas_hz = gas_hthickness/2;
  mylarB_hz = winback_hthickness/2;

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

  // Mylar back window
  G4Box *MylarBackWindow = new G4Box("MylarBackWindow",
			 	      dch_hx,
				      dch_hy,
			              mylarB_hz);

  MylarBackLogical = new G4LogicalVolume(MylarBackWindow,
				          mylar,
				          "MylarBackLogical",
				 	  0,
				          0,
				          0);

  G4cout << " Dimension of back window " << G4endl;
  G4cout << " dch_hx: " << dch_hx*2    << " mm " << G4endl;
  G4cout << " dch_hy: " << dch_hy*2    << " mm " << G4endl;
  G4cout << " dch_hz: " << mylarB_hz*2 << " mm " << G4endl;

}

G4bool TBdchY02::BuildDch(G4double x_place, G4double y_place, G4double z_place)
{
  
  G4cout << " Building Dch structure " << G4endl;

  G4cout << " x_place of full Dch detector " << x_place << G4endl;
  G4cout << " y_place of full Dch detector " << y_place << G4endl;
  G4cout << " z_place of full Dch detector " << z_place << G4endl;
  
  translateDch = G4ThreeVector(x_place, y_place, z_place);
  G4RotationMatrix rotateDch;
  rotateDch.rotateY(config_angle);
  transformDch = new G4Transform3D(rotateDch, translateDch);

  new G4PVPlacement(*transformDch,
	            DetectorLogical,
		    "DetectorPhys",
		    WorldLogVol,
		    0,
		    0);  

// Place gas part of the detector in the middle of detector box
  G4double gas_z = - winback_hthickness/2;
  new G4PVPlacement(0,
	    	    G4ThreeVector(0.,0.,gas_z),
		    GasLogical,
		    "GasPhys",
		    DetectorLogical,
		    false,
		    0);
  G4VisAttributes *gasColour = new G4VisAttributes(G4Colour(1.,1.,1.));
  gasColour->SetVisibility(true);
  GasLogical->SetVisAttributes(gasColour);
 
  G4double wback_z = dch_hthickness/2 - winback_hthickness/2;
  G4cout << " Back window Z position (centre, wrt detector box): " << wback_z << " mm " << G4endl;

  new G4PVPlacement(0,
		    G4ThreeVector(0.,0.,wback_z),
		    MylarBackLogical,
		    "WinBackPhys",
		    DetectorLogical,
		    false,
		    0);
  G4VisAttributes *winColour = new G4VisAttributes(G4Colour(0.,1.,0.));
  winColour->SetVisibility(true);
  MylarBackLogical->SetVisAttributes(winColour);

  return true;
}

void TBdchY02::SetSD(G4int idch)
{

  G4String base = "dchSDy";
  stringstream s;
  s << base << idch;
  
  G4cout <<" Sensitive detector " << s.str() << G4endl;
  TBSD_Dch01 *dchSD;
  dchSD = new TBSD_Dch01(s.str(), 0.001);

  // set active layer
  GasLogical->SetSensitiveDetector(dchSD);

  // register
  RegisterSensitiveDetector(dchSD);
}

void TBdchY02::SetDepthToLayer(G4int i) {
  depthToLayer = i;
  G4cout <<" DepthToLayer in Dch: " << depthToLayer << G4endl;
}

void TBdchY02::Print()
{
  G4cout << "\nTBdchY02 information: " << G4endl
	 << " ecal_front_face: "     << ecal_front_face     << " mm " << G4endl
	 << " z_place1: "            << z_place1            << " mm " << G4endl
	 << " z_place2: "            << z_place2            << " mm " << G4endl
	 << " z_place3: "            << z_place3            << " mm " << G4endl
	 << " z_place4: "            << z_place4            << " mm " << G4endl
	 << " layer_hthickness: "    << dch_hthickness      << " mm " << G4endl
	 << " winfront_hthickness: " << winfront_hthickness << " mm " << G4endl
	 << " winback_hthickness: "  << winback_hthickness  << " mm " << G4endl
	 << " gas_hthickness: "      << gas_hthickness      << " mm " << G4endl
	 << " gas_pressure: "        << gas_pressure        << " mm " << G4endl
	 << " gas_temperature: "     << gas_temperature     << " mm " << G4endl
	 << G4endl;  
}
