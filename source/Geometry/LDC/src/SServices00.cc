//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for ILC   -      *
//*                                                     *
//* For more information about Mokka, visit the         *
//*                                                     *
//*  Mokka.in2p3.fr  Mokka home page.                   *
//*                                                     *
//*******************************************************
//
// $Id: SServices00.cc,v 1.1 2009/06/04 11:40:42 musat Exp $
// $Name: mokka-07-00 $
//
// 
//
// SServices00.cc
//

#include "Control.hh"
#include "MyPlacement.hh"
#include "MySQLWrapper.hh"
#include "SServices00.hh"
#include "CGAGeometryManager.hh"

#include "globals.hh"
#include "G4Box.hh"
#include "G4Polyhedra.hh"
#include "G4SubtractionSolid.hh"
#include "G4IntersectionSolid.hh"
#include "G4Trd.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4PVReplica.hh"
#include "G4Polycone.hh"
#include "G4GeometryTolerance.hh"
    
#ifdef MOKKA_GEAR
#include "gear/CalorimeterParameters.h"
#include "gearimpl/CalorimeterParametersImpl.h"
#include "gearimpl/LayerLayoutImpl.h"
#include "MokkaGear.h"
#endif
    
#include "CGADefs.h"
INSTANTIATE(SServices00)

// #define VERBOSE 1

#define MAX_TPC_RINGS 8

#include <math.h>

G4bool 
SServices00::
ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
		    G4LogicalVolume *theWorld)
{

  G4cout << "\nBuilding SServices00"<< G4endl;
  
  // Initialize the Geant3 interface
  if(Control::DUMPG3) MyPlacement::Init("SERVICES","SServices00");

  TPC_inner_radius =
    aGeometryEnvironment.GetParameterAsDouble("TPC_inner_radius");

  Sit_cables_cylinder_thickness =
    aGeometryEnvironment.GetParameterAsDouble("SIT12_cable_thickness");

  FTD_db_name =
    aGeometryEnvironment.GetParameterAsString("SServices_FTD_db_name");

  TUBE_IPOuterBulge_end_z  =
    aGeometryEnvironment.GetParameterAsDouble("TUBE_IPOuterBulge_end_z");

  TUBE_IPOuterBulge_end_radius  =
    aGeometryEnvironment.GetParameterAsDouble("TUBE_IPOuterBulge_end_radius");

  Sit_cables_disk_thickness =(1+TUBE_IPOuterBulge_end_radius/TPC_inner_radius)*0.5*
    (aGeometryEnvironment.GetParameterAsDouble("SServices_FTD7_cables_thickness"));

  SIT1_Radius = aGeometryEnvironment.GetParameterAsDouble("SIT1_Radius");
  SIT2_Radius = aGeometryEnvironment.GetParameterAsDouble("SIT2_Radius");

  FTD2_cone_thickness =
    aGeometryEnvironment.GetParameterAsDouble("SServices_FTD2_cone_thickness");

  FTD3_cone_thickness =
    aGeometryEnvironment.GetParameterAsDouble("SServices_FTD3_cone_thickness");

  TPC_Ecal_Hcal_barrel_halfZ = 
     aGeometryEnvironment.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");

  TPC_outer_radius = 
     aGeometryEnvironment.GetParameterAsDouble("TPC_outer_radius");

  Ecal_cables_gap =
	aGeometryEnvironment.GetParameterAsDouble("Ecal_cables_gap");

  InnerServicesWidth = aGeometryEnvironment
	.GetParameterAsDouble("HcalServicesModule_InnerServicesWidth");

  RailHeight = 
     aGeometryEnvironment.GetParameterAsDouble("EcalBarrelServices_RailHeight");

  if(RailHeight > aGeometryEnvironment.GetParameterAsDouble("Hcal_Ecal_gap"))
	Control::Abort(
  	        "EcalBarrel services: RailHeight exceeds Hcal_Ecal_gap",
		MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

  G4double Ecal_inner_radius = TPC_outer_radius +
    aGeometryEnvironment.GetParameterAsDouble("Ecal_Tpc_gap");

  Ecal_outer_radius =
    aGeometryEnvironment.GetParameterAsDouble("Ecal_outer_radius");

  module_thickness = Ecal_outer_radius - Ecal_inner_radius;

  // module barrel key parameters
  bottom_dim_x = 2. * tan(pi/8.) * Ecal_inner_radius +
    module_thickness/sin(pi/4.);

  top_dim_x = bottom_dim_x - 2 * module_thickness;

#ifdef VERBOSE
  G4cout << "top_dim_x = " << top_dim_x << G4endl;
#endif

  VisAttAir = new G4VisAttributes(G4Colour(1.0,1.0,1.0));
  VisAttAir->SetForceWireframe(true);
  VisAttAir->SetVisibility(true);
  VisAttAir->SetDaughtersInvisible(true);

  VisAttPE = new G4VisAttributes(G4Colour::Yellow());
  VisAttPE->SetForceWireframe(true);
  VisAttPE->SetVisibility(true);

  VisAttCu = new G4VisAttributes(G4Colour::Red());
  VisAttCu->SetForceWireframe(true);
  VisAttCu->SetVisibility(true);

  VisAttStainlessSteel = 
	new G4VisAttributes(G4Colour::Gray());
  VisAttStainlessSteel->SetForceWireframe(true);
  VisAttStainlessSteel->SetVisibility(true);

  if (!BuildTPCEndplateServices(aGeometryEnvironment,theWorld))  return false;

  if (!BuildEcalBarrelServices(aGeometryEnvironment,theWorld))  return false;

  if (!BuildEcalBarrel_EndCapServices(aGeometryEnvironment,theWorld))  
		return false;

  if (!BuildHcalBarrel_EndCapServices(aGeometryEnvironment,theWorld))  
		return false;

  BuildSitCables(theWorld);

  return true;
}

SServices00::~SServices00() 
{
}  


void SServices00::BuildSitCables(G4LogicalVolume *theWorld) {

//First place the Cylinder

  Database * db = new Database(FTD_db_name.data());

  db->exec("select * from common_parameters;");
  db->getTuple();

  G4double ftd4to7_tpc_radial_gap  = 
        db->fetchDouble("ftd4to7_tpc_radial_gap");

  if(ftd4to7_tpc_radial_gap < Sit_cables_cylinder_thickness)
	Control::Abort("SServices_02_v00: the ftd-tpc radial gap is less than Sit cable thickness",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);

  G4double SitTube_inner_radius = TPC_inner_radius -
	ftd4to7_tpc_radial_gap/2.;

  db->exec("select * from disks where disk_number=3;");
  db->getTuple();

  G4double z_start_3 = TPC_Ecal_Hcal_barrel_halfZ * 
	db->fetchDouble("z_position_ReltoTPCLength");

  G4double petalairthickness_half = 0.5 * ( db->fetchDouble("petal_cp_support_thickness")
                                             + 2.0*(db->fetchDouble("disk_si_thickness")));

  G4double max_half_thickness_disk_3 = db->fetchDouble("petal_support_zoffset") + petalairthickness_half ;

  G4double z_half_len = (TPC_Ecal_Hcal_barrel_halfZ -
	z_start_3) / 2.;

  G4Tubs * SitTubeSolid = 
    new G4Tubs ("SitTubeSolid",
                SitTube_inner_radius,
                SitTube_inner_radius + Sit_cables_cylinder_thickness,
                z_half_len,
                0., 2 * pi);

  G4LogicalVolume* SitTubeLog =
    new G4LogicalVolume(SitTubeSolid,
                       CGAGeometryManager::GetMaterial("aluminium"),
                        "SitTubeLog",
                        0, 0, 0);
  
  G4VisAttributes * VisAttAlu = new G4VisAttributes(G4Colour::Gray());
  VisAttAlu->SetForceWireframe(true);
  VisAttAlu->SetVisibility(true);
  SitTubeLog->SetVisAttributes(VisAttAlu);

  new MyPlacement(0, G4ThreeVector(0, 0, z_start_3 + z_half_len),
                        SitTubeLog,
                        "SitTubePhysical+Z",
                        theWorld,
                        false,
                        0
#ifdef MOKKA_DEBUG
,true);
#else
);
#endif

  new MyPlacement(0, G4ThreeVector(0, 0, -z_start_3 - z_half_len),
                        SitTubeLog,
                        "SitTubePhysical-Z",
                        theWorld,
                        false,
                        0
#ifdef MOKKA_DEBUG
,true);
#else
);
#endif

