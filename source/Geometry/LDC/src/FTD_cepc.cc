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
// FTD_cepc.cc
//
// Simplified Implementation of a self scaling 7 disk FTD
// Based on SFtd06 but using simple trapezoid sensitive and support structures
// All disks have the same structure
// Sensitive material Silicon 
// Support material Carbon Fiber, Foam 

// All disks' envelop:
// Dimensions and coordinates are specified for the sensitive layer, support disks are built on to these
// _inner_radius = (  _beamTubeRadius + beamTubeClearance)

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
// _glEnv.Ecal_endcap_zmin
// _glEnv.TPC_inner_radius
// VXD_length_r3

// Parameters shared with other drivers:
// SSit03:_glEnv.SIT1_Half_Length_Z
// SSit03:_glEnv.SIT2_Half_Length_Z 
// SSit03:_glEnv.SIT1_Radius 
// SSit03:_glEnv.SIT2_Radius 
// TubeX01:TUBE_IPOuterTube_end_z
// TubeX01:TUBE_IPOuterTube_end_radius
// TubeX01:TUBE_IPOuterBulge_end_z
// TubeX01:TUBE_IPOuterBulge_end_radius

// October 15th 2008, Steve Aplin using description from SiLC Collaboration
// September 7th 2010, Jordi Duarte using mechanical design from IFCA group

// History:  
// - first implementation P. Mora de Freitas (sept 02)
// - fixed geometry overlap -- Adrian Vogel, 2005-12-05
// - implemented new GEAR interface -- K. Harder, T. Pinto Jayawardena  2007-07-31
// - SFtd03: Modified version of SFtd02: Rewritten as a self scaling driver which does not
//   make use of a seperate super driver. Steve Aplin (May 2008)
// - Fixes a bug in SFtd04.cc which meant that the copper cables on thin 
//   inside of the cylinder were far to thick (SJA 28/05/09)
// - SFtd06: Modified version of SFtd05 implementing realistic details of the disks 
//           4,5,6,7 structure. -- J. Duarte Campderros (Sept. 2010)
//           Added realistic description to disks 1,2,3. Changed disk 3 to micro-strips 
//           technology --- J. Duarte Campderros (Oct. 2010)
//           Included the alternative z-offset between petals --|
//           Included the use of the GEAR class FTDParameter  --| J. Duarte Campderros (July, 2011)
//           Modified the placement of the Volumes using the 
//           G4ReflectionFactory Place methods. Now the volumes in Z
//           negatives are specular images from the positives -- J. Duarte Campderros (Sept, 2011)
// - FTD_cepc: created to enable development of tracking code while SFtd06 is finalised.
//   S.J. Aplin (Nov 2011)

//#define DEBUG_VALUES 1

#include<map>
#include<vector>
#include<string>
#include<assert.h>

#include "Control.hh"
//#include "G4PVPlacement.hh"
#include "FTD_cepc.hh"
#include "TRKSD_FTD01.hh"
#include "CGAGeometryManager.hh"

#include "globals.hh"

#include "G4Cons.hh"
#include "G4Tubs.hh"
#include "G4Box.hh"
#include "G4SubtractionSolid.hh"
#include "G4ThreeVector.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4AssemblyVolume.hh"
#include "G4RotationMatrix.hh"


#include "MySQLWrapper.hh"

#include "CGADefs.h"

#ifdef MOKKA_GEAR
#include "gearimpl/FTDParametersImpl.h"
#include "MokkaGear.h"
#include "G4EmCalculator.hh"
#include "G4ParticleTable.hh"
#endif 

INSTANTIATE(FTD_cepc)

// Destructor: de-allocate the PV used with the G4ReflectionFactory
//  NEEDED ????
/*FTD_cepc::~FTD_cepc()
 {
 for(std::vector<G4VPhysicalVolume*>::iterator it = _registerPV.begin(); 
 it != _registerPV.end(); ++it)
 {
 if( *it != 0)
 {
 delete *it;
 }
 }
 }*/

