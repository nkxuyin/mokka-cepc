//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for Tesla -      *
//*                                                     *
//* For more information about Mokka, please, go to the *
//*                                                     *
//*  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
//*                                                     *
//* Mokka home page.                                    *
//*                                                     *
//*******************************************************
//
// $Id: Proto05.cc,v 1.5 2009/02/11 13:32:46 musat Exp $
// $Name: mokka-07-00 $
//
//
// TBscecal01.cc
//
// History:  

#include "MyPlacement.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4Material.hh"
#include "G4SubtractionSolid.hh"

#include "TBscecal01.hh"
#include "CGAGeometryManager.hh"

#include "globals.hh"
#include "G4GeometryTolerance.hh"
#include "G4Box.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4UserLimits.hh"
#include "MySQLWrapper.hh"
#include "Control.hh"

#include "G4Region.hh"
//#include "ProtoSD03.hh"
//#include "TRKSD00.hh"

#ifdef MOKKA_GEAR
#include "gear/CalorimeterParameters.h" 
#include "gearimpl/CalorimeterParametersImpl.h" 
#include "gearimpl/LayerLayoutImpl.h" 
#include "MokkaGear.h"
#endif

#include <algorithm>
#include <stdlib.h>

#include "CGADefs.h"

INSTANTIATE(TBscecal01)

G4bool TBscecal01::ContextualConstruct(
		const CGAGeometryEnvironment &aGeometryEnvironment,
		G4LogicalVolume *theWorld) {

	grid_size = aGeometryEnvironment.GetParameterAsDouble("grid_size"); //default = 5.0 ( mm ) 

	config_angle = 
           aGeometryEnvironment.GetParameterAsDouble("configuration_angle")
		+
           aGeometryEnvironment.GetParameterAsDouble("EcalRotationAngle");

	G4double XTranslation = aGeometryEnvironment.GetParameterAsDouble("EcalTranslateX");
	G4double YTranslation = aGeometryEnvironment.GetParameterAsDouble("EcalTranslateY");

	TranslationVector = G4ThreeVector(XTranslation, YTranslation, 0.0);

        absorberDens    = aGeometryEnvironment.GetParameterAsDouble("absorberDensity"); //default = 14.7
        cable_etcDens    = aGeometryEnvironment.GetParameterAsDouble("cable_etcDensity"); //Default = 0.8293

        massFraction_W  = aGeometryEnvironment.GetParameterAsDouble("massFraction_W"); // default = 0.8163
        massFraction_C  = aGeometryEnvironment.GetParameterAsDouble("massFraction_C"); // default = 0.0553
        massFraction_Co = aGeometryEnvironment.GetParameterAsDouble("massFraction_Co"); // default = 0.1249
        massFraction_Cr = aGeometryEnvironment.GetParameterAsDouble("massFraction_Cr"); // default = 0.0045
        
        
	G4String trkUse = aGeometryEnvironment.GetParameterAsString("jacker");

	if(trkUse == "true")
		useTracker = true;
	else
		useTracker = false;

//TODO	theCellProtoSD = 0;
//TODO	theGRProtoSD = 0;
//TODO	
        theTRKSD = 0;
	return construct (aGeometryEnvironment.GetDBName(),theWorld);
}
	
G4bool TBscecal01::construct(const G4String &aSubDetectorName,
			  G4LogicalVolume *WorldLog)

