///#########################################
//                                         #
//  Driver used to simulate the 1000x1000  #
//    muon counters in the CERN07 setup    #
//  (new origin of the coordinate system)  #
//                                         #
//##########################################

#include "TBmuoncount03.hh"
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

INSTANTIATE(TBmuoncount03)

TBmuoncount03::~TBmuoncount03()
{}

G4bool TBmuoncount03::ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
                  G4LogicalVolume *theWorld)
{
 // variable config_angle (a double) will be used by method "construct" to place the detector
 config_angle    = 0.0;
 // variables translateX and translateY have default value (0,0) and are used
 // to translate DCH and Sci wrt beam axis (user defined parameters)
 // translateX      = aGeometryEnvironment.GetParameterAsDouble("TranslateX");
 // translateY      = aGeometryEnvironment.GetParameterAsDouble("TranslateY");
 translateX      = 0.0;
 translateY      = 0.0;
 z_end_tcmt      = aGeometryEnvironment.GetParameterAsDouble("z_end_tcmt");

 G4cout << "\nBuilding TBmuoncount03..." << G4endl;
 db = new Database(aGeometryEnvironment.GetDBName());

 // fetch db parms
 FetchAll();
  
 G4bool doMC1 = SciConstruct (theWorld,ncell_xy[0],ncell_xy[1],x_place1,y_place1,z_place1,1); // 1000x1000 counter

 delete db;
 db = 0;
  
 G4bool doMC = false;
 if (doMC1) doMC = true;

 G4cout << "\nDone building TBmuoncount03" << G4endl;
 return doMC;

}

G4bool TBmuoncount03::SciConstruct(G4LogicalVolume *WorldLog, G4double xdim, G4double ydim,
 		 	           G4double x_place, G4double y_place, G4double z_place, 
			           G4int idsc)
{

  G4cout << " Building Muon Counter " << idsc << G4endl;
  WorldLogVol = WorldLog;

  // depth to layer
  SetDepthToLayer(1);

  G4cout << " Building Muon Counter elements " << G4endl;
  BuildElements(xdim, ydim);

  // do build process
  G4bool cokay = BuildSci(x_place, y_place, z_place, idsc);

  // set Sensitive Detector
  SetSD(idsc);

  return cokay;
}

void TBmuoncount03::FetchAll()
{
  config_angle = 0.0*deg;

  db->exec("select * from mc_virt;");
  db->getTuple();

  ncell_xy[0] = db->fetchDouble("mc_dim_x1")*mm;	// 1000 mm
  ncell_xy[1] = db->fetchDouble("mc_dim_y1")*mm;	// 1000 mm
  assert(ncell_xy[0]>0 && ncell_xy[1]>0);

  z_place1 = z_end_tcmt + db->fetchDouble("z_place1")*mm; // 

  // take into account configuration angle and translation
  x_place1 = translateX*mm;
  y_place1 = translateY*mm;

  sci_hthickness = db->fetchDouble("mc_thickness")*mm;            // 8 mm
  sci_window = db->fetchDouble("steel_thickness")*mm;             // 1 mm

  Print();
}

void TBmuoncount03::BuildElements(G4double xdim, G4double ydim)
{
 
  // materials
  G4String name, symbol;
  G4int nel;
  G4double a, z, density;
  G4double fractionmass;

  // Oxigen
  a = 16.00*g/mole;
  elO = new G4Element(name="Oxigen",   symbol="O",  z=8.,  a);
  // Nitrogen
  a = 14.01*g/mole;
  elN = new G4Element(name="Nitrogen", symbol="N",  z=7.,  a);

  // Air
  density = 1.29e-03*g/cm3; 
  air = new G4Material(name="Air", density, nel=2);
  air->AddElement(elN, fractionmass=0.7);
  air->AddElement(elO, fractionmass=0.3);

  // polystyrene
  poly = CGAGeometryManager::GetMaterial("polystyrene");

  // steel
  steel = CGAGeometryManager::GetMaterial("stainless_steel");

  // sci (half) dimensions
  sci_hx = (xdim)/2;
  sci_hy = (ydim)/2;  
  steelF_hz = sci_window/2;
  steelB_hz = sci_window/2;
  sci_hz = (sci_hthickness/2 + steelF_hz + steelB_hz);

  // detector
  G4Box *DetectorSolid = new G4Box("DetectorSolid",
				   sci_hx,
				   sci_hy,
				   sci_hz);

  DetectorLogical = new G4LogicalVolume(DetectorSolid,
					air,
					"DetectorLogical",
					0,
					0,
					0);

  G4cout << " Dimension of detector box "    << G4endl;
  G4cout << " mc_hx: " << sci_hx*2 << " mm " << G4endl;
  G4cout << " mc_hy: " << sci_hy*2 << " mm " << G4endl;
  G4cout << " mc_hz: " << sci_hz*2 << " mm " << G4endl;

  // Scintillator region
  G4Box *SensitiveRegion = new G4Box("SensitiveRegion",
			 	     sci_hx,
				     sci_hy,
			             sci_hthickness/2);

  SensitiveLogical = new G4LogicalVolume(SensitiveRegion,
				         poly,
				         "SensitiveLogical",
				 	 0,
				         0,
				         0);

  G4cout << " Dimension of scintillator region "   << G4endl;
  G4cout << " mc_hx: " << sci_hx*2       << " mm " << G4endl;
  G4cout << " mc_hy: " << sci_hy*2       << " mm " << G4endl;
  G4cout << " mc_hz: " << sci_hthickness << " mm " << G4endl;

  // Steel front window
  G4Box *SteelFrontWindow = new G4Box("SteelFrontWindow",
				      sci_hx,
				      sci_hy,
				      steelF_hz);
  
  SteelFrontLogical = new G4LogicalVolume(SteelFrontWindow,
					  steel,
					  "SteelFrontLogical",
					  0,
					  0,
					  0);
  
  G4cout << " Dimension of front steel window " << G4endl;
  G4cout << " mc_hx: " << sci_hx*2    << " mm " << G4endl;
  G4cout << " mc_hy: " << sci_hy*2    << " mm " << G4endl;
  G4cout << " mc_hz: " << steelF_hz*2 << " mm " << G4endl;

  // Steel back window
  G4Box *SteelBackWindow = new G4Box("SteelBackWindow",
				     sci_hx,
				     sci_hy,
				     steelB_hz);
  
  SteelBackLogical = new G4LogicalVolume(SteelBackWindow,
					 steel,
					 "SteelBackLogical",
					 0,
					 0,
					 0);
  
  G4cout << " Dimension of back steel window "  << G4endl;
  G4cout << " mc_hx: " << sci_hx*2    << " mm " << G4endl;
  G4cout << " mc_hy: " << sci_hy*2    << " mm " << G4endl;
  G4cout << " mc_hz: " << steelF_hz*2 << " mm " << G4endl;
  
}

