/*******************************************************/
/*                                                     */
/*                      Mokka                          */ 
/*   - the detailed geant4 simulation for ILC -        */
/*					               */  
/* For more information about Mokka, please, go to the */
/* Mokka home page:                                    */
/*                                                     */
/* http://polzope.in2p3.fr:8081/MOKKA                  */
/*                                                     */
/*******************************************************/

 /* History:  
  
   initial version: 
   F.Gaede: identical to  Hcal03 driver except that an additional gap
            for the fibres is introduced between scintillator and steel
   PMoradeFreitas: Super driver without tmp database and able
                   to build Hcal barrel with just two modules in stave.
   Angela Lucaci: similar to SHcal03, just that the drift chambers option is 
                  not considered anymore, only the scintillator one. In addition,
                  a gap in the middle of the stave is build, and fractional cells
                  at the edges are introduced (see SDHcalBarrel.cc)
   Ralf Diener: Corrected use of GEAR interface
   Andre Sailer: Added Tungsten
   Andre Sailer: Added possibility for different endcap/barrel material
                 Which needs three additional parameters: 
		 endcap_radiator_thickness, endcap_radiator_material, endcap_layers
                 Also added parameters to the database for this driver
 */

#include "Control.hh"
#include "MySQLWrapper.hh"
#include "MyPlacement.hh"
#include "G4UserLimits.hh"
#include "SHcalSc02.hh"
#include "CGAGeometryManager.hh"
#include "SDHcalBarrel.hh"
#include "SDHcalEndCap.hh"
#include "SDHcalEndCapTesla.hh"

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
#include "G4RotationMatrix.hh"
#include "G4IntersectionSolid.hh"

#include <algorithm>
#include <assert.h>

#include "CGADefs.h"

#ifdef MOKKA_GEAR
#include "gear/CalorimeterParameters.h" 
#include "gearimpl/CalorimeterParametersImpl.h" 
#include "gearimpl/LayerLayoutImpl.h" 
#include "MokkaGear.h"
#endif

  //#define SHCALSC02_DEBUG

