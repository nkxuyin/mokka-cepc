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
// $Id: Hcal04.cc,v 1.12 2007/12/21 11:47:26 kristian Exp $
// $Name: mokka-07-00 $
//
// Hcal04.cc
//
// History:  
//
// initial version: 
// F.Gaede: identical to  Hcal03 driver except that an additional gap
//          for the fibres is introduced between scintilaltor and steel


#include "MySQLWrapper.hh"
#include "Control.hh"
#include "MyPlacement.hh"
#include "G4UserLimits.hh"
#include "Hcal04.hh"
#include "CGAGeometryManager.hh"
#include "SD.hh"
#include "HECSD.hh"

#include "globals.hh"
#include "G4GeometryTolerance.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Polyhedra.hh"
#include "G4Trd.hh"
#include "G4Trap.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4UnionSolid.hh"

#include <algorithm>
#include <assert.h>

#include "CGADefs.h"

#ifdef MOKKA_GEAR
#include "gear/CalorimeterParameters.h" 
#include "gearimpl/CalorimeterParametersImpl.h" 
#include "gearimpl/LayerLayoutImpl.h" 
#include "MokkaGear.h"
#endif

INSTANTIATE(Hcal04)

G4bool 
Hcal04::construct(const G4String &aSubDetectorName,
		  G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding Hcal..." << G4endl;
  db = new Database(aSubDetectorName);
  
  if(Control::DUMPG3) MyPlacement::Init("HCAL",aSubDetectorName);

  //--------- BarrelHcal Sensitive detector -----

  // sensitive Model
  db->exec("select Model from `sensitive`;");
  db->getTuple();
  SensitiveModel = db->fetchString("Model");

  G4String SensitiveLog= "The sensitive model in Hcal chambers is " + SensitiveModel;
  Control::Log(SensitiveLog.data());

  
  // if RPC1 read the RPC parameters
  if (SensitiveModel == "RPC1")
    {
      db->exec("select * from rpc1;");
      db->getTuple();
      g10_thickness=db->fetchDouble("g10_thickness");
      glass_thickness=db->fetchDouble("glass_thickness");
      gas_thickness=db->fetchDouble("gas_thickness");
      spacer_thickness=db->fetchDouble("spacer_thickness");
      spacer_gap=db->fetchDouble("spacer_gap");
    }

  db->exec("select cell_dim_x, cell_dim_z, chamber_tickness from barrel_module,hcal;");
  db->getTuple();

  G4double cell_dim_x = db->fetchDouble("cell_dim_x");
  G4double chamber_tickness = db->fetchDouble("chamber_tickness");
  G4double cell_dim_z = db->fetchDouble("cell_dim_z");

  // The cell boundaries does not really exist as G4 volumes. So,
  // to avoid long steps over running  several cells, the 
  // theMaxStepAllowed inside the sensitive material is the
  // pad smaller x or z dimension.
  theMaxStepAllowed= std::min(cell_dim_x,cell_dim_z);
  
  //  Hcal  barrel regular modules
  theBarrilRegSD = 
    new SD(cell_dim_x,
	   cell_dim_z,
	   chamber_tickness,
	   HCALBARREL,
	   "HcalBarrelReg");
  RegisterSensitiveDetector(theBarrilRegSD);
  
  // Hcal  barrel end modules
  theBarrilEndSD = 
    new SD(db->fetchDouble("cell_dim_x"),
	   db->fetchDouble("cell_dim_z"),
	   db->fetchDouble("chamber_tickness"),
	   HCALBARREL,
	   "HcalBarrelEnd");
  RegisterSensitiveDetector(theBarrilEndSD);

  // Hcal  endcap modules
  theENDCAPEndSD =
    new HECSD(db->fetchDouble("cell_dim_x"),
	      db->fetchDouble("cell_dim_z"),
	      db->fetchDouble("chamber_tickness"),
	      HCALENDCAPMINUS,
	      "HcalEndCaps");
  RegisterSensitiveDetector(theENDCAPEndSD);

  // Set up the Radiator Material to be used
  db->exec("select material from radiator;");
  db->getTuple();
  G4String RadiatorMaterialName = db->fetchString("material");
  if(RadiatorMaterialName == "Iron")
    RadiatorMaterial =  CGAGeometryManager::GetMaterial("iron");
  else
    if(RadiatorMaterialName == "WMod")
      RadiatorMaterial =  CGAGeometryManager::GetMaterial("tungsten_11gccm");
    else Control::Abort("Hcal04: invalid radiator material name. \nIt has to be either Iron either WMod!!!",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

  RadiatorMaterialName+= " is the radiator material being placed.";
  Control::Log(RadiatorMaterialName.data());

  //----------------------------------------------------
  // Barrel
  //----------------------------------------------------
  Barrel(WorldLog);
  
  //----------------------------------------------------
  // EndCap Modules
  //----------------------------------------------------
  
  Endcaps(WorldLog);

#ifdef MOKKA_GEAR
  //----------------------------------------------------
  // MokkaGear
  //----------------------------------------------------

  // get Manager
  MokkaGear* gearMgr = MokkaGear::getMgr() ;


  // get the information that are not yet included
  db->exec("SELECT barrel_phi_offset FROM barrel;");
  db->getTuple();
  helpBarrel.phi0 = db->fetchDouble("barrel_phi_offset");

  // acquire parameters from mokkaGearManager
  // they are only set if the Superdriver has been called 
  bool hasSuperdriverParams = true ;
  try{
    dblParamHalfZ                     = gearMgr->tmpParam.getDoubleVal( "TPC_Ecal_Hcal_barrel_halfZ" ) ;
    dblParamLateralStructureThickness = gearMgr->tmpParam.getDoubleVal( "Hcal_lateral_structure_thickness" ) ;
    dblParamStavesGap                 = gearMgr->tmpParam.getDoubleVal( "Hcal_stave_gaps" ) ;
    dblParamBackPlateThickness        = gearMgr->tmpParam.getDoubleVal( "Hcal_back_plate_thickness" ) ;
    intParamEndModuleType             = gearMgr->tmpParam.getIntVal( "Hcal_barrel_end_module_type" ) ;
    dblParamDigiTileSize              = gearMgr->tmpParam.getDoubleVal( "Hcal_digitization_tile_size" ) ;
  } 
  catch( gear::UnknownParameterException& e ){
    
    hasSuperdriverParams = false ;
    dblParamDigiTileSize = cell_dim_x;
    
    G4cout << " WARNING: HCal04: the gear parameters TPC_Ecal_Hcal_barrel_halfZ, " << std::endl 
	   << "        Hcal_lateral_structure_thickness, Hcal_stave_gaps and " << std::endl 
	   << "        Hcal_back_plate_thickness are undefined (hcal superdriver not called)." << std::endl 
	   << "        They might be needed  for proper digitization (MarlinReco-MokkaCaloDigi). "
	   << std::endl  ;
  }


  // calculate zMax as total length/2
  helpBarrel.zMax = (helpBarrel.mostZ - helpBarrel.leastZ) / 2 ;

  // HCAL Barrel
  gear::CalorimeterParametersImpl* barrelParam = 
    new gear::CalorimeterParametersImpl( helpBarrel.innerRadius, helpBarrel.zMax, 8, helpBarrel.phi0 );

  // write all layers by position
  for (int i=0; i < helpBarrel.count; i++) {
    
    G4double calcThick  = helpBarrel.layerPos[i+1] - helpBarrel.layerPos[i] ;
    G4double calcAbsorb = calcThick - helpBarrel.sensThickness[i] - helpBarrel.gapThickness[i] ;

    // on last layer, gap has to be taken into account
    if( i == ( helpBarrel.count - 1 ) ) {
      G4double layerGap = helpBarrel.layerPos[i] - helpBarrel.layerPos[i-1] - calcThick ;
      
      // check if layerGap makes sense
      if ( layerGap < calcThick ) {
	
	// add gap to Thickness
	calcThick = calcThick + layerGap ;
      }
    }

    barrelParam->layerLayout().positionLayer
      (0, calcThick, dblParamDigiTileSize*cell_dim_z/cell_dim_x, dblParamDigiTileSize, calcAbsorb );
  }

  // write additional parameters - needed for MarlinReco-MokkaCaloDigi
  if( hasSuperdriverParams ){

    barrelParam->setDoubleVal( "TPC_Ecal_Hcal_barrel_halfZ"  , dblParamHalfZ ) ;
    barrelParam->setDoubleVal( "Hcal_lateral_structure_thickness" , dblParamLateralStructureThickness ) ;
    barrelParam->setDoubleVal( "Hcal_stave_gaps"             , dblParamStavesGap ) ;
    barrelParam->setDoubleVal( "Hcal_back_plate_thickness"   , dblParamBackPlateThickness ) ;
  }

  barrelParam->setIntVal   ( "Hcal_barrel_end_module_type" , intParamEndModuleType ) ;
  barrelParam->setDoubleVal( "Hcal_modules_gap"            , dblParamModulesGap ) ;
  barrelParam->setDoubleVal( "Hcal_virtual_cell_size"      , cell_dim_x );
 
  // fg/kh --- tell the user that the outer polygonal shape of the barrel has symmetry 16
  barrelParam->setIntVal   ( "Hcal_outer_polygon_order" ,  16  ) ;
  barrelParam->setIntVal   ( "Hcal_outer_polygon_phi0" ,  0  ) ;
    
  // write Barrel parameters to GearManager
  gearMgr->setHcalBarrelParameters( barrelParam ) ;  

  // HCAL Endcap
  gear::CalorimeterParametersImpl * endcapParam =
    new gear::CalorimeterParametersImpl (helpEndcap.innerRadius, helpEndcap.outerRadius, helpEndcap.leastZ, 2, helpEndcap.phi0);

  // write all layers by position
  for (int i=0; i < helpEndcap.count; i++) {
    
    G4double calcThick = helpEndcap.layerPos[i+1] - helpEndcap.layerPos[i] ;
    G4double calcAbsorb = calcThick - helpEndcap.sensThickness[i] - helpEndcap.gapThickness[i] ;  

    // on last layer, gap has to be taken into account
    if( i == ( helpEndcap.count - 1 ) ) {
      G4double layerGap = helpEndcap.layerPos[i] - helpEndcap.layerPos[i-1] - calcThick ;
      
      // check if layerGap makes sense
      if ( layerGap < calcThick ) {
	
	// add gap to Thickness
	calcThick = calcThick + layerGap ;
      }
    }

    endcapParam->layerLayout().positionLayer
      (0, calcThick, dblParamDigiTileSize*cell_dim_z/cell_dim_x, dblParamDigiTileSize, calcAbsorb );
    
  }

  // write Endcap parameters to GearManager
  endcapParam->setDoubleVal( "Hcal_virtual_cell_size"      , cell_dim_x );
  gearMgr->setHcalEndcapParameters( endcapParam ) ;

#endif

  // Closes Database connection
  delete db;

  return true;
}
Hcal04::~Hcal04() 
{
  //  if(theBarrilRegSD!=0) delete theBarrilRegSD;
  //  if(theBarrilEndSD!=0) delete theBarrilEndSD;
  //  if(theENDCAPEndSD!=0) delete theENDCAPEndSD;
}  


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              Barrel
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Hcal04::Barrel(G4LogicalVolume* MotherLog)
{
  BarrelRegularModules(MotherLog);
  BarrelEndModules(MotherLog);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              Barrel Regular Modules               ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Hcal04::BarrelRegularModules(G4LogicalVolume* MotherLog)
{
  // Regular modules

  db->exec("select bottom_dim_x/2 AS BHX,midle_dim_x/2. AS MHX, top_dim_x/2 AS THX, y_dim1_for_x/2. AS YX1H,y_dim2_for_x/2. AS YX2H,module_dim_z/2. AS DHZ from barrel_module,barrel_regular_module;");
  db->getTuple();
  G4double BottomDimY = db->fetchDouble("YX1H");
  G4double chambers_y_off_correction = db->fetchDouble("YX2H");
  
#ifdef MOKKA_GEAR
  G4double DHZ = db -> fetchDouble("DHZ");
#endif

  // Attention: on bâtit le module dans la verticale
  // à cause du G4Trd et on le tourne avant de le positioner
  G4Trd * Bottom = new G4Trd("Bottom_Barrel_Module",
			   db->fetchDouble("BHX"), 
			   db->fetchDouble("MHX"),
			   db->fetchDouble("DHZ"),
			   db->fetchDouble("DHZ"),
			   db->fetchDouble("YX1H"));

  G4Trd * Top = new G4Trd("Top_Barrel_Module",
			   db->fetchDouble("MHX"), 
			   db->fetchDouble("THX"),
			   db->fetchDouble("DHZ"),
			   db->fetchDouble("DHZ"),
			   db->fetchDouble("YX2H"));

  G4UnionSolid* ModuleSolid = 
    new G4UnionSolid("ModuleSolid",
		     Bottom,
		     Top,
		     0,
		     G4ThreeVector(0,
				   0,
				   db->fetchDouble("YX1H")+
				   db->fetchDouble("YX2H")));
  
  EnvLogHcalModuleBarrel  = new G4LogicalVolume(ModuleSolid,
 						RadiatorMaterial,
 						"barrelHcalModule", 
 						0, 0, 0);

  G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(.8,.8,.2));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  VisAtt->SetDaughtersInvisible(true);
  EnvLogHcalModuleBarrel->SetVisAttributes(VisAtt);

#ifdef MOKKA_GEAR
  // calculate ground-level for layers
  G4double yTotal = db->fetchDouble( "YX1H" ) + db->fetchDouble( "YX2H" ) ;
  // add first position of layer as ground level
  helpBarrel.layerPos.push_back( -yTotal ) ;
#endif 

  //----------------------------------------------------
  // Chambers in the Hcal Barrel 
  //----------------------------------------------------
  BarrelRegularChambers(EnvLogHcalModuleBarrel,chambers_y_off_correction);

//   // BarrelStandardModule placements
  db->exec("select stave_id,module_id,module_type,stave_phi_offset,inner_radius,module_z_offset from barrel,barrel_stave, barrel_module, barrel_modules where module_type = 1;"); //  un module: AND stave_id=1 AND module_id = 2
  db->getTuple();
  G4double Y;
  Y = db->fetchDouble("inner_radius")+BottomDimY;

#ifdef MOKKA_GEAR
  // starting value for iteration of inner_radius
  helpBarrel.innerRadius = Y ;
  G4double lastPosition = -1. ;
  G4int lastStaveID = -1;
  dblParamModulesGap=0.;
#endif

  do {
    G4double phirot = db->fetchDouble("stave_phi_offset")*pi/180;
    G4RotationMatrix *rot=new G4RotationMatrix();
    rot->rotateX(pi*0.5); // on couche le module.
    rot->rotateY(phirot);
    new MyPlacement(rot,
		    G4ThreeVector(-Y*sin(phirot),
				  Y*cos(phirot),
				  db->fetchDouble("module_z_offset")),
		    EnvLogHcalModuleBarrel,
		    "BarrelHcalModule",
		    MotherLog,
		    false,
		    HCALBARREL*100+db->fetchInt("stave_id")*10+
		    db->fetchInt("module_id"));
    theBarrilRegSD->SetStaveRotationMatrix(db->fetchInt("stave_id"),phirot);
    theBarrilRegSD->
      SetModuleZOffset(db->fetchInt("module_id"),
		       db->fetchDouble("module_z_offset"));

#ifdef MOKKA_GEAR
    // get inner Radius as minimum of all occurances of inner_radius
    helpBarrel.innerRadius = std::min( helpBarrel.innerRadius,db->fetchDouble("inner_radius") );

    // find out the borders of construction in both dimensions
    G4double thisModuleOffset = db->fetchDouble("module_z_offset") ;
    G4int thisStaveID = db->fetchInt("stave_id");
    helpBarrel.mostZ = std::max( helpBarrel.mostZ , thisModuleOffset + DHZ) ;
    helpBarrel.leastZ= std::min( helpBarrel.leastZ, thisModuleOffset - DHZ) ;
    dblParamBarrelMostZ = helpBarrel.mostZ ;

    // to find out gap one compares the delta in position to the module size
    // only if a last position exists
    if( !(lastPosition == -1.) && (thisStaveID==lastStaveID) ) {
      G4double thisGap = std::fabs( thisModuleOffset - lastPosition ) - 2*DHZ ;
      dblParamModulesGap = std::max( dblParamModulesGap , thisGap ) ;
    }
    lastPosition = thisModuleOffset ;
    lastStaveID=thisStaveID;

#endif

  } while(db->getTuple()!=NULL);
}

void Hcal04::BarrelRegularChambers(G4LogicalVolume* MotherLog,G4double chambers_y_off_correction)
{
  
  G4LogicalVolume * ChamberLog[200];
  G4Box * ChamberSolid;
  
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(.2,.8,.2));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  
  db->exec("select layer_id,chamber_dim_x/2. AS xdh,chamber_tickness/2. AS ydh,chamber_dim_z/2. AS zdh, fiber_gap from hcal,barrel_regular_layer,barrel_regular_module;");
  
  G4UserLimits* pULimits=
    new G4UserLimits(theMaxStepAllowed);

  while(db->getTuple()!=NULL){
    ChamberSolid = 
      new G4Box("ChamberSolid", 
		db->fetchDouble("xdh"),  //hx
		db->fetchDouble("zdh"),  //hz attention!
		db->fetchDouble("ydh")); //hy attention!
    
    if(SensitiveModel == "scintillator")
      {	
	
	//fg: introduce (empty) fiber gap - should be filled with fibres and cables
	// - so far we fill it  with air ...
	G4LogicalVolume *ChamberLogical =
	  new G4LogicalVolume(ChamberSolid,
			      CGAGeometryManager::GetMaterial("air"), 
			      "ChamberLogical", 
			      0, 0, 0);

	// the scintillator width is the chamber width - fiber gap 
	double fiber_gap = db->fetchDouble("fiber_gap")  ;

	double scintHalfWidth =  db->fetchDouble("ydh") - fiber_gap / 2. ;
	
	// fiber gap can't be larger than total chamber
	assert( scintHalfWidth > 0. ) ;

	G4Box * ScintSolid = 
	  new G4Box("ScintSolid", 
		    db->fetchDouble("xdh"),  //hx
		    db->fetchDouble("zdh"),  //hz attention!
		    scintHalfWidth ); //hy attention!
	
	G4LogicalVolume* ScintLog =
	  new G4LogicalVolume(ScintSolid,
			      CGAGeometryManager::GetMaterial("polystyrene"),
			      "ScintLogical", 
			      0, 0, pULimits);  
	
	// only scinitllator is sensitive
	ScintLog->SetSensitiveDetector(theBarrilRegSD);

	int layer_id = db->fetchInt("layer_id") ;

	new MyPlacement(0, G4ThreeVector( 0,0,  - fiber_gap / 2. ), ScintLog,
			"Scintillator", ChamberLogical, false, layer_id );   

	ChamberLog [ layer_id ] = ChamberLogical ;

#ifdef MOKKA_GEAR
	 // get heighth of sensible part of layer
	helpBarrel.sensThickness.push_back( scintHalfWidth*2 ) ;
        helpBarrel.gapThickness.push_back( fiber_gap ) ;
#endif

// 	ChamberLog [db->fetchInt("layer_id")] = 
// 	  new G4LogicalVolume(ChamberSolid,
// 			      CGAGeometryManager::GetMaterial("polystyrene"),
// 			      "ChamberLogical", 
// 			      0, 0, pULimits);  
// 	ChamberLog[db->fetchInt("layer_id")]->SetSensitiveDetector(theBarrilRegSD);
      }
    else 
      if (SensitiveModel == "RPC1")
	{
	  G4int layer_id = db->fetchInt("layer_id");
	  ChamberLog [layer_id] =
	    BuildRPC1Box(ChamberSolid,
			 theBarrilRegSD,
			 layer_id,
			 pULimits);
	}
      else Control::Abort("Invalid sensitive model in the dababase!",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

    ChamberLog[db->fetchInt("layer_id")]->SetVisAttributes(VisAtt);
  }   
  
  // Chamber Placements
  // module x and y offsets (needed for the SD)
  db->exec("select 0 AS module_x_offset, module_y_offset from barrel_module;");
  db->getTuple();
  
  G4double Xoff,Yoff;
  Xoff = db->fetchDouble("module_x_offset");
  Yoff = db->fetchDouble("module_y_offset");
  
  db->exec("select layer_id, 0. as chamber_x_offset,chamber_y_offset,0. as chamber_z_offset from barrel_regular_layer;");
   
  while(db->getTuple()!=NULL){    
    G4int layer_id = db->fetchInt("layer_id");
    new MyPlacement(0,
		    G4ThreeVector(db->fetchDouble("chamber_x_offset"),
				  db->fetchDouble("chamber_z_offset"),
				  db->fetchDouble("chamber_y_offset")+
				  chambers_y_off_correction),
		    //!!attention!! y<->z
		    ChamberLog [layer_id],
		    "ChamberBarrel",
		    MotherLog,false,layer_id);
    theBarrilRegSD->
      AddLayer(layer_id,
	       db->fetchDouble("chamber_x_offset") + Xoff - 
	       ((G4Box *)ChamberLog[layer_id]->GetSolid())->GetXHalfLength(),
	       db->fetchDouble("chamber_y_offset") + Yoff,
	       // - ((G4Box *)ChamberLog [layer_id]->GetSolid())->GetZHalfLength(),
	       db->fetchDouble("chamber_z_offset") - 
	       ((G4Box *)ChamberLog[layer_id]->GetSolid())->GetYHalfLength());    

#ifdef MOKKA_GEAR
    // count layers
    helpBarrel.count += 1;
    // get posoition for each layer
    // check for dynamic_cast
    G4Box * solidOfLog = dynamic_cast<G4Box*>(ChamberLog[layer_id]->GetSolid());
    if ( solidOfLog == 0 ) {
      Control::Abort("BarrelChambers are expected to be of type G4Box\nerror in 'Hcal03::BarrelRegularChambers' consturction MokkaGear.",MOKKA_OTHER_ERRORS);
    }
    helpBarrel.layerPos.push_back( db->fetchDouble("chamber_y_offset") + solidOfLog->GetZHalfLength() );
#endif

  }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              Barrel End Modules                 ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Hcal04::BarrelEndModules(G4LogicalVolume* MotherLog)
{
  // End modules
  // Get parameters
  db->exec("select y_dim1_for_z/2. AS YZ1H, y_dim2_for_z/2. AS YZ2H, y_dim3_for_z/2. AS YZ3H, top_end_dim_z/2.  AS TZ from barrel_end_module;");
  db->getTuple();
  G4double YZ1H = db->fetchDouble("YZ1H");
  G4double YZ2H = db->fetchDouble("YZ2H");
  G4double YZ3H = db->fetchDouble("YZ3H");
  G4double TZ = db->fetchDouble("TZ");
  G4double chambers_y_off_correction = YZ3H;

  db->exec("select bottom_dim_x/2 AS BHX,midle_dim_x/2. AS MHX, top_dim_x/2 AS THX, y_dim1_for_x/2. AS YX1H,y_dim2_for_x/2. AS YX2H,module_dim_z/2. AS DHZ from barrel_module,barrel_regular_module;");
  db->getTuple();
  G4double BHX = db->fetchDouble("BHX");
  G4double MHX = db->fetchDouble("MHX");
  G4double THX = db->fetchDouble("THX");
  G4double YX1H = db->fetchDouble("YX1H");
  G4double YX2H = db->fetchDouble("YX2H");
  G4double DHZ = db->fetchDouble("DHZ");

  // Attention: on bâtit le module dans la verticale
  // à cause des G4Trd et on le tourne avant de le positioner
  //
  // Base

  G4double MHXZ = BHX+(YZ1H+YZ2H)*(MHX-BHX)/YX1H;
  G4Trd * Base1 = new G4Trd("Base1_Barrel_End_Module",
			   BHX, // dx1 
			   MHXZ, // dx2
			   DHZ, // dy1
			   DHZ, // dy2
			   YZ1H+YZ2H); // dz
  
  G4Trd * Base2 = new G4Trd("Base2_Barrel_End_Module",
			    MHXZ,  
			    MHX,
			    TZ,
			    TZ,
			    YX1H-(YZ1H+YZ2H));

  G4UnionSolid* Base = 
    new G4UnionSolid("BasesEndSolid",
		     Base1,
		     Base2,
		     0,
		     G4ThreeVector(0,
				   TZ-DHZ,
				   YX1H));
  
  G4Trd * Top = new G4Trd("Top_Barrel_End_Module",
			  MHX, 
			  THX,
			  TZ,
			  TZ,
			  YX2H);
  
  G4UnionSolid* Ensemble1 = 
    new G4UnionSolid("EndSolid",
		     Base,
		     Top,
		     0,
		     G4ThreeVector(0,
				   TZ-DHZ,
				   2*YX1H-(YZ1H+YZ2H)+YX2H));

  G4double MHXZ1 = BHX+((YZ1H+YZ2H)-(TZ-DHZ))*(MHX-BHX)/YX1H;

  G4RotationMatrix *rot = new G4RotationMatrix();
  rot->rotateZ(pi/2.);


  G4double pDX = TZ-DHZ;
  G4double pDz = sqrt(4*YZ2H*YZ2H+pDX*pDX)/2.;  
  G4double pTheta = pi/2-atan(2*pDz/pDX);

  G4Trap* detail = new G4Trap("Trap",
			      pDz, //pDz
			      -pTheta,    //pTheta
			      0.,       //pPhi
			      MHXZ1, //pDy1
	G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(), //pDx1  
	G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(), //pDx2
			      0.,      //pAlp1
			      MHXZ, //pDy2,
			      pDX, //pDx3
			      pDX, //pDx4
			      0.);      //pAlp2
  
    
  G4UnionSolid* Ensemble2 = 
    new G4UnionSolid("EndSolid",
		     Ensemble1,
		     detail,
		     rot,
		     G4ThreeVector(0,
				   DHZ+pDX-pDz*tan(pTheta),
				   YZ1H+YZ2H-pDz));  //-pDX/2/tan(pTheta)
  
  EnvLogHcalModuleBarrel  = new G4LogicalVolume(Ensemble2,
 						RadiatorMaterial,
 						"barrelHcalModule", 
 						0, 0, 0);

  G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(.8,.8,.2));
  VisAtt->SetForceWireframe(true);
  VisAtt->SetDaughtersInvisible(true);
  //VisAtt->SetForceSolid(true);
  EnvLogHcalModuleBarrel->SetVisAttributes(VisAtt);


  //----------------------------------------------------
  // Chambers in the Hcal Barrel 
  //----------------------------------------------------
  BarrelEndChambers(EnvLogHcalModuleBarrel,chambers_y_off_correction);

//   // Barrel End Module placements
  db->exec("select stave_id,module_id,module_type,stave_phi_offset,inner_radius,module_z_offset from barrel,barrel_stave, barrel_module, barrel_modules where module_type = 2;");  // un module:  AND module_id = 1 AND stave_id = 1


  // Take care of this return here: if is possible when building
  // the Hcal prototype single module, where there isn't end modules
  // at all !!!
  if(db->getTuple()==NULL) return;

  G4double Y;
  Y = db->fetchDouble("inner_radius")+YZ1H+YZ2H;
  G4int stave_inv [8] = {1,8,7,6,5,4,3,2};
  do {
    G4double phirot = db->fetchDouble("stave_phi_offset")*pi/180;
    G4RotationMatrix *rot=new G4RotationMatrix();
    G4double Z = db->fetchDouble("module_z_offset");
    G4double Xplace = -Y*sin(phirot);
    G4double Yplace = Y*cos(phirot);
    G4int stave_number = db->fetchInt("stave_id");
    rot->rotateX(pi*0.5); // on couche le module.

    if(Z>0) {
      rot->rotateZ(pi);
      Xplace = - Xplace;
      stave_number = stave_inv[stave_number-1];
    }
    
    rot->rotateY(phirot);
    new MyPlacement(rot,
		    G4ThreeVector(Xplace,
				  Yplace,
				  Z),
		    EnvLogHcalModuleBarrel,
		    "BarrelHcalModule",
		    MotherLog,
		    false,
		    HCALBARREL*100+stave_number*10+db->fetchInt("module_id"));
    theBarrilEndSD->SetStaveRotationMatrix(db->fetchInt("stave_id"),phirot);
    theBarrilEndSD->
      SetModuleZOffset(db->fetchInt("module_id"),
		       db->fetchDouble("module_z_offset"));

#ifdef MOKKA_GEAR
    // get inner Radius as minimum of all occurances of inner_radius
    // gear actually assumes only _one_ inner radius ...
    helpBarrel.innerRadius = std::min( helpBarrel.innerRadius, db->fetchDouble("inner_radius") );
    
    // find out the borders of construction in both dimensions
    // here one has to take TZ in account since the top of the module has another length 
    // then the bottom! (overlap of DZ)
    helpBarrel.mostZ  = std::max( helpBarrel.mostZ , Z + TZ) ;
    helpBarrel.leastZ = std::min( helpBarrel.leastZ, Z - TZ) ;

    // find out gap between barrel and endcap
    dblParamStavesGap = std::max( dblParamStavesGap, Z - DHZ - dblParamBarrelMostZ ) ; 

#endif
    
  } while(db->getTuple()!=NULL);
}

void Hcal04::BarrelEndChambers(G4LogicalVolume* MotherLog, 
			     G4double chambers_y_off_correction)
{
  G4LogicalVolume * ChamberLog[200];
  G4Box * ChamberSolid;

  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(.2,.8,.2));
  VisAtt->SetForceWireframe(true);

  db->exec("select barrel_regular_layer.layer_id,chamber_dim_x/2. AS xdh,chamber_tickness/2. AS ydh,chamber_dim_z/2. AS zdh, fiber_gap from hcal,barrel_regular_layer,barrel_end_layer where barrel_regular_layer.layer_id = barrel_end_layer.layer_id ;");

  G4UserLimits* pULimits=
    new G4UserLimits(theMaxStepAllowed);
  
   while(db->getTuple()!=NULL)
     {
       ChamberSolid = 
	 new G4Box("ChamberSolid", 
		   db->fetchDouble("xdh"),  //hx
		   db->fetchDouble("zdh"),  //hz attention!
		   db->fetchDouble("ydh")); //hy attention!
      
       if(SensitiveModel == "scintillator")
	 {
	   //fg: introduce (empty) fiber gap - should be filled with fibres and cables
	   // - so far we fill it  with air ...
	   G4LogicalVolume *ChamberLogical =
	     new G4LogicalVolume(ChamberSolid,
				 CGAGeometryManager::GetMaterial("air"), 
				 "ChamberLogical", 
				 0, 0, 0);
	   
	   double fiber_gap = db->fetchDouble("fiber_gap")  ;
	   double scintHalfWidth =  db->fetchDouble("ydh") - fiber_gap  / 2. ;

	   // fiber gap can't be larger than total chamber
	   assert( scintHalfWidth > 0. ) ;

	   
	   G4Box * ScintSolid = 
	     new G4Box("ScintSolid", 
		       db->fetchDouble("xdh"),  //hx
		       db->fetchDouble("zdh"),  //hz attention!
		       scintHalfWidth ); //hy attention!
	   
	   G4LogicalVolume* ScintLog =
	     new G4LogicalVolume(ScintSolid,
				 CGAGeometryManager::GetMaterial("polystyrene"),
				 "ScintLogical", 
				 0, 0, pULimits);  
	   
	   // only scinitllator is sensitive
	   ScintLog->SetSensitiveDetector(theBarrilEndSD);
	   
	   int layer_id = db->fetchInt("layer_id") ;
	   
	   new MyPlacement(0, G4ThreeVector( 0,0,  - fiber_gap / 2. ), ScintLog,
			   "Scintillator", ChamberLogical, false, layer_id);   

	   ChamberLog [ layer_id ] = ChamberLogical ;
	   
// 	   ChamberLog [db->fetchInt("layer_id")] = 
// 	     new G4LogicalVolume(ChamberSolid,
// 				 CGAGeometryManager::GetMaterial("polystyrene"),
// 				 "ChamberLogical", 
// 				 0, 0, pULimits);  
// 	   ChamberLog[db->fetchInt("layer_id")]->SetSensitiveDetector(theBarrilEndSD);
	 }
       else 
	 if (SensitiveModel == "RPC1")
	   {
	     G4int layer_id = db->fetchInt("layer_id");
	     ChamberLog [layer_id] =
	       BuildRPC1Box(ChamberSolid,theBarrilEndSD,layer_id,pULimits);
	   }
	 else Control::Abort("Invalid sensitive model in the dababase!",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
       ChamberLog[db->fetchInt("layer_id")]->SetVisAttributes(VisAtt);
     }
   
   
   // End Chamber Placements
  // module x and y offsets (needed for the SD)
   db->exec("select 0 AS module_x_offset, module_y_offset from barrel_module;");
   db->getTuple();
   G4double Xoff,Yoff;
   Xoff = db->fetchDouble("module_x_offset");
   Yoff = db->fetchDouble("module_y_offset");
   
   db->exec("select barrel_regular_layer.layer_id, 0. as chamber_x_offset,chamber_y_offset,chamber_z_offset as chamber_z_offset,chamber_dim_z/2 AS YHALF,chamber_dim_x/2. as XHALF from barrel_regular_layer,barrel_end_layer where barrel_regular_layer.layer_id = barrel_end_layer.layer_id;");
   
   while(db->getTuple()!=NULL){
     G4int layer_id = db->fetchInt("layer_id");
     new MyPlacement(0,
		     G4ThreeVector(db->fetchDouble("chamber_x_offset"),
				   db->fetchDouble("chamber_z_offset"),
				   db->fetchDouble("chamber_y_offset")+
				   chambers_y_off_correction),
		     //!!attention!! y<->z
		     ChamberLog [db->fetchInt("layer_id")],
		     "ChamberBarrel",
		     MotherLog,false,layer_id);
     theBarrilEndSD->
       AddLayer(layer_id,
		db->fetchDouble("chamber_x_offset") + 
		Xoff - db->fetchDouble("XHALF"),
		db->fetchDouble("chamber_y_offset") + Yoff,
		- (db->fetchDouble("chamber_z_offset")+
		   db->fetchDouble("YHALF")));

#ifdef MOKKA_GEAR
     // These layers are _not_ taken into account.
     // The layer definition for the barrel solely comes out of
     // the definition of barrel regular layers
#endif
   }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              Endcaps                 ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Hcal04::Endcaps(G4LogicalVolume* MotherLog)
{
  db->exec("select module_radius AS pRMax, module_dim_z/2. AS pDz, center_box_size/2. AS pRMin from endcap_standard_module;");
  db->getTuple();

  G4double zPlane[2];
  zPlane[0]=-db->fetchDouble("pDz");
  zPlane[1]=-zPlane[0];

  G4double rInner[2],rOuter[2];
  rInner[0]=rInner[1]=db->fetchDouble("pRMin");
  rOuter[0]=rOuter[1]=db->fetchDouble("pRMax");

#ifdef MOKKA_GEAR
  // Write parameters in helper class
  // attention: the outer symmetrie is 32-fold... this is not
  // taken into account in gear
  
  // the outer radius is in Geant4 defined as the tangent outer radius.
  // In Gear description differs. We will take the heigth of the triangle
  // in account as well in order to get the max radius.

  helpEndcap.outerRadius = rOuter[0] * cos( pi/9 );  // outer radius (triangle)
  helpEndcap.innerRadius = rInner[0] ;              // inner radius
  helpEndcap.phi0 = 0. ;                             // phi0
  
  // set starting value for inner_z
  // it should _not_ stay at this value
  helpEndcap.leastZ = 999999999. ;
#endif

  G4Polyhedra *EndCapSolid=
    new G4Polyhedra("HcalEndCapSolid",
		    0.,
		    360.,
		    32,
		    2,
		    zPlane,
		    rInner,
		    rOuter);
		    

  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(.8,.8,.2));
  VisAtt->SetForceWireframe(true);
  VisAtt->SetDaughtersInvisible(true);
  //VisAtt->SetForceSolid(true);

  G4LogicalVolume* EndCapLogical =
    new G4LogicalVolume(EndCapSolid,
			RadiatorMaterial,
			"EndCapLogical",
			0, 0, 0);
  EndCapLogical->SetVisAttributes(VisAtt);

#ifdef MOKKA_GEAR
  // add first position of layer as ground level
  helpEndcap.layerPos.push_back( zPlane[0] ) ;
#endif

  // build and place the chambers in the Hcal Endcaps
  EndcapChambers(EndCapLogical);

  // Placements
   
  db->exec("select endcap_id,endcap_z_offset from endcap;");
   
  G4RotationMatrix *rotEffect=new G4RotationMatrix();
  rotEffect->rotateZ(pi/8.);
  G4int ModuleNumber = HCALENDCAPPLUS*100+16;
  G4double Z1=0;
  while(db->getTuple()!=NULL){    
    Z1=db->fetchDouble("endcap_z_offset");
    new MyPlacement(rotEffect,
		    G4ThreeVector(0.,
				  0.,
				  Z1),
		    EndCapLogical,
		    "EndCapPhys",
		    MotherLog,
		    false,
		    ModuleNumber);
    rotEffect=new G4RotationMatrix();
    rotEffect->rotateZ(pi/8.);
    rotEffect->rotateY(pi); // On inverse les endcaps 
    ModuleNumber -= (HCALENDCAPPLUS-HCALENDCAPMINUS)*100 + 6;

#ifdef MOKKA_GEAR
    // take inner_z as minimum of all offsets - halfThickness
    helpEndcap.leastZ = std::min( helpEndcap.leastZ, std::abs(Z1)-std::abs(zPlane[0]) );
#endif 

  }
  theENDCAPEndSD->
    SetModuleZOffset(0,
		     fabs(Z1));
  theENDCAPEndSD->
    SetModuleZOffset(6,
		     fabs(Z1));
}

void Hcal04::EndcapChambers(G4LogicalVolume* MotherLog)
{
  // Chambers in the Hcal04::Endcaps
  // standard endcap chamber solid:
  db->exec("select chamber_radius AS pRMax, chamber_tickness/2. AS pDz, fiber_gap, center_box_size/2. AS pRMin from endcap_standard_module,hcal;");
  db->getTuple();
  
  // G4Polyhedra Envelope parameters
  G4double phiStart = 0.;
  G4double phiTotal = 360.;
  G4int numSide = 32;
  G4int numZPlanes = 2;

  G4double zPlane[2];
  zPlane[0]=-db->fetchDouble("pDz");
  zPlane[1]=-zPlane[0];

  G4double rInner[2],rOuter[2];
  rInner[0]=rInner[1]=db->fetchDouble("pRMin");
  rOuter[0]=rOuter[1]=db->fetchDouble("pRMax");

  G4Polyhedra *EndCapChamberSolid=
    new G4Polyhedra("EndCapChamberSolid",
		    phiStart,
		    phiTotal,
		    numSide,
		    numZPlanes,
		    zPlane,
		    rInner,
		    rOuter);
		    
  G4UserLimits* pULimits=
    new G4UserLimits(theMaxStepAllowed);
  
  // standard endcap chamber logical
  G4LogicalVolume* EndCapChamberLogical=0;

  if(SensitiveModel == "scintillator")
    {
      //fg: introduce (empty) fiber gap - should be filled with fibres and cables
      // - so far we fill it  with air ...
      EndCapChamberLogical =
 	     new G4LogicalVolume(EndCapChamberSolid,
 				 CGAGeometryManager::GetMaterial("air"), 
				 "EndCapChamberLogical", 
				 0, 0, 0);
	   
	   double fiber_gap = db->fetchDouble("fiber_gap")  ;
	   double scintHalfWidth =  db->fetchDouble("pDz") - fiber_gap  / 2. ;

	   // fiber gap can't be larger than total chamber
	   assert( scintHalfWidth > 0. ) ;

	   
	   G4double zPlaneScint[2];
	   zPlaneScint[0]=-scintHalfWidth ;
	   zPlaneScint[1]=-zPlaneScint[0];

	   G4Polyhedra *EndCapScintSolid=
	     new G4Polyhedra("EndCapScintSolid",
			     phiStart,
			     phiTotal,
			     numSide,
			     numZPlanes,
			     zPlaneScint,
			     rInner,
			     rOuter);

	   G4LogicalVolume* ScintLog =
	     new G4LogicalVolume(EndCapScintSolid,
				 CGAGeometryManager::GetMaterial("polystyrene"),
				 "EndCapScintLogical", 
				 0, 0, pULimits);  
	   
	   // only scinitllator is sensitive
	   ScintLog->SetSensitiveDetector(theENDCAPEndSD);

#ifdef MOKKA_GEAR
	   // thickness of sensible part as often as zPlanes are there
	   helpEndcap.sensThickness.push_back( scintHalfWidth*2 ) ;
           helpEndcap.gapThickness.push_back( fiber_gap ) ;
#endif
	   
	   
	   new MyPlacement(0, G4ThreeVector( 0,0,  - fiber_gap / 2.  ), ScintLog,
			   "EndCapScintillator", EndCapChamberLogical, false, 0 );   
	   
//       EndCapChamberLogical =
// 	new G4LogicalVolume(EndCapChamberSolid,
// 			    CGAGeometryManager::GetMaterial("polystyrene"),
// 			    "EndCapChamberLogical",
// 			    0, 0, pULimits);
//       // LE SENSITIVE ICI
//       EndCapChamberLogical->SetSensitiveDetector(theENDCAPEndSD);
    }
    else 
      if (SensitiveModel == "RPC1")
	{
	  EndCapChamberLogical =
	    BuildRPC1Polyhedra(EndCapChamberSolid,
			       theENDCAPEndSD,
			       phiStart,
			       phiTotal,
			       numSide,
			       numZPlanes,
			       zPlane,
			       rInner,
			       rOuter,
			       pULimits);
	}
      else Control::Abort("Invalid sensitive model in the dababase!",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
  
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(1.,1.,1.));
  EndCapChamberLogical->SetVisAttributes(VisAtt);


  // standard endcap chamber placements
  db->exec("select layer_id,chamber_z_offset AS Zoff from endcap_layer;");
  
  G4int layer_id;
  while(db->getTuple()!=NULL){
    layer_id=db->fetchInt("layer_id");
    new MyPlacement(0,
		    G4ThreeVector(0.,
				  0.,
				  db->fetchDouble("Zoff")),
		    EndCapChamberLogical,
		    "EndCapChamberPhys",
		    MotherLog,false,layer_id);
    theENDCAPEndSD->
      AddLayer(layer_id,
	       0,
	       0,
	       db->fetchDouble("Zoff"));
    
#ifdef MOKKA_GEAR
    // count the layers
    helpEndcap.count += 1;
    // position of layer as offset + half thickness
    helpEndcap.layerPos.push_back( db->fetchDouble("Zoff") + std::abs(zPlane[0]) ) ;
    
    // sensible Area is the same every time
    // value set in 0 within construction of Scint or RPC
    helpEndcap.sensThickness.push_back( helpEndcap.sensThickness[0] );
    helpEndcap.gapThickness.push_back( helpEndcap.gapThickness[0] );
    
#endif

  }  
}

G4LogicalVolume * 
Hcal04::BuildRPC1Box(G4Box* ChamberSolid, 
		     SD* theSD, 
		     G4int layer_id,
		     G4UserLimits* pULimits)
{
  if(ChamberSolid->GetEntityType()=="G4Box")
    {
      // fill the Chamber Envelope with G10
      G4LogicalVolume *ChamberLog =
	new G4LogicalVolume(ChamberSolid,
			    CGAGeometryManager::GetMaterial("g10"), 
			    "RPC1", 
			    0, 0, 0);

      //
      // build the RPC glass 
       //!!attention!! y<->z
      G4Box * GlassSolid =
	new G4Box("RPC1Glass", 
		  ChamberSolid->GetXHalfLength(),
		  ChamberSolid->GetYHalfLength(),
		  glass_thickness/2.);
      
      G4LogicalVolume *GlassLogical =
	new G4LogicalVolume(GlassSolid,
			    CGAGeometryManager::GetMaterial("pyrex"),
			    "RPC1glass", 
			    0, 0, 0);
      
      G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(.8,0,.2));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      GlassLogical->SetVisAttributes(VisAtt);
      
      //
      // build the standard spacer
       //!!attention!! y<->z
      G4Box * SpacerSolid =
	new G4Box("RPC1Spacer", 
		  ChamberSolid->GetXHalfLength(),
		  spacer_thickness/2.,
		  gas_thickness/2.);
      
      G4LogicalVolume *SpacerLogical =
 	new G4LogicalVolume(SpacerSolid,
 			    CGAGeometryManager::GetMaterial("g10"),
 			    "RPC1Spacer", 
 			    0, 0, 0);

      VisAtt = new G4VisAttributes(G4Colour(1,1.1));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      SpacerLogical->SetVisAttributes(VisAtt);

      //
      // build the gas gap
      //
      G4Box * GasSolid =
	new G4Box("RPC1Gas", 
		  ChamberSolid->GetXHalfLength(),
		  ChamberSolid->GetYHalfLength(),
		  gas_thickness/2.);
      
      G4LogicalVolume *GasLogical =
	new G4LogicalVolume(GasSolid,
			    CGAGeometryManager::GetMaterial("RPCGAS1"),
			    "RPC1gas", 
			    0, 
			    0, 
			    pULimits);
      
      VisAtt = new G4VisAttributes(G4Colour(.1,0,.8));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      GasLogical->SetVisAttributes(VisAtt);

      // PLugs the sensitive detector HERE!
      GasLogical->SetSensitiveDetector(theSD);

#ifdef MOKKA_GEAR
      // get heighth of sensible part of layer
      helpBarrel.sensThickness.push_back( GasSolid->GetZHalfLength()*2 );
      helpBarrel.gapThickness.push_back( spacer_thickness ) ;
#endif
      
      // placing the spacers inside the gas gap
      G4double NextY = 
	- ChamberSolid->GetYHalfLength() + spacer_thickness/2.;
      while ( NextY < ChamberSolid->GetYHalfLength()-spacer_thickness/2.)
	{
	  new MyPlacement(0,
			  G4ThreeVector(0,
					NextY,
					0),
			  SpacerLogical,
			  "RPCSpacer",
			  GasLogical,
			  false,
			  0);
	  NextY += spacer_gap;
	}

      // placing the all. 
      // ZOffset starts pointing to the chamber border.
      G4double ZOffset = - ChamberSolid->GetZHalfLength();

      // first glass border after the g10_thickness
      ZOffset += g10_thickness + glass_thickness/2.;

      new MyPlacement(0,
		      G4ThreeVector(0,
				    0,
				    ZOffset),
		      GlassLogical,
		      "RPCGlass",
		      ChamberLog,
		      false,
		      0);
      
      // set ZOffset to the next first glass border
      ZOffset += glass_thickness/2.;
      
      // gas gap placing 
      ZOffset += gas_thickness/2.; // center !

      new MyPlacement(0,
		      G4ThreeVector(0,
				    0,
				    ZOffset),
		      GasLogical,
		      "RPCGas",
		      ChamberLog,
		      false,
		      layer_id);   // layer_id is set up here!
      
      // set ZOffset to the next gas gap border
      ZOffset += gas_thickness/2.;
      
      // second glass.
      ZOffset += glass_thickness/2.; // center !

      new MyPlacement(0,
		      G4ThreeVector(0,
				    0,
				    ZOffset),
		      GlassLogical,
		      "RPCGlass",
		      ChamberLog,
		      false,
		      0);
      return ChamberLog;
    }

  Control::Abort("Hcal04::BuildRPC1Box: invalid ChamberSolidEnvelope",MOKKA_OTHER_ERRORS);
  return NULL;
}

