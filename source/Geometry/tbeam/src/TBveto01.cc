///#########################################
//                                         #
//  Driver used to simulate the 1000x1000  #
//  veto counter the CERN07 and FNAL08 tb  #
//  (new origin of the coordinate system)  #
//                                         #
//##########################################

#include "TBveto01.hh"
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

INSTANTIATE(TBveto01)

TBveto01::~TBveto01()
{}

G4bool TBveto01::ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
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

 G4cout << "\nBuilding TBveto01..." << G4endl;
 db = new Database(aGeometryEnvironment.GetDBName());

 // fetch db parms
 FetchAll();
 
 // 1000x1000 scintillator 
 G4bool doVeto1 = SciConstruct (theWorld,ncell_xy[0],ncell_xy[1],x_place1,y_place1,z_place1,1); 

 delete db;
 db = 0;
  
 G4bool doMC = false;
 if (doVeto1) doMC = true;

 G4cout << "\nDone building TBveto01" << G4endl;
 return doMC;

}

G4bool TBveto01::SciConstruct(G4LogicalVolume *WorldLog, G4double xdim, G4double ydim,
 		 	           G4double x_place, G4double y_place, G4double z_place, 
			           G4int idsc)
{

  G4cout << " Building Veto Counter " << idsc << G4endl;
  WorldLogVol = WorldLog;

  // depth to layer
  SetDepthToLayer(1);

  G4cout << " Building Veto Counter elements " << G4endl;
  BuildElements(xdim, ydim);

  // do build process
  G4bool cokay = BuildSci(x_place, y_place, z_place, idsc);

  // set Sensitive Detector
  SetSD(idsc);

  return cokay;
}

void TBveto01::FetchAll()
{
  config_angle = 0.0*deg;

  db->exec("select * from veto_virt;");
  db->getTuple();

  ncell_xy[0] = 1000.0*mm; // db->fetchDouble("veto_dim_x1")*mm;	// 1000 mm
  ncell_xy[1] = 1000.0*mm; // db->fetchDouble("veto_dim_y1")*mm;	// 1000 mm
  ncell_xy[2] =  200.0*mm; // db->fetchDouble("veto_dim_x2")*mm;	//  200 mm
  ncell_xy[3] =  200.0*mm; // db->fetchDouble("veto_dim_y2")*mm;	//  200 mm

  assert(ncell_xy[0]>0 && ncell_xy[1]>0);
  assert(ncell_xy[2]>0 && ncell_xy[3]>0);

  z_place1 = db->fetchDouble("z_place1")*mm;		//- 2272.5 mm (CERN07)
							//- 3686.5 mm (FNAL08)

  // take into account configuration angle and translation
  x_place1 = 0.0*mm;
  y_place1 = 0.0*mm;

  sci_hthickness = db->fetchDouble("veto_thickness")*mm;          // 27 mm
  sci_window = db->fetchDouble("steel_thickness")*mm;             //  1 mm

  Print();
}

void TBveto01::BuildElements(G4double xdim, G4double ydim)
{
 
  // Air
  air = CGAGeometryManager::GetMaterial("air");

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
  G4cout << " veto_hx: " << sci_hx*2 << " mm " << G4endl;
  G4cout << " veto_hy: " << sci_hy*2 << " mm " << G4endl;
  G4cout << " veto_hz: " << sci_hz*2 << " mm " << G4endl;

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
  G4cout << " veto_hx: " << sci_hx*2       << " mm " << G4endl;
  G4cout << " veto_hy: " << sci_hy*2       << " mm " << G4endl;
  G4cout << " veto_hz: " << sci_hthickness << " mm " << G4endl;

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
  G4cout << " steelFront_hx: " << sci_hx*2    << " mm " << G4endl;
  G4cout << " steelFront_hy: " << sci_hy*2    << " mm " << G4endl;
  G4cout << " steelFront_hz: " << steelF_hz*2 << " mm " << G4endl;

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
  G4cout << " steelBack_hx: " << sci_hx*2    << " mm " << G4endl;
  G4cout << " steelBack_hy: " << sci_hy*2    << " mm " << G4endl;
  G4cout << " steelBack_hz: " << steelF_hz*2 << " mm " << G4endl;
  
  // 200x200 hole in scintillator
  sci_hx = ncell_xy[2]/2;
  sci_hy = ncell_xy[3]/2;

  G4Box *HoleSolidSci = new G4Box("HoleSolidSci",
				  sci_hx,
				  sci_hy,
				  sci_hthickness/2);

  HoleLogicalSci = new G4LogicalVolume(HoleSolidSci,
				       air,
				       "HoleLogicalSci",
				       0,
				       0,
				       0);

  G4cout << " Dimension of hole in scintillator part of the veto " << G4endl;
  G4cout << " hole_hx: " << sci_hx*2 << " mm " << G4endl;
  G4cout << " hole_hy: " << sci_hy*2 << " mm " << G4endl;
  G4cout << " hole_hz: " << sci_hthickness << " mm " << G4endl;

  // 200x200 hole in front window
  G4Box *HoleSolidFW = new G4Box("HoleSolidFW",
				 sci_hx,
				 sci_hy,
				 steelF_hz);

  HoleLogicalFW = new G4LogicalVolume(HoleSolidFW,
				      air,
				      "HoleLogicalFW",
				      0,
				      0,
				      0);

  G4cout << " Dimension of hole in front window of the veto " << G4endl;
  G4cout << " hole_hx: " << sci_hx*2 << " mm " << G4endl;
  G4cout << " hole_hy: " << sci_hy*2 << " mm " << G4endl;
  G4cout << " hole_hz: " << steelF_hz*2 << " mm " << G4endl;

  // 200x200 hole in front window
  G4Box *HoleSolidBW = new G4Box("HoleSolidBW",
				 sci_hx,
				 sci_hy,
				 steelB_hz);

  HoleLogicalBW = new G4LogicalVolume(HoleSolidBW,
				      air,
				      "HoleLogicalBW",
				      0,
				      0,
				      0);

  G4cout << " Dimension of hole in back window of the veto " << G4endl;
  G4cout << " hole_hx: " << sci_hx*2 << " mm " << G4endl;
  G4cout << " hole_hy: " << sci_hy*2 << " mm " << G4endl;
  G4cout << " hole_hz: " << steelB_hz*2 << " mm " << G4endl;

}