// FIXME?: check the correct destruction of the G4AssemblyVolume, otherwise do it in the destructor	
G4bool FTD_cepc::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog)
{
  // Globals and cosmetics definitions
	_VisAttSensitive = new G4VisAttributes(G4Colour(1.,1.,.45));
	
  //G4VisAttributes *
	_VisAttSupport = new G4VisAttributes(G4Colour(1,.5,.5));

	_VisAttHolePetal = new G4VisAttributes(G4Color(1,.5,.5,1.0));
  
	G4VisAttributes *VisAttAirDisk = new G4VisAttributes(G4Colour(.5,.3,.8,0.98));
	VisAttAirDisk->SetVisibility(0);
	
	G4VisAttributes *VisAttAirPetal = new G4VisAttributes(G4Colour(.5,.3,.8,0.98));
	VisAttAirPetal->SetVisibility(0);
	
	G4VisAttributes *VisAttCyl = new G4VisAttributes(G4Colour(0.45,.2,0.9,.98));
  
  G4VisAttributes *VisAttCables = new G4VisAttributes(G4Colour(0.,0.9,0.));
  VisAttCables->SetForceWireframe(false);
  
  
	G4double start_phy = 0.*deg;
  G4double stop_phy = 360.*deg;
  
	G4PhysicalVolumesPair Phys;
  
  // Get and set the Globals from the surrounding environment TPC ECAL SIT VTX and Beam-Pipe
	SetEnvironPar( env );
	
  // Get and set the variables global to the FTD cables_thickness, ftd1_vtx3_distance_z, etc
  Database * db = new Database(env.GetDBName());
	SetdbParCommon( db );
  
  //And register the sensitive detectors
	RegisterSDs( db );
	
  // Materials definitions
  _SiMat = CGAGeometryManager::GetMaterial("silicon_2.33gccm");
  _KaptonMat = CGAGeometryManager::GetMaterial("kapton");
  _CuMat = CGAGeometryManager::GetMaterial("copper"); 
	_AirMat = CGAGeometryManager::GetMaterial("air");
  
  // -- FIXME: PROVISIONAL -- Carbon Fiber definition from database ?? 
	G4double density;
 	G4String name, symbol;
  G4int nel;
  G4double fractionmass, volumefraction;
  
  volumefraction = 0.5;
  density = (1.3 + volumefraction / 3 ) * g/cm3;
  fractionmass = 1 - 1.3 * (1 - volumefraction) / (density / (g/cm3));
  
	_CarbonFiberMat = new G4Material(name = "CarbonFiber", density, nel=2);
	_CarbonFiberMat->AddElement(CGAGeometryManager::GetElement("C"), fractionmass);
	_CarbonFiberMat->AddMaterial(CGAGeometryManager::GetMaterial("epoxy"),1.0-fractionmass);
	G4cout << "CarbonFiber->GetRadlen() = " << _CarbonFiberMat->GetRadlen() / cm << " cm" << G4endl;
  // <-------------------------------------------------------------------------------------------
  
  //add by xuyin: change fdisk with lstar
  G4double lstar_zbegin=env.GetParameterAsDouble("LStar_zbegin");
	
  G4cout << "FTD_cepc:"  
  << "\t inner support thickness = " << _dbParCommon.inner_cylinder_total_thickness  
  << "\t cables thickness = " << _dbParCommon.cables_thickness
  << "\t 2 x cable shield thickness = " << 2 * _dbParCommon.cable_shield_thickness
  << "\t beamTubeClearance = " << _dbParCommon.beamTubeClearance
  << G4endl;
  
  // Now we can start to build the disks -------------------------------------------
	const G4double theta = _dbParCommon.petal_half_angle_support;
  
  // Disk parameters
  _dbParDisk.ZStartOuterCylinder=0;
  _dbParDisk.ZStopOuterCylinder=0;
  G4double OuterCylinderInnerRadius=0;
  
  _dbParDisk.ZStartInnerCylinder=0;
  _dbParDisk.ZStopInnerCylinder=0;
  G4double InnerCylinderOuterRadius1=0;
  G4double InnerCylinderOuterRadius2=0;
  
  db->exec("select * from disks;");
  db->getTuple();
  //... assembling detector
  do {

    //... 
    G4int disk_number(-1);
    _inner_radius = 0.0;
    _outer_radius = 0.0;
    _z_position = 0.0;
    _beamTubeRadius = 0.0;
    _zEnd = 0.0;
    // Get and set the parameters disk specific
		SetParDisk( db );
		
    disk_number = _dbParDisk.disk_number;
#ifdef ONE_DISK
		if (disk_number != ONE_DISK )
        {
			continue;
        }
#endif
		
    switch (disk_number) 
          {
            case 1:
          // z defined by distance from end of VTX layer 3
          _z_position = ( _glEnv.VXD_layer3_maxZ + _dbParCommon.ftd1_vtx3_distance_z );
          
          //          _z_position = disk_number * 100.0 * mm;
          
          // outer r defined by radial difference to SIT layer 1
          _outer_radius = ( _glEnv.SIT1_Radius + _dbParCommon.ftd1_sit1_radial_diff ); 
          
          
          // beam tube radius at backside of disk 
          _zEnd = _z_position +  _dbParDisk.petal_support_zoffset + 0.5 * _dbParDisk.petal_cp_support_thickness + ( _dbParDisk.double_sided * _dbParDisk.disks_Si_thickness ) ;
          
          // check which part of the beam tube this disk lies above
          _beamTubeRadius = (_zEnd < _glEnv.zEnd_IPOuterTube ) ? _glEnv.rEnd_IPOuterTube : _glEnv.rEnd_IPOuterTube + ( (_zEnd - _glEnv.zEnd_IPOuterTube ) * _glEnv.beamTubeTangent );
          
          _inner_radius = (  _beamTubeRadius + _dbParCommon.beamTubeClearance);
          
          // check that there is no overlap with SIT1
          if( _z_position <= _glEnv.SIT1_Half_Length_Z && _outer_radius>=_glEnv.SIT1_Radius) 
              {
            G4cout << "FTD_cepc:Stop: Overlap between FTD1 and SIT1" << G4endl;
            G4cout << "FTD_cepc:FTD1 Radius = " << _outer_radius << "SIT1 Radius = " << _glEnv.SIT1_Radius << G4endl;
            exit(1);
              }
          if( db->fetchDouble("z_position_ReltoTPCLength") != 0.0) 
              {
            G4cout << "FTD_cepc:Stop: The z position of FTD1 is not relative. The relative value will not be used. It should be set to 0.0 in the DB." << G4endl;
            G4cout << "FTD_cepc:Stop: The z position of FTD1 is set by the distance between the centre of the sensitive layer and the max z of VTX layer 3." << G4endl;
            exit(1);
              }
          break;
          
            case 2:
          // z defined relative to TPC half-length: to ensure positioning with SIT set these numbers to the same value in DB
          _z_position = (_glEnv.TPC_Ecal_Hcal_barrel_halfZ * db->fetchDouble("z_position_ReltoTPCLength")) * mm;
          
          //          _z_position = disk_number * 100.0 * mm;
          
          // outer r defined by radial difference to SIT layer 1
          _outer_radius = _glEnv.SIT1_Radius + _dbParCommon.ftd2_sit1_radial_diff; 
          
          // beam tube radius at backside of disk 
          _zEnd = _z_position +  _dbParDisk.petal_support_zoffset + 0.5 * _dbParDisk.petal_cp_support_thickness + ( _dbParDisk.double_sided * _dbParDisk.disks_Si_thickness ) ;
          
          // check which part of the beam tube this disk lies above
          _beamTubeRadius = (_zEnd < _glEnv.zEnd_IPOuterTube ) ? _glEnv.rEnd_IPOuterTube : _glEnv.rEnd_IPOuterTube + ( (_zEnd - _glEnv.zEnd_IPOuterTube ) * _glEnv.beamTubeTangent );
          
          _inner_radius = (  _beamTubeRadius + _dbParCommon.beamTubeClearance) ;
          
          //... keep information for inner support cylinder with 0.5mm saftey clearance from inner radius of disks
          _dbParDisk.ZStartInnerCylinder = _glEnv.zEnd_IPOuterTube;
          
          InnerCylinderOuterRadius1 = _inner_radius - ( ( _zEnd - _glEnv.zEnd_IPOuterTube ) * _glEnv.beamTubeTangent ) - 0.5 * mm; 
          
          // check that there is no overlap with SIT1
          if( _z_position <= _glEnv.SIT1_Half_Length_Z && _outer_radius>=_glEnv.SIT1_Radius) 
              {
            G4cout << "FTD_cepc:Stop:Overlap between FTD2 and SIT1" << G4endl;
            G4cout << "FTD_cepc:FTD2 Radius = " << _outer_radius << "SIT1 Radius = " << _glEnv.SIT1_Radius << G4endl;
            exit(1);
              }
          break;
          
            case 3:
          // z defined relative to TPC half-length: to ensure positioning with SIT set these numbers to the same value in DB
          _z_position = (_glEnv.TPC_Ecal_Hcal_barrel_halfZ * db->fetchDouble("z_position_ReltoTPCLength")) * mm;
          
          //          _z_position = disk_number * 100.0 * mm;
          
          // outer r defined by radial difference to SIT layer 2
          _outer_radius = _glEnv.SIT2_Radius + _dbParCommon.ftd3_sit2_radial_diff; 
          
          // beam tube radius at backside of disk 
          _zEnd = _z_position +  _dbParDisk.petal_support_zoffset + 0.5 * _dbParDisk.petal_cp_support_thickness + ( _dbParDisk.double_sided * _dbParDisk.disks_Si_thickness ) ;
          
          // check which part of the beam tube this disk lies above
          _beamTubeRadius = (_zEnd < _glEnv.zEnd_IPOuterTube ) ? _glEnv.rEnd_IPOuterTube : _glEnv.rEnd_IPOuterTube + ( (_zEnd - _glEnv.zEnd_IPOuterTube ) * _glEnv.beamTubeTangent );
          
          _inner_radius = _beamTubeRadius + _dbParCommon.beamTubeClearance ;
          
          // check that there is no overlap with SIT1
          if( _z_position <= _glEnv.SIT2_Half_Length_Z && _outer_radius>=_glEnv.SIT2_Radius) 
              {
            G4cout << "FTD_cepc:Stop:Overlap between FTD3 and SIT2" <<  G4endl;
            G4cout << "FTD_cepc:FTD3 Radius = " << _outer_radius << "SIT2 Radius = " << _glEnv.SIT2_Radius << G4endl;
            exit(1);
              }
          break;
          
            case 4:
            case 5:
            case 6:
          // z defined relative to TPC half-length
          _z_position = (_glEnv.TPC_Ecal_Hcal_barrel_halfZ * db->fetchDouble("z_position_ReltoTPCLength")) * mm;
          
          //          _z_position = disk_number * 100.0 * mm;
          
          // outer r defined by gap between TPC inner radius and FTD disks
          _outer_radius = _glEnv.TPC_inner_radius - _dbParCommon.ftd4to7_tpc_radial_gap; 
          
          // beam tube radius at backside of disk 
          _zEnd = _z_position +  _dbParDisk.petal_support_zoffset + 0.5 * _dbParDisk.petal_cp_support_thickness + ( _dbParDisk.double_sided * _dbParDisk.disks_Si_thickness ) ;

          
          // check which part of the beam tube this disk lies above
          _beamTubeRadius = (_zEnd < _glEnv.zEnd_IPOuterTube ) ? _glEnv.rEnd_IPOuterTube : _glEnv.rEnd_IPOuterTube + ( (_zEnd - _glEnv.zEnd_IPOuterTube ) * _glEnv.beamTubeTangent );
          
          _inner_radius = _beamTubeRadius + _dbParCommon.beamTubeClearance ;
          
          
          // keep the information for outer cylinder
          if(disk_number==4)
              {
            _dbParDisk.ZStartOuterCylinder = _z_position;
              }
          break;
          
            case 7:
          // z defined by distance from front of ECal endcap
          _z_position = _glEnv.Ecal_endcap_zmin - _dbParCommon.ftd7_ecal_distance_z;
          
          //          _z_position = disk_number * 100.0 * mm;
          
          // outer r defined by gap between TPC inner radius and FTD disks
          _outer_radius = _glEnv.TPC_inner_radius - _dbParCommon.ftd4to7_tpc_radial_gap; 
          
          // beam tube radius at backside of disk 
          _zEnd = _z_position +  _dbParDisk.petal_support_zoffset + 0.5 * _dbParDisk.petal_cp_support_thickness + ( _dbParDisk.double_sided * _dbParDisk.disks_Si_thickness ) ;
          
          // check which part of the beam tube this disk lies above
          _beamTubeRadius = (_zEnd < _glEnv.zEnd_IPOuterTube ) ? _glEnv.rEnd_IPOuterTube : _glEnv.rEnd_IPOuterTube + ( (_zEnd - _glEnv.zEnd_IPOuterTube ) * _glEnv.beamTubeTangent );
          
          _inner_radius = _beamTubeRadius + _dbParCommon.beamTubeClearance ;
          
          // End of Support Structure: 0.5mm clearance from disks
          _dbParDisk.ZStopOuterCylinder = _zEnd;
          _dbParDisk.ZStopInnerCylinder = _zEnd;
          
          OuterCylinderInnerRadius = _outer_radius + 0.5 * mm;
          InnerCylinderOuterRadius2 = _inner_radius - 0.5 * mm; 
          
          if( db->fetchDouble("z_position_ReltoTPCLength") != 0.0) 
              {
            G4cout << "FTD_cepc:Stop: The z position of FTD7 is not relative. The relative value will not be used. It should be set to 0.0 in the DB." << G4endl;
            G4cout << "FTD_cepc:Stop: The z position of FTD7 is set by the distance between the centre of the sensitive layer and the min z of the ECal Endcap." << G4endl;
            exit(1);
              }
          break;
          
            default:
          G4cout << "FTD_cepc: Error disk number must be between 1-7: disk number = " << disk_number << G4endl;
          exit(1);
          }
    
    G4cout << "FTD_cepc: Disk:" << disk_number
    << "\t z = " << _z_position
    << "\t inner rad = " << _inner_radius
    << "\t outer rad = " << _outer_radius
    << "\t beamtube rad = " << _beamTubeRadius
    << "\t free space = " << (_inner_radius - 0.5 * mm - _dbParCommon.inner_cylinder_total_thickness - (2*_dbParCommon.cable_shield_thickness) - _dbParCommon.cables_thickness) - _beamTubeRadius 
    << G4endl;
		

    //modify by xuyin: change disk;
    //if (disk_number>5) continue;
    if (_z_position>(lstar_zbegin-220)) continue;
		/**************************************************************************************
     ** Begin construction of disks with appropiate parameters    **
     **************************************************************************************/
    
    //================================== AIR DISK =======================================//
    //  The air-disk is the container and mother volume of the petals. There will be
    //  7x2 air disks copies placed in the world volume.
    //  
    //  Check the comments at the beginning of this file for the description of 
    //  each # disk parameter.
    //  
    //  Input parameters:
    //       _inner_radius: inner radius of the whole structure
    //       _outer_radius: outer radius of the whole structure
    //       max_half_thickness_disk: 
    //           the maximum thickness of the disk = 2.0 * ( sensitive thickness + support thickness + Zoffset ) 
    
    //                                 Zoffset=the displacement of the disks in z-direction
    
    // 
    //		// The thickness of the air petal (containing the support and sensors)
    //		double petalairthickness_half = 0.5 * ( _dbParDisk.petal_cp_support_thickness +
    //			_dbParDisk.disks_Si_thickness )  
    
    
    
    // need enough space for double sided 
    G4double petalairthickness_half = 0.5 * ( _dbParDisk.petal_cp_support_thickness
                                             + 2.0*_dbParDisk.disks_Si_thickness ) ;
    
    G4double max_half_thickness_disk = _dbParDisk.petal_support_zoffset + petalairthickness_half ;

		
		G4Tubs *FTDDiskSolid = new G4Tubs("FTDAirDiskSolid",
                                      _inner_radius,
                                      _outer_radius,
                                      max_half_thickness_disk,
                                      start_phy,
                                      stop_phy
                                      );
    
		G4LogicalVolume *FTDDiskLogical = new G4LogicalVolume(FTDDiskSolid,
                                                          _AirMat,
                                                          "FTDAirDiskLogical", 
                                                          0, 
                                                          0, 
                                                          0);
    FTDDiskLogical->SetVisAttributes(VisAttAirDisk);
		
    
		G4RotationMatrix *rotDiskPositive = new G4RotationMatrix();
    // Sensors facing the IP)
		rotDiskPositive->rotateY(pi);
    // Re-allocating the local disk frame to the global frame
		rotDiskPositive->rotateZ(-pi/2.0);
    
		G4Transform3D transPositive( *rotDiskPositive, G4ThreeVector( 0.,0.,_z_position) );
    
    // Place the positive copy in the world
		Phys = G4ReflectionFactory::Instance()->Place( transPositive,
                                                  "FTDAirDisk",
                                                  FTDDiskLogical,
                                                  worldLog,
                                                  false,
                                                  disk_number);
		registerPV( Phys );
    
#ifdef DEBUG_VALUES
		G4cout << "===================================================================== " << "\n" <<
    "FTDAirDisk:\n" << 
    " Inner Radius= " << _inner_radius <<  "\n" <<
    " Outer Radius= " << _outer_radius <<  "\n" <<
    " thickness =   " << max_half_thickness_disk*2.0 << "\n" <<
    " placed at \n" << 
    " x =   " <<  transPositive.dx() << "\n" <<
    " y =   " <<  transPositive.dy() << "\n" <<
    " z =   " <<  transPositive.dz() << "\n" <<
    G4endl;
#endif
    
    
    // Place negative copy
#ifndef DEBUG_POSITIVE
		G4RotationMatrix *rotDiskNegative = new G4RotationMatrix();
		rotDiskNegative->rotateZ(-pi/2.0);
    
		G4Transform3D transNegative( *rotDiskNegative, G4ThreeVector( 0.,0.,-_z_position ) );
    //Specular image
		transNegative = transNegative*G4ReflectX3D();
		
		Phys = G4ReflectionFactory::Instance()->Place( transNegative,
                                                  "FTDAirDisk",
                                                  FTDDiskLogical,
                                                  worldLog,
                                                  false,
                                                  -disk_number);
		registerPV( Phys );
    
#ifdef DEBUG_VALUES
		G4cout << "===================================================================== " << "\n" <<
    "FTDAirDisk:\n" << 
    " Inner Radius= " << _inner_radius <<  "\n" <<
    " Outer Radius= " << _outer_radius <<  "\n" <<
    " thickness =   " << max_half_thickness_disk*2.0 << "\n" <<
    " placed at \n" << 
    " x =   " <<  transNegative.dx() << "\n" <<
    " y =   " <<  transNegative.dy() << "\n" <<
    " z =   " <<  transNegative.dz() << "\n" <<
    G4endl;
#endif
    
    
#endif
    //=END=============================== AIR DISK  =================================END=/
    
    //=================================== AIR PETAL =====================================/
    // Air container for the petal: the mother of the real support petal and the silicon 
    // sensors. This air petal will be placed inside the Air Disk,
    // generating N rotated copies along the z-axis.               
    //  Input parameters:     dxMax                                _
    //                      --------                              | |    
    //                      \      /   |                          | |              
    //       XY-Plane        \    /    | dy          YZ-Plane     | |    
    //                        \__/     |                          |_|     
    //                        dxMin                                dz
    // 
    //                     dxMax: given by the database
    //                     dxMin: depends of the _inner_radius of each disk
    //                     dy:    heigth, depends of each disk
    //                     dz:    thickness of the supports + thickness of Si
    //                     theta: given by the db, semi-angle which defines the trapezoid
		
    // Dimensions for the disk
    
    const G4double petal_cp_supp_half_dxMin = Getdx( _inner_radius )/2.0;
    const G4double petal_cp_support_dy = Getdy(_inner_radius);
    
    // ------------------------------------------------------------------------
    
    
    G4Trap *FTDPetalAirSolid = new G4Trap( "FTDPetalAirSolid",
                                          petalairthickness_half, //thickness (calculated in the disk zone)
                                          0.0,
                                          0.0,
                                          petal_cp_support_dy/2.0,  // dy
                                          petal_cp_supp_half_dxMin, //dxMin 
                                          _dbParDisk.petal_cp_support_dxMax/2.0, //dxMax
                                          0.0,
                                          petal_cp_support_dy/2.0,  // dy
                                          petal_cp_supp_half_dxMin,  // dxMin
                                          _dbParDisk.petal_cp_support_dxMax/2.0, //dxMax
                                          0.0);
    
    
    
		G4LogicalVolume *FTDPetalAirLogical = new G4LogicalVolume(FTDPetalAirSolid,
                                                              _AirMat,
                                                              "FTDPetalAirLogical", 
                                                              0, 
                                                              0, 
                                                              0);
		FTDPetalAirLogical->SetVisAttributes(VisAttAirPetal);
    
		
    // Placing N-copies of the air petal inside the air disk. The copies are built using the z-axis as
    // the axis of rotation
		const G4int petal_max_number = (G4int)(360.0*deg/(2.0*theta)) ; 
		for (int i = 0; i < petal_max_number; i++)		
        { 
#ifdef DEBUG_PETAL
          if(i != DEBUG_PETAL )
              {
            continue;
              }
#endif
          G4RotationMatrix *rotPetal = new G4RotationMatrix();
          
          // Put the petal in the position inside the disk
          double petalCdtheta = i*2.0*theta;
          rotPetal->rotateZ(-petalCdtheta);
          
          int zsign = pow((double)-1,i);
          
          
          
          
          // Petal i=0 parameters for gear
          if( i == 0 )
              {
            _ftdparameters[gearpar::PHI0].push_back(petalCdtheta);
            _ftdparameters[gearpar::PETAL0SIGNOFFSET].push_back(zsign);
              }
#ifdef DEBUG_PETAL
          _ftdparameters[gearpar::PHI0].push_back(0);
          _ftdparameters[gearpar::PETAL0SIGNOFFSET].push_back(1);
#endif
          
          double dx = (petal_cp_support_dy/2.0 + _inner_radius)*sin(i*2.0*theta);
          double dy = (petal_cp_support_dy/2.0 + _inner_radius)*cos(i*2.0*theta); 
          double dz = zsign*( _dbParDisk.petal_support_zoffset) ;
          
          G4Transform3D transPetal( *rotPetal, G4ThreeVector( dx, dy, dz) );
          
          
          Phys = G4ReflectionFactory::Instance()->Place(
                                                        transPetal,
                                                        "FTDPetalAir",
                                                        FTDPetalAirLogical,
                                                        FTDDiskLogical,
                                                        false,
                                                        i+1);
          registerPV( Phys );
          
#ifdef DEBUG_VALUES
          G4cout << "===================================================================== " << "\n" <<
          "FTDPetalAir:\n" << 
          " Petal Offset = " << zsign*_dbParDisk.petal_support_zoffset << 
          " Inner Radius= " << _inner_radius <<  "\n" <<
          " Outer Radius= " << _outer_radius <<  "\n" <<
          " xMax = " << _dbParDisk.petal_cp_support_dxMax <<  "\n" <<
          " xMin = " << 2.0*petal_cp_supp_half_dxMin << "\n" <<
          " dy =   " << petal_cp_support_dy << "\n" <<
          " thickness =   " << petalairthickness_half*2.0 << "\n" <<
          " placed at \n" << 
          " x =   " <<  transPetal.dx() << "\n" <<
          " y =   " <<  transPetal.dy() << "\n" <<
          " z =   " <<  transPetal.dz() << "\n" <<
          G4endl;
#endif
          
        }   
    
    //Gear disk parameters
		int sensorType = gear::FTDParameters::PIXEL;  
                int isDoubleSided = false;
                int nSensors = 1;
		if( _dbParDisk.sensor_is_pixel != 1 ) {
                  
                  sensorType = gear::FTDParameters::STRIP;
                  isDoubleSided = true;
                  nSensors = 2;
                }
#ifdef DEBUG_PETAL
		_ftdparameters[gearpar::NPETALS].push_back(1);
#else
		_ftdparameters[gearpar::NPETALS].push_back(petal_max_number);
#endif
		_ftdparameters[gearpar::SENSORTYPE].push_back(sensorType);
                _ftdparameters[gearpar::ISDOUBLESIDED].push_back(isDoubleSided);
                _ftdparameters[gearpar::NSENSORS].push_back(nSensors);
		_ftdparameters[gearpar::ZPOSITION].push_back(_z_position);
		_ftdparameters[gearpar::ZOFFSET].push_back(_dbParDisk.petal_support_zoffset);		
		_ftdparameters[gearpar::ALPHA].push_back(0.0); // staggered design has no tilt
		_ftdparameters[gearpar::HALFANGLEPETAL].push_back(_dbParCommon.petal_half_angle_support);
    
    //=END=============================== AIR PETAL =================================END=/
    
    //=========================== PETALS & SENSORS ==============================/ 
    
		/******************************************************
     ** Support, sensors and electronics are built via   **
     ** DoAnPlaceDisk, see the appropiate functions:     **
     **                                                  **  
     **   +---------------------++--------------------+  **
     **   |    Petal Supports   ||       sensors      |  **
     **   +---------------------++--------------------+  **
     **   | petalSupportPixels  || pixelSensors       |  **
     **   +---------------------++--------------------+  **
     **                                                  **
     **                                                  **
     **                                                  **
     ******************************************************/
    
		std::map<std::string,G4double> valuesDict;
    
		valuesDict["petal_cp_supp_half_dxMin"] = petal_cp_supp_half_dxMin;
		valuesDict["petal_cp_support_dy"] = petal_cp_support_dy;
		valuesDict["_inner_radius"] = _inner_radius;
    
		
		DoAndPlaceDisk( valuesDict, FTDPetalAirLogical );		
    
    //=END======================= PETALS, SENSORS & ELECT. ==========================END=/ 
    
  } while(db->getTuple()!=NULL);
  
	
  //================================ OUTER CYLINDER ==================================/
#ifndef DEBUG_PETAL
#ifndef ONE_DISK
  assert(_dbParDisk.ZStartOuterCylinder>0);
  assert(_dbParDisk.ZStopOuterCylinder>0);
  
	G4double OuterCylinder_half_z = (_dbParDisk.ZStopOuterCylinder-_dbParDisk.ZStartOuterCylinder)/2.;
  assert(OuterCylinder_half_z>0);
  
  G4double OuterCylinder_position = _dbParDisk.ZStartOuterCylinder + OuterCylinder_half_z;
  
	G4Tubs *FTDOuterCylinderSolid = new G4Tubs("FTDOuterCylinder",
                                             OuterCylinderInnerRadius,
                                             OuterCylinderInnerRadius+_dbParCommon.outer_cylinder_total_thickness,
                                             OuterCylinder_half_z,
                                             start_phy, 
                                             stop_phy);
  
  G4LogicalVolume *FTDOuterCylinderLogical= new G4LogicalVolume(FTDOuterCylinderSolid,
                                                                _KaptonMat,
                                                                "FTDOuterCylinder", 
                                                                0, 
                                                                0, 
                                                                0);
  FTDOuterCylinderLogical->SetVisAttributes(VisAttCyl);
	
	G4Transform3D transCylPlus( G4RotationMatrix(), G4ThreeVector(0.,0.,OuterCylinder_position));
	G4Transform3D transCylMinus( G4RotationMatrix(), G4ThreeVector(0.,0.,-OuterCylinder_position));
	transCylMinus = transCylMinus*G4ReflectZ3D();
  
//	Phys= G4ReflectionFactory::Instance()->Place(
//                                               transCylPlus,
//                                               "FTDOuterCylinder",
//                                               FTDOuterCylinderLogical,
//                                               worldLog,
//                                               false,
//                                               0);      
//	registerPV( Phys );
//  
//	Phys= G4ReflectionFactory::Instance()->Place(
//                                               transCylMinus,
//                                               "FTDOuterCylinder",
//                                               FTDOuterCylinderLogical,
//                                               worldLog,
//                                               false,
//                                               0);      
//	registerPV( Phys );
  //=END============================ OUTER CYLINDER =============================END==/
	
  //================================ INNER CYLINDER ==================================/
  //... Inner cylinder (cone)
  assert(_dbParDisk.ZStartInnerCylinder>0);
  assert(_dbParDisk.ZStopInnerCylinder>0);
  
	G4double InnerCylinder_half_z =  (_dbParDisk.ZStopInnerCylinder-_dbParDisk.ZStartInnerCylinder)/2.;
  assert(InnerCylinder_half_z>0);
  
 	//G4double InnerCylinder_position = _dbParDisk.ZStartInnerCylinder + InnerCylinder_half_z; NOT USED
  
  G4double InnerCylinderRmin1 = InnerCylinderOuterRadius1 - _dbParCommon.inner_cylinder_total_thickness - (2.0*_dbParCommon.cable_shield_thickness) - _dbParCommon.cables_thickness ;
  G4double InnerCylinderRmax1 = InnerCylinderOuterRadius1;
  G4double InnerCylinderRmin2 = InnerCylinderOuterRadius2 - _dbParCommon.inner_cylinder_total_thickness - (2.0*_dbParCommon.cable_shield_thickness) - _dbParCommon.cables_thickness ;
  G4double InnerCylinderRmax2 = InnerCylinderOuterRadius2;
	
	G4double cableShieldRmin1 = InnerCylinderRmin1;  G4double cableShieldRmax1 = cableShieldRmin1 + (2.0*_dbParCommon.cable_shield_thickness) + _dbParCommon.cables_thickness ;
  G4double cableShieldRmin2 = InnerCylinderRmin2;
  G4double cableShieldRmax2 = cableShieldRmin2 + (2.0*_dbParCommon.cable_shield_thickness) + _dbParCommon.cables_thickness;
	
  G4double cablesRmin1 = cableShieldRmin1 + _dbParCommon.cable_shield_thickness; 
  G4double cablesRmax1 = cablesRmin1 + _dbParCommon.cables_thickness;
  G4double cablesRmin2 = cableShieldRmin2 + _dbParCommon.cable_shield_thickness; 
  G4double cablesRmax2 = cablesRmin2 + _dbParCommon.cables_thickness;
  
	G4Cons *FTDInnerCylinderSolid = new G4Cons("FTDInnerCylinder",
                                             InnerCylinderRmin1,
                                             InnerCylinderRmax1,
                                             InnerCylinderRmin2,
                                             InnerCylinderRmax2,		 
                                             InnerCylinder_half_z,
                                             start_phy, 
                                             stop_phy);
	
  G4LogicalVolume *FTDInnerCylinderLogical= new G4LogicalVolume(FTDInnerCylinderSolid,
                                                                _KaptonMat,
                                                                "FTDInnerCylinder", 
                                                                0, 
                                                                0, 
                                                                0);
  FTDInnerCylinderLogical->SetVisAttributes(VisAttCyl);
  
  G4Cons *FTDCableShieldSolid = new G4Cons("FTDInnerCableShield",
                                           cableShieldRmin1,
                                           cableShieldRmax1,
                                           cableShieldRmin2,
                                           cableShieldRmax2,
                                           InnerCylinder_half_z,
                                           start_phy, 
                                           stop_phy);
  
  G4LogicalVolume *FTDCableShieldLogical= new G4LogicalVolume(FTDCableShieldSolid,
                                                              _KaptonMat,
                                                              "FTDInnerCableShield", 
                                                              0, 
                                                              0, 
                                                              0);
  FTDCableShieldLogical->SetVisAttributes(VisAttCables);
  
	G4Cons *FTDCablesSolid = new G4Cons("FTDInnerCables",
                                      cablesRmin1,
                                      cablesRmax1,
                                      cablesRmin2,
                                      cablesRmax2,
                                      InnerCylinder_half_z,
                                      start_phy, 
                                      stop_phy);
  
  G4LogicalVolume *FTDCablesLogical= new G4LogicalVolume(FTDCablesSolid,
                                                         _CuMat,
                                                         "FTDInnerCables", 
                                                         0, 
                                                         0, 
                                                         0);
  FTDCablesLogical->SetVisAttributes(VisAttCables);
  
//  //... the cables are placed inside the cylinder
//	Phys = G4ReflectionFactory::Instance()->Place(
//                                                G4Transform3D(),
//                                                "FTDInnerCables",
//                                                FTDCablesLogical,
//                                                FTDCableShieldLogical,
//                                                false,
//                                                0);      
//	registerPV( Phys );
//	
//	Phys = G4ReflectionFactory::Instance()->Place(
//                                                G4Transform3D(),
//                                                "FTDInnerCableShield",
//                                                FTDCableShieldLogical,
//                                                FTDInnerCylinderLogical,
//                                                false,
//                                                0);      
//	registerPV( Phys );
//  
//	Phys = G4ReflectionFactory::Instance()->Place(
//                                                G4Transform3D(G4RotationMatrix(), 
//                                                              G4ThreeVector(0., 0., InnerCylinder_position) ),
//                                                "FTDInnerCylinder",
//                                                FTDInnerCylinderLogical,
//                                                worldLog,
//                                                false,
//                                                0);
//	registerPV( Phys );
//  
//  G4Transform3D Tcyl( G4RotationMatrix(), G4ThreeVector(0.,0.,-InnerCylinder_position) );
//	Tcyl = Tcyl*G4ReflectZ3D();
//	Phys = G4ReflectionFactory::Instance()->Place(
//                                                Tcyl,
//                                                "FTDInnerCylinder",
//                                                FTDInnerCylinderLogical,
//                                                worldLog,
//                                                false,
//                                                0);
	registerPV( Phys );
  //=END============================ INNER CYLINDER =============================END==/
#endif
#endif
  // Closes Database connection
  delete db;
  db = 0;  
  
	return true;
}


