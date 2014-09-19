
/*
$Id: LumiCalV00.cc 157 2011-01-17 09:58:15Z bogdan $
$Name$

history: version as LumiCalX01 except it does not build real cells
         but virtual cells are handled in LumiCalSD                Mar. 2010 (bp)
*/

#include "Control.hh"
#include "LumiCalV00.hh"

#ifdef MOKKA_GEAR
#include "gearimpl/CalorimeterParametersImpl.h"
//#include "gearimpl/LCALParametersImpl.h"
#include "gearimpl/LayerLayoutImpl.h"
#include "MokkaGear.h"
#include <math.h>
#endif

// define some levels of detail for graphical display
#define DM_FULL 3
#define DM_ABSORBERANDSENSITIVE 2
#define DM_WHOLELAYERONLY  1

//create an instance of class which is registered vs. the
//Mokka management system for the drivers 

INSTANTIATE (LumiCalV00)

  LumiCalV00::~LumiCalV00() {/*no op*/}

G4bool LumiCalV00::construct(const G4String &aSubDetectorDBName, G4LogicalVolume *WorldLog) 
{

  //Fetch database constants

  //open database (database opening routines provided by a MySQL wrapper
  //implemented in Mokka
  db = new Database(aSubDetectorDBName.data());
  
  //Set the data member WorldLogical
  WorldLogical = WorldLog;
  
  FetchdbEntries();
  AddMaterial();
  BuildElements();
  G4bool cokay = Build_LumiCal();
  
  return cokay;
}

