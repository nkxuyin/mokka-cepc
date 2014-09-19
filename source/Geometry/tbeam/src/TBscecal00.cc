/* For ScECAL 2nd prototype tested at FNAL 2008 and 2009,
   this class was made 30th, Nov, 2011 by K.Kotera
   For 00 version
   - not implemented with MPPC, reflector for lateral separetio, and strip 
     shape.
   - uniform in lateral direction and implement materials in 
     longitudinal ( beam ) direction.
   - a set of layer has:
     1. Tungsten absorber 3.5 mm thick, as a chemical compound of:
        W:C:Co:Cr = 0.8163:0.0553:0.1249:0.0045 (wt ratio) and 
        density is 14.7 g/cm^3,
     2. Two 0.057 mm thickness of reflector film ( total 0.114 mm ) has:
        Polyethylenetelephtarate ( 0.050 mm -> but ignore other materials)
        has a density of 1.29 - 1.40 g/cm^3 from the japan plastics industry 
        federation (JPIF)  http://www.jpif.gr.jp/00plastics/conts/pet_c.htm,          
        we take 1.35 g/cm^3,--> we take 1.39 from PDG 2006 as Mylar sheet
	chemical fomura is (C_10 H_8 O_4)
     3. Scintillator layer with thickness of 3.019 mm,
        segmented in 5 mm x 5 mm,
        made from Polystyrene, TODO we need confirm if plystyltolene or not 
        from ???? Chemical Co. through KNU,
     4. Again 0.0057 mm thick reflector film is the same as (2.), but 
        in this case single layer,
     5. Mixture of flat cables of polyimide, black sheet, G10, clear fiber,
         flat cable,
            volume  : 0.20 mm x (178 + 88) mm x 25.0 mm) x 4,
            material: polyimide
            density : ?
         black sheet,
            volume  : 0.095 mm x 180 mm x 180 mm
                      + 0.18 mm x 19 mm x 180 mm x 4,
              black sheet has holes for LED light, but this roughly 
              compensetes with tapes around black-sheet
            material: polyvinyl chloride,
            density : 1.30-1.58 from JPIF --> 1.44 g/cm^3,
         G10,
            volume  : 0.5 mm x 180 mm x 15.0 mm x 4,
            material: G10,
            density : 1.88 g/cm^3,
         Clear fibers,
	    volume  : 0.5 mm x 0.5 mm x pi x 180 mm x 4
            material: polymethyl methacrylate,
            density : 1.19 g/cm3 TODO

     6. Thickness of air gap is   mm.	    
*/

#define HARD_WIRED_TEST 1

#include "Control.hh"
//#include "TBecal02.hh"
#include "TBscecal00.hh"
#include "CGADefs.h"
#include "MySQLWrapper.hh"
#include "CGAGeometryManager.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VisAttributes.hh"

INSTANTIATE(TBscecal00)
TBscecal00::~TBscecal00()
{}

G4bool TBscecal00::construct(const G4String &aSubDetectorDBName,
                           G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding TBscecal00..." << G4endl;
  db = new Database(aSubDetectorDBName.data());

  WorldLogical = WorldLog;

  FetchAll();
  BuildElements();
  Print();

  G4bool cokay = BuildEcal();

  delete db;
  db = 0;

  G4cout << "\nDone building TBscecal00" << G4endl;
  return cokay;
}

void TBscecal00::FetchAll() 
{
  db->exec("select * from ecal_virt;");
  db->getTuple();

#if HARD_WIRED_TEST

  n_layers = 30;
  assert(n_layers>0);

  y_place = 715.6;


  ncell_xz[0] = (G4int)(180./ grid_size);
  ncell_xz[1] = (G4int)(180./ grid_size);
  assert(ncell_xz[0]>0 && ncell_xz[1]>0);

  grid_size= 1.0;
//  y_place=db->fetchDouble("y_place"); 

  db->exec("select * from layer_thickness;");
  db->getTuple();

  w_hthickness =        3.50 / 2;
  reffront_hthickness = 0.057;
  sc_hthickness =       3.019 / 2;
  refrear_hthickness =  0.057 / 2;
  mixgap_hthickness =   0.995 / 2;
  air_hthickness =      1.238 / 2;

#else 

  n_layers = db->fetchInt("n_layers");
  assert(n_layers>0);

  n_layers = db->fetchInt("n_layers");
  y_place = db->fetchDouble("y_place");

  ncell_xz[0] = db->fetchInt("ncell_x");
  ncell_xz[1] = db->fetchInt("ncell_z");
  assert(ncell_xz[0]>0 && ncell_xz[1]>0);

  grid_size=db->fetchDouble("grid_size");
//  y_place=db->fetchDouble("y_place"); 

  db->exec("select * from layer_thickness;");
  db->getTuple();

  w_hthickness =        db->fetchDouble("w_thickness")/2;
  reffront_hthickness = db->fetchDouble("ref_thickness");
  sc_hthickness =       db->fetchDouble("sc_thickness")/2;
  refrear_hthickness =  db->fetchDouble("ref_thickness")/2;
  mixgap_hthickness =   db->fetchDouble("mixgap_thickness")/2;
  air_hthickness =      db->fetchDouble("air_thickness")/2;

#endif

}

