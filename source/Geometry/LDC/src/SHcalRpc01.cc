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
// $Id: SHcalRpc01.cc,v 1.16 2009/02/05 17:27:05 musat Exp $
// $Name: mokka-07-00 $
//
//
// History:  
//
// initial version: 
//	G.Musat: Implementation following the example from Emmanuel Latour
// 		 Super driver without tmp database and able
//               to build Hcal barrel with just two modules in stave.

#include "Control.hh"
#include "MySQLWrapper.hh"
#include "MyPlacement.hh"
#include "G4UserLimits.hh"
#include "SHcalRpc01.hh"
#include "CGAGeometryManager.hh"
#include "SDHcalSD01.hh"
#include "SDHcalECSD01.hh"

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

#include "G4GeometryTolerance.hh"

#include <algorithm>
#include <assert.h>

#include "CGADefs.h"


#ifdef MOKKA_GEAR
#include "gear/CalorimeterParameters.h" 
#include "gearimpl/CalorimeterParametersImpl.h" 
#include "gearimpl/LayerLayoutImpl.h" 
#include "MokkaGear.h"
#endif

INSTANTIATE(SHcalRpc01)
  
//#define HCAL_DEBUG 1

G4bool 
SHcalRpc01::ContextualConstruct(const CGAGeometryEnvironment 
			     &aGeometryEnvironment,
			     G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding Hcal..." << G4endl;
  if (!Setup(aGeometryEnvironment)) return false;

  BuildMaterials();
  
  if(Control::DUMPG3) MyPlacement::Init("HCAL",GetName());
  
  hcalECChamberInnerHole = 0;
  rotBox = 0;
  
  //  Hcal  barrel regular modules
  theBarrilRegSD = 
    new SDHcalSD01(Hcal_cell_dim_x,
	   Hcal_cell_dim_z,
	   RPC_Gap_Thickness,
	   HCALBARREL,
	   "HcalBarrel",RPC_PadSeparation);
  RegisterSensitiveDetector(theBarrilRegSD);
  

  // Hcal  endcap modules
  theENDCAPEndSD =
    new SDHcalECSD01(Hcal_cell_dim_x,
	      Hcal_cell_dim_z,
	      Hcal_chamber_tickness,
	      HCALENDCAPMINUS,
	      "HcalEndCaps",
	      RPC_PadSeparation);
  RegisterSensitiveDetector(theENDCAPEndSD);
  
  if(Hcal_ring > 0 )
    {
      theENDCAPRingSD =
	new SDHcalECSD01(Hcal_cell_dim_x,
		  Hcal_cell_dim_z,
		  Hcal_chamber_tickness,
		  HCALENDCAPMINUS,
		  "HcalEndCapRings",
		  RPC_PadSeparation);
      RegisterSensitiveDetector(theENDCAPRingSD);
    }
  
  // Set up the Radiator Material to be used
  if(Hcal_radiator_material == "Iron")
      RadiatorMaterial =  CGAGeometryManager::GetMaterial("iron");
  else if(Hcal_radiator_material == "stainless_steel")
      RadiatorMaterial =  CGAGeometryManager::GetMaterial("stainless_steel");
  else if(Hcal_radiator_material == "Tungsten")
      RadiatorMaterial =  CGAGeometryManager::GetMaterial("tungsten_19.3gccm");
  else 
      Control::
	Abort("SHcalRpc01: invalid radiator material name. \nIt has to be Iron, stainless_steel or Tungsten!!!",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
  
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
  
  
  // acquire parameters from mokkaGearManager
  G4int intParamNcellsz              = gearMgr->tmpParam.getIntVal( "N_cells_z" ) ;
  dblParamHalfZ                     = gearMgr->tmpParam.getDoubleVal( "TPC_Ecal_Hcal_barrel_halfZ" ) ;
  dblParamLateralStructureThickness = gearMgr->tmpParam.getDoubleVal( "Hcal_lateral_structure_thickness" ) ;
  dblParamModulesGap                = gearMgr->tmpParam.getDoubleVal( "Hcal_modules_gap" ) ;
  dblParamStavesGap                 = gearMgr->tmpParam.getDoubleVal( "Hcal_stave_gaps" ) ;

    // calculate zMax as total length/2
  helpBarrel.zMax = dblParamHalfZ;

  // get the information that are not yet included
  helpBarrel.phi0 = pi*0.5;  

  helpBarrel.innerRadius = hPrime;

  // HCAL Barrel
  gear::CalorimeterParametersImpl* barrelParam = 
    new gear::CalorimeterParametersImpl( helpBarrel.innerRadius, helpBarrel.zMax, 8, helpBarrel.phi0 );

  // write all layers by position
  for (int i=0; i < helpBarrel.count; i++) {
    
    G4double calcAbsorb = Hcal_radiator_thickness;
    G4double calcThick  = Hcal_chamber_tickness + calcAbsorb;

/*
    // on last layer, gap has to be taken into account
    if( i == ( helpBarrel.count - 1 ) ) {
      G4double layerGap = helpBarrel.layerPos[i] - helpBarrel.layerPos[i-1] - calcThick ;
      
      // check if layerGap makes sense
      if ( layerGap < calcThick ) {
	
	// add gap to Thickness
	calcThick = calcThick + layerGap ;
      }
    }
*/

    barrelParam->layerLayout().positionLayer
      (0, calcThick, Hcal_cell_dim_z, Hcal_cell_dim_x, calcAbsorb );
  }

  // write additional parameters
  barrelParam->setDoubleVal( "TPC_Ecal_Hcal_barrel_halfZ"  , dblParamHalfZ ) ;
  barrelParam->setDoubleVal( "Hcal_outer_radius"           , Hcal_outer_radius );
  barrelParam->setIntVal( "Hcal_barrel_number_modules"     , Hcal_barrel_number_modules );
  barrelParam->setDoubleVal( "Hcal_modules_gap"            , dblParamModulesGap ) ;
  barrelParam->setDoubleVal( "Hcal_lateral_structure_thickness"  , dblParamLateralStructureThickness ) ;
  barrelParam->setDoubleVal( "Hcal_virtual_cell_size"      , Hcal_cell_dim_x );
  
//GM: New geometry params:
  barrelParam->setDoubleVal( "InnerOctoSize"             , d_InnerOctoSize );
  barrelParam->setDoubleVal( "RPC_PadSeparation"             , RPC_PadSeparation );
  barrelParam->setIntVal( "N_cells_z"             , intParamNcellsz );
  barrelParam->setDoubleVal( "FrameWidth"             , RPC_EdgeWidth );

  // write Barrel parameters to GearManager
  gearMgr->setHcalBarrelParameters( barrelParam ) ;  

  // HCAL Endcap
  gear::CalorimeterParametersImpl * endcapParam =
    new gear::CalorimeterParametersImpl (helpEndcap.innerRadius, helpEndcap.outerRadius, helpEndcap.leastZ, 2, helpEndcap.phi0);

  // write all layers by position
  for (int i=0; i < helpEndcap.count; i++) {
    
    G4double calcAbsorb = Hcal_radiator_thickness;
    G4double calcThick  = Hcal_chamber_tickness + calcAbsorb;

/*
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
*/

    endcapParam->layerLayout().positionLayer
      (0, calcThick, Hcal_cell_dim_z, Hcal_cell_dim_x, calcAbsorb);
    
  }

  // write Endcap parameters to GearManager
  endcapParam->setDoubleVal( "Hcal_virtual_cell_size"      , Hcal_cell_dim_x );
  endcapParam->setDoubleVal( "FrameWidth",0.0);
  gearMgr->setHcalEndcapParameters( endcapParam ) ;


  //HCAL Endcap Ring
 gear::CalorimeterParametersImpl * endcapRingParam =
    new gear::CalorimeterParametersImpl (helpEndcapRing.innerRadius, helpEndcapRing.outerRadius, helpEndcapRing.leastZ, 2, helpEndcapRing.phi0);

// write all layers by position
  for (int i=0; i < helpEndcapRing.count; i++) {
    
    G4double calcAbsorb = Hcal_radiator_thickness;
    G4double calcThick  = Hcal_chamber_tickness + calcAbsorb;
/*
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
*/
    endcapRingParam->layerLayout().positionLayer
      (0, calcThick, Hcal_cell_dim_z, Hcal_cell_dim_x, calcAbsorb);
  }
    // write EndcapRing parameters to GearManager
    endcapRingParam->setDoubleVal( "Hcal_virtual_cell_size", Hcal_cell_dim_x );
    endcapRingParam->setDoubleVal( "FrameWidth",0.0);
    gearMgr->setHcalRingParameters( endcapRingParam ) ;

#endif

  // Closes Database connection
  delete db;

  return true;
}
SHcalRpc01::~SHcalRpc01() 
{
  //  if(theBarrilRegSD!=0) delete theBarrilRegSD;
  //  if(theENDCAPEndSD!=0) delete theENDCAPEndSD;
}  

void SHcalRpc01::BuildMaterials(void) {

// here we only build the mixture of Graphite, Mylar, Air, g10 
// for EndCaps and EndCapRings:

// since these materials cover all the surface of the chamber, their volume
// fractions are given by their thicknesses

G4double graphiteThickness = RPC_Graphite_ThicknessAnode + 
				RPC_Graphite_ThicknessCathode;
G4double mylarThickness = RPC_mylar_ThicknessAnode +
				RPC_mylar_ThicknessCathode;
G4double airThickness = RPC_Free_Thickness;
G4double g10Thickness = RPC_ChipPackageThickness + RPC_PCB_Thickness;

G4double mixThickness = graphiteThickness + mylarThickness +
		airThickness + g10Thickness; 

G4double graphiteVF = graphiteThickness / mixThickness;
G4double mylarVF    = mylarThickness / mixThickness;
G4double airVF      = airThickness / mixThickness;
G4double g10VF      = g10Thickness / mixThickness;

theGraphiteMaterial = CGAGeometryManager::GetMaterial("graphite");
theMylarMaterial = CGAGeometryManager::GetMaterial("mylar");
theAirMaterial = CGAGeometryManager::GetMaterial("air");
theG10Material = CGAGeometryManager::GetMaterial("g10");


G4double graphiteDensity = (theGraphiteMaterial->GetDensity()) / (g/cm3);
G4double mylarDensity    = (theMylarMaterial->GetDensity()) / (g/cm3);
G4double airDensity      = (theAirMaterial->GetDensity()) / (g/cm3);
G4double g10Density      = (theG10Material->GetDensity()) / (g/cm3);

G4double mixDensity = graphiteDensity*graphiteVF + mylarDensity*mylarVF +
			airDensity*airVF + g10Density*g10VF;

G4double graphiteFractionMass = graphiteVF * graphiteDensity / mixDensity;
G4double mylarFractionMass    = mylarVF * mylarDensity / mixDensity;
G4double airFractionMass      = airVF * airDensity / mixDensity;
G4double g10FractionMass      = g10VF * g10Density / mixDensity;

mixDensity *= (g/cm3);

theRPC2ECRMixture =  new G4Material("RPC2ECRMix", mixDensity, 4);
theRPC2ECRMixture->AddMaterial(theGraphiteMaterial,graphiteFractionMass);
theRPC2ECRMixture->AddMaterial(theMylarMaterial,mylarFractionMass);
theRPC2ECRMixture->AddMaterial(theAirMaterial,airFractionMass);
theRPC2ECRMixture->AddMaterial(theG10Material,g10FractionMass);

G4cout << "theRPC2ECRMixture->GetRadlen() = "
                << theRPC2ECRMixture->GetRadlen() /mm   << " mm\n";
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              Barrel
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalRpc01::Barrel(G4LogicalVolume* MotherLog)
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
void SHcalRpc01::BarrelVirtualModules(G4LogicalVolume* MotherLog)
{
  G4UserLimits* pULimits=
    new G4UserLimits(theMaxStepAllowed);

for(G4int layerId=0; layerId < Hcal_nlayers; layerId++) {

  G4double yn = sqrt(Hcal_outer_radius*Hcal_outer_radius -
   (hPrime + (layerId+1)*layerThickness)*(hPrime + (layerId+1)*layerThickness));
  G4double Yn = 0.5*d_InnerOctoSize - (layerId+1)*layerThickness;

  G4double halfX = Hcal_chamber_tickness/2.;
  G4double halfZ = (yn+Yn)/2.;

#ifdef MOKKA_DEBUG
if((halfZ-RPC_EdgeWidth)<=0.)
	Control::Abort("SHcalRpc01:Too short layer",MOKKA_OTHER_ERRORS);
#endif

//  G4double halfY = Hcal_normal_dim_z / 2.;
  G4double halfY = Hcal_regular_chamber_dim_z / 2.;

  G4Box * chamberSolid = new G4Box("ChamberSolid",
	halfX, halfY, halfZ);

  G4LogicalVolume * chamberLogical= BuildRPC2Box(chamberSolid,
			   theBarrilRegSD,
			   layerId+1,
			   pULimits);

  G4VisAttributes * VisAttChamber = new G4VisAttributes(G4Colour(.9,.1,.1));
  VisAttChamber->SetForceWireframe(true);
  VisAttChamber->SetDaughtersInvisible(true);
  chamberLogical->SetVisAttributes(VisAttChamber);

  G4double localXPos = hPrime + Hcal_radiator_thickness +
		Hcal_chamber_tickness/2. + layerId*layerThickness;;

#ifdef MOKKA_GEAR
	helpBarrel.count += 1;

	// position of layer as offset + half thickness
	helpBarrel.layerPos.push_back(localXPos);
#endif
      
  G4double localYPos = -Yn + 0.5*(Yn + yn);
  
  G4double stave_phi_offset, module_z_offset;

  theBarrilRegSD->AddLayer(layerId+1,
	localXPos+RPC_GapPosX,localYPos,0.5*(yn+Yn)-RPC_EdgeWidth,Hcal_regular_chamber_dim_z/2.-RPC_EdgeWidth); 
  
  stave_phi_offset = pi*0.5;  
  for (G4int stave_id = 1;
       stave_id <= 8;
       stave_id++)
    {

      G4double phirot = stave_phi_offset+(stave_id-1)*pi/4.;

  	G4RotationMatrix *rot=new G4RotationMatrix();
  	rot->rotateX(pi*0.5);
	rot->rotateY(phirot);

  	G4RotationMatrix *rotInverse=new G4RotationMatrix();
	rotInverse->rotateY(-phirot);
	rotInverse->rotateX(-pi*0.5);

	if(layerId ==0)
		theBarrilRegSD->SetStaveRotationMatrix(stave_id,rotInverse);

    for (G4int module_id = 0;
	 module_id < Hcal_barrel_number_modules;
	 module_id++)
      {
        module_z_offset = - TPC_Ecal_Hcal_barrel_halfZ+
		Hcal_normal_dim_z/2. + 
		module_id*(Hcal_normal_dim_z+Hcal_modules_gap);

        G4ThreeVector localPos(localXPos,-module_z_offset,localYPos);
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
		theBarrilRegSD->SetModuleZOffset(module_id,module_z_offset);
      }
    }
}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              Endcaps                 ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalRpc01::Endcaps(G4LogicalVolume* MotherLog)
{
  // old parameters from database
  G4double pRMax, pDz, pRMin;
  pRMax = Hcal_endcap_rmax;

  // for the moment the endcap has the same structure as the barrel,
  // so the same thickness.
  pDz = Hcal_endcap_thickness / 2.;
  pRMin = Hcal_endcap_center_box_size / 2.;
  
  G4double zPlane[2];
  zPlane[0]=-pDz;
  zPlane[1]=-zPlane[0];

  G4double rInner[2],rOuter[2];
  rInner[0]=rInner[1]=0.;
  rOuter[0]=rOuter[1]=pRMax;

#ifdef MOKKA_GEAR
  // Write parameters in helper class
  // attention: the outer symmetrie is 32-fold... this is not
  // taken into account in gear
  
  // the outer radius is in Geant4 defined as the tangent outer radius.
  // In Gear description differs. We will take the heigth of the triangle
  // in account as well in order to get the max radius.

  helpEndcap.outerRadius = rOuter[0] / cos(pi/8.);
  helpEndcap.innerRadius = Hcal_endcap_center_box_size / 2. ;
  helpEndcap.phi0 = 0. ;                             // phi0
  
  // set starting value for inner_z
  // it should _not_ stay at this value
  helpEndcap.leastZ = 999999999. ;
#endif

  G4Polyhedra *EndCapSolidPoly=
    new G4Polyhedra("HcalEndCapSolidPoly",
		    0.,
		    360.,
		    8,
		    2,
		    zPlane,
		    rInner,
		    rOuter);
		    
  G4Box * hcalECInnerHole = 
    new G4Box("HcalEndCapHole",
	Hcal_endcap_center_box_size / 2.,
	Hcal_endcap_center_box_size / 2.,
	Hcal_total_dim_y);

  rotBox=new G4RotationMatrix();
  rotBox->rotateZ(-pi/8.);
  G4VSolid *solidHCalEC=new G4SubtractionSolid("HcalEndCapPoly-Box",EndCapSolidPoly,hcalECInnerHole,rotBox,G4ThreeVector(0,0,0)); 

  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(.8,.8,.2));
  VisAtt->SetForceWireframe(true);
  VisAtt->SetDaughtersInvisible(true);
  //VisAtt->SetForceSolid(true);

  G4LogicalVolume* EndCapLogical =
    new G4LogicalVolume(solidHCalEC,
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
  G4double endcap_z_offset = Hcal_start_z + Hcal_endcap_thickness/2. ;
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
		      "HcalEndCapPhys",
		      MotherLog,
		      false,
		      ModuleNumber);
      rotEffect=new G4RotationMatrix();
      rotEffect->rotateZ(-pi/8.);
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

void SHcalRpc01::EndcapChambers(G4LogicalVolume* MotherLog,
			     SDHcalECSD01* theSD, bool rings)
{

  G4double phiStart = 0.;
  G4double phiTotal = 360.;
  G4int numSide = 8;
  G4int numZPlanes = 2;
  
  G4double zPlane[2];
  G4double rInner[2],rOuter[2];

  G4double pRMax, pDz, pRMin;

  G4UserLimits* pULimits=
    new G4UserLimits(theMaxStepAllowed);
  
  G4LogicalVolume* EndCapChamberLogical=0;

  G4int number_of_chambers = Hcal_nlayers;
  G4int possible_number_of_chambers = Hcal_nlayers;

  G4double zValue = 0;
  if(rings) 
  {
  G4Polyhedra *motherSolid =
    (G4Polyhedra*) MotherLog->GetSolid();

  G4PolyhedraHistorical* motherPolyhedraParameters =
    motherSolid->GetOriginalParameters();

  pRMax = (*(motherPolyhedraParameters->Rmax)) * cos(pi/8.)
    - Hcal_lateral_plate_thickness;
  pDz = Hcal_chamber_tickness / 2.;
  pRMin = (*(motherPolyhedraParameters->Rmin)) * cos(pi/8.)
    + Hcal_lateral_plate_thickness;

  // G4Polyhedra Envelope parameters
  numSide = motherPolyhedraParameters->numSide;

  zPlane[0]=-pDz;
  zPlane[1]=-zPlane[0];

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

  EndCapChamberLogical =
          BuildRPC2Polyhedra(EndCapChamberSolid,
                             theSD, rings,
                             phiStart,
                             phiTotal,
                             numSide,
                             numZPlanes,
                             zPlane,
                             rInner,
                             rOuter,
                             pULimits);

    G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(1.,1.,1.));
    EndCapChamberLogical->SetVisAttributes(VisAtt);

    possible_number_of_chambers = (G4int)
    floor(2*abs(*(motherPolyhedraParameters->Z_values)) /
          (Hcal_chamber_tickness + Hcal_radiator_thickness));

    zValue = *(motherPolyhedraParameters->Z_values);

  }//end if rings == true
  else 
  {
    // Chambers in the SHcalRpc01::Endcaps
    // standard endcap chamber solid:
    pRMax = Hcal_endcap_rmax - Hcal_lateral_plate_thickness;
    pDz = Hcal_chamber_tickness / 2.;
  
    // G4Polyhedra Envelope parameters
    zPlane[0]=-pDz;
    zPlane[1]=-zPlane[0];

    rInner[0]=rInner[1]=0.;
    rOuter[0]=rOuter[1]=pRMax;

    G4Polyhedra *EndCapChamberPoly=
      new G4Polyhedra("EndCapChamberSolid",
		    phiStart,
		    phiTotal,
		    numSide,
		    numZPlanes,
		    zPlane,
		    rInner,
		    rOuter);
  
    hcalECChamberInnerHole = 
      new G4Box("HcalEndCapChamberHole",
	Hcal_endcap_center_box_size / 2.  + Hcal_lateral_plate_thickness,
	Hcal_endcap_center_box_size / 2.  + Hcal_lateral_plate_thickness,
	Hcal_total_dim_y);

    G4VSolid *EndCapChamberSolid=new G4SubtractionSolid("HcalEndCapChPoly-Box",EndCapChamberPoly,hcalECChamberInnerHole,rotBox,G4ThreeVector(0,0,0)); 

  // standard endcap chamber logical
  
	EndCapChamberLogical =
	  BuildRPC2Polyhedra(EndCapChamberSolid,
			     theSD, rings,
			     phiStart,
			     phiTotal,
			     numSide,
			     numZPlanes,
			     zPlane,
			     rInner,
			     rOuter,
			     pULimits);
  
    G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(1.,1.,1.));
    VisAtt->SetDaughtersInvisible(true);
    EndCapChamberLogical->SetVisAttributes(VisAtt);

    possible_number_of_chambers = (G4int) (floor(abs(Hcal_endcap_thickness-
		Hcal_back_plate_thickness) /
	  (Hcal_chamber_tickness + Hcal_radiator_thickness)));

    zValue = Hcal_endcap_thickness/2.;
  }//end else if rings == false
  
  if(possible_number_of_chambers < number_of_chambers)
    number_of_chambers = possible_number_of_chambers;

  // chamber placements
  for (G4int layer_id = 1;
       layer_id <= number_of_chambers;
       layer_id++)
    {
      G4double Zoff = - abs(zValue)
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
		      layer_id
#ifdef MOKKA_DEBUG
,true);
#else
);  
#endif

      theSD->
	AddLayer(layer_id,
		 0,
		 0,
		 Zoff+ECR_RPC_GapPosX);
      
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
SHcalRpc01::BuildRPC2Box(G4Box* ChamberSolid, 
		       SDHcalSD01* theSD, 
		       G4int layerId,
		       G4UserLimits* pULimits)
{
  if(ChamberSolid->GetEntityType()=="G4Box")
    {
      //ChamberSolid->GetXHalfLength()

      //parameters of the chamber
      G4double RPC_Length                   =2*(ChamberSolid->GetZHalfLength());
      G4double RPC_Width                    =2*(ChamberSolid->GetYHalfLength());//Parameter that changes
      G4double RPC_Thickness                =2*(ChamberSolid->GetXHalfLength());
/*
      G4double RPC_Free_Thickness           =RPC_Thickness-RPC_ChipPackageThickness-RPC_PCB_Thickness-RPC_mylar_ThicknessAnode-RPC_mylar_ThicknessCathode-RPC_Graphite_ThicknessAnode-RPC_Graphite_ThicknessCathode-RPC_ThinGlass-RPC_ThickGlass-RPC_Gap_Thickness;
*/
  
      G4double RPCChipPackageX   =RPC_Thickness/2   -RPC_ChipPackageThickness/2;
      G4double RPC_PCBPosX       =RPCChipPackageX   -RPC_ChipPackageThickness/2     -RPC_PCB_Thickness/2;
      G4double RPC_mylarPosX     =RPC_PCBPosX       -RPC_PCB_Thickness/2            -RPC_mylar_ThicknessAnode/2;
      G4double RPC_GraphitePosX  =RPC_mylarPosX     -RPC_mylar_ThicknessAnode/2     -RPC_Graphite_ThicknessAnode/2;
      G4double RPC_ThinGlassPosX =RPC_GraphitePosX  -RPC_Graphite_ThicknessAnode/2  -RPC_ThinGlass/2;
      RPC_GapPosX       =RPC_ThinGlassPosX -RPC_ThinGlass/2                -RPC_Gap_Thickness/2;
      G4double RPC_ThickGlassPosX=RPC_GapPosX       -RPC_Gap_Thickness/2            -RPC_ThickGlass/2;
      G4double RPC_GraphitePosX2 =RPC_ThickGlassPosX-RPC_ThickGlass/2               -RPC_Graphite_ThicknessCathode/2;
      G4double RPC_mylarPosX2    =RPC_GraphitePosX2 -RPC_Graphite_ThicknessCathode/2-RPC_mylar_ThicknessCathode/2;
      G4double RPC_FreePosX      =RPC_mylarPosX2    -RPC_mylar_ThicknessCathode/2   -RPC_Free_Thickness/2;  
      G4double RPC_GazInlet_In_Y =RPC_Width/2       -RPC_EdgeWidth                  -RPCGazInletOuterRadius;
      G4double RPC_GazInlet_In_Z =RPC_Length/2-RPC_EdgeWidth/2;
      G4double RPC_GazInlet_Out_Y=-RPC_GazInlet_In_Y;
      G4double RPC_GazInlet_Out_Z=RPC_GazInlet_In_Z;
/*
      G4double RPCSpacerLength   =0.75*(RPC_Length-2*RPC_EdgeWidth);
      G4double RPCSpacerYOffset  =(RPC_Width-2*RPC_EdgeWidth)/10;
      G4double RPCSpacerYgap     =(RPC_Width-2*RPC_EdgeWidth)/5;
      G4double RPCSpacerY1       =RPC_Width/2-RPC_EdgeWidth-RPCSpacerYOffset;
      G4double RPCSpacerY2       =RPCSpacerY1-RPCSpacerYgap;
      G4double RPCSpacerY3       =RPCSpacerY2-RPCSpacerYgap;
      G4double RPCSpacerY4       =RPCSpacerY3-RPCSpacerYgap;
      G4double RPCSpacerY5       =RPCSpacerY4-RPCSpacerYgap;
      G4double RPCSpacerZ1       =(RPC_Length-2*RPC_EdgeWidth)/2-RPCSpacerLength/2;
      G4double RPCSpacerZ2       =-RPCSpacerZ1;
*/    

      G4double gap_hZ = (RPC_Length-2*RPC_EdgeWidth)/2.;
      G4double gap_hY = (RPC_Width-2*RPC_EdgeWidth)/2.;

      // fill the Chamber Envelope with vacuum
      G4NistManager *man=G4NistManager::Instance();
      G4Material *Vacuum=man->FindOrBuildMaterial("G4_Galactic");
      G4Material *defaultMaterial=Vacuum;

      G4LogicalVolume *logicRPC =
	new G4LogicalVolume(ChamberSolid,
			    defaultMaterial, 
			    "RPC2", 
			    0, 0, 0);


      //
      //build electronics
      //
      G4Box *solidRPCElectronics = new G4Box("solidRPCElectronics",		//its name
					     RPC_ChipPackageThickness/2,RPC_Width/2,RPC_Length/2);//size
    
      G4LogicalVolume *logicRPCElectronics = new G4LogicalVolume(solidRPCElectronics,    //its solid
								 theG10Material, //its material
								 "logicRPC"); //name
  
      new G4PVPlacement(0,		   //no rotation
			G4ThreeVector(RPCChipPackageX,0,0),  //its position
			logicRPCElectronics,     //its logical volume		    
			"physiRPCElectronics", //its name
			logicRPC,        //its mother
			false,             //no boulean operat
			0);                //copy number

      G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(0.6,0.6,0.9));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      logicRPCElectronics->SetVisAttributes(VisAtt);
  


      //
      //build PCB
      //
      G4Box *solidRPCPCB = new G4Box("solidRPCPCB",		//its name
				     RPC_PCB_Thickness/2,RPC_Width/2,RPC_Length/2);//size
  
      G4LogicalVolume *logicRPCPCB = new G4LogicalVolume(solidRPCPCB,    //its solid
							 theG10Material, //its material
							 "logicRPCPCB"); //name
 
      new G4PVPlacement(0,		   //no rotation
			G4ThreeVector(RPC_PCBPosX,0.,0.),  //its position
			logicRPCPCB,     //its logical volume		    
			"physiRPCPCB", //its name
			logicRPC,        //its mother
			false,             //no boulean operat
			0);                //copy number

      VisAtt = new G4VisAttributes(G4Colour(0.6,0.9,0.6));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      logicRPCPCB->SetVisAttributes(VisAtt);



      //
      //build mylar layers
      //
      G4Box *solidRPCmylarAnode = new G4Box("solidRPCmylarAnode",		//its name
					    RPC_mylar_ThicknessAnode/2,RPC_Width/2,RPC_Length/2);//size
  
      G4LogicalVolume *logicRPCmylarAnode = new G4LogicalVolume(solidRPCmylarAnode,    //its solid
								theMylarMaterial, //its material
								"logicRPCmylarAnode"); //name
 
      new G4PVPlacement(0,		   //no rotation
			G4ThreeVector(RPC_mylarPosX,0.,0.),  //its position
			logicRPCmylarAnode,     //its logical volume		    
			"physiRPCmylar", //its name
			logicRPC,        //its mother
			false,             //no boulean operat
			0);                //copy number



      G4Box *solidRPCmylarCathode = new G4Box("solidRPCmylarCathode",		//its name
					      RPC_mylar_ThicknessCathode/2,RPC_Width/2,RPC_Length/2);//size
  
      G4LogicalVolume *logicRPCmylarCathode = new G4LogicalVolume(solidRPCmylarCathode,    //its solid
								  theMylarMaterial, //its material
								  "logicRPCmylarCathode"); //name
  
      new G4PVPlacement(0,		   //no rotation
			G4ThreeVector(RPC_mylarPosX2,0.,0.),  //its position
			logicRPCmylarCathode,     //its logical volume		    
			"physiRPCmylarCathode", //its name
			logicRPC,        //its mother
			false,             //no boulean operat
			0);                //copy number

      VisAtt = new G4VisAttributes(G4Colour(0.6,0.6,0.9));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      logicRPCmylarAnode->SetVisAttributes(VisAtt);
      logicRPCmylarCathode->SetVisAttributes(VisAtt);



      //
      //build graphite layers
      //
      G4Box *solidRPCGraphiteAnode = new G4Box("solidRPCGraphiteAnode",		//its name
					       RPC_Graphite_ThicknessAnode/2,RPC_Width/2,RPC_Length/2);//size

      G4LogicalVolume *logicRPCGraphiteAnode = new G4LogicalVolume(solidRPCGraphiteAnode,    //its solid
								   theGraphiteMaterial, //its material
								   "logicRPCGraphiteAnode"); //name
  
      new G4PVPlacement(0,		   //no rotation
			G4ThreeVector(RPC_GraphitePosX,0.,0.),  //its position
			logicRPCGraphiteAnode,     //its logical volume		    
			"physiRPCGraphiteAnode", //its name
			logicRPC,        //its mother
			false,             //no boulean operat
			0);                //copy number





      G4Box *solidRPCGraphiteCathode = new G4Box("solidRPCGraphiteCathode",		//its name
						 RPC_Graphite_ThicknessCathode/2,RPC_Width/2,RPC_Length/2);//size
 
      G4LogicalVolume *logicRPCGraphiteCathode = new G4LogicalVolume(solidRPCGraphiteCathode,    //its solid
								     theGraphiteMaterial, //its material
								     "logicRPCGraphiteCathode"); //name
 
      new G4PVPlacement(0,		   //no rotation
			G4ThreeVector(RPC_GraphitePosX2,0.,0.),  //its position
			logicRPCGraphiteCathode,     //its logical volume		    
			"physiRPCGraphiteCathode", //its name
			logicRPC,        //its mother
			false,             //no boulean operat
			0);                //copy number

      VisAtt = new G4VisAttributes(G4Colour(.2,0.2,0.2));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      logicRPCGraphiteAnode->SetVisAttributes(VisAtt);
      logicRPCGraphiteCathode->SetVisAttributes(VisAtt);



      //
      //build glass layers 
      //
      G4Box *solidRPCThinGlass = new G4Box("solidRPCThinGlass",		//its name
					   RPC_ThinGlass/2,RPC_Width/2,RPC_Length/2);//size
  
      G4LogicalVolume *logicRPCThinGlass = new G4LogicalVolume(solidRPCThinGlass,    //its solid
							       CGAGeometryManager::GetMaterial("FloatGlass"), //its material
							       "logicRPCThinGlass"); //name
 
      new G4PVPlacement(0,		   //no rotation
			G4ThreeVector(RPC_ThinGlassPosX,0.,0.),  //its position
			logicRPCThinGlass,     //its logical volume		    
			"physiRPCThinGlass", //its name
			logicRPC,        //its mother
			false,             //no boulean operat
			0);                //copy number


      G4Box *solidRPCThickGlass = new G4Box("solidRPCThickGlass",		//its name
					    RPC_ThickGlass/2,RPC_Width/2,RPC_Length/2);//size

  
      G4LogicalVolume *logicRPCThickGlass = new G4LogicalVolume(solidRPCThickGlass,    //its solid
								CGAGeometryManager::GetMaterial("FloatGlass"), //its material
								"logicRPCThickGlass"); //name
 
      new G4PVPlacement(0,		   //no rotation
			G4ThreeVector(RPC_ThickGlassPosX,0.,0.),  //its position
			logicRPCThickGlass,     //its logical volume		    
			"physiRPCThickGlass", //its name
			logicRPC,        //its mother
			false,             //no boulean operat
			0);                //copy number


      VisAtt = new G4VisAttributes(G4Colour(.8,0,.2));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      logicRPCThinGlass->SetVisAttributes(VisAtt);
      logicRPCThickGlass->SetVisAttributes(VisAtt);


      //
      //build edges
      //

      G4VSolid* solidRPCEdge1 = new G4Box("solidRPCEdge1",RPC_Gap_Thickness/2,RPC_Width/2,RPC_Length/2);
      G4VSolid* solidRPCEdge2 = new G4Box("solidRPCEdge2",RPC_Gap_Thickness/2,RPC_Width/2-RPC_EdgeWidth,RPC_Length/2-RPC_EdgeWidth);

      G4VSolid* solidRPCEdge = new G4SubtractionSolid("solidRPCEdge",solidRPCEdge1,solidRPCEdge2,0, G4ThreeVector(0.,0.,0.));


      G4LogicalVolume *logicRPCEdge = new G4LogicalVolume(solidRPCEdge,    //its solid
							  CGAGeometryManager::GetMaterial("PEEK-GF30"), //its material
							  "logicRPCEdge"); //name

      new G4PVPlacement(0,		   //no rotation
			G4ThreeVector(RPC_GapPosX,0.,0.),  //its position
			logicRPCEdge,     //its logical volume		    
			"physiRPCEdge", //its name
			logicRPC,        //its mother
			false,             //no boulean operat
			0);                //copy number

      VisAtt = new G4VisAttributes(G4Colour(0.2,0.4,0.6));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      logicRPCEdge->SetVisAttributes(VisAtt);

      //
      //build gap layer
      //
      G4Box *solidRPCGap = new G4Box("solidRPCGap",		//its name
				     RPC_Gap_Thickness/2,RPC_Width/2-RPC_EdgeWidth,RPC_Length/2-RPC_EdgeWidth);//size

  
      G4LogicalVolume *logicRPCGap = new G4LogicalVolume(solidRPCGap,    //its solid
							 CGAGeometryManager::GetMaterial("RPCGAS2"), //its material
							 "logicRPCGap",//name
							 0,
							 0,
							 pULimits); 
 
      new G4PVPlacement(0,		   //no rotation
			G4ThreeVector(RPC_GapPosX,0.,0.),  //its position
			logicRPCGap,     //its logical volume		    
			"physiRPCGap", //its name
			logicRPC,        //its mother
			false,             //no boulean operat
			layerId);                //copy number

      // PLugs the sensitive detector HERE!
      logicRPCGap->SetSensitiveDetector(theSD);

      VisAtt = new G4VisAttributes(G4Colour(0.1,0,0.8));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      logicRPCGap->SetVisAttributes(VisAtt);
  

      //
      //build spacers: fishing line
      //
      G4VSolid *solidRPCSpacer=new G4Tubs("tube",0,Hcal_spacer_thickness/2,
	RPC_Gap_Thickness/2 - 
		G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
	0,2*pi);

      G4LogicalVolume *logicRPCSpacer = new G4LogicalVolume(solidRPCSpacer,    //its solid
							    CGAGeometryManager::GetMaterial("nylon"), //its material
							    "logicRPCSpacer"); //name


      G4RotationMatrix* rotSpacer = new G4RotationMatrix;
	rotSpacer->rotateY(pi/2.);

     G4int y_number_of_separations = (G4int)(2*gap_hY/Hcal_spacer_separation);
     G4int z_number_of_separations = (G4int)(2*gap_hZ/Hcal_spacer_separation);
     G4double y_lateral_space = (2*gap_hY - 
	y_number_of_separations*Hcal_spacer_separation)/2;
     G4double z_lateral_space = (2*gap_hZ - 
	z_number_of_separations*Hcal_spacer_separation)/2;

     if(y_lateral_space < Hcal_spacer_thickness/2.)
     {
	y_number_of_separations = (G4int)((2*gap_hY-Hcal_spacer_thickness)
				/Hcal_spacer_separation);
        y_lateral_space = (2*gap_hY - 
	       y_number_of_separations*Hcal_spacer_separation)/2;

     }

     if(z_lateral_space < Hcal_spacer_thickness/2.)
     {
	z_number_of_separations = (G4int)((2*gap_hZ-Hcal_spacer_thickness)
				/Hcal_spacer_separation);
        z_lateral_space = (2*gap_hZ - 
	       z_number_of_separations*Hcal_spacer_separation)/2;

     }

     for(G4int y_counter = 0; y_counter <=y_number_of_separations; y_counter++) 
     {
       G4double SpacerY = 
		gap_hY - y_lateral_space - y_counter*Hcal_spacer_separation;

       for(G4int z_counter = 0; z_counter <=z_number_of_separations; 
	z_counter++) 
       {
	G4double SpacerZ = 
		gap_hZ - z_lateral_space - z_counter*Hcal_spacer_separation;

	new G4PVPlacement(rotSpacer,
			G4ThreeVector(0,SpacerY,SpacerZ),  //its position
			logicRPCSpacer,     //its logical volume		    
			"physiRPCSpacer1", //its name
			logicRPCGap,        //its mother
			false,             //no boulean operat
			0
#ifdef HCAL_DEBUG
,true);
#else
);
#endif

      }
     }
/*
      new G4PVPlacement(0,		   //no rotation
			G4ThreeVector(0,RPCSpacerY1,RPCSpacerZ1),  //its position
			logicRPCSpacer,     //its logical volume		    
			"physiRPCSpacer1", //its name
			logicRPCGap,        //its mother
			false,             //no boulean operat
			0);                //copy number

      new G4PVPlacement(0,		   //no rotation
			G4ThreeVector(0,RPCSpacerY2,RPCSpacerZ2),  //its position
			logicRPCSpacer,     //its logical volume		    
			"physiRPCSpacer2", //its name
			logicRPCGap,        //its mother
			false,             //no boulean operat
			0);                //copy number

      new G4PVPlacement(0,		   //no rotation
			G4ThreeVector(0,RPCSpacerY3,RPCSpacerZ1),  //its position
			logicRPCSpacer,     //its logical volume		    
			"physiRPCSpacer3", //its name
			logicRPCGap,        //its mother
			false,             //no boulean operat
			0);                //copy number

      new G4PVPlacement(0,		   //no rotation
			G4ThreeVector(0,RPCSpacerY4,RPCSpacerZ2),  //its position
			logicRPCSpacer,     //its logical volume		    
			"physiRPCSpacer4", //its name
			logicRPCGap,        //its mother
			false,             //no boulean operat
			0);                //copy number

      new G4PVPlacement(0,		   //no rotation
			G4ThreeVector(0,RPCSpacerY5,RPCSpacerZ1),  //its position
			logicRPCSpacer,     //its logical volume		    
			"physiRPCSpacer5", //its name
			logicRPCGap,        //its mother
			false,             //no boulean operat
			0);                //copy number

*/

      VisAtt = new G4VisAttributes(G4Colour(1,1.1));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      logicRPCSpacer->SetVisAttributes(VisAtt);
 

      //
      //build gaz outlets: capillary 
      //
      G4VSolid *solidRPCGazInlet=new G4Tubs("tube",RPCGazInletInnerRadius,RPCGazInletOuterRadius,RPCGazInletLength/2,0,2*pi);

      G4LogicalVolume *logicRPCGazInlet = new G4LogicalVolume(solidRPCGazInlet,    //its solid
							      CGAGeometryManager::GetMaterial("PEEK-GF30"), //its material
							      "logicRPCGazInlet"); //name

      new G4PVPlacement(0,		   //no rotation
			G4ThreeVector(0,RPC_GazInlet_In_Y,RPC_GazInlet_In_Z),  //its position
			logicRPCGazInlet,     //its logical volume		    
			"logicRPCGazInletIn", //its name
			logicRPCEdge,        //its mother
			false,             //no boulean operat
			0);                //copy number


      new G4PVPlacement(0, 		   //no rotation
			G4ThreeVector(0,RPC_GazInlet_Out_Y,RPC_GazInlet_Out_Z),  //its position
			logicRPCGazInlet,     //its logical volume		    
			"logicRPCGazInletOut", //its name
			logicRPCEdge,        //its mother
			false,             //no boulean operat
			0);                //copy number



  
      G4VSolid *solidRPCGazInsideInlet=new G4Tubs("tube",0,RPCGazInletInnerRadius,RPCGazInletLength/2,0,2*pi);

      G4LogicalVolume *logicRPCGazInsideInlet = new G4LogicalVolume(solidRPCGazInsideInlet,    //its solid
								    CGAGeometryManager::GetMaterial("RPCGAS2"), //its material
								    "logicRPCGazInlet"); //name

      new G4PVPlacement(0,		   //no rotation
			G4ThreeVector(0,RPC_GazInlet_In_Y,RPC_GazInlet_In_Z),  //its position
			logicRPCGazInsideInlet,     //its logical volume		    
			"physiRPCGazInsideInlet", //its name
			logicRPCEdge,        //its mother
			false,             //no boulean operat
			0);                //copy number

      new G4PVPlacement(0,		   //no rotation
			G4ThreeVector(0,RPC_GazInlet_Out_Y,RPC_GazInlet_Out_Z),  //its position
			logicRPCGazInsideInlet,     //its logical volume		    
			"physiRPCGazInsideOutlet", //its name
			logicRPCEdge,        //its mother
			false,             //no boulean operat
			0);                //copy number


      VisAtt = new G4VisAttributes(G4Colour(0.2,0.4,0.6));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      logicRPCGazInlet->SetVisAttributes(VisAtt);
  
      VisAtt = new G4VisAttributes(G4Colour(0.1,0,0.8));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      logicRPCGazInsideInlet->SetVisAttributes(VisAtt);
  

      //
      //build free space 
      //
  
      G4Box *solidRPCFree = new G4Box("solidRPCFree",		//its name
				      RPC_Free_Thickness/2,RPC_Width/2,RPC_Length/2);//size
      
      
      G4LogicalVolume *logicRPCFree = new G4LogicalVolume(solidRPCFree,    //its solid
							  theAirMaterial, //its material
							  "logicRPCFree"); //name
      
      new G4PVPlacement(0,		   //no rotation
			G4ThreeVector(RPC_FreePosX,0.,0.),  //its position
			logicRPCFree,     //its logical volume		    
			"physiRPCFree", //its name
			logicRPC,        //its mother
			false,             //no boulean operat
			0 //copy number
#ifdef MOKKA_DEBUG
,true);
#else
);  
#endif

      VisAtt = new G4VisAttributes(G4Colour(0.6,0.4,0.6));
      logicRPCFree->SetVisAttributes(VisAtt);

      return logicRPC;
    }

  Control::Abort("SHcalRpc01::BuildRPC2: invalid ChamberSolidEnvelope",
	MOKKA_OTHER_ERRORS);
  return NULL;
}