//================================ PETAL BUILD FUNCTIONS ======================================/
//***********************************************************************************************
// Build the support, sensors and electronics, choosing what technology have the current disk    
//
// Input Parameters: 
//                   valuesDict: map (name variable, its value)  containing some dimension-disk 
//                               parameters, to be passed to the real building functions
//                   mother:     G4LogicalVolume where will be placed the volumes are going to
//                               build.
//
void FTD_cepc::DoAndPlaceDisk( std::map<std::string,G4double> valuesDict, G4LogicalVolume * mother )
{
  
  petalSupport( valuesDict, mother ) ; // support is placed at 0,0,0 withing the petal
  petalSensor( valuesDict, mother );
  
}

//***********************************************************************************************
// Build the petal  support. The support is a trapezoid made of foam.
//
// Input Parameters: 
//                   valuesDict: map (name variable, its value)  containing some dimension-disk 
//                               parameters, to be passed to the real building functions
//                   mother:     G4LogicalVolume the volumes built are to be placed.

void FTD_cepc::petalSupport( std::map<std::string,G4double> valuesDict, G4LogicalVolume * FTDPetalAirLogical )
{
	G4double petal_cp_supp_half_dxMin = valuesDict["petal_cp_supp_half_dxMin"];
	G4double petal_cp_support_dy = valuesDict["petal_cp_support_dy"];
	G4double _inner_radius = valuesDict["_inner_radius"];
  
	if( _dbParDisk.sensor_is_pixel == 1)
	{
	      G4Trap *FTDPetalSupportSolid = new G4Trap( "FTDPetalSupportSolid",
                                                  _dbParDisk.petal_cp_support_thickness/2.0, //thickness
                                                  0.0,
                                                  0.0,
                                                  petal_cp_support_dy/2.0,  // dy
                                                  petal_cp_supp_half_dxMin, //dxMin 
                                                  _dbParDisk.petal_cp_support_dxMax/2.0, //dxMax
                                                  0.0,
                                                  petal_cp_support_dy/2.0,  // dy
                                                  petal_cp_supp_half_dxMin,  // dxMin
                                                  _dbParDisk.petal_cp_support_dxMax/2.0, //dxMax
                                                  0.0);
  
	      G4LogicalVolume *FTDPetalSupportLogical = new G4LogicalVolume(FTDPetalSupportSolid,
                                                                      _CarbonFiberMat, 
                                                                      "FTDPetalSupportLogical", 
                                                                      0, 
                                                                      0, 
                                                                      0);
	      FTDPetalSupportLogical->SetVisAttributes(_VisAttSupport);
  
 	      G4ThreeVector Ta;
	      Ta.setX(0.0); 
	      Ta.setY(0.0); 
	      Ta.setZ(0.0); 
  
	      G4PhysicalVolumesPair Phys = G4ReflectionFactory::Instance()->Place(
                                                                            G4Transform3D(G4RotationMatrix(),Ta),
                                                                            "FTDPetalSupport",
                                                                            FTDPetalSupportLogical,
                                                                            FTDPetalAirLogical,
                                                                            false,
                                                                            0);
	      registerPV(Phys);
	}
	else
	{
	      G4Trap *FTDPetalSupportCPSolid = new G4Trap( "FTDPetalCPSupport",
	      		_dbParDisk.petal_cp_support_thickness/2.0 *mm,//thickness
	      		0.0,
	      		0.0,
	      		petal_cp_support_dy/2.0,  // height
	      		petal_cp_supp_half_dxMin * mm, 
	      		_dbParDisk.petal_cp_support_dxMax/2.0 * mm,
	      		0.0,
	      		petal_cp_support_dy/2.0,  // height
	      		petal_cp_supp_half_dxMin * mm, 
	      		_dbParDisk.petal_cp_support_dxMax/2.0 * mm,
	      		0.0);

	      
	      //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Holes  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%/
	      G4Trap *FTDPetalSupportHoleDownSolid = SemiPetalSolid( petal_cp_support_dy, "DOWN", false );
	      G4Trap *FTDPetalSupportHoleUpSolid  = SemiPetalSolid( petal_cp_support_dy, "UP", false );
	      
	      // some transformation needed 
	      const G4double petal_cp_holes_separation = 10.0*mm; // WARNING! HARDCODED...

	      G4RotationMatrix * idRot = new G4RotationMatrix;
	      G4ThreeVector movDown( 0.0, -petal_cp_support_dy/4.0+petal_cp_holes_separation/4.0, 0.0 );
	      G4ThreeVector movUp( 0.0, petal_cp_support_dy/4.0-petal_cp_holes_separation/4.0, 0.0);
	      
	      G4SubtractionSolid *FTDPetalSupportSolid_Prov = new G4SubtractionSolid("FTDPetalSupport_Prov",
	      		FTDPetalSupportCPSolid,
	      		FTDPetalSupportHoleDownSolid,
	      		idRot,
	      		movDown
	      		);
	      G4SubtractionSolid *FTDPetalSupportSolid = new G4SubtractionSolid("FTDPetalSupport",
	      		FTDPetalSupportSolid_Prov,
	      		FTDPetalSupportHoleUpSolid,
	      		idRot,
	      		movUp
	      		);
	      //%END%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Holes  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%END%/

	      // Petal support with two holes substracted
	      G4LogicalVolume *FTDPetalSupportLogical = new G4LogicalVolume(FTDPetalSupportSolid,
    	      		_CarbonFiberMat,
    	      		"FTDPetalSupport", 
    	      		0, 
    	      		0, 
    	      		0);
	      FTDPetalSupportLogical->SetVisAttributes(_VisAttSupport);


	      // Placing all together ( support petal boolean  ) inside the air petal container
	      G4PhysicalVolumesPair Phys = G4ReflectionFactory::Instance()->Place(
	      		G4Transform3D(),
	      		"FTDPetalSupport",
	      		FTDPetalSupportLogical,
	      		FTDPetalAirLogical,
	      		false,
	      		0);
	      registerPV(Phys);
	}
	//-END--------------------------- Central Part ----------------------------------END-/
	
  
#ifdef DEBUG_VALUES
  G4cout << "===================================================================== " << "\n" <<
  "FTDPetalSupport:\n" << 
  " Inner Radius= " << _inner_radius <<  "\n" <<
  " Outer Radius= " << _outer_radius <<  "\n" <<
  " xMax = " << _dbParDisk.petal_cp_support_dxMax <<  "\n" <<
  " xMin = " << 2.0*petal_cp_supp_half_dxMin << "\n" <<
  " dy =   " << petal_cp_support_dy << "\n" <<
  " thickness =   " << _dbParDisk.petal_cp_support_thickness << "\n" <<
 // " placed at \n" << 
 // " x =   " <<  Ta.getX() << "\n" <<
 // " y =   " <<  Ta.getY() << "\n" <<
 // " z =   " <<  Ta.getZ() << "\n" <<
  G4endl;
#endif
  
  
  
  
  // Gear Ladder 
	_ftdparameters[gearpar::SUPPORTRINNER].push_back(_inner_radius);
	_ftdparameters[gearpar::SUPPORTLENGTHMIN].push_back(2.0*petal_cp_supp_half_dxMin);
	_ftdparameters[gearpar::SUPPORTLENGTHMAX].push_back(_dbParDisk.petal_cp_support_dxMax);
	_ftdparameters[gearpar::SUPPORTWIDTH].push_back(petal_cp_support_dy);
	_ftdparameters[gearpar::SUPPORTTHICKNESS].push_back(_dbParDisk.petal_cp_support_thickness);
}