{
  G4cout << "\nBuilding TBscecal release 01" << G4endl;
  this->WorldLog = WorldLog;
//  PlateGroups.clear();
//  Plates.clear();
  theSubDetectorName = aSubDetectorName;

  db = new Database(aSubDetectorName.data());
  
//TODO  if(Control::DUMPG3) MyPlacement::Init("TBSCECAL",aSubDetectorName);

//
  DefineMaterial();

  // BuildElements takes the main parameters and builds 
  // the basic logical volumes
  BuildElements();
  
  //--------------
  // Detector:
  //--------------

//  BuildStructures();
  BuildEcal();
//TODO  DetectorLogical->SetVisAttributes(VisAttAir);
  
  G4double x_center,y_center,z_center;
  x_center=y_center=z_center=0; 
  db->exec("select x_center,y_center,z_center from center_pos;");
  if(db->getTuple())
    {
      x_center = db->fetchDouble("x_center");
      y_center = db->fetchDouble("y_center");
      z_center = db->fetchDouble("z_center");
    }
  else
    {
      Control::Log("no database values of (x_center,y_center,z_center) , assuming (0.,0., 2*cal_hy-->cal_hz after turn).");
      z_center = 2*cal_hy;
    }

  EcalPosition = G4ThreeVector(x_center, y_center, z_center);


  EcalRotation = new G4RotationMatrix();
  EcalRotation->rotateX(-pi*0.5);

  G4ThreeVector displacement(0.0, 0.0, 0.0);//TODO 
//  displacement = (*EcalRotation) * displacement;
//  EcalPosition += displacement;
//  G4RotationMatrix *EcalPartialRotation = new G4RotationMatrix();
//  EcalPartialRotation->rotateY(-config_angle*pi/180.0);

//  G4ThreeVector displacement1(-max+fabs(struct_shift[2]) +g10_x_out+
//		  HalfEcalX, 0.0, 0.0);
//  displacement1 = (*EcalPartialRotation)*displacement1;

//  EcalPosition += displacement1;

//  EcalRotation->rotateZ(-config_angle*pi/180.0);
//  EcalPosition += TranslationVector;

//  start_layer_number=0;

//  db->exec("select start_layer_number from proto;");
//  db->getTuple();
//  start_layer_number = db->fetchInt("start_layer_number");
  
  // sensitive detector plugin
  //  G4String theSDName = G4String("ProtoSD") + theSubDetectorName.data();
//TODO
//SetSensitiveDetector" was done in "BuildElements"
/*  G4String theCellSDName = G4String("ProtoSD03");
  G4String theGRSDName = G4String("ProtoSD03GuardRing");

  if(theCellProtoSD==0)
    {
      theCellProtoSD = new
	ProtoSD03(cell_dim_x, cell_dim_z, n_cell_x, n_cell_z, 
		n_waffers_x, n_waffers_z + 1, 
		upper_scinti_shift + wafer_x_shift,
		garde_size, cell_y_pos_in_alveolus, alveolus_y_spacing,
		exit_fiber_thickness,
		slab_shifts_vector,// waffer et slab shifts
		struct_shift,StructHalfX,
		&EcalPosition, EcalRotation,
		theCellSDName, start_layer_number,
		inter_waffer_gap, HalfAlveolusX,
		max, HalfEcalX, HalfEcalY, 
		inter_tower_fiber_thickness + al_cf_z_gap,
		inter_structures_gap, n_guard_ring_zones,
		lateralWaferGap, endcap_x, useID1);
      RegisterSensitiveDetector(theCellProtoSD);
    }

  siCellLogical->SetSensitiveDetector(theCellProtoSD);

  if(theGRProtoSD==0)
    {
      theGRProtoSD = new
	ProtoSD03(cell_dim_x, cell_dim_z, n_cell_x, n_cell_z, 
		n_waffers_x, n_waffers_z + 1, 
		upper_scinti_shift + wafer_x_shift,
		garde_size, cell_y_pos_in_alveolus, alveolus_y_spacing,
		exit_fiber_thickness,
		slab_shifts_vector,// waffer et slab shifts
		struct_shift,StructHalfX,
		&EcalPosition, EcalRotation,
		theGRSDName, start_layer_number,
		inter_waffer_gap, HalfAlveolusX,
		max, HalfEcalX, HalfEcalY, 
		inter_tower_fiber_thickness + al_cf_z_gap,
		inter_structures_gap, n_guard_ring_zones,
		lateralWaferGap, endcap_x,true);
      RegisterSensitiveDetector(theGRProtoSD);
    }

  SiWafferLogical->SetSensitiveDetector(theGRProtoSD); // for the guard-ring


  if((useTracker) && (theTRKSD == 0)){
	theTRKSD = new TRKSD00("ProtoTRKSD03", 0, Control::TPCCut);
	RegisterSensitiveDetector(theTRKSD);
	TrackerLogical->SetSensitiveDetector(theTRKSD);
  }
*/

  //--------------
  // Detector Placement
  //--------------
  char buff[80];
  sprintf(buff,"TBscecal01: Size is (%f,%f,%f) mm",2*cal_hx, 2*cal_hz, 2*cal_hy);
  Control::Log(buff);
  sprintf(buff,"TBscecal01: placing prototype at (%f,%f,%f) mm",
		  EcalPosition(0), EcalPosition(1), EcalPosition(2));
  Control::Log(buff);
  
  new MyPlacement(EcalRotation,
  		    EcalPosition,
		    DetectorLogical,
		    "EcalDetectorPhys",
		    WorldLog,
		    false,0);

//TODO  CGAGeometryManager::GetCGAGeometryManager()->
//	RegisterGeometryRegion(G10Region, Control::PCBRangeCut);

//TODO  CGAGeometryManager::GetCGAGeometryManager()->
//	RegisterGeometryRegion(WRegion, Control::RadiatorRangeCut);

  /*-------------------------------------------------------------------
    GEAR information
    (see http://www.hep.phy.cam.ac.uk/~drw1/Takuma_Thesis.pdf, pages 91/92)
  */
//TODO gear issues//////???  G4double innerRadius = 0;
//  G4double outerRadius = HalfEcalZ; // /*I know, Z instead of Y*/
//  G4double leastZ      = EcalPosition(2) - HalfEcalY;/*I know, Z instead of Y*/
//  G4int symmetryOrder  = 4;      /*this is a standalone prototype*/
// G4double phi         = 0;
//  gear::CalorimeterParametersImpl *gearParam = 
//    new gear::CalorimeterParametersImpl(innerRadius, outerRadius, leastZ, symmetryOrder, phi);

  /*
  for (unsigned int i_group = 0; i_group < PlateGroups.size(); i_group++)
  {
  G4cout<<" i_group="<<i_group<<" n_layers: "<<PlateGroups[i_group]->n_layers
  <<" w_thickness="<<PlateGroups[i_group]->nominal_w_thickness<<G4endl;
  }
  */
//TODO gear //??  const G4int n_layers = 30;
//  G4double layerThickness = 0;
//  for (G4int iLayer = 0; iLayer < n_layers; ++iLayer)
//    {
//      G4double distance = 0; /*distance of this layer from the origin*/
      
//      if (iLayer % 2 == 0)
//       layerThickness = 2 * deadw_fiber_thickness 
//         + PlateGroups[iLayer/10]->nominal_w_thickness 
//         + al_cf_y_gap 
//         + al_thickness  //aluminum
//         + al_g10_gap //air 
//         + g10_thickness //PCB
//         + HalfWafferY * 2 //silicon thickness
//         + exit_fiber_thickness;
//      else
//       layerThickness = PlateGroups[iLayer/10]->nominal_w_thickness  
//         + HalfWafferY * 2 //silicon thickness
//         + g10_thickness //PCB
//         + al_g10_gap //air 
//         +  al_thickness  //aluminum
//         + inter_structures_gap;
//
//      G4double cellSize0 = cell_dim_z; //10; /*cell size along the beam axis*/
//      G4double cellSize1 = cell_dim_x; //10; /*cell size along the axis perpendicular to the beam axis, in mm*/
//      G4double absorberThickness =  PlateGroups[iLayer/10]->nominal_w_thickness;
//      
//      gearParam->layerLayout().positionLayer(distance, layerThickness, cellSize0, cellSize1, absorberThickness);
//      
//    }/*end loop over iLayer*/
  /* write parameters to GearManager*/
//  MokkaGear* gearMgr = MokkaGear::getMgr() ;
//  gearMgr->setEcalEndcapParameters( gearParam ) ;  
  /*-------------------------------------------------------------------*/


  // Closes Database connection
  delete db;
  db = 0;
  G4cout << "Proto done.\n" << G4endl;
  return true;
}