G4bool TBveto01::BuildSci(G4double x_place, G4double y_place, G4double z_place,
			   G4int idsc)
{
  
  G4cout << " Building Veto Counter structure: " << idsc << G4endl;

  G4cout << " x_place " << x_place << G4endl;
  G4cout << " y_place " << y_place << G4endl;
  G4cout << " z_place " << z_place << G4endl;
  
  translateSci = G4ThreeVector(x_place, y_place, z_place);
  G4RotationMatrix rotateSci;
  rotateSci.rotateY(config_angle);

  transformSci = new G4Transform3D(rotateSci, translateSci);

  // Place detector box in World volume
  new G4PVPlacement(*transformSci,
	            DetectorLogical,
		    "DetectorPhys",
		    WorldLogVol,
		    0,
		    0);  

  // Place sentive part of the detector (scintillator) in the middle of detector box
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

  // Place the hole in the middle of scintillator box
  new G4PVPlacement(0,
	    	    G4ThreeVector(0.,0.,0.),
		    HoleLogicalSci,
		    "HolePhysSci",
		    SensitiveLogical,
		    false,
		    0);
  G4VisAttributes *holeColour = new G4VisAttributes(G4Colour(1.,1.,1.));
  holeColour->SetVisibility(true);
  HoleLogicalSci->SetVisAttributes(holeColour);

  // Front steel window
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

  // Place the hole in the middle of the front window
  new G4PVPlacement(0,
	    	    G4ThreeVector(0.,0.,0.),
		    HoleLogicalFW,
		    "HolePhysFW",
		    SteelFrontLogical,
		    false,
		    0);
  holeColour->SetVisibility(true);
  HoleLogicalFW->SetVisAttributes(holeColour);
  
  // Back steel window
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

  // Place the hole in the middle of the back window
  new G4PVPlacement(0,
	    	    G4ThreeVector(0.,0.,0.),
		    HoleLogicalBW,
		    "HolePhysBW",
		    SteelBackLogical,
		    false,
		    0);
  holeColour->SetVisibility(true);
  HoleLogicalBW->SetVisAttributes(holeColour);
  
  return true;
}

void TBveto01::SetSD(G4int idch)
{

  G4String base = "vetoSD";
  stringstream s;
  s << base << idch;
  
  G4cout <<" Sensitive detector " << s.str() << G4endl;

  G4int xdim, ydim;

  xdim = (G4int) ncell_xy[0];
  ydim = (G4int) ncell_xy[1];

  G4cout <<" Dimensions of sensitive detector " << xdim << "x" << ydim << G4endl;
  
  //  TBSD_Dch01 *vetoSD;
  //  vetoSD = new TBSD_Dch01(s.str(), 0.1);
  
  TBSD_VCell03 *vetoSD;
  vetoSD = new TBSD_VCell03(s.str(),
			    1.0,
			    xdim,
			    ydim,
			    depthToLayer,
			    TBVETOCOUNT);
  
  // set active layer
  SensitiveLogical->SetSensitiveDetector(vetoSD);

  // register
  RegisterSensitiveDetector(vetoSD);
}

void TBveto01::SetDepthToLayer(G4int i) {
  depthToLayer = i;
  G4cout <<" DepthToLayer in Veto Counter: " << depthToLayer << G4endl;
}

void TBveto01::Print()
{
  G4cout << "\nTBveto01 information: " << G4endl
	 << " z_place1: "            << z_place1      << " mm " << G4endl
	 << " veto_hthickness: "       << sci_hz*2      << " mm " << G4endl
	 << G4endl;  
}
