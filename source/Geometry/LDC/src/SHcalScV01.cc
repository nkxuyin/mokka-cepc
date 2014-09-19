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
//
//
// History:  
//
// initial version: 
//	S.Lu: Implementation of Videau Geometry for Scintillator Hcal
//        of Barrel in ILD.
//        Endcap has been update for both SHcalScV01 and SHcalSc03 to the new AHCAl endcap technical design.
//        And both Endcap and Ring are identical to SHcalSc03
//

#include "Control.hh"
#include "MySQLWrapper.hh"
#include "MyPlacement.hh"
#include "G4UserLimits.hh"
#include "SHcalScV01.hh"
#include "CGAGeometryManager.hh"
#include "SDHcalBarrelV.hh"
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
#include "G4Transform3D.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4UnionSolid.hh"
#include "G4NistManager.hh"
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

//#define HCAL_DEBUG
#define SHCAL_CHECK_OVERLAP 0

INSTANTIATE(SHcalScV01)

G4bool 
SHcalScV01::ContextualConstruct(const CGAGeometryEnvironment 
				&aGeometryEnvironment,
				G4LogicalVolume *WorldLog)
{
        G4cout << "\nBuilding Hcal..." << G4endl;
	if (!Setup(aGeometryEnvironment)) return false;
	
	if(Control::DUMPG3) MyPlacement::Init("HCAL",GetName());
	
	//----------------------------------------------------	
	// Set up the Radiator Material to be used
	//----------------------------------------------------
	if(Hcal_radiator_material == "Iron")
		RadiatorMaterial =  CGAGeometryManager::GetMaterial("iron");
	else if(Hcal_radiator_material == "stainless_steel")
		RadiatorMaterial =  CGAGeometryManager::GetMaterial("stainless_steel");
	else if(Hcal_radiator_material == "Tungsten")
		RadiatorMaterial =  CGAGeometryManager::GetMaterial("tungsten_19.3gccm");
	else 
		Control::
		Abort("SHcalScV01: invalid radiator material name. \nIt has to be Iron, stainless_steel or Tungsten!!!",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
	
	Hcal_radiator_material+= " is the radiator material being placed.";
	Control::Log(Hcal_radiator_material.data());
	
	
	if(Hcal_endcap_radiator_material == "Iron")
		EndcapRadiatorMaterial =  CGAGeometryManager::GetMaterial("iron");
	else if(Hcal_endcap_radiator_material == "WMod")
		EndcapRadiatorMaterial =  CGAGeometryManager::GetMaterial("tungsten_11gccm");
	else if(Hcal_endcap_radiator_material == "TungstenDens24")
		EndcapRadiatorMaterial =  CGAGeometryManager::GetMaterial("TungstenDens24");
	else
		Control::Abort("SHcalScV01: invalid EndcapRadiator material name. \nIt has to be either Iron either WMod or TungstenDens24!!!",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
	
	Hcal_endcap_radiator_material+= " is the endcap radiator material being placed.";
	Control::Log(Hcal_endcap_radiator_material.data());
	
	
	
	//--------------------------------------------------------------------------
	S235 = CGAGeometryManager::GetMaterial("S235");
	
	/* PCB (Printed Circuit Board) Material FR4
	   Composition and density found under 
	   http://pcba10.ba.infn.it/temp/ddd/ma/materials/list.html 
	 */
	PCB = CGAGeometryManager::GetMaterial("PCB");
	
	Cu = CGAGeometryManager::GetMaterial("G4_Cu");
	
	Air = CGAGeometryManager::GetMaterial("air");
	
	
	
	//----------------------------------------------------
	// Hcal Barrel modules
	//----------------------------------------------------
       	
        theBarrilRegSD = new SDHcalBarrelV(Hcal_cell_dim_x,
					   Hcal_cell_dim_z,
					   Hcal_scintillator_thickness,
					   HCALBARREL,
					   "HcalBarrelReg",
					   Hcal_apply_Birks_law);
	
	RegisterSensitiveDetector(theBarrilRegSD);
	
	Barrel(WorldLog);
	
	
	//----------------------------------------------------
	// HCAL endcap modules 
	//----------------------------------------------------
	
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
	else Control::Abort("SHcalScV01: Invalid sensitive model for the chosen HCAL superdriver!",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
	
	//----------------------------------------------------
	// HCAL endcap rings
	//----------------------------------------------------
	
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
	
	
#ifdef MOKKA_GEAR
	//----------------------------------------------------
	// MokkaGear
	//----------------------------------------------------
	
	// get Manager
	MokkaGear* gearMgr = MokkaGear::getMgr() ;
	
	
	// acquire parameters from mokkaGearManager
	dblParamHalfZ                         = gearMgr->tmpParam.getDoubleVal( "TPC_Ecal_Hcal_barrel_halfZ" ) ;
	dblParamModulesGap                    = gearMgr->tmpParam.getDoubleVal( "Hcal_modules_gap" ) ;
	dblParamStavesGap                     = gearMgr->tmpParam.getDoubleVal( "Hcal_stave_gaps" ) ;
	dblParamBackPlateThickness            = gearMgr->tmpParam.getDoubleVal( "Hcal_back_plate_thickness" );
	dblParamSteelCassetteThickness        = gearMgr->tmpParam.getDoubleVal( "Hcal_steel_cassette_thickness" );
	dblParamScintillatorThickness         = gearMgr->tmpParam.getDoubleVal( "Hcal_scintillator_thickness" );
	dblParamCuThickness                   = gearMgr->tmpParam.getDoubleVal( "Hcal_Cu_thickness" );
	dblParamPCBThickness                  = gearMgr->tmpParam.getDoubleVal( "Hcal_PCB_thickness" );
	dblParamHcalModulesGap                = gearMgr->tmpParam.getDoubleVal( "Hcal_modules_gap" );
	dblParamHcalStaveGaps                 = gearMgr->tmpParam.getDoubleVal( "Hcal_stave_gaps" );
	dblParamTPCEcalHcalbarrelHalfZ        = gearMgr->tmpParam.getDoubleVal( "TPC_Ecal_Hcal_barrel_halfZ" );
	dblParamHcalLateralStructureThickness = gearMgr->tmpParam.getDoubleVal( "Hcal_lateral_structure_thickness" );
	dblHcalBarrelEndModuleType            = gearMgr->tmpParam.getIntVal(    "Hcal_barrel_end_module_type" );
	dblParamHcalEndcapSensitiveCenterBox  = gearMgr->tmpParam.getDoubleVal( "Hcal_endcap_sensitive_center_box" );

	
    // calculate zMax as total length/2
	helpBarrel.zMax = dblParamHalfZ;
	
	// get the information that are not yet included
	helpBarrel.phi0 = -pi*0.5;  
	
	helpBarrel.innerRadius = hPrime;
	
	// HCAL Barrel
	gear::CalorimeterParametersImpl* barrelParam = 
    new gear::CalorimeterParametersImpl( helpBarrel.innerRadius, helpBarrel.zMax, 8, helpBarrel.phi0 );
	
	// write all layers by position
	for (int i=0; i < helpBarrel.count; i++) {
		
		G4double calcAbsorb = Hcal_radiator_thickness;
		G4double calcThick  = Hcal_chamber_thickness + calcAbsorb;
		
		barrelParam->layerLayout().positionLayer
		(0, calcThick, Hcal_cell_dim_z, Hcal_cell_dim_x, calcAbsorb );
	}
	
	// write additional parameters
	barrelParam->setDoubleVal( "TPC_Ecal_Hcal_barrel_halfZ"  , dblParamHalfZ ) ;
	barrelParam->setDoubleVal( "Hcal_outer_radius"           , Hcal_outer_radius );
	barrelParam->setIntVal( "Hcal_barrel_number_modules"     , Hcal_barrel_number_modules );
	barrelParam->setDoubleVal( "Hcal_modules_gap"            , dblParamModulesGap ) ;
	barrelParam->setDoubleVal( "Hcal_lateral_structure_thickness"  , dblParamHcalLateralStructureThickness ) ;
	barrelParam->setDoubleVal( "Hcal_virtual_cell_size"      , Hcal_cell_dim_x );
	
	barrelParam->setDoubleVal( "Hcal_back_plate_thickness",        dblParamBackPlateThickness ) ;
	barrelParam->setDoubleVal( "Hcal_scintillator_thickness",      dblParamScintillatorThickness);
	barrelParam->setDoubleVal( "Hcal_Cu_thickness",                dblParamCuThickness);
	barrelParam->setDoubleVal( "Hcal_PCB_thickness",               dblParamPCBThickness);
	barrelParam->setDoubleVal( "Hcal_modules_gap" ,                dblParamHcalModulesGap );
	barrelParam->setDoubleVal( "TPC_Ecal_Hcal_barrel_halfZ",       dblParamTPCEcalHcalbarrelHalfZ );
	barrelParam->setDoubleVal( "Hcal_stave_gaps"  ,                dblParamHcalStaveGaps );              
	barrelParam->setIntVal(    "Hcal_barrel_end_module_type",      dblHcalBarrelEndModuleType );

	//GM: New geometry params:
	barrelParam->setDoubleVal( "InnerOctoSize"             , d_InnerOctoSize );
	
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
  endcapParam->setDoubleVal( "Hcal_stave_gaps"  ,      Hcal_stave_gaps );              
  endcapParam->setDoubleVal( "Hcal_endcap_sensitive_center_box",  Hcal_endcap_sensitive_center_box );
  endcapParam->setDoubleVal( "Hcal_steel_cassette_thickness",     Hcal_steel_cassette_thickness );
  endcapParam->setDoubleVal( "Hcal_scintillator_thickness",       Hcal_scintillator_thickness );
  endcapParam->setDoubleVal( "Hcal_Cu_thickness",                 Hcal_Cu_thickness );
  endcapParam->setDoubleVal( "Hcal_PCB_thickness",                Hcal_PCB_thickness );
  endcapParam->setDoubleVal( "Hcal_endcap_module_width",          Hcal_endcap_module_width );
  endcapParam->setDoubleVal( "Hcal_endcap_lateral_structure_thickness",  Hcal_endcap_lateral_structure_thickness );
  endcapParam->setDoubleVal( "Hcal_endcap_layer_air_gap",      Hcal_endcap_layer_air_gap );
  endcapParam->setDoubleVal( "Hcal_endcap_module_number",      Hcal_endcap_module_number );
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
	(0, calcThick, Hcal_cell_dim_x, Hcal_cell_dim_x, calcAbsorb);
    }

  // write EndcapRing parameters to GearManager
  endcapRingParam->setDoubleVal( "Hcal_virtual_cell_size", Hcal_cell_dim_x );
  endcapRingParam->setDoubleVal( "Hcal_stave_gaps"  ,      Hcal_stave_gaps );              
  endcapRingParam->setDoubleVal( "Hcal_lateral_structure_thickness" , dblParamHcalLateralStructureThickness );
  endcapRingParam->setDoubleVal( "Hcal_steel_cassette_thickness",     dblParamSteelCassetteThickness );
  endcapRingParam->setDoubleVal( "Hcal_scintillator_thickness",       dblParamScintillatorThickness );
  endcapRingParam->setDoubleVal( "Hcal_Cu_thickness",                 dblParamCuThickness );
  endcapRingParam->setDoubleVal( "Hcal_PCB_thickness",                dblParamPCBThickness );
  gearMgr->setHcalRingParameters( endcapRingParam ) ;
	
#endif
	
	// Closes Database connection
	delete db;
	
	return true;
}
SHcalScV01::~SHcalScV01() 
{
  //No op!
}  

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              Barrel
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalScV01::Barrel(G4LogicalVolume* MotherLog)
{
	G4VSolid *solidCaloTube=new G4Tubs("HcalBarrelTube",0*cm,Hcal_outer_radius,
					   TPC_Ecal_Hcal_barrel_halfZ,0*deg,360*deg);
	
	G4double ri[2]={0*cm,0*cm};
	G4double z[2]={-2.*TPC_Ecal_Hcal_barrel_halfZ,2.*TPC_Ecal_Hcal_barrel_halfZ};
	G4double ro[2]={hPrime,hPrime};
	
	G4Polyhedra *solidOctogon = new G4Polyhedra("HcalBarrelOctogon",
						    0.*deg,
						    360.*deg,
						    8,		
						    2,
						    z,
						    ri,
						    ro); 
	G4RotationMatrix *rotOctogon=new G4RotationMatrix(G4ThreeVector(0,0,1),360/16.0*deg);
	G4VSolid *solidCalo=new G4SubtractionSolid("HcalBarrelTube-Octogon",solidCaloTube,solidOctogon,rotOctogon,G4ThreeVector(0,0,0)); 
	
	G4LogicalVolume *logicCalo = new G4LogicalVolume(solidCalo,   
							 RadiatorMaterial , 
							 "HcalBarrelEnvLog"); 
	new MyPlacement(0,
			G4ThreeVector(0.,0.,0.),
			logicCalo, 
			"HcalBarrelEnvPhys", 
			MotherLog,
			false,
			0);
	
	G4VisAttributes * VisAttBarrel = new G4VisAttributes(G4Colour(.8,.8,.2));
	VisAttBarrel->SetForceWireframe(true);
	//VisAttBarrel->SetForceSolid(true);
	VisAttBarrel->SetDaughtersInvisible(true);
	logicCalo->SetVisAttributes(VisAttBarrel);
	
	BarrelVirtualModules(logicCalo);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              Barrel Virtual Modules               ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalScV01::BarrelVirtualModules(G4LogicalVolume* MotherLog)
{
	G4UserLimits* pULimits=
	  new G4UserLimits(theMaxStepAllowed);
	
	for(G4int layerId=0; layerId < Hcal_nlayers; layerId++) {
		
		G4double yn = sqrt(Hcal_outer_radius*Hcal_outer_radius -
				   (hPrime + (layerId+1)*layerThickness)*(hPrime + (layerId+1)*layerThickness));
		G4double Yn = 0.5*d_InnerOctoSize - (layerId+1)*layerThickness;
		
		G4double halfX = Hcal_chamber_thickness/2.;
		G4double halfY = (yn+Yn)/2.;
		G4double halfZ = Hcal_regular_chamber_dim_z / 2.;
		
		G4Box * chamberSolid = new G4Box("ChamberSolid",
						 halfX, halfY, halfZ);
		
		G4LogicalVolume * chamberLogical= BuildChamberBox(chamberSolid,
								  theBarrilRegSD,
								  layerId+1,
								  pULimits);
		
		G4VisAttributes * VisAttChamber = new G4VisAttributes(G4Colour::Blue());
		VisAttChamber->SetForceWireframe(true);
		VisAttChamber->SetDaughtersInvisible(true);
		chamberLogical->SetVisAttributes(VisAttChamber);
		
		G4double localXPos = hPrime + Hcal_radiator_thickness +
		Hcal_scintillator_thickness/2. + layerId*layerThickness;
		
#ifdef MOKKA_GEAR
		helpBarrel.count += 1;
		
		// position of layer as offset + half thickness
		helpBarrel.layerPos.push_back(localXPos);
#endif
		
		G4double localYPos = (yn - Yn)*0.5;
		
		G4double stave_phi_offset, module_z_offset;
		
		theBarrilRegSD->AddLayer(layerId+1,localXPos,-Yn,Hcal_normal_dim_z/2.);
		
		stave_phi_offset = -pi*0.5;  //SLU: 12 o'clock is the stave 1 in the barrel, 3 o'clock is stave 3
		for (G4int stave_id = 1;
			 stave_id <= 8;
			 stave_id++)
		{
			
			G4double phirot = stave_phi_offset+(stave_id-1)*pi/4.;
			
			G4RotationMatrix *rot=new G4RotationMatrix();
			rot->rotateZ(phirot);
			
			
			G4RotationMatrix *rotInverse=new G4RotationMatrix();
			rotInverse->rotateZ(-phirot);
			
			
			if(layerId ==0)
				theBarrilRegSD->SetStaveRotationMatrix(stave_id,phirot);
			
			for (G4int module_id = 1;
				 module_id <= Hcal_barrel_number_modules;
				 module_id++)
			{
				module_z_offset = - TPC_Ecal_Hcal_barrel_halfZ+
				Hcal_normal_dim_z/2. + 
				(module_id-1)*(Hcal_normal_dim_z+Hcal_modules_gap);
				
				G4ThreeVector localPos(localXPos,localYPos,-module_z_offset);
				G4ThreeVector newPos = (*rotInverse)*localPos;
				
				G4int copyNb = 1000*stave_id + 100*module_id + HCALBARREL;
				
				new MyPlacement(rot,
						newPos,
						chamberLogical, 
						"HcalChamberPhys", 
						MotherLog,
						false,
						copyNb);
				if((stave_id == 1) && (layerId ==0))
					theBarrilRegSD->SetModuleZOffset(module_id,-module_z_offset);
			}
		}
	}
}



G4LogicalVolume * 
SHcalScV01::BuildChamberBox(G4Box* ChamberSolid, 
			    SDHcalBarrelV* theSD, 
			    G4int layerId,
			    G4UserLimits* pULimits)
{
	if(ChamberSolid->GetEntityType()=="G4Box")
    {
		
		//parameters of the chamber
		G4double Chamber_Length                   =2*(ChamberSolid->GetZHalfLength());
		G4double Chamber_Width                    =2*(ChamberSolid->GetYHalfLength());//Parameter that changes
		G4double Chamber_Thickness                =2*(ChamberSolid->GetXHalfLength());
				
		G4VisAttributes *VisAtt;
		
		/*--------------------------------------------------------------------------------
		 build chamber box, with the calculated dimensions 
		 -------------------------------------------------------------------------------*/
		G4LogicalVolume *LogicalLayer =
		new G4LogicalVolume(ChamberSolid,
				    CGAGeometryManager::GetMaterial("air"),
				    "LogicalLayer", 
				    0, 0, 0);
		VisAtt = new G4VisAttributes(G4Colour::Blue());
		VisAtt->SetForceSolid(true);
		LogicalLayer->SetVisAttributes(VisAtt);
		
		/*-------------------------------------------------------------------------------
		 build the scintillator box 
		 ------------------------------------------------------------------------------*/
		
		G4Box *ScintSolid = new G4Box("ScintSolid", Hcal_scintillator_thickness/2.,Chamber_Width/2.,Chamber_Length/2.);
		
		G4LogicalVolume* ScintLog = new G4LogicalVolume(ScintSolid,
								CGAGeometryManager::GetMaterial("polystyrene"),
								"ScintLogical", 
								0, 0, pULimits); 
		VisAtt = new G4VisAttributes(G4Colour::Yellow());
		VisAtt->SetForceSolid(true);
		ScintLog->SetVisAttributes(VisAtt);
		
		G4double ScintillatorPosX = - (Chamber_Thickness - Hcal_scintillator_thickness)/2.;
		
		new G4PVPlacement(0,		   //no rotation
				  G4ThreeVector(ScintillatorPosX,0,0),  //its position
				  ScintLog,     //its logical volume		    
				  "Scintillator", //its name
				  LogicalLayer,        //its mother
				  false,             //no boulean operat
				  layerId);       
		
		/* only scintillator is sensitive*/
		ScintLog->SetSensitiveDetector(theSD);
		
		/*--------------------------------------------------------------------------------
		 build the PCB box 
		 -------------------------------------------------------------------------------*/
		G4Box *PCBBox = new G4Box("PCBBox", Hcal_PCB_thickness/2.,Chamber_Width/2.,Chamber_Length/2.);
		G4LogicalVolume *PCBLog = new G4LogicalVolume(PCBBox, CGAGeometryManager::GetMaterial("PCB"), "PCBLog", 0, 0, 0);
		VisAtt = new G4VisAttributes(G4Colour::Grey());
		VisAtt->SetForceSolid(true);
		PCBLog->SetVisAttributes(VisAtt);
		
		G4double PCBPosX = ScintillatorPosX + Hcal_scintillator_thickness/2 + Hcal_PCB_thickness/2;
		
		new G4PVPlacement(0,		   //no rotation
				  G4ThreeVector(PCBPosX,0.,0.),  //its position
				  PCBLog,     //its logical volume		    
				  "PCB", //its name
				  LogicalLayer,        //its mother
				  false,             //no boulean operat
				  0);                //copy number
		
		/*--------------------------------------------------------------------------------
		 build the Cu box 
		 --------------------------------------------------------------------------------*/
		
		G4Box *CuBox = new G4Box("CuBox", Hcal_Cu_thickness/2,Chamber_Width/2,Chamber_Length/2);
		G4LogicalVolume *CuLog = new G4LogicalVolume(CuBox, CGAGeometryManager::GetMaterial("G4_Cu"), "CuLog", 0, 0, 0);
		VisAtt = new G4VisAttributes(G4Colour::Cyan());
		VisAtt->SetForceSolid(true);
		CuLog->SetVisAttributes(VisAtt);
		
		G4double CuPosX = PCBPosX + Hcal_PCB_thickness/2 + Hcal_Cu_thickness/2;
		
		new G4PVPlacement(0,		   //no rotation
				  G4ThreeVector(CuPosX,0.,0.),  //its position
				  CuLog,     //its logical volume		    
				  "Cu", //its name
				  LogicalLayer,        //its mother
				  false,             //no boulean operat
				  0);                //copy number
		
		/*--------------------------------------------------------------------------------
		 build the air box for Hcal_fiber_gap which used for electronic cables 
		 --------------------------------------------------------------------------------*/
		
		G4Box *AirGapBox = new G4Box("AirGapBox", Hcal_fiber_gap/2,Chamber_Width/2,Chamber_Length/2);
		G4LogicalVolume *AirGapLog = new G4LogicalVolume(AirGapBox, CGAGeometryManager::GetMaterial("air"), "AirGapLog", 0, 0, 0);
		VisAtt = new G4VisAttributes(G4Colour::White());
		VisAtt->SetForceSolid(true);
		AirGapLog->SetVisAttributes(VisAtt);
		
		G4double AirGapPosX =  CuPosX + Hcal_Cu_thickness/2 + Hcal_fiber_gap/2;
		
		new G4PVPlacement(0,		   //no rotation
				  G4ThreeVector(AirGapPosX,0.,0.),  //its position
				  AirGapLog,     //its logical volume		    
				  "Air", //its name
				  LogicalLayer,        //its mother
				  false,             //no boulean operat
				  0);                //copy number
		
		
		
		return LogicalLayer;
		
    }
	
	Control::Abort("SHcalScV01::BuildChamberBox: invalid ChamberSolidEnvelope",
				   MOKKA_OTHER_ERRORS);
	return NULL;
}




void 
SHcalScV01::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
	// In this complex subdetector the best is to read
	// all hits in the one of the sensitive detectors,
	// just for visualisation, so just for your eyes...
	theBarrilRegSD->LoadEvent(theSubDetectorEventHitsFileInput);
}

G4bool 
SHcalScV01::Setup(const CGAGeometryEnvironment &theGeometryEnvironment)
{
	Hcal_radiator_thickness         = theGeometryEnvironment.GetParameterAsDouble("Hcal_radiator_thickness");
	Hcal_radiator_material		= theGeometryEnvironment.GetParameterAsString("Hcal_radiator_material");
	Hcal_ring			= theGeometryEnvironment.GetParameterAsInt("Hcal_ring");
	Hcal_radial_ring_inner_gap	= theGeometryEnvironment.GetParameterAsInt("Hcal_radial_ring_inner_gap");
	Hcal_stave_gaps			= theGeometryEnvironment.GetParameterAsDouble("Hcal_stave_gaps");
	Hcal_modules_gap		= theGeometryEnvironment.GetParameterAsDouble("Hcal_modules_gap");
	Hcal_nlayers			= theGeometryEnvironment.GetParameterAsInt("Hcal_nlayers");
	Hcal_barrel_number_modules	= theGeometryEnvironment.GetParameterAsInt("Hcal_barrel_number_modules");
	Hcal_chamber_thickness		= theGeometryEnvironment.GetParameterAsDouble("Hcal_chamber_thickness");
	Hcal_steel_cassette_thickness   = theGeometryEnvironment.GetParameterAsDouble("Hcal_steel_cassette_thickness");
	Hcal_sensitive_model            = theGeometryEnvironment.GetParameterAsString("Hcal_sensitive_model");
	
	Ecal_endcap_zmin		= theGeometryEnvironment.GetParameterAsDouble("Ecal_endcap_zmin");
	
	Ecal_endcap_outer_radius	= theGeometryEnvironment.GetParameterAsDouble("Ecal_endcap_outer_radius");
	
	Hcal_endcap_radiator_thickness  = theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_radiator_thickness");
	Hcal_endcap_nlayers             = theGeometryEnvironment.GetParameterAsInt   ("Hcal_endcap_nlayers");
	
	Hcal_endcap_module_width                 = theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_module_width");
	Hcal_endcap_module_number                = theGeometryEnvironment.GetParameterAsInt("Hcal_endcap_module_number");
	Hcal_endcap_lateral_structure_thickness  = theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_lateral_structure_thickness");
	Hcal_endcap_layer_air_gap                = theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_layer_air_gap");
	Hcal_endcap_services_module_width         = theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_services_module_width");
	HcalServices_outer_FR4_thickness         = theGeometryEnvironment.GetParameterAsDouble("HcalServices_outer_FR4_thickness");
	HcalServices_outer_Cu_thickness          = theGeometryEnvironment.GetParameterAsDouble("HcalServices_outer_Cu_thickness");
	
	hPrime =
	  theGeometryEnvironment.GetParameterAsDouble("Ecal_outer_radius")
	  + theGeometryEnvironment.GetParameterAsDouble("Hcal_Ecal_gap");
	Hcal_inner_radius = hPrime / cos(pi/8.);
	
	Hcal_endcap_cables_gap		= theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_cables_gap");
	Hcal_endcap_ecal_gap		= theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_ecal_gap");
	
	TPC_Ecal_Hcal_barrel_halfZ	= theGeometryEnvironment.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");
	
	Hcal_normal_dim_z = (2 * TPC_Ecal_Hcal_barrel_halfZ -
			     (Hcal_barrel_number_modules-1)*Hcal_modules_gap)
	                     /Hcal_barrel_number_modules;
	
	Hcal_start_z = TPC_Ecal_Hcal_barrel_halfZ + Hcal_endcap_cables_gap;
	
	/* This was true for only 2 modules along Z in Barrel:
	 Hcal_start_z =  Hcal_normal_dim_z 
	 + Hcal_modules_gap / 2. + Hcal_endcap_cables_gap;
	 */
	// Hcal_start_z is the Hcal Endcap boundary coming from the IP
	// Test Hcal_start_z against Ecal_endcap_zmax + Hcal_endcap_ecal_gap
	// to avoid overlap problems with Ecal if scaled.
	//
	Ecal_endcap_zmax		= theGeometryEnvironment.GetParameterAsDouble("Ecal_endcap_zmax");
	Hcal_endcap_radiator_material   = theGeometryEnvironment.GetParameterAsString("Hcal_endcap_radiator_material");
	
	if( Hcal_start_z < Ecal_endcap_zmax + Hcal_endcap_ecal_gap )
		Hcal_start_z = Ecal_endcap_zmax + Hcal_endcap_ecal_gap;
	
	Hcal_lateral_plate_thickness	= theGeometryEnvironment.GetParameterAsDouble("Hcal_lateral_structure_thickness");
	Hcal_cells_size			= theGeometryEnvironment.GetParameterAsDouble("Hcal_cells_size");
	

	theMaxStepAllowed = Hcal_cells_size;
	
	Hcal_scintillator_thickness	= theGeometryEnvironment.GetParameterAsDouble("Hcal_scintillator_thickness");
	Hcal_Cu_thickness		= theGeometryEnvironment.GetParameterAsDouble("Hcal_Cu_thickness");
	Hcal_PCB_thickness		= theGeometryEnvironment.GetParameterAsDouble("Hcal_PCB_thickness");
	Hcal_fiber_gap			= theGeometryEnvironment.GetParameterAsDouble("Hcal_fiber_gap");
	Hcal_endcap_center_box_size	= theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_center_box_size");
	Hcal_apply_Birks_law		= theGeometryEnvironment.GetParameterAsDouble("Hcal_apply_Birks_law");
	
	
	// general calculated parameters
	G4double MinNumCellsInTransvPlane  = 6;//Length of 48th Layer is 30 mm * 6 = 180 mm (LMin)
	G4double AngleRatio=0.76536686;//"k"
	d_InnerOctoSize=AngleRatio*Hcal_inner_radius;//"d"
	layerThickness = Hcal_radiator_thickness +Hcal_chamber_thickness;
	
	G4double LMin = Hcal_cells_size*MinNumCellsInTransvPlane;// 180 mm
	
	G4double Ynl = 0.5*d_InnerOctoSize - Hcal_nlayers*layerThickness;
	
	Hcal_outer_radius = 
	sqrt((LMin-Ynl)*(LMin-Ynl) +
		 (hPrime + Hcal_nlayers*layerThickness)*
		 (hPrime + Hcal_nlayers*layerThickness));
	
	Hcal_total_dim_y = Hcal_outer_radius - hPrime;
	
	Hcal_back_plate_thickness	= theGeometryEnvironment.GetParameterAsDouble("Hcal_back_plate_thickness");
	
	Hcal_endcap_thickness  = Hcal_nlayers*layerThickness + Hcal_back_plate_thickness;
	
	Hcal_endcap_rmax = Hcal_outer_radius * cos(pi/8.);
	Hcal_endcap_total_z = Hcal_endcap_nlayers * (Hcal_endcap_radiator_thickness + Hcal_chamber_thickness) + Hcal_back_plate_thickness;
	
	// the  y_dim1_for_z kept as the original value in TDR
	Hcal_regular_chamber_dim_z =  Hcal_normal_dim_z - 2 *(Hcal_lateral_plate_thickness);
	Hcal_cell_dim_x = Hcal_cells_size;
	Hcal_cell_dim_z = Hcal_cells_size;
	
#ifdef MOKKA_GEAR
	MokkaGear* mokkaGearMgr = MokkaGear::getMgr() ;
	
	mokkaGearMgr->tmpParam.setDoubleVal("TPC_Ecal_Hcal_barrel_halfZ" , 
					    theGeometryEnvironment.GetParameterAsDouble(  "TPC_Ecal_Hcal_barrel_halfZ" ) ) ;	
	mokkaGearMgr->tmpParam.setDoubleVal("Hcal_back_plate_thickness" , 
					    theGeometryEnvironment.GetParameterAsDouble("Hcal_back_plate_thickness") ) ;
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


G4bool 
SHcalScV01::PostConstructAction(CGAGeometryEnvironment& )
{
	//
	// Propagates the changes to Coil, if any. The SHcal has also the responsability 
	// to change the calorimeter region parameters.
	//
	
	G4double Hcal_R_max = Hcal_outer_radius;
	std::ostringstream oss1;
	oss1 << Hcal_R_max;
	(*Control::globalModelParameters)["Hcal_R_max"] =
	  oss1.str();
	(*Control::globalModelParameters)["calorimeter_region_rmax"] =
	  oss1.str();
	
	std::ostringstream oss2;
	oss2 << Hcal_start_z;
	(*Control::globalModelParameters)["Hcal_endcap_zmin"] =
	  oss2.str();
	
	G4double calorimeter_region_zmax =
	  Hcal_start_z + Hcal_total_dim_y;
	std::ostringstream oss3;  
	oss3 << calorimeter_region_zmax;
	(*Control::globalModelParameters)["calorimeter_region_zmax"] =
	  oss3.str();
	
	
	G4cout << "SHcalScV01 information: Hcal_outer_radius = "
	<< Hcal_outer_radius
	<< "\n                   module thickness = "
	<< Hcal_total_dim_y
	<< "\n                   endcap module thickness = "
	<< Hcal_endcap_total_z
	<< "\n                   Hcal_R_max = "
	<< Hcal_R_max
	<< "\n                   Hcal_endcap_zmin = "
	<< Hcal_start_z
	<< "\n                   calorimeter_region_rmax = "
	<< Hcal_R_max
	<< "\n                   calorimeter_region_zmax = "
	<< calorimeter_region_zmax
	<< G4endl;
	
	return true;    
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Build HCAL endcaps                                            ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalScV01::EndcapsAhcal(G4LogicalVolume* MotherLog)
{
        G4cout << "Hcal_outer_radius: " << Hcal_outer_radius <<G4endl;
	
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
	    else Control::Abort("SHcalScV01: Invalid sensitive HCAL model !",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
	    
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
void SHcalScV01::EndcapChambersAhcal(G4LogicalVolume* MotherLog, 
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
void SHcalScV01::EndcapsAhcalFrontEndElectronics(G4LogicalVolume* MotherLog, G4int layer_id)
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
//~              EndcapRings                          ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalScV01::EndcapRings(G4LogicalVolume* MotherLog)
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
	
#ifdef HCAL_DEBUG
	G4cout<<"    EndcapRings         : Rinner="<<pRMin<<"mm Router="<<pRMax<<G4endl;
#endif
	
	if (rOuter[0] <= rInner[0]) 
		G4Exception("SHcalScV01::EndcapRings() - not enough place for endcap rings (try a larger Hcal_nlayers number)!");
	
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
#ifdef HCAL_DEBUG
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

//=======================================================================
void SHcalScV01::EndcapChambers(G4LogicalVolume* MotherLog, SDHcalEndCap* theSD, bool rings)
{
	// Chambers in the SHcalScV01::Endcaps
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
	
#ifdef HCAL_DEBUG
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
		
		G4double scintHalfWidth = pDz - (Hcal_PCB_thickness + Hcal_Cu_thickness) / 2. ;
		
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
#ifdef HCAL_DEBUG
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
			helpEndcapRing.sensThickness.push_back( Hcal_scintillator_thickness );
			helpEndcapRing.steelCassetteThickness.push_back( Hcal_steel_cassette_thickness ) ;
			helpEndcapRing.PCBThickness.push_back( Hcal_PCB_thickness ) ;
			helpEndcapRing.CuThickness.push_back( Hcal_Cu_thickness ) ;
		}
		else{
			helpEndcap.sensThickness.push_back( Hcal_scintillator_thickness ) ;
			helpEndcap.steelCassetteThickness.push_back( Hcal_steel_cassette_thickness );
			helpEndcap.PCBThickness.push_back( Hcal_PCB_thickness );
			helpEndcap.CuThickness.push_back( Hcal_Cu_thickness );
		}
#endif
		
		new MyPlacement(0, 
				G4ThreeVector( 0, 0,  - (Hcal_PCB_thickness + Hcal_Cu_thickness) / 2.), 
				ScintLog,
				"EndCapScintillator", 
				EndCapStaveLogical, 
				false, 
				0);   
	  }
	else Control::Abort("SHcalScV01: Invalid sensitive model parameter!",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
	
	G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour::Blue());
#ifdef HCAL_DEBUG
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
			helpEndcapRing.layerPos.push_back(Zoff + std::abs(zPlane[0]) + (Hcal_PCB_thickness + Hcal_Cu_thickness)/2 ) ;
			
			helpEndcapRing.sensThickness.push_back( Hcal_scintillator_thickness );
			helpEndcapRing.steelCassetteThickness.push_back( Hcal_steel_cassette_thickness );
			helpEndcapRing.PCBThickness.push_back(Hcal_PCB_thickness);
			helpEndcapRing.CuThickness.push_back(Hcal_Cu_thickness);
		}
		else {
			helpEndcap.count += 1;
			
			// position of layer as offset + half thickness
			helpEndcap.layerPos.push_back(Zoff + std::abs(zPlane[0]) + (Hcal_PCB_thickness + Hcal_Cu_thickness)/2) ;
			
			helpEndcap.sensThickness.push_back( Hcal_scintillator_thickness );
			helpEndcap.steelCassetteThickness.push_back( Hcal_steel_cassette_thickness );
			helpEndcap.PCBThickness.push_back(Hcal_PCB_thickness);
			helpEndcap.CuThickness.push_back(Hcal_Cu_thickness);
		}
#endif
		
	  }  
}

