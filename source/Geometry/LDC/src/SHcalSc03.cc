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
   Shaojun Lu:  Barrel driver has been changed for the new engineering design shape.
                Endcaps driver has been changed for the new engineering design shape.
 */

#include "Control.hh"
#include "MySQLWrapper.hh"
#include "MyPlacement.hh"
#include "G4UserLimits.hh"
#include "SHcalSc03.hh"
#include "CGAGeometryManager.hh"
#include "SDHcalBarrel.hh"
#include "SDHcalEndCap.hh"
#include "SDAHcalEndCap.hh"

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

  //#define SHCALSC03_DEBUG
#define SHCAL_CHECK_OVERLAP 0

INSTANTIATE(SHcalSc03)

  G4bool SHcalSc03::ContextualConstruct(const CGAGeometryEnvironment 
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
    Control::Abort("SHcalSc03: invalid radiator material name. \nIt has to be either Iron either WMod!!!",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);

  Hcal_radiator_material+= " is the radiator material being placed.";
  Control::Log(Hcal_radiator_material.data());


  if(Hcal_endcap_radiator_material == "Iron")
    EndcapRadiatorMaterial =  CGAGeometryManager::GetMaterial("iron");
  else if(Hcal_endcap_radiator_material == "WMod")
    EndcapRadiatorMaterial =  CGAGeometryManager::GetMaterial("tungsten_11gccm");
  else if(Hcal_endcap_radiator_material == "TungstenDens24")
    EndcapRadiatorMaterial =  CGAGeometryManager::GetMaterial("TungstenDens24");
  else
    Control::Abort("SHcalSc03: invalid EndcapRadiator material name. \nIt has to be either Iron either WMod or TungstenDens24!!!",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);

  Hcal_endcap_radiator_material+= " is the endcap radiator material being placed.";
  Control::Log(Hcal_endcap_radiator_material.data());

  /*--------------------------------------------------------------------------*/
 S235 = CGAGeometryManager::GetMaterial("S235");

    /* PCB (Printed Circuit Board) Material FR4
     Composition and density found under 
     http://pcba10.ba.infn.it/temp/ddd/ma/materials/list.html 
  */
  PCB = CGAGeometryManager::GetMaterial("PCB");

  Cu = CGAGeometryManager::GetMaterial("G4_Cu");

  Air = CGAGeometryManager::GetMaterial("air");

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
				    (Hcal_middle_stave_gaps/2 + Hcal_layer_air_gap),
				    Hcal_apply_Birks_law);
  RegisterSensitiveDetector(theBarrilRegSD);

  if (Hcal_sensitive_model == "scintillator")
    {	
      //draw the HCAL barrel
      BarrelRegularModules(WorldLog);
    } 
  else Control::Abort("SHcalSc03: Invalid sensitive model for the chosen HCAL superdriver!",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);

  //====================================================
  //                                                  //
  // HCAL endcap modules                              //
  //                                                  //
  //====================================================
  // Hcal  endcap modules
  theENDCAPEndSD = new SDAHcalEndCap(Hcal_cell_dim_x,
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
      EndcapsAhcal(WorldLog);
    }
  else Control::Abort("SHcalSc03: Invalid sensitive model for the chosen HCAL superdriver!",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
  
  
  //==================================================//
  //                                                  //
  // HCAL endcap rings                                //
  //                                                  //
  //==================================================//
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
  dblParamSteelCassetteThickness        = gearMgr->tmpParam.getDoubleVal( "Hcal_steel_cassette_thickness" );
  dblParamScintillatorThickness         = gearMgr->tmpParam.getDoubleVal( "Hcal_scintillator_thickness" );
  dblParamCuThickness                   = gearMgr->tmpParam.getDoubleVal( "Hcal_Cu_thickness" );
  dblParamPCBThickness                  = gearMgr->tmpParam.getDoubleVal( "Hcal_PCB_thickness" );
  dblParamLayerAirGap                   = gearMgr->tmpParam.getDoubleVal( "Hcal_layer_air_gap" );
  dblParamMiddleStaveGaps               = gearMgr->tmpParam.getDoubleVal( "Hcal_middle_stave_gaps" ) ;
  dblParamHcalModulesGap                = gearMgr->tmpParam.getDoubleVal( "Hcal_modules_gap" );
  dblParamHcalStaveGaps                 = gearMgr->tmpParam.getDoubleVal( "Hcal_stave_gaps" );
  dblParamTPCEcalHcalbarrelHalfZ        = gearMgr->tmpParam.getDoubleVal( "TPC_Ecal_Hcal_barrel_halfZ" );
  dblParamHcalLateralStructureThickness = gearMgr->tmpParam.getDoubleVal( "Hcal_lateral_structure_thickness" );
  dblHcalBarrelEndModuleType            = gearMgr->tmpParam.getIntVal(    "Hcal_barrel_end_module_type" );
  dblParamHcalEndcapSensitiveCenterBox  = gearMgr->tmpParam.getDoubleVal( "Hcal_endcap_sensitive_center_box" );


  // calculate zMax as total length/2
  helpBarrel.zMax = (helpBarrel.mostZ - helpBarrel.leastZ) / 2 ;

  // HCAL Barrel
  gear::CalorimeterParametersImpl* barrelParam = 
    new gear::CalorimeterParametersImpl( helpBarrel.innerRadius, helpBarrel.zMax, 8, helpBarrel.phi0 );

  // write all layers by position
  for (int i = 0; i < helpBarrel.count; i++) 
    {
      G4double calcThick  = helpBarrel.layerPos[i+1] - helpBarrel.layerPos[i] ;
      G4double calcAbsorb = calcThick - helpBarrel.fiberGap[i]
	- helpBarrel.sensThickness[i] - helpBarrel.PCBThickness[i] - helpBarrel.CuThickness[i];

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
  barrelParam->setDoubleVal( "Hcal_scintillator_thickness",      dblParamScintillatorThickness);
  barrelParam->setDoubleVal( "Hcal_Cu_thickness",                dblParamCuThickness);
  barrelParam->setDoubleVal( "Hcal_PCB_thickness",               dblParamPCBThickness);
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
      G4double calcAbsorb = calcThick - Hcal_chamber_thickness;

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
  endcapParam->setDoubleVal( "Hcal_endcap_sensitive_center_box",  dblParamHcalEndcapSensitiveCenterBox );
  endcapParam->setDoubleVal( "Hcal_steel_cassette_thickness",     dblParamSteelCassetteThickness );
  endcapParam->setDoubleVal( "Hcal_scintillator_thickness",       dblParamScintillatorThickness );
  endcapParam->setDoubleVal( "Hcal_Cu_thickness",                 dblParamCuThickness );
  endcapParam->setDoubleVal( "Hcal_PCB_thickness",                dblParamPCBThickness );
  endcapParam->setDoubleVal( "Hcal_endcap_module_width",          Hcal_endcap_module_width );
  endcapParam->setDoubleVal( "Hcal_endcap_lateral_structure_thickness",  Hcal_endcap_lateral_structure_thickness );
  endcapParam->setDoubleVal( "Hcal_endcap_layer_air_gap",      Hcal_endcap_layer_air_gap );
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
      G4double calcAbsorb = calcThick - helpEndcapRing.steelCassetteThickness[i]  
	- helpEndcapRing.sensThickness[i] - helpEndcapRing.PCBThickness[i] - helpEndcapRing.CuThickness[i];  
      
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
  endcapRingParam->setDoubleVal( "Hcal_steel_cassette_thickness",     dblParamSteelCassetteThickness );
  endcapRingParam->setDoubleVal( "Hcal_scintillator_thickness",       dblParamScintillatorThickness );
  endcapRingParam->setDoubleVal( "Hcal_Cu_thickness",                 dblParamCuThickness );
  endcapRingParam->setDoubleVal( "Hcal_PCB_thickness",                dblParamPCBThickness );
  gearMgr->setHcalRingParameters( endcapRingParam ) ;

#endif

  // Closes Database connection
  delete db;

#ifdef SHCALSC03_DEBUG
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
SHcalSc03::~SHcalSc03() 
{
}  


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              Barrel Regular Modules               ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalSc03::BarrelRegularModules(G4LogicalVolume* MotherLog)
{
  G4double BottomDimY =   Hcal_total_dim_y / 2.;
  G4double chambers_y_off_correction = 0.;
 
  // stave modules shaper parameters
  G4double BHX  = (Hcal_bottom_dim_x + Hcal_stave_gaps)/2.;
  G4double THX  = (Hcal_total_dim_y + Hcal_inner_radius)*tan(pi/8.);
  G4double YXH  = Hcal_total_dim_y / 2.;
  G4double DHZ  = (Hcal_normal_dim_z - Hcal_lateral_plate_thickness) / 2.;

  G4Trd * stave_shaper = new G4Trd("stave_shaper",
			     BHX, THX, DHZ, DHZ, YXH);

  G4VSolid *solidCaloTube=new G4Tubs("HcalBarrelTube",0*cm,Hcal_module_radius,
				     DHZ,
				     0*deg,360*deg);
  
  G4RotationMatrix *rot = new G4RotationMatrix(G4ThreeVector(1,0,0),360/4.0*deg);

  G4IntersectionSolid* ModuleSolid = new G4IntersectionSolid("ModuleSolid",
						       stave_shaper,
						       solidCaloTube,
						       rot,
						       G4ThreeVector(0, 0, -(Hcal_inner_radius + Hcal_total_dim_y / 2.)) );

  EnvLogHcalModuleBarrel  = new G4LogicalVolume(ModuleSolid,
						BarrelRadiatorMaterial,//iron or tungsten
						"barrelHcalModule", 
						0, 0, 0);

  G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(.8,.8,.2));
  VisAtt->SetForceWireframe(true);
  VisAtt->SetForceSolid(false);
  VisAtt->SetForceLineSegmentsPerCircle(360);
  VisAtt->SetDaughtersInvisible(false); 
  EnvLogHcalModuleBarrel->SetVisAttributes(VisAtt);

#ifdef SHCALSC03_DEBUG
  std::cout<< " ==> Hcal_module_radius: "<<Hcal_module_radius <<std::endl;
#endif

  //stave modules lateral plate shaper parameters
  G4double BHX_LP  = BHX;
  G4double THX_LP  = THX;
  G4double YXH_LP  = YXH;

  //build lateral palte here to simulate lateral plate in the middle of barrel.
  G4double DHZ_LP  = Hcal_lateral_plate_thickness; 

  G4Trd * stave_shaper_LP = new G4Trd("stave_shaper_LP",
			     BHX_LP, THX_LP, DHZ_LP, DHZ_LP, YXH_LP);

 
  G4VSolid *solidCaloTube_LP=new G4Tubs("HcalBarrelTube_LP",0*cm,Hcal_module_radius,
				     DHZ,
				     0*deg,360*deg);
  


  G4IntersectionSolid* Module_lateral_plate = new G4IntersectionSolid("Module_Lateral_Plate",
						       stave_shaper_LP,
						       solidCaloTube_LP,
						       rot,
						       G4ThreeVector(0, 0, -(Hcal_inner_radius + Hcal_total_dim_y / 2.)) );

  G4LogicalVolume* EnvLogHcalModuleBarrel_LP  = new G4LogicalVolume(Module_lateral_plate,
						   BarrelRadiatorMaterial,//iron or tungsten
						   "barrelHcalModule_LP", 
						   0, 0, 0);

 
  VisAtt = new G4VisAttributes(G4Colour(.8,.8,.2));
  VisAtt->SetForceWireframe(true);
  VisAtt->SetForceSolid(true);
  VisAtt->SetForceLineSegmentsPerCircle(360);
  VisAtt->SetDaughtersInvisible(false); 
  EnvLogHcalModuleBarrel_LP->SetVisAttributes(VisAtt);



#ifdef MOKKA_GEAR
  // calculate ground-level for layers
  G4double yTotal = Hcal_total_dim_y/2.;
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
  //BarrelModuleGap(EnvLogHcalModuleBarrel);

  //build the chambers (scintillator + air gap for cabels)
  BarrelHalfRegularChambersTrap(EnvLogHcalModuleBarrel, chambers_y_off_correction);

  //build the air gap between chambers and layer support structure
  BarrelChambersGap(EnvLogHcalModuleBarrel, chambers_y_off_correction);


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

  //TODO: rotaion pi/8.0 (22.5 degree) with the updated engineering design, after agree with all sub-detectors.
  stave_phi_offset = 0;

  //-------- start loop over HCAL BARREL staves ----------------------------
  for (G4int stave_id = 1;
       stave_id <= 8;
       stave_id++)
    {
      module_z_offset = - (Hcal_normal_dim_z + Hcal_modules_gap + Hcal_lateral_plate_thickness)/2.;


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


      G4RotationMatrix *rot_LP = new G4RotationMatrix();
      rot_LP->rotateX(pi*0.5);
      rot_LP->rotateY(stave_phi_offset);
      new MyPlacement(rot_LP,
		      G4ThreeVector(-Y*sin(stave_phi_offset),
				    Y*cos(stave_phi_offset),
				    0),
		      EnvLogHcalModuleBarrel_LP,
		      "BarrelHcalModule_LP",
		      MotherLog,
		      false,
		      0);



      stave_phi_offset -=  pi/4.;
    }
  //-------- end loop over HCAL BARREL staves ----------------------------

}



//=======================================================================
//
//
//=======================================================================
void SHcalSc03::EndcapRingChambers(G4LogicalVolume* MotherLog, SDHcalEndCap* theSD, bool rings)
{
  // Chambers in the SHcalSc03::EndcapRings
  // standard endcap chamber solid:
  G4Polyhedra *motherSolid = (G4Polyhedra*) MotherLog->GetSolid();

  G4PolyhedraHistorical* motherPolyhedraParameters = motherSolid->GetOriginalParameters();

  G4double pRMax, pDz, pRMin;

  pRMax = (*(motherPolyhedraParameters->Rmax) * cos(pi/motherPolyhedraParameters->numSide))
    - (Hcal_lateral_plate_thickness);

  pDz = Hcal_chamber_thickness / 2.;

  pRMin = ( *(motherPolyhedraParameters->Rmin) * cos(pi/motherPolyhedraParameters->numSide))
    + (Hcal_lateral_plate_thickness);

  // G4Polyhedra Envelope parameters
  G4double phiStart = pi/8.;
  G4double phiTotal = 2.*pi;
  G4int numSide     = motherPolyhedraParameters->numSide;
  G4int numZPlanes  = 2;

  G4double zPlane[2];
  zPlane[0] = - pDz;
  zPlane[1] = - zPlane[0];

  G4double rInner[2],rOuter[2];
  rInner[0] = rInner[1] = pRMin;
  rOuter[0] = rOuter[1] = pRMax;

#ifdef SHCALSC03_DEBUG
  if(rings==true){
    G4cout<<"    EndcapRingsSensitive: Rinner="<<pRMin<<"mm Router="<<pRMax<<G4endl<<G4endl;
  }
#endif


  G4Polyhedra *HcalEndCapRingChamberSolid = new G4Polyhedra("HcalEndCapRingChamberSolid",
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
  G4ThreeVector IntersectXYZtrans((pRMax + (Hcal_stave_gaps/2.)),
				  (pRMax + (Hcal_stave_gaps/2.)),
				  (Hcal_total_dim_y/2.));
  G4RotationMatrix *rot = new G4RotationMatrix;
  rot->rotateZ(0.);
  // intersect the octagonal layer with a square to get only one quadrant
  G4IntersectionSolid  *HcalEndCapRingStaveSolid = new G4IntersectionSolid( "HcalEndCapRingStaveSolid",
								    HcalEndCapRingChamberSolid,
								    IntersectionStaveBox,
								    rot, 
								    IntersectXYZtrans); 

  G4UserLimits* pULimits = new G4UserLimits(theMaxStepAllowed);

  // Endcap ring chamber logical
  G4LogicalVolume* HcalEndCapRingStaveLogical = 0;

  if(Hcal_sensitive_model == "scintillator")
    {
      //fg: introduce (empty) fiber gap - should be filled with fibres and cables
      // - so far we fill it  with air ...
      HcalEndCapRingStaveLogical = new G4LogicalVolume(HcalEndCapRingStaveSolid,
					       CGAGeometryManager::GetMaterial("air"), 
					       "HcalEndCapRingChamberLogical", 
					       0, 0, 0);

      G4double scintHalfWidth = pDz - (Hcal_PCB_thickness + Hcal_Cu_thickness) / 2. ;

      // fiber gap can't be larger than total chamber
      assert( scintHalfWidth > 0. ) ;

      G4double zPlaneScint[2];
      zPlaneScint[0] = - scintHalfWidth ;
      zPlaneScint[1] = - zPlaneScint[0];

      G4Polyhedra *HcalEndCapRingScintSolid = new G4Polyhedra("HcalEndCapRingScintSolid",
						      phiStart,
						      phiTotal,
						      numSide,
						      numZPlanes,
						      zPlaneScint,
						      rInner,
						      rOuter);
      G4IntersectionSolid  *HcalEndCapRingScintStaveSolid = new G4IntersectionSolid( "HcalEndcapRingScintStaveSolid",
									     HcalEndCapRingScintSolid,
									     IntersectionStaveBox,
									     rot, 
									     IntersectXYZtrans);

      G4LogicalVolume* RingScintLog = new G4LogicalVolume(HcalEndCapRingScintStaveSolid,
						      CGAGeometryManager::GetMaterial("polystyrene"),
						      "HcalEndCapRingScintLogical", 
						      0, 0, pULimits);  
      G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::Yellow());
#ifdef SHCALSC03_DEBUG
      if(rings==true){
	VisAtt->SetColour(G4Colour(.2,.5,.2));
      }
#endif
      VisAtt->SetForceSolid(true);
      RingScintLog->SetVisAttributes(VisAtt);

      // only scintillator is sensitive
      RingScintLog->SetSensitiveDetector(theSD);

#ifdef MOKKA_GEAR
      // thickness of sensible part as often as zPlanes are there
      if(rings==true){
	helpEndcapRing.sensThickness.push_back( Hcal_scintillator_thickness );
	helpEndcapRing.steelCassetteThickness.push_back( Hcal_steel_cassette_thickness ) ;
	helpEndcapRing.PCBThickness.push_back( Hcal_PCB_thickness ) ;
	helpEndcapRing.CuThickness.push_back( Hcal_Cu_thickness ) ;
      }
#endif

      new MyPlacement(0, 
		      G4ThreeVector( 0, 0,  - (Hcal_PCB_thickness + Hcal_Cu_thickness) / 2.), 
		      RingScintLog,
		      "HcalEndCapRingScintillator", 
		      HcalEndCapRingStaveLogical, 
		      false, 
		      0);   
    }
  else Control::Abort("SHcalSc03: Invalid sensitive model parameter!",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);

  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::Blue());
#ifdef SHCALSC03_DEBUG
  if(rings==true){
    VisAtt->SetColour(G4Colour(.0,.2,.0));
  }
#endif
  HcalEndCapRingStaveLogical->SetVisAttributes(VisAtt);

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
        + (Hcal_chamber_thickness - Hcal_PCB_thickness - Hcal_Cu_thickness)/2.;

      //place the four staves in their right positions
      for (G4int stave_id = 1;
	   stave_id <= 4;
	   stave_id++)
	{
	  G4RotationMatrix *rotEffect = new G4RotationMatrix();
	  rotEffect->rotateZ(((stave_id-1)*pi/2.));
	  new MyPlacement(rotEffect,
			  G4ThreeVector(0.,0.,Zoff),
			  HcalEndCapRingStaveLogical,
			  "HcalEndCapRingStavePhys",
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
	helpEndcapRing.layerPos.push_back(Zoff + std::abs(zPlane[0]) + (Hcal_PCB_thickness + Hcal_Cu_thickness)/2 ) ;

	helpEndcapRing.sensThickness.push_back( Hcal_scintillator_thickness );
	helpEndcapRing.steelCassetteThickness.push_back( Hcal_steel_cassette_thickness );
	helpEndcapRing.PCBThickness.push_back(Hcal_PCB_thickness);
	helpEndcapRing.CuThickness.push_back(Hcal_Cu_thickness);
      }
      
#endif

    }  
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Build HCAL endcaps                                            ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalSc03::EndcapsAhcal(G4LogicalVolume* MotherLog)
{
	G4cout << "Hcal_R_max: " << Hcal_module_radius <<G4endl;
	
	G4UserLimits* pULimits = new G4UserLimits(theMaxStepAllowed);
	
	G4double box_half_x = Hcal_endcap_module_width/2.; //375.0 mm module
	
	G4double theXOffset = - (Hcal_endcap_module_width + eps) * Hcal_endcap_module_number/2. - eps/2.; //8 modules.
	
	// Initialize box_half_y from module_length, which number is from Karsten technical design.
	// module_length has been write into Mokka database table "hcal04"
	G4double box_half_y[MAX_ENDCAP_MODULES_NUMBER];

	G4String aSubDetectorName = "hcal04"; //Mokka database table for endcap modules length
	db = new Database(aSubDetectorName);
	db->exec("select * from endcap;");
	for(int i = 0; i<MAX_ENDCAP_MODULES_NUMBER; i++){
	  db->getTuple();
	  box_half_y[i] = db->fetchDouble("module_length")/2.0;
	}

	G4double endcap_rmax = -1.0;

	for(int i = 0; i<6; i++)
	  {
	    G4double endcap_radius = sqrt( (box_half_y[i]*2.+ Hcal_endcap_services_module_width) 
					   * (box_half_y[i]*2.+ Hcal_endcap_services_module_width) 
					   + Hcal_endcap_module_width * ( Hcal_endcap_module_number/2. -i)
					   * Hcal_endcap_module_width * ( Hcal_endcap_module_number/2. -i) );

	    if ( endcap_radius > endcap_rmax ) endcap_rmax = endcap_radius;
	  }

	G4cout << "endcap_rmax: " << endcap_rmax <<G4endl;

	
	G4double box_half_z = Hcal_endcap_total_z/2;
	
	G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::Green());
	VisAtt->SetForceWireframe(false);
	VisAtt->SetDaughtersInvisible(false);
	VisAtt->SetForceSolid(false);
	
	
	// the shift are calculate from the center of the module,
	// all module will use the same function.
	G4double shift_x;
	G4double shift_y[MAX_ENDCAP_MODULES_NUMBER];
	G4double shift_z = Hcal_start_z + Hcal_endcap_total_z/2.;
	
	//--------- front_end_electronic ----------
	G4Box *FEE = new G4Box("FEE", box_half_x, Hcal_endcap_services_module_width/2.,box_half_z);
	G4LogicalVolume *FEELogical = new G4LogicalVolume(FEE, Air, "FEELogical", 0, 0, 0);
	
	for (G4int layer_id = 1; layer_id <= Hcal_endcap_nlayers; layer_id++) {
		EndcapsAhcalFrontEndElectronics(FEELogical,layer_id);
	}
	
	G4VisAttributes *VisAttFEE = new G4VisAttributes(G4Colour::Blue());
	VisAttFEE->SetForceWireframe(true);
	VisAttFEE->SetDaughtersInvisible(false);
	VisAttFEE->SetForceSolid(false);
	
	FEELogical->SetVisAttributes(VisAttFEE);
	
	
#ifdef MOKKA_GEAR
	// Write parameters in helper class
	// attention: the outer symmetrie is 32-fold... this is not
	// taken into account in gear
	
	// The outer radius is in Geant4 defined as the tangent outer radius.
	// same is taken in Gear to be consistent with Ecal and Barrel
	helpEndcap.outerRadius = endcap_rmax; // Hcal_outer_radius; //Hcal_endcap_rmax;  // outer radius
	helpEndcap.innerRadius = Hcal_endcap_center_box_size/2.;  // inner radius
	helpEndcap.phi0 = 0.; // phi0
	
	// Inner_z minimum 
	helpEndcap.leastZ = Hcal_start_z;
	
	// Add first position of layer as ground level
	helpEndcap.layerPos.push_back( - Hcal_endcap_total_z/2. ) ;

	G4double endcap_layer_shift_z;

	for (G4int layer_id = 1; layer_id <= Hcal_endcap_nlayers; layer_id++) {
	  /* count the layers*/
	  helpEndcap.count += 1;
	
	  /* position of layer as offset along z*/
	  endcap_layer_shift_z = - Hcal_endcap_total_z/2
	    + layer_id *(Hcal_chamber_thickness + Hcal_endcap_radiator_thickness);
	
	  helpEndcap.layerPos.push_back(endcap_layer_shift_z) ;
 	
	  helpEndcap.sensThickness.push_back( Hcal_scintillator_thickness );
	  helpEndcap.steelCassetteThickness.push_back( Hcal_steel_cassette_thickness );
	  helpEndcap.PCBThickness.push_back( Hcal_PCB_thickness );
	  helpEndcap.CuThickness.push_back( Hcal_Cu_thickness );

	}	
#endif      
	
	
	for (G4int endCapID = 1; endCapID <= Hcal_endcap_module_number; endCapID++)
	  {
	    
	    //Create the endcap logical volume.
	    G4Box *EndcapModule = new G4Box("EndcapModule",
					    box_half_x,
					    box_half_y[endCapID-1],
					    box_half_z);
	    
	    G4LogicalVolume *EndcapLogical = new G4LogicalVolume(EndcapModule, EndcapRadiatorMaterial, "EndcapLogical", 0, 0, 0);
	    
	    if(Hcal_sensitive_model == "scintillator")
	      {
		G4double layer_offset_x = -box_half_x + Hcal_endcap_lateral_structure_thickness + Hcal_endcap_layer_air_gap;
		G4double layer_offset_y = 0.0;
		G4double layer_offset_z = 0.0;
		
		// build the layers into the endcap, for each endCapID.
		for (G4int layer_id = 1; layer_id <= Hcal_endcap_nlayers; layer_id++) 
		  {
		    EndcapChambersAhcal(EndcapLogical,theENDCAPEndSD, layer_id,pULimits); 
		    
		    if (endCapID==1) theENDCAPEndSD->AddLayer((layer_id), layer_offset_x, layer_offset_y, layer_offset_z);   
		  }
		
	      }
	    else Control::Abort("SHcalSc03: Invalid sensitive HCAL model !",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
	    
	    EndcapLogical->SetVisAttributes(VisAtt);
	    
	    //======================================
	    // place EndcapLogical
	    //
	    // loop for the module 0 and 6
	    //======================================
	    
	    for (int theModule=0; theModule<=1; theModule++) 
	      { // end cap Module 0 and 6, +/-Z
		
		
		shift_x = theXOffset + Hcal_endcap_module_width/2. + (endCapID-1)* (Hcal_endcap_module_width + eps);
		if (theModule == 1) shift_x = -shift_x;
		
		if (endCapID == 8 || endCapID == 9 ) //center with hole.
		  {
		    //G4cout <<"Hcal_endcap_center_box_size (362*2): "<<Hcal_endcap_center_box_size <<G4endl; 
		    //Mokka database is 700mm, half is (350mm)
		    //The endcap center box size was calculated  in y-axis : (362.0mm)
		    // It is the endcap_module_width in x-axis : (375.0mm) 
		    
		    shift_y[endCapID-1] = box_half_y[endCapID-1] + (box_half_y[6] - box_half_y[endCapID-1])*2. + eps; 
		  }
		else {
		  shift_y[endCapID-1] = box_half_y[endCapID-1] + eps;
		}
		
		
		//----- place EndcapLogical -----
		for (int theStave=1; theStave<=2; theStave++) 
		  {  
		    
		    //---- place into up half and down half stave 1 & 2 -----
		    G4RotationMatrix *rotEffect = new G4RotationMatrix();
		    G4ThreeVector module_shift;
		    
		    if(theStave == 1){
		      rotEffect->rotateZ(0.0);
		      module_shift.setX( shift_x);
		      module_shift.setY( shift_y[endCapID-1] );
		    }
		    else if(theStave == 2) {
		      rotEffect->rotateZ(pi);
		      module_shift.setX( -shift_x );
		      module_shift.setY( - shift_y[endCapID-1] );
		    }
		    
		    if(theModule == 0 ){
		      rotEffect->rotateY(0.0);
		      module_shift.setZ( shift_z );
		    }
		    else if (theModule == 1 ){
		      rotEffect->rotateY(pi);
		      module_shift.setZ( - shift_z );
		    }
		    
		    if(theStave==1 && theModule==0) 
		      theENDCAPEndSD->SetModuleYOffset(endCapID-1,-box_half_y[endCapID-1]);

		    G4int copyNb = theStave*1000 + (endCapID-1)*10 + HCALENDCAPMINUS;
		    
		    new MyPlacement(rotEffect, //<========= rotaion here ==========
				    module_shift,
				    EndcapLogical,
				    "EndcapsAhcalPhys",
				    MotherLog,
				    false,
				    copyNb); 
		    
		  }
			
			
		//----- place front end electronic -----	
		G4RotationMatrix *FEErotEffect = new G4RotationMatrix();
		G4ThreeVector FEE_module_shift;
		
		if (theModule == 0){
		  FEErotEffect->rotateY(0.0);
		  FEE_module_shift.setZ( shift_z );
		}
		else if (theModule == 1){
		  FEErotEffect->rotateY(pi);
		  FEE_module_shift.setZ( - shift_z );
		}
		
		G4double FEE_shift_y  = shift_y[endCapID-1] + box_half_y[endCapID-1] + Hcal_endcap_services_module_width/2. + eps;
		
		if (endCapID >=2 && endCapID <=15)
		  {
		    
		    for (int theStave=1; theStave<=2; theStave++) {
		      
		      
		      if(theStave == 1){
			FEE_module_shift.setX( shift_x);
			FEE_module_shift.setY( FEE_shift_y );
		      }
		      else if(theStave == 2) {
			FEE_module_shift.setX( - shift_x );
			FEE_module_shift.setY( - FEE_shift_y );
		      } 
		      
		      new MyPlacement(FEErotEffect,
				      FEE_module_shift,
				      FEELogical,
				      "FEEPhys",
				      MotherLog,
				      false,
				      0);
		    }
		    
		  }
		else // front end electronic is at top side only. (+y)
		  {
		    FEE_module_shift.setX( shift_x);
		    FEE_module_shift.setY( FEE_shift_y );
		    
		    new MyPlacement(FEErotEffect,
				    FEE_module_shift,
				    FEELogical,
				    "FEEPhys",
				    MotherLog,
				    false,
				    0);
		  }
		
		
	      }// end loop :  end cap Module 0 and 6, +/-Z
	    
	  }//---------- end loop over endcap id -----------------
	
	
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~        Chambers in the Endcaps                    ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalSc03::EndcapChambersAhcal(G4LogicalVolume* MotherLog, 
				    SDAHcalEndCap* theENDCAPEndSDi, 
				    G4int layer_id,
				    G4UserLimits* pULimits)
{
	
	/*------------------------------------------------------------------------------------------
	 General variables      
	 -----------------------------------------------------------------------------------------*/
	G4Box *Endcap = (G4Box*)MotherLog->GetSolid();
	
	G4double box_half_x       = Endcap->GetXHalfLength();
	G4double box_half_y       = Endcap->GetYHalfLength();
	//G4double box_half_z       = Endcap->GetZHalfLength();
	
	
	G4double new_box_half_x = 0;
	G4double new_box_half_y = 0;
	G4double new_box_half_z = 0;
	
	/*------------------------------------------------------------------------------------------
	 Build the layer chamber
	 -----------------------------------------------------------------------------------------*/
	
	new_box_half_x = box_half_x - Hcal_endcap_lateral_structure_thickness; //Air gap put into this EndcapChamberLogical
	new_box_half_y = box_half_y;
	new_box_half_z = Hcal_chamber_thickness/2.;
	
	G4Box *EndcapChambers = new G4Box("EndcapChambers",
					  new_box_half_x,  new_box_half_y, new_box_half_z);
	
	G4LogicalVolume *EndcapChamberLogical = new G4LogicalVolume(EndcapChambers, 
								    Air,
								    "EndcapChamberLogical",
								    0, 0, 0);
	G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::Grey());
	VisAtt->SetForceSolid(true);
	EndcapChamberLogical->SetVisAttributes(VisAtt);
	
	new_box_half_x = box_half_x - Hcal_endcap_lateral_structure_thickness - Hcal_endcap_layer_air_gap; //Leave an air gap in layer chamber
	new_box_half_y = box_half_y;
	/*------------------------------------------------------------------------------------------
	 Build the scintillators (polystyrene)      
	 -----------------------------------------------------------------------------------------*/
	
	new_box_half_z = Hcal_scintillator_thickness/2.;
	G4Box *EndcapScintillator = new G4Box("EndcapScintillator",
					      new_box_half_x, new_box_half_y, new_box_half_z);
	
	G4LogicalVolume *EndcapScintillatorLogical = new G4LogicalVolume(EndcapScintillator,
									 CGAGeometryManager::GetMaterial("polystyrene"),
									 "EndcapScintillatorLogical",
									 0, 0, pULimits);
	VisAtt = new G4VisAttributes(G4Colour::Yellow());
	VisAtt->SetForceWireframe(true);
	VisAtt->SetForceSolid(true);
	VisAtt->SetDaughtersInvisible(true);
	EndcapScintillatorLogical->SetVisAttributes(VisAtt);
	
	G4double ScintillatorPosZ = - (Hcal_chamber_thickness - Hcal_scintillator_thickness)/2. + eps;
	
	new MyPlacement(0,		   //no rotation
			G4ThreeVector(0.,0.,ScintillatorPosZ),  //its position
			EndcapScintillatorLogical,     //its logical volume		    
			"EndCapScintillator", //its name
			EndcapChamberLogical,        //its mother
			false,             //no boulean operat
			layer_id, //Layer number for SD
			SHCAL_CHECK_OVERLAP);
	
	
	//set the sensitive detector
	//(only scintillator is sensitive) 
	
	EndcapScintillatorLogical->SetSensitiveDetector( theENDCAPEndSDi );
	
	
	/*------------------------------------------------------------------------------------------
	 Build the PCB
	 -----------------------------------------------------------------------------------------*/
	
	new_box_half_z = Hcal_PCB_thickness/2.;
	G4Box *EndcapPCB = new G4Box("EndcapPCB", new_box_half_x,  new_box_half_y, new_box_half_z);
	
	G4LogicalVolume *EndcapPCBLogical = new G4LogicalVolume(EndcapPCB, PCB, "EndcapPCBLogical", 0, 0, 0);
	VisAtt = new G4VisAttributes(G4Colour::Green());
	VisAtt->SetForceSolid(true);
	EndcapPCBLogical->SetVisAttributes(VisAtt);
	
	G4double PCBPosZ = ScintillatorPosZ + Hcal_scintillator_thickness/2. + Hcal_PCB_thickness/2. + eps;
	
	new MyPlacement(0,		   //no rotation
			  G4ThreeVector(0.,0.,PCBPosZ),  //its position
			  EndcapPCBLogical,     //its logical volume		    
			  "EndCapPCB", //its name
			  EndcapChamberLogical,        //its mother
			  false,             //no boulean operat
			  0,                //copy number
			  SHCAL_CHECK_OVERLAP);
	
	/*------------------------------------------------------------------------------------------
	 Build the Cu
	 -----------------------------------------------------------------------------------------*/
	
	new_box_half_z = Hcal_Cu_thickness/2.;
	G4Box *EndcapCu = new G4Box("EndcapCu", new_box_half_x,  new_box_half_y, new_box_half_z);
	
	G4LogicalVolume *EndcapCuLogical = new G4LogicalVolume(EndcapCu, Cu, "EndcapCuLogical", 0, 0, 0);
	VisAtt = new G4VisAttributes(G4Colour::Red());
	VisAtt->SetForceSolid(true);
	EndcapCuLogical->SetVisAttributes(VisAtt);
	
	G4double CuPosZ = PCBPosZ + Hcal_PCB_thickness/2. + Hcal_Cu_thickness/2. + eps;
	
	new MyPlacement(0,		   //no rotation
			G4ThreeVector(0.,0.,CuPosZ),  //its position
			EndcapCuLogical,     //its logical volume		    
			"EndCapCu", //its name
			EndcapChamberLogical,        //its mother
			false,             //no boulean operat
			0,                //copy number
			SHCAL_CHECK_OVERLAP);
	 
	/*--------------------------------------------------------------------------------
	 build the air box for Hcal_fiber_gap which used for electronic cables and pins
	 --------------------------------------------------------------------------------*/
	// This air gap is not neccessery to build twice.
	/*
	new_box_half_z = Hcal_fiber_gap/2.;
	G4Box *EndcapAirGapBox = new G4Box("EndcapAirGapBox", new_box_half_x,  new_box_half_y, new_box_half_z);
	
	G4LogicalVolume *EndcapAirGapLog = new G4LogicalVolume(EndcapAirGapBox, Air, "EndcapAirGapLog", 0, 0, 0);
	VisAtt = new G4VisAttributes(G4Colour::White());
	VisAtt->SetForceSolid(true);
	EndcapAirGapLog->SetVisAttributes(VisAtt);
	
	G4double AirGapPosZ =  CuPosZ + Hcal_Cu_thickness/2. + Hcal_fiber_gap/2.;
	
	new MyPlacement(0,		   //no rotation
	                G4ThreeVector(0.,0.,AirGapPosZ),  //its position
			EndcapAirGapLog,     //its logical volume		    
			"Air", //its name
			EndcapChamberLogical,        //its mother
			false,             //no boulean operat
			0,                //copy number
	                SHCAL_CHECK_OVERLAP); // donot check overlapping
	*/
	/*------------------------------------------------------------------------------------------
	 
	 Place the Layer Chamber into the HCAL module
	 
	 -----------------------------------------------------------------------------------------*/
	
	G4double shift_z  = 0;
	
	shift_z = - Hcal_endcap_total_z/2 + Hcal_radiator_thickness + Hcal_chamber_thickness/2. + (layer_id-1)*layerThickness;
	
	new MyPlacement(NULL, G4ThreeVector(0, 0, shift_z),
			EndcapChamberLogical, "EndcapChamberPhys",
			MotherLog, false, layer_id, SHCAL_CHECK_OVERLAP);
	
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              Endcap front end electronics         ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalSc03::EndcapsAhcalFrontEndElectronics(G4LogicalVolume* MotherLog, G4int layer_id)
{
	/*------------------------------------------------------------------------------------------
	 General variables      
	 -----------------------------------------------------------------------------------------*/
	G4Box *Endcap = (G4Box*)MotherLog->GetSolid();
	
	G4double box_half_x       = Endcap->GetXHalfLength();
	G4double box_half_y       = Endcap->GetYHalfLength();
	//G4double box_half_z       = Endcap->GetZHalfLength();
	
	
	G4double new_box_half_x = 0;
	G4double new_box_half_y = 0;
	G4double new_box_half_z = 0;
	
	//G4double Hcal_endcap_lateral_structure_thickness = 5.0; //5mm side walls each in redesign endcap
	//G4double Hcal_endcap_layer_air_gap = 2.5; //2.5mm air gap between side walls and chamber in redesign endcap
	//G4double Hcal_steel_cassette_thickness = 0.5;
	//G4double HcalServices_outer_FR4_thickness = 2.8;
	//G4double HcalServices_outer_Cu_thickness = 0.4;
	/*------------------------------------------------------------------------------------------
	 Build the layer chamber
	 -----------------------------------------------------------------------------------------*/
	
	new_box_half_x = box_half_x - Hcal_endcap_lateral_structure_thickness; //Air gap put into this EndcapFEEChamberLogical
	new_box_half_y = box_half_y;
	new_box_half_z = Hcal_chamber_thickness/2.;
	
	G4Box *EndcapFEEChambers = new G4Box("EndcapFEEChambers",
					     new_box_half_x,  new_box_half_y, new_box_half_z);
	
	G4LogicalVolume *EndcapFEEChamberLogical = new G4LogicalVolume(EndcapFEEChambers, 
								       CGAGeometryManager::GetMaterial("air"),
								       "EndcapFEEChamberLogical",
								       0, 0, 0);
	G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::Grey());
	VisAtt->SetForceSolid(true);
	EndcapFEEChamberLogical->SetVisAttributes(VisAtt);
	
	
	/*------------------------------------------------------------------------------------------
	 Build the 0.5 mm steel cassette      
	 -----------------------------------------------------------------------------------------*/
	new_box_half_x = box_half_x - Hcal_endcap_lateral_structure_thickness - Hcal_endcap_layer_air_gap; //Leave an air gap in layer chamber
	new_box_half_z = Hcal_steel_cassette_thickness/2.; //0.5mm
	G4Box *EndcapFEEcassette = new G4Box("EndcapFEEcassette",
					     new_box_half_x, new_box_half_y, new_box_half_z);
	
	G4LogicalVolume *EndcapFEEcassetteLogical = new G4LogicalVolume(EndcapFEEcassette,
									CGAGeometryManager::GetMaterial("stainless_steel"),
									"EndcapFEEcassetteLogical",
									0, 0, 0);
	VisAtt = new G4VisAttributes(G4Colour::Blue());
	VisAtt->SetForceWireframe(true);
	VisAtt->SetForceSolid(true);
	VisAtt->SetDaughtersInvisible(true);
	EndcapFEEcassetteLogical->SetVisAttributes(VisAtt);
	
	G4double FEEcassettePosZ = - (Hcal_chamber_thickness - Hcal_steel_cassette_thickness)/2. + eps;
	
	new MyPlacement(0,		   //no rotation
			G4ThreeVector(0.,0.,FEEcassettePosZ),  //its position
			EndcapFEEcassetteLogical,     //its logical volume		    
			"EndcapFEEcassette", //its name
			EndcapFEEChamberLogical,        //its mother
			false,             //no boulean operat
			0, //copy number
			SHCAL_CHECK_OVERLAP);
	
	/*------------------------------------------------------------------------------------------
	 Build the PCB
	 -----------------------------------------------------------------------------------------*/
	new_box_half_z = HcalServices_outer_FR4_thickness/2.;// 2.8 mm

	G4Box *EndcapFEEPCB = new G4Box("EndcapFEEPCB", new_box_half_x,  new_box_half_y, new_box_half_z);
	
	G4LogicalVolume *EndcapFEEPCBLogical = new G4LogicalVolume(EndcapFEEPCB, CGAGeometryManager::GetMaterial("PCB"), "EndcapFEEPCBLogical", 0, 0, 0);
	VisAtt = new G4VisAttributes(G4Colour::Green());
	VisAtt->SetForceSolid(true);
	EndcapFEEPCBLogical->SetVisAttributes(VisAtt);
	
	G4double FEEPCBPosZ = FEEcassettePosZ + Hcal_steel_cassette_thickness/2. + HcalServices_outer_FR4_thickness/2. + eps;
	
	new MyPlacement(0,		   //no rotation
			  G4ThreeVector(0.,0.,FEEPCBPosZ),  //its position
			  EndcapFEEPCBLogical,     //its logical volume		    
			  "EndCapPCB", //its name
			  EndcapFEEChamberLogical,        //its mother
			  false,             //no boulean operat
			  0,                //copy number
	          SHCAL_CHECK_OVERLAP);
	
	/*------------------------------------------------------------------------------------------
	 Build the Cu
	 -----------------------------------------------------------------------------------------*/
	new_box_half_z = HcalServices_outer_Cu_thickness/2.; //0.4 mm
	G4Box *EndcapFEECu = new G4Box("EndcapFEECu", new_box_half_x,  new_box_half_y, new_box_half_z);
	
	G4LogicalVolume *EndcapFEECuLogical = new G4LogicalVolume(EndcapFEECu, CGAGeometryManager::GetMaterial("G4_Cu"), "EndcapFEECuLogical", 0, 0, 0);
	VisAtt = new G4VisAttributes(G4Colour::Red());
	VisAtt->SetForceSolid(true);
	EndcapFEECuLogical->SetVisAttributes(VisAtt);
	
	G4double FEECuPosZ = FEEPCBPosZ + HcalServices_outer_FR4_thickness/2. + HcalServices_outer_Cu_thickness/2. + eps;
	
	new MyPlacement(0,		   //no rotation
			G4ThreeVector(0.,0.,FEECuPosZ),  //its position
			EndcapFEECuLogical,     //its logical volume		    
			"EndCapCu", //its name
			EndcapFEEChamberLogical,        //its mother
			false,             //no boulean operat
			0,                //copy number
			SHCAL_CHECK_OVERLAP);
	
	/*------------------------------------------------------------------------------------------
	 
	 Place the Layer FEE Chamber into the HCAL FEE module
	 
	 -----------------------------------------------------------------------------------------*/
	
	G4double shift_z  = 0;
	
	shift_z = - Hcal_endcap_total_z/2 + Hcal_radiator_thickness + Hcal_chamber_thickness/2. + (layer_id-1)*layerThickness;
	
	new MyPlacement(NULL, G4ThreeVector(0, 0, shift_z),
			EndcapFEEChamberLogical, "EndcapFEEChamberPhys",
			MotherLog, false, 1, SHCAL_CHECK_OVERLAP);
	
	
	
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~       Load event                                 ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalSc03::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
  // In this complex subdetector the best is to read
  // all hits in the one of the sensitive detectors,
  // just for visualisation, so just for your eyes...
  theBarrilRegSD->LoadEvent(theSubDetectorEventHitsFileInput);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~       Setup                                      ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
G4bool SHcalSc03::Setup(const CGAGeometryEnvironment &theGeometryEnvironment)
{
  Hcal_barrel_end_module_type = theGeometryEnvironment.GetParameterAsInt("Hcal_barrel_end_module_type");
  if( Hcal_barrel_end_module_type != 1)
    Control::Abort("SHcalSc03: Sorry, but TESLA like end modules in barrel are not available with this driver.",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);

  Hcal_layer_air_gap              = theGeometryEnvironment.GetParameterAsDouble("Hcal_layer_air_gap");
  Hcal_apply_Birks_law            = theGeometryEnvironment.GetParameterAsInt("Hcal_apply_Birks_law");
  Hcal_radiator_thickness         = theGeometryEnvironment.GetParameterAsDouble("Hcal_radiator_thickness");
  Hcal_radiator_material          = theGeometryEnvironment.GetParameterAsString("Hcal_radiator_material");
  Hcal_ring                       = theGeometryEnvironment.GetParameterAsInt("Hcal_ring");
  Hcal_radial_ring_inner_gap      = theGeometryEnvironment.GetParameterAsInt("Hcal_radial_ring_inner_gap");
  Hcal_sensitive_model            = theGeometryEnvironment.GetParameterAsString("Hcal_sensitive_model");
  Hcal_back_plate_thickness       = theGeometryEnvironment.GetParameterAsDouble("Hcal_back_plate_thickness");
  Hcal_nlayers                    = theGeometryEnvironment.GetParameterAsInt("Hcal_nlayers");
  Ecal_endcap_zmin                = theGeometryEnvironment.GetParameterAsDouble("Ecal_endcap_zmin");
  Ecal_endcap_outer_radius        = theGeometryEnvironment.GetParameterAsDouble("Ecal_endcap_outer_radius");
  Hcal_endcap_radiator_thickness  = theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_radiator_thickness");
  Hcal_endcap_radiator_material   = theGeometryEnvironment.GetParameterAsString("Hcal_endcap_radiator_material");
  Hcal_endcap_nlayers             = theGeometryEnvironment.GetParameterAsInt   ("Hcal_endcap_nlayers");

  Hcal_endcap_module_width                 = theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_module_width");
  Hcal_endcap_module_number                = theGeometryEnvironment.GetParameterAsInt("Hcal_endcap_module_number");
  Hcal_endcap_lateral_structure_thickness  = theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_lateral_structure_thickness");
  Hcal_endcap_layer_air_gap                = theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_layer_air_gap");
  Hcal_endcap_services_module_width         = theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_services_module_width");
  HcalServices_outer_FR4_thickness         = theGeometryEnvironment.GetParameterAsDouble("HcalServices_outer_FR4_thickness");
  HcalServices_outer_Cu_thickness          = theGeometryEnvironment.GetParameterAsDouble("HcalServices_outer_Cu_thickness");

  Hcal_scintillator_thickness     = theGeometryEnvironment.GetParameterAsDouble("Hcal_scintillator_thickness");
  Hcal_steel_cassette_thickness   = theGeometryEnvironment.GetParameterAsDouble("Hcal_steel_cassette_thickness");
  Hcal_Cu_thickness               = theGeometryEnvironment.GetParameterAsDouble("Hcal_Cu_thickness");
  Hcal_PCB_thickness              = theGeometryEnvironment.GetParameterAsDouble("Hcal_PCB_thickness");


  //Hcal_steel_cassette_thickness = 0.5;  //not used by barrel any more, it has been built into the absorber 20mm = 19mm + 2*0.5mm
  //Hcal_Cu_thickness             = 0.1;
  //Hcal_PCB_thickness            = 0.7;
  //Hcal_scintillator_thickness   = 3;

  // the Hcal_lateral_plate_thickness will ber used only in the middle of the barrel,
  // there is not steel  between service board and chamber.
  Hcal_lateral_plate_thickness    = theGeometryEnvironment.GetParameterAsDouble("Hcal_lateral_structure_thickness");
  Hcal_middle_stave_gaps          = theGeometryEnvironment.GetParameterAsDouble("Hcal_middle_stave_gaps");
  Hcal_stave_gaps                 = theGeometryEnvironment.GetParameterAsDouble("Hcal_stave_gaps");
  Hcal_modules_gap                = theGeometryEnvironment.GetParameterAsDouble("Hcal_modules_gap");
  Hcal_fiber_gap                  = theGeometryEnvironment.GetParameterAsDouble("Hcal_fiber_gap");

  Hcal_chamber_thickness  = Hcal_scintillator_thickness + Hcal_PCB_thickness + Hcal_Cu_thickness + Hcal_fiber_gap;
  //Hcal_chamber_thickness  = theGeometryEnvironment.GetParameterAsDouble("Hcal_chamber_thickness");
	
  layerThickness = Hcal_radiator_thickness +Hcal_chamber_thickness;
	
  Hcal_inner_radius               = theGeometryEnvironment.GetParameterAsDouble("Ecal_outer_radius")
                                    + theGeometryEnvironment.GetParameterAsDouble("Hcal_Ecal_gap");
  Hcal_endcap_cables_gap          = theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_cables_gap");
  Hcal_endcap_ecal_gap            = theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_ecal_gap");

  TPC_Ecal_Hcal_barrel_halfZ      = theGeometryEnvironment.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");

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

  Hcal_cells_size                  = theGeometryEnvironment.GetParameterAsDouble("Hcal_cells_size");
  Hcal_digi_cells_size             = theGeometryEnvironment.GetParameterAsDouble("Hcal_digitization_tile_size");
  Hcal_endcap_center_box_size      = theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_center_box_size");
  Hcal_endcap_sensitive_center_box = theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_sensitive_center_box");

  //=======================================================
  //                                                     //
  // general calculated parameters                       //
  //                                                     //
  //=======================================================  
  // There is no Hcal_back_palte in the updated design.
  Hcal_total_dim_y   = Hcal_nlayers * (Hcal_radiator_thickness + Hcal_chamber_thickness) 
                       + Hcal_radiator_thickness - 1.0; //Engineering absorber is 19.0 mm
  Hcal_endcap_total_z = Hcal_endcap_nlayers * (Hcal_endcap_radiator_thickness + Hcal_chamber_thickness) + Hcal_back_plate_thickness;

  
  Hcal_module_radius = ( Hcal_inner_radius + 
			 ( Hcal_radiator_thickness + Hcal_chamber_thickness ) * 40. // bottom: layer number 40, top: layer number 8.
			 + Hcal_radiator_thickness -1.0) / cos(pi/8.); //Engineering absorber is 19.0 mm

  Hcal_y_dim1_for_x  = Hcal_module_radius*cos(pi/8.) - Hcal_inner_radius;
  Hcal_y_dim2_for_x  = Hcal_total_dim_y - Hcal_y_dim1_for_x;
  Hcal_bottom_dim_x  = 2.*Hcal_inner_radius*tan(pi/8.)- Hcal_stave_gaps;
  Hcal_midle_dim_x   = Hcal_bottom_dim_x + 2* Hcal_y_dim1_for_x*tan(pi/8.);
  Hcal_top_dim_x     = sqrt(Hcal_module_radius * Hcal_module_radius
			    - (Hcal_inner_radius + Hcal_total_dim_y)
			    * (Hcal_inner_radius + Hcal_total_dim_y) );
  Hcal_endcap_rmax   = Hcal_inner_radius + Hcal_y_dim1_for_x;

  //only the middle has the steel plate.
  Hcal_regular_chamber_dim_z = Hcal_normal_dim_z - Hcal_lateral_plate_thickness;
  Hcal_cell_dim_x            = Hcal_cells_size;
  Hcal_cell_dim_z            = Hcal_regular_chamber_dim_z / floor (Hcal_regular_chamber_dim_z/Hcal_cell_dim_x);
  Hcal_digi_cell_dim_x       = Hcal_digi_cells_size;
  Hcal_digi_cell_dim_z       = Hcal_regular_chamber_dim_z / floor (Hcal_regular_chamber_dim_z/Hcal_digi_cell_dim_x);

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
  mokkaGearMgr->tmpParam.setDoubleVal("Hcal_layer_air_gap" , 
				      theGeometryEnvironment.GetParameterAsDouble( "Hcal_layer_air_gap" ) ) ;
  mokkaGearMgr->tmpParam.setDoubleVal("Hcal_middle_stave_gaps" , 
				      theGeometryEnvironment.GetParameterAsDouble( "Hcal_middle_stave_gaps" ) ) ;
  mokkaGearMgr->tmpParam.setDoubleVal("Hcal_modules_gap" , 
				      theGeometryEnvironment.GetParameterAsDouble( "Hcal_modules_gap" ) ) ;
  mokkaGearMgr->tmpParam.setDoubleVal("Hcal_stave_gaps" , 
				      theGeometryEnvironment.GetParameterAsDouble( "Hcal_stave_gaps" ) ) ;
  mokkaGearMgr->tmpParam.setDoubleVal("Hcal_steel_cassette_thickness" , 
				      theGeometryEnvironment.GetParameterAsDouble( "Hcal_steel_cassette_thickness" ) ) ;
  mokkaGearMgr->tmpParam.setDoubleVal("Hcal_scintillator_thickness" , 
				      theGeometryEnvironment.GetParameterAsDouble( "Hcal_scintillator_thickness" ) ) ;
  mokkaGearMgr->tmpParam.setDoubleVal("Hcal_Cu_thickness" , 
				      theGeometryEnvironment.GetParameterAsDouble( "Hcal_Cu_thickness" ) ) ;
  mokkaGearMgr->tmpParam.setDoubleVal("Hcal_PCB_thickness" , 
				      theGeometryEnvironment.GetParameterAsDouble( "Hcal_PCB_thickness" ) ) ;
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
void SHcalSc03::EndcapRings(G4LogicalVolume* MotherLog)
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

#ifdef SHCALSC03_DEBUG
  G4cout<<"    EndcapRings         : Rinner="<<pRMin<<"mm Router="<<pRMax<<G4endl;
#endif

  if (rOuter[0] <= rInner[0]) 
    G4Exception("SHcalSc03::EndcapRings() - not enough place for endcap rings (try a larger Hcal_nlayers number)!");

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

  G4Polyhedra *HcalEndCapRingSolid = new G4Polyhedra("HcalEndCapRingSolid",
					     pi/8.,
					     2.*pi,
					     8,
					     2,
					     zPlane,
					     rInner,
					     rOuter);

  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(.8,.8,.2));
  VisAtt->SetForceWireframe(true);
  VisAtt->SetDaughtersInvisible(true);
#ifdef SHCALSC03_DEBUG
  VisAtt->SetColour(G4Colour(.2,.8,.2));
  VisAtt->SetDaughtersInvisible(false); 
#endif

  G4LogicalVolume* HcalEndCapRingLogical = new G4LogicalVolume(HcalEndCapRingSolid,
						       EndcapRadiatorMaterial,
						       "HcalEndCapRingLogical",
						       0, 0, 0);
  HcalEndCapRingLogical->SetVisAttributes(VisAtt);

#ifdef MOKKA_GEAR
  // add first position of layer as ground level
  helpEndcapRing.layerPos.push_back( zPlane[0] ) ;
#endif

  //------------------------------------------------------
  // build and place the chambers in the Hcal EndcapRings
  EndcapRingChambers(HcalEndCapRingLogical,theENDCAPRingSD, true);
  //------------------------------------------------------

  // Placements
  G4double endcap_z_offset = Ecal_endcap_zmin + pDz;
  G4RotationMatrix *rotEffect = new G4RotationMatrix();
  rotEffect->rotateZ(0.);

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
		      HcalEndCapRingLogical,
		      "HcalEndCapRingPhys",
		      MotherLog,
		      false,
		      ModuleNumber);
      rotEffect = new G4RotationMatrix();
      rotEffect->rotateZ(0.);
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
G4bool SHcalSc03::PostConstructAction(CGAGeometryEnvironment& )
{
  //
  // Propagates the changes to Coil, if any. The SHcal has also the responsability 
  // to change the calorimeter region parameters.
  //
  G4double Hcal_R_max = Hcal_module_radius;
  std::ostringstream oss1;
  oss1 << Hcal_R_max;
  (*Control::globalModelParameters)["Hcal_R_max"] = oss1.str();
  (*Control::globalModelParameters)["calorimeter_region_rmax"] = oss1.str();

  std::ostringstream oss2;
  oss2 << Hcal_start_z;
  (*Control::globalModelParameters)["Hcal_endcap_zmin"] = oss2.str();
  
  G4double Hcal_outer_radius = Hcal_module_radius;

  G4double calorimeter_region_zmax = Hcal_start_z + Hcal_endcap_total_z;
  std::ostringstream oss3;  
  oss3 << calorimeter_region_zmax;
  (*Control::globalModelParameters)["calorimeter_region_zmax"] = oss3.str();
			    
  G4cout << "SHcalSc03 information: Hcal_outer_radius = "
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
void SHcalSc03::CalculateFractTileSize(G4double x_length, 
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
#ifdef SHCALSC03_DEBUG
  G4cout<<"  xLayer="<<x_length<<"   x_fractionalTileSize="<<x_fractionalTileSize<<G4endl;
#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~     Build gap between HCAL barrel modules         ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Updated: The Airgap is not needed, and removed from both header and code.
/*void SHcalSc03::BarrelModuleGap(G4LogicalVolume *MotherLog) 
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
  VisAttb->SetDaughtersInvisible(false); 
  gapLog->SetVisAttributes(VisAttb);
 
}
*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~     Build HCAL barrel chambers                    ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalSc03::BarrelHalfRegularChambersTrap(G4LogicalVolume *MotherLog, 
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

  /*---------------------------------- start loop over HCAL layers ----------------------------------------*/
  G4int logical_layer_id = 0;

  for (G4int layer_id = 1; layer_id <= (2*Hcal_nlayers); layer_id++)
    {
      /*
	calculate x-length of an HCAL barrel layer
	-> this sets logical_layer_id, xOffset, x_length, xShift
       */
      this->CalculateXLayer(layer_id, logical_layer_id, xOffset, x_length, xShift);

      /*
	calculate the size of a fractional tile
	-> this sets fract_cell_dim_x
      */
      G4double fract_cell_dim_x = 0.;
      this->CalculateFractTileSize(2*x_length, Hcal_cell_dim_x, fract_cell_dim_x);
      
      G4ThreeVector newFractCellDim(fract_cell_dim_x, Hcal_chamber_thickness, Hcal_cell_dim_z);
      theBarrilRegSD->SetFractCellDimPerLayer(layer_id, newFractCellDim);
       
      /*--------------------------------------------------------------------------------
	build chamber box, with the calculated dimensions 
	-------------------------------------------------------------------------------*/
      ChamberSolid = new G4Box("ChamberSolid", 
			       x_length,  //x
			       z_width,   //z attention!
			       y_height); //y attention!

      G4LogicalVolume *ChamberLogical = new G4LogicalVolume(ChamberSolid,
							    CGAGeometryManager::GetMaterial("air"), 
							    "ChamberLogical", 
							    0, 0, 0);   
      
      ChamberLog[layer_id] = ChamberLogical ;

      G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::White());
      //VisAtt->SetForceWireframe(true);
      //VisAtt->SetDaughtersInvisible(true);
      VisAtt->SetForceSolid(false);
      ChamberLog[layer_id]->SetVisAttributes(VisAtt);
 
      /*-------------------------------------------------------------------------------
	build the scintillator box 
	------------------------------------------------------------------------------*/
      G4double scintHalfWidth = Hcal_scintillator_thickness/2.;      
      
      G4Box *ScintSolid = new G4Box("ScintSolid", 
				    x_length, //x
				    z_width,          //z attention!
				    scintHalfWidth ); //y attention!
      
      G4LogicalVolume* ScintLog = new G4LogicalVolume(ScintSolid,
						      CGAGeometryManager::GetMaterial("polystyrene"),
						      "ScintLogical", 
						      0, 0, pULimits); 
      VisAtt = new G4VisAttributes(G4Colour::Yellow());
      VisAtt->SetForceSolid(false);
      ScintLog->SetVisAttributes(VisAtt);
      
      /* only scintillator is sensitive*/
      ScintLog->SetSensitiveDetector(theBarrilRegSD);
      G4double yScintillator = -(Hcal_PCB_thickness + Hcal_Cu_thickness + Hcal_fiber_gap)/2;
      
      new MyPlacement(0, G4ThreeVector(0, 0, yScintillator ), 
		      ScintLog, "Scintillator", ChamberLogical, 
		      false, layer_id, SHCAL_CHECK_OVERLAP);   
      
      /*--------------------------------------------------------------------------------
	build the PCB box 
	-------------------------------------------------------------------------------*/
      G4Box *PCBBox = new G4Box("PCBBox", x_length, z_width, Hcal_PCB_thickness/2);
      G4LogicalVolume *PCBLog = new G4LogicalVolume(PCBBox, PCB, "PCBLog", 0, 0, 0);
      VisAtt = new G4VisAttributes(G4Colour::Grey());
      VisAtt->SetForceSolid(false);
      PCBLog->SetVisAttributes(VisAtt);

      G4double yPCB = yScintillator + Hcal_scintillator_thickness/2 + Hcal_PCB_thickness/2;
      new MyPlacement(0, G4ThreeVector(0, 0, yPCB),
		      PCBLog, "PCB", ChamberLogical, false, layer_id, SHCAL_CHECK_OVERLAP);
   
      /*--------------------------------------------------------------------------------
	 build the Cu box 
	 --------------------------------------------------------------------------------*/
      G4Box *CuBox = new G4Box("CuBox", x_length, z_width, Hcal_Cu_thickness/2);
      G4LogicalVolume *CuLog = new G4LogicalVolume(CuBox, Cu, "CuLog", 0, 0, 0);
      VisAtt = new G4VisAttributes(G4Colour::Cyan());
      VisAtt->SetForceSolid(false);
      CuLog->SetVisAttributes(VisAtt);

      G4double yCu = yPCB + Hcal_PCB_thickness/2 + Hcal_Cu_thickness/2;
      new MyPlacement(0, G4ThreeVector(0, 0, yCu),
		      CuLog, "Cu", ChamberLogical, false, layer_id, SHCAL_CHECK_OVERLAP);
   
      
      /*--------------------------------------------------------------------------------
	 build the air box for Hcal_fiber_gap which used for electronic cables 
	 --------------------------------------------------------------------------------*/
      G4Box *AirGapBox = new G4Box("AirGapBox", x_length, z_width, Hcal_fiber_gap/2);
      G4LogicalVolume *AirGapLog = new G4LogicalVolume(AirGapBox, Air, "AirGapLog", 0, 0, 0);
      VisAtt = new G4VisAttributes(G4Colour::White());
      VisAtt->SetForceSolid(false);
      AirGapLog->SetVisAttributes(VisAtt);

      G4double yAirGap = yCu + Hcal_Cu_thickness/2 + Hcal_fiber_gap/2;
      new MyPlacement(0, G4ThreeVector(0, 0, yAirGap),
		      AirGapLog, "Air", ChamberLogical, false, layer_id, SHCAL_CHECK_OVERLAP);
   
      
      /*(---------------------------  Chamber Placements -----------------------------------------*/
      /* module x and y offsets (needed for the SD)*/
      G4double Xoff,Yoff;
      Xoff = 0.;
      Yoff = Hcal_inner_radius + Hcal_total_dim_y/2.;
      
      G4double chamber_x_offset, chamber_y_offset, chamber_z_offset;
      chamber_x_offset = xShift;

      chamber_z_offset = 0;


      chamber_y_offset = -Hcal_total_dim_y/2. 
	+ (logical_layer_id-1) *(Hcal_chamber_thickness + Hcal_radiator_thickness)
	+ Hcal_radiator_thickness + Hcal_chamber_thickness/2.;
      
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
      if (layer_id <= Hcal_nlayers)
	{
	  // get height of sensible part of layer
	  helpBarrel.fiberGap.push_back( Hcal_fiber_gap );
	  helpBarrel.sensThickness.push_back( Hcal_scintillator_thickness );
	  helpBarrel.PCBThickness.push_back( Hcal_PCB_thickness );
	  helpBarrel.CuThickness.push_back( Hcal_Cu_thickness );
	  
	  // count layers
	  helpBarrel.count += 1;
	  // get position for each layer
	  // check for dynamic_cast
	  G4Box * solidOfLog = dynamic_cast<G4Box*>(ChamberLog[layer_id]->GetSolid());
	  if ( solidOfLog == 0 ) {
	    Control::Abort("SHcalSc03: BarrelChambers are expected to be of type G4Box\nerror in 'HcalScint::BarrelRegularChambers' construction MokkaGear.",MOKKA_OTHER_ERRORS);
	  }
	  helpBarrel.layerPos.push_back(chamber_y_offset + solidOfLog->GetZHalfLength());
	  
	}
#endif
    }//end loop over HCAL nlayers;
  
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~     Build air gaps between HCAL barrel chambers and layer support structures    ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalSc03::BarrelChambersGap(G4LogicalVolume *MotherLog,
				  G4double chambers_y_off_correction)
{
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::Cyan());
  VisAtt->SetForceWireframe(true);
  VisAtt->SetForceSolid(false);


  G4double x_length = 0.;
  G4double y_height = Hcal_chamber_thickness/2.;
  G4double z_width  = (Hcal_normal_dim_z - Hcal_lateral_plate_thickness)/2.;
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
							 CGAGeometryManager::GetMaterial("air"),
							 //CGAGeometryManager::GetMaterial("stainless_steel"),
							 "ChamberGapLog");
    ChamberGapLog->SetVisAttributes(VisAtt);

    G4double chamber_x_offset, chamber_y_offset, chamber_z_offset;
    chamber_x_offset = xShift;
    chamber_z_offset = 0;
    chamber_y_offset = -Hcal_total_dim_y/2. 
      + (logical_layer_id-1) *(Hcal_chamber_thickness + Hcal_radiator_thickness)
      + Hcal_radiator_thickness + Hcal_chamber_thickness/2.;


    //right side, right edge
    G4double xAbsShiftChamberGap_rightEdge = (Hcal_middle_stave_gaps/2.
					      + 2.*x_length + 3/2.*Hcal_layer_air_gap);
    G4double xShiftChamberGap_rightEdge = 0.;
    if (layer_id <= Hcal_nlayers) xShiftChamberGap_rightEdge = - xAbsShiftChamberGap_rightEdge;
    else  xShiftChamberGap_rightEdge = xAbsShiftChamberGap_rightEdge;

    std::stringstream stringForLayerNo; /*string to save layer number*/
    stringForLayerNo << layer_id;

    new MyPlacement(0,
		    G4ThreeVector(xShiftChamberGap_rightEdge, chamber_z_offset,
				  chamber_y_offset + chambers_y_off_correction),
		    ChamberGapLog,
		    G4String("ChamberGapRight") + G4String(stringForLayerNo.str()),
		    MotherLog,
		    false,
		    layer_id);

    //right side, left edge
    G4double xAbsShiftChamberGap_leftEdge = 
      (Hcal_middle_stave_gaps/2. + Hcal_layer_air_gap/2.);

    G4double xShiftChamberGap_leftEdge = 0.;
    if (layer_id <= Hcal_nlayers)  xShiftChamberGap_leftEdge = -xAbsShiftChamberGap_leftEdge;
    else  xShiftChamberGap_leftEdge = xAbsShiftChamberGap_leftEdge;

    new MyPlacement(0,
		    G4ThreeVector(xShiftChamberGap_leftEdge, chamber_z_offset,
				  chamber_y_offset + chambers_y_off_correction),
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
void SHcalSc03::CalculateXLayer(G4int layer_id, G4int &logical_layer_id,
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
  if( logical_layer_id *(Hcal_radiator_thickness + Hcal_chamber_thickness)
      < (Hcal_module_radius * cos(pi/8.) - Hcal_inner_radius ) ) {
    xOffset = (logical_layer_id * Hcal_radiator_thickness 
	       + (logical_layer_id -1) * Hcal_chamber_thickness) * TanPiDiv8;

    x_total  = Hcal_bottom_dim_x/2 - Hcal_middle_stave_gaps/2 + xOffset;
    x_length = x_total - 2*Hcal_layer_air_gap;
    x_halfLength = x_length/2.;

  } else {//----- top barrel -------------------------------------------------
    G4double y_layerID = logical_layer_id * (Hcal_radiator_thickness + Hcal_chamber_thickness) + Hcal_inner_radius;
    G4double ro_layer = Hcal_module_radius - Hcal_radiator_thickness;
      
    x_total = sqrt( ro_layer * ro_layer - y_layerID * y_layerID);
      
    x_length = x_total - Hcal_middle_stave_gaps;

    x_halfLength = x_length/2.;

    xOffset = (logical_layer_id * Hcal_radiator_thickness 
	       + (logical_layer_id - 1) * Hcal_chamber_thickness - Hcal_y_dim1_for_x) / TanPiDiv8
      + Hcal_chamber_thickness / TanPiDiv8;

  }

  G4double xAbsShift = (Hcal_middle_stave_gaps/2 + Hcal_layer_air_gap + x_halfLength);

  if (layer_id <= Hcal_nlayers)     xShift = - xAbsShift;
  else if (layer_id > Hcal_nlayers) xShift = xAbsShift;

  //   G4cout<<"layer_id="<<layer_id<<" logical_layer_id="<<logical_layer_id
  // 	<<" xOffset="<<xOffset<<" x_halfLength="<<x_halfLength<<" xShift="<<xShift<<G4endl;
 
}

