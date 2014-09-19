#include "Control.hh"
#include "TBhcal09.hh"
#include "CGADefs.h"
#include "MySQLWrapper.hh"
#include "CGAGeometryManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4PVParameterised.hh"
#include "G4UnitsTable.hh"
#include "UserInit.hh"
#include "G4Polyhedra.hh"
#include "G4SubtractionSolid.hh"

#ifdef MOKKA_GEAR
#include "gear/CalorimeterParameters.h" 
#include "gearimpl/CalorimeterParametersImpl.h" 
#include "gearimpl/LayerLayoutImpl.h" 
#include "MokkaGear.h"
#endif

#include <sstream>

/*In case you want to make all materials in HCAL sensitive, uncomment the line below
  (may be useful for studies of the sampling fractions)
*/
//#define USE_ABSORBER


//#define TBHCAL09_DEBUG

/* define some levels of detail for graphical display*/
#define DM_FULL 3
#define DM_ABSORBERANDSENSITIVE 2
#define DM_WHOLELAYERONLY  1



INSTANTIATE(TBhcal09)
/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
  TBhcal09::TBhcal09() : VSubDetectorDriver("TBhcal09","TBhcal"),
			 db(0),
			 _aGeometryEnvironment("","",NULL, NULL),
			 config_angle(0)  
{
  checkForOverlappingVolumes = false;
#ifdef TBHCAL09_DEBUG
  checkForOverlappingVolumes = true;
#endif
}
/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
TBhcal09::~TBhcal09()
{}
/*================================================================================*/
/*                                                                                */
/*     Main function                                                              */
/*                                                                                */
/*================================================================================*/
G4bool TBhcal09::ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
				     G4LogicalVolume *WorldLog)
{
  G4cout<<"\n ***************************************************************"<<G4endl;
  G4cout<<" *                                                             *"<<G4endl;
  G4cout<<" *    Build HCAL based on the TBhcal09 driver                  *"<<G4endl;
  G4cout<<" *                                                             *"<<G4endl;
  G4cout<<" ***************************************************************"<<G4endl;

  /*Obtain the pointer to our database via the Environment object*/
  _aGeometryEnvironment = aGeometryEnvironment;
  db = new Database(_aGeometryEnvironment.GetDBName());

  WorldLogical = WorldLog; 
  
  G4cout<<"\n Before FetchMySQLVariables"<<G4endl;
  FetchMySQLVariables();

  G4cout<<"\n before DefineHcalMaterials "<<G4endl;
  DefineHcalMaterials();


  G4cout<<"\n before BuildHcalElements "<<G4endl;
  BuildHcalElements();
  G4cout<<"\n before Print "<<G4endl;
  Print();
  
 /*-------------------------------------------------------------------
    GEAR information
   */
  G4double innerRadius = 0;
  G4double outerRadius = cal_hx ;
  G4double leastZ      = z_begin;
  G4int symmetryOrder  = 4;      /*this is a standalone prototype*/
  G4double phi         = 0;
  gear::CalorimeterParametersImpl *gearParam = 
    new gear::CalorimeterParametersImpl(innerRadius, outerRadius, leastZ, symmetryOrder, phi);

  for (G4int iLayer = 0; iLayer < HCAL_n_layers; ++iLayer)
    {
      G4double distance = 0; /*distance of this layer from the origin*/
      G4double layerThickness = (tungsten_hthickness + steel_support_hthickness + scinCassette_hthickness) * 2.;
      G4double cellSize0 = 30. * mm; /*cell size along the x-axis*/
      G4double cellSize1 = 30. * mm; /*cell size along the y-axis*/
      G4double absorberThickness = tungsten_hthickness * 2.;

      gearParam->layerLayout().positionLayer(distance, layerThickness, cellSize0, cellSize1, absorberThickness);
      
    }/*end loop over iLayer*/

  /* write parameters to GearManager*/
  MokkaGear* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setHcalEndcapParameters( gearParam ) ;  
  /*-------------------------------------------------------------------*/

  G4bool cokay = PlaceHcalLayers(WorldLogical);

  delete db;
  db = 0;
  
  G4cout << "\nDone building TBhcal09" << G4endl;
  
  return cokay;
}