//Then place the cone

  db->exec("select * from disks where disk_number=2;");
  db->getTuple();

  G4double z_start_2 = TPC_Ecal_Hcal_barrel_halfZ * 
	db->fetchDouble("z_position_ReltoTPCLength");

  petalairthickness_half = 0.5 * ( db->fetchDouble("petal_cp_support_thickness")
                                             + 2.0*(db->fetchDouble("disk_si_thickness")));

  G4double max_half_thickness_disk_2 = db->fetchDouble("petal_support_zoffset") + petalairthickness_half ;

  db->exec("select * from common_parameters;");
  db->getTuple();

  G4double FTD2_outer_radius = SIT1_Radius + 
	db->fetchDouble("ftd2_sit1_radial_diff");

  G4double FTD3_outer_radius = SIT2_Radius +
	db->fetchDouble("ftd3_sit2_radial_diff");

  G4double zPlane[2];
  zPlane[0] = z_start_2 + max_half_thickness_disk_2 + 
	G4GeometryTolerance::GetInstance()->GetSurfaceTolerance();

  zPlane[1] = z_start_3 - max_half_thickness_disk_3 -
	G4GeometryTolerance::GetInstance()->GetSurfaceTolerance();

  G4double rInner[2];
  rInner[0] = FTD2_outer_radius +
	G4GeometryTolerance::GetInstance()->GetSurfaceTolerance();
  rInner[1] = FTD3_outer_radius +
	G4GeometryTolerance::GetInstance()->GetSurfaceTolerance();

  G4double rOuter[2];
  rOuter[0] = FTD2_outer_radius + FTD2_cone_thickness;
  rOuter[1] = FTD3_outer_radius + FTD3_cone_thickness;

  G4Polycone * SitConeSolid = new G4Polycone("SitConeSolid",
		0., 2 * pi, 2, zPlane, rInner, rOuter);
		
  G4LogicalVolume* SitConeLog =
    new G4LogicalVolume(SitConeSolid,
                       CGAGeometryManager::GetMaterial("aluminium"),
                        "SitConeLog",
                        0, 0, 0);
  
  SitConeLog->SetVisAttributes(VisAttAlu);

  new MyPlacement(0, 
	G4ThreeVector(0, 0, 0),//(z_start_2 + z_start_3)/2.),
        SitConeLog,
        "SitConePhysical+Z",
        theWorld,
        false,
        0
#ifdef MOKKA_DEBUG
,true);
#else
);
#endif

  G4RotationMatrix * rot = new G4RotationMatrix;
  rot->rotateY(pi);

  new MyPlacement(rot,
	G4ThreeVector(0, 0, 0),// -(z_start_2 + z_start_3)/2.),
        SitConeLog,
        "SitConePhysical-Z",
        theWorld,
        false,
        0
#ifdef MOKKA_DEBUG
,true);
#else
);
#endif

  delete db;

//Then place the disk

  G4Tubs * SitDiskSolid = 
    new G4Tubs ("SitDiskSolid",
                TUBE_IPOuterBulge_end_radius+
			G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
                TPC_inner_radius,
                Sit_cables_disk_thickness/2.-
			G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
                0., 2 * pi);

  G4LogicalVolume* SitDiskLog =
    new G4LogicalVolume(SitDiskSolid,
                       CGAGeometryManager::GetMaterial("aluminium"),
                        "SitDiskLog",
                        0, 0, 0);
  
  SitDiskLog->SetVisAttributes(VisAttAlu);

  new MyPlacement(0, 
	G4ThreeVector(0, 0, TUBE_IPOuterBulge_end_z + Sit_cables_disk_thickness/2.),
        SitDiskLog,
        "SitDiskPhysical+Z",
        theWorld,
        false,
        0
#ifdef MOKKA_DEBUG
,true);
#else
);
#endif

  new MyPlacement(0, 
	G4ThreeVector(0, 0, -TUBE_IPOuterBulge_end_z - Sit_cables_disk_thickness/2.),
        SitDiskLog,
        "SitDiskPhysical-Z",
        theWorld,
        false,
        0
#ifdef MOKKA_DEBUG
,true);
#else
);
#endif

}

G4bool 
SServices00::PostConstructAction(CGAGeometryEnvironment&)
{
  return true;
}

/*
void SServices00::DefineMaterial(void) {
}
*/