G4LogicalVolume * 
SHcalRpc01::BuildRPC2Polyhedra(G4VSolid* ChamberSolid, 
			   SDHcalECSD01* theSD,
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
   if(rings && (ChamberSolid->GetEntityType()!="G4Polyhedra"))
	 Control::Abort("SHcalRpc01::BuildRPC2Polyhedra: invalid ChamberSolidEnvelope for the Hcal Rings", MOKKA_OTHER_ERRORS);

      // fill the Chamber Envelope with mixture
      G4LogicalVolume *ChamberLog =
	new G4LogicalVolume(ChamberSolid,
			    theRPC2ECRMixture,
			    "RPC2ECREnveloppe", 
			    0, 0, 0);
      //
      // build the RPC thick glass 
      
      G4double NewZPlane[2];
      NewZPlane[0] = RPC_ThickGlass/2.;
      NewZPlane[1] = -NewZPlane[0];

  G4VSolid * ThickGlassSolid = 0;
  if(rings)
  {
      ThickGlassSolid =
        new G4Polyhedra("RPC2RingThickGlass",
                        phiStart,
                        phiTotal,
                        numSide,
                        numZPlanes,
                        NewZPlane,
                        rInner,
                        rOuter);
  }//end if rings == true
  else
  {
      G4Polyhedra * ThickGlassSolidPoly =
	new G4Polyhedra("RPC2ECThickGlassPoly", 
			phiStart,
			phiTotal,
			numSide,
			numZPlanes,
			NewZPlane,
			rInner,
			rOuter);	
      
      ThickGlassSolid=
		new G4SubtractionSolid("RPC2ECThickGlassPoly-Box",
					ThickGlassSolidPoly,
					hcalECChamberInnerHole,
					rotBox,G4ThreeVector(0,0,0)); 

   }// end else if rings == false

      G4LogicalVolume *ThickGlassLogical =
	new G4LogicalVolume(ThickGlassSolid,
			    CGAGeometryManager::GetMaterial("FloatGlass"),
			    "RPC2ECRThickGlass", 
			    0, 0, 0);
      
      G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(.8,0,.2));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      ThickGlassLogical->SetVisAttributes(VisAtt);

      //
      // build the RPC thin glass 
      
      NewZPlane[0] = RPC_ThinGlass/2.;
      NewZPlane[1] = -NewZPlane[0];

  G4VSolid * ThinGlassSolid = 0;
  if(rings)
  {
      ThinGlassSolid =
        new G4Polyhedra("RPC2RingThinGlass",
                        phiStart,
                        phiTotal,
                        numSide,
                        numZPlanes,
                        NewZPlane,
                        rInner,
                        rOuter);
  }//end if rings == true
  else
  {
      G4Polyhedra * ThinGlassSolidPoly =
	new G4Polyhedra("RPC2ECThinGlassPoly", 
			phiStart,
			phiTotal,
			numSide,
			numZPlanes,
			NewZPlane,
			rInner,
			rOuter);	
      
      ThinGlassSolid=
		new G4SubtractionSolid("RPC2ECThinGlassPoly-Box",
					ThinGlassSolidPoly,
					hcalECChamberInnerHole,
					rotBox,G4ThreeVector(0,0,0)); 

   }// end else if rings == false

      G4LogicalVolume *ThinGlassLogical =
	new G4LogicalVolume(ThinGlassSolid,
			    CGAGeometryManager::GetMaterial("FloatGlass"),
			    "RPC2ECRThinGlass", 
			    0, 0, 0);
      
      VisAtt = new G4VisAttributes(G4Colour(.8,0,.2));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      ThinGlassLogical->SetVisAttributes(VisAtt);

      //
      // build the gas gap
      //
      NewZPlane[0] = RPC_Gap_Thickness/2.;
      NewZPlane[1] = -NewZPlane[0];

   G4VSolid *GasSolid=0;
   if(rings)
   {
      GasSolid =
        new G4Polyhedra("RPC2GasRing",
                        phiStart,
                        phiTotal,
                        numSide,
                        numZPlanes,
                        NewZPlane,
                        rInner,
                        rOuter);
   }// end if rings == true
   else
   {
      G4Polyhedra * GasSolidPoly =
	new G4Polyhedra("RPC2GasECPoly", 
			phiStart,
			phiTotal,
			numSide,
			numZPlanes,
			NewZPlane,
			rInner,
			rOuter);
      
      GasSolid=
			new G4SubtractionSolid("RPC2GasECPoly-Box",
						GasSolidPoly,
						hcalECChamberInnerHole,
						rotBox,G4ThreeVector(0,0,0)); 
    }// end else if rings == false

      G4LogicalVolume *GasLogical =
	new G4LogicalVolume(GasSolid,
			    CGAGeometryManager::GetMaterial("RPCGAS2"),
			    "RPC2gasECR", 
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
	helpEndcapRing.sensThickness.push_back( RPC_Gap_Thickness ) ;
	helpEndcapRing.gapThickness.push_back( 0.6 ) ;
      }
      else{
	helpEndcap.sensThickness.push_back( RPC_Gap_Thickness ) ;
	helpEndcap.gapThickness.push_back( 0.6 ) ;
      }
