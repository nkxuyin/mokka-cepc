// $Header: /home/flc/cvs/Mokka/source/Geometry/tbeam/src/TBcatcher07.cc,v 1.4 2008/01/23 10:16:56 musat Exp $

#include "Control.hh"
#include "TBcatcher07.hh"
#include "TBCellReplication.hh"

#include "CGADefs.h"
#include "MySQLWrapper.hh"
#include "CGAGeometryManager.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"

#ifdef MOKKA_GEAR
#include "gear/CalorimeterParameters.h"
#include "gearimpl/CalorimeterParametersImpl.h"
#include "gearimpl/LayerLayoutImpl.h"
#include "MokkaGear.h"
#endif


INSTANTIATE(TBcatcher07)

// #define TCMT_DEBUG

TBcatcher07::~TBcatcher07()
{}

TBcatcher07::TBcatcher07()
  : VSubDetectorDriver("TBcatcher07","TBcatcher"),
    db(0),
    _aGeometryEnvironment("","",NULL, NULL),
    config_angle(0), _layerPattern("1111111111111111")
{
  // set all LV ptr = 0
  SteelFrontLogical = PolyFrontLogical = TyvekFrontLogical =
    PolyActiveLogical = TyvekBackLogical = PolyBackLogical =
    SteelBackLogical =  FineAbsorberLogical = CoarseAbsorberLogical =
    ReadoutModuleLogical = DetectorLogical = 0;

  n_layers = n_fine_layers = n_coarse_layers = 0;

  xlen_tcmt = ylen_tcmt = zlen_tcmt = 0;

  // PV name prepend
  pre_name = G4String("pv_");

  // depth to layer
  SetDepthToLayer(2);
}

void TBcatcher07::DefineMaterials()
{
  // materials
  poly = steel = tyvek = air = 0;
  poly = CGAGeometryManager::GetMaterial("polystyrene");
  steel = CGAGeometryManager::GetMaterial("stainless_steel");
  tyvek = CGAGeometryManager::GetMaterial("tyvek");
  air = CGAGeometryManager::GetMaterial("air");

  assert(poly);
  assert(steel);
  assert(tyvek);
  assert(air);

// #ifdef TCMT_DEBUG
//   G4cout << "materials..." << G4endl;
//   G4cout << *poly << G4endl;
//   G4cout << *steel << G4endl;
//   G4cout << *tyvek << G4endl;
//   G4cout << *air << G4endl;
// #endif
}

G4bool TBcatcher07::ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
					G4LogicalVolume *WorldLog)
{
  G4cout << G4endl << "\n ======== Driver TBcatcher07 ========" << G4endl;

  //Obtain the pointer to our database via the Environment object
  _aGeometryEnvironment = aGeometryEnvironment;
  db = new Database(_aGeometryEnvironment.GetDBName());


  WorldLogical = WorldLog;

  // define material
  DefineMaterials();

  // fetch db parms
  FetchAll();

  // do build process
  G4bool cokay = BuildCatcher();

  // set sensitive detector
  SetSD();

  // print info
  Print();

  delete db;
  db = 0;


 /*-------------------------------------------------------------------
    GEAR information
  */
  G4double innerRadius = 0;
  G4double outerRadius = xlen_tcmt ;//* sqrt(2);
  G4double leastZ      = z_begin;
  G4int symmetryOrder  = 4;          /*this is a standalone prototype*/
  G4double phi         = 0;
  gear::CalorimeterParametersImpl *gearParam = 
    new gear::CalorimeterParametersImpl(innerRadius, outerRadius, leastZ, symmetryOrder, phi);

  G4double distance = 0; /*distance of this layer from the origin*/
  G4double layerThickness;
  //G4double cellSize0 = 2.* poly_active_hthickness; /*cell size along the beam axis*/
  G4double cellSize0 = grid_size * mm; /*cell size along the beam axis*/
  G4double cellSize1 = 2. * xlen_tcmt * mm;/*cell size along the axis perpendicular to the beam axis, in mm*/
  G4double absorberThickness = 0;

  for (G4int iLayer = 0; iLayer < n_layers; ++iLayer)
    {
      if (iLayer < n_fine_layers)
        {
          layerThickness = 2. * (air_front_hthickness + readout_module_hthickness + air_back_hthickness + steel_absorber_fine_hthickness);
          absorberThickness = 2. * steel_absorber_fine_hthickness;
        }
      else
        {
          layerThickness = 2. * (air_front_hthickness + readout_module_hthickness + air_back_hthickness + steel_absorber_coarse_hthickness);
          absorberThickness = 2. * steel_absorber_coarse_hthickness;
        }

      /*the TCMT layers are alternating: first layer has vertical stripes, the second layer has horizontal stripes, and so on*/
      if (iLayer % 2 == 0)
        gearParam->layerLayout().positionLayer(distance, layerThickness, cellSize0, cellSize1, absorberThickness);
      else
        gearParam->layerLayout().positionLayer(distance, layerThickness, cellSize1, cellSize0, absorberThickness);


    }/*end loop over iLayer*/

  /* write parameters to GearManager*/
  MokkaGear* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setYokeEndcapParameters( gearParam ) ;
  /*-------------------------------------------------------------------*/

  G4cout << "\n======== Done with driver TBcatcher07 ========" << G4endl;

  return cokay;
}