//***********************************************************************************************
// Build the petal sensitive. The sensitive volume is a trapezoid made of silicon.
//
// Input Parameters: 
//                   valuesDict: map (name variable, its value)  containing some dimension-disk 
//                               parameters, to be passed to the real building functions
//                   mother:     G4LogicalVolume the volumes built are to be placed.

void FTD_cepc::petalSensor( std::map<std::string,G4double> valuesDict, G4LogicalVolume * FTDPetalAirLogical )
{
	G4double petal_half_dxMin = valuesDict["petal_cp_supp_half_dxMin"];
	G4double petal_dy = valuesDict["petal_cp_support_dy"];
	G4double _inner_radius = valuesDict["_inner_radius"];
  
  G4Trap *FTDPetalSeinsitiveSolid = new G4Trap( "FTDPetalSeinsitiveSolid",
                                               _dbParDisk.disks_Si_thickness/2.0, //thickness
                                               0.0,
                                               0.0,
                                               petal_dy/2.0,  // dy
                                               petal_half_dxMin, //dxMin 
                                               _dbParDisk.petal_cp_support_dxMax/2.0, //dxMax
                                               0.0,
                                               petal_dy/2.0,  // dy
                                               petal_half_dxMin,  // dxMin
                                               _dbParDisk.petal_cp_support_dxMax/2.0, //dxMax
                                               0.0);
  
  // Now check 
  
  TRKSD_FTD01* sensitive_det = 0 ;
  
  if ( _dbParDisk.sensor_is_pixel ) {
    sensitive_det = _theFTDSD_pixel;
  } 
  else {
    sensitive_det = _theFTDSD_strip;
  }
  
	G4LogicalVolume *FTDPetalSensitiveLogical = new G4LogicalVolume(FTDPetalSeinsitiveSolid,
                                                                  _SiMat, 
                                                                  "FTDPetalSensitiveLogical", 
                                                                  0, 
                                                                  0, 
                                                                  0);
	FTDPetalSensitiveLogical->SetVisAttributes(_VisAttSensitive);
  FTDPetalSensitiveLogical->SetSensitiveDetector(sensitive_det);
  
  
  
  G4ThreeVector Ta;
  Ta.setX(0.0); 
  Ta.setY(0.0); 
   
  // front sensor
  Ta.setZ( (_dbParDisk.petal_cp_support_thickness + _dbParDisk.disks_Si_thickness)/2.0 );    
  
  G4PhysicalVolumesPair Phys_front = G4ReflectionFactory::Instance()->Place(
                                                                            G4Transform3D(G4RotationMatrix(),Ta),
                                                                            "FTDPetalSensitive",
                                                                            FTDPetalSensitiveLogical,
                                                                            FTDPetalAirLogical,
                                                                            false,
                                                                            1);
  
#ifdef DEBUG_VALUES
  G4cout << "===================================================================== " << "\n" <<
  "FTDPetalSensitive:\n" << 
  " Inner Radius= " << _inner_radius <<  "\n" <<
  " Outer Radius= " << _outer_radius <<  "\n" <<
  " xMax = " <<  _dbParDisk.petal_cp_support_dxMax <<  "\n" <<
  " xMin = " << 2.0*petal_half_dxMin << "\n" <<
  " dy =   " << petal_dy << "\n" <<
  " thickness =   " << _dbParDisk.disks_Si_thickness << "\n" <<
  " placed at\n " << 
  " x =   " <<  Ta.getX() << "\n" <<
  " y =   " <<  Ta.getY() << "\n" <<
  " z =   " <<  Ta.getZ() << "\n" <<
  G4endl;
#endif
  
  if(_dbParDisk.double_sided == 1 ) { // first two disks are single sided pixel
    
    // rear sensor
    Ta.setZ( -(_dbParDisk.petal_cp_support_thickness + _dbParDisk.disks_Si_thickness)/2.0 );    
    G4PhysicalVolumesPair Phys_rear = G4ReflectionFactory::Instance()->Place(
                                                                             G4Transform3D(G4RotationMatrix(),Ta),
                                                                             "FTDPetalSensitive",
                                                                             FTDPetalSensitiveLogical,
                                                                             FTDPetalAirLogical,
                                                                             false,
                                                                             2);
    
    registerPV(Phys_front);
    registerPV(Phys_rear);
    
    
#ifdef DEBUG_VALUES
    G4cout << "===================================================================== " << "\n" <<
    "FTDPetalSensitive:\n" << 
    " Inner Radius= " << _inner_radius <<  "\n" <<
    " Outer Radius= " << _outer_radius <<  "\n" <<
    " xMax = " <<  _dbParDisk.petal_cp_support_dxMax <<  "\n" <<
    " xMin = " << 2.0*petal_half_dxMin << "\n" <<
    " dy =   " << petal_dy << "\n" <<
    " thickness =   " << _dbParDisk.disks_Si_thickness << "\n" <<
    " placed at\n " << 
    " x =   " <<  Ta.getX() << "\n" <<
    " y =   " <<  Ta.getY() << "\n" <<
    " z =   " <<  Ta.getZ() << "\n" <<
    G4endl;
#endif
  }  
  
  //        //================================ SILICON SENSORS ==================================/
  //        // Sensors build as boxes of silicon. Assemblyblblaa
  //        // 
  //        //
  //	G4double pixel_si_width = 9.9 * mm; //FIXME: From DB 
  //	G4double pixel_si_length = 7.0 * mm; //FIXME: From DB 
  //	G4double pixel_si_interspace = 5.0 * um; //FIXME: From DB
  //    
  //	G4Box * FTDPixelSolid = new G4Box( "FTDPixelSensor",
  //                                      pixel_si_length/2.0,
  //                                      pixel_si_width/2.0,
  //                                      _dbParDisk.disks_Si_thickness/2.0
  //                                      );
  //    
  //	G4LogicalVolume * FTDPixelLogical = new G4LogicalVolume( FTDPixelSolid,
  //                                                            _SiMat,
  //                                                            "FTDPixelSensor",
  //                                                            0,
  //                                                            0,
  //                                                            0);
  //    
  //	FTDPixelLogical->SetVisAttributes( _VisAttSensitive );
  //	FTDPixelLogical->SetSensitiveDetector(_theFTDSD); //Sensitive
  //    
  //        // Defining two (one) rows of pixels per assembly
  //	std::vector<G4AssemblyVolume*> pixelsAssemblyRows;
  //	
  //	G4int howManyTotalRows = (G4int)(petal_cp_support_dy/pixel_si_width);
  //        // How many rows have in the first assembly (disk 1 = 2, disk 2 =2 );
  //        // except the first assembly, all the others have two rows per assembly
  //	G4int numberRows = 2;
  //	if( howManyTotalRows % 2 != 0 )
  //        {
  //		numberRows = 1;
  //        }
  //	G4int howManyMinPixels = (G4int)(2.0*petal_cp_supp_half_dxMin/pixel_si_length);
  //#ifdef DEBUG_VALUES
  //	G4cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n" <<
  //    "Total Pixel rows: " << howManyTotalRows << " ( really done " << howManyTotalRows/2.0 << " assemblies ) \n";
  //#endif
  //	
  //	G4ThreeVector rowTrans( 0.0, 0.0, 0.0);
  //	for(int row = 0; row < (howManyTotalRows/2 + howManyTotalRows%2); ++row)
  //        {
  //            //Instantiating the assembly
  //		pixelsAssemblyRows.push_back( new G4AssemblyVolume() );
  //            // Number of pixels (lowest row should have 4)
  //		G4int nPixel = howManyMinPixels+row;
  //            // Positioning: note that if there are a odd number of
  //            // pixels, the center pixel is centered in x=0
  //		G4double x_offset = 0.0;
  //		if( nPixel % 2 == 1 )
  //            {
  //			x_offset = pixel_si_length/2.0;
  //                // Placing the central pixel
  //			rowTrans.setX( 0.0 );
  //			for(int j = 0; j < numberRows; ++j)
  //                {
  //				rowTrans.setY( pixel_si_width*(1.0/2.0 + j) +pixel_si_interspace );
  //				pixelsAssemblyRows.back()->AddPlacedVolume( FTDPixelLogical, 
  //                                                           rowTrans, (G4RotationMatrix*)0 );
  //                }
  //            }
  //            // The others pixels except the central one, if there is
  //		for(int pixelId = 0 ; pixelId < nPixel/2; ++pixelId)
  //            {
  //			rowTrans.setX( x_offset + pixel_si_length/2.0 + pixel_si_interspace + pixelId*pixel_si_length); //pixel_si_offset
  //			for(int j = 0; j < numberRows; ++j)
  //                {
  //				rowTrans.setY( pixel_si_width*(1.0/2.0 + j) + pixel_si_interspace );
  //                    // Assembly built as two (or one) rows of pixels
  //				pixelsAssemblyRows.back()->AddPlacedVolume( FTDPixelLogical, 
  //                                                           rowTrans, (G4RotationMatrix*)0 );
  //				rowTrans.setX( -rowTrans.getX() ) ;
  //				pixelsAssemblyRows.back()->AddPlacedVolume( FTDPixelLogical, 
  //                                                           rowTrans, (G4RotationMatrix*)0 );
  //                }
  //            }
  //            // All the others assemblies have two rows
  //		numberRows = 2;
  //        }
  //        //Placing the assemblies inside the air petal, begining from the bottom	
  //        //	G4double dz = _dbParDisk.petal_cp_support_thickness/2.0 + _dbParDisk.kapton_petal_thickness + _dbParDisk.disks_Si_thickness/2.0; 
  //	G4double dz = _dbParDisk.disks_Si_thickness /2.0; 
  //    
  //	G4double dx = 0.0;
  //	G4double dy = -petal_cp_support_dy/2.0;
  //	G4ThreeVector inMotherTrans(dx, dy, dz); 
  //	if( howManyTotalRows % 2 != 0 )
  //        {	
  //            numberRows = 1;
  //        }
  //	for( std::vector<G4AssemblyVolume*>::iterator assemblyRow = pixelsAssemblyRows.begin();
  //        assemblyRow != pixelsAssemblyRows.end(); ++assemblyRow )
  //        {
  //#ifdef DEBUG_VALUES
  //		static int i = 0;
  //        G4cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n" <<
  //        " Placement pixels sensors: \n" 
  //        << "   Row " << i << ": dx=" << dx << ", dy=" << dy << ", dz=" << dz 
  //        << " -- # pixel: " << (*assemblyRow)->GetInstanceCount() << "\n";
  //        G4cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n" << G4endl;
  //		i++;
  //#endif
  //        (*assemblyRow)->MakeImprint( FTDPetalSupportAirLogical, inMotherTrans, (G4RotationMatrix*)0 );
  //            // Ready to put the next one
  //		dy += 2.0*pixel_si_width;
  //		inMotherTrans.setY( dy );
  //		if( numberRows == 1 )
  //            {
  //			dy -= pixel_si_width;
  //			inMotherTrans.setY( dy );
  //			numberRows = 2;
  //            }
  //        }
  //        // Gear
  //	G4int howManyPixelsUp = (G4int)(_dbParDisk.petal_cp_support_dxMax/pixel_si_length);
	
	_ftdparameters[gearpar::SENSITIVERINNER].push_back(_inner_radius);
	_ftdparameters[gearpar::SENSITIVELENGTHMIN].push_back(2.0*petal_half_dxMin);
	_ftdparameters[gearpar::SENSITIVELENGTHMAX].push_back(_dbParDisk.petal_cp_support_dxMax);
	_ftdparameters[gearpar::SENSITIVEWIDTH].push_back(petal_dy);
	_ftdparameters[gearpar::SENSITIVETHICKNESS].push_back(_dbParDisk.disks_Si_thickness);
}