void TBscecal01::BuildStructures(void) {
// I do not use this so far,.... after I learn from Proto05 ...
}

void TBscecal01::DefineMaterial(void) {

// materials registered here:
//  *w, *reffront, *sc, *refrear, *mixgap, *g10, *air;  
	
//  G4double density;
  G4String name, symbol;
//  G4int nel;
  G4double fractionmass;
//  G4double  volumefraction;

  // materials 
  // WC + Co + Cr, WC is a metal cristal of 1:1 of W and C,
  // W:C:Co:Cr = 0.8163:0.0553:0.1249:0.0045 (wt ratio) 
//                                   name,  symbol,   z,  density 
  G4Element* elW  = new G4Element("Tungsten","elW",  74, 183.850*g/mole);
  G4Element* elC  = new G4Element("Carbon",  "elC",   6,  12.011*g/mole);
  G4Element* elCo = new G4Element("Cobalt",  "elCo", 27,  58.933*g/mole);
  G4Element* elCr = new G4Element("Chromium","elCr", 24,  51.996*g/mole);
  G4Element* elH  = new G4Element("Hydrogen","elH",   1,   1.008*g/mole);
  G4Element* elN  = new G4Element("Nigrogen","elN",   7,  14.007*g/mole);
  G4Element* elO  = new G4Element("Oxygen",  "elO",   8,  15.999*g/mole);
  G4Element* elCl = new G4Element("Chlorine","elCl", 17,  35.453*g/mole);
  G4Element* elAr = new G4Element("Argone",  "elAr", 18,  39.948*g/mole);

  // materials
  //111130.1900                               |the number of elements  
  //density = 14.6666*g/cm3;                  V
  G4Material * myW = new G4Material("W_C_Co_Cr", absorberDens*g/cm3, 4); //default = 14.7
  //                |mass fracrtion
  //                V
  myW->AddElement(elW,  massFraction_W ); // default = 0.8163
  myW->AddElement(elC,  massFraction_C ); // default = 0.0553
  myW->AddElement(elCo, massFraction_Co); // default = 0.1249
  myW->AddElement(elCr, massFraction_Cr); // default = 0.0045

// from PDG (2006) as Mylar 1.39
  G4Material * myPET = new G4Material("C10_H8_O4", 1.39*g/cm3, 3);
  // TotMol = 12.011x10 + 1.008x8 + 15.999x4
  myPET->AddElement(elC,  0.625); // 12.011x10/TotMol
  myPET->AddElement(elH,  0.042); // 1.008x8/TotMol
  myPET->AddElement(elO,  0.333); // 15.999x4/TotMol

//TODO density of Kapton? <-- 1.42 from PDG(TODO Check latest)) 
  G4Material * myKapton = new G4Material("Kapton", 1.42*g/cm3, 4);
  myKapton->AddElement(elH, fractionmass = 0.0273);
  myKapton->AddElement(elC, fractionmass = 0.7213);
  myKapton->AddElement(elN, fractionmass = 0.0765);
  myKapton->AddElement(elO, fractionmass = 0.1749);

//TODO dehsity and material of clear fiber?
  G4Material * myAcryl  = new G4Material("C5_H8_O2", 1.19*g/cm3, 3);
  myAcryl->AddElement(elC, fractionmass = 0.5998);//12.011x5/Total
  myAcryl->AddElement(elH, fractionmass = 0.0805);//1.008x8/Total
  myAcryl->AddElement(elO, fractionmass = 0.3196);//15.999x2/Total

//TODO density and material of black sheet and vinyl tape
  G4Material * myPVC    = new G4Material("C2_H3_CL", 1.44*g/cm3, 3);
  myPVC->AddElement(elC,   fractionmass = 0.38436); //12.011x2/Total
  myPVC->AddElement(elH,   fractionmass = 0.04839); //1.008x3/Total
  myPVC->AddElement(elCl,  fractionmass = 0.56726); //35.453x1/Total

  G4Material * Air25 = new G4Material("air at 25 D ofC, 1 atm", 0.00118*g/cm3,3);
  Air25->AddElement(elN,  fractionmass = 0.7555);
  Air25->AddElement(elO,  fractionmass = 0.2320);
  Air25->AddElement(elAr, fractionmass = 0.0124);

//TODO Expression of lateral directions?
  G4Material * Cable_etc = new G4Material("Cable etc", cable_etcDens * g/cm3, 6); //default 0.8293
  Cable_etc->AddMaterial(myKapton, 0.2826); //Flat cable
  Cable_etc->AddMaterial(myPVC, 0.1658);    //Black sheet
  g10 = CGAGeometryManager::GetMaterial("g10");
  Cable_etc->AddMaterial(g10, 0.3797);      //G10
  Cable_etc->AddMaterial(myAcryl, 0.0063);  //Clear fiber
  Cable_etc->AddMaterial(myPVC, 0.1649);    //Black tape
  Cable_etc->AddMaterial(Air25, 0.0007);     //empty or void

  w        = myW;
  reffront = myPET; 
  sc =      CGAGeometryManager::GetMaterial("polystyrene"); 
//When We follow PDG, density of scintillator is 1.032*g/cm3, 
//I do not know if "polystyren", here has the same density or not.
  refrear  = myPET;
  mixgap   = Cable_etc;
  air      = Air25;

//  w      =  CGAGeometryManager::GetMaterial("myW");
//  reffront =  CGAGeometryManager::GetMaterial("myPET"); 
//  sc =      CGAGeometryManager::GetMaterial("polystyrene"); 
////When We follow PDG, density of scintillator is 1.032*g/cm3, 
////I do not know if "polystyren", here has the same density or not.
//  refrear = CGAGeometryManager::GetMaterial("myPET");
//  mixgap =  CGAGeometryManager::GetMaterial("Cable_etc");
//  air =     CGAGeometryManager::GetMaterial("Air25")

  G4cout << "w(WC-Co-Cr)->GetRadlen() = " << w->GetRadlen() /mm   << " mm\n";
  G4cout << "reffront->GetRadlen() = " << reffront->GetRadlen() /mm   << " mm\n";
  G4cout << "sc(polystyrene)->GetRadlen() = " << sc->GetRadlen() /mm   << " mm\n";
  G4cout << "refrear->GetRadlen() = " << refrear->GetRadlen() /mm   << " mm\n";
  G4cout << "mixgap(Cable_etc)>GetRadlen() = " << mixgap->GetRadlen() /mm   << " mm\n";
  G4cout << "air(Air25)>GetRadlen() = " << air->GetRadlen() /mm   << " mm\n";



}