void LumiCalV00::FetchdbEntries(){
  //fetch geometry constants 
  //provide access to database table block
  //
  G4cout << G4endl;
  G4cout << " LumiCalV00 start building ... : " << G4endl << G4endl;
  db->exec("select * from lumical;");
  db->getTuple();

  //LumiCal dimensions
  cal_innerradius =db->fetchDouble("calo_inner_radius") ;   
  cal_outerradius =db->fetchDouble("calo_outer_radius") ;   
  cal_extra_size  =db->fetchDouble("calo_extra_size") ;
  assert(cal_innerradius > 0. && cal_innerradius < cal_outerradius);
 //number of strips in R direction
  ncell_theta = db->fetchInt("nstrips_theta");
  //number of cells in phi angle
  ncell_phi = db->fetchInt("nstrips_phi");
  assert(ncell_phi >= 1 && ncell_theta >= 1);
  n_layers = db->fetchInt("n_layers");
  n_tiles  = db->fetchInt("n_tiles");
  assert( n_tiles > 0 && ncell_phi%n_tiles == 0 );
  assert(n_layers > 0);
  //beginning of calorimeter
  z_begin = db->fetchDouble("z_begin");
  //material thicknesses
  ear_height = 20.*mm;
  metalization_hthickness = 0.010 *mm;
  n_Bolts = 3;
  //
  fanout_hthickness = db->fetchDouble("support_thickness")/4.;
  tungsten_hthickness = db->fetchDouble("tungsten_thickness")/2.;
  silicon_hthickness = db->fetchDouble("silicon_thickness")/2.;
  layer_gap =db->fetchDouble("layer_gap") ;
  tile_gap  =db->fetchDouble("tile_gap"); 
 // beam crossing angle
  bx_angle = db->fetchDouble("crossing_angle")/2. * mrad; 
  //plane offset
  plane_phi_offset = db->fetchDouble("sensor_phi_offset") *deg;
  // Lcal phi offset
  phi_offset = db->fetchDouble("phi_offset") *mrad;
 

  //
  // calculate basic dimension   
  layer_hz = (tungsten_hthickness + 2.*fanout_hthickness+ metalization_hthickness + silicon_hthickness )*mm;
  cal_hz = ((G4double)n_layers) * (layer_hz + layer_gap/2.) *mm;
  cal_sphi = 0.*rad;
  cal_ephi = 2.*M_PI *rad;
  tile_dPhi= cal_ephi/((G4double)n_tiles);
  phistrip_dphi = (G4double) (cal_ephi/ncell_phi) ;
  sectors_per_tile = ncell_phi/n_tiles;
	assert ( ncell_phi%n_tiles == 0 );
  cal_sensor_rmin = cal_innerradius / cos ( G4double(sectors_per_tile)* phistrip_dphi / 2. );
  thetastrip_dr = (G4double) ((cal_outerradius-cal_sensor_rmin-2*tile_gap)/ncell_theta) *mm;
  silicon = CGAGeometryManager::GetMaterial("silicon_2.33gccm");
  Alu =  CGAGeometryManager::GetMaterial("aluminium");
   printf ("%s\n","====================================================");
   printf ("%s %5.3f %s\n","|  LumiCal sensor inner radius set to ",cal_sensor_rmin,"[mm] |" );
   printf ("%s\n","====================================================");
   //
  //
  /*
  G4cout << " LumiCal Zstart : "<< z_begin << " mm " << G4endl;
  G4cout << " LumiCal Zend   : "<< z_begin+2.*cal_hz << " mm " << G4endl;
  G4cout << " LumiCal Rmin   : "<< cal_innerradius << " mm" << G4endl;
  G4cout << " LumiCal Rmax   : "<< cal_outerradius << " mm" << G4endl;
  G4cout << " LumiCal length : "<< 2.*cal_hz << " mm" << G4endl;
  G4cout << " LumiCal #of rings     : "<< ncell_theta<< G4endl;
  G4cout << " LumiCal #of sectors   : "<< ncell_phi<< G4endl;
  G4cout << " Theta cell size : "<< thetastrip_dr<< " mm" << G4endl;
  G4cout << " LumiCal Phi offset: "<< phi_offset / mrad << " mrad" << G4endl;
  G4cout << " Phi cell size   : "<< phistrip_dphi / deg << " deg" << G4endl;
  G4cout << " Phi angle from  : "<< cal_sphi / deg << " to "<< cal_ephi / deg <<" deg" << G4endl;
  */
 //

#ifdef MOKKA_GEAR
  

//fg: switched to normal CalorimeterParameters for LCAL
  gear::CalorimeterParametersImpl * lcalParams = 
    new gear::CalorimeterParametersImpl(cal_sensor_rmin+tile_gap,
					cal_outerradius-tile_gap,
					z_begin,
					1, 
					db->fetchInt("phi_offset") );

  //fg: change this back to 14 mrad
  double bx_angle_mrad =  db->fetchDouble("crossing_angle") ; 
  lcalParams->setDoubleVal("beam_crossing_angle", bx_angle_mrad  ) ;

  cout << "gear: lcalParameters  BeamCrossingAngle = " << lcalParams->getDoubleVal("beam_crossing_angle" ) << endl;
  cout << "gear: lcalParameters  Phi0 = " << lcalParams->getPhi0() << endl;

  G4double l_thickness = tungsten_hthickness*2 + fanout_hthickness*4 +
    silicon_hthickness*2 + metalization_hthickness*2 + layer_gap;

  G4double a_thickness = tungsten_hthickness*2;

  G4double cellsize0 = thetastrip_dr;

  G4double cellsize1 = M_PI * 2.0 / ncell_phi;

  for (int i = 0; i < n_layers; i++)
     lcalParams->layerLayout().addLayer(l_thickness, cellsize0, cellsize1, a_thickness);
  
  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setLcalParameters( lcalParams ) ;

#endif

 
}


