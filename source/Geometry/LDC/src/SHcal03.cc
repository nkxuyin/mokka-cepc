//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for ILC   -      *
//*                                                     *
//* For more information about Mokka, please, visit     *
//*                                                     *
//*  http://polywww.in2p3.fr:8081/MOKKA/                *
//*                                                     *
//*******************************************************
//
// $Id: SHcal03.cc,v 1.6 2007/12/21 11:16:36 frank Exp $
// $Name: mokka-07-00 $
//
//
// History:  
//
// initial version: 
// F.Gaede: identical to  Hcal03 driver except that an additional gap
//          for the fibres is introduced between scintilaltor and steel
// PMoradeFreitas: Super driver without tmp database and able
//          to build Hcal barrel with just two modules in stave.

#include "Control.hh"
#include "MySQLWrapper.hh"
#include "MyPlacement.hh"
#include "G4UserLimits.hh"
#include "SHcal03.hh"
#include "CGAGeometryManager.hh"
#include "SD.hh"
#include "HECSD.hh"

#include "globals.hh"
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

INSTANTIATE(SHcal03)
  

G4bool 
SHcal03::ContextualConstruct(const CGAGeometryEnvironment 
			     &aGeometryEnvironment,
			     G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding Hcal..." << G4endl;
  if (!Setup(aGeometryEnvironment)) return false;
  
  if(Control::DUMPG3) MyPlacement::Init("HCAL",GetName());
  
  //--------- BarrelHcal Sensitive detector -----

  // sensitive Model
  G4String SensitiveLog= "The sensitive model in Hcal chambers is " + Hcal_sensitive_model;
  Control::Log(SensitiveLog.data());
  
  
  // if RPC1 read the RPC parameters
  if (Hcal_sensitive_model == "RPC1")
    {
      // WARNING : SHOULD BE GLOBAL PARAMETERS!!!
      g10_thickness= 1;
      glass_thickness = 1;
      gas_thickness = 1.2;
      spacer_thickness = 0.6;
      spacer_gap = 50;
    }

  // The cell boundaries does not really exist as G4 volumes. So,
  // to avoid long steps over running  several cells, the 
  // theMaxStepAllowed inside the sensitive material is the
  // pad smaller x or z dimension.
  theMaxStepAllowed= std::min(Hcal_cell_dim_x,Hcal_cell_dim_z);
  
  //  Hcal  barrel regular modules
  theBarrilRegSD = 
    new SD(Hcal_cell_dim_x,
	   Hcal_cell_dim_z,
	   Hcal_chamber_tickness,
	   HCALBARREL,
	   "HcalBarrelReg");
  RegisterSensitiveDetector(theBarrilRegSD);
  

  // Hcal  endcap modules
  theENDCAPEndSD =
    new HECSD(Hcal_cell_dim_x,
	      Hcal_cell_dim_z,
	      Hcal_chamber_tickness,
	      HCALENDCAPMINUS,
	      "HcalEndCaps");
  RegisterSensitiveDetector(theENDCAPEndSD);
  
  if(Hcal_ring > 0 )
    {
      theENDCAPRingSD =
	new HECSD(Hcal_cell_dim_x,
		  Hcal_cell_dim_z,
		  Hcal_chamber_tickness,
		  HCALENDCAPMINUS,
		  "HcalEndCapRings");
      RegisterSensitiveDetector(theENDCAPRingSD);
    }
  
  // Set up the Radiator Material to be used
  if(Hcal_radiator_material == "Iron")
    RadiatorMaterial =  CGAGeometryManager::GetMaterial("iron");
  else
    if(Hcal_radiator_material == "WMod")
      RadiatorMaterial =  CGAGeometryManager::GetMaterial("tungsten_11gccm");
    else 
      Control::
	Abort("SHcal03: invalid radiator material name. \nIt has to be either Iron either WMod!!!",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
  
  Hcal_radiator_material+= " is the radiator material being placed.";
  Control::Log(Hcal_radiator_material.data());
  
  //----------------------------------------------------
  // Barrel
  //----------------------------------------------------
  Barrel(WorldLog);
  
  //----------------------------------------------------
  // EndCap Modules
  //----------------------------------------------------
  
  Endcaps(WorldLog);

  //----------------------------------------------------
  // EndCap Rings 
  //----------------------------------------------------
  
  if(Hcal_ring > 0)
    EndcapRings(WorldLog);
  
#ifdef MOKKA_GEAR
  //----------------------------------------------------
  // MokkaGear
  //----------------------------------------------------
  
  // get Manager
  MokkaGear* gearMgr = MokkaGear::getMgr() ;
  
  
  // get the information that are not yet included
  helpBarrel.phi0 = 0.;

  // acquire parameters from mokkaGearManager
  dblParamHalfZ                     = gearMgr->tmpParam.getDoubleVal( "TPC_Ecal_Hcal_barrel_halfZ" ) ;
  dblParamLateralStructureThickness = gearMgr->tmpParam.getDoubleVal( "Hcal_lateral_structure_thickness" ) ;
  dblParamStavesGap                 = gearMgr->tmpParam.getDoubleVal( "Hcal_stave_gaps" ) ;
  dblParamBackPlateThickness        = gearMgr->tmpParam.getDoubleVal( "Hcal_back_plate_thickness" ) ;

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
      (0, calcThick, Hcal_digi_cell_dim_z, Hcal_digi_cell_dim_x, calcAbsorb );
  }

  // write additional parameters
  barrelParam->setIntVal   ( "Hcal_barrel_end_module_type" ,  Hcal_barrel_end_module_type  ) ;
  barrelParam->setDoubleVal( "TPC_Ecal_Hcal_barrel_halfZ"  , dblParamHalfZ ) ;
  barrelParam->setDoubleVal( "Hcal_lateral_structure_thickness" , dblParamLateralStructureThickness ) ;
  barrelParam->setDoubleVal( "Hcal_modules_gap"            , dblParamModulesGap ) ;
  barrelParam->setDoubleVal( "Hcal_stave_gaps"             , dblParamStavesGap ) ;
  barrelParam->setDoubleVal( "Hcal_back_plate_thickness"   , dblParamBackPlateThickness ) ;
  barrelParam->setDoubleVal( "Hcal_virtual_cell_size"      , Hcal_cell_dim_x );
  
  // fg --- tell the user that the outer polygonal shape of the barrel has symmetry 16
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
      (0, calcThick, Hcal_digi_cell_dim_z, Hcal_digi_cell_dim_x, calcAbsorb);
    
  }

  // write Endcap parameters to GearManager
  endcapParam->setDoubleVal( "Hcal_virtual_cell_size"      , Hcal_cell_dim_x );
  gearMgr->setHcalEndcapParameters( endcapParam ) ;


  //HCAL Endcap Ring
 gear::CalorimeterParametersImpl * endcapRingParam =
    new gear::CalorimeterParametersImpl (helpEndcapRing.innerRadius, helpEndcapRing.outerRadius, helpEndcapRing.leastZ, 2, helpEndcapRing.phi0);