#endif
      
      // placing the all. 
      // ZOffset starts pointing to the chamber border.
      G4double ZOffset = zPlane[0];
      
      // first glass 
      ZOffset += RPC_ThickGlass/2.;
      new G4PVPlacement(0,
		      G4ThreeVector(0,
				    0,
				    ZOffset),
		      ThickGlassLogical,
		      "RPCECRThickGlass",
		      ChamberLog,
		      false,
		      0);
      
      // set ZOffset to the next first glass border
      ZOffset += RPC_ThickGlass/2.;
      
      // gas gap placing 
      ZOffset += RPC_Gap_Thickness/2.; // center !
      ECR_RPC_GapPosX = ZOffset;
      new G4PVPlacement(0,
		      G4ThreeVector(0,
				    0,
				    ZOffset),
		      GasLogical,
		      "RPC2GasECRPhys",
		      ChamberLog,
		      false,
		      0);

      // set ZOffset to the next gas gap border
      ZOffset += RPC_Gap_Thickness/2.;
      
      // second glass, after the first glass
      // and the gas gap.
      ZOffset += RPC_ThinGlass/2.; // center !
      new G4PVPlacement(0,
		      G4ThreeVector(0,
				    0,
				    ZOffset),
		      ThinGlassLogical,
		      "RPC2ECRThinGlass",
		      ChamberLog,
		      false,
		      0);
      return ChamberLog;      
}

