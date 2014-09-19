// $Header: /home/flc/cvs/Mokka/source/Geometry/tbeam/src/TBcatcher05.cc,v 1.3 2007/03/05 23:40:13 guilherme Exp $

#include "Control.hh"
#include "TBcatcher05.hh"
#include "TBCellReplication.hh"

#include "CGADefs.h"
#include "MySQLWrapper.hh"
#include "CGAGeometryManager.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"

INSTANTIATE(TBcatcher05)

TBcatcher05::~TBcatcher05()
{}

TBcatcher05::TBcatcher05() 
  : VSubDetectorDriver("TBcatcher05","TBcatcher"),
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

  cal_hx = cal_hy = cal_hz = 0;
  
  // PV name prepend
  pre_name = G4String("pv_");
  
  // depth to layer 
  SetDepthToLayer(2);
}

void TBcatcher05::DefineMaterials()
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

  G4cout << "materials..." << G4endl;
  G4cout << *poly << G4endl;
  G4cout << *steel << G4endl;
  G4cout << *tyvek << G4endl;
  G4cout << *air << G4endl;
}

G4bool TBcatcher05::ContextualConstruct(const
    CGAGeometryEnvironment &aGeometryEnvironment,
    G4LogicalVolume *WorldLog)
{
  G4cout << G4endl << "Building TBcatcher05..." << G4endl;

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

  G4cout << "\nDone building TBcatcher05" << G4endl;
  
  return cokay;
}

void TBcatcher05::FetchAll()
{
  // get layer pattern from DB
  _layerPattern = _aGeometryEnvironment.GetParameterAsString("Tcmt_layer_pattern");
  G4cout<<" TCMT layer pattern: "<< _layerPattern << G4endl;

  // config angle from environment object
  config_angle = _aGeometryEnvironment.GetParameterAsDouble ("configuration_angle");
  G4cout << "config_angle <" << config_angle << ">" << G4endl;
  config_angle = config_angle*deg;

  db->exec("select * from catcher_virt;");
  db->getTuple();

  n_layers = db->fetchInt("n_layers");
  n_fine_layers = db->fetchInt("n_fine_layers");
  assert(n_fine_layers <= n_layers);
  n_coarse_layers = n_layers - n_fine_layers;

  // gap width between Hcal and Catcher
  z_gap = db->fetchDouble("z_gap");
  
  grid_size=db->fetchDouble("grid_size");

  ncell_xy[0]=db->fetchInt("ncell_x");
  ncell_xy[1]=db->fetchInt("ncell_y");

  db->exec("select * from catcher_layer_thickness;");
  db->getTuple();

  // new TC values & derived quantities

//   air_front_hthickness = 17.25/2;
//   air_back_hthickness =  5.0/2;
  air_front_hthickness             = db->fetchDouble("air_front_thickness")/2;
  air_back_hthickness              = db->fetchDouble("air_back_thickness")/2;

  steel_front_hthickness           = db->fetchDouble("steel_front_thickness")/2;
  poly_front_hthickness            = db->fetchDouble("poly_front_thickness")/2;;
  tyvek_front_hthickness           = db->fetchDouble("tyvek_front_thickness")/2;;
  poly_active_hthickness           = db->fetchDouble("poly_active_thickness")/2;;
  tyvek_back_hthickness            = db->fetchDouble("tyvek_back_thickness")/2;;
  poly_back_hthickness             = db->fetchDouble("poly_back_thickness")/2;;
  steel_back_hthickness            = db->fetchDouble("steel_back_thickness")/2;;
  steel_absorber_fine_hthickness   = db->fetchDouble("steel_absorber_fine_thickness")/2;;
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
  G4cout << G4endl;

  cal_hz = n_fine_layers * fine_layer_hthickness +
    n_coarse_layers * coarse_layer_hthickness;

  G4cout << "cal_hz <" << 2*cal_hz << ">" << G4endl;
  G4cout << "n_fine_layers <" << n_fine_layers << ">" << G4endl;
  //

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

}

void TBcatcher05::SetDepthToLayer(G4int i) {
  depthToLayer = i;
#ifdef TBSD_DEBUG
  G4cout <<"DepthToLayer in Catcher: " << depthToLayer << G4endl;
#endif
}

G4bool TBcatcher05::BuildCatcher()
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