// write all layers by position
  for (int i=0; i < helpEndcapRing.count; i++) {
    
    G4double calcThick = helpEndcapRing.layerPos[i+1] - helpEndcapRing.layerPos[i] ;
    G4double calcAbsorb = calcThick - helpEndcapRing.sensThickness[i] - helpEndcapRing.gapThickness[i] ;  

    // on last layer, gap has to be taken into account
    if( i == ( helpEndcapRing.count - 1 ) ) {
      G4double layerGap = helpEndcapRing.layerPos[i] - helpEndcapRing.layerPos[i-1] - calcThick ;
      
      // check if layerGap makes sense
      if ( layerGap < calcThick ) {
	
	// add gap to Thickness
	calcThick = calcThick + layerGap ;
      }
    }

    endcapRingParam->layerLayout().positionLayer
      (0, calcThick, Hcal_digi_cell_dim_z, Hcal_digi_cell_dim_x, calcAbsorb);
  }
    // write EndcapRing parameters to GearManager
    endcapRingParam->setDoubleVal( "Hcal_virtual_cell_size", Hcal_cell_dim_x );
    gearMgr->setHcalRingParameters( endcapRingParam ) ;

#endif

  // Closes Database connection
  delete db;

  return true;
}
SHcal03::~SHcal03() 
{
  //  if(theBarrilRegSD!=0) delete theBarrilRegSD;
  //  if(theENDCAPEndSD!=0) delete theENDCAPEndSD;
}  


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              Barrel
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcal03::Barrel(G4LogicalVolume* MotherLog)
{
  BarrelRegularModules(MotherLog);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              Barrel Regular Modules               ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcal03::BarrelRegularModules(G4LogicalVolume* MotherLog)
{
  // Regular modules

  G4double BHX = Hcal_bottom_dim_x /2.;
  G4double MHX = Hcal_midle_dim_x / 2.;
  G4double THX = Hcal_top_dim_x / 2.;
  G4double YX1H = Hcal_y_dim1_for_x / 2.;
  G4double YX2H = Hcal_y_dim2_for_x / 2.;
  G4double DHZ = Hcal_normal_dim_z / 2.;
  
  G4double BottomDimY = YX1H;
  G4double chambers_y_off_correction = YX2H;
  
  // Attention: on bâtit le module dans la verticale
  // à cause du G4Trd et on le tourne avant de le positioner
  G4Trd * Bottom = new G4Trd("Bottom_Barrel_Module",
			     BHX, 
			     MHX,
			     DHZ,
			     DHZ,
			     YX1H);
  
  G4Trd * Top = new G4Trd("Top_Barrel_Module",
			  MHX, 
			  THX,
			  DHZ,
			  DHZ,
			  YX2H);
  
  G4UnionSolid* ModuleSolid = 
    new G4UnionSolid("ModuleSolid",
		     Bottom,
		     Top,
		     0,
		     G4ThreeVector(0,
				   0,
				   YX1H
				   + YX2H));
  
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
  G4double yTotal = YX1H + YX2H ;
  // add first position of layer as ground level
  helpBarrel.layerPos.push_back( -yTotal ) ;
#endif 
  
  //----------------------------------------------------
  // Chambers in the Hcal Barrel 
  //----------------------------------------------------

  BarrelRegularChambers(EnvLogHcalModuleBarrel,chambers_y_off_correction);

  // BarrelStandardModule placements
  // values retrieved from DB in old releases
  G4double stave_phi_offset, inner_radius, module_z_offset;
  inner_radius = Hcal_inner_radius;
  
  G4double Y;
  Y = inner_radius + BottomDimY;
  
#ifdef MOKKA_GEAR
  // starting value for iteration of inner_radius
  helpBarrel.innerRadius = Y ;
  G4double lastPosition = -1. ;
#endif
  
  stave_phi_offset = 0;  
  for (G4int stave_id = 1;
       stave_id <= 8;
       stave_id++)
    {
      module_z_offset = 
	- (Hcal_normal_dim_z + Hcal_modules_gap)/2.; 

    for (G4int module_id = 1;
	 module_id <=2;
	 module_id++)
      {
	G4double phirot = stave_phi_offset;
	G4RotationMatrix *rot=new G4RotationMatrix();
	rot->rotateX(pi*0.5); // on couche le module.
	rot->rotateY(phirot);
	new MyPlacement(rot,
			G4ThreeVector(-Y*sin(phirot),
				      Y*cos(phirot),
				      module_z_offset),
			EnvLogHcalModuleBarrel,
			"BarrelHcalModule",
			MotherLog,
			false,
			HCALBARREL*100+stave_id*10+
			module_id);
	theBarrilRegSD->SetStaveRotationMatrix(stave_id,phirot);
	theBarrilRegSD->
	  SetModuleZOffset(module_id,
			   module_z_offset);
	
#ifdef MOKKA_GEAR
	// get inner Radius as minimum of all occurances of inner_radius
	helpBarrel.innerRadius = 
	  std::min( helpBarrel.innerRadius,inner_radius);
	
	// find out the borders of construction in both dimensions
	G4double thisModuleOffset = module_z_offset;
	helpBarrel.mostZ = 
	  std::max( helpBarrel.mostZ , thisModuleOffset + DHZ) ;
	helpBarrel.leastZ= 
	  std::min( helpBarrel.leastZ, thisModuleOffset - DHZ) ;
	dblParamBarrelMostZ = helpBarrel.mostZ ;
	
	// to find out gap one compares the delta in position to 
	// the module size only if a last position exists
	if( !(lastPosition == -1.) ) {
	  G4double thisGap = 
	    std::abs( thisModuleOffset - lastPosition ) - 2*DHZ ;
	  dblParamModulesGap = 
	    std::max( dblParamModulesGap , thisGap ) ;
	}
	lastPosition = thisModuleOffset ;
#endif
	module_z_offset = - module_z_offset;
      }
    stave_phi_offset +=  pi/4;
    }
}

void SHcal03::BarrelRegularChambers(G4LogicalVolume* MotherLog,G4double chambers_y_off_correction)
{
  
  G4LogicalVolume * ChamberLog[200];
  G4Box * ChamberSolid;
  
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(.2,.8,.2));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  
  G4UserLimits* pULimits=
    new G4UserLimits(theMaxStepAllowed);
  
  G4double xdh,ydh,zdh;
  ydh = Hcal_chamber_tickness / 2.;
  zdh = Hcal_normal_dim_z/ 2. - 2 * Hcal_lateral_plate_thickness;
  
  G4double TanPiDiv8 = tan(pi/8.);
  
  for (G4int layer_id = 1;
       layer_id <= Hcal_nlayers;
       layer_id++)
    {
      if(layer_id *(Hcal_radiator_thickness + Hcal_chamber_tickness) 
	 < Hcal_y_dim1_for_x )
	xdh = 
	  (floor((Hcal_bottom_dim_x + 
		  2 * (layer_id * Hcal_radiator_thickness 
		       + (layer_id -1) * Hcal_chamber_tickness) 
		  * TanPiDiv8)
		 / Hcal_cell_dim_x) 
	   * Hcal_cell_dim_x)
	  / 2.;
      else
	xdh =
	  ((floor (( Hcal_midle_dim_x - 
		     2 *((layer_id * Hcal_radiator_thickness
			  + (layer_id -1) * Hcal_chamber_tickness)
			 - Hcal_y_dim1_for_x)
		     / TanPiDiv8)
		   - 2 * Hcal_chamber_tickness/TanPiDiv8
		   - 2 * Hcal_back_plate_thickness)
	    / Hcal_cell_dim_x)
	   * Hcal_cell_dim_x)
	  / 2.;
      
      ChamberSolid = 
	new G4Box("ChamberSolid", 
		  xdh,  //hx
		  zdh,  //hz attention!
		  ydh); //hy attention!
      
      if(Hcal_sensitive_model == "scintillator")
	{	
	  
	  //fg: introduce (empty) fiber gap - should be filled with fibres and cables
	  // - so far we fill it  with air ...
	  G4LogicalVolume *ChamberLogical =
	    new G4LogicalVolume(ChamberSolid,
				CGAGeometryManager::GetMaterial("air"), 
				"ChamberLogical", 
				0, 0, 0);
	  
	  // the scintillator width is the chamber width - fiber gap 
	  G4double fiber_gap = Hcal_fiber_gap;
	  
	  G4double scintHalfWidth =  ydh - fiber_gap / 2. ;
	  
	  // fiber gap can't be larger than total chamber
	  assert( scintHalfWidth > 0. ) ;
	  
	  G4Box * ScintSolid = 
	    new G4Box("ScintSolid", 
		      xdh,  //hx
		      zdh,  //hz attention!
		      scintHalfWidth ); //hy attention!
	  
	  G4LogicalVolume* ScintLog =
	    new G4LogicalVolume(ScintSolid,
				CGAGeometryManager::GetMaterial("polystyrene"),
				"ScintLogical", 
				0, 0, pULimits);  
	  
	  // only scinitllator is sensitive
	  ScintLog->SetSensitiveDetector(theBarrilRegSD);
	  
	  new MyPlacement(0, 
			  G4ThreeVector( 0, 0,  - fiber_gap / 2. ), 
			  ScintLog,
			  "Scintillator", 
			  ChamberLogical, 
			  false, 
			  layer_id );   
	  
	  ChamberLog [ layer_id ] = ChamberLogical ;
	  
#ifdef MOKKA_GEAR
	  // get heighth of sensible part of layer
	  helpBarrel.sensThickness.push_back( scintHalfWidth*2 ) ;
	  helpBarrel.gapThickness.push_back( fiber_gap ) ;
#endif
	  
	}
      else 
	if (Hcal_sensitive_model == "RPC1")
	  {
	    ChamberLog [layer_id] =
	      BuildRPC1Box(ChamberSolid,
			   theBarrilRegSD,
			   layer_id,
			   pULimits);
	  }
	else Control::Abort("Invalid sensitive model parameter!",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
      
      ChamberLog[layer_id]->SetVisAttributes(VisAtt);
      
      // Chamber Placements
      // module x and y offsets (needed for the SD)
      G4double Xoff,Yoff;
      Xoff = 0.;
      Yoff = Hcal_inner_radius + Hcal_total_dim_y/2.;

      G4double chamber_x_offset, chamber_y_offset,chamber_z_offset;
      chamber_x_offset = chamber_z_offset = 0;
      chamber_y_offset =
	-Hcal_total_dim_y/2. 
	+ (layer_id-1) *(Hcal_chamber_tickness + Hcal_radiator_thickness)
	+ Hcal_radiator_thickness 
	+ Hcal_chamber_tickness/2.;
      
      new MyPlacement(0,
		      G4ThreeVector(chamber_x_offset,
				    chamber_z_offset,
				    chamber_y_offset+
				    chambers_y_off_correction),
		      //!!attention!! y<->z
		      ChamberLog [layer_id],
		      "ChamberBarrel",
		      MotherLog,
		      false,
		      layer_id);
      theBarrilRegSD->
	AddLayer(layer_id,
		 chamber_x_offset + Xoff - 
		 ((G4Box *)ChamberLog[layer_id]->GetSolid())->GetXHalfLength(),
		 chamber_y_offset + Yoff,
		 chamber_z_offset - 
		 ((G4Box *)ChamberLog[layer_id]->GetSolid())->GetYHalfLength());    
      
#ifdef MOKKA_GEAR
      // count layers
      helpBarrel.count += 1;
      // get posoition for each layer
      // check for dynamic_cast
      G4Box * solidOfLog = 
	dynamic_cast<G4Box*>(ChamberLog[layer_id]->GetSolid());
      if ( solidOfLog == 0 ) 
	{
	  Control::Abort("BarrelChambers are expected to be of type G4Box\nerror in 'Hcal03::BarrelRegularChambers' construction MokkaGear.",MOKKA_OTHER_ERRORS);
	}
      helpBarrel.layerPos.push_back(chamber_y_offset 
				    + solidOfLog->GetZHalfLength() );
#endif
      
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              Endcaps                 ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcal03::Endcaps(G4LogicalVolume* MotherLog)
{
  // old parameters from database
  G4double pRMax, pDz, pRMin;
  pRMax = Hcal_endcap_rmax;

  // for the moment the endcap has the same structure as the barrel,
  // so the same thickness.
  pDz = Hcal_total_dim_y / 2.;
  pRMin = Hcal_endcap_center_box_size / 2.;
  
  G4double zPlane[2];
  zPlane[0]=-pDz;
  zPlane[1]=-zPlane[0];

  G4double rInner[2],rOuter[2];
  rInner[0]=rInner[1]=pRMin;
  rOuter[0]=rOuter[1]=pRMax;

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
		    8,
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
  EndcapChambers(EndCapLogical,theENDCAPEndSD,false);

  // Placements
  G4double endcap_z_offset = Hcal_start_z + Hcal_total_dim_y/2. ;
  G4RotationMatrix *rotEffect=new G4RotationMatrix();
  rotEffect->rotateZ(pi/8.);
  G4int ModuleNumber = HCALENDCAPPLUS*100+16;
  G4double Z1=0;
  for (G4int endcap_id = 1;
       endcap_id <= 2;
       endcap_id++)
    {
      Z1=endcap_z_offset;
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
      endcap_z_offset = - endcap_z_offset;
    }
  theENDCAPEndSD->
    SetModuleZOffset(0,
		     fabs(Z1));
  theENDCAPEndSD->
    SetModuleZOffset(6,
		     fabs(Z1));
}

void SHcal03::EndcapChambers(G4LogicalVolume* MotherLog,
			     HECSD* theSD, bool rings)
{
  // Chambers in the SHcal03::Endcaps
  // standard endcap chamber solid:
  G4Polyhedra *motherSolid =
    (G4Polyhedra*) MotherLog->GetSolid();
  
  G4PolyhedraHistorical* motherPolyhedraParameters =
    motherSolid->GetOriginalParameters();
  
  G4double pRMax, pDz, fiber_gap, pRMin;

  pRMax = *(motherPolyhedraParameters->Rmax) 
    * cos(pi/motherPolyhedraParameters->numSide)
    - Hcal_lateral_plate_thickness;
  pDz = Hcal_chamber_tickness / 2.;
  pRMin = *(motherPolyhedraParameters->Rmin)
    * cos(pi/motherPolyhedraParameters->numSide)
    + Hcal_lateral_plate_thickness;

  //  G4cout << "pDz = " << pDz << G4endl;

  fiber_gap = Hcal_fiber_gap;
  
  // G4Polyhedra Envelope parameters
  G4double phiStart = 0.;
  G4double phiTotal = 360.;
  //  G4int numSide = 32;
  G4int numSide = motherPolyhedraParameters->numSide;
  G4int numZPlanes = 2;
  
  G4double zPlane[2];
  zPlane[0]=-pDz;
  zPlane[1]=-zPlane[0];

  G4double rInner[2],rOuter[2];
  rInner[0]=rInner[1]=pRMin;
  rOuter[0]=rOuter[1]=pRMax;

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
  
  if(Hcal_sensitive_model == "scintillator")
    {
      //fg: introduce (empty) fiber gap - should be filled with fibres and cables
      // - so far we fill it  with air ...
      EndCapChamberLogical =
	new G4LogicalVolume(EndCapChamberSolid,
			    CGAGeometryManager::GetMaterial("air"), 
			    "EndCapChamberLogical", 
			    0, 0, 0);
      
      G4double scintHalfWidth =pDz - fiber_gap  / 2. ;
      
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
      ScintLog->SetSensitiveDetector(theSD);
      
#ifdef MOKKA_GEAR
      // thickness of sensible part as often as zPlanes are there
      if(rings==true){
      helpEndcapRing.sensThickness.push_back( scintHalfWidth*2 );
      helpEndcapRing.gapThickness.push_back( fiber_gap ) ;
      }
      else{
      helpEndcap.sensThickness.push_back( scintHalfWidth*2 ) ;
      helpEndcap.gapThickness.push_back( fiber_gap );
      }
#endif
      
      
      new MyPlacement(0, G4ThreeVector( 0,0,  - fiber_gap / 2.  ), ScintLog,
		      "EndCapScintillator", EndCapChamberLogical, false, 0 );   
    }
  else 
    if (Hcal_sensitive_model == "RPC1")
      {
	EndCapChamberLogical =
	  BuildRPC1Polyhedra(EndCapChamberSolid,
			     theSD, rings,
			     phiStart,
			     phiTotal,
			     numSide,
			     numZPlanes,
			     zPlane,
			     rInner,
			     rOuter,
			     pULimits);
      }
    else Control::Abort("Invalid sensitive model parameter!",
		MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
  
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(1.,1.,1.));
  EndCapChamberLogical->SetVisAttributes(VisAtt);
  
  
  // chamber placements

  G4int number_of_chambers = Hcal_nlayers;
  G4int possible_number_of_chambers = (G4int) 
    floor(2*abs(*(motherPolyhedraParameters->Z_values)) /
	  (Hcal_chamber_tickness + Hcal_radiator_thickness));
  if(possible_number_of_chambers < number_of_chambers)
    number_of_chambers = possible_number_of_chambers;
  
  for (G4int layer_id = 1;
       layer_id <= number_of_chambers;
       layer_id++)
    {
      G4double Zoff = - abs(*(motherPolyhedraParameters->Z_values))
	+ (layer_id-1) *(Hcal_chamber_tickness + Hcal_radiator_thickness)
	+ Hcal_radiator_thickness 
	+ Hcal_chamber_tickness/2.;
      
      new MyPlacement(0,
		      G4ThreeVector(0.,
				    0.,
				    Zoff),
		      EndCapChamberLogical,
		      "EndCapChamberPhys",
		      MotherLog,
		      false,
		      layer_id);
      theSD->
	AddLayer(layer_id,
		 0,
		 0,
		 Zoff);
      
#ifdef MOKKA_GEAR
      // count the layers
      if(rings==true){
	helpEndcapRing.count += 1;

	// position of layer as offset + half thickness
	helpEndcapRing.layerPos.push_back(Zoff + std::abs(zPlane[0]) ) ;

	// sensible Area is the same every time
	// value set in 0 within construction of Scint or RPC
	helpEndcapRing.sensThickness.push_back( helpEndcapRing.sensThickness[0] );
	helpEndcapRing.gapThickness.push_back( helpEndcapRing.gapThickness[0] );   
      }
      else {
	helpEndcap.count += 1;

	// position of layer as offset + half thickness
	helpEndcap.layerPos.push_back(Zoff + std::abs(zPlane[0]) ) ;
      
	// sensible Area is the same every time
	// value set in 0 within construction of Scint or RPC
	helpEndcap.sensThickness.push_back( helpEndcap.sensThickness[0] );
	helpEndcap.gapThickness.push_back( helpEndcap.gapThickness[0] );
      }
      
#endif
      
    }  
}

G4LogicalVolume * 
SHcal03::BuildRPC1Box(G4Box* ChamberSolid, 
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

  Control::Abort("SHcal03::BuildRPC1Box: invalid ChamberSolidEnvelope",
		MOKKA_OTHER_ERRORS);
  return NULL;
}

G4LogicalVolume * 
SHcal03::BuildRPC1Polyhedra(G4Polyhedra* ChamberSolid, 
			   SD* theSD,
			   bool rings,
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
      if(rings==true){
	helpEndcapRing.sensThickness.push_back( gas_thickness ) ;
	helpEndcapRing.gapThickness.push_back( spacer_thickness ) ;
      }
      else{
	helpEndcap.sensThickness.push_back( gas_thickness ) ;
	helpEndcap.gapThickness.push_back( spacer_thickness ) ;
      }
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
  Control::Abort("SHcal03::BuildRPC1Polyhedra: invalid ChamberSolidEnvelope",
		MOKKA_OTHER_ERRORS);
  return NULL;  
}

void 
SHcal03::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
  // In this complex subdetector the best is to read
  // all hits in the one of the sensitive detectors,
  // just for visualisation, so just for your eyes...
  theBarrilRegSD->LoadEvent(theSubDetectorEventHitsFileInput);
}

G4bool 
SHcal03::Setup(const CGAGeometryEnvironment &theGeometryEnvironment)
{
  Hcal_radiator_thickness =
    theGeometryEnvironment.GetParameterAsDouble("Hcal_radiator_thickness");
  Hcal_radiator_material =
    theGeometryEnvironment.GetParameterAsString("Hcal_radiator_material");
  Hcal_ring = 
    theGeometryEnvironment.GetParameterAsInt("Hcal_ring");
  Hcal_radial_ring_inner_gap =
    theGeometryEnvironment.GetParameterAsInt("Hcal_radial_ring_inner_gap");
  Hcal_sensitive_model = 
    theGeometryEnvironment.GetParameterAsString("Hcal_sensitive_model");
  Hcal_back_plate_thickness = 
    theGeometryEnvironment.GetParameterAsDouble("Hcal_back_plate_thickness");
  Hcal_stave_gaps = 
    theGeometryEnvironment.GetParameterAsDouble("Hcal_stave_gaps");
  Hcal_modules_gap = 
    theGeometryEnvironment.GetParameterAsDouble("Hcal_modules_gap");
  Hcal_nlayers =
    theGeometryEnvironment.GetParameterAsInt("Hcal_nlayers");
  Hcal_barrel_end_module_type =
    theGeometryEnvironment.GetParameterAsInt("Hcal_barrel_end_module_type");
  Hcal_fiber_gap =
    theGeometryEnvironment.GetParameterAsDouble("Hcal_fiber_gap");
  
  // For the moment the chamber thickness cannot be changed because the
  // rpc1 table should follow.
  Hcal_chamber_tickness = 6.5;

  Ecal_endcap_zmin =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_endcap_zmin");

  Ecal_endcap_outer_radius =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_endcap_outer_radius");

  Hcal_inner_radius = 
    theGeometryEnvironment.GetParameterAsDouble("Ecal_outer_radius")
    + theGeometryEnvironment.GetParameterAsDouble("Hcal_Ecal_gap");
  Hcal_endcap_cables_gap =
    theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_cables_gap");
  Hcal_endcap_ecal_gap =
    theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_ecal_gap");

  TPC_Ecal_Hcal_barrel_halfZ = 
    theGeometryEnvironment.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");

  if(Hcal_barrel_end_module_type == 1 )
    {
      // just two modules per stave
      Hcal_normal_dim_z = 
	(2 * TPC_Ecal_Hcal_barrel_halfZ - Hcal_modules_gap)/2.;
      
      Hcal_top_end_dim_z = 1180.0000;
      
      Hcal_start_z =  Hcal_normal_dim_z 
	+ Hcal_modules_gap / 2. + Hcal_endcap_cables_gap;
    }
  else
    {
      // the 140 and the proportions comes from Tesla TDR
      G4double total_z_size = 140 +
	2 * TPC_Ecal_Hcal_barrel_halfZ;
      G4double Hcal_normal_dim_z = total_z_size / 5. * 1080./1120.;
      G4double Hcal_top_end_dim_z = 
	(total_z_size - 3 * Hcal_normal_dim_z)/2;
      G4cout << "regular barrel module z size = " << Hcal_normal_dim_z
	     << ", end barrel z size = " << Hcal_top_end_dim_z << G4endl;
      Hcal_start_z =  
	1.5 * Hcal_normal_dim_z 
	+ Hcal_top_end_dim_z + 
	2 * Hcal_modules_gap + 
	Hcal_endcap_cables_gap;
    }
  // Hcal_start_z is the Hcal Endcap boundary coming from the IP
  // Test Hcal_start_z against Ecal_endcap_zmax + Hcal_endcap_ecal_gap
  // to avoid overlap problems with Ecal if scaled.
  //
  Ecal_endcap_zmax = 
    theGeometryEnvironment.GetParameterAsDouble("Ecal_endcap_zmax");

  if( Hcal_start_z < Ecal_endcap_zmax + Hcal_endcap_ecal_gap )
    Hcal_start_z = Ecal_endcap_zmax + Hcal_endcap_ecal_gap;

  Hcal_lateral_plate_thickness =
    theGeometryEnvironment.
    GetParameterAsDouble("Hcal_lateral_structure_thickness");

  Hcal_cells_size = 
    theGeometryEnvironment.GetParameterAsDouble("Hcal_cells_size");
  Hcal_digi_cells_size = 
    theGeometryEnvironment.GetParameterAsDouble("Hcal_digitization_tile_size");

  Hcal_endcap_center_box_size =
    theGeometryEnvironment.
    GetParameterAsDouble("Hcal_endcap_center_box_size");

  // general calculated parameters
  Hcal_total_dim_y = 
    Hcal_nlayers * (Hcal_radiator_thickness +Hcal_chamber_tickness) 
    + Hcal_back_plate_thickness;
  Hcal_module_radius = 
    Hcal_inner_radius + Hcal_total_dim_y;

  // y_dim2_for_x becomes calculed!
  //query = "set  @y_dim2_for_x = 213.9;";  
  Hcal_y_dim2_for_x = (Hcal_module_radius - Hcal_module_radius*cos(pi/8));
  Hcal_y_dim1_for_x = Hcal_total_dim_y - Hcal_y_dim2_for_x;
  Hcal_bottom_dim_x = 2.*Hcal_inner_radius*tan(pi/8.)- Hcal_stave_gaps;
  Hcal_midle_dim_x = Hcal_bottom_dim_x + 2* Hcal_y_dim1_for_x*tan(pi/8.);
  Hcal_top_dim_x = Hcal_midle_dim_x - 2 * Hcal_y_dim2_for_x/tan(pi/8.);

//   Hcal_endcap_rmax =
//     Hcal_inner_radius 
//     + Hcal_total_dim_y;
  
  Hcal_endcap_rmax =
    Hcal_inner_radius 
    + Hcal_y_dim1_for_x;
  
  // the  y_dim1_for_z kept as the original value in TDR
  Hcal_y_dim1_for_z = 134.8;
  Hcal_y_dim2_for_z = 
    (Hcal_top_end_dim_z-Hcal_normal_dim_z) * 0.8179077682; // tan(RADIANS(39.28))
  Hcal_y_dim3_for_z = 
    Hcal_total_dim_y - Hcal_y_dim1_for_z - Hcal_y_dim2_for_z;
  Hcal_regular_chamber_dim_z = 
    Hcal_normal_dim_z - 2 *(Hcal_lateral_plate_thickness);
  Hcal_cell_dim_x = Hcal_cells_size;
  Hcal_cell_dim_z =  
    Hcal_regular_chamber_dim_z 
    / floor (Hcal_regular_chamber_dim_z/Hcal_cell_dim_x);
  Hcal_digi_cell_dim_x = Hcal_digi_cells_size;
  Hcal_digi_cell_dim_z =  
    Hcal_regular_chamber_dim_z 
    / floor (Hcal_regular_chamber_dim_z/Hcal_digi_cell_dim_x);
  
#ifdef MOKKA_GEAR
  MokkaGear* mokkaGearMgr = MokkaGear::getMgr() ;

  mokkaGearMgr->tmpParam.setDoubleVal(  "TPC_Ecal_Hcal_barrel_halfZ" , theGeometryEnvironment.GetParameterAsDouble(  "TPC_Ecal_Hcal_barrel_halfZ" ) ) ;

  mokkaGearMgr->tmpParam.setDoubleVal( "Hcal_lateral_structure_thickness" , theGeometryEnvironment.GetParameterAsDouble( "Hcal_lateral_structure_thickness" ) ) ;			      

  mokkaGearMgr->tmpParam.setDoubleVal( "Hcal_stave_gaps" , theGeometryEnvironment.GetParameterAsDouble( "Hcal_stave_gaps" ) ) ;

  mokkaGearMgr->tmpParam.setDoubleVal( "Hcal_back_plate_thickness" , theGeometryEnvironment.GetParameterAsDouble( "Hcal_back_plate_thickness" ) ) ;
#endif

  if( Hcal_barrel_end_module_type != 1)
    Control::Abort("Sorry, but Tesla like end modules in barrel not yet available with this driver.", MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);

  return true;

}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              EndcapRings                 ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcal03::EndcapRings(G4LogicalVolume* MotherLog)
{
  // old parameters from database
  G4double pRMax, pDz, pRMin;
  pRMax = Hcal_endcap_rmax;

  // The rings start from inner Ecal endcap boundary
  // and finishes at inner Hcal endcap one.

  G4double start_z, stop_z;
  start_z = Ecal_endcap_zmin;

  G4double SpaceForLayers = Hcal_start_z 
    - Hcal_endcap_ecal_gap 
    - Ecal_endcap_zmin
    - 2 * Hcal_lateral_plate_thickness;
  
  G4int MaxNumberOfLayers = 
    (G4int) (SpaceForLayers / 
	     (Hcal_chamber_tickness + Hcal_radiator_thickness));

  G4cout << "Rings will have "
	 << MaxNumberOfLayers
	 << " layers."
	 << G4endl;
  
  stop_z = start_z 
    + MaxNumberOfLayers
    * (Hcal_chamber_tickness + Hcal_radiator_thickness)
    + 2 * Hcal_lateral_plate_thickness;

  pDz = (stop_z - start_z) / 2.;

  pRMin = Ecal_endcap_outer_radius 
    + Hcal_radial_ring_inner_gap;
  
  G4double zPlane[2];
  zPlane[0]=-pDz;
  zPlane[1]=-zPlane[0];
  
  G4double rInner[2],rOuter[2];
  rInner[0]=rInner[1]=pRMin;
  rOuter[0]=rOuter[1]=pRMax;
  
#ifdef MOKKA_GEAR
//   // Write parameters in helper class
//   // attention: the outer symmetrie is 32-fold... this is not
//   // taken into account in gear
  
//   // the outer radius is in Geant4 defined as the tangent outer radius.
//   // In Gear description differs. We will take the heigth of the triangle
//   // in account as well in order to get the max radius.

  helpEndcapRing.outerRadius = rOuter[0] * cos( pi/9 );  // outer radius (triangle)
  helpEndcapRing.innerRadius = rInner[0] ;              // inner radius
  helpEndcapRing.phi0 = 0. ;                             // phi0
  
  // set starting value for inner_z
  // it should _not_ stay at this value
  helpEndcapRing.leastZ = 999999999. ;
#endif

  G4Polyhedra *EndCapSolid=
    new G4Polyhedra("HcalEndCapRingSolid",
		    0.,
		    360.,
		    //32,
		    8,
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
			"EndCapRingLogical",
			0, 0, 0);
  EndCapLogical->SetVisAttributes(VisAtt);
  
#ifdef MOKKA_GEAR
  // add first position of layer as ground level
  helpEndcapRing.layerPos.push_back( zPlane[0] ) ;
#endif

//   // build and place the chambers in the Hcal EndcapRings

  EndcapChambers(EndCapLogical,theENDCAPRingSD,true);


  // Placements
  G4double endcap_z_offset = Ecal_endcap_zmin + pDz;
  G4RotationMatrix *rotEffect=new G4RotationMatrix();
  rotEffect->rotateZ(pi/8.);
  G4int ModuleNumber = HCALENDCAPPLUS*100+16;
  G4double Z1=0;
  for (G4int endcap_id = 1;
       endcap_id <= 2;
       endcap_id++)
    {
      Z1=endcap_z_offset;
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
      helpEndcapRing.leastZ = std::min( helpEndcapRing.leastZ, std::abs(Z1)-std::abs(zPlane[0]) );

#endif 

       endcap_z_offset = - endcap_z_offset;
    }

  theENDCAPRingSD->
    SetModuleZOffset(0,
		     fabs(Z1));
  theENDCAPRingSD->
    SetModuleZOffset(6,
		     fabs(Z1));
}

G4bool 
SHcal03::PostConstructAction(CGAGeometryEnvironment& )
{
  //
  // Propagates the changes to Coil, if any. The SHcal has also the responsability 
  // to change the calorimeter region parameters.
  //
  G4double Hcal_R_max =
    (Hcal_y_dim1_for_x +  Hcal_y_dim2_for_x +
     Hcal_inner_radius)/cos(pi/16);
  std::ostringstream oss1;
  oss1 << Hcal_R_max;
  (*Control::globalModelParameters)["Hcal_R_max"] =
    oss1.str();
  (*Control::globalModelParameters)["calorimeter_region_rmax"] =
    oss1.str();

  G4double Hcal_outer_radius =
    Hcal_inner_radius + Hcal_total_dim_y;
  
  G4double calorimeter_region_zmax =
    Hcal_start_z + Hcal_total_dim_y;
  std::ostringstream oss3;  
  oss3 << calorimeter_region_zmax;
  (*Control::globalModelParameters)["calorimeter_region_zmax"] =
    oss3.str();
 			    

  G4cout << "SHcal information: Hcal_outer_radius = "
	 << Hcal_outer_radius
	 << "\n                   module thickness = "
	 << Hcal_total_dim_y
	 << "\n                   Hcal_R_max = "
    	 << Hcal_R_max
	 << "\n                   calorimeter_region_rmax = "
	 << Hcal_R_max
	 << "\n                   calorimeter_region_zmax = "
	 << calorimeter_region_zmax
	 << G4endl;

  return true;    
}