/*================================================================================*/
/*                                                                                */
/*       Fetch HCAL related MySQL variables                                       */
/*                                                                                */
/*================================================================================*/
void TBhcal09::FetchMySQLVariables()
{

  /* config angle from environment object
     config_angle: angle reflecting the rotations for non normal incidence.*/
  config_angle = _aGeometryEnvironment.GetParameterAsDouble("HCAL_configuration_angle");
  config_angle = config_angle*deg;
  
  /*we are dealing with a simple cell grid 
    We only need to know the number of layers, the number of cells in x and z direction 
    and the basic grid size to get the layering n 
  */

  db->exec("select * from HCAL_cernJune2011_virt;");
  db->getTuple();

  HCAL_n_layers                    = db->fetchInt("HCAL_n_layers");//38;//db->fetchInt("HCAL_n_layers");
  //HCAL_n_layers                    = 30;

  /*Number of cells in x and y*/
  HCAL_Layer_ncell_x               = db->fetchInt("HCAL_Layer_ncell_x");//90*mm;//db->fetchInt("HCAL_Layer_ncell_x");
  HCAL_Layer_ncell_y               = db->fetchInt("HCAL_Layer_ncell_y");//90*mm;//db->fetchInt("HCAL_Layer_ncell_y");


  HCAL_Layer_X_dimension           = db->fetchInt("HCAL_Layer_X_dimension")/2;
  HCAL_Layer_Y_dimension           = db->fetchInt("HCAL_Layer_Y_dimension")/2;
  HCAL_grid_size                   = db->fetchDouble("HCAL_grid_size");

  assert(HCAL_n_layers > 0);
  assert(HCAL_Layer_ncell_x >= 0 || HCAL_Layer_ncell_y >= 0);

  /*the beginning of the Hcal */
  x_begin                          = db->fetchDouble("X_position_of_HCAL_frontFace");
  y_begin                          = db->fetchDouble("Y_position_of_HCAL_frontFace");
  z_begin                          = db->fetchDouble("Z_position_of_HCAL_frontFace");//488.7*mm;
  
  /* get 'global' x and y translation from steering file (at run time) or database*/
  G4double XTranslation = _aGeometryEnvironment.GetParameterAsDouble("HcalTranslateX");
  G4double YTranslation = _aGeometryEnvironment.GetParameterAsDouble("HcalTranslateY");
  x_begin += XTranslation;
  y_begin += YTranslation;

  /* get rotation angle from steering file (at run time) or database*/
  rotationAngle = _aGeometryEnvironment.GetParameterAsDouble("HcalRotationAngle");
  rotationAngle = rotationAngle*deg;
  
  /* thicknesses*/
  db->exec("select * from HCAL_cernJune2011_layerThickness_virt;");
  db->getTuple();

  poly_hthickness                     = db->fetchDouble("poly_thickness")/2;
  tungsten_hthickness                 = db->fetchDouble("tungsten_thickness")/2;
  steel_support_hthickness            = db->fetchDouble("steel_support_thickness")/2;//0.5/2.;//db->fetchDouble("steel_support_thickness")/2;

  airgap_hthickness                   = db->fetchDouble("airgap_thickness")/2;
  steel_cassette_hthickness           = db->fetchDouble("steel_cassette_thickness")/2.;
  foil_hthickness                     = db->fetchDouble("foil_thickness")/2.;
  pcb_hthickness                      = db->fetchDouble("pcb_thickness")/2.;
  cablefibre_mix_hthickness           = db->fetchDouble("cablefibre_mix_thickness")/2.;

  /*Absorber stuff*/
  db->exec("select * from HCAL_cernJune2011_absorber_virt;");
  db->getTuple();
  
  Octagonal_absorber_inner_radius[0] = db->fetchDouble("Octagonal_absorber_inner_radius_front");
  Octagonal_absorber_inner_radius[1] = db->fetchDouble("Octagonal_absorber_inner_radius_back"); 
  Octagonal_absorber_outer_radius[0] = db->fetchDouble("Octagonal_absorber_outer_radius_front");
  Octagonal_absorber_outer_radius[1] = db->fetchDouble("Octagonal_absorber_outer_radius_back"); 
  Octagonal_absorber_number_of_sides  = db->fetchInt("Octagonal_absorber_number_of_sides");    
  Octagonal_absorber_number_of_layers = db->fetchInt("Octagonal_absorber_number_of_layers");
  Octagonal_absorber_z[0]             = (-1) * tungsten_hthickness;      
  Octagonal_absorber_z[1]             = tungsten_hthickness;

  /* Materials*/
  db->exec("select * from HCAL_cernJune2011_materials_virt;");
  db->getTuple();
  PCB_density			      = db->fetchDouble("PCB_density")*g/cm3;
  PCB_silicon_fractiomass	      = db->fetchDouble("PCB_silicon_2.33_fractiomass"); 
  PCB_elO_fractionmass		      = db->fetchDouble("PCB_elO_fractionmass"); 
  PCB_graphite_fractionmass	      = db->fetchDouble("PCB_graphite_fractionmass"); 
  PCB_elH_fractionmass		      = db->fetchDouble("PCB_elH_fractionmass"); 
  PCB_elBr_fractionmass		      = db->fetchDouble("PCB_elBr_fractionmass"); 
  S235_density			      = db->fetchDouble("S235_density")*g/cm3;
  S235_iron_fractionmass	      = db->fetchDouble("S235_iron_fractionmass"); 
  S235_graphite_fractionmass	      = db->fetchDouble("S235_graphite_fractionmass"); 
  S235_manganese_fractionmass	      = db->fetchDouble("S235_manganese_fractionmass"); 
  tungstenalloy_density		      = db->fetchDouble("tungsten_density")*g/cm3; 
  coretun_density		      = db->fetchDouble("tungsten_core_density")*g/cm3; 
  tungsten_fractionmass	              = db->fetchDouble("tungsten_core_tungsten_fractionmass"); 
  nikel_fractionmass		      = db->fetchDouble("tungsten_nikel_fractionmass"); 
  nikel_density                       = db->fetchDouble("nikel_density")*g/cm3;
  copper_fractionmass		      = db->fetchDouble("tungsten_copper_fractionmass"); 
  PVC_density			      = db->fetchDouble("PVC_density")*g/cm3;
  Polystyrole_density		      = db->fetchDouble("Polystyrole_density")*g/cm3; 
  CF_Mix_density		      = db->fetchDouble("CF_Mix_density")*g/cm3;
  CF_MIX_air_fractionmass	      = db->fetchDouble("CF_MIX_air_fractionmass"); 
  CF_MIX_PVC_fractionmass	      = db->fetchDouble("CF_MIX_PVC_fractionmass"); 
  CF_MIX_Polystyrole_fractionmass     = db->fetchDouble("CF_MIX_Polystyrole_fractionmass");



  
  /*----------------------------------------------------------------------
    scintillator cassette + contents + air gaps:
   ----------------------------------------------------------------------*/
  scinCassette_hthickness =  2.0*airgap_hthickness /*2 times air gap*/
    + 2.0*steel_cassette_hthickness                /*2 times steel cassette*/
    + cablefibre_mix_hthickness                    /*cable-fibre mix*/
    + pcb_hthickness                               /*PCB*/
    + 2.0*foil_hthickness                          /*2 times 3M foil*/
    + poly_hthickness;                             /*scintillator*/

}
/*================================================================================*/
/*                                                                                */
/*  Define calculated variables, materials, solids, logical volumes               */
/*                                                                                */
/*================================================================================*/
void TBhcal09::BuildHcalElements()
{
  /*Information needed when hits are processed later on*/
  /*Must be set before calling the constructor of the sensitive detector,
   which is done in SetSD()!*/
  SetDepthToLayer(1);
  /* create and register the sensitive detector before
     defining the sensitive logical volumes!
  */
  SetSD();

  /*calorimeter dimensions*/
  cal_hz = 0;
  cal_hx = (G4double) (HCAL_Layer_ncell_x * HCAL_grid_size*mm)/2.;
  cal_hy = (G4double) (HCAL_Layer_ncell_y * HCAL_grid_size*mm)/2.;

  std::stringstream stringForLayerNo; /*string to save layer number*/

  G4Box              *WholeLayerSolid[MAX_TBHCAL_LAYERS]        = {NULL};
  G4Box              *WholeScinCassetteSolid[MAX_TBHCAL_LAYERS] = {NULL};
  G4Polyhedra        *AbsLayerSolid[MAX_TBHCAL_LAYERS]          = {NULL};
  G4Box              *AluminiumframeAll[MAX_TBHCAL_LAYERS]      = {NULL};
  G4SubtractionSolid *AluminiumframeSolid[MAX_TBHCAL_LAYERS]    = {NULL};
  G4Box              *SteelSupportSolid[MAX_TBHCAL_LAYERS]      = {NULL};

  /*the scintillator housing (made of S235)*/
  G4Box *ScinHousSolid = new G4Box("HcalScinHousSolid",             cal_hx, cal_hy, steel_cassette_hthickness);
  /*the cable-fibre mixture */
  G4Box *CFmix_LayerSolid = new G4Box("HcalCFmix_LayerSolid",       cal_hx, cal_hy, cablefibre_mix_hthickness);
  /*a PCB layer*/
  G4Box *PCBLayerSolid = new G4Box("HcalPCBLayerSolid",             cal_hx, cal_hy, pcb_hthickness);
  /*3M foil*/
  G4Box *FoilLayerSolid = new G4Box("Hcal3MFoilSolid",              cal_hx, cal_hy, foil_hthickness);
  /* Scintillator*/
  G4Box *WholeSensLayerSolid = new G4Box("HcalWholeSensLayerSolid", cal_hx, cal_hy, poly_hthickness);

  for (G4int i = 0; i < MAX_TBHCAL_LAYERS; ++i)
    {
      WholeLayerLogical[i]        = NULL;
      WholeScinCassetteLogical[i] = NULL;
      AbsLayerLogical[i]          = NULL;
      AluminiumframeLogical[i]    = NULL;
      SteelSupportLogical[i]      = NULL;
      ScinHousLogical[i]          = NULL;
      ScinHousLogical2[i]         = NULL;
      CFmix_LayerLogical[i]       = NULL;
      PCBLayerLogical[i]          = NULL;
      FoilLayerLogical_1[i]       = NULL;
      FoilLayerLogical_2[i]       = NULL;
      WholeSensLayerLogical[i]    = NULL;
    }

  /*----------------------------------------------------------------------------*/
  for (G4int iLayer = 0; iLayer < HCAL_n_layers; ++iLayer)
    {
      /* HCAL layer thickness */
      layer_hthickness[iLayer] = poly_hthickness       /*scintillator*/
	+ tungsten_hthickness                          /*absorber*/
	+ steel_support_hthickness                     /* steel support */  
	+ 2.0*airgap_hthickness                        /*2 times air gap*/
	+ 2.0*steel_cassette_hthickness                /*2 times steel cassette*/
	+ 2.0*foil_hthickness                          /*2 times 3M foil*/
	+ pcb_hthickness                               /*PCB*/
	+ cablefibre_mix_hthickness;                   /*cable-fibre mix*/

      cal_hz += layer_hthickness[iLayer];

      stringForLayerNo << (iLayer+1);      

      G4cout<<" layer "<<iLayer+1<<" thickness: "<<layer_hthickness[iLayer]*2<<G4endl;
      
      /*create a layer filled with air, with enough space for all its constituents*/
      WholeLayerSolid[iLayer]   = new G4Box("HcalLayerSolid", cal_hx, cal_hy, layer_hthickness[iLayer]);
      WholeLayerLogical[iLayer] = new G4LogicalVolume(WholeLayerSolid[iLayer], air, 
						      G4String("HcalLayerLogical") + G4String(stringForLayerNo.str()), 0, 0, 0);

      /*create a whole for: 
       air + steel cassette + PCB foil + 3M foil + scintillator + 3M foil + steel cassette + air*/
      WholeScinCassetteSolid[iLayer]   = new G4Box("WholeScinCassetteSolid", cal_hx, cal_hy, scinCassette_hthickness);
      WholeScinCassetteLogical[iLayer] = new G4LogicalVolume(WholeScinCassetteSolid[iLayer], air,
							     G4String("WholeScinCassetteLogicalLogical") 
							     + G4String(stringForLayerNo.str()), 0, 0, 0);

      /*create an absorber plate with an octogonal shape*/
      /*the 90/4 degrees is here needed because by default the octagon is tilted wrt how we want it*/
      AbsLayerSolid[iLayer] = new G4Polyhedra("AbsLayerSolid", 90/4*deg, 0,
 					      Octagonal_absorber_number_of_sides,
					      Octagonal_absorber_number_of_layers,
					      Octagonal_absorber_z,
					      Octagonal_absorber_inner_radius,
					      Octagonal_absorber_outer_radius);
   
      AbsLayerLogical[iLayer] = new G4LogicalVolume(AbsLayerSolid[iLayer], tungstenalloy, 
						    G4String("HcalAbsLayerLogical") + G4String(stringForLayerNo.str()), 0, 0, 0);
      
#ifdef USE_ABSORBER
      AbsLayerLogical[iLayer]->SetSensitiveDetector(hcalSD_W);
#endif

      /* And the aluminium frame around it*/          
      AluminiumframeAll[iLayer]     = new G4Box("AluminiumframeAll", HCAL_Layer_X_dimension, HCAL_Layer_Y_dimension, tungsten_hthickness);
      AluminiumframeSolid[iLayer]   = new G4SubtractionSolid("AluminiumframeAll - AbsLayerSolid", 
							     AluminiumframeAll[iLayer], AbsLayerSolid[iLayer]);
      AluminiumframeLogical[iLayer] = new G4LogicalVolume(AluminiumframeSolid[iLayer], aluminium,
							  G4String("HcalAluminiumframeLogical") + G4String(stringForLayerNo.str()), 
							  0, 0, 0);

      /*Steel Support*/
      SteelSupportSolid[iLayer] = new G4Box("SteelSupportSolid",cal_hx, cal_hy, steel_support_hthickness);
      SteelSupportLogical[iLayer] = new G4LogicalVolume(SteelSupportSolid[iLayer], S235,
							G4String("HcalSteelSupportLogical") + G4String(stringForLayerNo.str()), 0, 0, 0);
#ifdef USE_ABSORBER
      SteelSupportLogical[iLayer]->SetSensitiveDetector(hcalSD_FeSupport);
#endif
      /*the scintillator housing (made of S235)*/
      ScinHousLogical[iLayer] = new G4LogicalVolume(ScinHousSolid, S235, 
						    G4String("HcalScinHouseLogical") + G4String(stringForLayerNo.str()), 0, 0, 0);
#ifdef USE_ABSORBER
      ScinHousLogical[iLayer]->SetSensitiveDetector(hcalSD_FeCassette1);
#endif
      ScinHousLogical2[iLayer] = new G4LogicalVolume(ScinHousSolid, S235, 
						    G4String("HcalScinHouseLogical2") + G4String(stringForLayerNo.str()), 0, 0, 0);
#ifdef USE_ABSORBER
      ScinHousLogical2[iLayer]->SetSensitiveDetector(hcalSD_FeCassette2);
#endif

      /*the cable-fibre mixture */
      CFmix_LayerLogical[iLayer] = new G4LogicalVolume(CFmix_LayerSolid, CF_MIX, 
						       G4String("CFmix_LayerLogical") + G4String(stringForLayerNo.str()), 0, 0, 0);
      
      /*a pcb layer*/
      PCBLayerLogical[iLayer] = new G4LogicalVolume(PCBLayerSolid, PCB, 
						    G4String("HcalPCBLayerLogical") + G4String(stringForLayerNo.str()), 0, 0, 0);


      /*3M foil*/
      FoilLayerLogical_1[iLayer] = new G4LogicalVolume(FoilLayerSolid, Polystyrole,
						       G4String("HcalFoilLayerLogical_1") + G4String(stringForLayerNo.str()), 0, 0, 0);
      FoilLayerLogical_2[iLayer] = new G4LogicalVolume(FoilLayerSolid, Polystyrole, 
						       G4String("HcalFoilLayerLogical_2") + G4String(stringForLayerNo.str()), 0, 0, 0);
      
      /* Scintillator*/
      WholeSensLayerLogical[iLayer] = new G4LogicalVolume(WholeSensLayerSolid, poly, 
							  G4String("HcalWholeSensLayerLogical") 
							  + G4String(stringForLayerNo.str()), 0, 0, 0);
      WholeSensLayerLogical[iLayer]->SetSensitiveDetector(hcalSD);

    }/*end loop over layers*/
     
  
  /*-------------------------------------------------------------------------------------------------*/
  /*terminating absorber plate*/
  G4Polyhedra *AbsLayerSolid_term = new G4Polyhedra("AbsLayerSolid", 90/4*deg, 0,
						    Octagonal_absorber_number_of_sides,
						    Octagonal_absorber_number_of_layers,
						    Octagonal_absorber_z,
						    Octagonal_absorber_inner_radius,
						    Octagonal_absorber_outer_radius);
  AbsLayerLogical_term = new G4LogicalVolume(AbsLayerSolid_term, tungstenalloy, 
					     G4String("HcalAbsLayerLogical_term"), 0, 0, 0);
  
#ifdef USE_ABSORBER
  AbsLayerLogical_term->SetSensitiveDetector(hcalSD_W);
#endif
  /*-------------------------------------------------------------------------------------------------*/




  /*the z-position of the of the calorimeter*/
  z_place = z_begin + cal_hz;  

  /*create first the whole detector, filled with air*/
  G4Box *DetectorSolid = new G4Box("HcalDetectorSolid", cal_hx, cal_hy, cal_hz);
  DetectorLogical = new G4LogicalVolume(DetectorSolid, air, "HcalDetectorLogical", 0, 0, 0);

 /*------------------------------------------------------------*/
  int displayMode = UserInit::getInstance()->getInt("HcalDisplayMode") ;
  /* if nothing specified, display full details*/
  if( displayMode == 0 )  displayMode = DM_FULL ;

  G4cout << "\n  Using display mode " << displayMode<<"\n" << G4endl;

  if( displayMode < DM_ABSORBERANDSENSITIVE ) 
    {
      for (G4int iLayer = 0; iLayer < HCAL_n_layers; ++iLayer)
	{
	  AbsLayerLogical[iLayer]->SetVisAttributes(G4VisAttributes::Invisible);
	  AbsLayerLogical_term->SetVisAttributes(G4VisAttributes::Invisible);
	}
    }
  
  if( displayMode < DM_FULL )  
    {
      for (G4int iLayer = 0; iLayer < HCAL_n_layers; ++iLayer)
	{
	  ScinHousLogical[iLayer]->SetVisAttributes(G4VisAttributes::Invisible);
	  ScinHousLogical2[iLayer]->SetVisAttributes(G4VisAttributes::Invisible);
	  CFmix_LayerLogical[iLayer]->SetVisAttributes(G4VisAttributes::Invisible);
	  PCBLayerLogical[iLayer]->SetVisAttributes(G4VisAttributes::Invisible);
	  FoilLayerLogical_1[iLayer]->SetVisAttributes(G4VisAttributes::Invisible);
	  FoilLayerLogical_2[iLayer]->SetVisAttributes(G4VisAttributes::Invisible);
	  WholeSensLayerLogical[iLayer]->SetVisAttributes(G4VisAttributes::Invisible);
	}
    }
  
  if (displayMode == DM_FULL)
    {
      for (G4int iLayer = 0; iLayer < HCAL_n_layers; ++iLayer)
	{
	  G4VisAttributes *visAtt = new G4VisAttributes(G4Colour::Yellow());
	  //visAtt->SetForceWireframe(true);
	  visAtt->SetForceSolid(true);
	  visAtt->SetForceAuxEdgeVisible(true);

	  WholeSensLayerLogical[iLayer]->SetVisAttributes(visAtt);

	  visAtt = new G4VisAttributes(G4Colour::Blue());
	  AbsLayerLogical[iLayer]->SetVisAttributes(visAtt);	  

	  visAtt = new G4VisAttributes(G4Colour::White());
	  FoilLayerLogical_1[iLayer]->SetVisAttributes(visAtt);
	  FoilLayerLogical_2[iLayer]->SetVisAttributes(visAtt);

	  visAtt = new G4VisAttributes(G4Colour::Blue());
	  AbsLayerLogical[iLayer]->SetVisAttributes(visAtt);	  

	  visAtt = new G4VisAttributes(G4Colour::Magenta());
	  SteelSupportLogical[iLayer]->SetVisAttributes(visAtt);

	  visAtt = new G4VisAttributes(G4Colour::Green());
	  AluminiumframeLogical[iLayer]->SetVisAttributes(visAtt);

	  visAtt = new G4VisAttributes(G4Colour::Black());
	  CFmix_LayerLogical[iLayer]->SetVisAttributes(visAtt);

	  visAtt = new G4VisAttributes(G4Colour(0.63, 0.32, 0.18)); /*sienna*/
	  PCBLayerLogical[iLayer]->SetVisAttributes(visAtt);

	}
      
    }
  /*------------------------------------------------------------*/

#ifdef TBHCAL09_DEBUG
  G4cout<<" In BuildHcalElements(), before PlaceHcalElementsIntoLayer"<<G4endl;
#endif
  PlaceHcalElementsIntoLayer();


}
/*================================================================================*/
/*                                                                                */
/*  We have to place the Hcal layers into the world in order to cope              */
/*  with the various configuration (i.e. impact) angles                           */
/*                                                                                */
/*================================================================================*/
G4bool TBhcal09::PlaceHcalLayers(G4LogicalVolume *WorldLog)
{

  G4double inverseCosConfigAngle   = 1.0/cos(config_angle);

  /*coordinates of the middle of the layer*/
  G4double lay_x = x_begin;
  G4double lay_y = y_begin;
  G4double lay_z = z_begin + layer_hthickness[0] * inverseCosConfigAngle;

  G4double steel_support_z = z_begin + steel_support_hthickness * inverseCosConfigAngle;    
  G4double absorber_z      = z_begin + (2*steel_support_hthickness + tungsten_hthickness) * inverseCosConfigAngle;

  G4double scinCassette_z  = z_begin + (2*tungsten_hthickness + 2*steel_support_hthickness + scinCassette_hthickness)*inverseCosConfigAngle;


#ifdef TBHCAL09_DEBUG
  G4cout<<"inverseCosConfigAngle: "<<inverseCosConfigAngle<<G4endl;
  G4cout << setprecision(6);
  G4cout << "x Position of layer: " << lay_x << G4endl;
  G4cout << "y Position of Layer: " << lay_y << G4endl;
  G4cout << "z Position of absorber: " << absorber_z <<"\n"<< G4endl;
  G4cout << "Z position of steel support: " << steel_support_z <<"\n"<< G4endl;
  G4cout <<"inverseCosConfigAngle: "<<inverseCosConfigAngle<<G4endl;
  G4cout << "z Position of scinCassette: " << scinCassette_z <<"\n"<< G4endl;
  G4cout <<"scinCassette_hthickness: "<<scinCassette_hthickness<<endl;
#endif


  /*calculate full HCAL thickness*/
 G4double fullHCALThickness = 0;
  for (G4int iLayer = 0; iLayer < HCAL_n_layers; ++iLayer)
    {
      fullHCALThickness += 2.0 * layer_hthickness[iLayer] * inverseCosConfigAngle;
    }
  /*add terminating absorber plate*/
  fullHCALThickness = fullHCALThickness + 2.0 * tungsten_hthickness * inverseCosConfigAngle;


  G4cout<<"total HCAL thickness: "<<fullHCALThickness<<"\n\n"<<endl;

  /*helpers*/
  G4double deltaSteelSupport = fullHCALThickness/2. - steel_support_hthickness * inverseCosConfigAngle;
  G4double deltaAbsorber     = fullHCALThickness/2. - (2.* steel_support_hthickness + tungsten_hthickness)  * inverseCosConfigAngle;


  G4double deltaScinCassette = fullHCALThickness/2
    - 2*(tungsten_hthickness + steel_support_hthickness) * inverseCosConfigAngle - scinCassette_hthickness*inverseCosConfigAngle;

  G4double delta = fullHCALThickness/2. - layer_hthickness[0] * inverseCosConfigAngle;
 
#ifdef TBHCAL09_DEBUG
  G4cout<<" fullHCALThickness: "<<fullHCALThickness
	<<"  deltaAbsorber: "<<deltaAbsorber<<" deltaSteelSupport: "<< deltaSteelSupport<<" deltaScinCassette: "<< deltaScinCassette   
	<<"\n"<<G4endl;
#endif

  /*----------------------------------------------------------------------------------*/
  G4double xOffsetAbsorber = 0;
  G4double zOffsetAbsorber = 0;

  G4double xOffsetSteelSupport = 0;
  G4double zOffsetSteelSupport = 0;

  G4double xOffsetScinCassette = 0;
  G4double zOffsetScinCassette = 0;

  G4double xOffset = 0;
  G4double zOffset = 0;

   for (G4int iLayer = 0; iLayer < HCAL_n_layers; ++iLayer)
     {
      if (iLayer >= 1)
	{
	  lay_z += layer_hthickness[iLayer - 1] * inverseCosConfigAngle + layer_hthickness[iLayer] * inverseCosConfigAngle;

	  absorber_z      += (tungsten_hthickness 
			      + 2.*(scinCassette_hthickness + steel_support_hthickness) + tungsten_hthickness) * inverseCosConfigAngle;
	  steel_support_z += (steel_support_hthickness 
			      + 2*(tungsten_hthickness +  scinCassette_hthickness) + steel_support_hthickness) * inverseCosConfigAngle;

	  scinCassette_z  += (2*steel_support_hthickness + 2. * tungsten_hthickness + 2. * scinCassette_hthickness ) 
	    * inverseCosConfigAngle;

	  deltaSteelSupport =  deltaSteelSupport - steel_support_hthickness  * inverseCosConfigAngle
	    - (iLayer - 1) * scinCassette_hthickness * inverseCosConfigAngle; 
	  deltaAbsorber     = deltaAbsorber - (2. * steel_support_hthickness + tungsten_hthickness) * inverseCosConfigAngle 
	    - (iLayer - 1) * scinCassette_hthickness * inverseCosConfigAngle;
	  

	  deltaScinCassette = deltaScinCassette - 2. * (tungsten_hthickness +  steel_support_hthickness) * inverseCosConfigAngle  
	    - (iLayer + 1) * scinCassette_hthickness * inverseCosConfigAngle;

	  delta = delta - layer_hthickness[iLayer - 1] * inverseCosConfigAngle - layer_hthickness[iLayer] * inverseCosConfigAngle;
	}
      
       /* put in rotation*/
      xOffsetAbsorber = deltaAbsorber * sin(rotationAngle);
      zOffsetAbsorber = deltaAbsorber - deltaAbsorber * cos(rotationAngle);

      xOffsetSteelSupport = deltaSteelSupport * sin(rotationAngle);
      zOffsetSteelSupport = deltaSteelSupport - deltaSteelSupport * cos(rotationAngle);

      xOffsetScinCassette = deltaScinCassette * sin(rotationAngle);
      zOffsetScinCassette = deltaScinCassette - deltaScinCassette * cos(rotationAngle);

      xOffset = delta * sin(rotationAngle);
      zOffset = delta - delta * cos(rotationAngle);

#ifdef TBHCAL09_DEBUG
       G4cout<<"=========================================layer: "<<(iLayer+1)<<endl;
       G4cout<<" xOffsetAbsorber = "<<xOffsetAbsorber<<G4endl;
       G4cout<<" zOffsetAbsorber = "<<zOffsetAbsorber << G4endl;
       G4cout<<" xOffsetSteelSupport = "<<xOffsetSteelSupport<<G4endl;
       G4cout<<" zOffsetSteelSupport = "<<zOffsetSteelSupport << G4endl;
       G4cout<<" deltaAbsorber =  "<<deltaAbsorber<<G4endl;
       G4cout<<" deltaSteelSupport = "<< deltaSteelSupport<<G4endl;
       G4cout<<" deltaScinCassette = "<< deltaScinCassette<<G4endl;
       G4cout<<" tungsten_hthickness= "<<tungsten_hthickness<<G4endl;
       G4cout<<" 2. * scinCassette_hthickness = "<<2. * scinCassette_hthickness<<G4endl;
       G4cout<<" absorber_z = "<<absorber_z<<G4endl;
       G4cout<<" steel_support_z = " << steel_support_z<<G4endl;
       G4cout<<" scinCassette_z = " << scinCassette_z<<G4endl;
       G4cout<<" layer "<<(iLayer+1)<<" lay_x="<<lay_x<<" lay_y="<<lay_y <<" lay_z="<<lay_z<<G4endl<<G4endl;
       G4cout<<endl;
#endif       

       G4ThreeVector translateHcalAbsorber(    lay_x + xOffsetAbsorber,     lay_y, absorber_z + zOffsetAbsorber);
       G4ThreeVector translateHcalSteelSupport(lay_x + xOffsetSteelSupport, lay_y, steel_support_z + zOffsetSteelSupport);
       G4ThreeVector translateHcalScinCassette(lay_x + xOffsetScinCassette, lay_y, scinCassette_z + zOffsetScinCassette);

       G4ThreeVector translateHCALLayer(lay_x + xOffset, lay_y, lay_z + zOffset);

       //rotation
       G4RotationMatrix* rotation = new G4RotationMatrix();
       rotation->rotateY(config_angle + rotationAngle);
       
       std::stringstream stringForLayerNo;
       stringForLayerNo << (iLayer + 1); 

      /*place layer into logical volume reserved for the HCAL*/

       new G4PVPlacement(rotation,
			 translateHcalAbsorber,
			 AbsLayerLogical[iLayer],
			 G4String("AbsorberLayerPhys") + G4String(stringForLayerNo.str()),
			 WorldLog,
			 0,
			 (iLayer+1),
			 checkForOverlappingVolumes); 
       
       new G4PVPlacement(rotation,
			 translateHcalAbsorber,
			 AluminiumframeLogical[iLayer],
			 G4String("AluminiumframeLayerPhys") + G4String(stringForLayerNo.str()),
			 WorldLog,
			 0,
			 (iLayer+1),
			 checkForOverlappingVolumes); 

       new G4PVPlacement(rotation,
			 translateHcalSteelSupport,
			 SteelSupportLogical[iLayer],
			 G4String("SteelSupportLayerPhys") + G4String(stringForLayerNo.str()),
			 WorldLog,
			 0,
			 (iLayer+1),
			 checkForOverlappingVolumes); 
       
       new G4PVPlacement(rotation,
			 translateHcalScinCassette,
			 WholeScinCassetteLogical[iLayer],
			 G4String("WholeScinCassettePhys") + G4String(stringForLayerNo.str()),
			 WorldLog,
			 0,
			 (iLayer+1),
			 checkForOverlappingVolumes); 
       
     /*----------------------------------------------------
       In case of last layer, add terminating absorber plate
       -----------------------------------------------------
     */
       if ( iLayer == (HCAL_n_layers - 1) )
         {
           lay_z += layer_hthickness[iLayer] * inverseCosConfigAngle + tungsten_hthickness * inverseCosConfigAngle;
           delta = delta - layer_hthickness[iLayer] * inverseCosConfigAngle - tungsten_hthickness * inverseCosConfigAngle;

           xOffset = delta * sin(rotationAngle);
           zOffset = delta - delta * cos(rotationAngle);

           
#ifdef TBHCAL09_DEBUG
           G4cout<<"---------- terminating absorber:"<<G4endl;
           G4cout<<"   delta = "<<delta<<G4endl;
           G4cout<<"   lay_z = "<<lay_z<<G4endl;
           G4cout<<"   lay_x = "<<lay_x<<G4endl;
           G4cout<<" xOffset = "<<xOffset<<G4endl;
           G4cout<<" zOffset = "<<zOffset<<G4endl;
           G4cout<<"\n"<<G4endl;
#endif 
           /*terminating absorber plate*/
           G4ThreeVector translateTermLayer(lay_x + xOffset, lay_y, lay_z + zOffset);
           new G4PVPlacement(rotation,
                             translateTermLayer,
                             AbsLayerLogical_term,
                             "Terminating Absorber",
                             WorldLog,
                             0,
                             1,
                             checkForOverlappingVolumes);
         }
       
     }/*-------------- end loop over HCAL layers ----------------------------------------*/

   return true; 
}