void TBcatcher07::FetchAll()
{
  // get layer pattern from DB
  _layerPattern = _aGeometryEnvironment.GetParameterAsString("Tcmt_layer_pattern");

  // config angle from environment object
  // config_angle: angle reflecting the rotations for non normal incidence.
  config_angle = _aGeometryEnvironment.GetParameterAsDouble ("configuration_angle") * deg;

  // distance between Hcal and Catcher
  dist_hcal_tcmt = _aGeometryEnvironment.GetParameterAsDouble("dist_hcal_tcmt");

  db->exec("select * from catcher_virt;");
  db->getTuple();

  n_layers = db->fetchInt("n_layers");
  assert(n_layers > 0);
  n_fine_layers = db->fetchInt("n_fine_layers");
  assert(n_fine_layers <= n_layers);
  n_coarse_layers = n_layers - n_fine_layers;

  //number of cells in x
  ncell_xy[0] = db->fetchInt("ncell_x");
  //number of cells in y
  ncell_xy[1] = db->fetchInt("ncell_y");
  assert(ncell_xy[0] >= 0 || ncell_xy[1] >= 0);
  //grid size
  grid_size = db->fetchDouble("grid_size");

  db->exec("select * from catcher_layer_thickness;");
  db->getTuple();

  // new TC values & derived quantities

  air_front_hthickness             = db->fetchDouble("air_front_thickness")/2;
  air_back_hthickness              = db->fetchDouble("air_back_thickness")/2;

  steel_front_hthickness           = db->fetchDouble("steel_front_thickness")/2;
  poly_front_hthickness            = db->fetchDouble("poly_front_thickness")/2;
  tyvek_front_hthickness           = db->fetchDouble("tyvek_front_thickness")/2;
  poly_active_hthickness           = db->fetchDouble("poly_active_thickness")/2;
  tyvek_back_hthickness            = db->fetchDouble("tyvek_back_thickness")/2;
  poly_back_hthickness             = db->fetchDouble("poly_back_thickness")/2;
  steel_back_hthickness            = db->fetchDouble("steel_back_thickness")/2;
  steel_absorber_fine_hthickness   = db->fetchDouble("steel_absorber_fine_thickness")/2;
  steel_absorber_coarse_hthickness = db->fetchDouble("steel_absorber_coarse_thickness")/2;

  readout_module_hthickness =
    steel_front_hthickness +
    poly_front_hthickness +
    tyvek_front_hthickness +
    poly_active_hthickness +
    tyvek_back_hthickness +
    poly_back_hthickness +
    steel_back_hthickness;

  fine_layer_hthickness =
    air_front_hthickness +
    readout_module_hthickness +
    steel_absorber_fine_hthickness +
    air_back_hthickness;

  coarse_layer_hthickness =
    air_front_hthickness +
    readout_module_hthickness +
    steel_absorber_coarse_hthickness +
    air_back_hthickness;

  zlen_tcmt = n_fine_layers * fine_layer_hthickness +
    n_coarse_layers * coarse_layer_hthickness;

#ifdef TCMT_DEBUG
  G4cout<<" TCMT layer pattern: "<< _layerPattern << G4endl;
  G4cout << "config_angle <" << config_angle/deg << ">" << G4endl;
  G4cout << "catcher thicknesses..." << G4endl;
  G4cout << "air_front_hthickness <" << 2*air_front_hthickness << ">" << G4endl;
  G4cout << "steel_front_hthickness <" << 2*steel_front_hthickness << ">" << G4endl;
  G4cout << "poly_front_hthickness <" << 2*poly_front_hthickness << ">" << G4endl;
  G4cout << "tyvek_front_hthickness <" << 2*tyvek_front_hthickness << ">" << G4endl;
  G4cout << "poly_active_hthickness <" << 2*poly_active_hthickness << ">" << G4endl;
  G4cout << "tyvek_back_hthickness <" << 2*tyvek_back_hthickness << ">" << G4endl;
  G4cout << "poly_back_hthickness <" << 2*poly_back_hthickness << ">" << G4endl;
  G4cout << "steel_back_hthickness <" << 2*steel_back_hthickness << ">" << G4endl;
  G4cout << "air_back_hthickness <" << 2*air_back_hthickness << ">" << G4endl;
  G4cout << "steel_absorber_fine_hthickness <" << 2*steel_absorber_fine_hthickness << ">" << G4endl;
  G4cout << "steel_absorber_coarse_hthickness <" << 2*steel_absorber_coarse_hthickness << ">" << G4endl;
  G4cout << "readout_module_hthickness <" << 2*readout_module_hthickness << ">" << G4endl;
  G4cout << "fine_layer_hthickness <" << 2*fine_layer_hthickness << ">" << G4endl;
  G4cout << "coarse_layer_hthickness <" << 2*coarse_layer_hthickness << ">" << G4endl;
  G4cout << "zlen_tcmt <" << 2*zlen_tcmt << ">" << G4endl;
  G4cout << "n_fine_layers <" << n_fine_layers << ">" << G4endl;
  G4cout << G4endl;
#endif

  //get information from the Hcal
  db->exec("select * from hcal_virt;");
  db->getTuple();
  n_layers_hcal = db->fetchInt("n_layers");
  assert(n_layers_hcal > 0);

  //number of cells in x
  ncell_xy_hcal[0] = db->fetchInt("ncell_x");

  //number of cells in y
  ncell_xy_hcal[1] = db->fetchInt("ncell_y");
  assert(ncell_xy[0] >= 0 || ncell_xy[1] >= 0);

  //grid size
  grid_size_hcal = db->fetchDouble("grid_size");

  //the beginning of the Hcal
  z_begin_hcal = db->fetchDouble("z_begin");

  // thicknesses
  db->exec("select * from hcal_layer_thickness;");
  db->getTuple();

  // hcal parameters for placement computation
  poly_hthickness_hcal = db->fetchDouble("poly_thickness")/2;
  steel_hthickness_hcal = db->fetchDouble("steel_thickness")/2;
  airgap_hthickness_hcal = db->fetchDouble("air_gap")/2;
  steel_cassette_hthickness_hcal = db->fetchDouble("steel_cassette_thickness")/2.;
  foil_hthickness_hcal = db->fetchDouble("foil_thickness")/2.;
  pcb_hthickness_hcal = db->fetchDouble("pcb_thickness")/2.;
  cablefibre_mix_hthickness_hcal = db->fetchDouble("cablefibre_mix_thickness")/2.;

  //Fetch data for terminating absorber of Hcal
  db->exec("select * from hcal_termlayer;");
  db->getTuple();

  n_layer_term_hcal = db->fetchInt("n_layer_term");
  steel_hthickness_term_hcal = db->fetchDouble("steel_thickness_term")/2;

  // TCMT adjustments for alignment
  rotationAdjust = _aGeometryEnvironment.GetParameterAsDouble ("TCMTRotationAngle") * deg;
  G4double xTranslation = _aGeometryEnvironment.GetParameterAsDouble("TCMTTranslateX");
  G4double yTranslation = _aGeometryEnvironment.GetParameterAsDouble("TCMTTranslateY");

  translationAdjust = G4ThreeVector(xTranslation, yTranslation, 0.0);
}