//=END============================ PETAL BUILD FUNCTIONS ==================================END=/


//============================= PETAL DIMENSION FUNCTIONS =====================================/
// Functions to extract relative dimensions of the petals. The mechanical design for the petals
// is made taking as reference one disk (disk 1 for pixels, disk 4 for strips), so the petal
// is parametrized using variables given for the database (or calculates from them). All the
// dimensions of the petal are extracted having the inner radius and outer radius of the disk,
// the top length of the petal and the angle defined by the petal using trigonometry.
//
//   (*)outer radius
//                    dxMax/2                     dxMax
//	           ============               =============
//	          |          //               \           /
//      dy        |  (*)<--/ /                 \  PETAL  /
//	          |      /  /                   \       /
//	          -====/===/--> dxMin/2          =======   dxMin
//	          |  /    /   
//  inner radius  |/     /
//                -·····/
//	          |    /             dy = _outer_radius*cos( arcsin(dxMax/(2*_outer_radius)) ) - _inner_radius
//	          |   /              ( a = dxMax/(2*tag(theta)) - _inner_radius - dy )
//	 a        |  /               dxMin = 2*tan(theta)*( _inner_radius + a )
//	          |-/ theta
//	          |/
//    
//
// So, changing the inner radius it'll change the dy and dxMin as well, providing the diferents widths
// of the petal, needed to build the sensors, holes, etc...

