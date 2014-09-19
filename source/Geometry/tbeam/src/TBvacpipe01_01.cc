//###############################################
//                                              #
//Driver used to simulate the Vaccum pipe       #
//  downstream of the Cerenkov detector         # 
//            in the CERN06 setup               #
//                                              #
//###############################################

#include "TBvacpipe01_01.hh"
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

INSTANTIATE(TBvacpipe01_01)

TBvacpipe01_01::~TBvacpipe01_01()
{}

G4bool TBvacpipe01_01::ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
                G4LogicalVolume *theWorld)
{

 // variable config_angle (a double) will be used by method "construct" to place the detector
 config_angle    = 0.0;
 // variables translateX and translateY have default value (0,0) and are used
 // to translate Vacuum pipe wrt beam axis (user defined parameters)
 // translateX      = aGeometryEnvironment.GetParameterAsDouble("TranslateX");
 // translateY      = aGeometryEnvironment.GetParameterAsDouble("TranslateY");
 translateX      = 0.0*mm;
 translateY      = 0.0*mm;

 G4cout << "\nBuilding TBvacpipe01_01..." << G4endl;
 db = new Database(aGeometryEnvironment.GetDBName());
 // fetch db parms
 FetchAll();
  
 G4bool doVacPipe = VacConstruct (theWorld,x_place1,y_place1,z_place1,1);

 delete db;
 db = 0;
  
 G4cout << "\nDone building TBvacpipe01_01" << G4endl;
 return doVacPipe;

}

G4bool TBvacpipe01_01::VacConstruct(G4LogicalVolume *WorldLog, 
				    G4double x_place, G4double y_place, G4double z_place, 
				    G4int ivac)
{

  G4cout << " Building Vacuum pipe downstream of the Cerenkov " << ivac << G4endl;
  WorldLogVol = WorldLog;

  // depth to layer
  SetDepthToLayer(1);

  G4cout << " Building Vacuum pipe elements " << G4endl;
  BuildElements();

  // do build process
  G4bool cokay = BuildVac(x_place, y_place, z_place);

  return cokay;
}

void TBvacpipe01_01::FetchAll()
{
  config_angle = 0.0*deg;

  db->exec("select * from vacpipe_virt;");
  db->getTuple();

  ncell_xy[0] = db->fetchDouble("vacpipe_dim_x");		// 100 mm
  ncell_xy[1] = db->fetchDouble("vacpipe_dim_y");		// 100 mm
  assert(ncell_xy[0]>0 && ncell_xy[1]>0);

  z_place1 = db->fetchDouble("z_place1");		// 

// take into account configuration angle and translation
  x_place1 = translateX;
  y_place1 = translateY;
  
  vacpipe_hthickness = db->fetchDouble("vacpipe_thickness");      //9000 mm

  grid_size = 1;

  Print();
}

void TBvacpipe01_01::BuildElements()
{
 
  // Define vacuum as very low density material
  G4double atomicNumber = 1.;
  G4double massOfMole   = 1.008*g/mole;
  G4double density      = 1.e-25*g/cm3;

  temperature           = 2.73*kelvin;
  pressure              = 3.e-18*pascal;

  Vacuum = new G4Material("vacuumPipe", atomicNumber, 
			  massOfMole, density, kStateGas,
			  temperature, pressure);

  // Vacuum pipe (half) dimensions 
  vacpipe_hx = (ncell_xy[0])/2;
  vacpipe_hy = (ncell_xy[1])/2;
  vacpipe_hz = vacpipe_hthickness/2;

  // detector
  G4Box *DetectorSolid = new G4Box("DetectorSolid",
				   vacpipe_hx,
				   vacpipe_hy,
				   vacpipe_hz);

  DetectorLogical = new G4LogicalVolume(DetectorSolid,
					Vacuum,
					"DetectorLogical",
					0,
					0,
					0);

  // Vacuum
  G4Box *GasSolid = new G4Box("GasSolid",
			      vacpipe_hx,
			      vacpipe_hy,
			      vacpipe_hz);
  
  GasLogical = new G4LogicalVolume(GasSolid,
				   Vacuum,
				   "GasLogical",
				   0,
				   0,
				   0);

  G4cout << " Dimension of vacuum pipe " << G4endl;
  G4cout << " vacpipe_hx: " << vacpipe_hx*2 << " mm " << G4endl;
  G4cout << " vacpipe_hy: " << vacpipe_hy*2 << " mm " << G4endl;
  G4cout << " vacpipe_hz: " << vacpipe_hz*2 << " mm " << G4endl;

}

G4bool TBvacpipe01_01::BuildVac(G4double x_place, G4double y_place, G4double z_place)
{
  
  G4cout << " Building Vacuum pipe structure " << G4endl;

  G4cout << " x_place of full Vacuum pipe detector " << x_place << G4endl;
  G4cout << " y_place of full Vacuum pipe detector " << y_place << G4endl;
  G4cout << " z_place of full Vacuum pipe detector " << z_place << G4endl;
  
  translateVac = G4ThreeVector(x_place, y_place, z_place);
  G4RotationMatrix rotateVac;
  rotateVac.rotateY(config_angle);
  transformVac = new G4Transform3D(rotateVac, translateVac);

  new G4PVPlacement(*transformVac,
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
 
  return true;
}

void TBvacpipe01_01::SetDepthToLayer(G4int i) {
  depthToLayer = i;
  G4cout <<" DepthToLayer in Vacuum pipe: " << depthToLayer << G4endl;
}

void TBvacpipe01_01::Print()
{
  G4cout << "\nTBvacpipe01_01 information: " << G4endl
	 << " z_place1: "            << z_place1            << " mm " << G4endl
	 << " layer_hthickness: "    << vacpipe_hthickness    << " mm " << G4endl
	 << " gas_pressure: "        << pressure        << " mm " << G4endl
	 << " gas_temperature: "     << temperature     << " mm " << G4endl
	 << G4endl;  
}