void TBcatcher07::SetDepthToLayer(G4int i) {
  depthToLayer = i;
#ifdef TCMT_DEBUG
  G4cout <<"DepthToLayer in Catcher: " << depthToLayer << G4endl;
#endif
}

G4bool TBcatcher07::BuildCatcher()
{
  // build detector envelope
  BuildCatcherEnvelope();

  // build layers
  BuildCatcherLayers();

  // compute translation vection and rotation
  ComputeCatcherTransform();

  // place in WV
  PlaceCatcher();

  return true;
}

void TBcatcher07::SetSD()
{
  /*Birks law and time cut:
    default values: Tcmt_apply_Birks_law = 1, Tcmt_time_cut = 150. nsec
  */
  G4int Tcmt_apply_Birks_law  = _aGeometryEnvironment.GetParameterAsInt("Tcmt_apply_Birks_law");
  G4double Tcmt_time_cut      = _aGeometryEnvironment.GetParameterAsDouble("Tcmt_time_cut");

  G4double zBeginTemp = 0;
  if (Tcmt_time_cut > 0) zBeginTemp = z_begin_hcal;
  else zBeginTemp = 0;

  G4cout<<" Tcmt_apply_Birks_law:  "<<Tcmt_apply_Birks_law
	<<"\n Tcmt_time_cut: " <<Tcmt_time_cut <<G4endl;

  // create SD
  catcherSD = new TBSD_VCell03("catcherSD",
			       GetGridSize(),
			       ncell_xy[0],
			       ncell_xy[1],
			       GetDepthToLayer(),
			       TBCATCHER,
			       Tcmt_apply_Birks_law,
			       Tcmt_time_cut,
			       zBeginTemp);

  // set active layer
  PolyActiveLogical->SetSensitiveDetector(catcherSD);

  // register
  RegisterSensitiveDetector(catcherSD);
}


