/* Author Roman Poeschl DESY
   Routines adapted from Jeremy McCormick, NIU
   Updated for CERN test beam geometry by Oliver Wendt DESY
*/

#include "Control.hh"
#include "TBhcal07.hh"

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


#ifdef MOKKA_GEAR
#include "gear/CalorimeterParameters.h" 
#include "gearimpl/CalorimeterParametersImpl.h" 
#include "gearimpl/LayerLayoutImpl.h" 
#include "MokkaGear.h"
#endif

#include <sstream>


//#define TBHCAL07_DEBUG 1

/* define some levels of detail for graphical display*/
#define DM_FULL 3
#define DM_ABSORBERANDSENSITIVE 2
#define DM_WHOLELAYERONLY  1



INSTANTIATE(TBhcal07)
/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
  TBhcal07::TBhcal07() : VSubDetectorDriver("TBhcal07","TBhcal"),
			 db(0),
			 _aGeometryEnvironment("","",NULL, NULL),
			 config_angle(0)  
{
  checkForOverlappingVolumes = false;
#ifdef TBHCAL07_DEBUG
  checkForOverlappingVolumes = true;
#endif
}
/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
TBhcal07::~TBhcal07()
{}
/*================================================================================*/
/*                                                                                */
/*     Main function                                                              */
/*                                                                                */
/*================================================================================*/
G4bool TBhcal07::ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
				     G4LogicalVolume *WorldLog)
{
  G4cout<<"\n ***************************************************************"<<G4endl;
  G4cout<<" *                                                             *"<<G4endl;
  G4cout<<" *    Build HCAL based on the TBhcal07 driver                  *"<<G4endl;
  G4cout<<" *                                                             *"<<G4endl;
  G4cout<<" ***************************************************************"<<G4endl;

  /*Obtain the pointer to our database via the Environment object*/
  _aGeometryEnvironment = aGeometryEnvironment;
  db = new Database(_aGeometryEnvironment.GetDBName());

  WorldLogical = WorldLog; 
  
  FetchMySQLVariables();
  DefineHcalMaterials();
  BuildHcalElements();
  Print();

  /*-------------------------------------------------------------------
    GEAR information
   */
  G4double innerRadius = 0;
  G4double outerRadius = cal_hx ;//* sqrt(2);
  //G4double zMax        = cal_hz; /*cal_hz is calculated in BuildHcalElements()*/
  G4double leastZ      = z_begin;
  G4int symmetryOrder  = 4;      /*this is a standalone prototype*/
  G4double phi         = 0;
  gear::CalorimeterParametersImpl *gearParam = 
    new gear::CalorimeterParametersImpl(innerRadius, outerRadius, leastZ, symmetryOrder, phi);

  for (G4int iLayer = 0; iLayer < n_layers; ++iLayer)
    {
      G4double distance = 0; /*distance of this layer from the origin*/
      G4double layerThickness = (steel_hthickness[iLayer] + 2.0*airgap_hthickness + 2.0*steel_cassette_hthickness  
                                + cablefibre_mix_hthickness + pcb_hthickness + 2.0*foil_hthickness + poly_hthickness) * 2.;
      G4double cellSize0 = 30. * mm; /*cell size along the x-axis*/
      G4double cellSize1 = 30. * mm; /*cell size along the y-axis*/
      G4double absorberThickness = steel_hthickness[iLayer] * 2.;

      gearParam->layerLayout().positionLayer(distance, layerThickness, cellSize0, cellSize1, absorberThickness);
      
    }/*end loop over iLayer*/

  /* write parameters to GearManager*/
  MokkaGear* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setHcalEndcapParameters( gearParam ) ;  
  /*-------------------------------------------------------------------*/


  G4bool cokay = PlaceHcalLayers(WorldLogical);
  
  delete db;
  db = 0;
  
  G4cout << "\nDone building TBhcal07" << G4endl;
  
  return cokay;
}
/*================================================================================*/
/*                                                                                */
/*       Fetch HCAL related MySQL variables                                       */
/*                                                                                */
/*================================================================================*/
void TBhcal07::FetchMySQLVariables()
{

  /* config angle from environment object
     config_angle: angle reflecting the rotations for non normal incidence.*/
  config_angle = _aGeometryEnvironment.GetParameterAsDouble ("configuration_angle");
  config_angle = config_angle*deg;

  /*we are dealing with a simple cell grid 
    so we only need to know the number of layers,
    the number of cells in x and z direction and the
    basic grid size to get the layering n 
  */
  db->exec("select * from hcal_virt;");
  db->getTuple();
  n_layers = db->fetchInt("n_layers");
  //n_layers = 5;
  assert(n_layers > 0);

  /*number of cells in x*/
  ncell_xy[0] = db->fetchInt("ncell_x");

  /*number of cells in z*/
  ncell_xy[1] = db->fetchInt("ncell_y");
  assert(ncell_xy[0] >= 0 || ncell_xy[1] >= 0);
  /*grid size*/
  grid_size = db->fetchDouble("grid_size");

  /*the beginning of the Hcal */
  x_begin = db->fetchDouble("x_begin");
  y_begin = db->fetchDouble("y_begin");
  z_begin = db->fetchDouble("z_begin");
  
  /* get 'global' x and y translation from steering file (at run time) or database*/
  G4double XTranslation = _aGeometryEnvironment.GetParameterAsDouble("HcalTranslateX");
  G4double YTranslation = _aGeometryEnvironment.GetParameterAsDouble("HcalTranslateY");

  x_begin += XTranslation;
  y_begin += YTranslation;

  /* get rotation angle from steering file (at run time) or database*/
  rotationAngle = _aGeometryEnvironment.GetParameterAsDouble("HcalRotationAngle");
  rotationAngle = rotationAngle*deg;
  
  /* thicknesses*/
  db->exec("select * from hcal_layer_thickness;");
  db->getTuple();

  poly_hthickness    = db->fetchDouble("poly_thickness")/2;
  /*steel_hthickness = db->fetchDouble("steel_thickness")/2;*/
  /*airgap_hthickness         = db->fetchDouble("air_gap")/2;*/
  airgap_hthickness =  1.25/2.;

  steel_cassette_hthickness = db->fetchDouble("steel_cassette_thickness")/2.;
  foil_hthickness           = db->fetchDouble("foil_thickness")/2.;
  pcb_hthickness            = db->fetchDouble("pcb_thickness")/2.;
  cablefibre_mix_hthickness = db->fetchDouble("cablefibre_mix_thickness")/2.;

  /*Fetch data for terminating absorber of Hcal*/
  db->exec("select * from hcal_termlayer;");
  db->getTuple();

  //steel_hthickness_term = db->fetchDouble("steel_thickness_term")/2;
  steel_hthickness_term = 20.5*mm/2.;

  /*--------------------------
    scintillator cassette + contents + air gaps:
   -----------------------------------*/
  scinCassette_hthickness =  2.0*airgap_hthickness /*2 times air gap*/
    + 2.0*steel_cassette_hthickness                /*2 times steel cassette*/
    + cablefibre_mix_hthickness                    /*cable-fibre mix*/
    + pcb_hthickness                               /*PCB*/
    + 2.0*foil_hthickness                          /*2 times 3M foil*/
    + poly_hthickness;                             /*scintillator*/



  /*---------------------------------------
    Introduce a string of 1 and 0's to deal with 2006
    models, in which HCAL layers were only partially
    equipped.
  ---------------------------------------*/
  /* set up vector for the sensitive layer pattern*/
  G4String sensitiveLayerPatternFromSteeringFile = _aGeometryEnvironment.GetParameterAsString("Hcal_layer_pattern");
  
  /* set up default layer pattern with the lenghth 'n_layers'*/
  G4String sensitiveLayerPatternDefault("");
  for (G4int i = 0; i < n_layers; ++i) sensitiveLayerPatternDefault += '1';


  G4cout<<"\n\n\n PATTERN length = "<<sensitiveLayerPatternFromSteeringFile.length()
	<<" , "<<sensitiveLayerPatternFromSteeringFile<<G4endl;


  /*---------------
    if steering command:
    /Mokka/init/globalModelParameter Hcal_layer_pattern 11111111111111111111111111111101010101
    exists in steering file:
    set sensitiveLayerPatternVector to that value
    ----------*/
  if ( ((G4int)sensitiveLayerPatternFromSteeringFile.length()) != 0 ) 
    {
      if (isValidSensitiveLayerPattern(sensitiveLayerPatternFromSteeringFile)) 
	{
	  sensitiveLayerPatternVector = getSensitiveLayerPatternVector(sensitiveLayerPatternFromSteeringFile);
	}
      else 
	{
	  G4cout << "'/Mokka/init/HCALLayerPattern' parameter is not valid in steering file. ABORTING MOKKA" << G4endl;
	  exit(1);
	}
    }
  else /*take the sensitive layer pattern from the database*/
    {
      Database* db2 = new Database(Control::MODELS_DBNAME.data());
      db2->exec("select * from parameters where name='Hcal_layer_pattern';");
      db2->getTuple();

      G4String sensitiveLayerPatternFromDatabase("");
      sensitiveLayerPatternFromDatabase = db2->fetchString("default_value");
      
      if ( ((G4int)sensitiveLayerPatternFromDatabase.length()) != 0 ) 
	{
	  if (isValidSensitiveLayerPattern(sensitiveLayerPatternFromDatabase)) 
	    {
	      sensitiveLayerPatternVector = getSensitiveLayerPatternVector(sensitiveLayerPatternFromDatabase);
	    }
	  else 
	    {
	      G4cout << "No valid layer pattern for the HCAL prototype found in database 'parameter'"
		     <<" and no steering parameter '/Mokka/init/HCALLayerPattern' set." 
		     << G4endl
		     << "Using a fully equipped HCAL as a default" << G4endl;
	
	      sensitiveLayerPatternVector = getSensitiveLayerPatternVector(sensitiveLayerPatternDefault);
	    }
	}
      else 
	{
	  G4cout << "No valid layer pattern for the HCAL prototype found in database 'parameter' "
		 <<" and no steering parameter '/Mokka/init/HCALLayerPattern' set." 
		 << G4endl
		 << "Using a fully equipped HCAL as a default" << G4endl;
	
	  sensitiveLayerPatternVector = getSensitiveLayerPatternVector(sensitiveLayerPatternDefault);
	}
      
      delete db2;
      db2 = 0;
    }

}

