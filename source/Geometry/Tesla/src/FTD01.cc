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
// FTD01.cc
//
// FTD01 SiLC implementation (V.Saveliev)
// first 3 Disks are Si-pixel technology
// last 4 Disks are Si-strip technology
// Supports are Carbon Fiber Material
//
// History:  
// - first implementation P. Mora de Freitas (sept 02)
// - fixed geometry overlap -- Adrian Vogel, 2005-12-05
// - implemented new GEAR interface -- K. Harder, T. Pinto Jayawardena  2007-07-31

#include "Control.hh"
#include "G4PVPlacement.hh"
#include "FTD01.hh"
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

INSTANTIATE(FTD01)

FTD01::~FTD01()
{
  // if (!theFTDSD) delete theFTDSD;
}

G4bool FTD01::construct(const G4String &aSubDetectorName,
			 G4LogicalVolume *WorldLog)
{
  G4VisAttributes * VisAttDisks;
  VisAttDisks = new G4VisAttributes(G4Colour(0.,0.,.75));
  VisAttDisks->SetForceWireframe(true);
  //VisAttDisks->SetForceSolid(true);

  G4VisAttributes * VisAttCyl;
  VisAttCyl = new G4VisAttributes(G4Colour(.5,.5,.5));
  VisAttCyl->SetForceWireframe(true);
  //VisAttCyl->SetForceSolid(true);

  G4VisAttributes * VisAttCables;
  VisAttCables = new G4VisAttributes(G4Colour(.75,0.,0.));
  VisAttCables->SetForceWireframe(true);
  //VisAttCables->SetForceSolid(true);

  G4RotationMatrix *rot=new G4RotationMatrix();
  rot->rotateX(pi); // the same but other side

     G4PVPlacement *Phys;
     G4double start_phy = 0.*deg;
     G4double stop_phy = 360.*deg;

     G4cout << "Building  Ftd01" << aSubDetectorName << G4endl;

     db = new Database(aSubDetectorName.data());

  //... db common_parameters
     db->exec("select * from common_parameters;");
     db->getTuple();
 
     Disks_Si_thickness =  
                      db->fetchDouble("Si_thickness") * mm;

     Disks_Si_thickness_2 =  
                      db->fetchDouble("Si_thickness_2") * mm;

     gear_Disks_Si_thickness = Disks_Si_thickness;
     gear_Disks_Si_thickness_2 = Disks_Si_thickness_2;
     

     /*
     Disks_Si_thickness_2 =   
       db->fetchDouble("Si_thickness") * mm; 
     */
     inner_support_thickness = 
                      db->fetchDouble("inner_support_thickness") * mm;
     inner_support_length = 
                      db->fetchDouble("inner_support_length") * mm;
     outer_support_thickness = 
                      db->fetchDouble("outer_support_thickness") * mm;
     outer_support_thickness = 
                      db->fetchDouble("outer_support_thickness") * mm;
     outer_support_length = 
                      db->fetchDouble("outer_support_length") * mm;
     outer_cylinder_total_thichness = 
                      db->fetchDouble("outer_cylinder_total_thichness") * mm;
     cables_thichness = 
                      db->fetchDouble("cables_thichness") * mm;

     G4cout << "Ftd01 "<<aSubDetectorName << " "
	    << Disks_Si_thickness <<" " << Disks_Si_thickness_2 <<" " 
	    << inner_support_thickness<<" " << outer_support_thickness<<" "
            <<cables_thichness <<G4endl;

  //... The sensitive layer
  //... Threshold is 20% of a MIP. For Si we have 340 KeV/mm as MIP.

    theFTDSD = 
      new TRKSD00("FTD", 
		  Disks_Si_thickness
		  * 340 * keV/mm
		  * 0.2);

  RegisterSensitiveDetector(theFTDSD);

  //... Disks
        db->exec("select * from disk;");
	db->getTuple();

	ZStartOuterCylinder=0;
	ZstopOuterCylinder=0;
	G4double OuterCylinderInnerRadious=0;
	ZStartInnerCylinder=0;
	ZstopInnerCylinder=0;
	G4double InnerCylinderInnerRadious1=0;
	G4double InnerCylinderInnerRadious2=0;
  
	SiMat = CGAGeometryManager::GetMaterial("silicon_2.33gccm");
	KaptonMat = CGAGeometryManager::GetMaterial("kapton");
	//  KaptonMat = CGAGeometryManager::GetMaterial("graphite");
	CuMat = CGAGeometryManager::GetMaterial("copper"); 

	//... accembling detector
  do 
    {
      //... Get the disk parameters
      G4int disk_number;
      G4double inner_radious,outer_radious,z_position;
      disk_number = db->fetchInt("disk_number");
      inner_radious = db->fetchDouble("inner_radious") * mm ;
      inner_radiusVec.push_back(inner_radious);
      outer_radious  = db->fetchDouble("outer_radious") * mm ;
      outer_radiusVec.push_back(outer_radious);
      z_position = db->fetchDouble("z_position") * mm ;
      z_positionVec.push_back(z_position);
      G4Material *DiskMaterial;
      DiskMaterial = SiMat;

    //... Si sensitive
      G4Tubs *FTDSiDiskSolid
	= new G4Tubs("FTDSiDisk",
		     inner_radious,
		     outer_radious,
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
			  WorldLog,
			  false,
			  disk_number);      
      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., -z_position),
			  FTDSiDiskLogical,
			  "FTDSiDisk",
			  WorldLog,
			  false,
			  -disk_number);      

      //... Support
      G4Tubs *FTDInnerSupportRingSolid
	= new G4Tubs("FTDInnerSupportRing",
		     //	inner_radious-inner_support_thickness,
		     inner_radious,
		     outer_radious,
		     inner_support_thickness/2.,
		     start_phy, 
		     stop_phy);
      
      G4LogicalVolume *FTDInnerSupportRingLogical=
	new G4LogicalVolume(FTDInnerSupportRingSolid,
			    KaptonMat,
			    "FTDInnerSupportRing", 
			    0, 
			    0, 
			    0);
      
      G4VisAttributes * VisAttRings;
      VisAttRings = new G4VisAttributes(G4Colour(.5,.5,.5));
      VisAttRings->SetForceWireframe(true);
      //VisAttRings->SetForceSolid(true);
      FTDInnerSupportRingLogical->SetVisAttributes(VisAttRings);

      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., 
                                        z_position
					+Disks_Si_thickness/2.
					+inner_support_thickness/2.),
			  FTDInnerSupportRingLogical,
			  "FTDInnerSupportRing",
			  WorldLog,
			  false,+disk_number);      
      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., 
					-z_position
					-Disks_Si_thickness/2.
					-inner_support_thickness/2.),
			  FTDInnerSupportRingLogical,
			  "FTDInnerSupportRing",
			  WorldLog,
			  false,-disk_number);      

      /*
      //... Outer support rings

      G4Tubs *FTDOuterSupportRingSolid
	= new G4Tubs("FTDOuterSupportRing",
		     outer_radious,
		     outer_radious+outer_support_thickness,
		     outer_support_length/2.,
		     start_phy, 
		     stop_phy);
      
      G4LogicalVolume *FTDOuterSupportRingLogical=
	new G4LogicalVolume(FTDOuterSupportRingSolid,
			    KaptonMat,
			    "FTDOuterSupportRing", 
			    0, 
			    0, 
			    0);
      
      FTDOuterSupportRingLogical->SetVisAttributes(VisAttRings);
  
      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., z_position),
			  FTDOuterSupportRingLogical,
			  "FTDOuterSupportRing",
			  WorldLog,
			  false,+disk_number);      
      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., -z_position),
			  FTDOuterSupportRingLogical,
			  "FTDOuterSupportRing",
			  WorldLog,
			  false,-disk_number);

      */

      if(disk_number==1)
	{
         //... keep information for the outer and inner cylinders
	  ZStartInnerCylinder=z_position;
	  InnerCylinderInnerRadious1 =
	  inner_radious - 0.5 * mm; 
	  //... safety margin due to slope in rz-plane
	}
      if(disk_number==3)
	{
	  //... nothing for now
	}
      if(disk_number==4)
	{
          Disks_Si_thickness =  Disks_Si_thickness_2; 
	  inner_support_thickness = inner_support_thickness - 0.5 *mm;
	  ZStartOuterCylinder = z_position;
	}
      if(disk_number==5)
	{
	  //	ZStartOuterCylinder = z_position;
	}
      if(disk_number==7)
	{
	  ZstopInnerCylinder
	    = ZstopOuterCylinder
	    = z_position;
	  OuterCylinderInnerRadious = outer_radious + 0.5 * mm;
	  InnerCylinderInnerRadious2 = inner_radious  - 0.5 * mm; 
	  //... +-0.5*mm - safety margin due to slope in rz-plane
	}
    }

    while(db->getTuple()!=NULL);
  
  //... Outer cylinder 

  assert(ZStartOuterCylinder>0);
  assert(ZstopOuterCylinder>0);

  G4double OuterCylinder_half_z = 
    (ZstopOuterCylinder-ZStartOuterCylinder)/2.;
  assert(OuterCylinder_half_z>0);
  
  G4double OuterCylinder_position =
    ZStartOuterCylinder + OuterCylinder_half_z;
  
  G4Tubs *FTDOuterCylinderSolid
    = new G4Tubs("FTDOuterCylinder",
		 OuterCylinderInnerRadious,
		 OuterCylinderInnerRadious
		 +outer_cylinder_total_thichness
		 +cables_thichness,
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
  
  /*
  G4Tubs *FTDCablesSolid
    = new G4Tubs("FTDOuterCables",
		 OuterCylinderInnerRadious,
		 OuterCylinderInnerRadious
		 +cables_thichness,
		 OuterCylinder_half_z,
		 start_phy, 
		 stop_phy);
  
  G4LogicalVolume *FTDCablesLogical=
    new G4LogicalVolume(FTDCablesSolid,
			CuMat,
			"FTDOuterCables", 
			0, 
			0, 
			0);
  
  FTDCablesLogical->SetVisAttributes(VisAttCables);
  */  

  //... constructing outer cylinder
  /*
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., 0.),
		      FTDCablesLogical,
		      "FTDOuterCables",
		      FTDOuterCylinderLogical,
		      false,0);      
  */
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., OuterCylinder_position),
		      FTDOuterCylinderLogical,
		      "FTDOuterCylinder",
		      WorldLog,
		      false,+1);      
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., -OuterCylinder_position),
		      FTDOuterCylinderLogical,
		      "FTDOuterCylinder",
		      WorldLog,
		      false,-1);

  //... Inner cylinder (cone)

  assert(ZStartInnerCylinder>0);
  assert(ZstopInnerCylinder>0);
  
  G4double InnerCylinder_half_z = 
    (ZstopInnerCylinder-ZStartInnerCylinder)/2.;
  assert(InnerCylinder_half_z>0);
  
  G4double InnerCylinder_position =
    ZStartInnerCylinder + InnerCylinder_half_z;
  
  G4Cons *FTDInnerCylinderSolid
    = new G4Cons("FTDInnerCylinder",
		 InnerCylinderInnerRadious1
		 - outer_cylinder_total_thichness
		 - cables_thichness,
		 InnerCylinderInnerRadious1,
		 InnerCylinderInnerRadious2
		 - outer_cylinder_total_thichness
		 - cables_thichness,
		 InnerCylinderInnerRadious2,		 
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
		 InnerCylinderInnerRadious1
		 - cables_thichness,
		 InnerCylinderInnerRadious1,
		 InnerCylinderInnerRadious2
		 - cables_thichness,
		 InnerCylinderInnerRadious2,		 
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
		      WorldLog,
		      false,+1);      
  Phys = new G4PVPlacement(rot,
		      G4ThreeVector(0., 0., -InnerCylinder_position),
		      FTDInnerCylinderLogical,
		      "FTDInnerCylinder",
		      WorldLog,
		      false,-1);
  
  // Closes Database connection
  delete db;
  db = 0;
  G4cout << "FTD done.\n" << G4endl;
  return true;

}