G4LogicalVolume* TBcatcher07::BuildCoarseLayerLogical(bool withCassette, G4ThreeVector& offset)
{
  G4LogicalVolume *thisLogical = 0;

//   if( (withCassette==true && CoarseLayerLogical==0)
//       || (withCassette==false && CoarseLayerLogicalWithoutCassette==0) ) {

  // real dimensions defined in terms of sensitive layer size, which comes from mokka DB
  G4Box *CoarseAbsorberBox = new G4Box("CoarseAbsorberBox", 1.17*xlen_tcmt, 1.17*ylen_tcmt, steel_absorber_coarse_hthickness);
  G4Box *CoarseLayerBox    = new G4Box("CoarseLayerBox", 1.20*xlen_tcmt, 1.20*ylen_tcmt, coarse_layer_hthickness);

  CoarseAbsorberLogical = new G4LogicalVolume(CoarseAbsorberBox, steel, "CoarseAbsorber");
  thisLogical = new G4LogicalVolume(CoarseLayerBox, air, "CoarseLayer");


  G4ThreeVector vplace(0, 0, -coarse_layer_hthickness + 2*air_front_hthickness + readout_module_hthickness);

  // check whether cassette should be there or not
  if(withCassette) {
    G4LogicalVolume *ro_lv = BuildReadoutModuleLogical();

    //       G4cout<<"vplace+offset="<< vplace <<" + "<< offset <<" = "<< vplace+offset << G4endl;
    new G4PVPlacement(0,
		      vplace+offset,
		      ro_lv,
		      pre_name + ro_lv->GetName(),
		      thisLogical,
		      false,
		      0);
  }

  vplace.setZ(vplace.z() + readout_module_hthickness + 2*air_back_hthickness + steel_absorber_coarse_hthickness);

  new G4PVPlacement(0,
		    vplace,
		    CoarseAbsorberLogical,
		    pre_name + CoarseAbsorberLogical->GetName(),
		    thisLogical,
		    false,
		    0);

  assert(vplace.z() < abs(coarse_layer_hthickness));

  //     if( withCassette==true ) CoarseLayerLogical = thisLogical;
  //     else CoarseLayerLogicalWithoutCassette = thisLogical;
  //   }

  //   if( withCassette==true ) return CoarseLayerLogical;
  //   else return CoarseLayerLogicalWithoutCassette;
  return thisLogical;
}