G4bool
SServices00::
BuildTPCEndplateServices(const CGAGeometryEnvironment &theGeometryEnvironment,
                    G4LogicalVolume *theWorld)
{

   G4double tpcEndplateServices_R[MAX_TPC_RINGS], 
	tpcEndplateServices_r[MAX_TPC_RINGS];

   tpcEndplateServices_R[0] = 
	theGeometryEnvironment.GetParameterAsDouble("tpcEndplateServicesRing1_R");
   tpcEndplateServices_r[0] = 
	theGeometryEnvironment.GetParameterAsDouble("tpcEndplateServicesRing1_ro");

   tpcEndplateServices_R[1] = 
	theGeometryEnvironment.GetParameterAsDouble("tpcEndplateServicesRing2_R");
   tpcEndplateServices_r[1] = 
	theGeometryEnvironment.GetParameterAsDouble("tpcEndplateServicesRing2_ro");

   tpcEndplateServices_R[2] = 
	theGeometryEnvironment.GetParameterAsDouble("tpcEndplateServicesRing3_R");
   tpcEndplateServices_r[2] = 
	theGeometryEnvironment.GetParameterAsDouble("tpcEndplateServicesRing3_ro");

   tpcEndplateServices_R[3] = 
	theGeometryEnvironment.GetParameterAsDouble("tpcEndplateServicesRing4_R");
   tpcEndplateServices_r[3] = 
	theGeometryEnvironment.GetParameterAsDouble("tpcEndplateServicesRing4_ro");

   tpcEndplateServices_R[4] = 
	theGeometryEnvironment.GetParameterAsDouble("tpcEndplateServicesRing5_R");
   tpcEndplateServices_r[4] = 
	theGeometryEnvironment.GetParameterAsDouble("tpcEndplateServicesRing5_ro");

   tpcEndplateServices_R[5] = 
	theGeometryEnvironment.GetParameterAsDouble("tpcEndplateServicesRing6_R");
   tpcEndplateServices_r[5] = 
	theGeometryEnvironment.GetParameterAsDouble("tpcEndplateServicesRing6_ro");

   tpcEndplateServices_R[6] = 
	theGeometryEnvironment.GetParameterAsDouble("tpcEndplateServicesRing7_R");
   tpcEndplateServices_r[6] = 
	theGeometryEnvironment.GetParameterAsDouble("tpcEndplateServicesRing7_ro");

   tpcEndplateServices_R[7] = 
	theGeometryEnvironment.GetParameterAsDouble("tpcEndplateServicesRing8_R");
   tpcEndplateServices_r[7] = 
	theGeometryEnvironment.GetParameterAsDouble("tpcEndplateServicesRing8_ro");

   for(G4int i=0; i<MAX_TPC_RINGS; i++) {

      if(Ecal_cables_gap < 2*tpcEndplateServices_r[i])
	Control::Abort(
  	        "TPC endplate services: ring thickness exceeds Ecal_cables_gap",
		MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

      if(TPC_outer_radius < tpcEndplateServices_R[i] + tpcEndplateServices_r[i])
	Control::Abort(
	       "TPC endplate services: ring dimensions exceed TPC_outer_radius",
	       MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

      G4Torus * ringSolid = new G4Torus("ringSolid", 0,
                 tpcEndplateServices_r[i],
                 tpcEndplateServices_R[i],
                 0, 2*pi);

      G4LogicalVolume * ringLogical = 
		  new G4LogicalVolume(ringSolid,
                        CGAGeometryManager::GetMaterial("copper"),
                        "ringLogical",
                        0, 0, 0);

        G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(0.0,1.0,0.0));
        //VisAtt->SetForceWireframe(true);
        VisAtt->SetForceSolid(true);
        VisAtt->SetVisibility(true);
        ringLogical->SetVisAttributes(VisAtt);

      G4double z_position = TPC_Ecal_Hcal_barrel_halfZ + 
					tpcEndplateServices_r[i];
      
#ifdef VERBOSE

G4cout << "Placing ring number " << i << " at z = " << z_position << G4endl;

#endif
      //first placement at -Z
      new MyPlacement(0,
                  G4ThreeVector(0, 0, -z_position),
                  ringLogical,
                  "ringPhys",
                  theWorld,
                  false,-i);

      //second placement at +Z
      new MyPlacement(0,
                  G4ThreeVector(0, 0, z_position),
                  ringLogical,
                  "ringPhysical",
                  theWorld,
                  false,i);
   }
   
   return true;
}

G4bool
SServices00::
BuildEcalBarrelServices(const CGAGeometryEnvironment &theGeometryEnvironment,
                    G4LogicalVolume *theWorld)
{

  G4Box * ContainerSolid = 
      new G4Box("EcalBarrelServicesContainerSolid",
              top_dim_x/2.,
              RailHeight/2.,
              TPC_Ecal_Hcal_barrel_halfZ); 

  G4LogicalVolume * containerLogical = 
      new G4LogicalVolume(ContainerSolid,
                        CGAGeometryManager::GetMaterial("air"),
                        "EcalBarrelServicesContainerLogical",
                        0, 0, 0);

  containerLogical->SetVisAttributes(VisAttAir);
 
  if(!FillEcalBarrelServicesContainer(theGeometryEnvironment,containerLogical))
	return false;

  for (G4int stave_id = 1; stave_id < 9 ; stave_id++)
  {
	G4double phirot = (stave_id-1) * pi/4;
	G4RotationMatrix *rot=new G4RotationMatrix();
	rot->rotateZ(phirot);

	G4ThreeVector stavePosition = 
		G4ThreeVector(module_thickness * sin(pi/4.), 
				Ecal_outer_radius + RailHeight/2., 
				0);
	stavePosition.rotateZ(-phirot);

  	new MyPlacement(rot,
                  stavePosition,
                  containerLogical,
                  "EcalBarrelServicesContainerPhysical",
                  theWorld,
                  false,stave_id);
  }

  return true;
}

G4bool
SServices00::
FillEcalBarrelServicesContainer(
		const CGAGeometryEnvironment &theGeometryEnvironment,
		G4LogicalVolume *motherLogical)
{

  G4double RailDistanceToRight = theGeometryEnvironment.
		GetParameterAsDouble("EcalBarrelServices_RailDistanceToRight");

  G4double RailSeparation = theGeometryEnvironment.
		GetParameterAsDouble("EcalBarrelServices_RailSeparation");

  G4double RailWidth = theGeometryEnvironment.
		GetParameterAsDouble("EcalBarrelServices_RailWidth");

  G4bool RailSeparationChanged = false;
  if(fabs(
         top_dim_x/2. - (RailDistanceToRight + RailWidth*1.5 + RailSeparation)
         ) >= RailWidth*0.5)
  {
	RailSeparation = top_dim_x/2. - (RailDistanceToRight + RailWidth*1.5);
  	if(RailSeparation <= 0.)
		Control::Abort(
  	        	"EcalBarrel services:no room for rails, cables, etc...",
			MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

        RailSeparationChanged = true;
  }

  G4Box * railSolid = 
      new G4Box("RailSolid",
              RailWidth/2.,
              RailHeight/2.,
              TPC_Ecal_Hcal_barrel_halfZ); 

  G4LogicalVolume * railLogical = 
      new G4LogicalVolume(railSolid,
                        CGAGeometryManager::GetMaterial("aluminium"),
                        "RailLogical",
                        0, 0, 0);

  G4VisAttributes * VisAttAlu = new G4VisAttributes(G4Colour::Gray());
  VisAttAlu->SetForceWireframe(true);
  VisAttAlu->SetVisibility(true);
  railLogical->SetVisAttributes(VisAttAlu);
 
  G4double railPosition = top_dim_x/2. - RailDistanceToRight - RailWidth/2.;

  for(G4int i=1; i<=3; i++)
  {
  	new MyPlacement(0,
                  G4ThreeVector(railPosition,0,0),
                  railLogical,
                  "railPhysical",
                  motherLogical,
                  false,i);

        railPosition -= (RailWidth + RailSeparation);
  }

//-Z thicknesses 
  G4double ZMinus_FirstInterrail_PE_Thickness = theGeometryEnvironment.
	GetParameterAsDouble(
		"EcalBarrelServices_ZMinus_FirstInterrail_PE_Thickness"
			    );

  G4double ZMinus_FirstInterrail_Cu_Thickness = theGeometryEnvironment.
	GetParameterAsDouble(
		"EcalBarrelServices_ZMinus_FirstInterrail_Cu_Thickness"
			    );

  G4double ZMinus_SecondInterrail_Cu_Thickness = theGeometryEnvironment.
	GetParameterAsDouble(
		"EcalBarrelServices_ZMinus_SecondInterrail_Cu_Thickness"
			    );

//+Z thicknesses 
  G4double ZPlus_FirstInterrail_PE_Thickness = theGeometryEnvironment.
	GetParameterAsDouble(
		"EcalBarrelServices_ZPlus_FirstInterrail_PE_Thickness"
			    );

  G4double ZPlus_FirstInterrail_Cu_Thickness = theGeometryEnvironment.
	GetParameterAsDouble(
		"EcalBarrelServices_ZPlus_FirstInterrail_Cu_Thickness"
			    );

  G4double ZPlus_SecondInterrail_Cu_Thickness = theGeometryEnvironment.
	GetParameterAsDouble(
		"EcalBarrelServices_ZPlus_SecondInterrail_Cu_Thickness"
			    );


  if(RailSeparationChanged)
  {
     G4double OldRailSeparation = theGeometryEnvironment.
                GetParameterAsDouble("EcalBarrelServices_RailSeparation");

     G4double fraction = OldRailSeparation / RailSeparation;
     ZMinus_FirstInterrail_PE_Thickness *= fraction;
     ZMinus_FirstInterrail_Cu_Thickness *= fraction;
     ZMinus_SecondInterrail_Cu_Thickness *= fraction;

     ZPlus_FirstInterrail_PE_Thickness *= fraction;
     ZPlus_FirstInterrail_Cu_Thickness *= fraction;
     ZPlus_SecondInterrail_Cu_Thickness *= fraction;
  }

  G4double moduleLength = TPC_Ecal_Hcal_barrel_halfZ * 2 / 5.;

  //First place ingredients at -Z:
  for(G4int i=0; i<3; i++)
  {
     G4double x_half_dim = (RailSeparation  * (3 - i) / 3.) / 2.;
     G4Box * PESolid = 
         new G4Box("PESolid",
	      x_half_dim,
              ZMinus_FirstInterrail_PE_Thickness / 2.,
              moduleLength / 2.); 

     G4LogicalVolume * PELogical = 
         new G4LogicalVolume(PESolid,
                        CGAGeometryManager::GetMaterial("polyethylene"),
                        "PELogical",
                        0, 0, 0);

     PELogical->SetVisAttributes(VisAttPE);

     new MyPlacement(0,
         G4ThreeVector(
		top_dim_x/2.-RailDistanceToRight-RailWidth-RailSeparation+
			x_half_dim,
                -RailHeight/2. + ZMinus_FirstInterrail_PE_Thickness/2.,
                -TPC_Ecal_Hcal_barrel_halfZ + moduleLength*(i+1/2.) 
		  ),
                  PELogical,
                  "PEPhysical",
                  motherLogical,
                  false,i+1);

     G4Box * Cu_1_Solid = 
         new G4Box("Cu_1_Solid",
	      x_half_dim,
              ZMinus_FirstInterrail_Cu_Thickness / 2.,
              moduleLength / 2.); 

     G4LogicalVolume * Cu_1_Logical = 
         new G4LogicalVolume(Cu_1_Solid,
                        CGAGeometryManager::GetMaterial("copper"),
                        "Cu_1_Logical",
                        0, 0, 0);

     Cu_1_Logical->SetVisAttributes(VisAttCu);

     new MyPlacement(0,
         G4ThreeVector(
		top_dim_x/2.-RailDistanceToRight-RailWidth-RailSeparation+
			x_half_dim,
                -RailHeight/2. + ZMinus_FirstInterrail_PE_Thickness+
			ZMinus_FirstInterrail_Cu_Thickness/2.,
                -TPC_Ecal_Hcal_barrel_halfZ + moduleLength*(i+1/2.) 
		  ),
                  Cu_1_Logical,
                  "Cu_1_Physical",
                  motherLogical,
                  false,i+1);

     G4Box * Cu_2_Solid = 
         new G4Box("Cu_2_Solid",
	      x_half_dim,
              ZMinus_SecondInterrail_Cu_Thickness / 2.,
              moduleLength / 2.); 

     G4LogicalVolume * Cu_2_Logical = 
         new G4LogicalVolume(Cu_2_Solid,
                        CGAGeometryManager::GetMaterial("copper"),
                        "Cu_2_Logical",
                        0, 0, 0);

     Cu_2_Logical->SetVisAttributes(VisAttCu);

     new MyPlacement(0,
         G4ThreeVector(
		top_dim_x/2.-RailDistanceToRight-2*RailWidth-2*RailSeparation+
			x_half_dim,
                -RailHeight/2. + ZMinus_SecondInterrail_Cu_Thickness/2.,
                -TPC_Ecal_Hcal_barrel_halfZ + moduleLength*(i+1/2.) 
		  ),
                  Cu_2_Logical,
                  "Cu_2_Physical",
                  motherLogical,
                  false,i+1);

  }
 
  //Now place ingredients at +Z:
  for(G4int i=0; i<2; i++)
  {
     G4double x_half_dim = (RailSeparation  * (2 - i) / 2.) / 2.;
     G4Box * PESolid = 
         new G4Box("PESolid",
	      x_half_dim,
              ZPlus_FirstInterrail_PE_Thickness / 2.,
              moduleLength / 2.); 

     G4LogicalVolume * PELogical = 
         new G4LogicalVolume(PESolid,
                        CGAGeometryManager::GetMaterial("polyethylene"),
                        "PELogical",
                        0, 0, 0);

     PELogical->SetVisAttributes(VisAttPE);

     new MyPlacement(0,
         G4ThreeVector(
		top_dim_x/2.-RailDistanceToRight-RailWidth-RailSeparation+
			x_half_dim,
                -RailHeight/2. + ZPlus_FirstInterrail_PE_Thickness/2.,
                TPC_Ecal_Hcal_barrel_halfZ - moduleLength*(i+1/2.) 
		  ),
                  PELogical,
                  "PEPhysical",
                  motherLogical,
                  false,i+1);

     G4Box * Cu_1_Solid = 
         new G4Box("Cu_1_Solid",
	      x_half_dim,
              ZPlus_FirstInterrail_Cu_Thickness / 2.,
              moduleLength / 2.); 

     G4LogicalVolume * Cu_1_Logical = 
         new G4LogicalVolume(Cu_1_Solid,
                        CGAGeometryManager::GetMaterial("copper"),
                        "Cu_1_Logical",
                        0, 0, 0);

     Cu_1_Logical->SetVisAttributes(VisAttCu);

     new MyPlacement(0,
         G4ThreeVector(
		top_dim_x/2.-RailDistanceToRight-RailWidth-RailSeparation+
			x_half_dim,
                -RailHeight/2. + ZPlus_FirstInterrail_PE_Thickness+
			ZPlus_FirstInterrail_Cu_Thickness/2.,
                TPC_Ecal_Hcal_barrel_halfZ - moduleLength*(i+1/2.) 
		  ),
                  Cu_1_Logical,
                  "Cu_1_Physical",
                  motherLogical,
                  false,i+1);

     G4Box * Cu_2_Solid = 
         new G4Box("Cu_2_Solid",
	      x_half_dim,
              ZPlus_SecondInterrail_Cu_Thickness / 2.,
              moduleLength / 2.); 

     G4LogicalVolume * Cu_2_Logical = 
         new G4LogicalVolume(Cu_2_Solid,
                        CGAGeometryManager::GetMaterial("copper"),
                        "Cu_2_Logical",
                        0, 0, 0);

     Cu_2_Logical->SetVisAttributes(VisAttCu);

     new MyPlacement(0,
         G4ThreeVector(
		top_dim_x/2.-RailDistanceToRight-2*RailWidth-2*RailSeparation+
			x_half_dim,
                -RailHeight/2. + ZPlus_SecondInterrail_Cu_Thickness/2.,
                TPC_Ecal_Hcal_barrel_halfZ - moduleLength*(i+1/2.) 
		  ),
                  Cu_2_Logical,
                  "Cu_2_Physical",
                  motherLogical,
                  false,i+1);
  }

  return true;
}

G4bool
SServices00::
BuildEcalBarrel_EndCapServices(
		const CGAGeometryEnvironment &theGeometryEnvironment,
		G4LogicalVolume *theWorld)
{

//-Z thicknesses 
  G4double ZMinus_PE_Thickness = theGeometryEnvironment.
	GetParameterAsDouble(
		"EcalBarrel_EndCapServices_ZMinus_PE_Thickness"
			    );

  G4double ZMinus_Cu_Thickness = theGeometryEnvironment.
	GetParameterAsDouble(
		"EcalBarrel_EndCapServices_ZMinus_Cu_Thickness"
			    );

//+Z thicknesses 
  G4double ZPlus_PE_Thickness = theGeometryEnvironment.
	GetParameterAsDouble(
		"EcalBarrel_EndCapServices_ZPlus_PE_Thickness"
			    );

  G4double ZPlus_Cu_Thickness = theGeometryEnvironment.
	GetParameterAsDouble(
		"EcalBarrel_EndCapServices_ZPlus_Cu_Thickness"
			    );

  G4double containerThickness = ZMinus_PE_Thickness + ZMinus_Cu_Thickness;
  G4double z_position = -TPC_Ecal_Hcal_barrel_halfZ -containerThickness/2.;

  G4double Cu_Thickness = ZMinus_Cu_Thickness;
  G4double PE_Thickness = ZMinus_PE_Thickness;

  G4double container_x_dim = top_dim_x/2. + module_thickness*sin(pi/4.)-
		InnerServicesWidth/2.;

  for(G4int i=0; i<=1; i++)
  {

    if(Ecal_cables_gap < containerThickness)
      Control::Abort(
  	"EcalBarrel_EndCap services: Cu+PE thicknesses exceed Ecal_cables_gap",
		MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

    G4Box * ContainerSolid = 
        new G4Box("EcalBarrel_EndCapServicesContainerSolid",
              container_x_dim/2.,
              module_thickness/2.,
              containerThickness/2.); 

    G4LogicalVolume * containerLogical = 
        new G4LogicalVolume(ContainerSolid,
                        CGAGeometryManager::GetMaterial("air"),
                        "EcalBarrel_EndCapServicesContainerLogical",
                        0, 0, 0);

    containerLogical->SetVisAttributes(VisAttAir);

    G4Box * PESolid = 
         new G4Box("EcalBarrel_EndCap_PESolid",
	      container_x_dim/2.,
              module_thickness/2.,
              PE_Thickness/2.); 

    G4LogicalVolume * PELogical = 
         new G4LogicalVolume(PESolid,
                        CGAGeometryManager::GetMaterial("polyethylene"),
                        "EcalBarrel_EndCap_PELogical",
                        0, 0, 0);

    PELogical->SetVisAttributes(VisAttPE);

    new MyPlacement(0,
         G4ThreeVector(0,0,containerThickness/2. - PE_Thickness/2.),
                  PELogical,
                  "EcalBarrel_EndCap_PEPhysical",
                  containerLogical,
                  false,i);

    G4Box * Cu_Solid = 
         new G4Box("EcalBarrel_EndCap_Cu_Solid",
	      container_x_dim/2.,
              module_thickness/2.,
              Cu_Thickness/2.); 

    G4LogicalVolume * Cu_Logical = 
         new G4LogicalVolume(Cu_Solid,
                        CGAGeometryManager::GetMaterial("copper"),
                        "EcalBarrel_EndCap_Cu_Logical",
                        0, 0, 0);

    Cu_Logical->SetVisAttributes(VisAttCu);

    new MyPlacement(0,
         G4ThreeVector(0,0,-containerThickness/2. + Cu_Thickness/2.),
                  Cu_Logical,
                  "EcalBarrel_EndCap_CuPhysical",
                  containerLogical,
                  false,i);

    for (G4int stave_id = 1; stave_id < 9 ; stave_id++)
    {
	G4double phirot = (stave_id-1) * pi/4;
	G4RotationMatrix *rot=new G4RotationMatrix();
        if(z_position > 0) {
		rot->rotateY(pi);
		rot->rotateZ(-phirot);
	}
	else
		rot->rotateZ(phirot);

	G4ThreeVector stavePosition = 
		G4ThreeVector(-InnerServicesWidth/2. - container_x_dim/2., 
				Ecal_outer_radius - module_thickness/2., 
				z_position);
        if(z_position > 0) stavePosition[0] = - stavePosition[0];

	stavePosition.rotateZ(-phirot);

  	new MyPlacement(rot,
                  stavePosition,
                  containerLogical,
                  "EcalBarrel_EndCapServicesContainerPhysical",
                  theWorld,
                  false,stave_id);
    }

    containerThickness = ZPlus_PE_Thickness + ZPlus_Cu_Thickness;
    z_position = TPC_Ecal_Hcal_barrel_halfZ + containerThickness/2.;
    
    Cu_Thickness = ZPlus_Cu_Thickness;
    PE_Thickness = ZPlus_PE_Thickness;

  }
  return true;
}

G4bool
SServices00::
BuildHcalBarrel_EndCapServices(
		const CGAGeometryEnvironment &theGeometryEnvironment,
		G4LogicalVolume *theWorld)
{

  G4double Hcal_stave_gaps = 
	theGeometryEnvironment.GetParameterAsDouble("Hcal_stave_gaps");
  G4double Hcal_inner_radius = Ecal_outer_radius + 
	theGeometryEnvironment.GetParameterAsDouble("Hcal_Ecal_gap");
  G4double Hcal_R_max = 
	theGeometryEnvironment.GetParameterAsDouble("Hcal_R_max");

  Hcal_total_dim_y = Hcal_R_max * cos(pi/16) - Hcal_inner_radius;
  G4double Hcal_module_radius = Hcal_inner_radius + Hcal_total_dim_y;
  Hcal_y_dim2_for_x  = 
	(Hcal_module_radius - Hcal_module_radius*cos(pi/8));
  G4double Hcal_y_dim1_for_x  = Hcal_total_dim_y - Hcal_y_dim2_for_x;
  Hcal_bottom_dim_x  = 
	2.*Hcal_inner_radius*tan(pi/8.)- Hcal_stave_gaps;
  Hcal_midle_dim_x   = 
	Hcal_bottom_dim_x + 2* Hcal_y_dim1_for_x*tan(pi/8.);
  Hcal_top_dim_x     = 
	Hcal_midle_dim_x - 2 * Hcal_y_dim2_for_x/tan(pi/8.);

#ifdef VERBOSE
  G4double Hcal_outer_radius = Hcal_inner_radius + Hcal_total_dim_y;

  G4cout << "BuildHcalBarrel_EndCapServices information: "
         << "\n                       Hcal_outer_radius = "
         << Hcal_outer_radius
         << "\n                       module thickness = "
         << Hcal_total_dim_y
         << "\n                       Hcal_R_max = "
         << Hcal_R_max
         << "\n                       Hcal_bottom_dim_x = "
         << Hcal_bottom_dim_x
	 << G4endl;
#endif

  G4double BHX  = Hcal_bottom_dim_x /2.-
	G4GeometryTolerance::GetInstance()->GetSurfaceTolerance();
  G4double MHX  = Hcal_midle_dim_x / 2.-
	G4GeometryTolerance::GetInstance()->GetSurfaceTolerance();
  G4double THX  = Hcal_top_dim_x / 2.-
	G4GeometryTolerance::GetInstance()->GetSurfaceTolerance();
  G4double YX1H = Hcal_y_dim1_for_x / 2.;
  G4double YX2H = Hcal_y_dim2_for_x / 2.;
  G4double DHZ  = Ecal_cables_gap / 2.-
	G4GeometryTolerance::GetInstance()->GetSurfaceTolerance();

  // Attention: on bâtit le module dans la verticale
  // à cause du G4Trd et on le tourne avant de le positioner
  G4Trd * Bottom = new G4Trd("Bottom_Services_Module",
                             BHX, MHX, DHZ, DHZ, YX1H);

  G4Trd * Top = new G4Trd("Top_Services_Module",
                          MHX, THX, DHZ, DHZ, YX2H);

  G4UnionSolid* ModuleSolid = new G4UnionSolid("HcalServicesModuleSolid",
                                               Bottom,
                                               Top,
                                               0,
                                               G4ThreeVector(0, 0, YX1H + YX2H));


  G4LogicalVolume * ModuleLogicalZMinus  = new G4LogicalVolume(ModuleSolid,
                                                CGAGeometryManager::GetMaterial("air"),
                                                "ServicesHcalModuleZMinus",
                                                0, 0, 0);

  ModuleLogicalZMinus->SetVisAttributes(VisAttAir);

  G4LogicalVolume * ModuleLogicalZPlus  = new G4LogicalVolume(ModuleSolid,
                                                CGAGeometryManager::GetMaterial("air"),
                                                "ServicesHcalModuleZPlus",
                                                0, 0, 0);

  ModuleLogicalZPlus->SetVisAttributes(VisAttAir);

  //First place layer models of services coming from Ecal and TPC:
  if(!FillHcalServicesModuleWithInnerServices(
		theGeometryEnvironment,ModuleLogicalZMinus,ModuleLogicalZPlus))
      return false;

  //Then place layer models of HCAL electronics interface:
  G4String BuildHcalElectronicsInterface = theGeometryEnvironment
	.GetParameterAsString("BuildHcalElectronicsInterface");

  if(BuildHcalElectronicsInterface == "true")
    if(!FillHcalServicesModuleWithHcalElectronicsInterface(
		theGeometryEnvironment,ModuleLogicalZMinus,ModuleLogicalZPlus))
      return false;

  G4double Y = Hcal_inner_radius + YX1H;
  G4double stave_phi_offset = 0;

  for (G4int stave_id = 1;
       stave_id <= 8;
       stave_id++)
    {
          G4double module_z_offset = 
		- TPC_Ecal_Hcal_barrel_halfZ - Ecal_cables_gap/2.;

          G4double phirot = stave_phi_offset;
          G4RotationMatrix *rot = new G4RotationMatrix();
          rot->rotateX(pi*0.5);
          rot->rotateY(phirot);

          new MyPlacement(rot,
                          G4ThreeVector(-Y*sin(phirot),
                                        Y*cos(phirot),
                                        module_z_offset),
                          ModuleLogicalZMinus,
                          "ServicesHcalModuleZMinusPhysical",
                          theWorld,
                          false,
                          stave_id*10);

          module_z_offset = - module_z_offset;

          new MyPlacement(rot,
                          G4ThreeVector(-Y*sin(phirot),
                                        Y*cos(phirot),
                                        module_z_offset),
                          ModuleLogicalZPlus,
                          "ServicesHcalModuleZPlusPhysical",
                          theWorld,
                          false,
                          stave_id*10+1);

          stave_phi_offset -=  pi/4;
    }

   return true;
}

G4bool
SServices00::
FillHcalServicesModuleWithInnerServices(
		const CGAGeometryEnvironment &theGeometryEnvironment,
		G4LogicalVolume *ModuleLogicalZMinus,
		G4LogicalVolume *ModuleLogicalZPlus)
{

//-Z thicknesses 
  G4double ZMinus_StainlessSteel_Thickness = theGeometryEnvironment.
	GetParameterAsDouble(
		"HcalServicesModule_ZMinus_StainlessSteel_Thickness"
			    );

  G4double ZMinus_PE_Thickness = theGeometryEnvironment.
	GetParameterAsDouble(
		"HcalServicesModule_ZMinus_PE_Thickness"
			    );

  G4double ZMinus_Cu_Thickness = theGeometryEnvironment.
	GetParameterAsDouble(
		"HcalServicesModule_ZMinus_Cu_Thickness"
			    );

//+Z thicknesses 
  G4double ZPlus_StainlessSteel_Thickness = theGeometryEnvironment.
	GetParameterAsDouble(
		"HcalServicesModule_ZPlus_StainlessSteel_Thickness"
			    );

  G4double ZPlus_PE_Thickness = theGeometryEnvironment.
	GetParameterAsDouble(
		"HcalServicesModule_ZPlus_PE_Thickness"
			    );

  G4double ZPlus_Cu_Thickness = theGeometryEnvironment.
	GetParameterAsDouble(
		"HcalServicesModule_ZPlus_Cu_Thickness"
			    );

  G4double StainlessSteel_Thickness = ZMinus_StainlessSteel_Thickness;
  G4double Cu_Thickness = ZMinus_Cu_Thickness;
  G4double PE_Thickness = ZMinus_PE_Thickness;

  G4LogicalVolume *motherLogical = ModuleLogicalZMinus;

  G4double positionRotation = pi*0.5;
  G4double yShift = Hcal_y_dim2_for_x/2.;

  for(G4int i=0; i<=1; i++)
  {

    G4ThreeVector layerPosition = G4ThreeVector(0,yShift,
		Ecal_cables_gap/2. - StainlessSteel_Thickness/2.);
    layerPosition.rotateX(positionRotation);

    if(!PlaceHcalInnerServicesLayer(motherLogical,
		CGAGeometryManager::GetMaterial("stainless_steel"),
		VisAttStainlessSteel,
		StainlessSteel_Thickness, layerPosition))

	return false;

    layerPosition = G4ThreeVector(0,yShift,
		Ecal_cables_gap/2. -StainlessSteel_Thickness - PE_Thickness/2.);
    layerPosition.rotateX(positionRotation);

    if(!PlaceHcalInnerServicesLayer(motherLogical,
		CGAGeometryManager::GetMaterial("polyethylene"),
		VisAttPE,
		PE_Thickness, layerPosition))

	return false;

    layerPosition = G4ThreeVector(0,yShift,
		Ecal_cables_gap/2.-StainlessSteel_Thickness-PE_Thickness -
		Cu_Thickness/2.);
    layerPosition.rotateX(positionRotation);

    if(!PlaceHcalInnerServicesLayer(motherLogical,
		CGAGeometryManager::GetMaterial("copper"),VisAttCu,
		Cu_Thickness, layerPosition))

	return false;

    StainlessSteel_Thickness = ZPlus_StainlessSteel_Thickness;
    Cu_Thickness = ZPlus_Cu_Thickness;
    PE_Thickness = ZPlus_PE_Thickness;

    positionRotation = -pi*0.5;
    yShift = -Hcal_y_dim2_for_x/2.;

    motherLogical = ModuleLogicalZPlus;
  }

   return true;
}

G4bool
SServices00::
FillHcalServicesModuleWithHcalElectronicsInterface(
		const CGAGeometryEnvironment &theGeometryEnvironment,
		G4LogicalVolume *ModuleLogicalZMinus,
		G4LogicalVolume *ModuleLogicalZPlus)
{
   G4double Hcal_back_plate_thickness = theGeometryEnvironment.
	GetParameterAsDouble("Hcal_back_plate_thickness");
  
   G4double Hcal_nlayers = theGeometryEnvironment.
	GetParameterAsInt("Hcal_nlayers");

   G4double Hcal_radiator_thickness = theGeometryEnvironment.
	GetParameterAsDouble("Hcal_radiator_thickness");

   G4double Hcal_layer_thickenss = 
	(Hcal_total_dim_y - Hcal_back_plate_thickness) / Hcal_nlayers;

   G4double Hcal_chamber_thickness = 
	Hcal_layer_thickenss - Hcal_radiator_thickness;

   if(Hcal_chamber_thickness <= 0)
      Control::Abort(
  	"Hcal Barrel-EndCap services: Hcal chamber thicknesses  <= 0",
		MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

#ifdef VERBOSE
G4cout << "Hcal Barrel-EndCap services: Hcal_chamber_thickness = " <<
	Hcal_chamber_thickness << G4endl;
#endif

   G4double Hcal_y_dim1_for_x  = Hcal_total_dim_y - Hcal_y_dim2_for_x;

   G4double layer_x_dim = 0;
   G4double layer_y_offset = 0;

   for(G4int layer_id=1; layer_id<=Hcal_nlayers; layer_id++) {

	layer_y_offset = (layer_id - 1)*Hcal_layer_thickenss + 
                        Hcal_radiator_thickness;

        //---- bottom barrel----
	if(layer_id * Hcal_layer_thickenss  < Hcal_y_dim1_for_x )
		layer_x_dim = Hcal_bottom_dim_x + 2 * 
				layer_y_offset * tan(pi/8.);
	else   //----- top barrel ---
		layer_x_dim = Hcal_midle_dim_x - 2*
			( layer_y_offset + Hcal_chamber_thickness -
				Hcal_y_dim1_for_x ) / tan(pi/8.);

	if(!FillHcalElectronicsInterfaceLayer(theGeometryEnvironment,
                ModuleLogicalZMinus, ModuleLogicalZPlus,
		layer_y_offset, layer_x_dim))
		
		return false;
 
  }

   return true;
}

G4bool
SServices00::
FillHcalElectronicsInterfaceLayer(
		const CGAGeometryEnvironment &theGeometryEnvironment,
		G4LogicalVolume *ModuleLogicalZMinus,
		G4LogicalVolume *ModuleLogicalZPlus,
		G4double layer_y_offset, G4double layer_x_dim)
{
   G4double Hcal_y_dim1_for_x  = Hcal_total_dim_y - Hcal_y_dim2_for_x;
   G4double Hcal_steel_cassette_thickness = theGeometryEnvironment.
	GetParameterAsDouble("Hcal_steel_cassette_thickness");

   G4double y_position = -Hcal_y_dim1_for_x/2. + layer_y_offset +
		Hcal_steel_cassette_thickness/2.;

   if(!PlaceHcalElectronicsInterfaceComponent(ModuleLogicalZMinus,
	ModuleLogicalZPlus,
	CGAGeometryManager::GetMaterial("S235"),
	VisAttStainlessSteel,
	Hcal_steel_cassette_thickness,y_position,layer_x_dim)
     )
		return false;

   G4double FR4_thickness = theGeometryEnvironment.
        GetParameterAsDouble("HcalServices_outer_FR4_thickness");

   y_position += (Hcal_steel_cassette_thickness/2. +
			FR4_thickness/2.);

   if(!PlaceHcalElectronicsInterfaceComponent(ModuleLogicalZMinus,
	ModuleLogicalZPlus,
	CGAGeometryManager::GetMaterial("PCB"),
	VisAttPE,
	FR4_thickness,y_position,layer_x_dim)
     )
		return false;

   G4double Cu_thickness = theGeometryEnvironment.
        GetParameterAsDouble("HcalServices_outer_Cu_thickness");

   y_position += (FR4_thickness/2. + Cu_thickness/2.);

   if(!PlaceHcalElectronicsInterfaceComponent(ModuleLogicalZMinus,
	ModuleLogicalZPlus,
	CGAGeometryManager::GetMaterial("copper"),
	VisAttCu,
	Cu_thickness,y_position,layer_x_dim)
     )
		return false;

   return true;
}

G4bool
SServices00::
PlaceHcalElectronicsInterfaceComponent(
		G4LogicalVolume *ModuleLogicalZMinus,
		G4LogicalVolume *ModuleLogicalZPlus,
		G4Material *layerMaterial,
		G4VisAttributes *visAtt,
		G4double layerThickness,
		G4double y_position, G4double layer_x_dim)
{
   G4Box * layerSolid = 
         new G4Box("HcalBarrel_EndCap_ElectronicsComponent_Solid",
	      layer_x_dim/2.-
		4*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()),
              layerThickness/2.-
		4*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()),
              Ecal_cables_gap/2.-
		4*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance())); 

   G4VSolid * cutSolid = CutLayer(layerSolid,y_position);

   G4LogicalVolume * layerLogical = 
	new G4LogicalVolume(cutSolid,
                            layerMaterial,
                            "ElectronicsInterfaceLayerLogical",
                            0, 0, 0);

   layerLogical->SetVisAttributes(visAtt);
   
   G4RotationMatrix *rot = new G4RotationMatrix();
   rot->rotateX(pi*0.5);

   new MyPlacement(rot,
		  G4ThreeVector(0,0, y_position),
                  layerLogical,
                  "HcalBarrel_EndCap_ElectronicsInterfaceLayerPhysical",
                  ModuleLogicalZMinus,
                  false,0);

   new MyPlacement(rot,
		  G4ThreeVector(0,0, y_position),
                  layerLogical,
                  "HcalBarrel_EndCap_ElectronicsInterfaceLayerPhysical",
                  ModuleLogicalZPlus,
                  false,0);

   return true;
}

G4VSolid * SServices00::CutLayer(G4VSolid *layerSolid, G4double y_position)
{
    G4double tolerance = 2 * mm;

    G4Box * Solid_Side = 
         new G4Box("Solid_Side",
	      InnerServicesWidth/2. + tolerance,
              Hcal_total_dim_y + 8*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()),
              Ecal_cables_gap + 8*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance())); 

    G4double xShift = (Hcal_bottom_dim_x + Hcal_total_dim_y * tan(pi/8.)) / 2.;
    G4double yShift = Hcal_y_dim2_for_x/2. - y_position;

    G4ThreeVector rightSideLayerPosition(xShift,yShift,0);

    G4RotationMatrix *rotationSide = new G4RotationMatrix();
    rotationSide->rotateZ(pi/8.);

    G4SubtractionSolid* subtractionRight = 
    	new G4SubtractionSolid("HcalBarrel_EndCap_EI_Solid_Right",
        		layerSolid,Solid_Side,rotationSide,
			rightSideLayerPosition);

    G4ThreeVector leftSideLayerPosition(-xShift,yShift,0);
    rotationSide = new G4RotationMatrix();
    rotationSide->rotateZ(-pi/8.);

    G4SubtractionSolid* subtractionLeft =
    	new G4SubtractionSolid("HcalBarrel_EndCap_EI_Solid_Left",
        		subtractionRight,Solid_Side,rotationSide,
			leftSideLayerPosition);

    G4ThreeVector centerLayerPosition(0,yShift,0);

    G4SubtractionSolid* theFinalCut =
    	new G4SubtractionSolid("HcalBarrel_EndCap_EI_Solid_Final",
        		subtractionLeft,Solid_Side,0,
			centerLayerPosition);

    return theFinalCut;

}

