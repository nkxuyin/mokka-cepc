#include "TBscint01.hh"
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

INSTANTIATE(TBscint01)

TBscint01::~TBscint01()
{}

G4bool TBscint01::ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
                  G4LogicalVolume *theWorld)
{
 ecal_front_face = aGeometryEnvironment.GetParameterAsDouble("ecal_begin_z");
 // variable config_angle (a double) will be used by method "construct" to place the detector
 config_angle    = aGeometryEnvironment.GetParameterAsDouble("configuration_angle");
 // variables translateX and translateY have default value (0,0) and are used
 // to translate DCH and Sci wrt beam axis (user defined parameters)
 translateX      = aGeometryEnvironment.GetParameterAsDouble("TranslateX");
 translateY      = aGeometryEnvironment.GetParameterAsDouble("TranslateY");

 G4cout << "\nBuilding TBscint01..." << G4endl;
 db = new Database(aGeometryEnvironment.GetDBName());

 // fetch db parms
 FetchAll();
  
 G4bool doSci1 = SciConstruct (theWorld,ncell_xy[0],ncell_xy[1],x_place1,y_place1,z_place1,1); //  5x5 counter
 G4bool doSci2 = SciConstruct (theWorld,ncell_xy[0],ncell_xy[1],x_place2,y_place2,z_place2,2); //  5x5 counter
 G4bool doSci3 = SciConstruct (theWorld,ncell_xy[2],ncell_xy[3],x_place3,y_place3,z_place3,3); // 12x12 counter
 G4bool doSci4 = SciConstruct (theWorld,ncell_xy[2],ncell_xy[3],x_place4,y_place4,z_place4,4); // 12x12 counter

 delete db;
 db = 0;
  
 G4bool doSci = false;
 if (doSci1 && doSci2 && doSci3 && doSci4) doSci = true;

 G4cout << "\nDone building TBscint01" << G4endl;
 return doSci;

}

G4bool TBscint01::SciConstruct(G4LogicalVolume *WorldLog, G4double xdim, G4double ydim,
 		 	       G4double x_place, G4double y_place, G4double z_place, 
			       G4int idsc)
{

  G4cout << " Building Scintillator " << idsc << G4endl;
  WorldLogVol = WorldLog;

  // depth to layer
  SetDepthToLayer(1);

  G4cout << " Building Scintillator elements " << G4endl;
  BuildElements(xdim, ydim);

  // do build process
  G4bool cokay = BuildSci(x_place, y_place, z_place, idsc);

  // set Sensitive Detector
  SetSD(idsc);

  return cokay;
}

void TBscint01::FetchAll()
{
  diamond_angle = 45*deg;
  config_angle = config_angle*deg;

  db->exec("select * from sci_virt;");
  db->getTuple();

  ncell_xy[0] = db->fetchDouble("sci_dim_x1");		// 50 mm
  ncell_xy[1] = db->fetchDouble("sci_dim_y1");		// 50 mm
  assert(ncell_xy[0]>0 && ncell_xy[1]>0);
  ncell_xy[2] = db->fetchDouble("sci_dim_x2");          // 120 mm
  ncell_xy[3] = db->fetchDouble("sci_dim_y2");          // 120 mm
  assert(ncell_xy[2]>0 && ncell_xy[3]>0);

  z_place1 = db->fetchDouble("z_place1");		//  2192.5 mm from ECAL front face
  z_place2 = db->fetchDouble("z_place2");		//  2186.5 mm from ECAL front face
  z_place3 = db->fetchDouble("z_place3");		//   423.5 mm from ECAL front face
  z_place4 = db->fetchDouble("z_place4");		//  3596.5 mm from ECAL front face

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

  sci_hthickness = db->fetchDouble("sci_thickness");            // 5 mm

  Print();
}

void TBscint01::BuildElements(G4double xdim, G4double ydim)
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

  // sci (half) dimensions
  sci_hx = (xdim)/2;
  sci_hy = (ydim)/2;  
  sci_hz = sci_hthickness/2;

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

  // Scintillator region
  G4Box *SensitiveRegion = new G4Box("SensitiveRegion",
			 	     sci_hx,
				     sci_hy,
			             sci_hz);

  SensitiveLogical = new G4LogicalVolume(SensitiveRegion,
				         poly,
				         "SensitiveLogical",
				 	 0,
				         0,
				         0);

  G4cout << " Dimension of detector box " << G4endl;
  G4cout << " sci_hx: " << sci_hx*2 << " mm " << G4endl;
  G4cout << " sci_hy: " << sci_hy*2 << " mm " << G4endl;
  G4cout << " sci_hz: " << sci_hz*2 << " mm " << G4endl;

}

G4bool TBscint01::BuildSci(G4double x_place, G4double y_place, G4double z_place,
			   G4int idsc)
{
  
  G4cout << " Building Scintillator structure " << G4endl;

  G4cout << " x_place of Scintillator detector " << x_place << G4endl;
  G4cout << " y_place of Scintillator detector " << y_place << G4endl;
  G4cout << " z_place of Scintillator detector " << z_place << G4endl;
  
  translateSci = G4ThreeVector(x_place, y_place, z_place);
  G4RotationMatrix rotateSci;
  rotateSci.rotateY(config_angle);

  // Scintillators 3 and 4 are in diamond shape
  if (idsc == 3 || idsc == 4) {
    G4RotationMatrix rotateSciDiamond;
    rotateSciDiamond.rotateZ(diamond_angle);
    rotateSci = rotateSci*rotateSciDiamond;
  }
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
 
  return true;
}

void TBscint01::SetSD(G4int idch)
{

  G4String base = "sciSD";
  stringstream s;
  s << base << idch;
  
  G4cout <<" Sensitive detector " << s.str() << G4endl;

  G4int xdim, ydim;
  if (idch < 3) {
    xdim = (G4int) ncell_xy[0];
    ydim = (G4int) ncell_xy[1]; 
    G4cout <<" Dimensions of sensitive detector " << xdim << "x" << ydim << G4endl;
    }
  else {
    xdim = (G4int) ncell_xy[2];
    ydim = (G4int) ncell_xy[3]; 
    G4cout <<" Dimensions of sensitive detector " << xdim << "x" << ydim << G4endl;
  }

  //  TBSD_Dch01 *sciSD;
  //  sciSD = new TBSD_Dch01(s.str(), 0.001);

  TBSD_VCell03 *sciSD;
  sciSD = new TBSD_VCell03(s.str(),
			   1.0,
                           xdim,
                           ydim,
                           depthToLayer,
                           TBSCINT);

  // set active layer
  SensitiveLogical->SetSensitiveDetector(sciSD);

  // register
  RegisterSensitiveDetector(sciSD);
}

void TBscint01::SetDepthToLayer(G4int i) {
  depthToLayer = i;
  G4cout <<" DepthToLayer in Scintillator: " << depthToLayer << G4endl;
}

void TBscint01::Print()
{
  G4cout << "\nTBscint01 information: " << G4endl
	 << " ecal_front_face: "     << ecal_front_face     << " mm " << G4endl
	 << " z_place1: "            << z_place1            << " mm " << G4endl
	 << " z_place2: "            << z_place2            << " mm " << G4endl
	 << " z_place3: "            << z_place3            << " mm " << G4endl
	 << " z_place4: "            << z_place4            << " mm " << G4endl
	 << " sci_hthickness: "      << sci_hthickness      << " mm " << G4endl
	 << G4endl;  
}