void 
SHcalRpc01::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
  // In this complex subdetector the best is to read
  // all hits in the one of the sensitive detectors,
  // just for visualisation, so just for your eyes...
  theBarrilRegSD->LoadEvent(theSubDetectorEventHitsFileInput);
}

G4bool 
SHcalRpc01::Setup(const CGAGeometryEnvironment &theGeometryEnvironment)
{
  Hcal_radiator_thickness =
    theGeometryEnvironment.GetParameterAsDouble("Hcal_radiator_thickness");
  Hcal_radiator_material =
    theGeometryEnvironment.GetParameterAsString("Hcal_radiator_material");
  Hcal_ring = 
    theGeometryEnvironment.GetParameterAsInt("Hcal_ring");
  Hcal_radial_ring_inner_gap =
    theGeometryEnvironment.GetParameterAsInt("Hcal_radial_ring_inner_gap");
  Hcal_stave_gaps = 
    theGeometryEnvironment.GetParameterAsDouble("Hcal_stave_gaps");
  Hcal_modules_gap = 
    theGeometryEnvironment.GetParameterAsDouble("Hcal_modules_gap");
  Hcal_nlayers =
    theGeometryEnvironment.GetParameterAsInt("Hcal_nlayers");
  Hcal_barrel_number_modules =
    theGeometryEnvironment.GetParameterAsInt("Hcal_barrel_number_modules");
  Hcal_chamber_tickness = 
    theGeometryEnvironment.GetParameterAsDouble("Hcal_chamber_thickness");

  Ecal_endcap_zmin =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_endcap_zmin");

  Ecal_endcap_outer_radius =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_endcap_outer_radius");

  hPrime =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_outer_radius")
    + theGeometryEnvironment.GetParameterAsDouble("Hcal_Ecal_gap");
  Hcal_inner_radius = hPrime / cos(pi/8.);

  Hcal_endcap_cables_gap =
    theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_cables_gap");
  Hcal_endcap_ecal_gap =
    theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_ecal_gap");

  TPC_Ecal_Hcal_barrel_halfZ = 
    theGeometryEnvironment.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");

  Hcal_normal_dim_z = (2 * TPC_Ecal_Hcal_barrel_halfZ -
	(Hcal_barrel_number_modules-1)*Hcal_modules_gap)
	/Hcal_barrel_number_modules;
      
  //Hcal_top_end_dim_z = 1180.0000;
      
  Hcal_start_z = TPC_Ecal_Hcal_barrel_halfZ + Hcal_endcap_cables_gap;

/* This was true for only 2 modules along Z in Barrel:
  Hcal_start_z =  Hcal_normal_dim_z 
	+ Hcal_modules_gap / 2. + Hcal_endcap_cables_gap;
*/
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

  // The cell boundaries does not really exist as G4 volumes. So,
  // to avoid long steps over running  several cells, the 
  // theMaxStepAllowed inside the sensitive material is the
  // pad smaller x or z dimension.
  theMaxStepAllowed =
    theGeometryEnvironment.GetParameterAsDouble("DHcal_max_step");

  Hcal_cells_size = 
    theGeometryEnvironment.GetParameterAsDouble("Hcal_cells_size");
/*
  Hcal_digi_cells_size = 
    theGeometryEnvironment.GetParameterAsDouble("Hcal_digitization_tile_size");
*/

  Hcal_endcap_center_box_size =
    theGeometryEnvironment.
    GetParameterAsDouble("Hcal_endcap_center_box_size");

  RPC_PCB_Thickness            = theGeometryEnvironment.
			GetParameterAsDouble
				("PCB_Thickness");
  RPC_mylar_ThicknessAnode     = theGeometryEnvironment.
			GetParameterAsDouble
				("mylar_ThicknessAnode");
  RPC_mylar_ThicknessCathode   = theGeometryEnvironment.
			GetParameterAsDouble
				("mylar_ThicknessCathode");
  RPC_Graphite_ThicknessAnode  = theGeometryEnvironment.
			GetParameterAsDouble
				("Graphite_ThicknessAnode");
  RPC_Graphite_ThicknessCathode= theGeometryEnvironment.
			GetParameterAsDouble
				("Graphite_ThicknessCathode");
  RPC_ThinGlass                = theGeometryEnvironment.
			GetParameterAsDouble
				("ThinGlass");
  RPC_Gap_Thickness            = theGeometryEnvironment.
			GetParameterAsDouble
				("Gap_Thickness");
  RPC_ThickGlass               = theGeometryEnvironment.
			GetParameterAsDouble
				("ThickGlass");
  RPC_EdgeWidth                = theGeometryEnvironment.
			GetParameterAsDouble
				("EdgeWidth");
  RPCGazInletInnerRadius       = theGeometryEnvironment.
			GetParameterAsDouble
				("GazInletInnerRadius");
  RPCGazInletOuterRadius       = theGeometryEnvironment.
			GetParameterAsDouble
				("GazInletOuterRadius");
  RPCGazInletLength            = theGeometryEnvironment.
			GetParameterAsDouble
				("GazInletLength");
  RPC_ChipPackageThickness     = theGeometryEnvironment.
			GetParameterAsDouble
				("ChipPackageThickness");
  RPC_PadSeparation     = theGeometryEnvironment.
			GetParameterAsDouble
				("PadSeparation");

  Hcal_spacer_thickness     = theGeometryEnvironment.
			GetParameterAsDouble("Hcal_spacer_thickness");

  Hcal_spacer_separation     = theGeometryEnvironment.
			GetParameterAsDouble("Hcal_spacer_separation");

  G4double MinNumCellsInTransvPlane     = theGeometryEnvironment.
			GetParameterAsDouble
				("Hcal_MinNumCellsInTransvPlane");

  // general calculated parameters
  G4double AngleRatio=0.76536686;//"k"
  d_InnerOctoSize=AngleRatio*Hcal_inner_radius;//"d"
  layerThickness = Hcal_radiator_thickness +Hcal_chamber_tickness;
  G4double LMin = 2*RPC_EdgeWidth+Hcal_cells_size*MinNumCellsInTransvPlane+
			(MinNumCellsInTransvPlane+1)*RPC_PadSeparation;

  G4double Ynl = 0.5*d_InnerOctoSize - Hcal_nlayers*layerThickness;
  Hcal_outer_radius = sqrt((LMin-Ynl)*(LMin-Ynl) +
		(hPrime + Hcal_nlayers*layerThickness)*
		(hPrime + Hcal_nlayers*layerThickness));

  Hcal_total_dim_y = Hcal_outer_radius - hPrime;

  Hcal_back_plate_thickness = theGeometryEnvironment.GetParameterAsDouble
				("Hcal_back_plate_thickness");

  Hcal_endcap_thickness = Hcal_nlayers*layerThickness +
			Hcal_back_plate_thickness;

  //Hcal_module_radius = Hcal_outer_radius;

  Hcal_endcap_rmax = Hcal_outer_radius * cos(pi/8.);
  
  // the  y_dim1_for_z kept as the original value in TDR
  Hcal_regular_chamber_dim_z = 
    Hcal_normal_dim_z - 2 *(Hcal_lateral_plate_thickness);
  Hcal_cell_dim_x = Hcal_cells_size;
  G4int N_cells_z =  static_cast <G4int> (
	(Hcal_regular_chamber_dim_z - 2*RPC_EdgeWidth - RPC_PadSeparation) /
        (Hcal_cell_dim_x + RPC_PadSeparation)
			   		 );
  Hcal_cell_dim_z = Hcal_cells_size;
//  Hcal_cell_dim_z=(Hcal_regular_chamber_dim_z-RPC_PadSeparation )/N_cells_z  
//			- RPC_PadSeparation;

#ifdef MOKKA_GEAR
  MokkaGear* mokkaGearMgr = MokkaGear::getMgr() ;

  mokkaGearMgr->tmpParam.setIntVal(  "N_cells_z" , N_cells_z);

  mokkaGearMgr->tmpParam.setDoubleVal(  "TPC_Ecal_Hcal_barrel_halfZ" , theGeometryEnvironment.GetParameterAsDouble(  "TPC_Ecal_Hcal_barrel_halfZ" ) ) ;

  mokkaGearMgr->tmpParam.setDoubleVal( "Hcal_lateral_structure_thickness" , theGeometryEnvironment.GetParameterAsDouble( "Hcal_lateral_structure_thickness" ) ) ;			      

  mokkaGearMgr->tmpParam.setDoubleVal( "Hcal_modules_gap" , theGeometryEnvironment.GetParameterAsDouble( "Hcal_modules_gap" ) ) ;

  mokkaGearMgr->tmpParam.setDoubleVal( "Hcal_stave_gaps" , theGeometryEnvironment.GetParameterAsDouble( "Hcal_stave_gaps" ) ) ;

#endif

      RPC_Free_Thickness           =Hcal_chamber_tickness-RPC_ChipPackageThickness-RPC_PCB_Thickness-RPC_mylar_ThicknessAnode-RPC_mylar_ThicknessCathode-RPC_Graphite_ThicknessAnode-RPC_Graphite_ThicknessCathode-RPC_ThinGlass-RPC_ThickGlass-RPC_Gap_Thickness;

  return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              EndcapRings                 ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SHcalRpc01::EndcapRings(G4LogicalVolume* MotherLog)
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

  helpEndcapRing.outerRadius = rOuter[0] / cos(pi/8.);
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
      new G4PVPlacement(rotEffect,
		      G4ThreeVector(0.,
				    0.,
				    Z1),
		      EndCapLogical,
		      "HcalEndCapRingPhys",
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
SHcalRpc01::PostConstructAction(CGAGeometryEnvironment& )
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
