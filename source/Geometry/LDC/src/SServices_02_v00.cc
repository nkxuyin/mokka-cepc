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
// $Id: SServices_02_v00.cc,v 1.1 2009/06/04 11:40:42 musat Exp $
// $Name: mokka-07-00 $
//
// 
//
// SServices_02_v00.cc
//

#include "Control.hh"
#include "MyPlacement.hh"
#include "MySQLWrapper.hh"
#include "SServices_02_v00.hh"
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
INSTANTIATE(SServices_02_v00)

// #define VERBOSE 1

#define MAX_TPC_RINGS 8

#include <math.h>

G4bool 
SServices_02_v00::
ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
		    G4LogicalVolume *theWorld)
{

  G4cout << "\nBuilding SServices_02_v00"<< G4endl;
  
  // Initialize the Geant3 interface
  if(Control::DUMPG3) MyPlacement::Init("SERVICES","SServices_02_v00");

  CoolingEcalStaves_MaxThickness = 0.;
  deltaYDueToCoolingStave2_ZMinus = 0.;
  deltaYDueToCoolingStave2_ZPlus = 0.;

  TPC_Ecal_Hcal_barrel_halfZ = 
     aGeometryEnvironment.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");

  TPC_outer_radius = 
     aGeometryEnvironment.GetParameterAsDouble("TPC_outer_radius");

  Ecal_cables_gap =
	aGeometryEnvironment.GetParameterAsDouble("Ecal_cables_gap");

  Hcal_outer_radius =
    aGeometryEnvironment.GetParameterAsDouble("Hcal_outer_radius");

  Ecal_outer_radius =
    aGeometryEnvironment.GetParameterAsDouble("Ecal_outer_radius");

  Hcal_inner_radius = Ecal_outer_radius +
        aGeometryEnvironment.GetParameterAsDouble("Hcal_Ecal_gap");

  Ecal_inner_radius = TPC_outer_radius +
    aGeometryEnvironment.GetParameterAsDouble("Ecal_Tpc_gap");

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

  Database * db = new Database(aGeometryEnvironment.GetDBName().data());

  BuildTPCEndplateServices(db,theWorld);

  BuildEcalBarrelServices(db,theWorld);

  BuildEcalCoolingServices(db,theWorld);

  BuildTPCAndEcalCables(db,theWorld);

  delete db;

  BuildSitCables(theWorld);

  return true;
}

SServices_02_v00::~SServices_02_v00() 
{
}  