INSTANTIATE(SHcalSc02)

  G4bool SHcalSc02::ContextualConstruct(const CGAGeometryEnvironment 
					&aGeometryEnvironment,
					G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding Hcal..." << G4endl;
  if (!Setup(aGeometryEnvironment)) return false;

  if(Control::DUMPG3) MyPlacement::Init("HCAL",GetName());

  /*--------- BarrelHcal Sensitive detector -----*/
  /* sensitive Model*/
  G4String SensitiveLog= "The sensitive model in Hcal chambers is " + Hcal_sensitive_model;
  Control::Log(SensitiveLog.data());

  /* The cell boundaries does not exist as G4 volumes. So,
     to avoid long steps over running  several cells, the 
     theMaxStepAllowed inside the sensitive material is the
     minimum between x- and z-dimension of the cell
  */
  theMaxStepAllowed = std::min(Hcal_cell_dim_x, Hcal_cell_dim_z);

  /* Set up the Radiator (absorber) Material to be used*/
  if(Hcal_radiator_material == "Iron")
    BarrelRadiatorMaterial =  CGAGeometryManager::GetMaterial("iron");
  else if(Hcal_radiator_material == "WMod")
    BarrelRadiatorMaterial =  CGAGeometryManager::GetMaterial("tungsten_11gccm");
  else if(Hcal_radiator_material == "TungstenDens24")
    BarrelRadiatorMaterial =  CGAGeometryManager::GetMaterial("TungstenDens24");
  else 
    Control::Abort("SHcalSc02: invalid radiator material name. \nIt has to be either Iron either WMod!!!",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);

  Hcal_radiator_material+= " is the radiator material being placed.";
  Control::Log(Hcal_radiator_material.data());


  if(Hcal_endcap_radiator_material == "Iron")
    EndcapRadiatorMaterial =  CGAGeometryManager::GetMaterial("iron");
  else if(Hcal_endcap_radiator_material == "WMod")
    EndcapRadiatorMaterial =  CGAGeometryManager::GetMaterial("tungsten_11gccm");
  else if(Hcal_endcap_radiator_material == "TungstenDens24")
    EndcapRadiatorMaterial =  CGAGeometryManager::GetMaterial("TungstenDens24");
  else
    Control::Abort("SHcalSc02: invalid EndcapRadiator material name. \nIt has to be either Iron either WMod or TungstenDens24!!!",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);

  Hcal_endcap_radiator_material+= " is the endcap radiator material being placed.";
  Control::Log(Hcal_endcap_radiator_material.data());



  //=====================================================
  //                                                   //
  //  HCAL barrel regular modules                      //
  //                                                   //
  //=====================================================
  //Barrel sensitive detector
  theBarrilRegSD = new SDHcalBarrel(Hcal_cell_dim_x,
				    Hcal_cell_dim_z,
				    Hcal_scintillator_thickness,
				    HCALBARREL,
				    "HcalBarrelReg",
				    (Hcal_middle_stave_gaps/2 + Hcal_layer_support_length + Hcal_layer_air_gap),
				    Hcal_apply_Birks_law);
  RegisterSensitiveDetector(theBarrilRegSD);

  if (Hcal_sensitive_model == "scintillator")
    {	
      //draw the HCAL barrel
      BarrelRegularModules(WorldLog);
    } 
  else Control::Abort("SHcalSc02: Invalid sensitive model for the chosen HCAL superdriver!",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
  
  //====================================================
  //                                                  //
  // HCAL endcap modules                              //
  //                                                  //
  //====================================================
  // Hcal  endcap modules
  theENDCAPEndSD = new SDHcalEndCapTesla(Hcal_cell_dim_x,
					 Hcal_scintillator_thickness,//inverse, due to definition in SD (which is made for barrel, ie. layers perpendicular on z)
					 Hcal_cell_dim_x,//really Hcal_cell_dim_x !!! cell should be a square in the endcaps
					 HCALENDCAPMINUS,
					 "HcalEndCaps",
					 Hcal_apply_Birks_law
					 );
  RegisterSensitiveDetector(theENDCAPEndSD);

  if (Hcal_sensitive_model == "scintillator")
    {	
      //draw the HCAL endcaps
      EndcapsTesla(WorldLog);
    }
  else Control::Abort("SHcalSc02: Invalid sensitive model for the chosen HCAL superdriver!",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
  
  
  //====================================================
  //                                                  //
  // HCAL endcap rings                                //
  //                                                  //
  //====================================================
  if(Hcal_ring > 0 )
    {
      theENDCAPRingSD = new SDHcalEndCap(Hcal_cell_dim_x,
					 Hcal_cell_dim_x,//really Hcal_cell_dim_x !!! cell should be a square in the endcaps
					 Hcal_scintillator_thickness,
					 HCALENDCAPMINUS,
					 "HcalEndCapRings",
					 Hcal_stave_gaps, 
					 Hcal_endcap_sensitive_center_box,
					 Hcal_apply_Birks_law
					 );
      RegisterSensitiveDetector(theENDCAPRingSD);

      //draw the HCAL endcap rings
      EndcapRings(WorldLog);         
    }



  //====================================================
  //                                                  //
  // MOKKA GEAR                                       //
  //                                                  //
  //====================================================
#ifdef MOKKA_GEAR
  // get Manager
  MokkaGear* gearMgr = MokkaGear::getMgr() ;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  //                
  // GEAR information for the HCAL BARREL     
  //
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // get the information that are not yet included
  helpBarrel.phi0 = 0.;

  // acquire parameters from mokkaGearManager
  dblParamBackPlateThickness            = gearMgr->tmpParam.getDoubleVal( "Hcal_back_plate_thickness" );
  dblParamFiberGap                      = gearMgr->tmpParam.getDoubleVal( "Hcal_fiber_gap" );
  dblParamLayerSupportLength            = gearMgr->tmpParam.getDoubleVal( "Hcal_layer_support_length" );
  dblParamLayerAirGap                   = gearMgr->tmpParam.getDoubleVal( "Hcal_layer_air_gap" );
  dblParamMiddleStaveGaps               = gearMgr->tmpParam.getDoubleVal( "Hcal_middle_stave_gaps" ) ;
  dblParamHcalModulesGap                = gearMgr->tmpParam.getDoubleVal( "Hcal_modules_gap" );
  dblParamHcalStaveGaps                 = gearMgr->tmpParam.getDoubleVal( "Hcal_stave_gaps" );
  dblParamTPCEcalHcalbarrelHalfZ        = gearMgr->tmpParam.getDoubleVal( "TPC_Ecal_Hcal_barrel_halfZ" );
  dblParamHcalLateralStructureThickness = gearMgr->tmpParam.getDoubleVal( "Hcal_lateral_structure_thickness" );
  dblHcalBarrelEndModuleType            = gearMgr->tmpParam.getIntVal( "Hcal_barrel_end_module_type" );
  dblParamHcalEndcapSensitiveCenterBox  = gearMgr->tmpParam.getDoubleVal( "Hcal_endcap_sensitive_center_box" );

  // calculate zMax as total length/2
  helpBarrel.zMax = (helpBarrel.mostZ - helpBarrel.leastZ) / 2 ;

  // HCAL Barrel
  gear::CalorimeterParametersImpl* barrelParam = 
    new gear::CalorimeterParametersImpl( helpBarrel.innerRadius, helpBarrel.zMax, 8, helpBarrel.phi0 );

  // write all layers by position
  for (int i=0; i < helpBarrel.count; i++) 
    {
      G4double calcThick  = helpBarrel.layerPos[i+1] - helpBarrel.layerPos[i] ;
      G4double calcAbsorb = calcThick - helpBarrel.sensThickness[i] - helpBarrel.gapThickness[i] ;
      
      // on last layer, gap has to be taken into account
      if( i == ( helpBarrel.count - 1 ) ) {
	G4double layerGap = helpBarrel.layerPos[i] - helpBarrel.layerPos[i-1] - calcThick ;
	
	// check if layerGap makes sense
	if ( layerGap < calcThick ) 
	  {
	    // add gap to Thickness
	    calcThick = calcThick + layerGap ;
	  }
      }
      
      barrelParam->layerLayout().positionLayer(0,                   //distance
					       calcThick,           //thickness 
					       Hcal_digi_cell_dim_z,//cellSize0
					       Hcal_digi_cell_dim_x,//cellSize1
					       //helpBarrel.fractCellSize1, //size 1 (x) of the fractional cell
					       calcAbsorb );//absorber thickness
    }

  // write additional parameters
  barrelParam->setDoubleVal( "Hcal_back_plate_thickness",        dblParamBackPlateThickness ) ;
  barrelParam->setDoubleVal( "Hcal_fiber_gap",                   dblParamFiberGap );
  barrelParam->setDoubleVal( "Hcal_layer_support_length",        dblParamLayerSupportLength);
  barrelParam->setDoubleVal( "Hcal_layer_air_gap",               dblParamLayerAirGap );
  barrelParam->setDoubleVal( "Hcal_middle_stave_gaps",           dblParamMiddleStaveGaps );
  barrelParam->setDoubleVal( "Hcal_modules_gap" ,                dblParamHcalModulesGap );
  barrelParam->setDoubleVal( "TPC_Ecal_Hcal_barrel_halfZ",       dblParamTPCEcalHcalbarrelHalfZ );
  barrelParam->setDoubleVal( "Hcal_virtual_cell_size",           Hcal_cell_dim_x );
  barrelParam->setDoubleVal( "Hcal_stave_gaps"  ,                dblParamHcalStaveGaps );              
  barrelParam->setDoubleVal( "Hcal_lateral_structure_thickness", dblParamHcalLateralStructureThickness );
  barrelParam->setIntVal(    "Hcal_barrel_end_module_type",      dblHcalBarrelEndModuleType );


  // tell the user that the outer polygonal shape of the barrel has symmetry 16
  barrelParam->setIntVal   ( "Hcal_outer_polygon_order" ,  16  ) ;
  barrelParam->setIntVal   ( "Hcal_outer_polygon_phi0" ,  0  ) ;

  // write Barrel parameters to GearManager
  gearMgr->setHcalBarrelParameters( barrelParam ) ;  



  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  //                
  // GEAR information for the HCAL ENDCAPS     
  //
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  gear::CalorimeterParametersImpl * endcapParam =
    new gear::CalorimeterParametersImpl (helpEndcap.innerRadius, helpEndcap.outerRadius, 
					 helpEndcap.leastZ, 2, helpEndcap.phi0);

  G4cout<<"\n\n\n =========================================="<<G4endl;
  // write all layers by position
  for (int i=0; i < helpEndcap.count; i++) 
    {
      G4double calcThick = helpEndcap.layerPos[i+1] - helpEndcap.layerPos[i] ;
      G4double calcAbsorb = calcThick - helpEndcap.sensThickness[i] - helpEndcap.gapThickness[i] ;  

      // on last layer, gap has to be taken into account
      if( i == ( helpEndcap.count - 1 ) ) 
	{
	  G4double layerGap = helpEndcap.layerPos[i] - helpEndcap.layerPos[i-1] - calcThick ;
	  
	  // check if layerGap makes sense
	  if ( layerGap < calcThick ) 
	    {
	      // add gap to Thickness
	      calcThick = calcThick + layerGap ;
	    }
	}
      
      endcapParam->layerLayout().positionLayer
	(0, calcThick, Hcal_cell_dim_x, Hcal_cell_dim_x, calcAbsorb);
      
    }

  // write Endcap parameters to GearManager
  endcapParam->setDoubleVal( "Hcal_virtual_cell_size", Hcal_cell_dim_x );
  endcapParam->setDoubleVal( "Hcal_stave_gaps"  ,                 dblParamHcalStaveGaps );              
  endcapParam->setDoubleVal( "Hcal_lateral_structure_thickness" , dblParamHcalLateralStructureThickness );
  endcapParam->setDoubleVal( "Hcal_endcap_sensitive_center_box", dblParamHcalEndcapSensitiveCenterBox );
  endcapParam->setDoubleVal( "Hcal_fiber_gap",                   dblParamFiberGap );
  gearMgr->setHcalEndcapParameters( endcapParam ) ;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  //                
  // GEAR information for the HCAL ENDCAP RINGS    
  //
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  gear::CalorimeterParametersImpl * endcapRingParam =
    new gear::CalorimeterParametersImpl (helpEndcapRing.innerRadius, helpEndcapRing.outerRadius,
					 helpEndcapRing.leastZ, 2, helpEndcapRing.phi0
					 );

  // write all layers by position
  for (int i=0; i < helpEndcapRing.count; i++) 
    {
      G4double calcThick = helpEndcapRing.layerPos[i+1] - helpEndcapRing.layerPos[i] ;
      G4double calcAbsorb = calcThick - helpEndcapRing.sensThickness[i] - helpEndcapRing.gapThickness[i] ;  
      
      // on last layer, gap has to be taken into account
      if( i == ( helpEndcapRing.count - 1 ) ) 
	{
	  G4double layerGap = helpEndcapRing.layerPos[i] - helpEndcapRing.layerPos[i-1] - calcThick ;
	  
	  // check if layerGap makes sense
	  if ( layerGap < calcThick ) 
	    {
	      // add gap to Thickness
	      calcThick = calcThick + layerGap ;
	    }
	}
      
      endcapRingParam->layerLayout().positionLayer
	(0, calcThick, Hcal_digi_cell_dim_x, Hcal_digi_cell_dim_x, calcAbsorb);
    }

  // write EndcapRing parameters to GearManager
  endcapRingParam->setDoubleVal( "Hcal_virtual_cell_size", Hcal_cell_dim_x );
  endcapRingParam->setDoubleVal( "Hcal_stave_gaps"  ,                 dblParamHcalStaveGaps );              
  endcapRingParam->setDoubleVal( "Hcal_lateral_structure_thickness" , dblParamHcalLateralStructureThickness );
  endcapRingParam->setDoubleVal( "Hcal_fiber_gap",                    dblParamFiberGap );
  gearMgr->setHcalRingParameters( endcapRingParam ) ;

#endif

  // Closes Database connection
  delete db;

#ifdef SHCALSC02_DEBUG
  //scintillator
  G4cout<<"\n\n\n Absorber: X0="<<BarrelRadiatorMaterial->GetRadlen()/cm
	<<" lambda="<<BarrelRadiatorMaterial->GetNuclearInterLength()/cm<<G4endl;
 
  G4cout<<"Scintillator (poly): X0="<<CGAGeometryManager::GetMaterial("polystyrene")->GetRadlen()/cm
	<<" lambda="<<CGAGeometryManager::GetMaterial("polystyrene")->GetNuclearInterLength()/cm<<G4endl;
  
  //gap in the middle of a module, along y
  G4cout<<"Stainless steel: X0="<<CGAGeometryManager::GetMaterial("stainless_steel")->GetRadlen()/cm
	<<" lambda="<<CGAGeometryManager::GetMaterial("stainless_steel")->GetNuclearInterLength()/cm<<G4endl;

  //air gap between layer support structure and layer
  G4cout<<"Air: X0="<<CGAGeometryManager::GetMaterial("air")->GetRadlen()/cm
	<<" lambda="<<CGAGeometryManager::GetMaterial("air")->GetNuclearInterLength()/cm<<G4endl;

  //aluminium layer support
  G4cout<<"Al: X0="<<CGAGeometryManager::GetMaterial("aluminium")->GetRadlen()/cm
	<<" lambda="<<CGAGeometryManager::GetMaterial("aluminium")->GetNuclearInterLength()/cm<<G4endl;

#endif

  return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              Destructor
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SHcalSc02::~SHcalSc02() 
{
}  


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              Barrel Regular Modules               ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalSc02::BarrelRegularModules(G4LogicalVolume* MotherLog)
{
  // Regular modules
  G4double BHX  = Hcal_bottom_dim_x /2.;
  G4double MHX  = Hcal_midle_dim_x / 2.;
  G4double THX  = Hcal_top_dim_x / 2.;
  G4double YX1H = Hcal_y_dim1_for_x / 2.;
  G4double YX2H = Hcal_y_dim2_for_x / 2.;
  G4double DHZ  = Hcal_normal_dim_z / 2.;

  G4double BottomDimY = YX1H;
  G4double chambers_y_off_correction = YX2H;

  // Attention: on bâtit le module dans la verticale
  // à cause du G4Trd et on le tourne avant de le positioner
  G4Trd * Bottom = new G4Trd("Bottom_Barrel_Module",
			     BHX, MHX, DHZ, DHZ, YX1H);

  G4Trd * Top = new G4Trd("Top_Barrel_Module",
			  MHX, THX, DHZ, DHZ, YX2H);

  G4UnionSolid* ModuleSolid = new G4UnionSolid("ModuleSolid",
					       Bottom,
					       Top,
					       0,
					       G4ThreeVector(0, 0, YX1H + YX2H));

  EnvLogHcalModuleBarrel  = new G4LogicalVolume(ModuleSolid,
						BarrelRadiatorMaterial,//iron or tungsten
						"barrelHcalModule", 
						0, 0, 0);

  G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(.8,.8,.2));
  VisAtt->SetForceWireframe(true);
  //  VisAtt->SetForceSolid(false);
  VisAtt->SetForceAuxEdgeVisible(true);
  VisAtt->SetDaughtersInvisible(true); 
  EnvLogHcalModuleBarrel->SetVisAttributes(VisAtt);


#ifdef MOKKA_GEAR
  // calculate ground-level for layers
  G4double yTotal = YX1H + YX2H ;
  // add first position of layer as ground level
  helpBarrel.layerPos.push_back( -yTotal ) ;
#endif 

  //====================================================
  //                                                  //
  // Chambers in the HCAL BARREL                      //
  //                                                  //
  //====================================================
  //build a box filled with air in the middle of the HCAL barrel
  //to simulate the gap between the two real halves of a module
  BarrelModuleGap(EnvLogHcalModuleBarrel);

  //build the chambers (scintillator + air gap for cabels)
  BarrelHalfRegularChambersTrap(EnvLogHcalModuleBarrel, chambers_y_off_correction);

  //build the air gap between chambers and layer support structure
  BarrelChambersGap(EnvLogHcalModuleBarrel, chambers_y_off_correction);

  //build the layers support structure
  BarrelChambersSupportTrap(EnvLogHcalModuleBarrel, chambers_y_off_correction);



  // BarrelStandardModule placements
  // values retrieved from DB in old releases
  G4double stave_phi_offset, inner_radius, module_z_offset;
  inner_radius = Hcal_inner_radius;

  G4double Y;
  Y = inner_radius + BottomDimY;

#ifdef MOKKA_GEAR
  // starting value for iteration of inner_radius
  helpBarrel.innerRadius = Y;
  G4double lastPosition  = -1.;
#endif

  stave_phi_offset = 0;  

  //-------- start loop over HCAL BARREL staves ----------------------------
  for (G4int stave_id = 1;
       stave_id <= 8;
       stave_id++)
    {
      module_z_offset = - (Hcal_normal_dim_z + Hcal_modules_gap)/2.;

      for (G4int module_id = 1;
	   module_id <=2;
	   module_id++)
	{
	  G4double phirot = stave_phi_offset;
	  G4RotationMatrix *rot = new G4RotationMatrix();
	  rot->rotateX(pi*0.5);
	  rot->rotateY(phirot);
	  new MyPlacement(rot,
			  G4ThreeVector(-Y*sin(phirot),
					Y*cos(phirot),
					module_z_offset),
			  EnvLogHcalModuleBarrel,
			  "BarrelHcalModule",
			  MotherLog,
			  false,
			  HCALBARREL*100 + stave_id*10 + module_id);

	  for (G4int j = 1; j >= 0; j--){
	    G4int temp = 2*stave_id - j; 
	    theBarrilRegSD->SetStaveRotationMatrix(temp, phirot);
	  }
	  theBarrilRegSD->SetModuleZOffset(module_id,module_z_offset);

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
      stave_phi_offset -=  pi/4;
    }
  //-------- end loop over HCAL BARREL staves ----------------------------
}



//=======================================================================
//
//
//=======================================================================
void SHcalSc02::EndcapChambers(G4LogicalVolume* MotherLog, SDHcalEndCap* theSD, bool rings)
{
  // Chambers in the SHcalSc02::Endcaps
  // standard endcap chamber solid:
  G4Polyhedra *motherSolid = (G4Polyhedra*) MotherLog->GetSolid();

  G4PolyhedraHistorical* motherPolyhedraParameters = motherSolid->GetOriginalParameters();

  G4double pRMax, pDz, fiber_gap, pRMin;

  pRMax = (*(motherPolyhedraParameters->Rmax) * cos(pi/motherPolyhedraParameters->numSide))
    - (Hcal_lateral_plate_thickness);

  pDz = Hcal_chamber_thickness / 2.;

  pRMin = ( *(motherPolyhedraParameters->Rmin) * cos(pi/motherPolyhedraParameters->numSide))
    + (Hcal_lateral_plate_thickness);

  fiber_gap = Hcal_fiber_gap;

  // G4Polyhedra Envelope parameters
  G4double phiStart = 0.;
  G4double phiTotal = 360.;
  G4int numSide     = motherPolyhedraParameters->numSide;
  G4int numZPlanes  = 2;

  G4double zPlane[2];
  zPlane[0] = - pDz;
  zPlane[1] = - zPlane[0];

  G4double rInner[2],rOuter[2];
  rInner[0] = rInner[1] = pRMin;
  rOuter[0] = rOuter[1] = pRMax;

#ifdef SHCALSC02_DEBUG
  if(rings==true){
    G4cout<<"    EndcapRingsSensitive: Rinner="<<pRMin<<"mm Router="<<pRMax<<G4endl<<G4endl;
  }else{
    G4cout<<"    EndcapSensitive     : Rinner="<<pRMin<<"mm Router="<<pRMax<<G4endl<<G4endl;
  }
#endif


  G4Polyhedra *EndCapChamberSolid = new G4Polyhedra("EndCapChamberSolid",
						    phiStart,
						    phiTotal,
						    numSide,
						    numZPlanes,
						    zPlane,
						    rInner,
						    rOuter);
  G4Box *IntersectionStaveBox = new G4Box("IntersectionStaveBox",
					  pRMax,
					  pRMax,
					  Hcal_total_dim_y);

  // set up the translation and rotation for the intersection process 
  // this happens in the mother volume coordinate system, so a coordinate transformation is needed
  G4ThreeVector IntersectXYZtrans((pRMax + (Hcal_stave_gaps/2.))*(cos(pi/numSide) - sin(pi/numSide)),
				  (pRMax + (Hcal_stave_gaps/2.))*(cos(pi/numSide) + sin(pi/numSide)),
				  (Hcal_total_dim_y/2.));
  G4RotationMatrix *rot = new G4RotationMatrix;
  rot->rotateZ(-(pi/motherPolyhedraParameters->numSide));
  // intersect the octagonal layer with a square to get only one quadrant
  G4IntersectionSolid  *EndCapStaveSolid = new G4IntersectionSolid( "EndCapStaveSolid",
								    EndCapChamberSolid,
								    IntersectionStaveBox,
								    rot, 
								    IntersectXYZtrans); 

  G4UserLimits* pULimits = new G4UserLimits(theMaxStepAllowed);

  // standard endcap chamber logical
  G4LogicalVolume* EndCapStaveLogical = 0;

  if(Hcal_sensitive_model == "scintillator")
    {
      //fg: introduce (empty) fiber gap - should be filled with fibres and cables
      // - so far we fill it  with air ...
      EndCapStaveLogical = new G4LogicalVolume(EndCapStaveSolid,
					       CGAGeometryManager::GetMaterial("air"), 
					       "EndCapChamberLogical", 
					       0, 0, 0);

      G4double scintHalfWidth = pDz - fiber_gap  / 2. ;

      // fiber gap can't be larger than total chamber
      assert( scintHalfWidth > 0. ) ;

      G4double zPlaneScint[2];
      zPlaneScint[0] = - scintHalfWidth ;
      zPlaneScint[1] = - zPlaneScint[0];

      G4Polyhedra *EndCapScintSolid = new G4Polyhedra("EndCapScintSolid",
						      phiStart,
						      phiTotal,
						      numSide,
						      numZPlanes,
						      zPlaneScint,
						      rInner,
						      rOuter);
      G4IntersectionSolid  *EndCapScintStaveSolid = new G4IntersectionSolid( "EndcapScintStaveSolid",
									     EndCapScintSolid,
									     IntersectionStaveBox,
									     rot, 
									     IntersectXYZtrans);

      G4LogicalVolume* ScintLog = new G4LogicalVolume(EndCapScintStaveSolid,
						      CGAGeometryManager::GetMaterial("polystyrene"),
						      "EndCapScintLogical", 
						      0, 0, pULimits);  
      G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::Yellow());
#ifdef SHCALSC02_DEBUG
      if(rings==true){
	VisAtt->SetColour(G4Colour(.2,.5,.2));
      }else{
	VisAtt->SetColour(G4Colour(.5,.2,.2));  
      }
#endif
      VisAtt->SetForceSolid(true);
      ScintLog->SetVisAttributes(VisAtt);

      // only scintillator is sensitive
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

      new MyPlacement(0, 
		      G4ThreeVector( 0, 0,  - fiber_gap / 2.), 
		      ScintLog,
		      "EndCapScintillator", 
		      EndCapStaveLogical, 
		      false, 
		      0);   
    }
  else Control::Abort("SHcalSc02: Invalid sensitive model parameter!",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);

  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::Blue());
#ifdef SHCALSC02_DEBUG
  if(rings==true){
    VisAtt->SetColour(G4Colour(.0,.2,.0));
  }else{
    VisAtt->SetColour(G4Colour(.2,.0,.0));  
  }
#endif
  EndCapStaveLogical->SetVisAttributes(VisAtt);

  // chamber placements
  G4int number_of_chambers = Hcal_endcap_nlayers;
  G4int possible_number_of_chambers = (G4int) floor(2*abs(*(motherPolyhedraParameters->Z_values)) /
						    (Hcal_chamber_thickness + Hcal_endcap_radiator_thickness));
  if(possible_number_of_chambers < number_of_chambers)
    number_of_chambers = possible_number_of_chambers;

  for (G4int layer_id = 1;
       layer_id <= number_of_chambers;
       layer_id++)
    {
      G4double Zoff = - abs(*(motherPolyhedraParameters->Z_values))
	+ (layer_id-1) *(Hcal_chamber_thickness + Hcal_endcap_radiator_thickness)
	+ Hcal_endcap_radiator_thickness 
        + (Hcal_chamber_thickness - Hcal_fiber_gap)/2.;

      //place the four staves in their right positions
      for (G4int stave_id = 1;
	   stave_id <= 4;
	   stave_id++)
	{
	  G4RotationMatrix *rotEffect = new G4RotationMatrix();
	  rotEffect->rotateZ(((stave_id-1)*pi/2.));
	  new MyPlacement(rotEffect,
			  G4ThreeVector(0.,0.,Zoff),
			  EndCapStaveLogical,
			  "EndCapStavePhys",
			  MotherLog,
			  false,
			  layer_id*10 + stave_id);
	}

      theSD->AddLayer(layer_id,
		      0,
		      0,
		      Zoff);

#ifdef MOKKA_GEAR
      // count the layers
      if(rings==true){
	helpEndcapRing.count += 1;

	// position of layer as offset + half thickness
	helpEndcapRing.layerPos.push_back(Zoff + std::abs(zPlane[0]) + Hcal_fiber_gap/2 ) ;

	// sensible Area is the same every time
	// value set in 0 within construction of Scint or RPC
	helpEndcapRing.sensThickness.push_back( helpEndcapRing.sensThickness[0] );
	helpEndcapRing.gapThickness.push_back( helpEndcapRing.gapThickness[0] );   
      }
      else {
	helpEndcap.count += 1;

	// position of layer as offset + half thickness
	helpEndcap.layerPos.push_back(Zoff + std::abs(zPlane[0]) + Hcal_fiber_gap/2) ;

	// sensible Area is the same every time
	// value set in 0 within construction of Scint or RPC
	helpEndcap.sensThickness.push_back( helpEndcap.sensThickness[0] );
	helpEndcap.gapThickness.push_back( helpEndcap.gapThickness[0] );
      }
#endif

    }  
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~        Chambers in the Endcaps                    ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalSc02::EndcapChambersTesla(G4LogicalVolume* MotherLog, SDHcalEndCapTesla* theSD)
{
  //====================================================
  //                                                  //
  // Build first the scintillators (polystyrene)      //
  //                                                  //
  //====================================================
  G4UnionSolid *EndcapUnion = (G4UnionSolid*)MotherLog->GetSolid();
  G4Box *BottomEndcap = (G4Box*)EndcapUnion->GetConstituentSolid(0);
  G4double box_half_x = BottomEndcap->GetXHalfLength();
  G4double box_half_z = BottomEndcap->GetYHalfLength();
  //G4double box_half_y = BottomEndcap->GetZHalfLength();

  //half length of a hexagon side
  G4double half_length  = Hcal_endcap_rmax * tan(pi/8.);
  G4double tanPiDiv8 = tan(pi/8.);
  G4double tanPiDiv4 = tan(pi/4.);
  G4double trap_small_x = half_length + Hcal_endcap_center_box_size/2. - Hcal_lateral_plate_thickness * tanPiDiv8;
  G4double trap_x       = Hcal_endcap_rmax + Hcal_endcap_center_box_size/2 - Hcal_lateral_plate_thickness;
  //G4double trap_y       = Hcal_total_dim_y;
  G4double trap_z       = (Hcal_endcap_rmax + Hcal_endcap_center_box_size/2 - trap_small_x - Hcal_lateral_plate_thickness)/tanPiDiv4;
 

  //G4Trap *TopEndcap     = (G4Trap*)EndcapUnion->GetConstituentSolid(1);


  G4double new_box_half_y = 0;
  G4double new_trap_y = 0;

  new_box_half_y = Hcal_scintillator_thickness/2.;
  G4Box *EndcapBottomScintillator = new G4Box("EndcapBottomScintillator",
					      box_half_x,
					      box_half_z,
					      new_box_half_y);
  
  new_trap_y = 2.* new_box_half_y;
  G4Trap *EndcapTopScintillator = new G4Trap("EndcapTopScintillator", 
					     new_trap_y,
					     trap_z,
					     trap_x,
					     trap_small_x);
  
  //-----------------------------------------------
  //Build an union out of the two
  //---------
  //x-dimension of the trapezoid center of gravity
  G4double trap_center_of_grav_half_x = (trap_small_x + trap_z/2 * tan(pi/4.))/2;
  
  //shift the top trapezoidal part with respect to the bottom part to get the union
  G4double shift_x = (-1)*abs(box_half_x - trap_center_of_grav_half_x);
  G4double shift_y = box_half_z + trap_z/2.;
  G4UnionSolid* EndcapScintillatorStave = new G4UnionSolid("EndcapScintillatorStave",
							   EndcapBottomScintillator,
							   EndcapTopScintillator,
							   0,
							   G4ThreeVector(shift_x, 
									 shift_y, 
									 0)); 

  G4UserLimits* pULimits = new G4UserLimits(theMaxStepAllowed);
  G4LogicalVolume *EndcapScintillatorLogical = new G4LogicalVolume(EndcapScintillatorStave,
								   CGAGeometryManager::GetMaterial("polystyrene"),
								   "EndcapScintillatorLogical",
								   0, 0, pULimits);
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::Cyan());
  VisAtt->SetForceWireframe(true);
  VisAtt->SetForceSolid(false);
  VisAtt->SetDaughtersInvisible(true);
  EndcapScintillatorLogical->SetVisAttributes(VisAtt);


  //====================================================
  //                                                  //
  // Build the air gap (for cables)                   //
  //                                                  //
  //====================================================
  new_box_half_y = Hcal_fiber_gap/2;
  G4Box *EndcapBottomFiberGap = new G4Box("EndcapBottomFiberGap",
					  box_half_x,
					  box_half_z, 
					  new_box_half_y);
  
  new_trap_y = 2.* new_box_half_y;
  G4Trap *EndcapTopFiberGap = new G4Trap("EndcapTopFiberGap", 
					 new_trap_y,
					 trap_z,
					 trap_x,
					 trap_small_x);

  G4UnionSolid* EndcapFiberGapStave = new G4UnionSolid("EndcapFiberGapStave",
						       EndcapBottomFiberGap,
						       EndcapTopFiberGap,
						       0,
						       G4ThreeVector(shift_x, 
								     shift_y, 
								     0)); 

  G4LogicalVolume *EndcapFiberGapLogical = new G4LogicalVolume(EndcapFiberGapStave,
							       CGAGeometryManager::GetMaterial("air"),
							       "EndcapFiberGapLogical",
							       0, 0, 0);
  VisAtt = new G4VisAttributes(G4Colour::White());
  VisAtt->SetForceSolid(false);
  EndcapFiberGapLogical->SetVisAttributes(VisAtt);
 
 
#ifdef MOKKA_GEAR
  helpEndcap.sensThickness.push_back( Hcal_scintillator_thickness );
  helpEndcap.gapThickness.push_back( Hcal_fiber_gap );
#endif


  //====================================================
  //                                                  //
  // Place these 48 times...                          //
  //                                                  //
  //====================================================
  G4double offset_x = 0;
  G4double offset_y = 0;
  G4double offset_z = 0;

  G4double shift_z  = 0;

  //------ start loop over HCAL layers ----------------------
  for (G4int layer_id = 1; layer_id <= Hcal_endcap_nlayers; layer_id++)
    //for (G4int layer_id = 1; layer_id <= 1 ; layer_id++)
    {
      //-----------------------------------------------
      //                                             //
      // Place the scintillator                      //
      //                                             //
      //-----------------------------------------------
//       shift_z = - Hcal_total_dim_y/2
// 	+ (layer_id-1) *(Hcal_chamber_thickness + Hcal_radiator_thickness)
// 	+ Hcal_radiator_thickness 
//         + Hcal_scintillator_thickness/2.;

     shift_z = - Hcal_endcap_total_z/2
        + (layer_id-1) *(Hcal_chamber_thickness + Hcal_endcap_radiator_thickness)
        + Hcal_endcap_radiator_thickness
        + Hcal_scintillator_thickness/2.;

      new MyPlacement(NULL,
		      G4ThreeVector(0, 0, shift_z),
		      EndcapScintillatorLogical,
		      "EndcapScintillatorPhys",
		      MotherLog,
		      false,
		      layer_id);
      
      //center of coordinate in the middle of the box
      offset_x = - box_half_x;
      offset_y = - box_half_z;
      offset_z = Hcal_scintillator_thickness/2.;//????

      theSD->AddLayer(layer_id,
		      offset_x,
		      offset_y,
		      offset_z);     

      //-------------------------------------------------------------
      //Very important: DON'T FORGET to set the sensitive detector
      // only scintillator is sensitive
      EndcapScintillatorLogical->SetSensitiveDetector( theENDCAPEndSD );
      //-------------------------------------------------------------

#ifdef MOKKA_GEAR
      // count the layers
      helpEndcap.count += 1;
      
      // position of layer as offset along z
//       shift_z = - Hcal_total_dim_y/2
// 	+ layer_id *(Hcal_chamber_thickness + Hcal_radiator_thickness);

     shift_z = - Hcal_endcap_total_z/2
        + layer_id *(Hcal_chamber_thickness + Hcal_endcap_radiator_thickness);

      helpEndcap.layerPos.push_back(shift_z) ;
      
      // sensitive area
      helpEndcap.sensThickness.push_back( helpEndcap.sensThickness[0] );
      helpEndcap.gapThickness.push_back( helpEndcap.gapThickness[0] );
      
#endif      
 
      //-----------------------------------------------
      //                                             //
      // Place the fiber gap                         //
      //                                             //
      //-----------------------------------------------
//       shift_z = - Hcal_total_dim_y/2
// 	+ layer_id * (Hcal_radiator_thickness + Hcal_scintillator_thickness)
// 	+ (layer_id - 1) * Hcal_fiber_gap + Hcal_fiber_gap/2.;

      shift_z = - Hcal_endcap_total_z/2
        + layer_id * (Hcal_endcap_radiator_thickness + Hcal_scintillator_thickness)
        + (layer_id - 1) * Hcal_fiber_gap + Hcal_fiber_gap/2.;


      new MyPlacement(NULL,
		      G4ThreeVector(0, 0, shift_z),
		      EndcapFiberGapLogical,
		      "EndcapFiberGapPhys",
		      MotherLog,
		      false,
		      1);


    }
  //------ end loop over HCAL layers -------------------------

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~       Load event                                 ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalSc02::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
  // In this complex subdetector the best is to read
  // all hits in the one of the sensitive detectors,
  // just for visualisation, so just for your eyes...
  theBarrilRegSD->LoadEvent(theSubDetectorEventHitsFileInput);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~       Setup                                      ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
G4bool SHcalSc02::Setup(const CGAGeometryEnvironment &theGeometryEnvironment)
{
  Hcal_barrel_end_module_type = theGeometryEnvironment.GetParameterAsInt("Hcal_barrel_end_module_type");
  if( Hcal_barrel_end_module_type != 1)
    Control::Abort("SHcalSc02: Sorry, but TESLA like end modules in barrel are not available with this driver.",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);

  Hcal_layer_support_length       = theGeometryEnvironment.GetParameterAsDouble("Hcal_layer_support_length");
  Hcal_layer_air_gap              = theGeometryEnvironment.GetParameterAsDouble("Hcal_layer_air_gap");
  Hcal_chamber_thickness          = theGeometryEnvironment.GetParameterAsDouble("Hcal_chamber_thickness");
  Hcal_middle_stave_gaps          = theGeometryEnvironment.GetParameterAsDouble("Hcal_middle_stave_gaps");
  Hcal_apply_Birks_law            = theGeometryEnvironment.GetParameterAsInt("Hcal_apply_Birks_law");
  Hcal_radiator_thickness         = theGeometryEnvironment.GetParameterAsDouble("Hcal_radiator_thickness");
  Hcal_radiator_material          = theGeometryEnvironment.GetParameterAsString("Hcal_radiator_material");
  Hcal_ring                       = theGeometryEnvironment.GetParameterAsInt("Hcal_ring");
  Hcal_radial_ring_inner_gap      = theGeometryEnvironment.GetParameterAsInt("Hcal_radial_ring_inner_gap");
  Hcal_sensitive_model            = theGeometryEnvironment.GetParameterAsString("Hcal_sensitive_model");
  Hcal_back_plate_thickness       = theGeometryEnvironment.GetParameterAsDouble("Hcal_back_plate_thickness");
  Hcal_stave_gaps                 = theGeometryEnvironment.GetParameterAsDouble("Hcal_stave_gaps");
  Hcal_modules_gap                = theGeometryEnvironment.GetParameterAsDouble("Hcal_modules_gap");
  Hcal_nlayers                    = theGeometryEnvironment.GetParameterAsInt("Hcal_nlayers");
  Hcal_fiber_gap                  = theGeometryEnvironment.GetParameterAsDouble("Hcal_fiber_gap");
  Ecal_endcap_zmin                = theGeometryEnvironment.GetParameterAsDouble("Ecal_endcap_zmin");
  Ecal_endcap_outer_radius        = theGeometryEnvironment.GetParameterAsDouble("Ecal_endcap_outer_radius");
  Hcal_endcap_radiator_thickness  = theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_radiator_thickness");
  Hcal_endcap_radiator_material   = theGeometryEnvironment.GetParameterAsString("Hcal_endcap_radiator_material");
  Hcal_endcap_nlayers             = theGeometryEnvironment.GetParameterAsInt   ("Hcal_endcap_nlayers");

//   Hcal_endcap_radiator_thickness  = 20.;
//   Hcal_endcap_radiator_material   = "TungstenDens24";
//   Hcal_endcap_nlayers             = 70;


  Hcal_inner_radius = theGeometryEnvironment.GetParameterAsDouble("Ecal_outer_radius")
    + theGeometryEnvironment.GetParameterAsDouble("Hcal_Ecal_gap");
  Hcal_endcap_cables_gap = theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_cables_gap");
  Hcal_endcap_ecal_gap   = theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_ecal_gap");

  TPC_Ecal_Hcal_barrel_halfZ = theGeometryEnvironment.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");

  // just two modules per stave
  Hcal_normal_dim_z = (2 * TPC_Ecal_Hcal_barrel_halfZ - Hcal_modules_gap)/2.;
  Hcal_top_end_dim_z = 1180.0000;
  Hcal_start_z =  Hcal_normal_dim_z + Hcal_modules_gap / 2. + Hcal_endcap_cables_gap;
 
 
  // Hcal_start_z is the Hcal Endcap boundary coming from the IP
  // Test Hcal_start_z against Ecal_endcap_zmax + Hcal_endcap_ecal_gap
  // to avoid overlap problems with Ecal if scaled.
  //
  Ecal_endcap_zmax = theGeometryEnvironment.GetParameterAsDouble("Ecal_endcap_zmax");

  if( Hcal_start_z < Ecal_endcap_zmax + Hcal_endcap_ecal_gap )
    Hcal_start_z = Ecal_endcap_zmax + Hcal_endcap_ecal_gap;

  Hcal_lateral_plate_thickness     = theGeometryEnvironment.GetParameterAsDouble("Hcal_lateral_structure_thickness");
  Hcal_cells_size                  = theGeometryEnvironment.GetParameterAsDouble("Hcal_cells_size");
  Hcal_digi_cells_size             = theGeometryEnvironment.GetParameterAsDouble("Hcal_digitization_tile_size");
  Hcal_endcap_center_box_size      = theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_center_box_size");
  Hcal_endcap_sensitive_center_box = theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_sensitive_center_box");

  //=======================================================
  //                                                     //
  // general calculated parameters                       //
  //                                                     //
  //=======================================================  
  Hcal_total_dim_y   = Hcal_nlayers * (Hcal_radiator_thickness + Hcal_chamber_thickness) + Hcal_back_plate_thickness;
  Hcal_endcap_total_z = Hcal_endcap_nlayers * (Hcal_endcap_radiator_thickness + Hcal_chamber_thickness) + Hcal_back_plate_thickness;
  Hcal_module_radius = Hcal_inner_radius + Hcal_total_dim_y;
  Hcal_y_dim2_for_x  = (Hcal_module_radius - Hcal_module_radius*cos(pi/8));
  Hcal_y_dim1_for_x  = Hcal_total_dim_y - Hcal_y_dim2_for_x;
  Hcal_bottom_dim_x  = 2.*Hcal_inner_radius*tan(pi/8.)- Hcal_stave_gaps;
  Hcal_midle_dim_x   = Hcal_bottom_dim_x + 2* Hcal_y_dim1_for_x*tan(pi/8.);
  Hcal_top_dim_x     = Hcal_midle_dim_x - 2 * Hcal_y_dim2_for_x/tan(pi/8.);  
  Hcal_endcap_rmax   = Hcal_inner_radius + Hcal_y_dim1_for_x;

  Hcal_regular_chamber_dim_z = Hcal_normal_dim_z - 2 *(Hcal_lateral_plate_thickness);
  Hcal_cell_dim_x            = Hcal_cells_size;
  Hcal_cell_dim_z            = Hcal_regular_chamber_dim_z / floor (Hcal_regular_chamber_dim_z/Hcal_cell_dim_x);
  Hcal_digi_cell_dim_x       = Hcal_digi_cells_size;
  Hcal_digi_cell_dim_z       = Hcal_regular_chamber_dim_z / floor (Hcal_regular_chamber_dim_z/Hcal_digi_cell_dim_x);

  Hcal_scintillator_thickness = Hcal_chamber_thickness - Hcal_fiber_gap;
  G4cout<<"Hcal_scintillator_thickness="<<Hcal_scintillator_thickness<<G4endl;
  G4cout<<"Hcal_start_z="<<Hcal_start_z <<G4endl;
  G4cout<<"Hcal_endcap_rmax="<<Hcal_endcap_rmax<<G4endl;
  G4cout<<"Hcal_total_dim_y="<<Hcal_total_dim_y<<G4endl;
  G4cout<<"Hcal_endcap_total_z="<<Hcal_endcap_total_z<<G4endl;
  G4cout<<G4endl;
  

  //=======================================================
  //                                                     //
  // Mokka GEAR                                          //
  //                                                     //
  //=======================================================  
#ifdef MOKKA_GEAR
  MokkaGear* mokkaGearMgr = MokkaGear::getMgr() ;

  mokkaGearMgr->tmpParam.setDoubleVal("Hcal_back_plate_thickness" , 
				      theGeometryEnvironment.GetParameterAsDouble("Hcal_back_plate_thickness") ) ;
  mokkaGearMgr->tmpParam.setDoubleVal("Hcal_fiber_gap" , 
				      theGeometryEnvironment.GetParameterAsDouble("Hcal_fiber_gap") ) ;
  mokkaGearMgr->tmpParam.setDoubleVal("Hcal_layer_support_length" , 
				      theGeometryEnvironment.GetParameterAsDouble( "Hcal_layer_support_length"));
  mokkaGearMgr->tmpParam.setDoubleVal("Hcal_layer_air_gap" , 
				      theGeometryEnvironment.GetParameterAsDouble( "Hcal_layer_air_gap" ) ) ;
  mokkaGearMgr->tmpParam.setDoubleVal("Hcal_middle_stave_gaps" , 
				      theGeometryEnvironment.GetParameterAsDouble( "Hcal_middle_stave_gaps" ) ) ;
  mokkaGearMgr->tmpParam.setDoubleVal("Hcal_modules_gap" , 
				      theGeometryEnvironment.GetParameterAsDouble( "Hcal_modules_gap" ) ) ;
  mokkaGearMgr->tmpParam.setDoubleVal("Hcal_stave_gaps" , 
				      theGeometryEnvironment.GetParameterAsDouble( "Hcal_stave_gaps" ) ) ;
  mokkaGearMgr->tmpParam.setDoubleVal("TPC_Ecal_Hcal_barrel_halfZ" , 
				      theGeometryEnvironment.GetParameterAsDouble( "TPC_Ecal_Hcal_barrel_halfZ" ) ) ;
  mokkaGearMgr->tmpParam.setDoubleVal("Hcal_lateral_structure_thickness" , 
				      theGeometryEnvironment.GetParameterAsDouble( "Hcal_lateral_structure_thickness" ) ) ;
  mokkaGearMgr->tmpParam.setIntVal("Hcal_barrel_end_module_type" , 
				   theGeometryEnvironment.GetParameterAsInt( "Hcal_barrel_end_module_type" ) ) ;
  mokkaGearMgr->tmpParam.setDoubleVal("Hcal_endcap_sensitive_center_box" , 
				      theGeometryEnvironment.GetParameterAsDouble( "Hcal_endcap_sensitive_center_box" ) ) ;
#endif

  return true;

}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              EndcapRings                          ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalSc02::EndcapRings(G4LogicalVolume* MotherLog)
{
  // old parameters from database
  G4double pRMax, pDz, pRMin;
  pRMax = Hcal_endcap_rmax;

  // The rings start from inner Ecal endcap boundary
  // and finish at inner Hcal endcap one.
  G4double start_z, stop_z;
  start_z = Ecal_endcap_zmin;
  G4double SpaceForLayers = Hcal_start_z - Hcal_endcap_ecal_gap
    - Ecal_endcap_zmin - Hcal_back_plate_thickness;

  G4int MaxNumberOfLayers = (G4int) (SpaceForLayers /
				     (Hcal_chamber_thickness + Hcal_endcap_radiator_thickness));

  G4cout<<"    HCAL endcap rings will have "<< MaxNumberOfLayers << " layers."<<G4endl<<G4endl;

  stop_z = start_z + MaxNumberOfLayers * (Hcal_chamber_thickness + Hcal_endcap_radiator_thickness)
    + Hcal_back_plate_thickness;

  pDz = (stop_z - start_z) / 2.;

  pRMin = Ecal_endcap_outer_radius
    + Hcal_radial_ring_inner_gap;

  G4double zPlane[2];
  zPlane[0]=-pDz;
  zPlane[1]=-zPlane[0];

  G4double rInner[2],rOuter[2];
  rInner[0]=rInner[1]=pRMin;
  rOuter[0]=rOuter[1]=pRMax;

#ifdef SHCALSC02_DEBUG
  G4cout<<"    EndcapRings         : Rinner="<<pRMin<<"mm Router="<<pRMax<<G4endl;
#endif

  if (rOuter[0] <= rInner[0]) 
    G4Exception("SHcalSc02::EndcapRings() - not enough place for endcap rings (try a larger Hcal_nlayers number)!");

#ifdef MOKKA_GEAR
  // Write parameters in helper class
  // attention: the outer symmetrie is 32-fold... this is not
  // taken into account in gear

  // the outer radius is in Geant4 defined as the tangent outer radius.
  // same is taken in Gear to be consistent with Ecal and Barrel
  helpEndcapRing.outerRadius = rOuter[0];  // outer radius
  helpEndcapRing.innerRadius = rInner[0];  // inner radius
  helpEndcapRing.phi0 = 0. ;               // phi0

  // set starting value for inner_z
  // it should _not_ stay at this value
  helpEndcapRing.leastZ = 999999999. ;
#endif

  G4Polyhedra *EndCapSolid = new G4Polyhedra("HcalEndCapRingSolid",
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
#ifdef SHCALSC02_DEBUG
  VisAtt->SetColour(G4Colour(.2,.8,.2));
  VisAtt->SetDaughtersInvisible(false); 
#endif

  G4LogicalVolume* EndCapLogical = new G4LogicalVolume(EndCapSolid,
						       EndcapRadiatorMaterial,
						       "EndCapRingLogical",
						       0, 0, 0);
  EndCapLogical->SetVisAttributes(VisAtt);

#ifdef MOKKA_GEAR
  // add first position of layer as ground level
  helpEndcapRing.layerPos.push_back( zPlane[0] ) ;
#endif

  //------------------------------------------------------
  // build and place the chambers in the Hcal EndcapRings
  EndcapChambers(EndCapLogical,theENDCAPRingSD, true);
  //------------------------------------------------------

  // Placements
  G4double endcap_z_offset = Ecal_endcap_zmin + pDz;
  G4RotationMatrix *rotEffect = new G4RotationMatrix();
  rotEffect->rotateZ(pi/8.);

  G4int ModuleNumber = HCALENDCAPPLUS*100 + 16;
  G4double Z1 = 0; 

  for (G4int endcap_id = 1;
       endcap_id <= 2;
       endcap_id++)
    {
      Z1 = endcap_z_offset;
      new MyPlacement(rotEffect,
		      G4ThreeVector(0.,
				    0.,
				    Z1),
		      EndCapLogical,
		      "EndCapPhys",
		      MotherLog,
		      false,
		      ModuleNumber);
      rotEffect = new G4RotationMatrix();
      rotEffect->rotateZ(-pi/8.);
      rotEffect->rotateY(pi);  // inverse the endcaps
      ModuleNumber -= (HCALENDCAPPLUS-HCALENDCAPMINUS)*100 + 6;

#ifdef MOKKA_GEAR
      // take inner_z as minimum of all offsets - halfThickness
      helpEndcapRing.leastZ = std::min( helpEndcap.leastZ, std::abs(Z1)-std::abs(zPlane[0]) );
#endif

      endcap_z_offset = - endcap_z_offset;
    }

  theENDCAPRingSD->SetModuleZOffset(0, fabs(Z1));
  theENDCAPRingSD->SetModuleZOffset(6, fabs(Z1));
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~           Post construct action                   ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
G4bool SHcalSc02::PostConstructAction(CGAGeometryEnvironment& )
{
  //
  // Propagates the changes to Coil, if any. The SHcal has also the responsability 
  // to change the calorimeter region parameters.
  //
  G4double Hcal_R_max = (Hcal_y_dim1_for_x +  Hcal_y_dim2_for_x + Hcal_inner_radius)/cos(pi/16);
  std::ostringstream oss1;
  oss1 << Hcal_R_max;
  (*Control::globalModelParameters)["Hcal_R_max"] = oss1.str();
  (*Control::globalModelParameters)["calorimeter_region_rmax"] = oss1.str();

  std::ostringstream oss2;
  oss2 << Hcal_start_z;
  (*Control::globalModelParameters)["Hcal_endcap_zmin"] = oss2.str();
  
  G4double Hcal_outer_radius = Hcal_inner_radius + Hcal_total_dim_y;

  G4double calorimeter_region_zmax = Hcal_start_z + Hcal_endcap_total_z;
  std::ostringstream oss3;  
  oss3 << calorimeter_region_zmax;
  (*Control::globalModelParameters)["calorimeter_region_zmax"] = oss3.str();
			    
  G4cout << "SHcalSc02 information: Hcal_outer_radius = "
	 << Hcal_outer_radius
	 << "\n                       module thickness = "
	 << Hcal_total_dim_y
	 << "\n                       endcap module thickness = "
	 << Hcal_endcap_total_z
         << "\n                       Hcal_R_max = "
   	 << Hcal_R_max
	 << "\n                       Hcal_endcap_zmin = "
	 << Hcal_start_z
	 << "\n                       calorimeter_region_rmax = "
	 << Hcal_R_max
	 << "\n                       calorimeter_region_zmax = "
	 << calorimeter_region_zmax
	 << G4endl;

  return true;    
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~      Calculate size of the fractional tile        ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalSc02::CalculateFractTileSize(G4double x_length, 
				       G4double x_integerTileSize, 
				       G4double &x_fractionalTileSize)
{
  G4int noOfIntCells = 0; //number of integer cells;
  G4double temp = x_length/x_integerTileSize;

  //check if x_length (scintillator length) is divisible with x_integerTileSize
  G4double fracPart, intPart;
  fracPart = modf(temp, &intPart);

  if (fracPart == 0){ //divisible
    noOfIntCells = int(temp);
    x_fractionalTileSize= 0.;
  }
  else if (fracPart>0){
    noOfIntCells = int(temp) -1;
    x_fractionalTileSize = (x_length - noOfIntCells * x_integerTileSize)/2.;
  }
#ifdef SHCALSC02_DEBUG
  G4cout<<"  xLayer="<<x_length<<"   x_fractionalTileSize="<<x_fractionalTileSize<<G4endl;
#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~     Build gap between HCAL barrel modules         ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalSc02::BarrelModuleGap(G4LogicalVolume *MotherLog)
{
  G4double gapBox_x = Hcal_middle_stave_gaps/2.;
  G4double gapBox_y = Hcal_total_dim_y/2.;
  G4double gapBox_z = Hcal_normal_dim_z/2.;

  G4Box* gapBox = new G4Box("gapox", gapBox_x, gapBox_z, gapBox_y);
  G4LogicalVolume* gapLog = new G4LogicalVolume(gapBox, 
						CGAGeometryManager::GetMaterial("stainless_steel"),
						"gapog");
  new MyPlacement(0,
		  G4ThreeVector(0,0,Hcal_y_dim2_for_x/2),//shift along y axis
		  gapLog, 
		  "myGap",
		  MotherLog,//MotherLog is the logical volume of the Barrel, EnvLogHcalModuleBarrel
		  true,
		  1);

  G4VisAttributes * VisAttb = new G4VisAttributes(G4Colour::Blue());
  VisAttb->SetForceWireframe(false);
  VisAttb->SetForceSolid(false);
  VisAttb->SetForceAuxEdgeVisible(true);
  VisAttb->SetDaughtersInvisible(true); 
  gapLog->SetVisAttributes(VisAttb);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~     Build HCAL barrel chambers                    ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalSc02::BarrelHalfRegularChambersTrap(G4LogicalVolume *MotherLog, 
					      G4double chambers_y_off_correction)
{
  G4LogicalVolume *ChamberLog[200];
  G4Box *ChamberSolid;

  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::Green());
  VisAtt->SetForceWireframe(true);
  VisAtt->SetForceSolid(false);

  theMaxStepAllowed = Hcal_cell_dim_x/2.;
  G4UserLimits* pULimits = new G4UserLimits(theMaxStepAllowed);

  G4double x_length; //dimension of an Hcal barrel layer on the x-axis
  G4double y_height; //dimension of an Hcal barrel layer on the y-axis
  G4double z_width;  //dimension of an Hcal barrel layer on the z-axis
  x_length = 0.;
  y_height = Hcal_chamber_thickness / 2.;
  z_width  = Hcal_regular_chamber_dim_z/2.;

  G4double xOffset = 0.;//the x_length of a barrel layer is calculated as a
  //barrel x-dimension plus (bottom barrel) or minus
  //(top barrel) an x-offset, which depends on the angle pi/8

  G4double xShift = 0.;//Geant4 draws everything in the barrel related to the 
  //center of the bottom barrel, so we need to shift the layers to
  //the left (or to the right) with the quantity xShift

  //G4double TanPiDiv8 = tan(pi/8.);
  G4int logical_layer_id = 0;

  for (G4int layer_id = 1; layer_id <= (2*Hcal_nlayers); layer_id++)
    {
      this->CalculateXLayer(layer_id, logical_layer_id, xOffset, x_length, xShift);
      
      //build chamber box, with the calculated dimensions
      ChamberSolid = new G4Box("ChamberSolid", 
			       x_length,  //x
			       z_width,   //z attention!
			       y_height); //y attention!

      //fg: introduce (empty) fiber gap - should be filled with fibres and cables
      // - so far we fill it  with air ...
      G4LogicalVolume *ChamberLogical = new G4LogicalVolume(ChamberSolid,
							    CGAGeometryManager::GetMaterial("air"), 
							    "ChamberLogical", 
							    0, 0, 0);   
      
      // the scintillator width is the chamber width - fiber gap 
      G4double scintHalfWidth = Hcal_scintillator_thickness/2.;
      // fiber gap can't be larger than total chamber
      if (scintHalfWidth <= 0.) G4Exception("SHcalSc02::BuildLefBarrelChambers() - scintHalfWidth invalid!");
      
      
      G4Box *ScintSolid = new G4Box("ScintSolid", 
				    x_length, //x
				    z_width,          //z attention!
				    scintHalfWidth ); //y attention!
      
      G4LogicalVolume* ScintLog = new G4LogicalVolume(ScintSolid,
						      CGAGeometryManager::GetMaterial("polystyrene"),
						      "ScintLogical", 
						      0, 0, pULimits);  
      ScintLog->SetVisAttributes(VisAtt);
      
      G4double fract_cell_dim_x = 0.;
      this->CalculateFractTileSize(2*x_length, Hcal_cell_dim_x, fract_cell_dim_x);
      
      G4ThreeVector newFractCellDim(fract_cell_dim_x, Hcal_chamber_thickness, Hcal_cell_dim_z);
      theBarrilRegSD->SetFractCellDimPerLayer(layer_id, newFractCellDim);
      
      // only scintillator is sensitive
      ScintLog->SetSensitiveDetector(theBarrilRegSD);
      
      new MyPlacement(0, 
		      G4ThreeVector(0, 0, -Hcal_fiber_gap / 2. ), 
		      ScintLog,
		      "Scintillator", 
		      ChamberLogical, 
		      false, 
		      layer_id);   
      
      ChamberLog [ layer_id ] = ChamberLogical ;
      
      //============================================================
      //============================================================
      
#ifdef MOKKA_GEAR
      // get height of sensible part of layer
      if (layer_id <= Hcal_nlayers){
	helpBarrel.sensThickness.push_back( scintHalfWidth * 2. ) ;
	helpBarrel.gapThickness.push_back( Hcal_fiber_gap ) ;
      }
#endif
      
      
      G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::White());
      //VisAtt->SetForceWireframe(true);
      VisAtt->SetForceSolid(false);
      ChamberLog[layer_id]->SetVisAttributes(VisAtt);
      
      
      // Chamber Placements
      // module x and y offsets (needed for the SD)
      G4double Xoff,Yoff;
      Xoff = 0.;
      Yoff = Hcal_inner_radius + Hcal_total_dim_y/2.;
      
      G4double chamber_x_offset, chamber_y_offset, chamber_z_offset;
      chamber_x_offset = xShift;
      chamber_z_offset = 0;
      chamber_y_offset = -Hcal_total_dim_y/2. 
	+ (logical_layer_id-1) *(Hcal_chamber_thickness + Hcal_radiator_thickness)
	+ Hcal_radiator_thickness + Hcal_chamber_thickness/2.;
      //+ Hcal_radiator_thickness + (Hcal_chamber_thickness - Hcal_fiber_gap)/2.;
      
      new MyPlacement(0,
		      G4ThreeVector(chamber_x_offset,
				    chamber_z_offset,
				    chamber_y_offset + chambers_y_off_correction),
		      //!!attention!! y<->z
		      ChamberLog [layer_id],
		      "ChamberBarrel",
		      MotherLog,
		      false,
		      layer_id);
      
      
      theBarrilRegSD->AddLayer(layer_id,
			       //chamber_x_offset + 
			       Xoff - 
			       2*((G4Box *)ChamberLog[layer_id]->GetSolid())->GetXHalfLength(),
			       chamber_y_offset + Yoff,
			       chamber_z_offset - 
			       ((G4Box *)ChamberLog[layer_id]->GetSolid())->GetYHalfLength());  
      
#ifdef MOKKA_GEAR
      if (layer_id <= Hcal_nlayers){
	// count layers
	helpBarrel.count += 1;
	// get position for each layer
	// check for dynamic_cast
	G4Box * solidOfLog = dynamic_cast<G4Box*>(ChamberLog[layer_id]->GetSolid());
	if ( solidOfLog == 0 ) {
	  Control::Abort("SHcalSc02: BarrelChambers are expected to be of type G4Box\nerror in 'HcalScint::BarrelRegularChambers' construction MokkaGear.",MOKKA_OTHER_ERRORS);
	}
	//helpBarrel.layerPos.push_back(chamber_y_offset + solidOfLog->GetZHalfLength() + Hcal_fiber_gap/2);
	helpBarrel.layerPos.push_back(chamber_y_offset + solidOfLog->GetZHalfLength());
	
      }
#endif
    }//end loop over HCAL nlayers;
  
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~     Build HCAL layer supports                     ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalSc02::BarrelChambersSupportTrap(G4LogicalVolume *MotherLog,
					  G4double chambers_y_off_correction)
{
  G4Trap *ChamberSupportSolidTrap  = NULL;
  G4Box  *ChamberSupportSolidBox   = NULL;
  G4RotationMatrix *rotationMatrix = NULL;
  G4double xShiftChamberSupportTrap        = 0.;
  G4double xShiftChamberSupportTrap_bottom = 0.;
  G4double xShiftChamberSupportTrap_top    = 0.;
  G4double xShiftChamberSupportBox         = 0.;

  //For certain numbers of Hcal layers, like 8 and 48,
  //a layer is build on the Hcal_midle_dim_x, so it is contained both
  //in the bottom as well as in the top barrel (not necessarily half-half).
  //Therefore, the right angular wedge of that layer support overlaps 
  //with the mother volume. The bool below is used to check for these cases.
  //If it is true, just a simple box is drawn, instead of the right angular wedge.
  G4bool isOutsideCorner = false;
  G4double xShiftBoxInsteadCorner = 0.;

  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::Gray());
  VisAtt->SetForceWireframe(true);
  VisAtt->SetForceSolid(true);

  G4double x_halfLength = 0.;
  G4double y_halfHeight = Hcal_chamber_thickness/2.;
  G4double z_halfWidth  = Hcal_normal_dim_z/2. - Hcal_lateral_plate_thickness;
  G4double xOffset  = 0.;
  G4double xShift   = 0.;
  G4int logical_layer_id = 0;

  for (G4int layer_id = 1; layer_id <= (2*Hcal_nlayers); layer_id++){
    isOutsideCorner = false;
    this->CalculateXLayer(layer_id, logical_layer_id, xOffset, x_halfLength, xShift);

    G4double xAbsShiftChamberSupportTrap_bottom = (Hcal_middle_stave_gaps/2. + Hcal_layer_support_length
						   + 2.*x_halfLength + 2.*Hcal_layer_air_gap
						   + (Hcal_layer_support_length + y_halfHeight*tan(pi/8.))/2.);

    G4double xAbsShiftChamberSupportTrap_top =  (Hcal_middle_stave_gaps/2. + Hcal_layer_support_length
						 + 2.* x_halfLength + 2.*Hcal_layer_air_gap
						 + (Hcal_layer_support_length + y_halfHeight/tan(pi/8.))/2.);

    G4double xAbsOutsideCorner = (Hcal_middle_stave_gaps/2 + 2*Hcal_layer_air_gap + 2*Hcal_layer_support_length
				  +2*x_halfLength + 2*y_halfHeight/tan(pi/8.));

    if (layer_id <= Hcal_nlayers) {
      xShiftChamberSupportTrap_bottom = - xAbsShiftChamberSupportTrap_bottom;
      xShiftChamberSupportTrap_top    = - xAbsShiftChamberSupportTrap_top;
    }
    else if (layer_id > Hcal_nlayers) {
      xShiftChamberSupportTrap_bottom = xAbsShiftChamberSupportTrap_bottom;
      xShiftChamberSupportTrap_top    = xAbsShiftChamberSupportTrap_top;
    }

    //---- bottom barrel --------------------------------------------------------------
    if (logical_layer_id *(Hcal_radiator_thickness + Hcal_chamber_thickness) < Hcal_y_dim1_for_x ){
      ChamberSupportSolidTrap = new G4Trap("ChamberSupportSolidTrap",
					   2.*z_halfWidth,  //! really z
					   2.*y_halfHeight, //! really y
					   Hcal_layer_support_length + 2.*y_halfHeight*tan(pi/8.),
					   Hcal_layer_support_length);
      xShiftChamberSupportTrap = xShiftChamberSupportTrap_bottom;

      //bottom barrel, left side
      if (layer_id <= Hcal_nlayers){
	rotationMatrix = new G4RotationMatrix();
	rotationMatrix->rotateY(pi);
	rotationMatrix->rotateX(3*pi/2.);
      }
      //bottom barrel, right side
      if (layer_id > Hcal_nlayers) {
	rotationMatrix = new G4RotationMatrix();
	rotationMatrix->rotateX(pi/2.);
      }

    } else {//------- top barrel --------------------------------------------------
      ChamberSupportSolidTrap = new G4Trap("ChamberSupportSolidTrap",
					   2.*z_halfWidth,  //! really z
					   2.*y_halfHeight, //! really y
					   Hcal_layer_support_length + 2.*y_halfHeight/tan(pi/8.),
					   Hcal_layer_support_length);
      xShiftChamberSupportTrap = xShiftChamberSupportTrap_top;

      //check if the corner of the right angular wedge is outside the volume
      if (xAbsOutsideCorner > (Hcal_midle_dim_x/2)) {
	isOutsideCorner = true;
      }
      G4double xAbsShiftBoxInsteadCorner = (Hcal_middle_stave_gaps/2 + Hcal_layer_support_length 
					    + 2*Hcal_layer_air_gap + 2*x_halfLength
					    + Hcal_layer_support_length/2);

      //top barrel, left side
      if (layer_id <= Hcal_nlayers){
	rotationMatrix = new G4RotationMatrix();
	rotationMatrix->rotateY(pi);
	rotationMatrix->rotateX(-3*pi/2.);

	xShiftBoxInsteadCorner = - xAbsShiftBoxInsteadCorner;
      }
      //top barrel, right side
      if (layer_id > Hcal_nlayers) {
	rotationMatrix = new G4RotationMatrix();
	rotationMatrix->rotateX(-pi/2.);

	xShiftBoxInsteadCorner = xAbsShiftBoxInsteadCorner;
      }

    }//end top barrel

    G4LogicalVolume *ChamberSupportTrapLog = new G4LogicalVolume(ChamberSupportSolidTrap,
								 CGAGeometryManager::GetMaterial("aluminium"),
								 "ChamberSupportTrapLog");
    ChamberSupportTrapLog->SetVisAttributes(VisAtt);

    G4double chamber_x_offset, chamber_y_offset, chamber_z_offset;
    chamber_x_offset = xShiftChamberSupportTrap;
    chamber_z_offset = 0;
    chamber_y_offset = -Hcal_total_dim_y/2. 
      + (logical_layer_id-1) *(Hcal_chamber_thickness + Hcal_radiator_thickness)
      //+ Hcal_radiator_thickness + (Hcal_chamber_thickness - Hcal_fiber_gap)/2.;
      + Hcal_radiator_thickness + Hcal_chamber_thickness/2.;

    if (isOutsideCorner == false){
      new MyPlacement(rotationMatrix,
		      G4ThreeVector(chamber_x_offset, chamber_z_offset, chamber_y_offset + chambers_y_off_correction),
		      ChamberSupportTrapLog,
		      "ChamberSupportTrap",
		      MotherLog,
		      false,
		      layer_id);
    }

    //================================================================================
    //--- draw support boxes in the middle -------------------------------------------
    ChamberSupportSolidBox = new G4Box("ChamberSupportSolidBox",
				       Hcal_layer_support_length/2.,//x
				       z_halfWidth,                 //z attention
				       y_halfHeight);               //y attention
    G4LogicalVolume *ChamberSupportBoxLog = new G4LogicalVolume(ChamberSupportSolidBox,
								CGAGeometryManager::GetMaterial("aluminium"),
								"ChamberSupportBoxLog");
    ChamberSupportBoxLog->SetVisAttributes(VisAtt);

    G4double xAbsShiftChamberSupportBox = (Hcal_middle_stave_gaps/2. + Hcal_layer_support_length/2.);

    if (layer_id <= Hcal_nlayers) xShiftChamberSupportBox = - xAbsShiftChamberSupportBox;
    else xShiftChamberSupportBox = xAbsShiftChamberSupportBox;

    new MyPlacement(0,
		    G4ThreeVector(xShiftChamberSupportBox, 0, chamber_y_offset + chambers_y_off_correction),
		    ChamberSupportBoxLog,
		    "ChamberSupportBox",
		    MotherLog,
		    false,
		    layer_id);
    //=============================================================================
    //-------- draw a support box instead of the right angular wedge, if the ------
    //-------- corner of the wedge is outside the mother volume              ------
    if (isOutsideCorner == true) {
      new MyPlacement(0,
		      G4ThreeVector(xShiftBoxInsteadCorner, chamber_z_offset, 
				    chamber_y_offset + chambers_y_off_correction),
		      ChamberSupportBoxLog,
		      "ChamberSupportBox",
		      MotherLog,
		      false,
		      layer_id);
    }


  }//end loop over Hcal_nlayers;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~     Build air gaps between HCAL barrel chambers and layer support structures    ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalSc02::BarrelChambersGap(G4LogicalVolume *MotherLog,
				  G4double chambers_y_off_correction)
{
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::Cyan());
  VisAtt->SetForceWireframe(true);
  VisAtt->SetForceSolid(true);

  G4double x_length = 0.;
  G4double y_height = Hcal_chamber_thickness/2.;
  G4double z_width  = Hcal_normal_dim_z/2. - Hcal_lateral_plate_thickness;
  G4double xOffset  = 0.;
  G4double xShift   = 0.;
  G4int logical_layer_id = 0;


  for (G4int layer_id = 1; layer_id <= 2*Hcal_nlayers; layer_id++){
    this->CalculateXLayer(layer_id, logical_layer_id, xOffset, x_length, xShift);

    G4Box *ChamberGapSolid = new G4Box("ChamberGapSolid",
				       Hcal_layer_air_gap/2.,//x
				       z_width,              //z attention!
				       y_height);            //y attention!
    G4LogicalVolume *ChamberGapLog = new G4LogicalVolume(ChamberGapSolid,
							 //CGAGeometryManager::GetMaterial("air"),
							 CGAGeometryManager::GetMaterial("stainless_steel"),
							 "ChamberGapLog");
    ChamberGapLog->SetVisAttributes(VisAtt);

    G4double chamber_x_offset, chamber_y_offset, chamber_z_offset;
    chamber_x_offset = xShift;
    chamber_z_offset = 0;
    chamber_y_offset = -Hcal_total_dim_y/2. 
      + (logical_layer_id-1) *(Hcal_chamber_thickness + Hcal_radiator_thickness)
      + Hcal_radiator_thickness + Hcal_chamber_thickness/2.;


    //right side, right edge
    G4double xAbsShiftChamberGap_rightEdge = (Hcal_middle_stave_gaps/2. + Hcal_layer_support_length 
					      + 2.*x_length + 3/2.*Hcal_layer_air_gap);
    G4double xShiftChamberGap_rightEdge = 0.;
    if (layer_id <= Hcal_nlayers) xShiftChamberGap_rightEdge = - xAbsShiftChamberGap_rightEdge;
    else  xShiftChamberGap_rightEdge = xAbsShiftChamberGap_rightEdge;

    std::stringstream stringForLayerNo; /*string to save layer number*/
    stringForLayerNo << layer_id;

    new MyPlacement(0,
		    G4ThreeVector(xShiftChamberGap_rightEdge, 
				  0, chamber_y_offset + chambers_y_off_correction),
		    ChamberGapLog,
		    G4String("ChamberGapRight") + G4String(stringForLayerNo.str()),
		    MotherLog,
		    false,
		    layer_id);

    //right side, left edge
    G4double xAbsShiftChamberGap_leftEdge = 
      (Hcal_middle_stave_gaps/2. + Hcal_layer_support_length + Hcal_layer_air_gap/2.);

    G4double xShiftChamberGap_leftEdge = 0.;
    if (layer_id <= Hcal_nlayers)  xShiftChamberGap_leftEdge = -xAbsShiftChamberGap_leftEdge;
    else  xShiftChamberGap_leftEdge = xAbsShiftChamberGap_leftEdge;

    new MyPlacement(0,
		    G4ThreeVector(xShiftChamberGap_leftEdge, 
				  0, chamber_y_offset + chambers_y_off_correction),
		    ChamberGapLog,
		    "ChamberGapLeft",
		    MotherLog,
		    false,
		    layer_id);

  }//end loop over Hcal_nlayers

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Calculate x-length of an HCAL barrel layer                  ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalSc02::CalculateXLayer(G4int layer_id, G4int &logical_layer_id,
				G4double &xOffset, G4double &x_halfLength, G4double &xShift)
{
  G4double TanPiDiv8 = tan(pi/8.);
  G4double x_total   = 0.;
  G4double x_length  = 0.;


  if ( (layer_id < Hcal_nlayers)
       || (layer_id > Hcal_nlayers && layer_id < (2*Hcal_nlayers)) )
    logical_layer_id = layer_id % Hcal_nlayers;
  else if ( (layer_id == Hcal_nlayers) 
	    || (layer_id == 2*Hcal_nlayers) ) logical_layer_id = Hcal_nlayers;

  //---- bottom barrel------------------------------------------------------------
  if(logical_layer_id *(Hcal_radiator_thickness + Hcal_chamber_thickness) < Hcal_y_dim1_for_x ){

    xOffset = (logical_layer_id * Hcal_radiator_thickness 
	       + (logical_layer_id -1) * Hcal_chamber_thickness) * TanPiDiv8;

    x_total  = Hcal_bottom_dim_x/2 - Hcal_middle_stave_gaps/2 + xOffset;
    x_length = x_total - 2*Hcal_layer_support_length - 2*Hcal_layer_air_gap;
    x_halfLength = x_length/2.;

  } else {//----- top barrel -------------------------------------------------
    xOffset = (logical_layer_id * Hcal_radiator_thickness 
	       + (logical_layer_id - 1) * Hcal_chamber_thickness - Hcal_y_dim1_for_x) / TanPiDiv8
      + Hcal_chamber_thickness / TanPiDiv8;

    x_total  = Hcal_midle_dim_x/2. - Hcal_middle_stave_gaps/2. - xOffset;
    x_length = x_total - 2.*Hcal_layer_support_length  - 2.* Hcal_layer_air_gap;
    x_halfLength = x_length/2.;

  }

  G4double xAbsShift = (Hcal_middle_stave_gaps/2 + Hcal_layer_support_length + Hcal_layer_air_gap + x_halfLength);

  if (layer_id <= Hcal_nlayers)     xShift = - xAbsShift;
  else if (layer_id > Hcal_nlayers) xShift = xAbsShift;

  //   G4cout<<"layer_id="<<layer_id<<" logical_layer_id="<<logical_layer_id
  // 	<<" xOffset="<<xOffset<<" x_halfLength="<<x_halfLength<<" xShift="<<xShift<<G4endl;
 
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Build HCAL endcaps                                            ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalSc02::EndcapsTesla(G4LogicalVolume* MotherLog)
{
  G4double tanPiDiv8 = tan(pi/8.);
  G4double tanPiDiv4 = tan(pi/4.);

#ifdef MOKKA_GEAR
  // Write parameters in helper class
  // attention: the outer symmetrie is 32-fold... this is not
  // taken into account in gear
  
  // The outer radius is in Geant4 defined as the tangent outer radius.
  // same is taken in Gear to be consistent with Ecal and Barrel
  helpEndcap.outerRadius = Hcal_endcap_rmax;  // outer radius
  helpEndcap.innerRadius = Hcal_endcap_center_box_size/2.;  // inner radius
  helpEndcap.phi0 = 0.; // phi0
  
  // Inner_z minimum 
  helpEndcap.leastZ = Hcal_start_z;

  // Add first position of layer as ground level
  helpEndcap.layerPos.push_back( - Hcal_endcap_total_z/2. ) ;
#endif

  
  //First: dimensions of the trapezoid
  //half length of a hexagon side
  G4double half_length  = Hcal_endcap_rmax * tan(pi/8.);

  //Hcal_stave_gaps=200;
  //Hcal_lateral_plate_thickness=200;
  G4double trap_small_x = half_length + Hcal_endcap_center_box_size/2. - Hcal_lateral_plate_thickness * tanPiDiv8;
  G4double trap_x       = Hcal_endcap_rmax + Hcal_endcap_center_box_size/2 - Hcal_lateral_plate_thickness;
  G4double trap_y       = Hcal_endcap_total_z;
  G4double trap_z       = (Hcal_endcap_rmax + Hcal_endcap_center_box_size/2 - trap_small_x - Hcal_lateral_plate_thickness)/tanPiDiv4;


  G4Trap *TopEndcap = new G4Trap("TopEndcap", 
				 trap_y, /*length in z*/
				 trap_z,
				 trap_x,
				 trap_small_x);

  //-----------------------------------------------
  //Then: dimensions of the box (box expects half side length as an input)
  G4double box_half_x = trap_x/2.;
  G4double box_half_y = trap_y/2;
  G4double box_half_z = (Hcal_endcap_rmax - trap_z - Hcal_endcap_center_box_size/2 - Hcal_stave_gaps - Hcal_lateral_plate_thickness)/2.;

  G4Box *BottomEndcap = new G4Box("BottomEndcap",
				  box_half_x,
				  box_half_z,
				  box_half_y);


  //-----------------------------------------------
  //Build an union out of the two
  //---------
  //x-dimension of the trapezoid center of gravity
  G4double trap_center_of_grav_half_x = (trap_small_x + trap_z/2 * tan(pi/4.))/2;

  //shift the top trapezoidal part with respect to the bottom part to get the union
  G4double shift_x = (-1)*abs(box_half_x - trap_center_of_grav_half_x);
  G4double shift_y = box_half_z + trap_z/2.;
  G4UnionSolid* EndcapStave = new G4UnionSolid("EndcapStave",
					       BottomEndcap,
					       TopEndcap,
					       0,
					       G4ThreeVector(shift_x, 
							     shift_y, 
							     0)); 
 
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::Magenta());
  VisAtt->SetForceWireframe(true);
  VisAtt->SetDaughtersInvisible(true);
  VisAtt->SetForceSolid(false);
#ifdef SHCALSC02_DEBUG
  VisAtt->SetColour(G4Colour(.8,.2,.2));
  VisAtt->SetDaughtersInvisible(false); 
#endif

  //Create the endcap logical volume
  G4LogicalVolume *EndcapLogical = new G4LogicalVolume(EndcapStave, EndcapRadiatorMaterial, "EndcapLogical", 0, 0, 0);
  EndcapLogical->SetVisAttributes(VisAtt);


  //===================================================
  //The most important part: build the chambers 
  //
  if(Hcal_sensitive_model == "scintillator")
    {
      EndcapChambersTesla(EndcapLogical,theENDCAPEndSD);
    }
  else Control::Abort("SHcalSc02: Invalid sensitive HCAL model !",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
  
  //
  //==================================================

  //Place....
  G4double endcap_z_offset = Hcal_start_z + Hcal_endcap_total_z/2. ;
  G4int ModuleNumber = 0;
  G4double Z1 = 0;
  G4RotationMatrix *rotEffect = new G4RotationMatrix();
  G4double angleRotationZ = 0;
  G4double angleRotationY = 0;
  G4double a=0, b=0, c=0, d=0, e=0; //helper variables
  //------------------------
  //endcapId=1: staveId=1
  //            x_shift = box_half_x - Hcal_endcap_center_box_size/2;
  //            y_shift = box_half_z + Hcal_endcap_center_box_size/2 + Hcal_stave_gaps;
  //endcapId=2  x_shift = - x_shift
  //            y_shift = y_shift
  //
  //endcapId=1: staveId=2
  //            x_shift = box_half_z + Hcal_endcap_center_box_size/2 + Hcal_stave_gaps;
  //            y_shift = - (box_half_x - Hcal_endcap_center_box_size/2);
  //endcapId=2  x_shift = - x_shift
  //            y_shift = y_shift
  //
  //endcapId=1: staveId=3
  //            x_shift = - (box_half_x - Hcal_endcap_center_box_size/2)
  //            y_shift = - (Hcal_endcap_center_box_size/2 + box_half_z + Hcal_stave_gaps);
  //endcapId=2  x_shift = - x_shift
  //            y_shift = y_shift
  //
  //endcapId=1: staveId=4
  //            x_shift = - (box_half_z + Hcal_endcap_center_box_size/2 + Hcal_stave_gaps)
  //            y_shift = box_half_x - Hcal_endcap_center_box_size/2 
  //endcapId=2  x_shift = - x_shift
  //            y_shift = y_shift

  

  //--------- loop over endcap id ---------------------
  for (G4int endcapId = 1; endcapId <= 2; endcapId ++)
    {
      //--------- loop over endcap stave id ---------------
      for (G4int endcapStaveId = 1; endcapStaveId <= 4; endcapStaveId++)
	{
	  Z1 = endcap_z_offset;
	  
	  if (endcapId < 2) ModuleNumber = HCALENDCAPPLUS * 10 + endcapStaveId;
	  else              ModuleNumber = HCALENDCAPMINUS * 10 + endcapStaveId;
	  
	  
	  angleRotationZ = (endcapStaveId - 1) * pi/2.;
	  theENDCAPEndSD->SetStaveRotationMatrix(endcapStaveId, angleRotationZ);
	  
	  a = (endcapStaveId + 2)%2 * std::pow(-1., ((endcapStaveId + 2)%4)%3 );
	  b = (endcapStaveId + 1)%2 * std::pow(-1., ((endcapStaveId + 1)%4)%3 );
	  c = std::pow(-1., (endcapStaveId % 3) % 2);
	  shift_x = a * box_half_x + b * box_half_z + c * Hcal_endcap_center_box_size/2. + b * Hcal_stave_gaps;
	  if (endcapId == 2) shift_x = -shift_x;

	  d = (endcapStaveId + 3)%2 * std::pow(-1., ((endcapStaveId + 3)%4)%3 );
	  e = std::pow( -1., G4int((endcapStaveId - 1)/2.) );
 	  shift_y = d * box_half_x + a * box_half_z + e * Hcal_endcap_center_box_size/2 + a * Hcal_stave_gaps; 

	  rotEffect = new G4RotationMatrix();
	  rotEffect->rotateY(angleRotationY);
	  rotEffect->rotateZ(angleRotationZ);
	  
	  new MyPlacement(rotEffect,
			  //G4ThreeVector(0., 0., Z1),
			  G4ThreeVector(shift_x, shift_y, Z1),
			  EndcapLogical,
			  "EndCapPhys",
			  MotherLog,
			  false,
			  ModuleNumber);
	
	}
      //--------- end loop over endcap stave id -----------
     
      endcap_z_offset = - endcap_z_offset;
      angleRotationY = pi;

      if (endcapId == 1)      theENDCAPEndSD->SetModuleZOffset(HCALENDCAPPLUS, fabs(Z1));
      else if (endcapId == 2) theENDCAPEndSD->SetModuleZOffset(HCALENDCAPMINUS, fabs(Z1));

    }//---------- end loop over endcap id -----------------

}