/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
void TBhcal09::SetSD()
{
  /*Birks law and time cut:
    default values: Hcal_apply_Birks_law = 1, Hcal_time_cut = 150. nsec
  */
  G4int Hcal_apply_Birks_law  = _aGeometryEnvironment.GetParameterAsInt("Hcal_apply_Birks_law");
  G4double Hcal_time_cut      = _aGeometryEnvironment.GetParameterAsDouble("Hcal_time_cut");

  G4double zBeginTemp = 0;
  if (Hcal_time_cut > 0) zBeginTemp = z_begin;
  else zBeginTemp = 0;

  /* create SD */
  hcalSD = new TBSD_VCell04("hcalSD",
			    GetGridSize(),
			    HCAL_Layer_ncell_x,
			    HCAL_Layer_ncell_y,
			    GetDepthToLayer(),
			    TBHCAL,
			    Hcal_apply_Birks_law,
			    Hcal_time_cut,
                            zBeginTemp);

  /* register*/
  RegisterSensitiveDetector(hcalSD);


#ifdef USE_ABSORBER
  /*================================================*/
  hcalSD_W = new TBSD_VCell04("hcalSD_W",
			     GetGridSize(),
			     HCAL_Layer_ncell_x,
			     HCAL_Layer_ncell_y,
			     GetDepthToLayer(),
			     TBHCAL,
			     false,
			     false,
			     0);

  /* register*/
  RegisterSensitiveDetector(hcalSD_W);
  /*================================================*/

  /*================================================*/
  hcalSD_FeSupport = new TBSD_VCell04("hcalSD_FeSupport",
			     GetGridSize(),
			     HCAL_Layer_ncell_x,
			     HCAL_Layer_ncell_y,
			     GetDepthToLayer(),
			     TBHCAL,
			     false,
			     false,
			     0);

  /* register*/
  RegisterSensitiveDetector(hcalSD_FeSupport);
  /*================================================*/
  hcalSD_FeCassette1 = new TBSD_VCell04("hcalSD_FeCassette1",
			     GetGridSize(),
			     HCAL_Layer_ncell_x,
			     HCAL_Layer_ncell_y,
			     GetDepthToLayer(),
			     TBHCAL,
			     false,
			     false,
			     0);

  /* register*/
  RegisterSensitiveDetector(hcalSD_FeCassette1);
  /*================================================*/

  hcalSD_FeCassette2 = new TBSD_VCell04("hcalSD_FeCassette2",
			     GetGridSize(),
			     HCAL_Layer_ncell_x,
			     HCAL_Layer_ncell_y,
			     GetDepthToLayer(),
			     TBHCAL,
			     false,
			     false,
			     0);

  /* register*/
  RegisterSensitiveDetector(hcalSD_FeCassette2);
#endif
  /*================================================*/



}