/*================================================================================*/
/*                                                                                */
/*  Set variable absorber thicknesses for each layer                              */
/*                                                                                */
/*================================================================================*/
void TBhcal07::SetHcalAbsorberThickness()
{
  G4double temp[MAX_TBHCAL_LAYERS] = {17.4,  /*layer 1   */
				      17.4,  /*layer 2   */
				      17.4,  /*layer 3   */
				      17.6,  /*layer 4   */
				      17.4,  /*layer 5   */
				      17.6,  /*layer 6   */
				      17.6,  /*layer 7   */
				      17.4,  /*layer 8   */
				      16.7,  /*layer 9   */
				      17.4,  /*layer 10  */
				      17.4,  /*layer 11  */
				      17.4,  /*layer 12  */
				      17.4,  /*layer 13  */
				      17.4,  /*layer 14  */
				      17.4,  /*layer 15  */
				      17.4,  /*layer 16  */
				      17.4,  /*layer 17  */
				      17.6,  /*layer 18  */
 				      17.4,  /*layer 19  */
 				      17.4,  /*layer 20  */
 				      17.4,  /*layer 21  */
 				      17.4,  /*layer 22  */
 				      17.6,  /*layer 23  */
 				      17.6,  /*layer 24  */
 				      17.4,  /*layer 25  */
 				      17.4,  /*layer 26  */
 				      17.6,  /*layer 27  */
 				      16.7,  /*layer 28  */
 				      17.4,  /*layer 29  */
 				      16.7,  /*layer 30  */
 				      17.4,  /*layer 31  */
 				      17.6,  /*layer 32  */
 				      17.6,  /*layer 33  */
 				      17.6,  /*layer 34  */
 				      17.6,  /*layer 35  */
 				      17.6,  /*layer 36  */
 				      17.6,  /*layer 37  */
 				      17.6  /*layer 38  */
  };

  for (G4int iLayer = 1; iLayer <= n_layers; ++iLayer)
    {
	steel_hthickness[iLayer-1] = temp[iLayer - 1]/2. /mm;
    }
}