void TBscecal00::BuildElements()
{
  // set Sensitive Detector, including cell LV
  SetSD();

  // ecal totals
  layer_hthickness = w_hthickness + reffront_hthickness + sc_hthickness + refrear_hthickness + mixgap_hthickness + air_hthickness;
 
  cal_hx = ( ncell_xz[0] * grid_size )/2;
  cal_hy = n_layers * layer_hthickness;
  cal_hz = ( ncell_xz[1] * grid_size )/2;

  G4cerr << "cal_hx: " << cal_hx << G4endl;
  G4cerr << "cal_hy: " << cal_hy << G4endl;
  G4cerr << "cal_hz: " << cal_hz << G4endl;

  G4double fractionmass; //This is needed..

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
  G4Material * myW = new G4Material("W_C_Co_Cr", 14.7*g/cm3, 4);
  //                |mass fracrtion
  //                V
  myW->AddElement(elW,  0.8163);
  myW->AddElement(elC,  0.0553);
  myW->AddElement(elCo, 0.1249);
  myW->AddElement(elCr, 0.0045);

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
  myPVC->AddElement(elC,   fractionmass = 0.3844); //12.011x2/Total
  myPVC->AddElement(elH,   fractionmass = 0.0048); //1.008x3/Total
  myPVC->AddElement(elCl,  fractionmass = 0.5673); //35.453x1/Total

  G4Material * Air25 = new G4Material("air at 25 D ofC, 1 atm", 0.00118*g/cm3,3);
  Air25->AddElement(elN,  fractionmass = 0.7555);
  Air25->AddElement(elO,  fractionmass = 0.2320);
  Air25->AddElement(elAr, fractionmass = 0.0124);

//TODO Expression of lateral directions?
  G4Material * Cable_etc = new G4Material("Cable etc", 0.8293*g/cm3, 6);
  Cable_etc->AddMaterial(myKapton, 0.2826); //Flat cable
  Cable_etc->AddMaterial(myPVC, 0.1658);    //Black sheet
  g10 = CGAGeometryManager::GetMaterial("g10");
  Cable_etc->AddMaterial(g10, 0.3797);      //G10
  Cable_etc->AddMaterial(myAcryl, 0.0063);  //Clear fiber
  Cable_etc->AddMaterial(myPVC, 0.1649);    //Black tape
  Cable_etc->AddMaterial(Air25, 0.00007);     //empty or void

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


  // solids & Logical volume

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

void TBscecal00::BuildLayer(G4LogicalVolume *DetLog, G4int nlay)
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

G4bool TBscecal00::BuildEcal()
{

  new G4PVPlacement(0,
                    G4ThreeVector(0,y_place,0),
                    DetectorLogical,
                    "EcalDetectorPhys",
                    WorldLogical,
                    0,
                    0);

  for ( G4int i = 1; i <= n_layers; i++ )
  {
    BuildLayer(DetectorLogical, i);
  }
 
  return true;
}

void TBscecal00::SetSD()
{
  ecalSD = new TBSD_VCell02("ecalSD",this);
  RegisterSensitiveDetector(ecalSD);   
}

void TBscecal00::Print()
{
  G4cout << "\nTBscecal00 information: " << G4endl
         << "n_layers: " << n_layers << G4endl 
         << "y_place: " << y_place << G4endl
         << "w_hthickness: " << w_hthickness << G4endl
         << "reffront_hthickness: " << reffront_hthickness << G4endl
         << "sc_hthickness: " << sc_hthickness << G4endl
         << "refrear_hthickness: " << refrear_hthickness << G4endl
         << "mixgap_hthickness: " << mixgap_hthickness << G4endl
         << "air_hthickness: " << air_hthickness << G4endl  
         << G4endl;

}