//------------------------------------------------------------------------------------------
// Get the dy of the petal which corresponds to a given radius
//
// Input Parameters:   inner radius
// Output Parameters:  dy
G4double FTD_cepc::Getdy(const G4double & innerRadius )
{
	return _outer_radius*cos( asin(_dbParDisk.petal_cp_support_dxMax/(2.0*_outer_radius)) ) - innerRadius;
}
//------------------------------------------------------------------------------------------
// Get the dxMin of the petal which corresponds to a given radius
//
// Input Parameters:   inner radius
// Output Parameters:  dxMin
G4double FTD_cepc::Getdx( const G4double & innerRadius )
{
	G4double a = _dbParDisk.petal_cp_support_dxMax/(2.0*tan(_dbParCommon.petal_half_angle_support)) - innerRadius - 
  Getdy( innerRadius );
	
	return 2.0*(innerRadius + a)*tan(_dbParCommon.petal_half_angle_support);
}

//------------------------------------------------------------------------------------------
// Get the dimensions of the up and down holes or the silicon up or down sensors:
// 
// Input parameters:      
//                   petal_cp_support_dy: height of the air petal container
//                   whereItgoes:         "UP" or "DOWN", where is placed
//                   isSilicon:           True of False, define if is the sensor or the holes
//
// Output:         
//                  std::vector<G4double>* =  [ xMin_half, xMax_half, dy_half, thickness_half ]
std::vector<G4double> * FTD_cepc::GetPetalDimensions( const G4double& petal_cp_support_dy, const G4String & whereItgoes, const G4bool isSilicon )
{
	const G4double theta = _dbParCommon.petal_half_angle_support;
	
	G4double central_separation_y = 10.0*mm/2.0; //HARDCODED _dbParDisk.petal_cp_holes_separation/2.0;
	G4double x_dim = 6.0*mm/cos(theta);          //HARDCODED _dbParDisk.petal_cp_holes_width_support/cos(theta);
	G4double y_dimension = 10.0*mm;        // HARDCODED _dbParDisk.petal_cp_holes_separation;
	G4double half_thickness = _dbParDisk.petal_cp_support_thickness/2.0;
    
        //Silicon detector or Hole?
	if(isSilicon)
        {
		central_separation_y = 0.0;
		const G4double padUp_Si_dxMax = 118.46*mm; 
		x_dim = (_dbParDisk.petal_cp_support_dxMax-padUp_Si_dxMax)/2.0; // HARDCODED
		//x_dim = (_dbParDisk.petal_cp_support_dxMax-_dbParDisk.padUp_Si_dxMax)/2.0;
		y_dimension = y_dimension/2.0;
		half_thickness = _dbParDisk.disks_Si_thickness/2.0;
        }
	
	G4double pseudo_radius_up;
	G4double pseudo_radius_down;
        // Up or down?
	if( whereItgoes == "UP" )
        {
        
		pseudo_radius_up   = _inner_radius+petal_cp_support_dy - y_dimension;
		pseudo_radius_down = _inner_radius+petal_cp_support_dy*_dbParCommon.petal_y_ratio + central_separation_y ;
        }
	else if( whereItgoes == "DOWN" )
        {
		pseudo_radius_up = _inner_radius + petal_cp_support_dy*_dbParCommon.petal_y_ratio - central_separation_y;
		pseudo_radius_down = _inner_radius + y_dimension;
        }
	else
        {
		G4cout << "FTD_cepc: Internal Error: The function FTD_cepc::SemiPetalSolid is not well called, the " <<
        "4th argument must be \"UP\" or \"DOWN\".\n Check the code!!" << G4endl;
		exit(-1);
        }
	const G4double xMin_half = (Getdx( pseudo_radius_down ) - x_dim)/2.0;
	const G4double xMax_half = (Getdx( pseudo_radius_up )- x_dim)/2.0;
	const G4double dy = pseudo_radius_up - pseudo_radius_down;
#ifdef DEBUG_VALUES
	G4cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n" <<
    "    IsSilicon?: " << isSilicon <<  
    " " << whereItgoes <<
    " xMin=" << 2.*xMin_half << 
    " xMax=" << 2.0*xMax_half << 
    " dy= " << dy << std::endl;
	G4cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n" << G4endl;
#endif
	std::vector<G4double> * dimensions = new std::vector<G4double>;
	dimensions->push_back( xMin_half );
	dimensions->push_back( xMax_half );
	dimensions->push_back( dy/2.0 );
	dimensions->push_back( half_thickness );
    
	return dimensions;
}
//
//    //------------------------------------------------------------------------------------------
//    // Construction of the up and down holes or the silicon up or down sensors
//    // 
//    // Input parameters:      
//    //                   petal_cp_support_dy: height of the air petal container
//    //                   whereItgoes:         "UP" or "DOWN", where is placed
//    //                   isSilicon:           True of False, define if is the sensor or the holes
G4Trap * FTD_cepc::SemiPetalSolid( const G4double& petal_cp_support_dy, const G4String& whereItgoes, const G4bool isSilicon )
{
        // Get dimensions
	std::vector<G4double> * dimensions = GetPetalDimensions( petal_cp_support_dy, whereItgoes, isSilicon );
	G4double xMin_half = dimensions->at(0);
	G4double xMax_half = dimensions->at(1);
	G4double dy_half = dimensions->at(2);
	G4double half_thickness = dimensions->at(3);
	delete dimensions;

	G4Trap *FTDSemiPetalSolid = new G4Trap( "FTDSemiPetal"+whereItgoes+"Support",
                                           half_thickness * mm,//thickness
                                           0.0,
                                           0.0,
                                           dy_half,  // height
                                           xMin_half * mm, 
                                           xMax_half * mm,
                                           0.0,
                                           dy_half,  // height
                                           xMin_half * mm, 
                                           xMax_half * mm,
                                           0.0);
	
	return FTDSemiPetalSolid;
}
//=END========================= PETAL DIMENSION FUNCTIONS =================================END=/

