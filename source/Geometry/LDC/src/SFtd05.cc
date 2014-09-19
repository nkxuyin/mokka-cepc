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
// SFtd05.cc
// 
// Fixes a bug in SFtd04.cc which meant that the copper cables on thin inside of the cylinder were far to thick (SJA 28/05/09)
//
// Implementation of a self scaling 7 disk FTD
// first 3 Disks are Si-pixel technology
// last 4 Disks are Si-strip technology
// Support material Kapton

// All disks:
// Dimentions and coordinates are specified for the sensitive layer, support disks are built on to these
// inner_radius = (  beamTubeRadius + beamTubeClearance)

// First Disk:
// z defined by distance from end of VTX layer 3
// outer r defined by radial difference to SIT layer 1

// Second Disk:
// z defined relative to TPC half-length: to ensure positioning with SIT set these numbers to the same value in DB
// outer r defined by radial difference to SIT layer 1

// Third Disk:
// z defined relative to TPC half-length: to ensure positioning with SIT set these numbers to the same value in DB
// outer r defined by radial difference to SIT layer 1

// Fourth, Fifth and Sixth Disk:
// z defined relative to TPC half-length
// outer r defined by gap between TPC inner radius and FTD disks

// Last Disk:
// z defined by distance from front of ECal endcap
// outer r defined by gap between TPC inner radius and FTD disks

// Parameters Set in Model Parameter DB Table:
// TPC_Ecal_Hcal_barrel_halfZ
// Ecal_endcap_zmin
// TPC_inner_radius
// VXD_length_r3

// Parameters shared with other drivers:
// SSit03:SIT1_Half_Length_Z
// SSit03:SIT2_Half_Length_Z 
// SSit03:SIT1_Radius 
// SSit03:SIT2_Radius 
// TubeX01:TUBE_IPOuterTube_end_z
// TubeX01:TUBE_IPOuterTube_end_radius
// TubeX01:TUBE_IPOuterBulge_end_z
// TubeX01:TUBE_IPOuterBulge_end_radius

// October 15th 2008, Steve Aplin using description from SiLC Collaboration

// History:  
// - first implementation P. Mora de Freitas (sept 02)
// - fixed geometry overlap -- Adrian Vogel, 2005-12-05
// - implemented new GEAR interface -- K. Harder, T. Pinto Jayawardena  2007-07-31
// - SFtd03: Modified version of SFtd02: Rewritten as a self scaling driver which does not
//   make use of a seperate super driver. Steve Aplin (May 2008)
// - Fixes a bug in SFtd04.cc which meant that the copper cables on thin inside of the cylinder were far to thick (SJA 28/05/09)

#include "Control.hh"
#include "G4PVPlacement.hh"
#include "SFtd05.hh"
#include "TRKSD00.hh"
#include "CGAGeometryManager.hh"

#include "globals.hh"

#include "G4Cons.hh"
#include "G4Tubs.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4RotationMatrix.hh"

#include "MySQLWrapper.hh"
#include <assert.h>

#include "CGADefs.h"

#ifdef MOKKA_GEAR
#include "gearimpl/GearParametersImpl.h"
#include "MokkaGear.h"
#include "G4EmCalculator.hh"
#include "G4ParticleTable.hh"
#endif 

INSTANTIATE(SFtd05)

G4bool SFtd05::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog)