void SServices_02_v00::BuildSitCables(G4LogicalVolume *theWorld) {

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
SServices_02_v00::PostConstructAction(CGAGeometryEnvironment&)
{
  return true;
}

void
SServices_02_v00::
BuildTPCEndplateServices(Database * db, G4LogicalVolume *theWorld)
{
  G4double tpcEndplateServices_R[MAX_TPC_RINGS], 
	tpcEndplateServices_r[MAX_TPC_RINGS];

  db->exec("select * from tpc_endplate;");
  db->getTuple();

   tpcEndplateServices_R[0] = 
	db->fetchDouble("tpcEndplateServicesRing1_R");
   tpcEndplateServices_r[0] = 
	db->fetchDouble("tpcEndplateServicesRing1_ro");

   tpcEndplateServices_R[1] = 
	db->fetchDouble("tpcEndplateServicesRing2_R");
   tpcEndplateServices_r[1] = 
	db->fetchDouble("tpcEndplateServicesRing2_ro");

   tpcEndplateServices_R[2] = 
	db->fetchDouble("tpcEndplateServicesRing3_R");
   tpcEndplateServices_r[2] = 
	db->fetchDouble("tpcEndplateServicesRing3_ro");

   tpcEndplateServices_R[3] = 
	db->fetchDouble("tpcEndplateServicesRing4_R");
   tpcEndplateServices_r[3] = 
	db->fetchDouble("tpcEndplateServicesRing4_ro");

   tpcEndplateServices_R[4] = 
	db->fetchDouble("tpcEndplateServicesRing5_R");
   tpcEndplateServices_r[4] = 
	db->fetchDouble("tpcEndplateServicesRing5_ro");

   tpcEndplateServices_R[5] = 
	db->fetchDouble("tpcEndplateServicesRing6_R");
   tpcEndplateServices_r[5] = 
	db->fetchDouble("tpcEndplateServicesRing6_ro");

   tpcEndplateServices_R[6] = 
	db->fetchDouble("tpcEndplateServicesRing7_R");
   tpcEndplateServices_r[6] = 
	db->fetchDouble("tpcEndplateServicesRing7_ro");

   tpcEndplateServices_R[7] = 
	db->fetchDouble("tpcEndplateServicesRing8_R");
   tpcEndplateServices_r[7] = 
	db->fetchDouble("tpcEndplateServicesRing8_ro");

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
}

void
SServices_02_v00::
BuildEcalBarrelServices(Database *db, G4LogicalVolume *theWorld)
{

  db->exec("select * from ecal_barrel;");
  db->getTuple();

  RailHeight = db->fetchDouble("RailHeight");

  if(RailHeight > Hcal_inner_radius-Ecal_outer_radius)
	Control::Abort(
  	        "EcalBarrel services: RailHeight exceeds Hcal_Ecal_gap",
		MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

  G4double module_thickness = Ecal_outer_radius - Ecal_inner_radius;

  // module barrel key parameters
  G4double bottom_dim_x = 2. * tan(pi/8.) * Ecal_inner_radius +
    module_thickness/sin(pi/4.);

  top_dim_x = bottom_dim_x - 2 * module_thickness;

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
 
  FillEcalBarrelServicesContainer(db,containerLogical);

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
}

void
SServices_02_v00::
FillEcalBarrelServicesContainer(Database *db, G4LogicalVolume *motherLogical)
{

  G4double RailDistanceToRight = db->fetchDouble("RailDistanceToRight");

  G4double RailSeparation = db->fetchDouble("RailSeparation");

  G4double RailWidth = db->fetchDouble("RailWidth");

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
  G4double ZMinus_FirstInterrail_PE_Thickness = db->fetchDouble(
		"ZMinus_FirstInterrail_PE_Thickness"
			    );

  G4double ZMinus_FirstInterrail_Cu_Thickness = db->fetchDouble(
		"ZMinus_FirstInterrail_Cu_Thickness"
			    );

  G4double ZMinus_SecondInterrail_Cu_Thickness = db->fetchDouble(
		"ZMinus_SecondInterrail_Cu_Thickness"
			    );

//+Z thicknesses 
  G4double ZPlus_FirstInterrail_PE_Thickness = db->fetchDouble(
		"ZPlus_FirstInterrail_PE_Thickness"
			    );

  G4double ZPlus_FirstInterrail_Cu_Thickness = db->fetchDouble(
		"ZPlus_FirstInterrail_Cu_Thickness"
			    );

  G4double ZPlus_SecondInterrail_Cu_Thickness = db->fetchDouble(
		"ZPlus_SecondInterrail_Cu_Thickness"
			    );


  if(RailSeparationChanged)
  {
     G4double OldRailSeparation = db->fetchDouble("RailSeparation");

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
                  "PEPhysical-",
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
                  "Cu_1_Physical-",
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
                  "Cu_2_Physical-",
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
                  "PEPhysical+",
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
                  "Cu_1_Physical+",
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
                  "Cu_2_Physical+",
                  motherLogical,
                  false,i+1);
  }
}

void
SServices_02_v00::
BuildTPCAndEcalCables(Database * db, G4LogicalVolume *theWorld)
{

  db->exec("select * from tpc_ecal_cables;" );
  db->getTuple();

//First place cables from Ecal Barrel at -Z
  G4double EcalCables_PEThickness_ZMinus = db->fetchDouble("EcalCables_PEThickness_ZMinus");
  G4double EcalCables_CuThickness_ZMinus = db->fetchDouble("EcalCables_CuThickness_ZMinus");

  if(Ecal_cables_gap < (EcalCables_PEThickness_ZMinus + EcalCables_CuThickness_ZMinus))
      Control::Abort(
	"SServices_02_v00::BuildTPCAndEcalCables : EcalCablesThickness at -Z exceeds Ecal_cables_gap",
		MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

  G4double EcalCablesYDimension_ZMinus = Hcal_outer_radius - Ecal_outer_radius - deltaYDueToCoolingStave2_ZMinus;
  G4double EcalCablesXDimension_ZMinus = db->fetchDouble("EcalCables_Width_ZMinus");

  G4double EcalCablesXPosition = Ecal_outer_radius*tan(pi/8.)*
		(db->fetchDouble("EcalCables_XPosition"));
  G4double EcalCablesYPosition_ZMinus = (Hcal_outer_radius + Ecal_outer_radius + deltaYDueToCoolingStave2_ZMinus)/2.;

  BuildCables(EcalCablesXDimension_ZMinus, EcalCables_PEThickness_ZMinus, EcalCables_CuThickness_ZMinus,
		EcalCablesYDimension_ZMinus, EcalCablesXPosition, EcalCablesYPosition_ZMinus,
		theWorld,-1);

//Then place cables from Ecal Barrel at +Z
  G4double EcalCables_PEThickness_ZPlus = db->fetchDouble("EcalCables_PEThickness_ZPlus");
  G4double EcalCables_CuThickness_ZPlus = db->fetchDouble("EcalCables_CuThickness_ZPlus");

  if(Ecal_cables_gap < (EcalCables_PEThickness_ZPlus + EcalCables_CuThickness_ZPlus))
      Control::Abort(
	"SServices_02_v00::BuildTPCAndEcalCables : EcalCablesThickness at +Z exceeds Ecal_cables_gap",
		MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

  G4double EcalCablesYDimension_ZPlus = Hcal_outer_radius - Ecal_outer_radius - deltaYDueToCoolingStave2_ZPlus;
  G4double EcalCablesXDimension_ZPlus = db->fetchDouble("EcalCables_Width_ZPlus");
  G4double EcalCablesYPosition_ZPlus = (Hcal_outer_radius + Ecal_outer_radius + deltaYDueToCoolingStave2_ZPlus)/2.;

  BuildCables(EcalCablesXDimension_ZPlus, EcalCables_PEThickness_ZPlus, EcalCables_CuThickness_ZPlus,
		EcalCablesYDimension_ZPlus, EcalCablesXPosition, EcalCablesYPosition_ZPlus,
		theWorld,+1);

//Then place cables from Ecal EndCaps
  G4double EcalEndcapCables_PEThickness = db->fetchDouble("EcalEndcapCables_PEThickness");
  G4double EcalEndcapCables_CuThickness = db->fetchDouble("EcalEndcapCables_CuThickness");

  if(Ecal_cables_gap < (EcalEndcapCables_PEThickness + EcalEndcapCables_CuThickness))
      Control::Abort(
	"SServices_02_v00::BuildTPCAndEcalCables : EcalEndcapCablesThickness exceeds Ecal_cables_gap",
		MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

  G4double EcalEndcapCablesYDimension = Hcal_outer_radius - Hcal_inner_radius;
  G4double EcalEndcapCablesXDimension = db->fetchDouble("EcalEndcapCables_Width");

  G4double EcalEndcapCablesXPosition = Hcal_inner_radius*tan(pi/8.)*
		(db->fetchDouble("EcalEndcapCables_XPosition"));
  G4double EcalEndcapCablesYPosition = (Hcal_outer_radius + Hcal_inner_radius)/2.;

  BuildCables(EcalEndcapCablesXDimension, EcalEndcapCables_PEThickness, EcalEndcapCables_CuThickness,
		EcalEndcapCablesYDimension, EcalEndcapCablesXPosition, EcalEndcapCablesYPosition,
		theWorld,0);

  G4double TPCCables_PEThickness = db->fetchDouble("TPCCables_PEThickness");
  G4double TPCCables_CuThickness = db->fetchDouble("TPCCables_CuThickness");

  if(Ecal_cables_gap < (TPCCables_PEThickness + TPCCables_CuThickness + CoolingEcalStaves_MaxThickness))
      Control::Abort(
	"SServices_02_v00::BuildTPCAndEcalCables : TPCCablesThickness + CoolingEcalStaves_MaxThickness exceeds Ecal_cables_gap",
		MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

  G4double TPCCablesYDimension = Hcal_outer_radius - TPC_outer_radius;
  G4double TPCCablesXDimension = db->fetchDouble("TPCCables_Width");

  G4double XPositionFraction  = db->fetchDouble("TPCCables_XPosition");
  G4double TPCCablesXPosition = TPC_outer_radius*tan(pi/8.)*XPositionFraction;

  G4double cos_alpha = sqrt(1-XPositionFraction*XPositionFraction*sin(pi/8.)*sin(pi/8.));
  G4double TPCCablesYPosition = (Hcal_outer_radius + TPC_outer_radius)/2. -
			TPC_outer_radius*(1-cos_alpha);

  BuildCables(TPCCablesXDimension, TPCCables_PEThickness, TPCCables_CuThickness,
		TPCCablesYDimension, TPCCablesXPosition, TPCCablesYPosition,
		theWorld,0,1);
}

void
SServices_02_v00::
BuildCables(G4double Width, G4double PEThickness, G4double CuThickness,
		G4double YDimension, G4double XPosition, G4double YPosition,
		G4LogicalVolume *theWorld, G4int iZ, G4int iTPC)
{

  G4double CablesThickness = PEThickness+CuThickness;

  G4Box * CablesSolid = new G4Box("Barrel_Endcap_CablesSolid",
              Width/2.,
              YDimension/2.,
              CablesThickness/2.); 

    G4LogicalVolume * CablesLogical = 
        new G4LogicalVolume(CablesSolid,
                        CGAGeometryManager::GetMaterial("air"),
                        "Barrel_Endcap_CablesLogical",
                        0, 0, 0);

    CablesLogical->SetVisAttributes(VisAttAir);

    G4Box * Cables_PESolid = 
         new G4Box("Barrel_Endcap_Cables_PESolid",
	      Width/2.,
              YDimension/2.,
              PEThickness/2.); 

    G4LogicalVolume * Cables_PELogical = 
         new G4LogicalVolume(Cables_PESolid,
                        CGAGeometryManager::GetMaterial("polyethylene"),
                        "Barrel_Endcap_Cables_PELogical",
                        0, 0, 0);

    Cables_PELogical->SetVisAttributes(VisAttPE);

    new MyPlacement(0,
         G4ThreeVector(0,0,CablesThickness/2. - PEThickness/2.),
                  Cables_PELogical,
                  "Barrel_Endcap_Cables_PEPhysical",
                  CablesLogical,
                  false,0);

    G4Box * Cables_CuSolid = 
         new G4Box("Barrel_Endcap_Cables_CuSolid",
	      Width/2.,
              YDimension/2.,
              CuThickness/2.); 

    G4LogicalVolume * Cables_CuLogical = 
         new G4LogicalVolume(Cables_CuSolid,
                        CGAGeometryManager::GetMaterial("copper"),
                        "Barrel_Endcap_Cables_CuLogical",
                        0, 0, 0);

    Cables_CuLogical->SetVisAttributes(VisAttCu);

    new MyPlacement(0,
         G4ThreeVector(0,0,-CablesThickness/2.+CuThickness/2.),
                  Cables_CuLogical,
                  "Barrel_Endcap_Cables_CuPhysical",
                  CablesLogical,
                  false,0);

  G4double ZPosition = TPC_Ecal_Hcal_barrel_halfZ
			+CoolingEcalStaves_MaxThickness*iTPC+CablesThickness/2.;

  G4int iStart=-9, iEnd=-9;

  switch(iZ) {
	case -1 :
		iStart = -1;
		iEnd   = -1;
		break;
	case 0:
		iStart = -1;
		iEnd   = +1;
		break;
	case +1 :
		iStart = +1;
		iEnd   = +1;
		break;
	default:
		Control::Abort("SServices_02_v00::BuildCables bad value for function argument iZ",MOKKA_OTHER_ERRORS);
  };

  for(G4int iEndCap = iStart; iEndCap <= iEnd; iEndCap += 2)
//  for(G4int iEndCap = -1; iEndCap <= 1; iEndCap += 2)
    for (G4int stave_id = 1; stave_id < 9 ; stave_id++)
    {
/*
//GM
if(iTPC) 
G4cout << "TPC Cables: CoolingEcalStaves_Thickness = " << CoolingEcalStaves_Thickness << " stave_id = " << stave_id << " iEndCap = " << iEndCap << G4endl;
*/
	G4double phirot = (stave_id-1) * pi/4;
	G4RotationMatrix *rot=new G4RotationMatrix();
        if(iEndCap > 0) {
		rot->rotateY(pi);
		rot->rotateZ(-phirot);
	}
	else
		rot->rotateZ(phirot);

	G4ThreeVector stavePosition(XPosition, YPosition, ZPosition*iEndCap);

        if(iEndCap < 0) stavePosition[0] = - stavePosition[0];

	stavePosition.rotateZ(-phirot);

  	new MyPlacement(rot,
                  stavePosition,
                  CablesLogical,
                  "Barrel_Endcap_CablesPhysical",
                  theWorld,
                  false,stave_id
#ifdef MOKKA_DEBUG
,true);
#else
);
#endif

    }
}

void
SServices_02_v00::
BuildEcalCoolingServices(Database * db, G4LogicalVolume *theWorld)
{
//First build barrel cooling at -Z
   BuildEcalBarrelCooling_Staves_1_2(db, theWorld, -1);
   BuildEcalBarrelCooling_Staves_3_4_5_6(db, theWorld, -1);
   BuildEcalBarrelCooling_Staves_7_8(db, theWorld, -1);

//Then build barrel cooling at +Z
   BuildEcalBarrelCooling_Staves_1_2(db, theWorld, +1);
   BuildEcalBarrelCooling_Staves_3_4_5_6(db, theWorld, +1);
   BuildEcalBarrelCooling_Staves_7_8(db, theWorld, +1);

   BuildEcalEndcapCooling(db, theWorld);
}

void
SServices_02_v00::
BuildEcalBarrelCooling_Staves_1_2(Database * db, G4LogicalVolume *theWorld, G4int iZ)
{

//First place pipes along stave1
  G4double CoolingStave1_Length = Ecal_inner_radius*tan(pi/8.);

  if(iZ == -1)
  	db->exec("select * from ecal_cooling_ZMinus;" );
  else
  	db->exec("select * from ecal_cooling_ZPlus;" );

  db->getTuple();

  G4double CoolingStave1_Width = db->fetchDouble("CoolingStave1_Width");
  G4double CoolingStaves_Thickness = db->fetchDouble("CoolingStaves_Thickness");

  if(Ecal_cables_gap < CoolingStaves_Thickness)
      Control::Abort(
	"SServices_02_v00::BuildEcalBarrelCooling_Staves_1_2 : CoolingStaves_Thickness exceeds Ecal_cables_gap",
		MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

  CoolingEcalStaves_MaxThickness = 
	std::max(CoolingEcalStaves_MaxThickness,CoolingStaves_Thickness);

  G4Box * CoolingStave1Solid = 
         new G4Box("CoolingStave1Solid",
	      CoolingStave1_Length/2.,
              CoolingStave1_Width/2.,
              CoolingStaves_Thickness/2.); 

  G4LogicalVolume * CoolingStave1Logical = 
         new G4LogicalVolume(CoolingStave1Solid,
                        CGAGeometryManager::GetMaterial("copper"),
                        "CoolingStave1Logical",
                        0, 0, 0);

  CoolingStave1Logical->SetVisAttributes(VisAttCu);

  G4double XPosition = CoolingStave1_Length/2.;
  G4double YPosition = Ecal_inner_radius + CoolingStave1_Width/2.;
  G4double ZPosition = TPC_Ecal_Hcal_barrel_halfZ+CoolingStaves_Thickness/2.;

  new MyPlacement(0,
                  G4ThreeVector(XPosition*iZ,YPosition,ZPosition*iZ),
                  CoolingStave1Logical,
                  "CoolingStave1Physical",
                  theWorld,
                  false,iZ
#ifdef MOKKA_DEBUG
,true);
#else
);
#endif

//Then place pipes along staves 2 and 3
//Start with pipe 1 continuing down as pipe 2a till pipe 2b touches pipe 3
  G4double CoolingStave2_Width = CoolingStave1_Width;

  G4double deltaYDueToCoolingStave2 = CoolingStave1_Width+CoolingStave2_Width 
		- (Ecal_outer_radius-Ecal_inner_radius);

  if(deltaYDueToCoolingStave2 < 0)
		deltaYDueToCoolingStave2 = 0;

  if(iZ == -1)
  	deltaYDueToCoolingStave2_ZMinus = deltaYDueToCoolingStave2;
  else
  	deltaYDueToCoolingStave2_ZPlus = deltaYDueToCoolingStave2;

  G4double CoolingStave3_Width = db->fetchDouble("CoolingStaves3456_Width");
  G4double DeltaLength = (Ecal_inner_radius*tan(pi/8.) - CoolingStave3_Width/2.
				- CoolingStave1_Width - CoolingStave2_Width)
			/sin(pi/4.) + CoolingStave1_Width + CoolingStave2_Width;
  G4double CoolingStave2a_Length = Ecal_inner_radius*tan(pi/8.)*2 + DeltaLength;

  G4Box * CoolingStave2aSolid = 
         new G4Box("CoolingStave2aSolid",
	      CoolingStave2a_Length/2.,
              CoolingStave1_Width/2.,
              CoolingStaves_Thickness/2.); 

  G4LogicalVolume * CoolingStave2aLogical = 
         new G4LogicalVolume(CoolingStave2aSolid,
                        CGAGeometryManager::GetMaterial("copper"),
                        "CoolingStave2aLogical",
                        0, 0, 0);

  CoolingStave2aLogical->SetVisAttributes(VisAttCu);

//pipe 2b starts at middle of stave 2 and ends in stave 3
  G4double CoolingStave2b_Length = Ecal_inner_radius*tan(pi/8.) + DeltaLength;

  G4Box * CoolingStave2bSolid = 
         new G4Box("CoolingStave2bSolid",
	      CoolingStave2b_Length/2.,
              CoolingStave2_Width/2.,
              CoolingStaves_Thickness/2.); 

  G4LogicalVolume * CoolingStave2bLogical = 
         new G4LogicalVolume(CoolingStave2bSolid,
                        CGAGeometryManager::GetMaterial("copper"),
                        "CoolingStave2bLogical",
                        0, 0, 0);

  CoolingStave2bLogical->SetVisAttributes(VisAttCu);

  XPosition = DeltaLength/2.;
  YPosition = Ecal_inner_radius + CoolingStave1_Width/2.;
  ZPosition = TPC_Ecal_Hcal_barrel_halfZ+CoolingStaves_Thickness/2.;


  G4double bXPosition = CoolingStave2b_Length/2.;
  G4double bYPosition = Ecal_inner_radius + CoolingStave1_Width + CoolingStave2_Width/2.;

  G4ThreeVector Stave2aPosition(XPosition*iZ,YPosition,ZPosition*iZ);
  G4ThreeVector Stave2bPosition(bXPosition*iZ,bYPosition,ZPosition*iZ);

  G4int stave_id = 2;

  G4double phirot = (stave_id-1) * pi/4;
  G4RotationMatrix *rot=new G4RotationMatrix();
  if(iZ > 0) {
		rot->rotateZ(phirot);
		Stave2aPosition.rotateZ(-phirot);
		Stave2bPosition.rotateZ(-phirot);
  }
  else {
		rot->rotateZ(-phirot);
		Stave2aPosition.rotateZ(phirot);
		Stave2bPosition.rotateZ(phirot);
  }

  new MyPlacement(rot,
                  Stave2aPosition,
                  CoolingStave2aLogical,
                  "CoolingStave2aPhysical",
                  theWorld,
                  false,stave_id*iZ
#ifdef MOKKA_DEBUG
,true);
#else
);
#endif

  new MyPlacement(rot,
                  Stave2bPosition,
                  CoolingStave2bLogical,
                  "CoolingStave2bPhysical",
                  theWorld,
                  false,stave_id*iZ
#ifdef MOKKA_DEBUG
,true);
#else
);
#endif
  
  G4double deltaY2 = (CoolingStave1_Width+CoolingStave2_Width)*sin(pi/4.);
  G4double deltaY3 = Ecal_inner_radius*tan(pi/8.) - CoolingStave3_Width/2.
	- CoolingStave1_Width - CoolingStave2_Width + deltaY2;
  G4double xStartPipes_1_2_InStave3 = deltaY3 + deltaY2;

  G4double CoolingStaves_1_2_in_Stave_3_XDimension = Hcal_outer_radius 
		- Ecal_inner_radius - xStartPipes_1_2_InStave3;
  G4double CoolingStaves_1_2_in_Stave_3_YDimension = 
		CoolingStave1_Width+CoolingStave2_Width;

  G4Box * CoolingStave12_inStave3_Solid = 
         new G4Box("CoolingStave12_inStave3_Solid",
	      CoolingStaves_1_2_in_Stave_3_XDimension/2.,
              CoolingStaves_1_2_in_Stave_3_YDimension/2.,
              CoolingStaves_Thickness/2.); 

  G4LogicalVolume * CoolingStave12_inStave3_Logical = 
         new G4LogicalVolume(CoolingStave12_inStave3_Solid,
                        CGAGeometryManager::GetMaterial("copper"),
                        "CoolingStave12_inStave3_Logical",
                        0, 0, 0);

  CoolingStave12_inStave3_Logical->SetVisAttributes(VisAttCu);

  XPosition = (Hcal_outer_radius+Ecal_inner_radius
			+xStartPipes_1_2_InStave3)/2.;
  YPosition = (CoolingStave1_Width+CoolingStave2_Width+CoolingStave3_Width)/2.;
  ZPosition = TPC_Ecal_Hcal_barrel_halfZ+CoolingStaves_Thickness/2.;

  new MyPlacement(0,
  		G4ThreeVector(XPosition*iZ,YPosition,ZPosition*iZ),
                CoolingStave12_inStave3_Logical,
                "CoolingStave12_inStave3_Physical",
                theWorld,
                false,3*iZ
#ifdef MOKKA_DEBUG
,true);
#else
);
#endif

}

void
SServices_02_v00::
BuildEcalBarrelCooling_Staves_3_4_5_6(Database * db, G4LogicalVolume *theWorld, G4int iZ)
{
  G4double CoolingStaves3456_YDimension = Hcal_outer_radius - Ecal_outer_radius;

  if(iZ == -1)
  	db->exec("select * from ecal_cooling_ZMinus;" );
  else
  	db->exec("select * from ecal_cooling_ZPlus;" );

  db->getTuple();

  G4double CoolingStaves3456_XDimension = db->fetchDouble("CoolingStaves3456_Width");
  G4double CoolingStaves_Thickness = db->fetchDouble("CoolingStaves_Thickness");

  G4Box * CoolingStaves3456_Solid = 
         new G4Box("CoolingStaves3456_Solid",
	      CoolingStaves3456_XDimension/2.,
              CoolingStaves3456_YDimension/2.,
              CoolingStaves_Thickness/2.); 

  G4LogicalVolume * CoolingStaves3456_Logical = 
         new G4LogicalVolume(CoolingStaves3456_Solid,
                        CGAGeometryManager::GetMaterial("copper"),
                        "CoolingStaves3456_Logical",
                        0, 0, 0);

  CoolingStaves3456_Logical->SetVisAttributes(VisAttCu);

  G4double ZPosition = TPC_Ecal_Hcal_barrel_halfZ+CoolingStaves_Thickness/2.;

    for (G4int stave_id = 3; stave_id <= 6 ; stave_id++)
    {
	G4ThreeVector sPosition(0,(Hcal_outer_radius+Ecal_outer_radius)/2.,
						 ZPosition*iZ);
	G4double phirot = (stave_id-1) * pi/4;
	G4RotationMatrix *rot=new G4RotationMatrix();
        if(iZ > 0) {
		rot->rotateZ(phirot);
		sPosition.rotateZ(-phirot);
	}
	else {
		rot->rotateZ(-phirot);
		sPosition.rotateZ(phirot);
	}

  	new MyPlacement(rot,
                  sPosition,
                  CoolingStaves3456_Logical,
                  "CoolingStaves3456_Physical",
                  theWorld,
                  false,stave_id*iZ
#ifdef MOKKA_DEBUG
,true);
#else
);
#endif

    }
}