G4LogicalVolume* TBcatcher07::BuildFineLayerLogical(bool withCassette, G4ThreeVector& offset)
{
  G4LogicalVolume *thisLogical = 0;

  //   if( (withCassette==true && FineLayerLogical==0)
  //       || (withCassette==false && FineLayerLogicalWithoutCassette==0) ) {

  // real dimensions defined in terms of sensitive layer size, which comes from mokka DB
  G4Box *FineAbsorberBox = new G4Box("FineAbsorberBox", 1.17*xlen_tcmt, 1.17*ylen_tcmt, steel_absorber_fine_hthickness);
  G4Box *FineLayerBox    = new G4Box("FineLayerBox", 1.20*xlen_tcmt, 1.20*ylen_tcmt, fine_layer_hthickness);

  FineAbsorberLogical = new G4LogicalVolume(FineAbsorberBox, steel, "FineAbsorber");
  thisLogical    = new G4LogicalVolume(FineLayerBox, air, "FineLayer");


  G4ThreeVector vplace(0., 0., -fine_layer_hthickness + 2*air_front_hthickness + readout_module_hthickness);

  // check whether cassette should be there or not
  if(withCassette) {
    G4LogicalVolume *ro_lv = BuildReadoutModuleLogical();

    //       G4cout<<"vplace+offset="<< vplace <<" + "<< offset <<" = "<< vplace+offset << G4endl;

    new G4PVPlacement(0,
		      vplace+offset,
		      ro_lv,
		      pre_name + ro_lv->GetName(),
		      thisLogical,
		      false,
		      0);
  }

  vplace.setZ(vplace.z() + readout_module_hthickness + 2*air_back_hthickness + steel_absorber_fine_hthickness);

  new G4PVPlacement(0,
		    vplace,
		    FineAbsorberLogical,
		    pre_name + FineAbsorberLogical->GetName(),
		    thisLogical,
		    false,
		    0);

  assert(vplace.z() < abs(fine_layer_hthickness));

//     if( withCassette==true ) FineLayerLogical = thisLogical;
//     else FineLayerLogicalWithoutCassette = thisLogical;
//   }

//   if( withCassette==true ) return FineLayerLogical;
//   else return FineLayerLogicalWithoutCassette;

  return thisLogical;
}