{

  G4VisAttributes *VisAttDisks = new G4VisAttributes(G4Colour(1.,1.,.8));
  //VisAttDisks->SetForceWireframe(true);
  //VisAttDisks->SetForceSolid(true);

  G4VisAttributes *VisAttSuport = new G4VisAttributes(G4Colour(1,.5,.5));
  //VisAttSuport->SetForceWireframe(true);
  //VisAttSuport->SetForceSolid(true);

  G4VisAttributes *VisAttCyl = new G4VisAttributes(G4Colour(0.45,.2,0.9));
  //VisAttCyl->SetForceWireframe(true);
  //VisAttCyl->SetForceSolid(true);

  G4VisAttributes *VisAttCables = new G4VisAttributes(G4Colour(0.,0.9,0.));
  //VisAttCables->SetForceWireframe(true);
  //VisAttCables->SetForceSolid(true); 

  G4RotationMatrix *rot=new G4RotationMatrix();
  rot->rotateX(pi); // the same but other side

  G4double start_phy = 0.*deg;
  G4double stop_phy = 360.*deg;

  G4PVPlacement *Phys;

  // Now get the Globals from the surrounding environment TPC ECAL SIT VTX and Beam-Pipe  

  const G4double TPC_Ecal_Hcal_barrel_halfZ = env.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");
  const G4double Ecal_endcap_zmin = env.GetParameterAsDouble("Ecal_endcap_zmin");
  const G4double TPC_inner_radius = env.GetParameterAsDouble("TPC_inner_radius");

  const G4double SIT1_Half_Length_Z = env.GetParameterAsDouble("SIT1_Half_Length_Z");
  const G4double SIT2_Half_Length_Z = env.GetParameterAsDouble("SIT2_Half_Length_Z");
  const G4double SIT1_Radius = env.GetParameterAsDouble("SIT1_Radius");
  const G4double SIT2_Radius = env.GetParameterAsDouble("SIT2_Radius");
  const G4double VXD_layer3_maxZ = env.GetParameterAsDouble("VXD_length_r3");
  
  const G4double zEnd_IPOuterTube = (env.GetParameterAsDouble("TUBE_IPOuterTube_end_z"));
  const G4double rEnd_IPOuterTube = (env.GetParameterAsDouble("TUBE_IPOuterTube_end_radius"));
  const G4double zEnd_IPOuterBulge = (env.GetParameterAsDouble("TUBE_IPOuterBulge_end_z"));
  const G4double rEnd_IPOuterBulge = (env.GetParameterAsDouble("TUBE_IPOuterBulge_end_radius"));
  
  const G4double beamTubeTangent = ( rEnd_IPOuterBulge - rEnd_IPOuterTube ) / (zEnd_IPOuterBulge - zEnd_IPOuterTube);

  // Now get the variables global to the FTD cables_thickness, ftd1_vtx3_distance_z, etc

  db = new Database(env.GetDBName());
  db->exec("select * from common_parameters;");
  db->getTuple();

  const G4double beamTubeClearance = db->fetchDouble("beamtube_clearance"); 
  outer_cylinder_total_thickness = db->fetchDouble("outer_cylinder_total_thickness") * mm;
  G4double inner_cylinder_total_thickness = outer_cylinder_total_thickness;
  cable_shield_thickness = db->fetchDouble("cable_shield_thickness") * mm;
  cables_thickness = db->fetchDouble("cables_thickness") * mm;

  // check that there is enough space for the cables and support
  if(beamTubeClearance < (cables_thickness + (2.0*cable_shield_thickness) + 0.5 *mm) ) {
    G4cout << "SFtd05:Stop: Not enough space for inner support structure and cables: increase beamTubeClearance" << G4endl;
    exit(1);
  }

  const G4double ftd1_vtx3_distance_z =  db->fetchDouble("ftd1_vtx3_distance_z"); 
  const G4double ftd7_ecal_distance_z =  db->fetchDouble("ftd7_ecal_distance_z"); 
  const G4double ftd1_sit1_radial_diff =  db->fetchDouble("ftd1_sit1_radial_diff"); 
  const G4double ftd2_sit1_radial_diff =  db->fetchDouble("ftd2_sit1_radial_diff"); 
  const G4double ftd3_sit2_radial_diff =  db->fetchDouble("ftd3_sit2_radial_diff"); 
  const G4double ftd4to7_tpc_radial_gap =  db->fetchDouble("ftd4to7_tpc_radial_gap"); 

  // Now we can start to build the disks 

  db->exec("select * from disks;");
  db->getTuple();

  double minDiskThickness (MAXFLOAT);
  do 
    {
      Disks_Si_thickness = db->fetchDouble("disk_si_thickness") * mm ;
      if(minDiskThickness>Disks_Si_thickness) minDiskThickness = Disks_Si_thickness;
    }  while(db->getTuple()!=NULL);

  //... The sensitive layer
  //... Threshold is 20% of a MIP. For Si we have 340 KeV/mm as MIP.
  

  theFTDSD = 
    new TRKSD00("FTD", minDiskThickness * 340 * keV/mm * 0.2);
     
  RegisterSensitiveDetector(theFTDSD);
     
  //... Disks
  db->exec("select * from disks;");
  db->getTuple();
     
  ZStartOuterCylinder=0;
  ZStopOuterCylinder=0;
  G4double OuterCylinderInnerRadius=0;
     
  ZStartInnerCylinder=0;
  ZStopInnerCylinder=0;
  G4double InnerCylinderOuterRadius1=0;
  G4double InnerCylinderOuterRadius2=0;
     
  SiMat = CGAGeometryManager::GetMaterial("silicon_2.33gccm");
  KaptonMat = CGAGeometryManager::GetMaterial("kapton");
  CuMat = CGAGeometryManager::GetMaterial("copper"); 
  
  G4cout << "SFtd05:" 
	 << "\t inner support thickness = " << inner_cylinder_total_thickness 
	 << "\t cables thickness = " << cables_thickness
	 << "\t 2 x cable shield thickness = " << 2 * cable_shield_thickness
	 << "\t beamTubeClearance = " << beamTubeClearance
	 << G4endl;
     
  //... assembling detector
  do 
    {
      //... Get the disk parameters
      G4int disk_number(-1);
      G4double inner_radius(0.0);
      G4double outer_radius(0.0);
      G4double z_position(0.0);
      G4double beamTubeRadius(0.0);
      G4double zEnd(0.0);

      disk_number = db->fetchInt("disk_number");
      Disks_Si_thickness = db->fetchDouble("disk_si_thickness") * mm ;
      Disks_Support_thickness = db->fetchDouble("disk_support_thickness") * mm ;

      // push back values to be stored in GEAR
      Disks_Si_thicknessVec.push_back(Disks_Si_thickness);
      Disks_Support_thicknessVec.push_back(Disks_Support_thickness);
                  
      switch (disk_number) {

      case 1:
	// z defined by distance from end of VTX layer 3
	z_position = VXD_layer3_maxZ + ftd1_vtx3_distance_z;
	z_positionVec.push_back(z_position);
	// outer r defined by radial difference to SIT layer 1
	outer_radius = SIT1_Radius + ftd1_sit1_radial_diff; 
	outer_radiusVec.push_back(outer_radius);
	// beam tube radius at backside of disk
	zEnd = (z_position + (Disks_Si_thickness*0.5) + Disks_Support_thickness );
	// check which part of the beam tube this disk lies above
	beamTubeRadius = (zEnd < zEnd_IPOuterTube ) ? rEnd_IPOuterTube : rEnd_IPOuterTube + ( (zEnd - zEnd_IPOuterTube ) * beamTubeTangent );
	inner_radius = (  beamTubeRadius + beamTubeClearance) * mm;
	inner_radiusVec.push_back(inner_radius);
	
	// check that there is no overlap with SIT1
	if( z_position <= SIT1_Half_Length_Z && outer_radius>=SIT1_Radius) {
	  G4cout << "SFtd05:Stop: Overlap between FTD1 and SIT1" << G4endl;
	  G4cout << "SFtd05:FTD1 Radius = " << outer_radius << "SIT1 Radius = " << SIT1_Radius << G4endl;
	  exit(1);
	}
	if( db->fetchDouble("z_position_ReltoTPCLength") != 0.0) {
	  G4cout << "SFtd05:Stop: The z position of FTD1 is not relative. The relative value will not be used. It should be set to 0.0 in the DB." << G4endl;
	  G4cout << "SFtd05:Stop: The z position of FTD1 is set by the distance between the centre of the sensitive layer and the max z of VTX layer 3." << G4endl;
	  exit(1);
	}

	break;
      
      case 2:
	// z defined relative to TPC half-length: to ensure positioning with SIT set these numbers to the same value in DB
	z_position = (TPC_Ecal_Hcal_barrel_halfZ * db->fetchDouble("z_position_ReltoTPCLength")) * mm;
	z_positionVec.push_back(z_position);
	// outer r defined by radial difference to SIT layer 1
	outer_radius = SIT1_Radius + ftd2_sit1_radial_diff; 
	outer_radiusVec.push_back(outer_radius);
	// beam tube radius at backside of disk
	zEnd = (z_position + (Disks_Si_thickness*0.5) + Disks_Support_thickness );
	// check which part of the beam tube this disk lies above
	beamTubeRadius = (zEnd < zEnd_IPOuterTube ) ? rEnd_IPOuterTube : rEnd_IPOuterTube + ( (zEnd - zEnd_IPOuterTube ) * beamTubeTangent );
	inner_radius = (  beamTubeRadius + beamTubeClearance) * mm;
	inner_radiusVec.push_back(inner_radius);

	//... keep information for inner support cylinder with 0.5mm saftey clearance from inner radius of disks
	ZStartInnerCylinder = zEnd_IPOuterTube;
	InnerCylinderOuterRadius1 = inner_radius - ( ( zEnd - zEnd_IPOuterTube ) * beamTubeTangent ) - 0.5 * mm; 

	// check that there is no overlap with SIT1
	if( z_position <= SIT1_Half_Length_Z && outer_radius>=SIT1_Radius) {
	  G4cout << "SFtd05:Stop:Overlap between FTD2 and SIT1" << G4endl;
	  G4cout << "SFtd05:FTD2 Radius = " << outer_radius << "SIT1 Radius = " << SIT1_Radius << G4endl;
	  exit(1);
	}
	break;

      case 3:
	// z defined relative to TPC half-length: to ensure positioning with SIT set these numbers to the same value in DB
	z_position = (TPC_Ecal_Hcal_barrel_halfZ * db->fetchDouble("z_position_ReltoTPCLength")) * mm;
	z_positionVec.push_back(z_position);
	// outer r defined by radial difference to SIT layer 2
	outer_radius = SIT2_Radius + ftd3_sit2_radial_diff; 
	outer_radiusVec.push_back(outer_radius);
	// beam tube radius at backside of disk
	zEnd = (z_position + (Disks_Si_thickness*0.5) + Disks_Support_thickness );
	// check which part of the beam tube this disk lies above
	beamTubeRadius = (zEnd < zEnd_IPOuterTube ) ? rEnd_IPOuterTube : rEnd_IPOuterTube + ( (zEnd - zEnd_IPOuterTube ) * beamTubeTangent );
	inner_radius = (  beamTubeRadius + beamTubeClearance) * mm;
	inner_radiusVec.push_back(inner_radius);

	// check that there is no overlap with SIT1
	if( z_position <= SIT2_Half_Length_Z && outer_radius>=SIT2_Radius) {
	  G4cout << "SFtd05:Stop:Overlap between FTD3 and SIT2" <<  G4endl;
	  G4cout << "SFtd05:FTD3 Radius = " << outer_radius << "SIT2 Radius = " << SIT2_Radius << G4endl;
	  exit(1);
	}
	break;

      case 4:
      case 5:
      case 6:
	// z defined relative to TPC half-length
	z_position = (TPC_Ecal_Hcal_barrel_halfZ * db->fetchDouble("z_position_ReltoTPCLength")) * mm;
	z_positionVec.push_back(z_position);
	// outer r defined by gap between TPC inner radius and FTD disks
	outer_radius = TPC_inner_radius - ftd4to7_tpc_radial_gap; 
	outer_radiusVec.push_back(outer_radius);
	// beam tube radius at backside of disk
	zEnd = (z_position + (Disks_Si_thickness*0.5) + Disks_Support_thickness );
	// check which part of the beam tube this disk lies above
	beamTubeRadius = (zEnd < zEnd_IPOuterTube ) ? rEnd_IPOuterTube : rEnd_IPOuterTube + ( (zEnd - zEnd_IPOuterTube ) * beamTubeTangent );
	inner_radius = (  beamTubeRadius + beamTubeClearance) * mm;
	inner_radiusVec.push_back(inner_radius);
	
	// keep the information for outer cylinder
	if(disk_number==4) ZStartOuterCylinder = z_position;
	break;

      case 7:
	// z defined by distance from front of ECal endcap
	z_position = Ecal_endcap_zmin - ftd7_ecal_distance_z;
	z_positionVec.push_back(z_position);
	// outer r defined by gap between TPC inner radius and FTD disks
	outer_radius = TPC_inner_radius - ftd4to7_tpc_radial_gap; 
	outer_radiusVec.push_back(outer_radius);
	// beam tube radius at backside of disk
	zEnd = (z_position + (Disks_Si_thickness*0.5) + Disks_Support_thickness );
	// check which part of the beam tube this disk lies above
	beamTubeRadius = (zEnd < zEnd_IPOuterTube ) ? rEnd_IPOuterTube : rEnd_IPOuterTube + ( (zEnd - zEnd_IPOuterTube ) * beamTubeTangent );
	inner_radius = (  beamTubeRadius + beamTubeClearance) * mm;
	inner_radiusVec.push_back(inner_radius);
	
	// End of Support Structure: 0.5mm clearance from disks
	ZStopOuterCylinder = zEnd;
	ZStopInnerCylinder = zEnd;
	OuterCylinderInnerRadius = outer_radius + 0.5 * mm;
	InnerCylinderOuterRadius2 = inner_radius - 0.5 * mm; 

	if( db->fetchDouble("z_position_ReltoTPCLength") != 0.0) {
	  G4cout << "SFtd05:Stop: The z position of FTD7 is not relative. The relative value will not be used. It should be set to 0.0 in the DB." << G4endl;
	  G4cout << "SFtd05:Stop: The z position of FTD7 is set by the distance between the centre of the sensitive layer and the min z of the ECal Endcap." << G4endl;
	  exit(1);
	}
	break;

      default:
	G4cout << "SFtd05: Error disk number must be between 1-7: disk number = " << disk_number << G4endl;
	exit(1);
      }

      G4cout << "SFtd05: Disk:" << disk_number
		<< "\t z = " << z_position
		<< "\t inner rad = " << inner_radius
		<< "\t outer rad = " << outer_radius
		<< "\t beamtube rad = " << beamTubeRadius
		<< "\t free space = " << (inner_radius - 0.5 * mm - inner_cylinder_total_thickness - (2*cable_shield_thickness) - cables_thickness) - beamTubeRadius 
		<< G4endl;

    
      G4Material *DiskMaterial;
      DiskMaterial = SiMat;

      //... Si sensitive
      G4Tubs *FTDSiDiskSolid
	= new G4Tubs("FTDSiDisk",
		     inner_radius,
		     outer_radius,
		     Disks_Si_thickness/2.,
		     start_phy,
		     stop_phy);

      G4LogicalVolume *FTDSiDiskLogical=
	new G4LogicalVolume(FTDSiDiskSolid,
			    DiskMaterial,
			    "FTDSiDisk", 
			    0, 
			    0, 
			    0);
      
      FTDSiDiskLogical->SetVisAttributes(VisAttDisks);
      FTDSiDiskLogical->SetSensitiveDetector(theFTDSD);

      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., z_position),
			  FTDSiDiskLogical,
			  "FTDSiDisk",
			  worldLog,
			  false,
			  disk_number);      
      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., -z_position),
			  FTDSiDiskLogical,
			  "FTDSiDisk",
			  worldLog,
			  false,
			  -disk_number);      

      //... Support
      G4Tubs *FTDDiskSupportSolid
	= new G4Tubs("FTDDiskSupport",
		     inner_radius,
		     outer_radius,
		     Disks_Support_thickness/2.,
		     start_phy, 
		     stop_phy);
      
      G4LogicalVolume *FTDDiskSupportLogical=
	new G4LogicalVolume(FTDDiskSupportSolid,
			    KaptonMat,
			    "FTDDiskSupport", 
			    0, 
			    0, 
			    0);
      

      FTDDiskSupportLogical->SetVisAttributes(VisAttSuport);

      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., 
					z_position
					+Disks_Si_thickness/2.
					+Disks_Support_thickness/2.),
			  FTDDiskSupportLogical,
			  "FTDDiskSupport",
			  worldLog,
			  false,0);      
      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., 
					-z_position
					-Disks_Si_thickness/2.
					-Disks_Support_thickness/2.),
			  FTDDiskSupportLogical,
			  "FTDDiskSupport",
			  worldLog,
			  false,0);      
      

    } while(db->getTuple()!=NULL);
  
  //... Outer cylinder 

  assert(ZStartOuterCylinder>0);
  assert(ZStopOuterCylinder>0);

  G4double OuterCylinder_half_z = (ZStopOuterCylinder-ZStartOuterCylinder)/2.;
  assert(OuterCylinder_half_z>0);
  
  G4double OuterCylinder_position =
    ZStartOuterCylinder + OuterCylinder_half_z;
  
  G4Tubs *FTDOuterCylinderSolid
    = new G4Tubs("FTDOuterCylinder",
		 OuterCylinderInnerRadius,
		 OuterCylinderInnerRadius
		 +outer_cylinder_total_thickness,
		 OuterCylinder_half_z,
		 start_phy, 
		 stop_phy);
  
  G4LogicalVolume *FTDOuterCylinderLogical=
    new G4LogicalVolume(FTDOuterCylinderSolid,
			KaptonMat,
			"FTDOuterCylinder", 
			0, 
			0, 
			0);
  
  FTDOuterCylinderLogical->SetVisAttributes(VisAttCyl);
  

  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., OuterCylinder_position),
		      FTDOuterCylinderLogical,
		      "FTDOuterCylinder",
		      worldLog,
		      false,0);      
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., -OuterCylinder_position),
		      FTDOuterCylinderLogical,
		      "FTDOuterCylinder",
		      worldLog,
		      false,0);


  //... Inner cylinder (cone)

  assert(ZStartInnerCylinder>0);
  assert(ZStopInnerCylinder>0);
  
  G4double InnerCylinder_half_z = 
    (ZStopInnerCylinder-ZStartInnerCylinder)/2.;
  assert(InnerCylinder_half_z>0);
  
  G4double InnerCylinder_position =
    ZStartInnerCylinder + InnerCylinder_half_z;
  


  G4double InnerCylinderRmin1 = InnerCylinderOuterRadius1 - inner_cylinder_total_thickness - (2.0*cable_shield_thickness) - cables_thickness ;
  G4double InnerCylinderRmax1 = InnerCylinderOuterRadius1;
  G4double InnerCylinderRmin2 = InnerCylinderOuterRadius2 - inner_cylinder_total_thickness - (2.0*cable_shield_thickness) - cables_thickness ;
  G4double InnerCylinderRmax2 = InnerCylinderOuterRadius2;

  G4double cableShieldRmin1 = InnerCylinderRmin1;
  G4double cableShieldRmax1 = cableShieldRmin1 + (2.0*cable_shield_thickness) + cables_thickness ;
  G4double cableShieldRmin2 = InnerCylinderRmin2;
  G4double cableShieldRmax2 = cableShieldRmin2 + (2.0*cable_shield_thickness) + cables_thickness;

  G4double cablesRmin1 = cableShieldRmin1 + cable_shield_thickness; 
  G4double cablesRmax1 = cablesRmin1 + cables_thickness;
  G4double cablesRmin2 = cableShieldRmin2 + cable_shield_thickness; 
  G4double cablesRmax2 = cablesRmin2 + cables_thickness;

  G4Cons *FTDInnerCylinderSolid
    = new G4Cons("FTDInnerCylinder",
		 InnerCylinderRmin1,
		 InnerCylinderRmax1,
		 InnerCylinderRmin2,
		 InnerCylinderRmax2,
		 InnerCylinder_half_z,
		 start_phy, 
		 stop_phy);

  G4LogicalVolume *FTDInnerCylinderLogical=
    new G4LogicalVolume(FTDInnerCylinderSolid,
			KaptonMat,
			"FTDInnerCylinder", 
			0, 
			0, 
			0);
  
  FTDInnerCylinderLogical->SetVisAttributes(VisAttCyl);

  G4Cons *FTDCableShieldSolid
    = new G4Cons("FTDInnerCableShield",
		 cableShieldRmin1,
		 cableShieldRmax1,
		 cableShieldRmin2,
		 cableShieldRmax2,
		 InnerCylinder_half_z,
		 start_phy, 
		 stop_phy);
  
  G4LogicalVolume *FTDCableShieldLogical=
    new G4LogicalVolume(FTDCableShieldSolid,
			KaptonMat,
			"FTDInnerCableShield", 
			0, 
			0, 
			0);
  
  FTDCableShieldLogical->SetVisAttributes(VisAttCables);

  G4Cons *FTDCablesSolid
    = new G4Cons("FTDInnerCables",
		 cablesRmin1,
		 cablesRmax1,
		 cablesRmin2,
		 cablesRmax2,
		 InnerCylinder_half_z,
		 start_phy, 
		 stop_phy);
  
  G4LogicalVolume *FTDCablesLogical=
    new G4LogicalVolume(FTDCablesSolid,
			CuMat,
			"FTDInnerCables", 
			0, 
			0, 
			0);
  
  FTDCablesLogical->SetVisAttributes(VisAttCables);


  
  //... the cables are placed inside the cylinder
  
  Phys = new G4PVPlacement(0,
			   G4ThreeVector(0., 0., 0.),
			   FTDCablesLogical,
			   "FTDInnerCables",
			   FTDCableShieldLogical,
			   false,0);    

  Phys = new G4PVPlacement(0,
			   G4ThreeVector(0., 0., 0.),
			   FTDCableShieldLogical,
			   "FTDInnerCableShield",
			   FTDInnerCylinderLogical,
			   false,0);      
  
  Phys = new G4PVPlacement(0,
			   G4ThreeVector(0., 0., InnerCylinder_position),
			   FTDInnerCylinderLogical,
			   "FTDInnerCylinder",
			   worldLog,
			   false,0);      
  Phys = new G4PVPlacement(rot,
			   G4ThreeVector(0., 0., -InnerCylinder_position),
			   FTDInnerCylinderLogical,
			   "FTDInnerCylinder",
			   worldLog,
			   false,0);
  
  // Closes Database connection
  delete db;
  db = 0;

  return true;

}