TBscecal01::~TBscecal01()
{
//   if(theProtoSD!=0) delete theProtoSD;
}

void TBscecal01::BuildEcal()
{

  // Detector
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

  for ( G4int i = 1; i <= n_layers; i++ )
  {
    BuildLayer(DetectorLogical, i);
  }

}

void TBscecal01::BuildLayer(G4LogicalVolume *DetLog, G4int nlay)
{

  G4cout << "Building Ecal layer: " << nlay << G4endl;

  G4double lay_y = cal_hy - layer_hthickness;

  if ( nlay > 1)
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

  new  G4PVPlacement(0,
                     G4ThreeVector(0,sub_y,0),
                     "WPhys",
                     WLogical,
                     WholeLayerPhys,
                     false,
                     0);

  sub_y -= ( w_hthickness + reffront_hthickness );

  new G4PVPlacement(0,
                    G4ThreeVector(0,sub_y,0),
                    "FrontRefPhys",
                    FrontRefLogical,
                    WholeLayerPhys,
                    false,
                    0);

  sub_y -= ( reffront_hthickness + sc_hthickness );

  new G4PVPlacement(0,
                    G4ThreeVector(0,sub_y,0), 
                    "ScPhys",
                    ScLogical,
                    WholeLayerPhys,
                    false,
                    0);

  sub_y -= ( sc_hthickness + refrear_hthickness );

  new G4PVPlacement(0,
                    G4ThreeVector(0,sub_y,0),
                    "RearRefPhys",
                    RearRefLogical,
                    WholeLayerPhys,
                    false,
                    0);

  sub_y -= ( refrear_hthickness + mixgap_hthickness );

  new G4PVPlacement(0,
                    G4ThreeVector(0,sub_y,0),
                    "MixgapPhys",
                    MixgapLogical,
                    WholeLayerPhys,
                    false,
                    0);

  sub_y -= ( mixgap_hthickness + air_hthickness );

  new G4PVPlacement(0,
                    G4ThreeVector(0,sub_y,0),
                    "AirPhys",
                    AirLogical,
                    WholeLayerPhys,
                    false,
                    0);

}