G4LogicalVolume* TBcatcher07::BuildReadoutModuleLogical()
{
  if (ReadoutModuleLogical == 0) {

    G4Box *SteelFrontBox    = new G4Box("SteelFrontBox", xlen_tcmt, ylen_tcmt, steel_front_hthickness);
    G4Box *PolyFrontBox     = new G4Box("PolyFrontBox", xlen_tcmt, ylen_tcmt, poly_front_hthickness);
    G4Box *TyvekFrontBox    = new G4Box("TyvekFrontBox", xlen_tcmt, ylen_tcmt, tyvek_front_hthickness);
    G4Box *PolyActiveBox    = new G4Box("PolyActiveBox", xlen_tcmt, ylen_tcmt, poly_active_hthickness);
    G4Box *TyvekBackBox     = new G4Box("TyvekBackBox", xlen_tcmt, ylen_tcmt, tyvek_back_hthickness);
    G4Box *PolyBackBox      = new G4Box("PolyBackBox", xlen_tcmt, ylen_tcmt, poly_back_hthickness);
    G4Box *SteelBackBox     = new G4Box("SteelBackBox", xlen_tcmt, ylen_tcmt, steel_back_hthickness);
    G4Box *ReadoutModuleBox = new G4Box("ReadoutModuleBox", xlen_tcmt, ylen_tcmt, readout_module_hthickness);

    SteelFrontLogical    = new G4LogicalVolume(SteelFrontBox, steel, "SteelFront");
    PolyFrontLogical     = new G4LogicalVolume(PolyFrontBox, poly, "PolyFront");
    TyvekFrontLogical    = new G4LogicalVolume(TyvekFrontBox, tyvek, "TyvekFront");
    PolyActiveLogical    = new G4LogicalVolume(PolyActiveBox, poly, "PolyActive");
    TyvekBackLogical     = new G4LogicalVolume(TyvekBackBox, tyvek, "TyvekBack");
    PolyBackLogical      = new G4LogicalVolume(PolyBackBox, poly, "PolyBack");
    SteelBackLogical     = new G4LogicalVolume(SteelBackBox, steel, "SteelBack");

    ReadoutModuleLogical = new G4LogicalVolume(ReadoutModuleBox, air, "ReadoutModule");

    G4ThreeVector vplace(0., 0., -readout_module_hthickness + steel_front_hthickness);

    new G4PVPlacement(0,
		      vplace,
		      SteelFrontLogical,
		      pre_name + SteelFrontLogical->GetName(),
		      ReadoutModuleLogical,
		      false,
		      0);

    vplace.setZ(vplace.z() + steel_front_hthickness  + poly_front_hthickness);

    new G4PVPlacement(0,
		      vplace,
		      PolyFrontLogical,
		      pre_name + PolyFrontLogical->GetName(),
		      ReadoutModuleLogical,
		      false,
		      1);

    vplace.setZ(vplace.z() + poly_front_hthickness + tyvek_front_hthickness);

    new G4PVPlacement(0,
		      vplace,
		      TyvekFrontLogical,
		      pre_name + TyvekFrontLogical->GetName(),
		      ReadoutModuleLogical,
		      false,
		      2);

    vplace.setZ(vplace.z() + tyvek_front_hthickness + poly_active_hthickness);

    new G4PVPlacement(0,
		      vplace,
		      PolyActiveLogical,
		      pre_name + PolyActiveLogical->GetName(),
		      ReadoutModuleLogical,
		      false,
		      3);

    vplace.setZ(vplace.z() + poly_active_hthickness + tyvek_back_hthickness);

    new G4PVPlacement(0,
		      vplace,
		      TyvekBackLogical,
		      pre_name + TyvekBackLogical->GetName(),
		      ReadoutModuleLogical,
		      false,
		      4);

    vplace.setZ(vplace.z() + tyvek_back_hthickness + poly_back_hthickness);

    new G4PVPlacement(0,
		      vplace,
		      PolyBackLogical,
		      pre_name + PolyBackLogical->GetName(),
		      ReadoutModuleLogical,
		      false,
		      5);

    vplace.setZ(vplace.z() + poly_back_hthickness + steel_back_hthickness);

    new G4PVPlacement(0,
		      vplace,
		      SteelBackLogical,
		      pre_name + SteelBackLogical->GetName(),
		      ReadoutModuleLogical,
		      false,
		      6);

    assert(vplace.z() < abs(readout_module_hthickness));
  }

  return ReadoutModuleLogical;
}