G4bool TBmuoncount03::BuildSci(G4double x_place, G4double y_place, G4double z_place,
			   G4int idsc)
{
  
  G4cout << " Building Muon Counter structure: MC " << idsc << G4endl;

  G4cout << " x_place of Muon Counter detector " << x_place << G4endl;
  G4cout << " y_place of Muon Counter detector " << y_place << G4endl;
  G4cout << " z_place of Muon Counter detector " << z_place << G4endl;
  
  translateSci = G4ThreeVector(x_place, y_place, z_place);
  G4RotationMatrix rotateSci;
  rotateSci.rotateY(config_angle);

  transformSci = new G4Transform3D(rotateSci, translateSci);

  new G4PVPlacement(*transformSci,
	            DetectorLogical,
		    "DetectorPhys",
		    WorldLogVol,
		    0,
		    0);  

// Place sentive part of the detector in the middle of detector box
  new G4PVPlacement(0,
	    	    G4ThreeVector(0.,0.,0.),
		    SensitiveLogical,
		    "SensitivePhys",
		    DetectorLogical,
		    false,
		    0);

  G4VisAttributes *sciColour = new G4VisAttributes(G4Colour(1.,0.,1.));
  sciColour->SetVisibility(true);
  SensitiveLogical->SetVisAttributes(sciColour);
 
  G4double wfront_z = sci_window/2 - sci_hz;
  G4cout << " Front window Z position (centre, wrt detector box): " << wfront_z << " mm " << G4endl;

  new G4PVPlacement(0,
		    G4ThreeVector(0.,0.,wfront_z),
		    SteelFrontLogical,
		    "WinFrontPhys",
		    DetectorLogical,
		    false,
		    0);
  G4VisAttributes *winColour = new G4VisAttributes(G4Colour(0.,1.,0.));
  winColour->SetVisibility(true);
  SteelFrontLogical->SetVisAttributes(winColour);

  G4double wback_z = sci_hz - sci_window/2;
  G4cout << " Back window Z position (centre, wrt detector box): " << wback_z << " mm " << G4endl;

  new G4PVPlacement(0,
		    G4ThreeVector(0.,0.,wback_z),
		    SteelBackLogical,
		    "WinBackPhys",
		    DetectorLogical,
		    false,
		    0);
  SteelBackLogical->SetVisAttributes(winColour);

  return true;
}

void TBmuoncount03::SetSD(G4int idch)
{

  G4String base = "mcSD";
  stringstream s;
  s << base << idch;
  
  G4cout <<" Sensitive detector " << s.str() << G4endl;

  G4int xdim, ydim;
  xdim = (G4int) ncell_xy[0];
  ydim = (G4int) ncell_xy[1];
  G4cout <<" Dimensions of sensitive detector " << xdim << "x" << ydim << G4endl;
  
//  TBSD_Dch01 *sciSD;
//  sciSD = new TBSD_Dch01(s.str(), 0.01);

  TBSD_VCell03 *sciSD;
  sciSD = new TBSD_VCell03(s.str(),
			   1.0,
                           xdim,
                           ydim,
                           depthToLayer,
                           TBMUONCOUNT);

  // set active layer
  SensitiveLogical->SetSensitiveDetector(sciSD);

  // register
  RegisterSensitiveDetector(sciSD);
}

void TBmuoncount03::SetDepthToLayer(G4int i) {
  depthToLayer = i;
  G4cout <<" DepthToLayer in Muon Counter: " << depthToLayer << G4endl;
}

void TBmuoncount03::Print()
{
  G4cout << "\nTBmuoncount03 information: " << G4endl
	 << " z_place1: "            << z_place1      << " mm " << G4endl
	 << " mc_hthickness: "       << sci_hz*2      << " mm " << G4endl
	 << G4endl;  
}