void TBscecal01::BuildElements() 
{
  //---------------------------------
  // including geometry sizes from DB 
  //---------------------------------

// grid size is from steering file

// db was opened in "construct" 
//  db = new Database(aSubDetectorDBName.data());
  
  db->exec("select * from calorimeter_sizes;" );
  db->getTuple();

  lateral_x   = db->fetchDouble("lateral_x"); // 180. ( mm )
  lateral_y   = db->fetchDouble("lateral_y"); // 180. ( mm )
  ncell_xz[0] = (G4int)(lateral_x/ grid_size);
  ncell_xz[1] = (G4int)(lateral_y/ grid_size);


  n_layers = db->fetchInt("n_layers"); // default = 30 

  w_hthickness =        db->fetchDouble("w_thickness")/2; //      3.5   ( mm )
  reffront_hthickness = db->fetchDouble("ref_thickness"); //      0.057
  sc_hthickness =       db->fetchDouble("sc_thickness")/2; //     3.019
  refrear_hthickness =  reffront_hthickness /2;  //
  mixgap_hthickness =   db->fetchDouble("mixgap_thickness")/2; // 0.995
  air_hthickness =      db->fetchDouble("air_thickness")/2;  //   1.238


  //---------------------------------------------
  // Building layer elements
  //---------------------------------------------

  // set Sensitive Detector, including cell LV
  SetSD();

  // ecal totals  CAUTION!! HALF thickness
  layer_hthickness = w_hthickness 
                   + reffront_hthickness 
                   + sc_hthickness 
                   + refrear_hthickness 
                   + mixgap_hthickness 
                   + air_hthickness;
 
  cal_hx = ( ncell_xz[0] * grid_size )/2;
  cal_hy = n_layers * layer_hthickness;
  cal_hz = ( ncell_xz[1] * grid_size )/2;

  G4cerr << "cal_hx: " << cal_hx << G4endl;
  G4cerr << "cal_hy: " << cal_hy << G4endl;
  G4cerr << "cal_hz: " << cal_hz << G4endl;

  // solids & Logical volume

  // Whole layer
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


  // Absorber
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

  //front reflector
  G4Box *FrontRefSolid = new G4Box("FrontRefSolid",
                                  cal_hx,
                                  reffront_hthickness,
                                  cal_hz);
  
  FrontRefLogical = new G4LogicalVolume(FrontRefSolid,
                                        reffront,
                                        "FrontRefLogical",
                                        0,
                                        0,
                                        0);

  //scintillator
  G4Box *ScSolid = new G4Box("ScSolid",
                             cal_hx,
                             sc_hthickness,
                             cal_hz);

  ScLogical = new G4LogicalVolume(ScSolid,
                                  sc,
                                  "ScLogical",
                                  0,
                                  0,
                                  0);

  //Rear reflector
  G4Box *RearRefSolid = new G4Box("RearRefSolid",
                                  cal_hx,
                                  refrear_hthickness,
                                  cal_hz);

  RearRefLogical = new G4LogicalVolume(RearRefSolid,
                                       refrear,
                                       "RearRefLogical",
                                       0,
                                       0,
                                       0);

  //mix gap
  G4Box *MixgapSolid = new G4Box("MixgapSolid",
                                 cal_hx,
                                 mixgap_hthickness,
                                 cal_hz);

  MixgapLogical = new G4LogicalVolume(MixgapSolid,
                                      mixgap,
                                      "MixgapLogical",
                                      0,
                                      0,
                                      0);

  ScLogical->SetSensitiveDetector(ecalSD);

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

void TBscecal01::SetSD()
{
//120228  ecalSD = new TBSD_VCell02("ecalSD",this);
  ecalSD = new TBSDVCellscecal01("ecalSD",this);
  RegisterSensitiveDetector(ecalSD);   
}


void TBscecal01::CalculateSlabShifts(void) {
}

//void TBscecal01::BuildAlveolaEnveloppe(WLAYERS* aPlate, G4int iSlabPair) {
 // See Calice/src/Proto05.cc/hh
  //------------------------------------
  // VisAttributes pour Alevolus et Slab
  //------------------------------------
  //-----------------------------------
  // Logical type Alveolus
  //-----------------------------------
 
//}


void  
TBscecal01::BeginOfEventAction(const G4Event*)
{
  Dumped=false;
}

void 
TBscecal01::EndOfEventAction(const G4Event* evt)
{
  if(!Dumped) 
    VSubDetectorDriver::EndOfEventAction(evt);
  Dumped = true;
}