//=========================== PARAMETERS SETTERS FUNCTIONS ====================================/
//*********************************************************************************************
// Set Environment variables (dependent of other subdetectors)
void FTD_cepc::SetEnvironPar( const CGAGeometryEnvironment & env )
{
	_glEnv.TPC_Ecal_Hcal_barrel_halfZ = env.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ") * mm;
  _glEnv.Ecal_endcap_zmin = env.GetParameterAsDouble("Ecal_endcap_zmin") * mm ;
  _glEnv.TPC_inner_radius = env.GetParameterAsDouble("TPC_inner_radius") * mm ;
	
  _glEnv.SIT1_Half_Length_Z = env.GetParameterAsDouble("SIT1_Half_Length_Z") * mm;
  _glEnv.SIT2_Half_Length_Z = env.GetParameterAsDouble("SIT2_Half_Length_Z") * mm;
  _glEnv.SIT1_Radius = env.GetParameterAsDouble("SIT1_Radius") * mm;
  _glEnv.SIT2_Radius = env.GetParameterAsDouble("SIT2_Radius") * mm;
  _glEnv.VXD_layer3_maxZ = env.GetParameterAsDouble("VXD_length_r3") * mm;
  
  _glEnv.zEnd_IPOuterTube = (env.GetParameterAsDouble("TUBE_IPOuterTube_end_z")) * mm ;  // ---> A lo mejor no hacen falta
  _glEnv.rEnd_IPOuterTube = (env.GetParameterAsDouble("TUBE_IPOuterTube_end_radius")) * mm;
  _glEnv.zEnd_IPOuterBulge = (env.GetParameterAsDouble("TUBE_IPOuterBulge_end_z")) * mm;
  _glEnv.rEnd_IPOuterBulge = (env.GetParameterAsDouble("TUBE_IPOuterBulge_end_radius")) * mm;
  
  _glEnv.beamTubeTangent = ( _glEnv.rEnd_IPOuterBulge - _glEnv.rEnd_IPOuterTube ) / (_glEnv.zEnd_IPOuterBulge - _glEnv.zEnd_IPOuterTube);
  
}

//*********************************************************************************************
// Set variables common to all disk, dumping 'common_parameters' table from 'ftd08' database
void FTD_cepc::SetdbParCommon( Database * db )
{
  // Getting common_parameters table
  db->exec("select * from common_parameters;");
  db->getTuple();
	
  _dbParCommon.beamTubeClearance = db->fetchDouble("beamtube_clearance") * mm; 
  _dbParCommon.outer_cylinder_total_thickness = db->fetchDouble("outer_cylinder_total_thickness") * mm;
	_dbParCommon.inner_cylinder_total_thickness = _dbParCommon.outer_cylinder_total_thickness;
  _dbParCommon.cable_shield_thickness = db->fetchDouble("cable_shield_thickness") * mm;
  _dbParCommon.cables_thickness = db->fetchDouble("cables_thickness") * mm;
  
  // check that there is enough space for the cables and support
  if( _dbParCommon.beamTubeClearance < (_dbParCommon.cables_thickness + (2.0*_dbParCommon.cable_shield_thickness) + 0.5 *mm) ) 
      {
    G4cout << "FTD_cepc:Stop: Not enough space for inner support structure and cables: increase beamTubeClearance" << G4endl;
    exit(1);
      }
  
	_dbParCommon.ftd1_vtx3_distance_z =  db->fetchDouble("ftd1_vtx3_distance_z") *mm; 
  _dbParCommon.ftd7_ecal_distance_z =  db->fetchDouble("ftd7_ecal_distance_z") *mm; 
  _dbParCommon.ftd1_sit1_radial_diff =  db->fetchDouble("ftd1_sit1_radial_diff") *mm; 
  _dbParCommon.ftd2_sit1_radial_diff =  db->fetchDouble("ftd2_sit1_radial_diff") *mm; 
  _dbParCommon.ftd3_sit2_radial_diff =  db->fetchDouble("ftd3_sit2_radial_diff") *mm; 
  _dbParCommon.ftd4to7_tpc_radial_gap =  db->fetchDouble("ftd4to7_tpc_radial_gap") *mm; 
  // Petal Central Part Support: X-Y dimensions, thickness and angles. Same constant values
  // for all the micro-strips disks
	_dbParCommon.petal_half_angle_support = db->fetchDouble("petal_half_angle_support") * deg;
	_dbParCommon.petal_y_ratio = db->fetchDouble("petal_y_ratio") * mm;
 
  db->exec("select * from extended_reconstruction_parameters;");
  db->getTuple();
  
  _dbParExReco.strip_width_mm = db->fetchDouble("strip_width_mm") * mm;
  _dbParExReco.strip_length_mm = db->fetchDouble("strip_length_mm") * mm;
  _dbParExReco.strip_pitch_mm = db->fetchDouble("strip_pitch_mm") * mm;
  _dbParExReco.strip_angle_deg = db->fetchDouble("strip_angle_deg") * mm;
  
#ifdef DEBUG_VALUES
  G4cout << "FTD_cepc:SetdbParCommon:\n" 
  << "beamTubeClearance = " << _dbParCommon.beamTubeClearance << " \n"   
  << "outer_cylinder_total_thickness = " << _dbParCommon.outer_cylinder_total_thickness << " \n"   
  << "inner_cylinder_total_thickness = " << _dbParCommon.inner_cylinder_total_thickness << " \n"   
  << "cable_shield_thickness = " << _dbParCommon.cable_shield_thickness << " \n"   
  << "cables_thickness = " << _dbParCommon.cables_thickness << " \n"   
  << "ftd1_vtx3_distance_z = " << _dbParCommon.ftd1_vtx3_distance_z << " \n"   
  << "ftd7_ecal_distance_z = " << _dbParCommon.ftd7_ecal_distance_z << " \n"   
  << "ftd1_sit1_radial_diff = " << _dbParCommon.ftd1_sit1_radial_diff << " \n"   
  << "ftd2_sit1_radial_diff = " << _dbParCommon.ftd2_sit1_radial_diff << " \n"   
  << "ftd3_sit2_radial_diff = " << _dbParCommon.ftd3_sit2_radial_diff << " \n"   
  << "ftd4to7_tpc_radial_gap = " << _dbParCommon.ftd4to7_tpc_radial_gap << " \n"   
  << "petal_half_angle_support = " << _dbParCommon.petal_half_angle_support << " \n"   
  << "petal_y_ratio = " << _dbParCommon.petal_y_ratio << " \n"   
  << "strip_width_mm = " << _dbParExReco.strip_width_mm << " \n"   
  << "strip_length_mm = " << _dbParExReco.strip_length_mm << " \n"   
  << "strip_pitch_mm = " << _dbParExReco.strip_pitch_mm << " \n"   
  << "strip_angle_deg = " << _dbParExReco.strip_angle_deg << " \n"   
  << G4endl;

#endif
  
}