/*================================================================================*/
/*                                                                                */
/*  Define calculated variables, materials, solids, logical volumes               */
/*                                                                                */
/*================================================================================*/
void TBhcal07::BuildHcalElements()
{
  /*Information needed when hits are processed later on*/
  /*Must be set before calling the constructor of the sensitive detector,
   which is done in SetSD()!*/
  SetDepthToLayer(1);
  /* create and register the sensitive detector before
     defining the sensitive logical volumes!
  */
  SetSD();

  /*set absorber thickness */
  this->SetHcalAbsorberThickness();


  /*calorimeter dimensions*/
  cal_hz = 0;
  cal_hx = (G4double) (ncell_xy[0] * grid_size*mm)/2.;
  cal_hy = (G4double) (ncell_xy[1] * grid_size*mm)/2.;

  std::stringstream stringForLayerNo; /*string to save layer number*/

  G4Box *WholeLayerSolid[MAX_TBHCAL_LAYERS] = {NULL};

  G4Box *WholeScinCassetteSolid[MAX_TBHCAL_LAYERS] = {NULL};

  G4Box *AbsLayerSolid[MAX_TBHCAL_LAYERS] = {NULL};

  /*the scintillator housing (made of S235)*/
  G4Box *ScinHousSolid               = new G4Box("HcalScinHousSolid", cal_hx, cal_hy, steel_cassette_hthickness);

  /*the cable-fibre mixture */
  G4Box *CFmix_LayerSolid = new G4Box("HcalCFmix_LayerSolid", cal_hx, cal_hy, cablefibre_mix_hthickness);

    /*a pcb layer*/
  G4Box *PCBLayerSolid = new G4Box("HcalPCBLayerSolid",cal_hx, cal_hy, pcb_hthickness);

  /*3M foil*/
  G4Box *FoilLayerSolid = new G4Box("Hcal3MFoilSolid", cal_hx, cal_hy, foil_hthickness);

  /* Scintillator*/
  G4Box *WholeSensLayerSolid = new G4Box("HcalWholeSensLayerSolid", cal_hx, cal_hy, poly_hthickness);

 
  /*second full layer, but this time with an air gap instead of sensor casette*/
  G4Box *WholeLayer2Solid[MAX_TBHCAL_LAYERS] = {NULL};

  for (G4int i = 0; i < MAX_TBHCAL_LAYERS; ++i)
    {
      WholeLayerLogical[i]         = NULL;
      WholeScinCassetteLogical[i]  = NULL;
      AbsLayerLogical[i]           = NULL;
      ScinHousLogical[i]           = NULL;
      CFmix_LayerLogical[i]        = NULL;
      PCBLayerLogical[i]           = NULL;
      FoilLayerLogical_1[i]        = NULL;
      FoilLayerLogical_2[i]        = NULL;
      WholeSensLayerLogical[i]     = NULL;
    }

  /*----------------------------------------------------------------------------*/
  for (G4int iLayer = 0; iLayer < n_layers; ++iLayer)
    {
      /* HCAL layer thickness */
      layer_hthickness[iLayer] = poly_hthickness       /*scintillator*/
	+ steel_hthickness[iLayer]                     /*absorber*/
	+ 2.0*airgap_hthickness                        /*2 times air gap*/
	+ 2.0*steel_cassette_hthickness                /*2 times steel cassette*/
	+ 2.0*foil_hthickness                          /*2 times 3M foil*/
	+ pcb_hthickness                               /*PCB*/
	+ cablefibre_mix_hthickness;                   /*cable-fibre mix*/

      cal_hz += layer_hthickness[iLayer];

      stringForLayerNo << (iLayer+1);

      /*create a layer filled with air, with enough space for all its constituents*/
      WholeLayerSolid[iLayer]   = new G4Box("HcalLayerSolid", cal_hx, cal_hy, layer_hthickness[iLayer]);
      WholeLayerLogical[iLayer] = new G4LogicalVolume(WholeLayerSolid[iLayer], air, 
						      G4String("HcalLayerLogical") + G4String(stringForLayerNo.str()), 0, 0, 0);

      /*second full layer, but this time with an air gap instead of sensor casette*/
      WholeLayer2Solid[iLayer] = new G4Box("WholeLayer2Solid", cal_hx, cal_hy, layer_hthickness[iLayer]);
      WholeLayer2Logical[iLayer] = new G4LogicalVolume(WholeLayer2Solid[iLayer], air,
						       G4String("HcalLayer2Logical") + G4String(stringForLayerNo.str()), 0, 0, 0);

      /*create a whole for: 
       air + steel cassette + PCB foil + 3M foil + scintillator + 3M foil + steel cassette + air*/
      WholeScinCassetteSolid[iLayer]   = new G4Box("WholeScinCassetteSolid", cal_hx, cal_hy, scinCassette_hthickness);
      WholeScinCassetteLogical[iLayer] = new G4LogicalVolume(WholeScinCassetteSolid[iLayer], air,
							     G4String("WholeScinCassetteLogicalLogical") + G4String(stringForLayerNo.str()), 0, 0, 0);



      /*create an absorber plate */
      AbsLayerSolid[iLayer]   = new G4Box("HcalAbsLayerSolid", cal_hx, cal_hy, steel_hthickness[iLayer]);
      AbsLayerLogical[iLayer] = new G4LogicalVolume(AbsLayerSolid[iLayer], S235, 
						    G4String("HcalAbsLayerLogical") + G4String(stringForLayerNo.str()), 0, 0, 0);

      /*the scintillator housing (made of S235)*/
      ScinHousLogical[iLayer] = new G4LogicalVolume(ScinHousSolid, S235, 
						    G4String("HcalScinHouseLogical") + G4String(stringForLayerNo.str()), 0, 0, 0);

      /*the cable-fibre mixture */
      CFmix_LayerLogical[iLayer] = new G4LogicalVolume(CFmix_LayerSolid, CF_MIX, 
						       G4String("CFmix_LayerLogical") + G4String(stringForLayerNo.str()), 0, 0, 0);
      
      /*a pcb layer*/
      PCBLayerLogical[iLayer] = new G4LogicalVolume(PCBLayerSolid, PCB, G4String("HcalPCBLayerLogical") + G4String(stringForLayerNo.str()), 0, 0, 0);


      /*3M foil*/
      FoilLayerLogical_1[iLayer] = new G4LogicalVolume(FoilLayerSolid, Polystyrole,
						       G4String("HcalFoilLayerLogical_1") + G4String(stringForLayerNo.str()), 0, 0, 0);
      FoilLayerLogical_2[iLayer] = new G4LogicalVolume(FoilLayerSolid, Polystyrole, 
						       G4String("HcalFoilLayerLogical_2") + G4String(stringForLayerNo.str()), 0, 0, 0);
      
      /* Scintillator*/
      WholeSensLayerLogical[iLayer] = new G4LogicalVolume(WholeSensLayerSolid, poly, 
							  G4String("HcalWholeSensLayerLogical") + G4String(stringForLayerNo.str()), 0, 0, 0);
      WholeSensLayerLogical[iLayer]->SetSensitiveDetector(hcalSD);

    }/*end loop over layers*/

  cal_hz = cal_hz + steel_hthickness_term;
    

  /*the z-position of the of the calorimeter*/
  z_place = z_begin + cal_hz;  

  /*create first the whole detector, filled with air*/
  G4Box *DetectorSolid = new G4Box("HcalDetectorSolid", cal_hx, cal_hy, cal_hz);
  DetectorLogical = new G4LogicalVolume(DetectorSolid, air, "HcalDetectorLogical", 0, 0, 0);
  
  /*create an additional absorber plate for the terminating part of the HCAL*/
  G4Box *AbsLayerSolid_term = new G4Box("HcalAbsLayerSolid_term", cal_hx, cal_hy, 
					steel_hthickness_term);
  AbsLayerLogical_term = new G4LogicalVolume(AbsLayerSolid_term, S235, 
					     "HcalAbsLayerLogical_term", 0, 0, 0);



  /*-------------------------------
    air gap; used instead of scintillator housing, 3M foil Layer1, sensitive layer, 
    3M foil Layer2, PCB and Cable-Fibre mixture
    for simulation of HCAL with half the number of sensor casettes
  -----------------------------------*/

  airGapThickness = steel_cassette_hthickness + foil_hthickness + poly_hthickness 
    + foil_hthickness + pcb_hthickness + cablefibre_mix_hthickness 
    + steel_cassette_hthickness;

  G4Box *AirGap_LayerSolid = new G4Box("AirGap_LayerSolid",
				       cal_hx,
				       cal_hy,
				       airGapThickness);
  AirGap_LayerLogical = new G4LogicalVolume(AirGap_LayerSolid, air,
					    "HcalAirGap_LayerLogical",
					    0, 0, 0);


 /*------------------------------------------------------------*/
  int displayMode = UserInit::getInstance()->getInt("HcalDisplayMode") ;
  /* if nothing specified, display full details*/
  if( displayMode == 0 )  displayMode = DM_FULL ;

  G4cout << "\n  Using display mode " << displayMode<<"\n" << G4endl;

  if( displayMode < DM_ABSORBERANDSENSITIVE ) 
  {
    for (G4int iLayer = 0; iLayer < n_layers; ++iLayer)
      {
	AbsLayerLogical[iLayer]->SetVisAttributes(G4VisAttributes::Invisible);
      }
	AbsLayerLogical_term->SetVisAttributes(G4VisAttributes::Invisible);
  }
  
  if( displayMode < DM_FULL )  
    {
      for (G4int iLayer = 0; iLayer < n_layers; ++iLayer)
	{
	  ScinHousLogical[iLayer]->SetVisAttributes(G4VisAttributes::Invisible);
	  CFmix_LayerLogical[iLayer]->SetVisAttributes(G4VisAttributes::Invisible);
	  PCBLayerLogical[iLayer]->SetVisAttributes(G4VisAttributes::Invisible);
	  FoilLayerLogical_1[iLayer]->SetVisAttributes(G4VisAttributes::Invisible);
	  FoilLayerLogical_2[iLayer]->SetVisAttributes(G4VisAttributes::Invisible);
	  WholeSensLayerLogical[iLayer]->SetVisAttributes(G4VisAttributes::Invisible);
	}
    }
  
  if (displayMode == DM_FULL)
    {
      for (G4int iLayer = 0; iLayer < n_layers; ++iLayer)
	{
	  G4VisAttributes *visAtt = new G4VisAttributes(G4Colour::Yellow());
	  visAtt->SetForceSolid(true);

	  WholeSensLayerLogical[iLayer]->SetVisAttributes(visAtt);

	  visAtt = new G4VisAttributes(G4Colour::White());
	  FoilLayerLogical_1[iLayer]->SetVisAttributes(visAtt);
	  FoilLayerLogical_2[iLayer]->SetVisAttributes(visAtt);
	  
	  visAtt = new G4VisAttributes(G4Colour::Green());
	  ScinHousLogical[iLayer]->SetVisAttributes(visAtt);

	  visAtt = new G4VisAttributes(G4Colour::Magenta());
	  CFmix_LayerLogical[iLayer]->SetVisAttributes(visAtt);

	  visAtt = new G4VisAttributes(G4Colour(0.63, 0.32, 0.18)); /*sienna*/
	  PCBLayerLogical[iLayer]->SetVisAttributes(visAtt);

	}
      
    }
  /*------------------------------------------------------------*/

#ifdef TBHCAL07_DEBUG
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
G4bool TBhcal07::PlaceHcalLayers(G4LogicalVolume *WorldLog)
{
  G4double inverseCosConfigAngle   = 1.0/cos(config_angle);

  /*coordinates of the middle of the layer*/
  G4double lay_x = x_begin;
  G4double lay_y = y_begin;
  G4double lay_z = z_begin + layer_hthickness[0] * inverseCosConfigAngle;
  
  G4double absorber_z     = z_begin + steel_hthickness[0] * inverseCosConfigAngle;
  G4double scinCassette_z = z_begin + (2. * steel_hthickness[0] + scinCassette_hthickness) * inverseCosConfigAngle;

#ifdef TBHCAL07_DEBUG
  G4cout<<"inverseCosConfigAngle: "<<inverseCosConfigAngle<<G4endl;
  G4cout << setprecision(6);
  G4cout << "x Position of layer: " << lay_x << G4endl;
  G4cout << "y Position of Layer: " << lay_y << G4endl;
  G4cout << "z Position of absorber: " << absorber_z <<"\n"<< G4endl;
  G4cout <<" inverseCosConfigAngle: "<<inverseCosConfigAngle<<G4endl;
  G4cout << "z Position of scinCassette: " << scinCassette_z <<"\n"<< G4endl;
  G4cout <<" scinCassette_hthickness: "<<scinCassette_hthickness<<endl;
#endif


  /*calculate full HCAL thickness*/
 G4double fullHCALThickness = 0;
  for (G4int iLayer = 0; iLayer < n_layers; ++iLayer)
    {
      fullHCALThickness += 2.0 * layer_hthickness[iLayer] * inverseCosConfigAngle;
    }
  fullHCALThickness = fullHCALThickness + 2.0 * steel_hthickness_term * inverseCosConfigAngle;

  G4cout<<"\n\n total HCAL thickness: "<<fullHCALThickness<<"\n\n"<<endl;

  /*helpers*/
  G4double deltaAbsorber     = fullHCALThickness/2. - steel_hthickness[0] * inverseCosConfigAngle;
  G4double deltaScinCassette = fullHCALThickness/2 - 2. * steel_hthickness[0] * inverseCosConfigAngle 
    - scinCassette_hthickness * inverseCosConfigAngle;

  G4double delta = fullHCALThickness/2. - layer_hthickness[0] * inverseCosConfigAngle;
 
#ifdef TBHCAL07_DEBUG
  G4cout<<"  fullHCALThickness: "<<fullHCALThickness
	<<"  deltaAbsorber: "<<deltaAbsorber<<" deltaScinCassette: "<<deltaScinCassette
	<<"\n"<<G4endl;
#endif

  /*----------------------------------------------------------------------------------*/
  G4bool isHcalFullyInstrumented = false;
  G4int temp = 0;
  for (G4int iLayer = 0; iLayer < n_layers; ++iLayer)
    {
      temp += sensitiveLayerPatternVector.at(iLayer);
    }
  if (temp == n_layers) isHcalFullyInstrumented = true;
#ifdef TBHCAL07_DEBUG
  G4cout<<"\n isHcalFullyInstrumented = "<<isHcalFullyInstrumented<<endl;
#endif
  /*----------------------------------------------------------------------------------*/
  G4double xOffsetAbsorber = 0;
  G4double zOffsetAbsorber = 0;

  G4double xOffsetScinCassette = 0;
  G4double zOffsetScinCassette = 0;

  G4double xOffset = 0;
  G4double zOffset = 0;

   for (G4int iLayer = 0; iLayer < n_layers; ++iLayer)
     {
      if (iLayer >= 1)
	{
	  lay_z += layer_hthickness[iLayer - 1] * inverseCosConfigAngle + layer_hthickness[iLayer] * inverseCosConfigAngle;

	  absorber_z     += (steel_hthickness[iLayer - 1] + 2. * scinCassette_hthickness + steel_hthickness[iLayer]) * inverseCosConfigAngle;
	  scinCassette_z += (2. * steel_hthickness[iLayer] + 2. * scinCassette_hthickness ) * inverseCosConfigAngle;

	  deltaAbsorber     = deltaAbsorber - steel_hthickness[iLayer - 1] * inverseCosConfigAngle 
	    - (iLayer - 1) * scinCassette_hthickness * inverseCosConfigAngle;
	  deltaScinCassette = deltaScinCassette - 2. * steel_hthickness[iLayer] * inverseCosConfigAngle  
	    - (iLayer + 1) * scinCassette_hthickness * inverseCosConfigAngle;

	  delta = delta - layer_hthickness[iLayer - 1] * inverseCosConfigAngle - layer_hthickness[iLayer] * inverseCosConfigAngle;
	}
      
       /* put in rotation*/
      xOffsetAbsorber = deltaAbsorber * sin(rotationAngle);
      zOffsetAbsorber = deltaAbsorber - deltaAbsorber * cos(rotationAngle);

      xOffsetScinCassette = deltaScinCassette * sin(rotationAngle);
      zOffsetScinCassette = deltaScinCassette - deltaScinCassette * cos(rotationAngle);

      xOffset = delta * sin(rotationAngle);
      zOffset = delta - delta * cos(rotationAngle);

#ifdef TBHCAL07_DEBUG
       G4cout<<"=========================================layer: "<<(iLayer+1)<<endl;
       G4cout<<" xOffsetAbsorber = "<<xOffsetAbsorber<<G4endl;
       G4cout<<" zOffsetAbsorber = "<<zOffsetAbsorber << G4endl;
       G4cout<<" deltaAbsorber = "<<deltaAbsorber<<G4endl;
       G4cout<<G4endl;
       G4cout<<" steel_hthickness[iLayer - 1]: "<<steel_hthickness[iLayer - 1]<<" 2. * scinCassette_hthickness: "<<2. * scinCassette_hthickness
	     <<" steel_hthickness[iLayer]: "<<steel_hthickness[iLayer]<<G4endl;
       G4cout<<" absorber_z = "<<absorber_z<<G4endl;
       G4cout<<" layer "<<(iLayer+1)<<" lay_x="<<lay_x<<" lay_y="<<lay_y <<" lay_z="<<lay_z<<G4endl;
       G4cout<<endl;
#endif       
       G4ThreeVector translateHcalAbsorber(lay_x + xOffsetAbsorber, lay_y, absorber_z + zOffsetAbsorber);
       G4ThreeVector translateHcalScinCassette(lay_x + xOffsetScinCassette, lay_y, scinCassette_z + zOffsetScinCassette);

       G4ThreeVector translateHCALLayer(lay_x + xOffset, lay_y, lay_z + zOffset);

       G4ThreeVector rotationAxis(0.0, 1.0, 0.0);
       G4RotationMatrix* rotation = new G4RotationMatrix();
       rotation->rotate((config_angle + rotationAngle), rotationAxis);



       std::stringstream stringForLayerNo;
       stringForLayerNo << (iLayer + 1); 

      /*place layer into logical volume reserved for the HCAL*/
       if ( sensitiveLayerPatternVector.at(iLayer) == 1 )
	 {
// 	   new G4PVPlacement(rotation,
// 			     translateHCALLayer,
// 			     WholeLayerLogical[iLayer],
// 			     G4String("WholeLayerPhys") + G4String(stringForLayerNo.str()),
// 			     WorldLog,
// 			     0,
// 			     (iLayer+1),
// 			     checkForOverlappingVolumes); 



	   new G4PVPlacement(rotation,
			     translateHcalAbsorber,
			     AbsLayerLogical[iLayer],
			     G4String("AbsorberLayerPhys") + G4String(stringForLayerNo.str()),
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

	   
	 }
       else
	 {
	   new G4PVPlacement(rotation,
			     translateHCALLayer,
			     WholeLayer2Logical[iLayer],
			     G4String("WholeLayerPhys") + G4String(stringForLayerNo.str()),
			     WorldLog,
			     0,
			     (iLayer+1),
			     checkForOverlappingVolumes); 
	   
	 }
     /*----------------------------------------------------
       In case of last layer, add terminating absorber plate
       (only for fully instrumented models)
       -----------------------------------------------------
     */
       if ( (iLayer == (n_layers - 1))
	    && isHcalFullyInstrumented )
	 {
	   lay_z += layer_hthickness[iLayer] * inverseCosConfigAngle + steel_hthickness_term * inverseCosConfigAngle;
	   delta = delta - layer_hthickness[iLayer] * inverseCosConfigAngle - steel_hthickness_term * inverseCosConfigAngle;

	   xOffset = delta * sin(rotationAngle);
	   zOffset = delta - delta * cos(rotationAngle);

	   
#ifdef TBHCAL07_DEBUG
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
void TBhcal07::SetSD()
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
  hcalSD = new TBSD_VCell03("hcalSD",
			    GetGridSize(),
			    ncell_xy[0],
			    ncell_xy[1],
			    GetDepthToLayer(),
			    TBHCAL,
			    Hcal_apply_Birks_law,
			    Hcal_time_cut,
                            zBeginTemp);

  /* register*/
  RegisterSensitiveDetector(hcalSD);
}


/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
void TBhcal07::Print()
{
  G4cout << "\n  ------> TBhcal07 parameters: <---------------------" << G4endl;
  G4cout<<"  HCAL begins at position ("<<G4BestUnit(x_begin, "Length")
	<<", "<<G4BestUnit(y_begin, "Length")<<", "<<G4BestUnit(z_begin, "Length")<<")"<<G4endl;
  G4cout<<"  HCAL dimensions: x="<<G4BestUnit(cal_hx*2, "Length")
	<<", y="<<G4BestUnit(cal_hy*2, "Length")
	<<", z="<<G4BestUnit(cal_hz*2, "Length")
	<<G4endl;
  G4cout<<"  HCAL placed at z="<<G4BestUnit(z_place, "Length")<<G4endl;
    
  G4cout<<"  Number of HCAL layers:   "<<n_layers<<G4endl;
  G4cout<<"  HCAL rotation angle:     "<<G4BestUnit(rotationAngle, "Angle")<<G4endl;
  G4cout<<"  HCAL configuration angle:"<<G4BestUnit(config_angle, "Angle")<<G4endl;
  G4cout<<"  Number of cells in x:    "<<ncell_xy[0]<<G4endl;
  G4cout<<"  Number of cells in z:    "<<ncell_xy[1]<<G4endl;
  G4cout<<"  HCAL grid size:          "<<grid_size<<G4endl;
  G4cout<<"  Scintillator (poly) thickness:          "<<G4BestUnit(poly_hthickness*2,           "Length")<<G4endl;
  G4cout<<"  Terminating absorber (steel) thickness: "<<G4BestUnit(steel_hthickness_term*2,     "Length")<<G4endl;
  G4cout<<"  Air gap thickness:                      "<<G4BestUnit(airgap_hthickness*2,         "Length")<<G4endl;
  G4cout<<"  Steel cassette thickness:               "<<G4BestUnit(steel_cassette_hthickness*2, "Length")<<G4endl;
  G4cout<<"  3M foil thickness:                      "<<G4BestUnit(foil_hthickness*2,           "Length")<<G4endl;
  G4cout<<"  PCB plate thickness:                    "<<G4BestUnit(pcb_hthickness*2,            "Length")<<G4endl;
  G4cout<<"  Cable-fibre mix thickness:              "<<G4BestUnit(cablefibre_mix_hthickness*2, "Length")<<G4endl;

  G4cout<<"\n"<<G4endl;
  G4cout << resetiosflags(ios::left);
  G4cout<<"  Layer" <<" Absorber thickness"<<" Layer thickness"<<G4endl;
  for (G4int iLayer = 0; iLayer < n_layers; ++iLayer)
    {
      G4cout<<setw(4)<<(iLayer+1)<<" "
	    <<setw(7)<<G4BestUnit(steel_hthickness[iLayer]*2, "Length")<<" "
	    <<setw(15)<<G4BestUnit(layer_hthickness[iLayer]*2, "Length")<<G4endl;
    }

  G4String layers, pattern;

  for (G4int i = 0; i < n_layers; ++i) 
    {
      char dummy[100];
      sprintf(dummy,"%2d ",i);
      layers += dummy;
      sprintf(dummy,"%2d ",sensitiveLayerPatternVector.at(i));
      pattern += dummy;
    }  
 
  G4cout << G4endl << "HCAL layer numbers and status of sensitive layers (1=sensitive layer installed, 0=air gap)" << G4endl 
	 << layers << G4endl << pattern << G4endl << G4endl;
  G4cout<<"  ----------------------------------------------------------"<<G4endl;
}

/*================================================================================*/
/*                                                                                */
/*                                                                                */
/*                                                                                */
/*================================================================================*/
void TBhcal07::DefineHcalMaterials() {
  G4Element* elH  = CGAGeometryManager::GetElement("H", true);  /*Hydrogen */
  G4Element* elC  = CGAGeometryManager::GetElement("C", true);  /*Carbon */
  G4Element* elO  = CGAGeometryManager::GetElement("O", true);  /*Oxygen */
  G4Element* elCl = CGAGeometryManager::GetElement("Cl", true); /*Chlorine */
  G4Element* elBr = CGAGeometryManager::GetElement("Br", true); /*Bromine */

  G4double density, fractionmass;
  G4String name, symbol;
  G4int nel,natoms;

  /* PCB (Printed Circuit Board) Material FR4
     Composition and density found under 
     http://pcba10.ba.infn.it/temp/ddd/ma/materials/list.html 
  */
  density = 1.7 *g/cm3;
  PCB = new G4Material(name="PCB", density, nel=5);
  PCB->AddMaterial(CGAGeometryManager::GetMaterial("silicon_2.33gccm"), fractionmass=0.180774);
  PCB->AddElement(elO, fractionmass=0.405633);
  PCB->AddMaterial(CGAGeometryManager::GetMaterial("graphite"), fractionmass=0.278042);
  PCB->AddElement(elH, fractionmass=0.0684428);
  PCB->AddElement(elBr, fractionmass=0.0671091);

  /*The steel we are going to use in the Hcal: Material S235JR (old name St37)
    Numbers found under 
    http://n.ethz.ch/student/zwickers/ download/fs_pe_grundlagen_cyrill.pdf 
  */
  density = 7.87*g/cm3;
  S235 = new G4Material(name="S235", density, nel=3);
  S235->AddMaterial(CGAGeometryManager::GetMaterial("iron"), fractionmass=0.9843);
  S235->AddMaterial(CGAGeometryManager::GetMaterial("graphite"), fractionmass=0.0017);
  S235->AddMaterial(CGAGeometryManager::GetMaterial("manganese"), fractionmass=0.014);

  /*Now, we're composing the cable-fibre mix
    We start with PVC the main component of the coax cables
    Numbers from http://www.elpac.de/Kunststoffkleinteile/Kleines_Kunststoff-Know-_How/PVC-P/pvc-p.html
  */
  density = 1.35 *g/cm3;
  G4Material* PVC = new G4Material(name="PCB", density, nel=3);
  PVC->AddElement(elH, natoms=3);
  PVC->AddElement(elC, natoms=2);
  PVC->AddElement(elCl, natoms=1);
  
  /*...and continuing with Polystyrole, an approximation for the
    Scintillating fibres and for the 3M foils 
    Numbers from http://de.wikipedia.org/wiki/Polystyrol
    the structural formula for the Styrene Polymer is C6H5CH=CH2
    The difference to Styropor definition in CGAGeometryManager
    comes since we do not have the material in a foamed form */
  density = 1.065 *g/cm3;
  Polystyrole = new G4Material(name="Polystyrole", density, nel=2);
  Polystyrole->AddElement(elH, natoms=8);
  Polystyrole->AddElement(elC, natoms=8);

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
  density = 0.120*g/cm3; 
  CF_MIX = new G4Material(name="Cable Fibre Mix", density, nel=3);
  CF_MIX->AddMaterial(CGAGeometryManager::GetMaterial("air"), fractionmass = 0.009);
  CF_MIX->AddMaterial(PVC, fractionmass = 0.872);
  CF_MIX->AddMaterial(Polystyrole, fractionmass = 0.119);


  /*materials*/
  poly    = CGAGeometryManager::GetMaterial("polystyrene");
  air     = CGAGeometryManager::GetMaterial("air");


  G4cout<<"\n  -----------> TBhcal07 material properties <----------------"<<G4endl;
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
void TBhcal07::PlaceHcalElementsIntoLayer() 
{
  G4double absorberPosition[MAX_TBHCAL_LAYERS]       = {0.};
  G4double steelCassettePosition1[MAX_TBHCAL_LAYERS] = {0.};
  G4double steelCassettePosition2[MAX_TBHCAL_LAYERS] = {0.};
  G4double cableFibreMixPosition[MAX_TBHCAL_LAYERS]  = {0.};
  G4double pcbPosition[MAX_TBHCAL_LAYERS]            = {0.};
  G4double foil3MPosition1[MAX_TBHCAL_LAYERS]        = {0.};
  G4double foil3MPosition2[MAX_TBHCAL_LAYERS]        = {0.};
  G4double scintillatorPosition[MAX_TBHCAL_LAYERS]   = {0.};
  G4double airGapPosition[MAX_TBHCAL_LAYERS]         = {0.};
 
  for (G4int iLayer = 0; iLayer < n_layers; ++iLayer)
    {
      /*---------- Put absorber plate into the layer ------------*/
//       new G4PVPlacement(0,
// 			G4ThreeVector(0, 0, steel_hthickness[iLayer]),
// 			AbsLayerLogical[iLayer],
// 			"HcalAbsLayerPhys",
// 			WholeAbsLogical[iLayer],                                
// 			0,
// 			0,
// 			checkForOverlappingVolumes); 

      /*---- Put scintillator housing front plate into the layer 
	(after absorber and before scintillator)---------------*/

//      steelCassettePosition1[iLayer] = absorberPosition[iLayer] + steel_hthickness[iLayer]
//       	+ 2.0 * airgap_hthickness + steel_cassette_hthickness;

     steelCassettePosition1[iLayer] = - scinCassette_hthickness + 2.0 * airgap_hthickness + steel_cassette_hthickness;
     
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
			ScinHousLogical[iLayer],
			"HcalScinHousPhys Rear",
			WholeScinCassetteLogical[iLayer],                                
			0,
			1,
			checkForOverlappingVolumes);

      /*--------------------------------------------------------------
	In case of 2006 models, with uninstrumented layers:
      --------------------------------------------------------------*/
      absorberPosition[iLayer] =  - layer_hthickness[iLayer] + steel_hthickness[iLayer];
       new G4PVPlacement(0,
			G4ThreeVector(0, 0, absorberPosition[iLayer]),
			AbsLayerLogical[iLayer],
			"HcalAbsLayer2Phys",
			WholeLayer2Logical[iLayer],                                
			0,
			0,
			checkForOverlappingVolumes); 

      airGapPosition[iLayer] = absorberPosition[iLayer] + steel_hthickness[iLayer] 
	+ 2.0 * airgap_hthickness + airGapThickness; 
 
      new G4PVPlacement(0,
			G4ThreeVector(0, 0, airGapPosition[iLayer]),
			AirGap_LayerLogical,
			"HcalAbsLayer2Phys",
			WholeLayer2Logical[iLayer],                                
			0,
			0,
			checkForOverlappingVolumes); 

    }/*end loop over HCAL layers*/


#ifdef TBHCAL07_DEBUG
  G4cout<<" \n\n TBhcal07::PlaceHcalElementsIntoLayer() "<<G4endl;
  for (G4int iLayer = 0; iLayer < n_layers; ++iLayer)
    {
      G4cout<<"  Layer "<<(iLayer+1)<<G4endl;
      G4cout << "       absorber plate at:           " << absorberPosition[iLayer] << " mm" << G4endl;
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


/*================================================================================*/
/*                                                                                */
/*     Check if the given string contains a valid sensitive layer pattern,        */
/*     i.e. if the string has a length equal to the total number of layers        */
/*     and the string contains only '1' (instrumented layer) and '0'              */
/*     (uninstrumented layer)                                                     */
/*                                                                                */
/*================================================================================*/
G4bool TBhcal07::isValidSensitiveLayerPattern(G4String sensitiveLayerPattern) 
{
  
  if ( ((G4int)sensitiveLayerPattern.length()) != n_layers ) return false;
  
  G4bool valid = true;

  for(G4int i = 0; i < n_layers; ++i) 
    {
      if ( (sensitiveLayerPattern.data()[i]!='0') && (sensitiveLayerPattern.data()[i]!='1') ) 
	{
	  valid = false;
	  break;
	}
    }
  
  return valid;
  
}
/*================================================================================*/
/*                                                                                */
/*   Fill a vector of integers with the content of the sensitive layer            */
/*   pattern string                                                               */
/*                                                                                */
/*================================================================================*/
std::vector<G4int> TBhcal07::getSensitiveLayerPatternVector(G4String sensitiveLayerPattern) 
{
  std::vector<G4int> patternVector;
  
  for(G4int i = 0; i < n_layers; ++i) 
    {
      G4String digit(sensitiveLayerPattern.data()[i]);
      patternVector.push_back(std::atoi(digit.data()));
    }

  return patternVector;
  
}