void LumiCalV00::BuildElements() {


  // create and register SD
  SetSD();


  //Set displayMode 
  //  int displayMode = UserInit::getInstance()->getInt("DisplayMode") ;
  // if( displayMode == 0 )  // if nothing specified display full details
    G4int displayMode = DM_WHOLELAYERONLY ;
  
        G4VisAttributes *AbsorberVisAtt = new G4VisAttributes(G4Colour(0.7, 0.0, 0.7));

  //Create logical volumes 

  //an air tube which will hold all layers
  // whole LumiCal  
  air = CGAGeometryManager::GetMaterial("air");
  G4Tubs *WholeLumiCalSolid = new G4Tubs("WholeLumiCalSolid",
					 cal_innerradius,
					 cal_outerradius+cal_extra_size,  // extra size for support and electronics
					 cal_hz,
					 cal_sphi, cal_ephi);

  WholeLumiCalLogical = new G4LogicalVolume( WholeLumiCalSolid,
					     air,
					     "WholeLumiCalLogical",
					     0, 0, 0);
  WholeLumiCalLogical->SetVisAttributes(G4VisAttributes::Invisible);

                               //================================================================
  if ( cal_extra_size > 0. ) { // space for mechanical support structure and electronics
                               //================================================================
  G4Tubs *SupportSpace = new G4Tubs("SupportSpace",
				    cal_outerradius,
				    cal_outerradius + cal_extra_size,
				    cal_hz,
				    cal_sphi, cal_ephi);
  SupportSpaceLog = new G4LogicalVolume ( SupportSpace,
                                          air,
					  "SupportSpaceLog",
					  0, 0, 0); 
  G4Tubs *SupportLayer = new G4Tubs("SupportLayer",
				    cal_outerradius,
				    cal_outerradius + cal_extra_size,
				    layer_hz+layer_gap/2.,
				    cal_sphi, cal_ephi);
  SupportLayerLog = new G4LogicalVolume ( SupportLayer,
                                          air,
					  "SupportLayerLog",
					  0, 0, 0);

  SupportLayerLog->SetVisAttributes(G4VisAttributes::Invisible);
  SupportSpaceLog->SetVisAttributes(G4VisAttributes::Invisible);
 
  new G4PVDivision("LCalSupportLayer", SupportLayerLog, SupportSpaceLog, kZAxis, n_layers, 0);

  // 
  // add support "ears"
  //
  G4double ear_dx  = cal_outerradius + ear_height;
  G4double ear_gap = 10. *mm;
  G4double ear_dy  = ear_gap*ear_dx/sqrt(ear_dx*ear_dx-cal_outerradius*cal_outerradius);
  G4double ear_hdz = tungsten_hthickness -0.25 *mm;
  G4double ear_z   = layer_hz - layer_gap/2. - ear_hdz;
  G4double bolt_radius = 4. *mm; 
  //

   G4EllipticalTube *solidEar0 = new G4EllipticalTube( "solidEar0", ear_dx, ear_dy, ear_hdz);
   G4Tubs *clipper = new G4Tubs ( "clipper" , 0., cal_outerradius, layer_hz, cal_sphi, cal_ephi);
   G4Tubs *puncher = new G4Tubs ( "puncher", 0., bolt_radius, layer_hz+layer_gap/2., 0., 360. *deg );
   // cut out center 
   G4SubtractionSolid *solidEar1 = new G4SubtractionSolid ( "solidEar1",
							    solidEar0,
							    clipper,
							    0,
							    G4ThreeVector(0., 0., 0.));
   // punch a hole for bolt
   G4double xhole = cal_outerradius + ear_height/2.;
   G4SubtractionSolid *solidEar2 = new G4SubtractionSolid( "solidEar2", solidEar1, puncher, 0, G4ThreeVector(  xhole, 0., 0.));
   G4SubtractionSolid *solidEar  = new G4SubtractionSolid( "solidEar" , solidEar2, puncher, 0, G4ThreeVector( -xhole, 0., 0.));
   // final ear
   //
   G4LogicalVolume *logicEar = new G4LogicalVolume ( solidEar,
						     CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
						     "logicEar", 0, 0, 0);
       G4VisAttributes *SuppVisAtt = new G4VisAttributes(G4Colour(0.7, 0.0, 0.7));
       SuppVisAtt->SetForceSolid( true );
       logicEar -> SetVisAttributes ( SuppVisAtt );

      G4LogicalVolume *LcalBoltLog = new G4LogicalVolume( puncher, 
							 CGAGeometryManager::GetMaterial("stainless_steel"),
							 "LcalBoltLog",
							 0, 0, 0);
       G4VisAttributes *BoltVisAtt = new G4VisAttributes(G4Colour(0.7, 0.7, 0.7));
       BoltVisAtt->SetForceSolid( true );
       LcalBoltLog -> SetVisAttributes ( BoltVisAtt );
  // populate Support layer with ears and bolts
   G4double dPhi  = cal_ephi/(G4double)(2*n_Bolts);
   G4double suppAng = 0.;
   
      for ( int ie=0 ; ie < n_Bolts; ie++ ){
        std::stringstream earstr;
        earstr << ie+1;
        G4String EarName = G4String("Ear") + G4String(earstr.str());
		G4Transform3D transear ( G4RotationMatrix().rotateZ( suppAng ),G4ThreeVector( 0., 0., ear_z).rotateZ( suppAng));
	new G4PVPlacement ( transear, logicEar, EarName, SupportLayerLog, false, ie+1);
		// bolts
        G4Transform3D transH1( G4RotationMatrix().rotateZ( suppAng ), G4ThreeVector(  xhole, 0., 0.).rotateZ( suppAng ));
        G4Transform3D transH2( G4RotationMatrix().rotateZ( suppAng ), G4ThreeVector( -xhole, 0., 0.).rotateZ( suppAng ));
        new G4PVPlacement ( transH1, LcalBoltLog, "LcalBolt", SupportLayerLog, false, ie*2  ); 
        new G4PVPlacement ( transH2, LcalBoltLog, "LcalBolt", SupportLayerLog, false, ie*2+1); 
 		suppAng += dPhi;
      }




    // FE mother board and cooling
      G4double FE_hth = 0.5 *mm;
      G4double FE_phi0 = atan( ear_gap/cal_outerradius );
      G4double FE_dphi = dPhi - 2.*FE_phi0;
   // FE chips
      G4double FEChip_hx   = 0.4*cal_extra_size;
      G4int    nFE_Sectors  = ncell_phi/(2*n_Bolts);       
      G4double FE_Sec_dphi  =  FE_dphi/G4double( nFE_Sectors );
      G4double FEChip_hy   = xhole*sin( FE_Sec_dphi/2. )-2.;
      G4double FEChip_hdz  = 0.25 *mm;
      G4double FECool_hdz  = 1.00 *mm;

      G4Tubs *FEMothSolid = new G4Tubs("FEBoardSolid",cal_outerradius, cal_outerradius+cal_extra_size, layer_hz, FE_phi0, FE_dphi);
      G4Tubs *FEBoardSolid = new G4Tubs("FEBoardSolid",cal_outerradius, cal_outerradius+cal_extra_size, FE_hth, FE_phi0, FE_dphi);
      G4Tubs *FECoolSolid = new G4Tubs("FEBoardSolid",cal_outerradius, cal_outerradius+cal_extra_size, FECool_hdz, FE_phi0, FE_dphi);
      G4Tubs *FESectorSolid= new G4Tubs("FESectorSolid",cal_outerradius, cal_outerradius+cal_extra_size, FEChip_hdz, FE_phi0, FE_Sec_dphi);
      G4Tubs *ChipMothSolid= new G4Tubs("ChipMothSolid",cal_outerradius, cal_outerradius+cal_extra_size, FEChip_hdz, FE_phi0, FE_dphi);
    // FE chips
      G4Box *ChipSolid = new G4Box("ChipSolid", FEChip_hx, FEChip_hy, FEChip_hdz);

      FEMothLog  = new G4LogicalVolume( FEMothSolid, air, "FEMotherLog", 0,0,0); 
      FECoolLog  = new G4LogicalVolume( FECoolSolid, Alu,"FECoolLog", 0,0,0);
      FEBoardLog = new G4LogicalVolume( FEBoardSolid, fanele2,"FEBoardLog", 0,0,0);
      ChipMothLog= new G4LogicalVolume( ChipMothSolid, silicon,"ChipMothLog", 0,0,0);
      G4LogicalVolume *FESectorLog= new G4LogicalVolume( FESectorSolid, air, "FESectorLog", 0,0,0); 
      G4LogicalVolume *FEChipLog = new G4LogicalVolume(ChipSolid, silicon, "FEChipLog", 0,0,0);
      new G4PVDivision( "FE-sector", FESectorLog, ChipMothLog, kPhi, nFE_Sectors, 0);
      // put chip in sector
      G4double xCh = cal_outerradius+cal_extra_size/2.;
      G4Transform3D transFE1( G4RotationMatrix().rotateZ( FE_phi0 + FE_Sec_dphi/2 ), G4ThreeVector(xCh,0.,0.).rotateZ( FE_phi0 + FE_Sec_dphi/2 ));
      new G4PVPlacement ( transFE1, FEChipLog, "LcalFEchip", FESectorLog, false, 0); 
     //
     G4VisAttributes *FEBoardVisAtt = new G4VisAttributes(G4Colour(0., 0.7, 0.));
     FEBoardVisAtt->SetForceWireframe( true );
     FEBoardLog->SetVisAttributes( FEBoardVisAtt );
     FEMothLog->SetVisAttributes( G4VisAttributes::Invisible ); 
     ChipMothLog->SetVisAttributes( G4VisAttributes::Invisible ); 
     FESectorLog->SetVisAttributes( G4VisAttributes::Invisible ); 

     //
     assert ( (layer_hz + layer_gap/2.) >= ( FECool_hdz + FE_hth + FEChip_hdz));

     // cooler
        G4double zpos = layer_hz - FECool_hdz;
        new G4PVPlacement ( 0, G4ThreeVector(0.,0.,zpos), FECoolLog, "LCalFECooling", FEMothLog, false, 1);
	// PCB
	zpos -= (FECool_hdz + FE_hth);
        new G4PVPlacement ( 0, G4ThreeVector(0.,0.,zpos), FEBoardLog, "LCalFEBoard", FEMothLog, false, 1);
	// chips
	zpos -= (FE_hth + FEChip_hdz);
        new G4PVPlacement ( 0, G4ThreeVector(0.,0.,zpos), ChipMothLog, "LCalFEchip", FEMothLog, false, 1);

	// everything in support layer
        zpos = -layer_gap/2.;
        G4double phi_rot = 0.; 
        for ( int k=0; k< n_Bolts ; k++ ){
	G4Transform3D FErot1 ( G4RotationMatrix().rotateZ( phi_rot ), G4ThreeVector( 0., 0., zpos).rotateZ( phi_rot ));
	G4Transform3D FErot2 ( G4RotationMatrix().rotateZ( phi_rot +180.*deg), G4ThreeVector( 0., 0., zpos).rotateZ( phi_rot +180.*deg));
	new G4PVPlacement ( FErot1 , FEMothLog, "FrontEndChips", SupportLayerLog, false, k);
	new G4PVPlacement ( FErot2 , FEMothLog, "FrontEndChips", SupportLayerLog, false, k+n_Bolts );
        phi_rot += dPhi;
      }
           
 
	// finally support space in LumiCal

        new G4PVPlacement ( 0, G4ThreeVector(0.,0.,0.) , SupportSpaceLog, "LCalSupport",WholeLumiCalLogical, false, 1);

     G4VisAttributes *FEVisAtt = new G4VisAttributes(G4Colour(0.4, 0.4, 0.3));
     FEVisAtt -> SetForceSolid( true );
     FEChipLog-> SetVisAttributes ( FEVisAtt );
     
  }

  //===================================================
  //an air tube which will hold the layer components
  // sensor, absorber, fanouts   
  // 
  //===================================================
  G4Tubs *LayerSolid = new G4Tubs("LayerSolid",
				      cal_innerradius,
				      cal_outerradius,
				      layer_hz,
				      cal_sphi, cal_ephi);
  
  LayerLogical = new G4LogicalVolume(LayerSolid,
                                          air,
                                          "LayerLogical",
                                          0,
                                          0,
                                          0);
  
  G4VisAttributes * LayerVisAtt = new G4VisAttributes(G4Colour(1.,1.,0));
  //LayerVisAtt->SetDaughtersInvisible(true);
  LayerVisAtt->SetForceWireframe(true);
  //WholeLumiCalLogical->SetVisAttributes(LayerVisAtt);
  LayerLogical->SetVisAttributes(LayerVisAtt);
  
  
  
  //logical volumes for the absorber and the scintillator to be filled 
  //into the logical volume of the layer as defined above
  
  //Absorber Plate made of Tungsten
  G4Tubs *AbsLayerSolid = new G4Tubs("AbsLayerSolid",
				    cal_innerradius,
				    cal_outerradius,
				    tungsten_hthickness,
				    cal_sphi, cal_ephi);
  

  AbsLayerLogical = new G4LogicalVolume(AbsLayerSolid,
					CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
					"AbsLayerLogical",
					0,
					0,
					0);
  
      AbsorberVisAtt->SetForceWireframe( true );
 
  AbsLayerLogical->SetVisAttributes( AbsorberVisAtt );

  // electronic fanout plates for the sillicon detector
  // 
  
  G4Tubs *SolidFan1 = new G4Tubs("FanOutGround",cal_innerradius,cal_outerradius,fanout_hthickness,cal_sphi,cal_ephi);
        LogicFan1 = new G4LogicalVolume(SolidFan1, fanele2,"LogicFan1", 0,0,0);
    
  G4Tubs *SolidFan2 = new G4Tubs("FanOutPC",cal_innerradius,cal_outerradius,fanout_hthickness,cal_sphi,cal_ephi);
        LogicFan2 = new G4LogicalVolume(SolidFan2, fanele1,"LogicFan2", 0,0,0);
  
    G4VisAttributes *FanoutVisAtt = new G4VisAttributes(G4Colour(0.0, 0.7, 0.0));
    FanoutVisAtt->SetForceSolid( true );
 	if( displayMode < DM_ABSORBERANDSENSITIVE ) {
	  FanoutVisAtt->SetVisibility( false );
	}
           LogicFan1->SetVisAttributes(FanoutVisAtt);
           LogicFan2->SetVisAttributes(FanoutVisAtt);
 
  // a volume of sensitive material

  G4VisAttributes* visSenseSilicon = new G4VisAttributes(G4Colour(12.5,6,0));
  visSenseSilicon->SetForceWireframe(true);
  //visSenseSilicon->SetForceSolid(true);
  visSenseSilicon->SetDaughtersInvisible(true);
 
  
	//whole silicon (helper) plane
	//
   G4Tubs *SensorSolid0 = new G4Tubs("SensorSolid", 0.,cal_outerradius,silicon_hthickness, cal_sphi, cal_ephi);
   //
   // polyhedra puncher 
   //
   G4double rcorner[2], zcorner[2];
   G4double r0[2] = {0., 0.};
   rcorner[0] = cal_innerradius;
   rcorner[1] = cal_innerradius;
   zcorner[0] = -2.*silicon_hthickness;
   zcorner[1] =  2.*silicon_hthickness;
   G4Polyhedra *puncher0 = new G4Polyhedra("puncher0", cal_sphi, cal_ephi, n_tiles, 2, zcorner, r0, rcorner); 
   G4SubtractionSolid *SiWaferSolid = new G4SubtractionSolid("SiWaferSolid" , SensorSolid0, puncher0, 0, G4ThreeVector( 0., 0., 0.));
   G4LogicalVolume *SiWaferLog = new G4LogicalVolume(SiWaferSolid,silicon, "SiWaferLog", 0, 0, 0);
        SiWaferLog-> SetVisAttributes( visSenseSilicon );
   //
   // Sensitive detector
   //
   G4Tubs *SensorSolid = new G4Tubs("SensorSolid", cal_sensor_rmin + tile_gap,
				                   cal_outerradius - tile_gap,
				                   silicon_hthickness,
				                   cal_sphi, cal_ephi);
    SensorLog = new G4LogicalVolume(SensorSolid, silicon, "SensorLog", 0, 0, 0);
    G4VisAttributes *SensorVisAtt = new G4VisAttributes(G4Colour(0.7, 0.7, 0.0));
    SensorVisAtt->SetForceSolid( true );
    SensorLog->SetVisAttributes( SensorVisAtt );
   new G4PVPlacement ( 0, G4ThreeVector(0.,0.,0.), SensorLog, "LcalSiSensor", SiWaferLog, false, 0);
   //
   // pad metalization
   //
   G4Tubs *PadMetalSolid = new G4Tubs("PadMetalSolid", cal_sensor_rmin + tile_gap,
				                       cal_outerradius - tile_gap,
				                       metalization_hthickness,
                                                       cal_sphi, cal_ephi);
    PadMetalLog = new G4LogicalVolume( PadMetalSolid, Alu, "PadMetalLog", 0, 0, 0);
    if ( tile_gap > 0. ){
   // dead gaps
   //
       G4double dx = (cal_outerradius - cal_sensor_rmin - 2.*tile_gap);
       G4double hdy = tile_gap + 0.05 *mm;
       G4double sn  = sin ( hdy/dx ); 
       G4double hdx = sqrt( 1. - sn*sn )*dx/2.;  // must be a bit shorter not to extend outside sensor 

     G4Box *solidAirGap1     = new G4Box("dAirGap", hdx, 0.05*mm, silicon_hthickness );
     G4Box *solidDeadGap1    = new G4Box("DeadGap", hdx, tile_gap + 0.05*mm, silicon_hthickness );
     G4Box *solidAirGap2     = new G4Box("dAirGap", hdx, tile_gap/2.+0.05*mm, metalization_hthickness );
     G4Box *solidDeadGap2    = new G4Box("DeadGap", hdx, tile_gap  + 0.05*mm, metalization_hthickness );
     G4LogicalVolume *logicAirGap1 = new G4LogicalVolume( solidAirGap1, air, "logAirGap1", 0,0,0);
     G4LogicalVolume *logicAirGap2 = new G4LogicalVolume( solidAirGap2, air, "logAirGap2", 0,0,0);
     G4LogicalVolume *logicDeadGap1 = new G4LogicalVolume( solidDeadGap1, silicon, "logDeadGap1", 0,0,0);
     G4LogicalVolume *logicDeadGap2 = new G4LogicalVolume( solidDeadGap2, Alu, "logDeadGap2", 0,0,0);
     //
     new G4PVPlacement ( 0, G4ThreeVector( 0.,0.,0.), logicAirGap1, "AirGap1", logicDeadGap1, false, 0);
     new G4PVPlacement ( 0, G4ThreeVector( 0.,0.,0.), logicAirGap2, "AirGap2", logicDeadGap2, false, 0);
 
 
	// 
	// put dead gaps into the sensor and metalizationplane
        //
     G4double phiRot = 0.;
     G4double dphiRot = tile_dPhi;
     G4double xpos = ( cal_sensor_rmin + cal_outerradius)/2.;
     G4int nt = n_tiles/2;
     for ( G4int it = 0; it < nt; it++) {
      G4Transform3D zrot1 ( G4RotationMatrix().rotateZ( phiRot ), G4ThreeVector( xpos, 0., 0.).rotateZ( phiRot ));
      G4Transform3D zrot2 ( G4RotationMatrix().rotateZ( phiRot + 180.*deg), G4ThreeVector( xpos, 0., 0.).rotateZ( phiRot + 180. *deg));
     new G4PVPlacement( zrot1, logicDeadGap1, "deadGap1", SensorLog, false, it+1);
     new G4PVPlacement( zrot2, logicDeadGap1, "deadGap1", SensorLog, false, nt+it+1);
     new G4PVPlacement( zrot1, logicDeadGap2, "deadGap2", PadMetalLog, false, it+1);     
     new G4PVPlacement( zrot2, logicDeadGap2, "deadGap2", PadMetalLog, false, nt+it+1);     
       phiRot += dphiRot;
     }
    }

     //Declare the silcon strip  to be sensitive 
     //
     SensorLog->SetSensitiveDetector(theLumiCalSD); 

  
                    // Layer assembly here

   //Put fanout  
  G4double pos_fan = -layer_hz + fanout_hthickness; 
    
  new G4PVPlacement(0,G4ThreeVector(0,0,pos_fan),LogicFan1 ,"LcalFanOut1",LayerLogical,0, 0);

   //Put pad metalisation  
  G4double pos_met =  pos_fan + fanout_hthickness + metalization_hthickness ; 
    
  new G4PVPlacement(0,G4ThreeVector(0,0,pos_met),PadMetalLog ,"LcalPadMetal",LayerLogical,0, 0);

 //Put silicon detector
  G4double pos_sens = pos_met + metalization_hthickness + silicon_hthickness; 
  
  new G4PVPlacement(0,G4ThreeVector(0,0,pos_sens),SiWaferLog,"LcalSiWafer",LayerLogical,0,0);

  //Put ground plate 
          pos_fan = pos_sens + silicon_hthickness + fanout_hthickness; 
    
  new G4PVPlacement(0,G4ThreeVector(0,0,pos_fan), LogicFan2,"LcalFanOut2",LayerLogical,0, 0);

 //Put absorber plate 
  G4double pos_abs = pos_fan + fanout_hthickness + tungsten_hthickness ;
 
  new G4PVPlacement(0,G4ThreeVector(0,0,pos_abs), AbsLayerLogical,"LcalAbsorber", LayerLogical, 0, 0);
  
  
    
}