void
SServices_02_v00::
BuildEcalBarrelCooling_Staves_7_8(Database * db, G4LogicalVolume *theWorld, G4int iZ)
{

  if(iZ == -1)
  	db->exec("select * from ecal_cooling_ZMinus;" );
  else
  	db->exec("select * from ecal_cooling_ZPlus;" );

  db->getTuple();

  G4double CoolingStaves78_YDimension = db->fetchDouble("CoolingStaves78_Width");
  G4double CoolingStave8_XDimension = Ecal_inner_radius*tan(pi/8.);
  G4double CoolingStaves_Thickness = db->fetchDouble("CoolingStaves_Thickness");

  G4Box * CoolingStave8_Solid = 
         new G4Box("CoolingStave8_Solid",
	      CoolingStave8_XDimension/2.,
              CoolingStaves78_YDimension/2.,
              CoolingStaves_Thickness/2.); 

  G4LogicalVolume * CoolingStave8_Logical = 
         new G4LogicalVolume(CoolingStave8_Solid,
                        CGAGeometryManager::GetMaterial("copper"),
                        "CoolingStaves8_Logical",
                        0, 0, 0);

  CoolingStave8_Logical->SetVisAttributes(VisAttCu);

  G4double XPosition = -CoolingStave8_XDimension/2.;
  G4double YPosition = Ecal_outer_radius - CoolingStaves78_YDimension/2.;
  G4double ZPosition = TPC_Ecal_Hcal_barrel_halfZ+CoolingStaves_Thickness/2.;

  G4int stave_id = 8;

        G4ThreeVector Stave8Position(XPosition*iZ,YPosition,ZPosition*iZ);

	G4double phirot = (stave_id-1) * pi/4;
	G4RotationMatrix *rot=new G4RotationMatrix();
        if(iZ > 0) {
		rot->rotateZ(phirot);
		Stave8Position.rotateZ(-phirot);
	}
	else {
		rot->rotateZ(-phirot);
		Stave8Position.rotateZ(phirot);
	}

  	new MyPlacement(rot,
                  Stave8Position,
                  CoolingStave8_Logical,
                  "CoolingStave8_Physical",
                  theWorld,
                  false,stave_id*iZ
#ifdef MOKKA_DEBUG
,true);
#else
);
#endif

  G4double CoolingStave7_XDimension = Ecal_outer_radius*tan(pi/8.);

  G4Box * CoolingStave7_Solid = 
         new G4Box("CoolingStave7_Solid",
	      CoolingStave7_XDimension/2.,
              CoolingStaves78_YDimension/2.,
              CoolingStaves_Thickness/2.); 

  G4LogicalVolume * CoolingStave7_Logical = 
         new G4LogicalVolume(CoolingStave7_Solid,
                        CGAGeometryManager::GetMaterial("copper"),
                        "CoolingStaves7_Logical",
                        0, 0, 0);

  CoolingStave7_Logical->SetVisAttributes(VisAttCu);

  XPosition = CoolingStave7_XDimension/2.;
  YPosition = Ecal_outer_radius - CoolingStaves78_YDimension/2.;
  ZPosition = TPC_Ecal_Hcal_barrel_halfZ+CoolingStaves_Thickness/2.;

  stave_id = 7;
  
        G4ThreeVector Stave7Position(XPosition*iZ,YPosition,ZPosition*iZ);

	phirot = (stave_id-1) * pi/4;
	rot=new G4RotationMatrix();
        if(iZ > 0) {
		rot->rotateZ(phirot);
		Stave7Position.rotateZ(-phirot);
	}
	else {
		rot->rotateZ(-phirot);
		Stave7Position.rotateZ(phirot);
	}

  	new MyPlacement(rot,
                  Stave7Position,
                  CoolingStave7_Logical,
                  "CoolingStave7_Physical",
                  theWorld,
                  false,stave_id*iZ
#ifdef MOKKA_DEBUG
,true);
#else
);
#endif

  G4double CoolingStaves78_inStave7_YDimension = Hcal_outer_radius - Ecal_outer_radius;
  G4double CoolingStaves78_inStave7_XDimension = db->fetchDouble("CoolingStaves78_inStave7_Width");

  G4Box * CoolingStaves78_inStave7_Solid = 
         new G4Box("CoolingStaves78_inStave7_Solid",
	      CoolingStaves78_inStave7_XDimension/2.,
              CoolingStaves78_inStave7_YDimension/2.,
              CoolingStaves_Thickness/2.); 

  G4LogicalVolume * CoolingStaves78_inStave7_Logical = 
         new G4LogicalVolume(CoolingStaves78_inStave7_Solid,
                        CGAGeometryManager::GetMaterial("copper"),
                        "CoolingStaves78_inStave7_Logical",
                        0, 0, 0);

  CoolingStaves78_inStave7_Logical->SetVisAttributes(VisAttCu);

  XPosition = 0;
  YPosition = (Hcal_outer_radius + Ecal_outer_radius)/2.;
  ZPosition = TPC_Ecal_Hcal_barrel_halfZ+CoolingStaves_Thickness/2.;

  stave_id = 7;
  
        G4ThreeVector Stave7bPosition(XPosition,YPosition,ZPosition*iZ);

	phirot = (stave_id-1) * pi/4;
	rot=new G4RotationMatrix();
        if(iZ > 0) {
		rot->rotateZ(phirot);
		Stave7bPosition.rotateZ(-phirot);
	}
	else {
		rot->rotateZ(-phirot);
		Stave7bPosition.rotateZ(phirot);
	}

  	new MyPlacement(rot,
                  Stave7bPosition,
                  CoolingStaves78_inStave7_Logical,
                  "CoolingStaves78_inStave7_Physical",
                  theWorld,
                  false,stave_id*iZ
#ifdef MOKKA_DEBUG
,true);
#else
);
#endif
}