//*********************************************************************************************
// Set variables disk number specific, dumping 'disk' table from 'ftd08' database
void FTD_cepc::SetParDisk( Database * db)
{
	_dbParDisk.disk_number = db->fetchInt( "disk_number" );
  _dbParDisk.disks_Si_thickness = db->fetchDouble("disk_si_thickness") * mm ;
  _dbParDisk.petal_cp_support_thickness = db->fetchDouble("petal_cp_support_thickness") * mm ;
  _dbParDisk.petal_cp_support_dxMax = db->fetchDouble("petal_cp_support_dxMax") * mm; 
	_dbParDisk.petal_support_zoffset = db->fetchDouble("petal_support_zoffset") * mm; //NEW
  _dbParDisk.sensor_is_pixel = db->fetchInt("sensor_is_pixel"); //NEW
  _dbParDisk.double_sided = db->fetchInt("double_sided"); //NEW
  
#ifdef DEBUG_VALUES
  G4cout << "FTD_cepc:SetParDisk:\n" 
  << "disk_number = " << _dbParDisk.disk_number << " \n"   
  << "sensor_is_pixel = " << _dbParDisk.sensor_is_pixel << " \n"   
  << "double_sided = " << _dbParDisk.double_sided << " \n"   
  << "petal_support_zoffset = " << _dbParDisk.petal_support_zoffset << " \n"   
  << "disks_Si_thickness = " << _dbParDisk.disks_Si_thickness << " \n"   
  << "petal_cp_support_thickness = " << _dbParDisk.petal_cp_support_thickness << " \n"   
  << "petal_cp_support_dxMax = " << _dbParDisk.petal_cp_support_dxMax << " \n"   
  << "petal_support_zoffset = " << _dbParDisk.petal_support_zoffset << " \n"   
  << G4endl;
#endif
  
}
//=END======================= PARAMETERS SETTERS FUNCTIONS ================================END=/


//*********************************************************************************************
// Register two ftd sensitive detectors, one for the pixel disks and one for the strip disks
void FTD_cepc::RegisterSDs( Database * db )
{
  // Getting parameters disk-specific
  db->exec("select * from disks;");
  db->getTuple();
  
  double minDiskThickness (MAXFLOAT);
  do
    	{
    _dbParDisk.disks_Si_thickness = db->fetchDouble("disk_si_thickness") * mm ;
    if(minDiskThickness>_dbParDisk.disks_Si_thickness)
        {
			minDiskThickness = _dbParDisk.disks_Si_thickness;
        }
     	} while(db->getTuple()!=NULL);
  
  this->_theFTDSD_pixel =  new TRKSD_FTD01( "FTD_PIXEL", minDiskThickness * 340 * keV/mm * 0.2,true);
  RegisterSensitiveDetector(_theFTDSD_pixel);

  this->_theFTDSD_strip =  new TRKSD_FTD01( "FTD_STRIP", minDiskThickness * 340 * keV/mm * 0.2,true);
  RegisterSensitiveDetector(_theFTDSD_strip);

}


void FTD_cepc::registerPV(const G4PhysicalVolumesPair & pvPair )
{
	if( pvPair.first != 0 )
      {
		_registerPV.push_back( pvPair.first );
      }
	if( pvPair.second != 0 )
      {
		_registerPV.push_back( pvPair.second );
      }
}

//================================ GEAR STUFF FUNCTIONS ====================================/
#ifdef MOKKA_GEAR

void FTD_cepc::GearSetup()
{	
  //--- Added carbon fiber.  
  //    TODO: It is needed some other changes??  
  //    October, 2010, J.Duarte
  G4double Si_RadLen, Si_dEdx;
  G4double Kapton_RadLen, Kapton_dEdx;
  G4double CarbonFiber_RadLen, CarbonFiber_dEdx;
  G4double Cu_RadLen, Cu_dEdx;
	
  Si_RadLen = _SiMat->GetRadlen();
  Kapton_RadLen = _KaptonMat->GetRadlen();
  Cu_RadLen = _CuMat->GetRadlen();
  CarbonFiber_RadLen = _CarbonFiberMat->GetRadlen();
	
  //... Looping over bins in the DEDX table to obtain the mip DEDX 
  //... From energy 0.0001MeV to 1000MeV in steps of 10 (See GetdEdx function)
  Si_dEdx=GetdEdx( _SiMat );
  Kapton_dEdx=GetdEdx(_KaptonMat);
	CarbonFiber_dEdx = GetdEdx(_CarbonFiberMat);
  Cu_dEdx=GetdEdx( _CuMat );
  
  // Parameters for FTD
  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  
	gear::FTDParametersImpl* ftdParam = new gear::FTDParametersImpl();
  
  
  // Write gearParameters to GearMgr
	for(unsigned int layer = 0; layer < _ftdparameters[gearpar::NPETALS].size(); layer++)
      {
    // Extract all the param
		int nPetals       = (int)_ftdparameters[gearpar::NPETALS].at(layer);
                int nSensors      = (int) _ftdparameters[gearpar::NSENSORS].at(layer);
                bool isDoubleSided = (bool)_ftdparameters[gearpar::ISDOUBLESIDED].at(layer);
		int sensorType    = (int)_ftdparameters[gearpar::SENSORTYPE].at(layer);
		double phalfangle = _ftdparameters[gearpar::HALFANGLEPETAL].at(layer);
		double phi0       = _ftdparameters[gearpar::PHI0].at(layer);
    // Correct the sign: axis of the trapezoids built inside a ref. with Z --> -Z
		double signoffset = -_ftdparameters[gearpar::PETAL0SIGNOFFSET].at(layer);
		double alpha      = _ftdparameters[gearpar::ALPHA].at(layer);
		double zposition  = _ftdparameters[gearpar::ZPOSITION].at(layer);
		double zoffset    = _ftdparameters[gearpar::ZOFFSET].at(layer);
		double suprtRin   = _ftdparameters[gearpar::SUPPORTRINNER].at(layer);
		double suprtThic  = _ftdparameters[gearpar::SUPPORTTHICKNESS].at(layer);
		double suprtLMin  = _ftdparameters[gearpar::SUPPORTLENGTHMIN].at(layer);
		double suprtLMax  = _ftdparameters[gearpar::SUPPORTLENGTHMAX].at(layer);
		double suprtW     = _ftdparameters[gearpar::SUPPORTWIDTH].at(layer);
    //double suprtRL   = _ftdparameters[gearpar::SUPPORTRADLENGTH].at(layer); FIXME
		double suprtRL   = Si_RadLen;
		double sensitRin  = _ftdparameters[gearpar::SENSITIVERINNER].at(layer);
		double sensitThic = _ftdparameters[gearpar::SENSITIVETHICKNESS].at(layer);
		double sensitLMin = _ftdparameters[gearpar::SENSITIVELENGTHMIN].at(layer);
		double sensitLMax = _ftdparameters[gearpar::SENSITIVELENGTHMAX].at(layer);
		double sensitW    = _ftdparameters[gearpar::SENSITIVEWIDTH].at(layer);
    //double sensitRL   = _ftdparameters[gearpar::SENSITIVERADLENGTH].at(layer); //FIXME
		double sensitRL   = Si_RadLen;
    
		ftdParam->addLayer( nPetals, nSensors, isDoubleSided, sensorType, phalfangle, phi0, alpha,zposition, zoffset, signoffset,
                       suprtRin, suprtThic, 
                       suprtLMin, suprtLMax,
                       suprtW, suprtRL,
                       sensitRin, sensitThic,
                       sensitLMin, sensitLMax,
                       sensitW, sensitRL ) ;
      }
  
  
  // Add the extended_reconstruction_parameters
  
  ftdParam->setDoubleVal("strip_width_mm",_dbParExReco.strip_width_mm);
  ftdParam->setDoubleVal("strip_length_mm",_dbParExReco.strip_length_mm);
  ftdParam->setDoubleVal("strip_pitch_mm",_dbParExReco.strip_pitch_mm);
  ftdParam->setDoubleVal("strip_angle_deg",_dbParExReco.strip_angle_deg);
  
  
	gearMgr->setFTDParameters(ftdParam);
}

G4double FTD_cepc::GetdEdx( const G4Material * material )
{
	G4ParticleTable* theParticleTable=G4ParticleTable::GetParticleTable();
  
  //... Looping over bins in the DEDX table to obtain the mip DEDX 
  //... From energy 0.0001MeV to 1000MeV in steps of 10
  const G4double step_size=10;
  
	G4double mindEdx = 99999; //Initialization
  
  G4EmCalculator findDEdx;
  G4double CurrentdEdx=0;
	for(G4double step=0.0001; step<=1000.0; step+=step_size)
    	{
		CurrentdEdx= findDEdx.ComputeTotalDEDX(step,theParticleTable->FindParticle("mu-"), material);
		if(CurrentdEdx<mindEdx)
        {
			mindEdx=CurrentdEdx;
        }
    	}
  
  return (mindEdx)/1000.0;
}
//=END============================ GEAR STUFF FUNCTIONS ===============================END=/
#endif
