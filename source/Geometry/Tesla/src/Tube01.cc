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
// $Id: Tube01.cc,v 1.3 2007/08/13 08:12:42 kristian Exp $
// $Name: mokka-07-00 $
//
//
// Tube01.cc
//
// History:  
// - first implementation P. Mora de Freitas (sept 02)
// - lateral tube altered B.Pawlik (march 2005)

#include "Control.hh"
#include "G4PVPlacement.hh"
#include "Tube01.hh"
#include "CGAGeometryManager.hh"

#include "globals.hh"

#include "G4Tubs.hh"
#include "G4Cons.hh"
#include "G4Box.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
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

INSTANTIATE(Tube01)

G4bool Tube01::construct(const G4String &aSubDetectorName,
			 G4LogicalVolume *WorldLog)
{
  G4double start_phi = 0.0*deg;
  G4double stop_phi = 360.0*deg;

  G4VisAttributes * VisAttCentral = 
    new G4VisAttributes(G4Colour(1,.75,.5));
  //VisAttCentral->SetForceWireframe(true);
  VisAttCentral->SetForceSolid(true);

  G4VisAttributes * VisAttEnds = 
    new G4VisAttributes(G4Colour(0.8,0.8,0.8));
  //VisAttEnds->SetForceWireframe(true);
  VisAttEnds->SetForceSolid(true);

  G4VisAttributes *VisAttVacuum = 
    new G4VisAttributes(G4Colour(0.,0.6,0.));
  VisAttVacuum->SetForceWireframe(true);
  //VisAttVacuum->SetForceSolid(true);

  G4VisAttributes *VisAttStripLines =
    new G4VisAttributes(G4Colour(1.,1.,1.));
  //VisAttVacuum->SetForceWireframe(true);
  VisAttStripLines->SetForceSolid(true);

  G4PVPlacement* Phys;
  G4RotationMatrix *rot=new G4RotationMatrix();
  rot->rotateX(pi); // the same but other side

  G4cout << "\nBuilding Tube..." << G4endl;
  db = new Database(aSubDetectorName.data());
  
  //******************************************
  // The central tube
  //******************************************
  db->exec("select * from central_tube;");
  db->getTuple();

  central_half_z = db->fetchDouble("half_z");
  central_inner_radious  = db->fetchDouble("inner_radious");
  central_thickness = db->fetchDouble("thickness");

  // beam vacuum inside the tube  
  G4Tubs *CentralTubeVaccumSolid
    = new G4Tubs("CentralTubeVaccum",
		 0.,
		 central_inner_radious,
		 central_half_z,
		 start_phi, 
		 stop_phi);
    
  G4LogicalVolume *CentralTubeVaccumLogical=
    new G4LogicalVolume(CentralTubeVaccumSolid,
			CGAGeometryManager::GetMaterial("beam"),
			//			CGAGeometryManager::GetMaterial("air"),
			"CentralTubeVaccum", 
			0, 
			0, 
			0);
  
  CentralTubeVaccumLogical->SetVisAttributes(VisAttVacuum);
  
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., 0.),
		      CentralTubeVaccumLogical,
		      "CentralTubeVaccum",
		      WorldLog,
		      false,0);

  // Be tube
  G4Tubs *CentralTubeSolid
    = new G4Tubs("CentralTube",
		 central_inner_radious,
		 central_inner_radious+central_thickness,
		 central_half_z,
		 start_phi, 
		 stop_phi);
 
  BeamMaterial = CGAGeometryManager::GetMaterial("beryllium");
  G4LogicalVolume *CentralTubeLogical=
    new G4LogicalVolume(CentralTubeSolid,
			BeamMaterial,
			"CentralTube", 
			0, 
			0, 
			0);
  
  CentralTubeLogical->SetVisAttributes(VisAttCentral);
  
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., 0.),
		      CentralTubeLogical,
		      "CentralTube",
		      WorldLog,
		      false,0);      
  
  //*************************************
  // Lateral cones and tubes
  //*************************************
  G4double lateral_ends_start_z,lateral_ends_stop_z,
    lateral_ends_thickness,lateral_inner_radious;
  G4double lateral_inners_start_z, lateral_inners_thickness,
    striplines_gap,striplines_thickness;
  db->exec("select * from lateral_tubes;");
  db->getTuple();
  lateral_ends_start_z = db->fetchDouble("ends_start_z");
  lateral_ends_stop_z = db->fetchDouble("ends_stop_z");
  lateral_ends_thickness = db->fetchDouble("ends_thickness");
  lateral_inner_radious = db->fetchDouble("inner_radious");

  lateral_inners_start_z = db->fetchDouble("inners_start_z");
  lateral_inners_thickness = db->fetchDouble("inners_thickness");
  striplines_gap = db->fetchDouble("striplines_gap");
  striplines_thickness = db->fetchDouble("striplines_thickness");
  
  // lateral cones
  G4double cone_half_z = 
    (lateral_inners_start_z - central_half_z)/2.;
  assert (cone_half_z>0);

  G4double ZCone;
  ZCone = central_half_z + cone_half_z;

  // Beam vacuum inside cones
  G4Cons *LateralConeVacuum
    = new G4Cons("LateralConeVacuumSolid",
		 0., // Rmin at -Z
		 central_inner_radious, // Rmax at +Z
		 0., // Rmin at +Z
		 lateral_inner_radious, // Rmax at +Z
		 cone_half_z,
		 start_phi,
		 stop_phi);
  
  G4LogicalVolume *LateralConeVacuumLogical=
    new G4LogicalVolume(LateralConeVacuum,
			CGAGeometryManager::GetMaterial("beam"),
			//			CGAGeometryManager::GetMaterial("air"),
			"LateralConeVacuum", 
			0, 
			0, 
			0);
  LateralConeVacuumLogical->SetVisAttributes(VisAttVacuum);

  Phys= new G4PVPlacement(0,
			  G4ThreeVector(0., 0., ZCone),
			  LateralConeVacuumLogical,
			  "LateralConeVacuum",
			  WorldLog,
			  false,0);

  Phys= new G4PVPlacement(rot,
			  G4ThreeVector(0., 0., -ZCone),
			  LateralConeVacuumLogical,
			  "LateralConeVacuum",
			  WorldLog,
			  false,0);

  // Be cones
  G4Cons *LateralCone
    = new G4Cons("LateralConeSolid",
		 central_inner_radious, // Rmin at -Z
		 central_inner_radious+central_thickness, // Rmax at +Z
		 lateral_inner_radious, // Rmin at +Z
		 lateral_inner_radious+lateral_inners_thickness, // Rmax at +Z
		 cone_half_z,
		 start_phi,
		 stop_phi);
  
  G4LogicalVolume *LateralConeLogical=
    new G4LogicalVolume(LateralCone,
			BeamMaterial,
			"LateralCone", 
			0, 
			0, 
			0);
  LateralConeLogical->SetVisAttributes(VisAttCentral);

  Phys= new G4PVPlacement(0,
			  G4ThreeVector(0., 0., ZCone),
			  LateralConeLogical,
			  "LateralCone",
			  WorldLog,
			  false,0);

  Phys= new G4PVPlacement(rot,
			  G4ThreeVector(0., 0., -ZCone),
			  LateralConeLogical,
			  "LateralCone",
			  WorldLog,
			  false,0);
  
  //********************************
  // inner lateral tubes
  //********************************
  G4double lateral_inners_half_z;
  lateral_inners_half_z = 
    (lateral_ends_start_z - lateral_inners_start_z)/2.;
  assert(lateral_inners_half_z>0);
  G4double ZTube;
  ZTube = lateral_inners_start_z + lateral_inners_half_z;

  // inner beam vacuum lateral tubes
  G4Tubs *LateralTubeVacuumSolid
    = new G4Tubs("LateralTubeVacuum",
		 0.,
		 lateral_inner_radious,
		 lateral_inners_half_z,
		 start_phi, 
		 stop_phi);
  
  G4LogicalVolume *LateralTubeVacuumLogical=
    new G4LogicalVolume(LateralTubeVacuumSolid,
			CGAGeometryManager::GetMaterial("beam"),
			//			CGAGeometryManager::GetMaterial("air"),
			"LateralTubeVacuum", 
			0, 
			0, 
			0);
  LateralTubeVacuumLogical->SetVisAttributes(VisAttVacuum);
  
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., ZTube),
		      LateralTubeVacuumLogical,
		      "LateralTubeVacuum",
		      WorldLog,
		      false,0);      

  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., -ZTube),
		      LateralTubeVacuumLogical,
		      "LateralTubeVacuum",
		      WorldLog,
		      false,0);      


  // inner Be lateral tubes
  G4Tubs *LateralTubeSolid
    = new G4Tubs("LateralTubeBe",
		 lateral_inner_radious,
		 lateral_inner_radious+lateral_inners_thickness,
		 lateral_inners_half_z,
		 start_phi, 
		 stop_phi);
  
  G4LogicalVolume *LateralTubeLogical=
    new G4LogicalVolume(LateralTubeSolid,
			BeamMaterial,
			"LateralTubeBe", 
			0, 
			0, 
			0);
  LateralTubeLogical->SetVisAttributes(VisAttCentral);
  
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., ZTube),
		      LateralTubeLogical,
		      "LateralTubeBe",
		      WorldLog,
		      false,0);      

  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., -ZTube),
		      LateralTubeLogical,
		      "LateralTubeBe",
		      WorldLog,
		      false,0);      

  // VXD strip lines on lateral tubes
  G4Tubs *StripLinesLateralSolid
    = new G4Tubs("StripLinesLateral",
		 lateral_inner_radious
		 +lateral_inners_thickness 
		 + striplines_gap,
		 lateral_inner_radious
		 +lateral_inners_thickness 
		 + striplines_gap
		 + striplines_thickness,
		 lateral_inners_half_z,
		 start_phi, 
		 stop_phi);
  
  G4LogicalVolume *StripLinesLateralLogical=
    new G4LogicalVolume(StripLinesLateralSolid,
			CGAGeometryManager::GetMaterial("kapton"),
			"StripLinesLateral", 
			0, 
			0, 
			0);
  StripLinesLateralLogical->SetVisAttributes(VisAttStripLines);
  
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., ZTube),
		      StripLinesLateralLogical,
		      "StripLinesLateral",
		      WorldLog,
		      false,0);      

  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., -ZTube),
		      StripLinesLateralLogical,
		      "StripLinesLateral",
		      WorldLog,
		      false,0);      



  //*******************************
  // ends lateral cones and tubes
  //*******************************
      //
      //altered by B.Pawlik INP PAS Krakow (March 2005)
      // polycone shape, VXD striplines removed
      //
  G4double dz_tube[7];
  G4double z_tube[7];
  G4double r_tube[7];
  G4double r_vacu[7];
  //
  // cone parameters
  //
  G4double dz_cone = (db->fetchDouble("endtube_z1")-lateral_ends_start_z)/2.; 
  G4double r_cone_vac1 = lateral_inner_radious; 
  G4double r_cone_vac2 = db->fetchDouble("endtube_r1"); 
  G4double r_cone_Fe1  = lateral_inner_radious+lateral_ends_thickness;
  G4double r_cone_Fe2  = r_cone_vac2+lateral_ends_thickness;
    z_tube[1]= db->fetchDouble("endtube_z1");
    r_vacu[1]= db->fetchDouble("endtube_r1"); 
    r_tube[1]=r_vacu[1]+lateral_ends_thickness;
    //
    // tube 1
    //
    dz_tube[0]= (db->fetchDouble("endtube_z2")-db->fetchDouble("endtube_z1"))/2.;            
    r_vacu[0]=r_cone_vac2;
    r_tube[0]=r_cone_Fe2 ; 
    //
    // tube 2 
    dz_tube[1]= lateral_ends_thickness/2.;                
    r_vacu[1]=db->fetchDouble("endtube_r2");             // lumiCal (r_min-10mm)
    r_tube[1]=r_cone_Fe2;
   //
   // tube 3
    dz_tube[2]= (db->fetchDouble("endtube_z3")-db->fetchDouble("endtube_z2"))/2.;
    r_vacu[2]= db->fetchDouble("endtube_r2");                          
    r_tube[2]=r_vacu[2]+lateral_ends_thickness;
    //
    // tube 4
    dz_tube[3]= dz_tube[1];                                            
    r_vacu[3]= db->fetchDouble("endtube_r3");                          // BeamCal r_min
    r_tube[3]= r_tube[2];
   //
   // tube 5
    dz_tube[4]= (db->fetchDouble("endtube_z4")-db->fetchDouble("endtube_z3"))/2.;             
    r_vacu[4]=r_vacu[3];                                              // BeamCal r_min
    r_tube[4]=r_vacu[3]+lateral_ends_thickness;
    G4int ntubes = 5;

    // "Beam" from CGAGeometryMnanger causes segmentation fault for misterious reson
    // so define vacuum here for a while 
    /* G4String name;
    G4int ncomponents;
    G4double fractionmass;
    G4double density     = 1.e-5*g/cm3;
    G4double pressure    = 2.e-2*bar;
    G4double temperature = STP_Temperature;                      //from PhysicalConstants.h
    G4Material* beam = new G4Material(name="Beam", density, ncomponents=1,
				      kStateGas,temperature,pressure);
				      beam->AddMaterial(CGAGeometryManager::GetMaterial("air"), fractionmass=1.);*/


  // vacuum lateral cone
  G4Cons *EndConeVacSolid = new G4Cons("EndConeVac",
				       0.,
				       r_cone_vac1,
				       0.,
				       r_cone_vac2,
				       dz_cone,
				       start_phi, 
				       stop_phi);
 // Fe lateral cone
  G4Cons *EndConeFeSolid = new G4Cons("EndConeFe",
				      r_cone_vac1,
				      r_cone_Fe1,
				      r_cone_vac2,
				      r_cone_Fe2,
				      dz_cone,
				      start_phi, 
				      stop_phi);

  G4LogicalVolume* EndConeVacLog = new G4LogicalVolume(EndConeVacSolid,
						       CGAGeometryManager::GetMaterial("beam"),
						       "EndTubeVac0",
						       0,0,0);
  G4LogicalVolume* EndConeFeLog = new G4LogicalVolume(EndConeFeSolid,
						      CGAGeometryManager::GetMaterial("iron"),
						      "EndTubeFe0",
						      0,0,0);
 
   EndConeFeLog->SetVisAttributes(VisAttEnds);
   EndConeVacLog->SetVisAttributes(VisAttVacuum);
   //
   // put vacuum and iron cones into world
   //
  
    ZTube += lateral_inners_half_z +dz_cone;
    // cone 
      new G4PVPlacement(0,G4ThreeVector(0., 0., ZTube),
			EndConeFeLog,
			"BeamConeFe",
			WorldLog,
			false,0);    
      new G4PVPlacement(rot,G4ThreeVector(0., 0., -ZTube),
			EndConeFeLog,
			"BeamConeFe",
			WorldLog,
			false,0);
      new G4PVPlacement(0, G4ThreeVector(0., 0., ZTube),
			EndConeVacLog,
			"BeamConeVac",
			WorldLog,
			false,0);    
      new G4PVPlacement(rot,G4ThreeVector(0., 0., -ZTube),
			EndConeVacLog,
			"BeamConeVac",
			WorldLog,
			false,0);
      // 
      // lateral tubes Fe and vacuum
      //
      G4Tubs* EndTubeFeSolid[5];
      G4Tubs* EndTubeVacSolid[5];
      G4LogicalVolume *EndTubeVacLog[5];
      G4LogicalVolume *EndTubeFeLog[5];
      //
      ZTube += dz_cone;
      for(int i=0;i<ntubes;i++){
	std::stringstream number;
	number << (i+1);

	// solids
	EndTubeFeSolid[i] = new G4Tubs("EndTubeFe",
				       r_vacu[i],
				       r_tube[i],
				       dz_tube[i],
				       start_phi, 
				       stop_phi);
	EndTubeVacSolid[i] = new G4Tubs("EndTubeVac",
					0,
					r_vacu[i],
					dz_tube[i],
					start_phi, 
					stop_phi);
	// logicals
	EndTubeFeLog[i] = new G4LogicalVolume(EndTubeFeSolid[i],
					   CGAGeometryManager::GetMaterial("iron"),
					   G4String("EndTubeFe")+G4String(number.str()),
					   0,0,0);
	EndTubeVacLog[i] = new G4LogicalVolume(EndTubeVacSolid[i],
					       CGAGeometryManager::GetMaterial("beam"),
					       G4String("EndTubeVac")+G4String(number.str()),
					       0,0,0);

	EndTubeFeLog[i]->SetVisAttributes(VisAttEnds);
	EndTubeVacLog[i]->SetVisAttributes(VisAttVacuum);
	//
	// place them into world
	//
	ZTube += dz_tube[i];
	new G4PVPlacement(0,G4ThreeVector(0., 0., ZTube),
			  EndTubeFeLog[i], 
			  G4String("EndTubeFe")+G4String(number.str()),
			  WorldLog,
			  false,0);
	new G4PVPlacement(rot,G4ThreeVector(0., 0.,-ZTube),
			  EndTubeFeLog[i], 
			  G4String("EndTubeFe")+G4String(number.str()),
			  WorldLog,
			  false,0);
	new G4PVPlacement(0,G4ThreeVector(0., 0., ZTube),
			  EndTubeVacLog[i], 
			  G4String("EndTubeVac")+G4String(number.str()),
			  WorldLog,
			  false,0);
	new G4PVPlacement(rot,G4ThreeVector(0., 0.,-ZTube),
			  EndTubeVacLog[i], 
			  G4String("EndTubeVac")+G4String(number.str()),
			  WorldLog,
			  false,0);
	// update ZTube
	ZTube += dz_tube[i];

      }
 
  // Closes Database connection
  delete db;
  db = 0;
  G4cout << "Tube done.\n" << G4endl;
  return true;
}

