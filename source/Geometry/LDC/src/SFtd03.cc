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
// SFtd03.cc
//
// SFtd03 SiLC implementation (V.Saveliev)
// first 3 Disks are Si-pixel technology
// last 4 Disks are Si-strip technology
// Supports are Kapton Material

//   Disk positions in z are set according to the relative position to the 
//   TPC length, the relative values are stored in the ftd database.
//   The inner radius is calculated to follow the opening angle of the beamtube
//   with clearance given for cables plus a safety factor

// History:  
// - first implementation P. Mora de Freitas (sept 02)
// - fixed geometry overlap -- Adrian Vogel, 2005-12-05
// - implemented new GEAR interface -- K. Harder, T. Pinto Jayawardena  2007-07-31
// - Modified version of SFtd02: Rewritten as a self scaling driver which does not
//   make use of a seperate super driver. Steve Aplin (May 2008)

#include "Control.hh"
#include "G4PVPlacement.hh"
#include "SFtd03.hh"
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

INSTANTIATE(SFtd03)

G4bool SFtd03::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog)

{

  std::cout << "About to start building ftd with " << env.GetDBName() << std::endl;

  G4VisAttributes * VisAttDisks;
  VisAttDisks = new G4VisAttributes(G4Colour(1.,1.,.8));
  //VisAttDisks->SetForceWireframe(true);
  //VisAttDisks->SetForceSolid(true);

  G4VisAttributes * VisAttSuport;
  VisAttSuport = new G4VisAttributes(G4Colour(1,.5,.5));
  //VisAttSuport->SetForceWireframe(true);
  //VisAttSuport->SetForceSolid(true);

  G4VisAttributes * VisAttCyl;
  VisAttCyl = new G4VisAttributes(G4Colour(0.45,.2,0.9));
  VisAttCyl->SetForceWireframe(true);
  //VisAttCyl->SetForceSolid(true);

  G4VisAttributes * VisAttCables;
  VisAttCables = new G4VisAttributes(G4Colour(0.,0.9,0.));
  VisAttCables->SetForceWireframe(true);
  //VisAttCables->SetForceSolid(true); 

  G4RotationMatrix *rot=new G4RotationMatrix();
  rot->rotateX(pi); // the same but other side

  G4PVPlacement *Phys;
  G4double start_phy = 0.*deg;
  G4double stop_phy = 360.*deg;

  db = new Database(env.GetDBName());
  //... db common_parameters
  db->exec("select * from common_parameters;");
  db->getTuple();

  
  const G4double TPC_Ecal_Hcal_barrel_halfZ = env.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");
  G4double beamTubeTangent = (env.GetParameterAsDouble("TUBE_opening_angle"));
  // SJA:FIXME: this value is not updated when the model is scaled.

  const G4double beamTubeClearance = db->fetchDouble("beamtube_clearance"); 
  cable_shield_thickness = 
    db->fetchDouble("cable_shield_thickness") * mm;
  outer_cylinder_total_thickness = 
    db->fetchDouble("outer_cylinder_total_thickness") * mm;
  cables_thickness = 
    db->fetchDouble("cables_thickness") * mm;


  //... Disks
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
    new TRKSD00("FTD", 
		Disks_Si_thickness
		* 340 * keV/mm
		* 0.2);
     
  RegisterSensitiveDetector(theFTDSD);
     
  //... Disks
  db->exec("select * from disks;");
  db->getTuple();
     
  ZStartOuterCylinder=0;
  ZStopOuterCylinder=0;
  G4double OuterCylinderInnerRadius=0;
     
  ZStartInnerCylinder=0;
  ZStopInnerCylinder=0;
  G4double InnerCylinderInnerRadius1=0;
  G4double InnerCylinderInnerRadius2=0;
     
  SiMat = CGAGeometryManager::GetMaterial("silicon_2.33gccm");
  KaptonMat = CGAGeometryManager::GetMaterial("kapton");
  CuMat = CGAGeometryManager::GetMaterial("copper"); 
     
  //... assembling detector
  do 
    {
      //... Get the disk parameters
      G4int disk_number;
      G4double inner_radius,outer_radius,z_position;

      disk_number = db->fetchInt("disk_number");
      Disks_Si_thickness = db->fetchDouble("disk_si_thickness") * mm ;
      Disks_Support_thickness = db->fetchDouble("disk_support_thickness") * mm ;

      z_position = 
	(TPC_Ecal_Hcal_barrel_halfZ * db->fetchDouble("z_position_ReltoTPCLength")) * mm;

      inner_radius = ((z_position
		       +(Disks_Si_thickness)/2.0
		       +Disks_Support_thickness)*beamTubeTangent 
		      + cables_thickness
		      + (2.0*cable_shield_thickness) 
		      + beamTubeClearance) * mm;

      outer_radius  = db->fetchDouble("outer_radius") * mm ;

      // push back values to be stored in GEAR
      Disks_Si_thicknessVec.push_back(Disks_Si_thickness);
      Disks_Support_thicknessVec.push_back(Disks_Support_thickness);
      z_positionVec.push_back(z_position);
      inner_radiusVec.push_back(inner_radius);
      outer_radiusVec.push_back(outer_radius);

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
			  false,+disk_number);      
      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., 
					-z_position
					-Disks_Si_thickness/2.
					-Disks_Support_thickness/2.),
			  FTDDiskSupportLogical,
			  "FTDDiskSupport",
			  worldLog,
			  false,-disk_number);      


      if(disk_number==1)
	{
	  //... keep information for the outer and inner cylinders
	  ZStartInnerCylinder = z_position;
	  InnerCylinderInnerRadius1 = 
	    inner_radius - ((Disks_Si_thickness)/2.0 + Disks_Support_thickness)*beamTubeTangent - 0.5 * mm; 
	  //... safety margin due to slope in rz-plane
	}
      if(disk_number==3)
	{
	  // ...
	}
      if(disk_number==4)
	{
	  ZStartOuterCylinder = z_position;
	}
      if(disk_number==5)
	{
	  //	ZStartOuterCylinder = z_position;
	}
      if(disk_number==7)
	{
	  ZStopInnerCylinder = ZStopOuterCylinder = z_position + (Disks_Si_thickness)/2.0 + Disks_Support_thickness ;
	  OuterCylinderInnerRadius = outer_radius + 0.5 * mm;
	  InnerCylinderInnerRadius2 = 
	    inner_radius - ((Disks_Si_thickness)/2.0 + Disks_Support_thickness)*beamTubeTangent - 0.5 * mm; 
	  //... 0.5*mm - safety margin due to slope in rz-plane
	}
    }

  while(db->getTuple()!=NULL);
  
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
		 +outer_cylinder_total_thickness
		 +cables_thickness,
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
		      false,+1);      
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., -OuterCylinder_position),
		      FTDOuterCylinderLogical,
		      "FTDOuterCylinder",
		      worldLog,
		      false,-1);


  //... Inner cylinder (cone)

  assert(ZStartInnerCylinder>0);
  assert(ZStopInnerCylinder>0);
  
  G4double InnerCylinder_half_z = 
    (ZStopInnerCylinder-ZStartInnerCylinder)/2.;
  assert(InnerCylinder_half_z>0);
  
  G4double InnerCylinder_position =
    ZStartInnerCylinder + InnerCylinder_half_z;
  
  G4Cons *FTDInnerCylinderSolid
    = new G4Cons("FTDInnerCylinder",
		 InnerCylinderInnerRadius1
		 - outer_cylinder_total_thickness
		 - cables_thickness,
		 InnerCylinderInnerRadius1,
		 InnerCylinderInnerRadius2
		 - outer_cylinder_total_thickness
		 - cables_thickness,
		 InnerCylinderInnerRadius2,		 
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

  G4Cons *FTDCables2Solid
    = new G4Cons("FTDInnerCables",
		 InnerCylinderInnerRadius1
		 - (2.0*cable_shield_thickness)
		 - cables_thickness,
		 InnerCylinderInnerRadius1,
		 InnerCylinderInnerRadius2
		 - (2.0*cable_shield_thickness)
		 - cables_thickness,
		 InnerCylinderInnerRadius2,		 
		 InnerCylinder_half_z,
		 start_phy, 
		 stop_phy);
  
  G4LogicalVolume *FTDCables2Logical=
    new G4LogicalVolume(FTDCables2Solid,
			CuMat,
			"FTDInnerCables", 
			0, 
			0, 
			0);
  
  FTDCables2Logical->SetVisAttributes(VisAttCables);
  
  //... the cables are placed inside the cylinder
  
  Phys = new G4PVPlacement(0,
			   G4ThreeVector(0., 0., 0.),
			   FTDCables2Logical,
			   "FTDInnerCables",
			   FTDInnerCylinderLogical,
			   false,0);      
  
  Phys = new G4PVPlacement(0,
			   G4ThreeVector(0., 0., InnerCylinder_position),
			   FTDInnerCylinderLogical,
			   "FTDInnerCylinder",
			   worldLog,
			   false,+1);      
  Phys = new G4PVPlacement(rot,
			   G4ThreeVector(0., 0., -InnerCylinder_position),
			   FTDInnerCylinderLogical,
			   "FTDInnerCylinder",
			   worldLog,
			   false,-1);
  
  // Closes Database connection
  delete db;
  db = 0;
  G4cout << "FTD done.\n" << G4endl;
  return true;

}

#ifdef MOKKA_GEAR

void SFtd03::GearSetup()
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