G4bool LumiCalV00::Build_LumiCal() {

  //position of first layer
  G4double lay_z = -cal_hz + layer_hz;
  G4RotationMatrix* zrotPhi = new G4RotationMatrix();
  zrotPhi-> rotateZ( plane_phi_offset );
  //Put the layers into the LumiCal
  for (int nLay = 1; nLay < n_layers+1; nLay++) {
    
    std::stringstream slay;
    slay << nLay;
        
    //place layers into the LumiCal sub-module 
    //    G4cout << G4String("Layer") + G4String(slay.str())  <<" z= " << lay_z << G4endl;
    //
    G4String LayerName = G4String("LcalSensorLayer") + G4String(slay.str());

    if( nLay%2 )
      {// put odd layer without rotation
    new G4PVPlacement(0      ,  G4ThreeVector(0, 0, lay_z), LayerLogical, LayerName ,WholeLumiCalLogical, 0, nLay);
      } else {
      // rotate around Z by half of sector deltaphi
    new G4PVPlacement(zrotPhi,  G4ThreeVector(0, 0, lay_z), LayerLogical, LayerName ,WholeLumiCalLogical, 0, nLay);
    }

    lay_z += (layer_hz*2.0+layer_gap); 
    
  }
  //place two LumiCal sub-modules into the world
  //
  G4double z_center = z_begin+cal_hz;
  G4double rotAngle1 = 180. *deg - bx_angle;
  G4cout << " rotangle1 " << rotAngle1/deg << " bx_angle "<< bx_angle/deg <<" [deg]" <<  G4endl ;
  G4double rotAngle2 = bx_angle;
  G4Transform3D transformer1(G4RotationMatrix().rotateY(rotAngle1),G4ThreeVector(0, 0,  z_center).rotateY(rotAngle1));
  G4Transform3D transformer2(G4RotationMatrix().rotateY(rotAngle2),G4ThreeVector(0, 0,  z_center).rotateY(rotAngle2));


     new G4PVPlacement(transformer1,WholeLumiCalLogical,"LumiCalN",WorldLogical, 0, 0);
     new G4PVPlacement(transformer2,WholeLumiCalLogical,"LumiCalP",WorldLogical, 0, 1);
 
    delete db;
    db = 0;
    G4cout <<" LumiCalV00 done.\n" << G4endl;

  return true;
}