G4LogicalVolume * 
Hcal04::BuildRPC1Polyhedra(G4Polyhedra* ChamberSolid, 
			   SD* theSD,
			   G4double phiStart,
			   G4double phiTotal,
			   G4int numSide,
			   G4int numZPlanes,
			   const G4double zPlane[],
			   const G4double rInner[],
			   const G4double rOuter[],
			   G4UserLimits* pULimits)
{
  if(ChamberSolid->GetEntityType()=="G4Polyhedra")
    {
      // fill the Chamber Envelope with G10
      G4LogicalVolume *ChamberLog =
	new G4LogicalVolume(ChamberSolid,
			    CGAGeometryManager::GetMaterial("g10"),
			    "RPC1", 
			    0, 0, 0);
      //
      // build the RPC glass 
      
      G4double NewZPlane[2];
      NewZPlane[0] = glass_thickness/2.;
      NewZPlane[1] = -NewZPlane[0];

      G4Polyhedra * GlassSolid =
	new G4Polyhedra("RPC1Glass", 
			phiStart,
			phiTotal,
			numSide,
			numZPlanes,
			NewZPlane,
			rInner,
			rOuter);	
      
      G4LogicalVolume *GlassLogical =
	new G4LogicalVolume(GlassSolid,
			    CGAGeometryManager::GetMaterial("pyrex"),
			    "RPC1glass", 
			    0, 0, 0);
      
      G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(.8,0,.2));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      GlassLogical->SetVisAttributes(VisAtt);

      //
      // build the gas gap
      //
      NewZPlane[0] = gas_thickness/2.;
      NewZPlane[1] = -NewZPlane[0];
      G4Polyhedra * GasSolid =
	new G4Polyhedra("RPC1Gas", 
			phiStart,
			phiTotal,
			numSide,
			numZPlanes,
			NewZPlane,
			rInner,
			rOuter);
      
      G4LogicalVolume *GasLogical =
	new G4LogicalVolume(GasSolid,
			    CGAGeometryManager::GetMaterial("RPCGAS1"),
			    "RPC1gas", 
			    0, 0, pULimits);
      
      VisAtt = new G4VisAttributes(G4Colour(.1,0,.8));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      GasLogical->SetVisAttributes(VisAtt);

      // PLugs the sensitive detector HERE!
      GasLogical->SetSensitiveDetector(theSD);

      //
#ifdef MOKKA_GEAR
      // get heighth of sensible part of layer for each layer
      helpEndcap.sensThickness.push_back( gas_thickness ) ;
      helpEndcap.gapThickness.push_back( spacer_thickness ) ;
#endif
      
      // placing the all. 
      // ZOffset starts pointing to the chamber border.
      G4double ZOffset = zPlane[0];
      
      // first glass after the g10_thickness
      ZOffset += g10_thickness + glass_thickness/2.;
      new MyPlacement(0,
		      G4ThreeVector(0,
				    0,
				    ZOffset),
		      GlassLogical,
		      "RPCGlass",
		      ChamberLog,
		      false,
		      0);
      
      // set ZOffset to the next first glass border
      ZOffset += glass_thickness/2.;
      
      // gas gap placing 
      ZOffset += gas_thickness/2.; // center !
      new MyPlacement(0,
		      G4ThreeVector(0,
				    0,
				    ZOffset),
		      GasLogical,
		      "RPCGas",
		      ChamberLog,
		      false,
		      0);

      // set ZOffset to the next gas gap border
      ZOffset += gas_thickness/2.;
      
      // second glass, after the g10_thickness, the first glass
      // and the gas gap.
      ZOffset += glass_thickness/2.; // center !
      new MyPlacement(0,
		      G4ThreeVector(0,
				    0,
				    ZOffset),
		      GlassLogical,
		      "RPCGlass",
		      ChamberLog,
		      false,
		      0);
      return ChamberLog;      
    }
  Control::Abort("Hcal04::BuildRPC1Polyhedra: invalid ChamberSolidEnvelope",MOKKA_OTHER_ERRORS);
  return NULL;  
}

void 
Hcal04::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
  // In this complex subdetector the best is to read
  // all hits in the one of the sensitive detectors,
  // just for visualisation, so just for your eyes...
  theBarrilRegSD->LoadEvent(theSubDetectorEventHitsFileInput);
}