void TBcatcher05::SetSD()
{
  // create SD
  catcherSD = new TBSD_VCell03("catcherSD",
			       GetGridSize(),
			       ncell_xy[0],
			       ncell_xy[1],
			       GetDepthToLayer(),
			       TBCATCHER);  

  // set active layer
  PolyActiveLogical->SetSensitiveDetector(catcherSD);

  // register
  RegisterSensitiveDetector(catcherSD);
}


G4LogicalVolume* TBcatcher05::BuildCoarseLayerLogical(bool withCassette)
{
  G4LogicalVolume *thisLogical = 0;

  if( (withCassette==true && CoarseLayerLogical==0)
      || (withCassette==false && CoarseLayerLogicalWithoutCassette==0) ) {

    G4Box *CoarseAbsorberBox = new G4Box("CoarseAbsorberBox", cal_hx, cal_hy, steel_absorber_coarse_hthickness);
    G4Box *CoarseLayerBox    = new G4Box("CoarseLayerBox", 1.01*cal_hx, 1.01*cal_hy, coarse_layer_hthickness);

    CoarseAbsorberLogical = new G4LogicalVolume(CoarseAbsorberBox, steel, "CoarseAbsorber");
    thisLogical = new G4LogicalVolume(CoarseLayerBox, air, "CoarseLayer");


    G4ThreeVector vplace(0, 0, -coarse_layer_hthickness + 2*air_front_hthickness + readout_module_hthickness);

    // check whether cassette should be there or not
    if(withCassette) {
      G4LogicalVolume *ro_lv = BuildReadoutModuleLogical();

      new G4PVPlacement(0,
			vplace,
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

    if( withCassette==true ) CoarseLayerLogical = thisLogical;
    else CoarseLayerLogicalWithoutCassette = thisLogical;
  }

  if( withCassette==true ) return CoarseLayerLogical;
  else return CoarseLayerLogicalWithoutCassette;
}

G4LogicalVolume* TBcatcher05::BuildFineLayerLogical(bool withCassette)
{
  G4LogicalVolume *thisLogical = 0;

  if( (withCassette==true && FineLayerLogical==0)
      || (withCassette==false && FineLayerLogicalWithoutCassette==0) ) {

    G4Box *FineAbsorberBox = new G4Box("FineAbsorberBox", cal_hx, cal_hy, steel_absorber_fine_hthickness);
    G4Box *FineLayerBox    = new G4Box("FineLayerBox", 1.01*cal_hx, 1.01*cal_hy, fine_layer_hthickness);

    FineAbsorberLogical = new G4LogicalVolume(FineAbsorberBox, steel, "FineAbsorber");
    thisLogical    = new G4LogicalVolume(FineLayerBox, air, "FineLayer");


    G4ThreeVector vplace(0., 0., -fine_layer_hthickness + 2*air_front_hthickness + readout_module_hthickness);

    // check whether cassette should be there or not
    if(withCassette) {
      G4LogicalVolume *ro_lv = BuildReadoutModuleLogical();

      new G4PVPlacement(0,
			vplace,
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

    if( withCassette==true ) FineLayerLogical = thisLogical;
    else FineLayerLogicalWithoutCassette = thisLogical;
  }

  if( withCassette==true ) return FineLayerLogical;
  else return FineLayerLogicalWithoutCassette;
}

G4LogicalVolume* TBcatcher05::BuildReadoutModuleLogical()
{
  if (ReadoutModuleLogical == 0) {

    G4Box *SteelFrontBox    = new G4Box("SteelFrontBox", cal_hx, cal_hy, steel_front_hthickness);
    G4Box *PolyFrontBox     = new G4Box("PolyFrontBox", cal_hx, cal_hy, poly_front_hthickness);
    G4Box *TyvekFrontBox    = new G4Box("TyvekFrontBox", cal_hx, cal_hy, tyvek_front_hthickness);
    G4Box *PolyActiveBox    = new G4Box("PolyActiveBox", cal_hx, cal_hy, poly_active_hthickness);
    G4Box *TyvekBackBox     = new G4Box("TyvekBackBox", cal_hx, cal_hy, tyvek_back_hthickness);
    G4Box *PolyBackBox      = new G4Box("PolyBackBox", cal_hx, cal_hy, poly_back_hthickness);
    G4Box *SteelBackBox     = new G4Box("SteelBackBox", cal_hx, cal_hy, steel_back_hthickness);
    G4Box *ReadoutModuleBox = new G4Box("ReadoutModuleBox", cal_hx, cal_hy, readout_module_hthickness);

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

G4LogicalVolume* TBcatcher05::BuildCatcherEnvelope()
{
  if (DetectorLogical == 0) {

    ComputeCalXY();

    G4Box *CatcherBox = new G4Box("CatcherBox", 1.01*cal_hx, 1.01*cal_hy, cal_hz);

    DetectorLogical = new G4LogicalVolume(CatcherBox, steel, "Catcher");
  }

  return DetectorLogical;
}

void TBcatcher05::BuildCatcherLayers()
{
  G4LogicalVolume *catcher = BuildCatcherEnvelope();

  G4LogicalVolume *thisLayer = 0;
  G4ThreeVector vplace(0., 0., -cal_hz);
  G4double hthickness = 0;

  for (G4int i = 1; i <= n_layers; i++) {

    G4cout << "placing catcher layer <" << i << ">" << G4endl;
    bool withCassette = (_layerPattern.data()[i-1] == '1');

    if (i <= n_fine_layers) {
      G4cout << "fine layer";
      if( !withCassette ) G4cout << " without cassette";
      G4cout << G4endl;
      thisLayer = BuildFineLayerLogical(withCassette);
      hthickness = fine_layer_hthickness;
    }
    else {
      G4cout << "coarse layer";
      if( !withCassette ) G4cout << " without cassette";
      G4cout << G4endl;
      thisLayer = BuildCoarseLayerLogical(withCassette);
      hthickness = coarse_layer_hthickness;
    }

    vplace.setZ(vplace.z() + hthickness);

    G4cout << "position " << vplace << G4endl;

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

void TBcatcher05::ComputeCalXY()
{
  cal_hx = (ncell_xy[0] * grid_size)/2;
  cal_hy = (ncell_xy[1] * grid_size)/2;
}

void TBcatcher05::ComputeCatcherTransform()
{
  G4cout << G4endl << "TBcatcher05::ComputeCatcherTransform()" << G4endl;
  
  // hcal dims needed to align the catcher w.r.t the Hcal
  cal_hx_hcal = (G4double) (ncell_xy_hcal[0] * grid_size_hcal*mm)/2.;
  cal_hy_hcal = (G4double) (ncell_xy_hcal[1] * grid_size_hcal*mm)/2.;

  G4cout << "cal_hx_hcal <" << cal_hx_hcal << ">" << G4endl;
  G4cout << "cal_hy_hcal <" << cal_hy_hcal << ">" << G4endl;
  
  layer_hthickness_hcal =  
    poly_hthickness_hcal +
    steel_hthickness_hcal + 
    2.0*airgap_hthickness_hcal +
    2.0*steel_cassette_hthickness_hcal +
    2.0*foil_hthickness_hcal +
    pcb_hthickness_hcal + 
    cablefibre_mix_hthickness_hcal;

  G4cout << "layer_hthickness_hcal <" << layer_hthickness_hcal << ">" << G4endl;

  // z-extension of Hcal
  cal_hz_hcal = (G4double) (n_layers_hcal * layer_hthickness_hcal +
			    steel_hthickness_hcal );

  G4cout << "cal_hz_hcal <" << cal_hz_hcal << ">" << G4endl;

  //calculate the end point of the hcal
  G4double z_end_hcal = z_begin_hcal + 2.*cal_hz_hcal;
  z_begin = z_end_hcal + z_gap;

  G4cout << "z_end_hcal <" << z_end_hcal << ">" << G4endl;
  G4cout << "z_begin <" << z_begin << ">" << G4endl;

  //derive from that the position where to place the Catcher 
  z_place = z_begin + cal_hz;

  G4cout << "z_place <" << z_place << ">" << G4endl;

  G4double main_trajectory = 0.;
  G4double m_gapeh = 0.;
  G4double m_gaphc = 0.;
  G4double m_termlay = 0.;

  if ( cos(config_angle) != 0) {

    //Calculation of translation of the catcher
    //calculate adjacent leg given by the angle (config_angle) and
    //the width of a layer. This results in the absolute distance 
    //which the main trajectory passes through the layer 
    main_trajectory = 2. * layer_hthickness_hcal / cos(config_angle);

    //calculate the main distance the main trajectory passes in air due to a
    //possible gap between ecal and hcal
    m_gapeh = z_begin_hcal/cos(config_angle); 

    //..and a possible gap between hcal and catcher
    m_gaphc = z_gap/cos(config_angle); 

    //calculate the main distance the main trajectory passes throught
    //the terminating layer of the Hcal
    m_termlay = 2.*steel_hthickness_hcal/cos(config_angle);;
  }
  else {
    G4cout << "Bad Configuration Angle: " << config_angle/deg << G4endl;
    G4cout << "ending..." << G4endl;
    exit(1);
  }

  //calculate the distance the main trajectory has travelled in x
  //direction
  //We need x_travel later on to calculate the correct position of the
  //catcher. From geometrical considerations it can be derived that
  //this x_travel has to be calculated for the type of the last layer
  //the main trajectory has to pass before it reaches the catcher.
  //If there is a terminating layer we have to calculated it for such
  //a layer. In general one would have to check what kind of layer
  //is the last one. But ok. we assume that terminating layer tells us
  //something ...

  G4double x_travel = 0.;

  if(n_layer_term_hcal > 0) {
    x_travel = 
      sqrt(m_termlay*m_termlay - 4*steel_hthickness_term_hcal*steel_hthickness_term_hcal);
  } 
  else {
    x_travel = 
      sqrt(main_trajectory*main_trajectory - 4*layer_hthickness_hcal*layer_hthickness_hcal);
  }



  //we're looking into the x-z plane
  //the minimal distance the catcher has to be away from the Hcal is
  //given by the triangle which is defined by 
  // a) the difference between the x-extension of a layer and the distance x_travel as defined above 
  // b) a line which crosses perpendicularly the main trajectory and meets the upper right corner of a layer  
  // c) the minimal distance under consideration
  G4double min_dist = (cal_hx_hcal - x_travel) * abs(sin(config_angle));

  G4cout << "min_dist <" << min_dist << ">" << G4endl;

  //The displacement in z is therefore given by the following
  //expression
  z_place = (m_gapeh + 
	     m_gaphc + 
	     n_layers_hcal * main_trajectory + 
	     n_layer_term_hcal * m_termlay + 
	     min_dist + 
	     cal_hz)
    * cos(config_angle);


  G4cout << "z_place <" << z_place << ">" << G4endl;

  // and analogously the displacement in x
  G4cout << "m_gapeh <" << m_gapeh << ">" << G4endl;
  G4cout << "m_gaphc <" << m_gaphc << ">" << G4endl;
  G4cout << "n_layers_hcal <" << n_layers_hcal << ">" << G4endl;
  G4cout << "n_layer_term_hcal <" << n_layer_term_hcal << ">" << G4endl;
  G4cout << "main_trajectory <" << main_trajectory << ">" << G4endl;
  G4cout << "m_termlay <" << m_termlay << ">" << G4endl;
  G4cout << "min_dist <" << min_dist << ">" << G4endl;
  G4cout << "sin(config_angle) <" << sin(config_angle) << ">" << G4endl;

  x_place = (m_gapeh + 
	     m_gaphc + 
	     n_layers_hcal * main_trajectory + 
	     n_layer_term_hcal * m_termlay + 
	     min_dist + 
	     cal_hx) 
    * sin(config_angle);

  G4cout << "x_place <" << x_place << ">" << G4endl;

  translateCatcher = G4ThreeVector(x_place, 0., z_place); 
  G4RotationMatrix rotateCatcher; 
  rotateCatcher.rotateY(config_angle);
  transformCatcher = new G4Transform3D(rotateCatcher,
				       translateCatcher);
}

// create placement in world vol
void TBcatcher05::PlaceCatcher()
{
  new G4PVPlacement(*transformCatcher,
		    DetectorLogical,
		    "Catcher",
		    WorldLogical,
		    false,
		    0);
}

void TBcatcher05::Print()
{
  G4cout << "TBcatcher info: " << G4endl
	 << "n_layers: " << n_layers << G4endl 
	 << "z_place Catcher: " << z_place << G4endl 
	 << "z_gap: " << z_gap << G4endl 
	 << "cal_hx: " << cal_hx << G4endl
	 << "cal_hy: " << cal_hy << G4endl
	 << "cal_hz: " << cal_hz << G4endl
         << "Configuration Angle: " << config_angle/deg << G4endl
	 << G4endl;
}