G4bool
SServices00::
PlaceHcalInnerServicesLayer(G4LogicalVolume *motherLogical,
			    G4Material*layerMaterial,
			    G4VisAttributes* visAtt,
			    G4double layerThickness,
			    G4ThreeVector &layerPosition)
{

    G4RotationMatrix *rot = new G4RotationMatrix();
    rot->rotateX(pi*0.5);

    G4Box * Center_Solid = 
         new G4Box("HcalBarrel_EndCap_Center_Solid",
	      InnerServicesWidth/2.-
		2*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()),
              Hcal_total_dim_y/2.-
		2*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()),
              layerThickness/2.-
		2*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance())); 

    G4LogicalVolume * Center_Logical = 
         new G4LogicalVolume(Center_Solid,
			layerMaterial,
                        "HcalBarrel_EndCap_Center_Logical",
                        0, 0, 0);

    Center_Logical->SetVisAttributes(visAtt);

    new MyPlacement(rot,
		  layerPosition,
                  Center_Logical,
                  "HcalBarrel_EndCap_CenterPhysical",
                  motherLogical,
                  false,0);

    G4Box * Solid_Side = 
         new G4Box("Solid_Side",
	      InnerServicesWidth/2.-
		2*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()),
              Hcal_total_dim_y-
		4*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()),
              layerThickness/2.-
		2*(G4GeometryTolerance::GetInstance()->GetSurfaceTolerance())); 

    G4VSolid* motherSolid = motherLogical->GetSolid();

    G4double xShift = (Hcal_bottom_dim_x + Hcal_total_dim_y * tan(pi/8.)) / 2.;

    G4ThreeVector sideLayerPosition = layerPosition;
    sideLayerPosition[0] = xShift;

    G4RotationMatrix *rotationSide = new G4RotationMatrix();
    rotationSide->rotateX(pi*0.5);
    rotationSide->rotateZ(-pi/8.);

    G4IntersectionSolid* intersectionSolidRight =
    	new G4IntersectionSolid("HcalBarrel_EndCap_Solid_Right",
        		motherSolid,Solid_Side,rotationSide,sideLayerPosition);

    G4LogicalVolume * Logical_Right = 
         new G4LogicalVolume(intersectionSolidRight,
			layerMaterial,
                        "HcalBarrel_EndCap_Logical_Right",
                        0, 0, 0);

    Logical_Right->SetVisAttributes(visAtt);

    new MyPlacement(0,
		  G4ThreeVector(0,0,0),
                  Logical_Right,
                  "HcalBarrel_EndCap_Physical_Right",
                  motherLogical,
                  false,0);

    sideLayerPosition = layerPosition;
    sideLayerPosition[0] = -xShift;

    rotationSide = new G4RotationMatrix();
    rotationSide->rotateX(pi*0.5);
    rotationSide->rotateZ(pi/8.);

    G4IntersectionSolid* intersectionSolidLeft =
    	new G4IntersectionSolid("HcalBarrel_EndCap_Solid_Left",
        		motherSolid,Solid_Side,rotationSide,sideLayerPosition);

    G4LogicalVolume * Logical_Left = 
         new G4LogicalVolume(intersectionSolidLeft,
			layerMaterial,
                        "HcalBarrel_EndCap_Logical_Left",
                        0, 0, 0);

    Logical_Left->SetVisAttributes(visAtt);

    new MyPlacement(0,
		  G4ThreeVector(0,0,0),
                  Logical_Left,
                  "HcalBarrel_EndCap_Physical_Left",
                  motherLogical,
                  false,0);

   return true;
}