#ifdef MOKKA_GEAR

void FTD01::GearSetup()
{
  
  G4double CurrentdEdx=0;
  G4double Si_RadLen, Si_dEdx;
  //  G4double Si872_RadLen, Si872_dEdx;
  G4double Kapton_RadLen, Kapton_dEdx;
  G4double Cu_RadLen, Cu_dEdx;
  G4EmCalculator findDEdx;
  
  G4ParticleTable* theParticleTable=G4ParticleTable::GetParticleTable();

  Si_RadLen = SiMat->GetRadlen();
  //  Si872_RadLen = Si872Mat->GetRadlen();
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
  gearParameters -> setDoubleVal( "FTDDiskThickness" ,  gear_Disks_Si_thickness );
  gearParameters -> setDoubleVal( "FTDDiskThickness_2" ,  gear_Disks_Si_thickness_2 );
  gearParameters -> setDoubleVal( "FTDInnerSupportdR" ,  inner_support_length );
  gearParameters -> setDoubleVal( "FTDOuterSupportdR" ,  outer_support_length );
  gearParameters -> setDoubleVal( "FTDInnerSupportThickness" ,  inner_support_thickness );
  gearParameters -> setDoubleVal( "FTDOuterSupportThickness" ,  outer_support_thickness );
  gearParameters -> setDoubleVal( "zFTDOuterCylinderStart" , ZStartOuterCylinder );
  gearParameters -> setDoubleVal( "zFTDOuterCylinderEnd" , ZstopOuterCylinder );
  gearParameters -> setDoubleVal( "zFTDInnerConeStart" ,  ZStartInnerCylinder );
  gearParameters -> setDoubleVal( "zFTDInnerConeEnd" , ZstopInnerCylinder );
  gearParameters -> setDoubleVal( "FTDCopperThickness" , cables_thichness );
  gearParameters -> setDoubleVal( "FTDOuterCylinderThickness" , outer_cylinder_total_thichness );
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