G4LogicalVolume* TBcatcher07::BuildCatcherEnvelope()
{
  if (DetectorLogical == 0) {

    ComputeCalXY();

    // xlen_tcmt, ylen_tcmt, zlen_tcmt are x,y,z-dimensions of sensitive layer (1m x 1m)
    G4Box *CatcherBox = new G4Box("CatcherBox", 1.20*xlen_tcmt, 1.20*ylen_tcmt, zlen_tcmt);

    DetectorLogical = new G4LogicalVolume(CatcherBox, steel, "Catcher");
  }

  return DetectorLogical;
}

void TBcatcher07::BuildCatcherLayers()
{
  G4LogicalVolume *catcher = BuildCatcherEnvelope();

  G4LogicalVolume *thisLayer = 0;
  G4ThreeVector vplace(0., 0., -zlen_tcmt);
  G4double hthickness = 0;

  // staggering is defined here... hardcoded (at least for now)
  G4ThreeVector stagger[17] = {G4ThreeVector(0,0,0)};
  G4double in = 0.*mm;
  stagger[3].setX( -1*in );
  stagger[4].setY(  1*in );
  stagger[7].setX( -1*in );
  stagger[8].setY(  1*in );
  stagger[11].setX( -1*in );
  stagger[12].setY(  1*in );
  stagger[15].setX( -1*in );
  stagger[16].setY(  1*in );

  for (G4int i = 1; i <= n_layers; i++) {

    //     G4cout << "placing catcher layer <" << i << ">" << G4endl;
    bool withCassette = (_layerPattern.data()[i-1] == '1');

    if (i <= n_fine_layers) {
//       G4cout << "fine layer";
//       if( !withCassette ) G4cout << " without cassette";
//       G4cout << G4endl;
      thisLayer = BuildFineLayerLogical(withCassette, stagger[i]);
      hthickness = fine_layer_hthickness;
    }
    else {
//       G4cout << "coarse layer";
//       if( !withCassette ) G4cout << " without cassette";
//       G4cout << G4endl;
      thisLayer = BuildCoarseLayerLogical(withCassette, stagger[i]);
      hthickness = coarse_layer_hthickness;
    }

    vplace.setZ(vplace.z() + hthickness);

//  G4cout << "position " << vplace <<" + "<< stagger[i] << G4endl;

    new G4PVPlacement(0,
		      vplace,
		      thisLayer,
		      pre_name + thisLayer->GetName(),
		      catcher,
		      false,
		      i);

    vplace.setZ(vplace.z() + hthickness);
  }
}

void TBcatcher07::ComputeCalXY()
{
  xlen_tcmt = (ncell_xy[0] * grid_size)/2;
  ylen_tcmt = (ncell_xy[1] * grid_size)/2;
}