#ifdef MOKKA_GEAR

void SFtd05::GearSetup()
{
  
  G4double CurrentdEdx=0;
  G4double Si_RadLen, Si_dEdx;
  G4double Kapton_RadLen, Kapton_dEdx;
  G4double Cu_RadLen, Cu_dEdx;
  G4EmCalculator findDEdx;
  
  G4ParticleTable* theParticleTable=G4ParticleTable::GetParticleTable();

  Si_RadLen = SiMat->GetRadlen();
  Kapton_RadLen = KaptonMat->GetRadlen();
  Cu_RadLen = CuMat->GetRadlen();

  //... Looping over bins in the DEDX table to obtain the mip DEDX 
  //... From energy 0.0001MeV to 1000MeV in steps of 10

  G4double step_size=10,step,mindEdx = 99999;;
  
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx= 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  SiMat);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }
  
  Si_dEdx=(mindEdx)/1000;
   
  mindEdx=99999;
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx= 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  KaptonMat);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }
  Kapton_dEdx=(mindEdx)/1000;
  
  mindEdx = 99999;
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx= 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  CuMat);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }
  Cu_dEdx=(mindEdx)/1000;

  gear::GearParametersImpl* gearParameters = new gear::GearParametersImpl;
  
  gearParameters -> setDoubleVals( "FTDZCoordinate" , z_positionVec );
  gearParameters -> setDoubleVals( "FTDInnerRadius" , inner_radiusVec );
  gearParameters -> setDoubleVals( "FTDOuterRadius" , outer_radiusVec );
  gearParameters -> setDoubleVals( "FTDDiskSiThickness" ,  Disks_Si_thicknessVec );
  gearParameters -> setDoubleVals( "FTDDiskSupportThickness" ,  Disks_Support_thicknessVec );
  gearParameters -> setDoubleVal( "zFTDOuterCylinderStart" , ZStartOuterCylinder );
  gearParameters -> setDoubleVal( "zFTDOuterCylinderEnd" , ZStopOuterCylinder );
  gearParameters -> setDoubleVal( "zFTDInnerConeStart" ,  ZStartInnerCylinder );
  gearParameters -> setDoubleVal( "zFTDInnerConeEnd" , ZStopInnerCylinder );
  gearParameters -> setDoubleVal( "FTDCopperThickness" , cables_thickness );
  gearParameters -> setDoubleVal( "FTDOuterCylinderThickness" , outer_cylinder_total_thickness );
  gearParameters -> setIntVal( "LastHeavyLayer" , LastHeavyLayer );
  gearParameters -> setDoubleVal( "Silicon_RadLen" , Si_RadLen );
  gearParameters -> setDoubleVal( "Silicon_dEdx" , Si_dEdx );
  gearParameters -> setDoubleVal( "Kapton_RadLen" , Kapton_RadLen );
  gearParameters -> setDoubleVal( "Kapton_dEdx" , Kapton_dEdx );
  gearParameters -> setDoubleVal( "Copper_RadLen" , Cu_RadLen );
  gearParameters -> setDoubleVal( "Copper_dEdx" , Cu_dEdx );

  // Write gearParameters to GearMgr
  // Parameters for FTD
  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setGearParameters("FTD", gearParameters ) ;
}

#endif