void
SServices_02_v00::
BuildEcalEndcapCooling(Database * db, G4LogicalVolume *theWorld)
{
  G4double EndcapCooling_YDimension = Hcal_outer_radius - Ecal_outer_radius;

  db->exec("select * from ecal_endcap_cooling;" );
  db->getTuple();

  G4double EndcapCooling_width = db->fetchDouble("EndcapCooling_width");
  G4double EndcapCooling_thickness = db->fetchDouble("EndcapCooling_thickness");

  if(Ecal_cables_gap < EndcapCooling_thickness)
      Control::Abort(
	"SServices_02_v00::BuildEcalEndcapCooling : EndcapCooling_thickness exceeds Ecal_cables_gap",
		MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

  G4Box * CoolingSolid = 
         new G4Box("EcalEndcapCoolingSolid",
	      EndcapCooling_width/2.,
              EndcapCooling_YDimension/2.,
              EndcapCooling_thickness/2.); 

  G4LogicalVolume * CoolingLogical = 
         new G4LogicalVolume(CoolingSolid,
                        CGAGeometryManager::GetMaterial("copper"),
                        "EcalEndcapCoolingLogical",
                        0, 0, 0);

  CoolingLogical->SetVisAttributes(VisAttCu);

  G4double XPosition = Ecal_outer_radius*tan(pi/8.);
  G4double YPosition = (Hcal_outer_radius + Ecal_outer_radius)/2.;

  G4double ZPosition = -TPC_Ecal_Hcal_barrel_halfZ-EndcapCooling_thickness/2.;

 for(G4int iXPos = -1; iXPos <= 1; iXPos += 2)
 {
  for(G4int iEndCap = -1; iEndCap <= 1; iEndCap += 2)
  {
    for (G4int stave_id = 4; stave_id <= 6 ; stave_id += 2)
    {
	G4double phirot = (stave_id-1) * pi/4;
	G4RotationMatrix *rot=new G4RotationMatrix();
        if(ZPosition > 0) {
		rot->rotateY(pi);
		rot->rotateZ(-phirot);
	}
	else
		rot->rotateZ(phirot);

	G4ThreeVector stavePosition(XPosition, YPosition, ZPosition);

        if(ZPosition > 0) stavePosition[0] = - stavePosition[0];

	stavePosition.rotateZ(-phirot);

  	new MyPlacement(rot,
                  stavePosition,
                  CoolingLogical,
                  "EcalEndcapCoolingPhysical",
                  theWorld,
                  false,stave_id
#ifdef MOKKA_DEBUG
,true);
#else
);
#endif

    }
    ZPosition = -ZPosition;
  }
  XPosition = -XPosition;
 }

}