/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
void TBhcal09::Print()
{
  G4cout << "\n  ------> TBhcal09 parameters: <---------------------" << G4endl;
  G4cout<<"  HCAL begins at position ("<<G4BestUnit(x_begin, "Length")
	<<", "<<G4BestUnit(y_begin, "Length")<<", "<<G4BestUnit(z_begin, "Length")<<")"<<G4endl;
  G4cout<<"  HCAL dimensions: x="<<G4BestUnit(cal_hx*2, "Length")
	<<", y="<<G4BestUnit(cal_hy*2, "Length")
	<<", z="<<G4BestUnit(cal_hz*2, "Length")
	<<G4endl;
  G4cout<<"  HCAL placed at z="<<G4BestUnit(z_place, "Length")<<G4endl;
    
  G4cout<<"  Number of HCAL layers:   "<<HCAL_n_layers<<G4endl;
  G4cout<<"  HCAL rotation angle:     "<<G4BestUnit(rotationAngle, "Angle")<<G4endl;
  G4cout<<"  HCAL configuration angle:"<<G4BestUnit(config_angle, "Angle")<<G4endl;
  G4cout<<"  Number of cells in x:    "<<HCAL_Layer_ncell_x<<G4endl;
  G4cout<<"  Number of cells in z:    "<<HCAL_Layer_ncell_y<<G4endl;
  G4cout<<"  HCAL grid size:          "<<HCAL_grid_size<<G4endl;
  G4cout<<"  Scintillator (poly) thickness:          "<<G4BestUnit(poly_hthickness*2,           "Length")<<G4endl;
  G4cout<<"  Air gap thickness:                      "<<G4BestUnit(airgap_hthickness*2,         "Length")<<G4endl;
  G4cout<<"  Steel cassette thickness:               "<<G4BestUnit(steel_cassette_hthickness*2, "Length")<<G4endl;
  G4cout<<"  3M foil thickness:                      "<<G4BestUnit(foil_hthickness*2,           "Length")<<G4endl;
  G4cout<<"  PCB plate thickness:                    "<<G4BestUnit(pcb_hthickness*2,            "Length")<<G4endl;
  G4cout<<"  Cable-fibre mix thickness:              "<<G4BestUnit(cablefibre_mix_hthickness*2, "Length")<<G4endl;
  G4cout<<"  steel_support_hthickness:               "<<G4BestUnit(steel_support_hthickness*2,  "Length")<<G4endl;

  G4cout<<"\n HCAL_n_layers: "<<HCAL_n_layers<<G4endl;
  G4cout << resetiosflags(ios::left);
  G4cout<<"  Layer" <<" Absorber thickness"<<" Layer thickness"<<G4endl;

  G4cout << "       TBHcal 08 materials          " << G4endl;  
  G4cout << "      PCB_density		     " << G4BestUnit(PCB_density,"Volumic Mass") << G4endl;
  G4cout << "      PCB_silicon_fractiomass       " << PCB_silicon_fractiomass << G4endl; 
  G4cout << "      PCB_elO_fractionmass	     " << PCB_elO_fractionmass << G4endl; 
  G4cout << "      PCB_graphite_fractionmass     " << PCB_graphite_fractionmass << G4endl; 
  G4cout << "      PCB_elH_fractionmass	     " << PCB_elH_fractionmass << G4endl; 
  G4cout << "      PCB_elBr_fractionmass	     " << PCB_elBr_fractionmass << G4endl; 
  G4cout << "      S235_density		     " <<  G4BestUnit(S235_density,"Volumic Mass")<< G4endl; 
  G4cout << "      S235_iron_fractionmass        " << S235_iron_fractionmass << G4endl; 
  G4cout << "      S235_graphite_fractionmass    " << S235_graphite_fractionmass << G4endl; 
  G4cout << "      S235_manganese_fractionmass   " << S235_manganese_fractionmass << G4endl; 
  G4cout << "      tungsten_density		     " << G4BestUnit(tungstenalloy_density,"Volumic Mass") << G4endl; 
  G4cout << "      tungsten_thickness		     " << G4BestUnit(tungsten_hthickness*2,"Length") << G4endl; 
  G4cout << "      tungsten_core_density	     " << G4BestUnit(coretun_density,"Volumic Mass") << G4endl; 
  G4cout << "      nikel_density	     " << G4BestUnit(nikel_density,"Volumic Mass") << G4endl; 
  G4cout << "      tungsten_core_tungsten_fractionmass	" << tungsten_fractionmass << G4endl; 
  G4cout << "      tungsten_nikel_fractionmass   " << nikel_fractionmass << G4endl; 
  G4cout << "      tungsten_copper_fractionmass  " << copper_fractionmass << G4endl; 
  G4cout << "      PVC_density		     " <<  G4BestUnit(PVC_density,"Volumic Mass") << G4endl; 
  G4cout << "      Polystyrole_density	     " << G4BestUnit(Polystyrole_density,"Volumic Mass") << G4endl; 
  G4cout << "      CF_Mix_density	             " << G4BestUnit(CF_Mix_density,"Volumic Mass") << G4endl; 
  G4cout << "      CF_MIX_air_fractionmass       " << CF_MIX_air_fractionmass << G4endl; 
  G4cout << "      CF_MIX_PVC_fractionmass       " << CF_MIX_PVC_fractionmass << G4endl; 
  G4cout << "      CF_MIX_Polystyrole_fractionmass " << CF_MIX_Polystyrole_fractionmass << G4endl;
  

   G4cout<<"\n OCTAGON"<<G4endl;
   G4cout<<" inner radius 0: "<<Octagonal_absorber_inner_radius[0]<<" 1: "<<Octagonal_absorber_inner_radius[1]<<G4endl;
   G4cout<<" outer radius 0: "<<Octagonal_absorber_outer_radius[0]<<" 1: "<<Octagonal_absorber_outer_radius[1]<<G4endl;
   G4cout<<" z 0: "<<Octagonal_absorber_z[0]<<" 1: "<<Octagonal_absorber_z[1]<<G4endl;
   G4cout<<" number of sides: "<<Octagonal_absorber_number_of_sides <<G4endl;
   G4cout<<" number of layers: "<<Octagonal_absorber_number_of_layers<<G4endl;


  for (G4int iLayer = 0; iLayer < HCAL_n_layers; ++iLayer)
    {
      G4cout<<setw(4)<<(iLayer+1)<<" "
	    <<setw(7)<<G4BestUnit(tungsten_hthickness*2, "Length")<<" "
	    <<setw(15)<<G4BestUnit(layer_hthickness[iLayer]*2, "Length")<<G4endl;
    }

  G4String layers, pattern;


  G4cout<<"  ----------------------------------------------------------"<<G4endl;
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
void TBhcal09::DefineHcalMaterials() {
  G4Element* elH  = CGAGeometryManager::GetElement("H", true);  /*Hydrogen */
  G4Element* elC  = CGAGeometryManager::GetElement("C", true);  /*Carbon */
  G4Element* elO  = CGAGeometryManager::GetElement("O", true);  /*Oxygen */
  G4Element* elCl = CGAGeometryManager::GetElement("Cl", true); /*Chlorine */
  G4Element* elBr = CGAGeometryManager::GetElement("Br", true); /*Bromine */

  G4String name; 
  G4int nel,natoms;

  /* PCB (Printed Circuit Board) Material FR4
     Composition and density found under 
     http://pcba10.ba.infn.it/temp/ddd/ma/materials/list.html 
  */

  PCB = new G4Material(name="PCB", PCB_density, nel=5);
  PCB->AddMaterial(CGAGeometryManager::GetMaterial("silicon_2.33gccm"),PCB_silicon_fractiomass);
  PCB->AddElement(elO, PCB_elO_fractionmass);
  PCB->AddMaterial(CGAGeometryManager::GetMaterial("graphite"),PCB_graphite_fractionmass);
  PCB->AddElement(elH, PCB_elH_fractionmass);
  PCB->AddElement(elBr, PCB_elBr_fractionmass);

  /*The steel we are going to use in the Hcal: Material S235JR (old name St37)
    Numbers found under 
    http://n.ethz.ch/student/zwickers/ download/fs_pe_grundlagen_cyrill.pdf 
  */

  S235 = new G4Material(name="S235", S235_density, nel=3);
  S235->AddMaterial(CGAGeometryManager::GetMaterial("iron"), S235_iron_fractionmass);
  S235->AddMaterial(CGAGeometryManager::GetMaterial("graphite"), S235_graphite_fractionmass);
  S235->AddMaterial(CGAGeometryManager::GetMaterial("manganese"), S235_manganese_fractionmass);


  //Tungsten is built with density 17.84*g/cm3 and it has 5.25% of nikel, 1.76% of copper and 92.99% of 19.3gr/cm3 tungsten*/  
  G4double a, z; 
  double newtungstenalloy_density = 17.84*g/cm3;			
  nikel          = new G4Material(name="nikel",   z=28.,  a=58.71*g/mole,  nikel_density);
  tungstenalloy  = new G4Material(name="tungstenalloy", newtungstenalloy_density, nel=3);  
  tungstenalloy->AddMaterial(CGAGeometryManager::GetMaterial("copper"),copper_fractionmass);
  tungstenalloy->AddMaterial(CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),tungsten_fractionmass);
  tungstenalloy->AddMaterial(nikel,  nikel_fractionmass); 

  //PVC
  //Numbers from http://www.elpac.de/Kunststoffkleinteile/Kleines_Kunststoff-Know-_How/PVC-P/pvc-p.html
  G4Material* PVC = new G4Material(name="PCB", PVC_density, nel=3);
  PVC->AddElement(elH, natoms=3);
  PVC->AddElement(elC, natoms=2);
  PVC->AddElement(elCl, natoms=1);
  
  //Polystyrole
  /*    Numbers from http://de.wikipedia.org/wiki/Polystyrol
	the structural formula for the Styrene Polymer is C6H5CH=CH2
	The difference to Styropor definition in CGAGeometryManager
	comes since we do not have the material in a foamed form*/
  Polystyrole = new G4Material(name="Polystyrole", Polystyrole_density, nel=2);
  Polystyrole->AddElement(elH, natoms=8);
  Polystyrole->AddElement(elC, natoms=8);

  //CFMix
  /*Now we define the material cf_mix
    We assume the following:
    a) a layer has a volume of V_total = 90x90x0.15 cm^3 = 1215 cm^3(last number is
       longitudinal space reserved for cable fibre mix)
    b) coax cable has diameter of 0.12 cm 
    fibre has diameter of 0.5 cm
    The cables are on average 45 cm long 
    => V_coax = PI*(0.06 cm)^2*45 cm = 0.510 cm^3
    => V_fibre = PI*(0.025 cm)^2*45 cm = 0.088 cm^3
    ...The rest is occupied by air 
    V_air = V_total - V_coax - V_fibre = 1214.402
    There is one coax. cable and one fibre per tile
    and we have on average 185 tiles per layer
    => Total mass of coax cable (fibre), m = density*V
    m_coax = (1.35*0.510)*185 = 127.37 g
    m_fibre = (1.065*0.088)*185 = 17.33 g
    ... and
    m_air = 1214.402*1.29e-03 = 1.45 g
    total density = (m_air + m_coax + m_fibre)/1215. = 0.120 g/cm^3 
  */
  
  CF_MIX = new G4Material(name="Cable Fibre Mix",CF_Mix_density, nel=3);
  CF_MIX->AddMaterial(CGAGeometryManager::GetMaterial("air"), CF_MIX_air_fractionmass);
  CF_MIX->AddMaterial(PVC, CF_MIX_PVC_fractionmass);
  CF_MIX->AddMaterial(Polystyrole, CF_MIX_Polystyrole_fractionmass);

  /*materials*/
  poly      = CGAGeometryManager::GetMaterial("polystyrene");
  air       = CGAGeometryManager::GetMaterial("air");
  aluminium = CGAGeometryManager::GetMaterial("Aluminium");

  G4cout<<"\n  -----------> TBhcal09 material properties <----------------"<<G4endl;
  G4cout<<"  TungstenALLOY: X0 = "<<tungstenalloy->GetRadlen() /cm <<" cm"
	<<",  Lambda_I ="<<tungstenalloy->GetNuclearInterLength()/cm<<" cm"
	<<G4endl;
  G4cout<<"  Tungsten19.3: X0 = "<<CGAGeometryManager::GetMaterial("tungsten_19.3gccm")->GetRadlen() /cm <<" cm"
	<<",  Lambda_I ="<<CGAGeometryManager::GetMaterial("tungsten_19.3gccm")->GetNuclearInterLength()/cm<<" cm"
	<<", density = "<<G4BestUnit(CGAGeometryManager::GetMaterial("tungsten_19.3gccm")->GetDensity(), "Volumic Mass")
	<<G4endl;
  
  G4cout<<"  Nickel: X0 = "<<nikel->GetRadlen() /cm <<" cm"
	<<",  Lambda_I ="<<nikel->GetNuclearInterLength()/cm<<" cm"
	<<", density = "<<G4BestUnit(nikel->GetDensity(), "Volumic Mass")
	<<G4endl;

  G4cout<<"  Copper: X0 = "<<CGAGeometryManager::GetMaterial("copper")->GetRadlen() /cm <<" cm"
	<<",  Lambda_I ="<<CGAGeometryManager::GetMaterial("copper")->GetNuclearInterLength()/cm<<" cm"
	<<", density = "<<G4BestUnit(CGAGeometryManager::GetMaterial("copper")->GetDensity(), "Volumic Mass")
	<<"\n"<<G4endl;


  G4cout<<"  Scintillator (polystyrene): X0 = "<<poly->GetRadlen() /cm <<" cm"
	<<",  Lambda_I ="<<poly->GetNuclearInterLength()/cm<<" cm"
	<<G4endl;
  G4cout<<"  Steel (ST235):              X0 = "<<S235->GetRadlen()/cm <<" cm"
	<<",  Lambda_I ="<<S235->GetNuclearInterLength()/cm<<" cm"
	<<G4endl;
  G4cout<<"  Air:                        X0 = "<<air->GetRadlen()/cm <<" cm"
	<<",  Lambda_I = "<<air->GetNuclearInterLength()/cm<<" cm"
	<<G4endl;
  G4cout<<"  Cable-fibre mix:            X0 = "<<CF_MIX->GetRadlen()/cm <<" cm"
	<<",  Lambda_I = "<<CF_MIX->GetNuclearInterLength()/cm<<" cm"
	<<G4endl;
   G4cout<<"  PCB material:               X0 = "<<PCB->GetRadlen() /cm <<" cm"
	<<",  Lambda_I = "<<PCB->GetNuclearInterLength()/cm<<" cm"
	<<G4endl;
  G4cout<<"  3M foils (polystyrole):     X0 = "<<Polystyrole->GetRadlen()/cm <<" cm"
	<<",  Lambda_I = "<<Polystyrole->GetNuclearInterLength()/cm<<" cm"
	<<G4endl;
 
  G4cout<<"  ----------------------------------------------------------\n"<<G4endl;

}


