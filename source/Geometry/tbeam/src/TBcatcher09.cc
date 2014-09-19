#include "Control.hh"
#include "TBcatcher09.hh"
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

//#define USE_ABSORBER

INSTANTIATE(TBcatcher09)

//#define TCMT_DEBUG

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
TBcatcher09::~TBcatcher09()
{

}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
TBcatcher09::TBcatcher09()
  : VSubDetectorDriver("TBcatcher09","TBcatcher"),
    db(0),
    _aGeometryEnvironment("","",NULL, NULL),
    config_angle(0), 
    _layerPattern("1111111111111111")
{
  checkForOverlappingVolumes = false;

#ifdef TCMT_DEBUG
  checkForOverlappingVolumes = true;
#endif
 

  /* set all logical volumse pointers to 0*/
  SteelFrontLogical = PolyFrontLogical = TyvekFrontLogical =
    PolyActiveLogical = TyvekBackLogical = PolyBackLogical =
    SteelBackLogical =  FineAbsorberLogical = CoarseAbsorberLogical =
    ReadoutModuleLogical = DetectorLogical = NULL;
  
  n_layers = n_fine_layers = n_coarse_layers = 0;

  xlen_tcmt = ylen_tcmt = zlen_tcmt = 0;

  /* PV name prepend*/
  pre_name = G4String("pv_");

  /* depth to layer*/
  SetDepthToLayer(2);
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
void TBcatcher09::DefineMaterials()
{
  /* materials*/
  poly  = steel = tyvek = air = 0;
  poly  = CGAGeometryManager::GetMaterial("polystyrene");
  steel = CGAGeometryManager::GetMaterial("stainless_steel");
  tyvek = CGAGeometryManager::GetMaterial("tyvek");
  air   = CGAGeometryManager::GetMaterial("air");

  assert(poly);
  assert(steel);
  assert(tyvek);
  assert(air);
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
G4bool TBcatcher09::ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
					G4LogicalVolume *WorldLog)
{
  G4cout << G4endl << "\n ======== Driver TBcatcher09 ========" << G4endl;

  /*Obtain the pointer to our database via the Environment object*/
  _aGeometryEnvironment = aGeometryEnvironment;
  db = new Database(_aGeometryEnvironment.GetDBName());

  WorldLogical = WorldLog;

  /* define material*/
  DefineMaterials();

  /* fetch db parms*/
  FetchAll();

  /* do build process*/
  G4bool cokay = BuildCatcher();

  /* set sensitive detector*/
  SetSD();

  /* print info*/
  G4cout<<"\n before print inside"<<G4endl;
  Print();

  delete db;
  db = 0;


 /*-------------------------------------------------------------------
    GEAR information
  */
  G4double innerRadius = 0;
  G4double outerRadius = xlen_tcmt ;
  G4double leastZ      = z_begin;
  G4int symmetryOrder  = 4;          /*this is a standalone prototype*/
  G4double phi         = 0;
  gear::CalorimeterParametersImpl *gearParam = 
    new gear::CalorimeterParametersImpl(innerRadius, outerRadius, leastZ, symmetryOrder, phi);

  G4double distance = 0; /*distance of this layer from the origin*/
  G4double layerThickness;
  G4double cellSize0 = grid_size * mm; /*cell size along the beam axis*/
  G4double cellSize1 = 2. * xlen_tcmt * mm;/*cell size along the axis perpendicular to the beam axis, in mm*/
  G4double absorberThickness = 0;

  for (int iLayer = 0; iLayer < n_layers; ++iLayer)
    {
      if (iLayer < n_fine_layers)
        {
          layerThickness = 2. * (air_front_hthickness[iLayer] + readout_module_hthickness 
				 + air_back_hthickness[iLayer] + steel_absorber_fine_hthickness);
          absorberThickness = 2. * steel_absorber_fine_hthickness;
        }
      else
        {
          layerThickness = 2. * (air_front_hthickness[iLayer] + readout_module_hthickness 
				 + air_back_hthickness[iLayer] + steel_absorber_coarse_hthickness);
          absorberThickness = 2. * steel_absorber_coarse_hthickness;
        }

      /*the TCMT layers are alternating: first layer has vertical stripes, 
	the second layer has horizontal stripes, and so on*/
      if (iLayer % 2 == 0)
        gearParam->layerLayout().positionLayer(distance, layerThickness, cellSize0, cellSize1, absorberThickness);
      else
        gearParam->layerLayout().positionLayer(distance, layerThickness, cellSize1, cellSize0, absorberThickness);


    }/*end loop over iLayer*/

  /* write parameters to GearManager*/
  MokkaGear* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setYokeEndcapParameters( gearParam ) ;
  /*-------------------------------------------------------------------*/

  G4cout << "\n======== Done with driver TBcatcher09 ========" << G4endl;

  return cokay;

}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
void TBcatcher09::FetchAll()
{
  /* get layer pattern from DB*/
  _layerPattern = _aGeometryEnvironment.GetParameterAsString("Tcmt_layer_pattern");

  /* config angle from environment object
     config_angle: angle reflecting the rotations for non normal incidence*/
  config_angle = _aGeometryEnvironment.GetParameterAsDouble ("configuration_angle") * deg;

  /* distance between Hcal and Catcher*/
  dist_hcal_tcmt = _aGeometryEnvironment.GetParameterAsDouble("dist_hcal_tcmt");

  db->exec("select * from catcher_virt;");
  db->getTuple();

  n_layers = db->fetchInt("n_layers");
  assert(n_layers > 0);
  n_fine_layers = db->fetchInt("n_fine_layers");
  assert(n_fine_layers <= n_layers);
  n_coarse_layers = n_layers - n_fine_layers;

  /*number of cells in x*/
  ncell_xy[0] = db->fetchInt("ncell_x");
  /*number of cells in y*/
  ncell_xy[1] = db->fetchInt("ncell_y");
  assert(ncell_xy[0] >= 0 || ncell_xy[1] >= 0);
  /*grid size*/
  grid_size = db->fetchDouble("grid_size");

  /*calculate calorimeter x and y*/
  xlen_tcmt = (ncell_xy[0] * grid_size)/2;
  ylen_tcmt = (ncell_xy[1] * grid_size)/2;

  db->exec("select * from catcher_layer_thickness;");
  db->getTuple();

  /* new TailCatcher values & derived quantities*/
  G4double air_front_hthickness1  = db->fetchDouble("air_front_thickness1")/2;
  G4double air_front_hthickness2  = db->fetchDouble("air_front_thickness2")/2;
  G4double air_front_hthickness3  = db->fetchDouble("air_front_thickness3")/2;
  G4double air_front_hthickness4  = db->fetchDouble("air_front_thickness4")/2;
  G4double air_front_hthickness5  = db->fetchDouble("air_front_thickness5")/2;
  G4double air_front_hthickness6  = db->fetchDouble("air_front_thickness6")/2;
  G4double air_front_hthickness7  = db->fetchDouble("air_front_thickness7")/2;
  G4double air_front_hthickness8  = db->fetchDouble("air_front_thickness8")/2;
  G4double air_front_hthickness9  = db->fetchDouble("air_front_thickness9")/2;
  G4double air_front_hthickness10 = db->fetchDouble("air_front_thickness10")/2;
  G4double air_front_hthickness11 = db->fetchDouble("air_front_thickness11")/2;
  G4double air_front_hthickness12 = db->fetchDouble("air_front_thickness12")/2;
  G4double air_front_hthickness13 = db->fetchDouble("air_front_thickness13")/2;
  G4double air_front_hthickness14 = db->fetchDouble("air_front_thickness14")/2;
  G4double air_front_hthickness15 = db->fetchDouble("air_front_thickness15")/2;
  G4double air_front_hthickness16 = db->fetchDouble("air_front_thickness16")/2;

  air_front_hthickness[0]  = air_front_hthickness1;
  air_front_hthickness[1]  = air_front_hthickness2;
  air_front_hthickness[2]  = air_front_hthickness3;
  air_front_hthickness[3]  = air_front_hthickness4;
  air_front_hthickness[4]  = air_front_hthickness5;
  air_front_hthickness[5]  = air_front_hthickness6;
  air_front_hthickness[6]  = air_front_hthickness7;
  air_front_hthickness[7]  = air_front_hthickness8;
  air_front_hthickness[8]  = air_front_hthickness9;
  air_front_hthickness[9]  = air_front_hthickness10;
  air_front_hthickness[10] = air_front_hthickness11;
  air_front_hthickness[11] = air_front_hthickness12;
  air_front_hthickness[12] = air_front_hthickness13;
  air_front_hthickness[13] = air_front_hthickness14;
  air_front_hthickness[14] = air_front_hthickness15;
  air_front_hthickness[15] = air_front_hthickness16;

  G4double air_back_hthickness1  = db->fetchDouble("air_back_thickness1")/2;
  G4double air_back_hthickness2  = db->fetchDouble("air_back_thickness2")/2;
  G4double air_back_hthickness3  = db->fetchDouble("air_back_thickness3")/2;
  G4double air_back_hthickness4  = db->fetchDouble("air_back_thickness4")/2;
  G4double air_back_hthickness5  = db->fetchDouble("air_back_thickness5")/2;
  G4double air_back_hthickness6  = db->fetchDouble("air_back_thickness6")/2;
  G4double air_back_hthickness7  = db->fetchDouble("air_back_thickness7")/2;
  G4double air_back_hthickness8  = db->fetchDouble("air_back_thickness8")/2;
  G4double air_back_hthickness9  = db->fetchDouble("air_back_thickness9")/2;
  G4double air_back_hthickness10 = db->fetchDouble("air_back_thickness10")/2;
  G4double air_back_hthickness11 = db->fetchDouble("air_back_thickness11")/2;
  G4double air_back_hthickness12 = db->fetchDouble("air_back_thickness12")/2;
  G4double air_back_hthickness13 = db->fetchDouble("air_back_thickness13")/2;
  G4double air_back_hthickness14 = db->fetchDouble("air_back_thickness14")/2;
  G4double air_back_hthickness15 = db->fetchDouble("air_back_thickness15")/2;
  G4double air_back_hthickness16 = db->fetchDouble("air_back_thickness16")/2;

  air_back_hthickness[0]  = air_back_hthickness1;
  air_back_hthickness[1]  = air_back_hthickness2;
  air_back_hthickness[2]  = air_back_hthickness3;
  air_back_hthickness[3]  = air_back_hthickness4;
  air_back_hthickness[4]  = air_back_hthickness5;
  air_back_hthickness[5]  = air_back_hthickness6;
  air_back_hthickness[6]  = air_back_hthickness7;
  air_back_hthickness[7]  = air_back_hthickness8;
  air_back_hthickness[8]  = air_back_hthickness9;
  air_back_hthickness[9]  = air_back_hthickness10;
  air_back_hthickness[10] = air_back_hthickness11;
  air_back_hthickness[11] = air_back_hthickness12;
  air_back_hthickness[12] = air_back_hthickness13;
  air_back_hthickness[13] = air_back_hthickness14;
  air_back_hthickness[14] = air_back_hthickness15;
  air_back_hthickness[15] = air_back_hthickness16;

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

  for (int i = 0; i < n_fine_layers; ++i)
    {
      fine_layer_hthickness[i] =
	air_front_hthickness[i] +
	readout_module_hthickness +
	steel_absorber_fine_hthickness +
	air_back_hthickness[i];
    }

  for (int i = 0; i < n_coarse_layers; ++i)
    {
      coarse_layer_hthickness[i] =
	air_front_hthickness[i] +
	readout_module_hthickness +
	steel_absorber_coarse_hthickness +
	air_back_hthickness[i];
    }

  zlen_tcmt = 0;
  for (int i = 0; i < n_fine_layers; ++i)
    {
      zlen_tcmt += fine_layer_hthickness[i];
    }

  for (int i = 0; i < n_coarse_layers; ++i)
    {
      zlen_tcmt += coarse_layer_hthickness[i];
    }
  
#ifdef TCMT_DEBUG
  G4cout<<"\n TCMT layer pattern: "<< _layerPattern << G4endl;
  G4cout << " config_angle:" << config_angle/deg << G4endl;
  for (int i = 0; i < TCMT_NLAYERS; ++i)
    {
      G4cout<<" \n layer "<<i+1<<G4endl;
      G4cout << " air_front_hthickness    : " << 2*air_front_hthickness[i] << G4endl;
      G4cout << " air_back_hthickness     : " << 2*air_back_hthickness[i] << G4endl;
      if (i < n_fine_layers - 1)
	G4cout << " fine_layer_hthickness   : " << 2*fine_layer_hthickness[i] << G4endl;
      else
	G4cout << " coarse_layer_hthickness : " << 2*coarse_layer_hthickness[i-n_coarse_layers] << G4endl;
    }
  G4cout << "\n steel_front_hthickness: " << 2*steel_front_hthickness << G4endl;
  G4cout << " poly_front_hthickness : " << 2*poly_front_hthickness << G4endl;
  G4cout << " tyvek_front_hthickness: " << 2*tyvek_front_hthickness << G4endl;
  G4cout << " poly_active_hthickness: " << 2*poly_active_hthickness << G4endl;
  G4cout << " tyvek_back_hthickness : " << 2*tyvek_back_hthickness << G4endl;
  G4cout << " poly_back_hthickness  : " << 2*poly_back_hthickness << G4endl;
  G4cout << " steel_back_hthickness : " << 2*steel_back_hthickness << G4endl;
  G4cout << " steel_absorber_fine_hthickness   : " << 2*steel_absorber_fine_hthickness << G4endl;
  G4cout << " steel_absorber_coarse_hthickness : " << 2*steel_absorber_coarse_hthickness << G4endl;
  G4cout << " readout_module_hthickness : " << 2*readout_module_hthickness << G4endl;
  G4cout << " n_fine_layers             : " << n_fine_layers << G4endl;
  G4cout << " n_coarse_layers           : " << n_coarse_layers << G4endl;
  G4cout << G4endl;
  G4cout << " Full TCMT thickness       : " << 2*zlen_tcmt << G4endl;
#endif

  /*get information from the Hcal*/
  db->exec("select * from hcal_virt;");
  db->getTuple();
  n_layers_hcal = db->fetchInt("n_layers");
  assert(n_layers_hcal > 0);

  /*number of cells in x*/
  ncell_xy_hcal[0] = db->fetchInt("ncell_x");

  /*number of cells in y*/
  ncell_xy_hcal[1] = db->fetchInt("ncell_y");
  assert(ncell_xy[0] >= 0 || ncell_xy[1] >= 0);

  /*grid size*/
  grid_size_hcal = db->fetchDouble("grid_size");

  /*the beginning of the Hcal*/
  z_begin_hcal = db->fetchDouble("z_begin");

  /* thicknesses*/
  db->exec("select * from hcal_layer_thickness;");
  db->getTuple();

  /* hcal parameters for placement computation*/
  poly_hthickness_hcal           = db->fetchDouble("poly_thickness")/2;
  absorber_hthickness_hcal          = db->fetchDouble("absorber_thickness")/2;
  airgap_hthickness_hcal         = db->fetchDouble("air_gap")/2;
  steel_cassette_hthickness_hcal = db->fetchDouble("steel_cassette_thickness")/2.;
  foil_hthickness_hcal           = db->fetchDouble("foil_thickness")/2.;
  pcb_hthickness_hcal            = db->fetchDouble("pcb_thickness")/2.;
  cablefibre_mix_hthickness_hcal = db->fetchDouble("cablefibre_mix_thickness")/2.;
  steel_support_hthickness_hcal  = db->fetchDouble("steel_support_thickness")/2.;

  /*Fetch data for terminating absorber of Hcal*/
  db->exec("select * from hcal_termlayer;");
  db->getTuple();

  n_layer_term_hcal = db->fetchInt("n_layer_term");
  absorber_hthickness_term_hcal = db->fetchDouble("absorber_thickness_term")/2;

  /* TCMT adjustments for alignment*/
  rotationAdjust = _aGeometryEnvironment.GetParameterAsDouble ("TCMTRotationAngle") * deg;
  G4double xTranslation = _aGeometryEnvironment.GetParameterAsDouble("TCMTTranslateX");
  G4double yTranslation = _aGeometryEnvironment.GetParameterAsDouble("TCMTTranslateY");

  translationAdjust = G4ThreeVector(xTranslation, yTranslation, 0.0);
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
void TBcatcher09::SetDepthToLayer(G4int i) 
{
  depthToLayer = i;
#ifdef TCMT_DEBUG
  G4cout <<"DepthToLayer in Catcher: " << depthToLayer << G4endl;
#endif
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
G4bool TBcatcher09::BuildCatcher()
{
#ifdef TCMT_DEBUG
  G4cout<<"\n Bcatcher08::BuildCatcher: "<<G4endl;
#endif

  /* build layers*/
  BuildCatcherLayers();

  /* compute translation vection and rotation*/
  ComputeCatcherTransform();

  /* place in world volume*/
  PlaceCatcher();

  return true;
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
void TBcatcher09::SetSD()
{
#ifdef TCMT_DEBUG
  G4cout<<"\n TBcatcher09::SetSD: "<<G4endl;
#endif

  /* create SD*/
  catcherSD = new TBSD_VCell04("catcherSD",
			       GetGridSize(),
			       ncell_xy[0],
			       ncell_xy[1],
			       GetDepthToLayer(),
			       TBCATCHER);

  /* set active layer*/
  PolyActiveLogical->SetSensitiveDetector(catcherSD);

  /* register*/
  RegisterSensitiveDetector(catcherSD);

  /*============================================================*/
#ifdef USE_ABSORBER
  catcherSD_steelFront = new TBSD_VCell04("catcherSD_steelFront",
					  GetGridSize(),
					  ncell_xy[0],
					  ncell_xy[1],
					  GetDepthToLayer(),
					  TBCATCHER);
  
  /* set active layer*/
  SteelFrontLogical->SetSensitiveDetector(catcherSD_steelFront);

  /* register*/
  RegisterSensitiveDetector(catcherSD_steelFront);

  /*-----------------------------------------------------------*/
  catcherSD_steelBack = new TBSD_VCell04("catcherSD_steelBack",
					  GetGridSize(),
					  ncell_xy[0],
					  ncell_xy[1],
					  GetDepthToLayer(),
					  TBCATCHER);
  
  /* set active layer*/
  SteelBackLogical->SetSensitiveDetector(catcherSD_steelBack);

  /* register*/
  RegisterSensitiveDetector(catcherSD_steelBack);

  /*-----------------------------------------------------------*/
  catcherSD_steelAbsorberFine = new TBSD_VCell04("catcherSD_steelAbsorberFine",
						 GetGridSize(),
						 ncell_xy[0],
						 ncell_xy[1],
						 GetDepthToLayer(),
						 TBCATCHER);
  
  /* set active layer*/
  FineAbsorberLogical->SetSensitiveDetector(catcherSD_steelAbsorberFine);

  /* register*/
  RegisterSensitiveDetector(catcherSD_steelAbsorberFine);

  /*-----------------------------------------------------------*/
  catcherSD_steelAbsorberCoarse = new TBSD_VCell04("catcherSD_steelAbsorberCoarse",
						 GetGridSize(),
						 ncell_xy[0],
						 ncell_xy[1],
						 GetDepthToLayer(),
						 TBCATCHER);
  
  /* set active layer*/
  CoarseAbsorberLogical->SetSensitiveDetector(catcherSD_steelAbsorberCoarse);

  /* register*/
  RegisterSensitiveDetector(catcherSD_steelAbsorberCoarse);


 
#endif
}


/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
G4LogicalVolume* TBcatcher09::BuildCoarseLayerLogical(bool withCassette, G4ThreeVector& offset, G4int layerNumber)
{
  G4LogicalVolume *thisLogical = 0;

  /* real dimensions defined in terms of sensitive layer size, which comes from mokka DB*/
  G4Box *CoarseAbsorberBox = new G4Box("CoarseAbsorberBox", 
				       1.17*xlen_tcmt, 
				       1.17*ylen_tcmt, 
				       steel_absorber_coarse_hthickness);

  G4Box *CoarseLayerBox    = new G4Box("CoarseLayerBox", 
				       1.20*xlen_tcmt, 
				       1.20*ylen_tcmt, 
				       coarse_layer_hthickness[layerNumber]);

  std::stringstream stringForLayerNo; /*string to save layer number*/
  stringForLayerNo << (layerNumber + 1);

  CoarseAbsorberLogical = new G4LogicalVolume(CoarseAbsorberBox, steel, 
					      G4String("CoarseAbsorber_") + G4String(stringForLayerNo.str()));
  
  thisLogical = new G4LogicalVolume(CoarseLayerBox, air, G4String("CoarseLayer")+ G4String(stringForLayerNo.str()));

  G4ThreeVector vplace(0, 0, -coarse_layer_hthickness[layerNumber] + readout_module_hthickness);

#ifdef TCMT_DEBUG
  G4cout<<"\n place readout at z="<<vplace.z()<<G4endl;
#endif
  /* check whether cassette should be there or not*/
  if(withCassette) 
    {
      G4LogicalVolume *ro_lv = BuildReadoutModuleLogical();

      new G4PVPlacement(0,
			vplace+offset,
			ro_lv,
			pre_name + ro_lv->GetName() + G4String(stringForLayerNo.str()),
			thisLogical,
			false,
			(layerNumber + 1),
			checkForOverlappingVolumes);
    }
  
  vplace.setZ(vplace.z() + readout_module_hthickness + 2*air_front_hthickness[layerNumber] + steel_absorber_coarse_hthickness);
#ifdef TCMT_DEBUG
  G4cout<<" coarse_layer_hthickness: "<<coarse_layer_hthickness[layerNumber]
	<<" 2*air_front_hthickness: "<<2*air_front_hthickness[layerNumber] 
	<<" readout_module_hthickness: "<<readout_module_hthickness
	<<" zplace="<<vplace.z()
	<<G4endl;
#endif
 
  new G4PVPlacement(0,
		    vplace,
		    CoarseAbsorberLogical,
		    pre_name + CoarseAbsorberLogical->GetName()+ G4String(stringForLayerNo.str()),
		    thisLogical,
		    false,
		    (layerNumber + 1),
		    checkForOverlappingVolumes);

  assert(vplace.z() < abs(coarse_layer_hthickness));

  return thisLogical;
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
G4LogicalVolume* TBcatcher09::BuildFineLayerLogical(bool withCassette, G4ThreeVector& offset, G4int layerNumber)
{
  G4LogicalVolume *thisLogical = NULL;

#ifdef TCMT_DEBUG
  G4cout<<"\n BuildFineLayerLogical: layerNumber="<<layerNumber<<" withCassette="<<withCassette<<G4endl;
#endif

  /* real dimensions defined in terms of sensitive layer size, which comes from mokka DB*/
  G4Box *FineAbsorberBox = new G4Box("FineAbsorberBox", 1.17*xlen_tcmt, 1.17*ylen_tcmt, steel_absorber_fine_hthickness);
  G4Box *FineLayerBox    = new G4Box("FineLayerBox", 1.20*xlen_tcmt, 1.20*ylen_tcmt, fine_layer_hthickness[layerNumber]);

   std::stringstream stringForLayerNo; /*string to save layer number*/
   stringForLayerNo << (layerNumber + 1); 

  FineAbsorberLogical = new G4LogicalVolume(FineAbsorberBox, steel, 
					    G4String("FineAbsorber") + G4String(stringForLayerNo.str()));
  thisLogical = new G4LogicalVolume(FineLayerBox, air, 
				    G4String("FineLayer_") + G4String(stringForLayerNo.str()));


  /*z-place for the readout module (i.e. scintillator) inside the fine layer*/
  G4ThreeVector vplace(0., 0., -fine_layer_hthickness[layerNumber] + readout_module_hthickness);

 #ifdef TCMT_DEBUG
 G4cout<<"\n place readout at z="<<vplace.z()<<G4endl;
 #endif
 /* check whether cassette should be there or not*/
  if(withCassette) 
    {
      G4LogicalVolume *ro_lv = BuildReadoutModuleLogical();
      new G4PVPlacement(0,
			vplace+offset,
			ro_lv,
			pre_name + ro_lv->GetName() + G4String(stringForLayerNo.str()),
			thisLogical,
			false,
			(layerNumber + 1),
			checkForOverlappingVolumes);
    }
  
  /*z-place of the absorber inside the fine layer*/
  vplace.setZ(vplace.z() + readout_module_hthickness + 2*air_front_hthickness[layerNumber] + steel_absorber_fine_hthickness);

#ifdef TCMT_DEBUG
  G4cout<<" fine_layer_hthickness: "<<fine_layer_hthickness[layerNumber]
	<<" 2*air_front_hthickness: "<<2*air_front_hthickness[layerNumber] 
	<<" readout_module_hthickness: "<<readout_module_hthickness
	<<" zplace="<<vplace.z()
	<<G4endl;
#endif

  new G4PVPlacement(0,
		    vplace,
		    FineAbsorberLogical,
		    pre_name + FineAbsorberLogical->GetName() + G4String(stringForLayerNo.str()),
		    thisLogical,
		    false,
		    (layerNumber + 1),
		    checkForOverlappingVolumes);
  return thisLogical;
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
G4LogicalVolume* TBcatcher09::BuildReadoutModuleLogical()
{
#ifdef TCMT_DEBUG
  G4cout<<"\n   TBcatcher09::BuildReadoutModuleLogical, ReadoutModuleLogical="<<ReadoutModuleLogical<<G4endl;
#endif

  if (ReadoutModuleLogical == 0) 
    {
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
			0,
			checkForOverlappingVolumes);
      
      vplace.setZ(vplace.z() + steel_front_hthickness  + poly_front_hthickness);

      new G4PVPlacement(0,
			vplace,
			PolyFrontLogical,
			pre_name + PolyFrontLogical->GetName(),
			ReadoutModuleLogical,
			false,
			1,
			checkForOverlappingVolumes);
      
      vplace.setZ(vplace.z() + poly_front_hthickness + tyvek_front_hthickness);
      
      new G4PVPlacement(0,
			vplace,
			TyvekFrontLogical,
			pre_name + TyvekFrontLogical->GetName(),
			ReadoutModuleLogical,
			false,
			2,
			checkForOverlappingVolumes);
      
      vplace.setZ(vplace.z() + tyvek_front_hthickness + poly_active_hthickness);
      
      new G4PVPlacement(0,
			vplace,
			PolyActiveLogical,
			pre_name + PolyActiveLogical->GetName(),
			ReadoutModuleLogical,
			false,
			3,
			checkForOverlappingVolumes);
      
      vplace.setZ(vplace.z() + poly_active_hthickness + tyvek_back_hthickness);
      
      new G4PVPlacement(0,
			vplace,
			TyvekBackLogical,
			pre_name + TyvekBackLogical->GetName(),
			ReadoutModuleLogical,
			false,
			4,
			checkForOverlappingVolumes);
      
      vplace.setZ(vplace.z() + tyvek_back_hthickness + poly_back_hthickness);
      
      new G4PVPlacement(0,
			vplace,
			PolyBackLogical,
			pre_name + PolyBackLogical->GetName(),
			ReadoutModuleLogical,
			false,
			5,
			checkForOverlappingVolumes);
      
      vplace.setZ(vplace.z() + poly_back_hthickness + steel_back_hthickness);
      
      new G4PVPlacement(0,
			vplace,
			SteelBackLogical,
			pre_name + SteelBackLogical->GetName(),
			ReadoutModuleLogical,
			false,
			6,
			checkForOverlappingVolumes);
      
      assert(vplace.z() < abs(readout_module_hthickness));
    }
  
  return ReadoutModuleLogical;
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
G4LogicalVolume* TBcatcher09::BuildCatcherEnvelope()
{
  if (DetectorLogical == 0) 
    {
      /* xlen_tcmt, ylen_tcmt, zlen_tcmt are x,y,z-dimensions of sensitive layer (1m x 1m)*/
      G4Box *CatcherBox = new G4Box("CatcherBox", 1.20*xlen_tcmt, 1.20*ylen_tcmt, zlen_tcmt);
      
      DetectorLogical = new G4LogicalVolume(CatcherBox, steel, "Catcher");
    }
  
  return DetectorLogical;
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
void TBcatcher09::BuildCatcherLayers()
{
#ifdef TCMT_DEBUG
  G4cout<<"\n TBcatcher09::BuildCatcherLayers: "<<G4endl;
#endif

  G4LogicalVolume *catcher = BuildCatcherEnvelope();

  G4LogicalVolume *thisLayer = 0;
  G4ThreeVector vplace(0., 0., -zlen_tcmt);

  /* staggering is defined here... hardcoded (at least for now)*/
  G4ThreeVector stagger[17] = {G4ThreeVector(0,0,0)};

  G4double zplace = -zlen_tcmt;

#ifdef TCMT_DEBUG
  G4cout<<" zplace="<<zplace<<" n_layers: "<<n_layers<<G4endl;
#endif


  for (G4int i = 0; i < n_layers; i++)
    {
      bool withCassette = (_layerPattern.data()[i] == '1');
      
      if (i < n_fine_layers) 
	{
#ifdef TCMT_DEBUG
	  G4cout<<"\n ==================================================="<<G4endl;
	  G4cout<<" FINE layer "<<i+1
		<<" hthickness: "<<fine_layer_hthickness[i]
		<<G4endl;
#endif
	  
	  thisLayer = BuildFineLayerLogical(withCassette, stagger[i], i);
	  
	  if (i == 0) zplace += fine_layer_hthickness[i];
	  else        zplace += fine_layer_hthickness[i-1] + fine_layer_hthickness[i];
	}
      else 
	{
	  G4int nLayerCoarse = i - n_coarse_layers;
	  	  
#ifdef TCMT_DEBUG
	G4cout<<"\n ==================================================="<<G4endl;
	G4cout<<" COARSE layer "<<i+1<<" nLayerCoarse="<<nLayerCoarse
	      <<" hthickness: "<<coarse_layer_hthickness[nLayerCoarse]
	      <<" withCassette: "<<withCassette
	      <<G4endl;
#endif
	thisLayer = BuildCoarseLayerLogical(withCassette, stagger[i], nLayerCoarse);

 	if (nLayerCoarse == 0) 
	  {
	    zplace += fine_layer_hthickness[i-1] + coarse_layer_hthickness[nLayerCoarse];
	  }
	else    
	  {
	    zplace += coarse_layer_hthickness[nLayerCoarse-1] + coarse_layer_hthickness[nLayerCoarse];
	  }
     }
    
    
    vplace.setZ(zplace);

#ifdef TCMT_DEBUG
    G4cout<<"\n Place layer at z = "<<vplace.z()<<" PolyActiveLogical="<<PolyActiveLogical<<G4endl;
#endif

    new G4PVPlacement(0,
		      vplace,
		      thisLayer,
		      pre_name + thisLayer->GetName(),
		      catcher,
		      false,
		      i,
		      checkForOverlappingVolumes);
    }/*end loop over layers*/
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
void TBcatcher09::ComputeCatcherTransform()
{
#ifdef TCMT_DEBUG
  G4cout << G4endl << "TBcatcher09::ComputeCatcherTransform()" << G4endl;
#endif
  G4double costheta = cos(config_angle);
  G4double sintheta = sin(config_angle);
  if(costheta <= 0) 
    {
      G4cout << "Bad Configuration Angle: " << config_angle/deg << G4endl;
      G4cout << "ending..." << G4endl;
      exit(1);
    }
  
  /* hcal dimensions needed to align the catcher w.r.t the Hcal*/
  xlen_hcal = (G4double) (ncell_xy_hcal[0] * grid_size_hcal*mm)/2.;
  ylen_hcal = (G4double) (ncell_xy_hcal[1] * grid_size_hcal*mm)/2.;

#ifdef TCMT_DEBUG
  G4cout << "config_angle <" << config_angle/deg << ">" << G4endl;
  G4cout << "xlen_hcal <" << xlen_hcal << ">" << G4endl;
  G4cout << "ylen_hcal <" << ylen_hcal << ">" << G4endl;
#endif

  /* half-thickness of hcal layers*/
  float tungsten_hthickness = absorber_hthickness_hcal;
  float steel_support_hthickness = steel_support_hthickness_hcal;//0.5/2*mm;

  layer_hthickness_hcal = poly_hthickness_hcal     /*scintillator*/
    + tungsten_hthickness                          /*absorber*/
    + steel_support_hthickness                     /* steel support */  
    + 2.0*airgap_hthickness_hcal                   /*2 times air gap*/
    + 2.0*steel_cassette_hthickness_hcal           /*2 times steel cassette*/
    + 2.0*foil_hthickness_hcal                     /*2 times 3M foil*/
    + pcb_hthickness_hcal                          /*PCB*/
    + cablefibre_mix_hthickness_hcal;              /*cable-fibre mix*/

#ifdef TCMT_DEBUG
  G4cout<<" \n ================================ "<<G4endl;
  G4cout<<"  HCAL for TCMT: \n"<<G4endl;
  G4cout<<" steel_support : "<<2*steel_support_hthickness<<G4endl;
  G4cout<<" tungsten      : "<<2*tungsten_hthickness<<G4endl;
  G4cout<<" airgap        : "<<2*airgap_hthickness_hcal<<G4endl;
  G4cout<<" steel_cassette: "<<2*steel_cassette_hthickness_hcal<<G4endl;
  G4cout<<" foil          : "<<2*foil_hthickness_hcal<<G4endl;
  G4cout<<" pcb           : "<<2*pcb_hthickness_hcal <<G4endl;
  G4cout<<" cablefibre_mix: "<<2*cablefibre_mix_hthickness_hcal<<G4endl;
  G4cout<<" scintillator  : "<<2*poly_hthickness_hcal<<G4endl;
  G4cout<<" n_layers_hcal : "<<n_layers_hcal<<G4endl;
  G4cout<<" layer_hthickness: "<<2*layer_hthickness_hcal<<G4endl;
#endif

  /* half-z-extension of Hcal*/
  zlen_hcal = (G4double) (n_layers_hcal * layer_hthickness_hcal + absorber_hthickness_term_hcal );

  /*calculate the end point of the hcal ALONG MAIN TRAJECTORY*/
  G4double z_end_hcal = z_begin_hcal + 2.*zlen_hcal/costheta;

  /* Positioning the catcher, following a TB configuration angle.
     account for hcal corners approaching the catcher due to config_angle*/
  z_begin = z_end_hcal + xlen_hcal*sintheta + dist_hcal_tcmt;

  /* derive from that the position where to place the Catcher (its center)*/
  z_place = z_begin + zlen_tcmt;

  z_end_tcmt = z_begin + 2 * zlen_tcmt;

#ifdef TCMT_DEBUG
  G4cout << " n_layer_term_hcal             : " << n_layer_term_hcal << G4endl;
  G4cout << " absorber_hthickness_term_hcal : " << absorber_hthickness_term_hcal << G4endl;
  G4cout << " zlen_hcal                     : " << zlen_hcal << G4endl;
  G4cout << " z_begin_hcal                  : " << z_begin_hcal << G4endl;
  G4cout << " z_end_hcal                    : " << z_end_hcal << G4endl;
  G4cout << " xlen_hcal*sintheta            : " << (xlen_hcal*sintheta) << G4endl;
  G4cout << " dist_hcal_tcmt                : " << dist_hcal_tcmt << G4endl;
  G4cout << " z_begin                       : " << z_begin << G4endl;
  G4cout << " z_place                       : " << z_place << G4endl;
#endif

  /* Next apply adjustments for alignment correction*/
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

/*================================================================================*/
/*                                                                                */
/*       create placement in world vol                                            */
/*                                                                                */
/*================================================================================*/
void TBcatcher09::PlaceCatcher()
{
#ifdef TCMT_DEBUG
  G4cout<<" \n\n PlaceCatcher: checkForOverlappingVolumes="<< checkForOverlappingVolumes<<G4endl;
  G4cout<<" DetectorLogical: "<<DetectorLogical<<G4endl;
#endif

  new G4PVPlacement(*transformCatcher,
		    DetectorLogical,
		    "Catcher",
		    WorldLogical,
		    false,
		    0,
		    checkForOverlappingVolumes);
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
void TBcatcher09::Print()
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

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
G4bool TBcatcher09::PostConstructAction(CGAGeometryEnvironment& )
{
  /*makes available the Z of the TCMT exit face for the Muon Counter
    present after the TCMT*/

  std::ostringstream oss;
  oss << z_end_tcmt;
  (*Control::globalModelParameters)["z_end_tcmt"] =
    oss.str();

  G4cout << "TCMT information: z_end_tcmt = " << z_end_tcmt << G4endl;

  return true;
}