void LumiCalV00::AddMaterial(){
  G4double a, z, density, fractionmass, f1,f2,f3,dtot;
  G4double   depox = 0.0750 *mm;
  G4double dkapton = 0.0500 *mm;
  G4double dcoppr1 = 0.0250 *mm;
  G4double dcoppr2 = 0.0125 *mm;
  G4String name, symbol;
  G4int nel, natoms;
  /* fanele1 - fanout ground, 0.075mm epoxy  (1.3g/cm3), 
                              0.050mm kapton (1.42 g/cm3) ,
                              0.025mm copper (8,96 g/cm3) */
    dtot = depox+dkapton+dcoppr1 ;
    f1 = depox / dtot;   f2 = dkapton / dtot ; f3 = dcoppr1 / dtot ;
    density = ( f1*1.3 + f2*1.42 + f3*8.96 ) *g/cm3 ; 
    fanele1 = new G4Material(name="fanele1",density,nel=3);
    fanele1->AddMaterial(CGAGeometryManager::GetMaterial("epoxy"),  fractionmass=f1);
    fanele1->AddMaterial(CGAGeometryManager::GetMaterial("kapton"), fractionmass=f2);
    fanele1->AddMaterial(CGAGeometryManager::GetMaterial("copper"), fractionmass=f3);
  /* fanele2 - fanout PC    , 0.075mm epoxy  (1.3g/cm3), 
                              0.050mm kapton (1.42 g/cm3) ,
                              0.0125mm copper (8,96 g/cm3) */ 
    density = 2.429 *g/cm3;
    dtot = depox+dkapton+dcoppr2 ;
    f1 = depox / dtot;   f2 = dkapton / dtot ; f3 = dcoppr2 / dtot ;
    density = ( f1*1.3 + f2*1.42 + f3*8.96 ) *g/cm3 ; 
    fanele2 = new G4Material(name="fanele2",density,nel=3);
    fanele2->AddMaterial(CGAGeometryManager::GetMaterial("epoxy"),  fractionmass=f1);
    fanele2->AddMaterial(CGAGeometryManager::GetMaterial("kapton"), fractionmass=f2);
    fanele2->AddMaterial(CGAGeometryManager::GetMaterial("copper"), fractionmass=f3); 
  /* suppMat cermic AL2O3 support for FE electronic
   */
    G4Element *Oxygen = new G4Element( name="Oxygen" ,       symbol="O", z=8., a=16.00*g/mole );
    G4Element *Aluminium = new G4Element( name="Aluminium" ,symbol="Al", z=13.,a=26.982*g/mole );
    Al2O3 = new G4Material( name="Al2O3",density = 3.9*g/cm3, nel=2 );
    Al2O3->AddElement( Oxygen,   natoms=3 );
    Al2O3->AddElement(Aluminium, natoms=2 );
    
    //
    // G4cout << *(G4Material::GetMaterialTable())<< G4endl; 
}



void LumiCalV00::SetSD() {
  
  //create an instance of a sensitive detector class
  //the actual registration of all sensitive
  //detectors to G4 is done via a Mokka interface,
  //so we create an instance of our sens. detector class 
  //and make this
  //known to Mokka 
  theLumiCalSD = new LumiCalSD("LumiCal",
			       "Pad",
			       cal_sensor_rmin+tile_gap,
			       cal_sphi,
			       GetRhoCellSize(),
			       GetPhiCellSize(),
			       ncell_theta,
			       ncell_phi,
			       //LUMICALO defined in Mokka/source/Kernel/include/Control.hh
			       0,
			       true );  // tell SD to create virtual cells
  
  RegisterSensitiveDetector(theLumiCalSD);
}