#ifdef MOKKA_GEAR

void Tube01::GearSetup()
{
  
  G4double CurrentdEdx, BeamPipe_RadLen, BeamPipe_dEdx;
  G4EmCalculator findDEdx;
  G4ParticleTable*theParticleTable=G4ParticleTable::GetParticleTable();

  BeamPipe_RadLen = BeamMaterial->GetRadlen();

  //Looping over bins in the DEDX table to obtain the mip DEDX 
  //From energy 0.0001MeV to 1000MeV in steps of 10
  G4double step_size=10,step,mindEdx=99999;
  
  
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx = 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  BeamMaterial);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }

  BeamPipe_dEdx=(mindEdx)/1000;

  gear::GearParametersImpl* gearParameters = new gear::GearParametersImpl;

  gearParameters -> setDoubleVal( "BeamPipeRadius", central_inner_radious  ) ;
  gearParameters -> setDoubleVal( "BeamPipeHalfZ" ,  central_half_z ) ;
  gearParameters -> setDoubleVal( "BeamPipeThickness" ,  central_thickness) ;
  gearParameters -> setDoubleVal( "BeamPipeProperties_dEdx" , BeamPipe_dEdx ) ;
  gearParameters -> setDoubleVal( "BeamPipeProperties_RadLen" , BeamPipe_RadLen ) ;


  // Write gearParameters to GearMgr
  // Parameters for SIT
  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setGearParameters("BeamPipe", gearParameters ) ;
}


#endif 