void TBcatcher07::ComputeCatcherTransform()
{
#ifdef TCMT_DEBUG
  G4cout << G4endl << "TBcatcher07::ComputeCatcherTransform()" << G4endl;
#endif
  G4double costheta = cos(config_angle);
  G4double sintheta = sin(config_angle);
  if(costheta<=0) {
    G4cout << "Bad Configuration Angle: " << config_angle/deg << G4endl;
    G4cout << "ending..." << G4endl;
    exit(1);
  }

  // hcal dimensions needed to align the catcher w.r.t the Hcal
  xlen_hcal = (G4double) (ncell_xy_hcal[0] * grid_size_hcal*mm)/2.;
  ylen_hcal = (G4double) (ncell_xy_hcal[1] * grid_size_hcal*mm)/2.;

#ifdef TCMT_DEBUG
  G4cout << "config_angle <" << config_angle/deg << ">" << G4endl;
  G4cout << "xlen_hcal <" << xlen_hcal << ">" << G4endl;
  G4cout << "ylen_hcal <" << ylen_hcal << ">" << G4endl;
#endif

  // half-thickness of hcal layers
  layer_hthickness_hcal =
    poly_hthickness_hcal +
    steel_hthickness_hcal +
    2.0*airgap_hthickness_hcal +
    2.0*steel_cassette_hthickness_hcal +
    2.0*foil_hthickness_hcal +
    pcb_hthickness_hcal +
    cablefibre_mix_hthickness_hcal;

  // half-z-extension of Hcal
  zlen_hcal = (G4double) (n_layers_hcal * layer_hthickness_hcal +
			  steel_hthickness_term_hcal );

  //calculate the end point of the hcal ALONG MAIN TRAJECTORY
  G4double z_end_hcal = z_begin_hcal + 2.*zlen_hcal/costheta;

  // Positioning the catcher, following a TB configuration angle.
  // account for hcal corners approaching the catcher due to config_angle
  z_begin = z_end_hcal + xlen_hcal*sintheta + dist_hcal_tcmt;

  // derive from that the position where to place the Catcher (its center)
  z_place = z_begin + zlen_tcmt;

  z_end_tcmt = z_begin + 2 * zlen_tcmt;

#ifdef TCMT_DEBUG
  G4cout << "n_layers_hcal <" << n_layers_hcal << ">" << G4endl;
  G4cout << "layer_hthickness_hcal <" << layer_hthickness_hcal << ">" << G4endl;
  G4cout << "n_layer_term_hcal <" << n_layer_term_hcal << ">" << G4endl;
  G4cout << "steel_hthickness_term_hcal <" << steel_hthickness_term_hcal << ">" << G4endl;
  G4cout << "zlen_hcal <" << zlen_hcal << ">" << G4endl;
  G4cout << "z_begin_hcal <" << z_begin_hcal << ">" << G4endl;
  G4cout << "z_end_hcal <" << z_end_hcal << ">" << G4endl;
  G4cout << "xlen_hcal*sintheta <" << (xlen_hcal*sintheta) << ">" << G4endl;
  G4cout << "dist_hcal_tcmt <" << dist_hcal_tcmt << ">" << G4endl;
  G4cout << "z_begin <" << z_begin << ">" << G4endl;
  G4cout << "z_place <" << z_place << ">" << G4endl;
#endif

  // Next apply adjustments for alignment correction
  translateCatcher = G4ThreeVector(0, 0, z_place);
  translateCatcher += translationAdjust;

  G4RotationMatrix rotateCatcher;
  rotateCatcher.rotateY(-rotationAdjust);
  transformCatcher = new G4Transform3D(rotateCatcher,
				       translateCatcher);

#ifdef TCMT_DEBUG
  G4cout <<"TranslationAdjust effect: "<< G4ThreeVector(0, 0, z_place)
	 <<" + "<< translationAdjust <<" = "<< translateCatcher << G4endl;
  G4cout <<"Misalignment rotation (around Y-axis): "<< rotationAdjust << G4endl;
#endif
}

// create placement in world vol
void TBcatcher07::PlaceCatcher()
{
  new G4PVPlacement(*transformCatcher,
		    DetectorLogical,
		    "Catcher",
		    WorldLogical,
		    false,
		    0);
}

void TBcatcher07::Print()
{
  G4cout << "\n === Summary of TBcatcher info: ===" << G4endl
         << "Configuration Angle: " << config_angle/deg << G4endl
	 << "n_layers: " << n_layers << G4endl
	 << "xlen_tcmt: " << xlen_tcmt << G4endl
	 << "ylen_tcmt: " << ylen_tcmt << G4endl
	 << "zlen_tcmt: " << zlen_tcmt << G4endl
	 << "dist_hcal_tcmt: " << dist_hcal_tcmt << G4endl
	 << "z_begin_tcmt: " << z_place-zlen_tcmt << G4endl
	 << "z_place Catcher: " << z_place << G4endl
	 << "z_end_tcmt: " << z_place+zlen_tcmt << G4endl;
}

G4bool TBcatcher07::PostConstructAction(CGAGeometryEnvironment& ){

  //makes available the Z of the TCMT exit face for the Muon Counter
  //present after the TCMT

  std::ostringstream oss;
  oss << z_end_tcmt;
  (*Control::globalModelParameters)["z_end_tcmt"] =
    oss.str();

  G4cout << "TCMT information: z_end_tcmt = " << z_end_tcmt << G4endl;

  return true;
}