/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
void TBhcal09::PlaceHcalElementsIntoLayer() 
{
  
  G4double steelCassettePosition1[MAX_TBHCAL_LAYERS] = {0.};
  G4double steelCassettePosition2[MAX_TBHCAL_LAYERS] = {0.};
  G4double cableFibreMixPosition[MAX_TBHCAL_LAYERS]  = {0.};
  G4double pcbPosition[MAX_TBHCAL_LAYERS]            = {0.};
  G4double foil3MPosition1[MAX_TBHCAL_LAYERS]        = {0.};
  G4double foil3MPosition2[MAX_TBHCAL_LAYERS]        = {0.};
  G4double scintillatorPosition[MAX_TBHCAL_LAYERS]   = {0.};
 
  for (G4int iLayer = 0; iLayer < HCAL_n_layers; ++iLayer)
    {

     steelCassettePosition1[iLayer] = - scinCassette_hthickness + 2.0 * airgap_hthickness 
       + steel_cassette_hthickness;
     
      new G4PVPlacement(0,
			G4ThreeVector(0, 0, steelCassettePosition1[iLayer]),
			ScinHousLogical[iLayer],
			"HcalScinHousPhys Front",
			WholeScinCassetteLogical[iLayer],                      
			0,
			0,
			checkForOverlappingVolumes);

      /*------Put the cable-fibre mixture into the layer ------*/
      cableFibreMixPosition[iLayer] = steelCassettePosition1[iLayer] 
	+ steel_cassette_hthickness + cablefibre_mix_hthickness;

      new G4PVPlacement(0,
			G4ThreeVector(0,0, cableFibreMixPosition[iLayer]),
			CFmix_LayerLogical[iLayer],
			"HcalCFmix_LayerPhys",
			WholeScinCassetteLogical[iLayer],                                
			0,
			0,
			checkForOverlappingVolumes);

      /*---- Put the PCB into the layer ----*/
      pcbPosition[iLayer] = cableFibreMixPosition[iLayer] + cablefibre_mix_hthickness + pcb_hthickness;

      new G4PVPlacement(0,
			G4ThreeVector(0, 0, pcbPosition[iLayer]),
			PCBLayerLogical[iLayer],
			"HcalSensLayerPhys",
			WholeScinCassetteLogical[iLayer],                                
			0,
			0,
			checkForOverlappingVolumes);
      
      /*---- Put first 3M foil Layer into the complete layer ----*/
      foil3MPosition1[iLayer] = pcbPosition[iLayer] + pcb_hthickness + foil_hthickness; 
      new G4PVPlacement(0,
			G4ThreeVector(0, 0,foil3MPosition1[iLayer]),
			FoilLayerLogical_1[iLayer],
			"HcalSensLayerPhys",
			WholeScinCassetteLogical[iLayer],                                
			0,
			0,
			checkForOverlappingVolumes);

      /*--- Put sensitive part (i.e. scintillator plate) into the layer ---*/
      std::stringstream stringForLayerNo;
      stringForLayerNo << (iLayer + 1); 
      
      scintillatorPosition[iLayer] = foil3MPosition1[iLayer] + foil_hthickness + poly_hthickness; 
      new G4PVPlacement(0,
			G4ThreeVector(0, 0, scintillatorPosition[iLayer]),
			WholeSensLayerLogical[iLayer],
			G4String("HcalSensLayerPhys") + G4String(stringForLayerNo.str()),
			WholeScinCassetteLogical[iLayer],                                
			0,
			0,
			checkForOverlappingVolumes);
      
      /*---- Put 3M foil into the layer ----*/
      foil3MPosition2[iLayer] = scintillatorPosition[iLayer] + poly_hthickness + foil_hthickness; 
      new G4PVPlacement(0,
			G4ThreeVector(0 ,0, foil3MPosition2[iLayer]),
			FoilLayerLogical_2[iLayer],
			"HcalSensLayerPhys",
			WholeScinCassetteLogical[iLayer],                                
			0,
			0,
			checkForOverlappingVolumes);
      

      /*---- Put scintillator housing rear plate into the layer ----*/
      steelCassettePosition2[iLayer] = foil3MPosition2[iLayer] + foil_hthickness + steel_cassette_hthickness;
      new G4PVPlacement(0,
			G4ThreeVector(0, 0, steelCassettePosition2[iLayer]),
			ScinHousLogical2[iLayer],
			"HcalScinHousPhys Rear",
			WholeScinCassetteLogical[iLayer],                                
			0,
			1,
			checkForOverlappingVolumes);

    }/*end loop over HCAL layers*/

  
#ifdef TBHCAL09_DEBUG
  G4cout<<" \n\n TBhcal09::PlaceHcalElementsIntoLayer() "<<G4endl;
  for (G4int iLayer = 0; iLayer < HCAL_n_layers; ++iLayer)
    {
      G4cout<<"  Layer "<<(iLayer+1)<<G4endl;
      G4cout << "       first steel cassette at:     " << steelCassettePosition1[iLayer] << " mm" << G4endl;
      G4cout << "       cable-fibre mixture at:      " << cableFibreMixPosition[iLayer] << " mm" << G4endl;
      G4cout << "       PCB at:                      " << pcbPosition[iLayer] << " mm" << G4endl;
      G4cout << "       3M foil 1 at:                " << foil3MPosition1[iLayer] << " mm" << G4endl;
      G4cout << "       scintillator at:             " << scintillatorPosition[iLayer] << " mm" << G4endl;
      G4cout << "       3M foil 2 at:                " << foil3MPosition2[iLayer] << " mm" << G4endl;
      G4cout << "       second steel cassette at:    " << steelCassettePosition2[iLayer] << " mm" << G4endl;
    
    }
 
    
#endif
}

