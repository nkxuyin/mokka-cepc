// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: YokePL04.cc,v 1.3 2008/10/31 15:41:52 frank Exp $
// $Name:  $
//
// History:  
// - first implementation P. Mora de Freitas (May 2001)
// - selectable symmetry, self-scaling, removed pole tips 
// - Adrian Vogel, 2006-03-17
// - muon system plus
//   instrumented pole tip back for TESLA models   
// - Predrag Krstonosic , 2006-08-30
// - added barrelEndcapGap, gear parameters, made barrel 
//   and endcap same thickness, made plug insensitive,  
// - F.Gaede, DESY 2008-10-04
//
#include "Yoke05.hh"
#include "G4PVPlacement.hh"
#include "CGADefs.h"
#include "MySQLWrapper.hh"
#include "Control.hh"
#include "MyPlacement.hh"
#include "G4UserLimits.hh"
#include "Hcal04.hh"
#include "CGAGeometryManager.hh"
#include "muonSD.hh"
#include "HECSD.hh"
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
#include "G4SubtractionSolid.hh"
#include <stdlib.h>

#ifdef MOKKA_GEAR
#include "gearimpl/GearParametersImpl.h"
#include "gearimpl/CalorimeterParametersImpl.h"
#include "MokkaGear.h"
#endif

INSTANTIATE(Yoke05)

G4bool Yoke05::ContextualConstruct(const CGAGeometryEnvironment &env, 
				   G4LogicalVolume *worldLog)
{
  Database *db = new Database(env.GetDBName());
  db->exec("SELECT * FROM `yoke`;");
  db->getTuple();
  //... Geometry parameters from the environment and from the database
  symmetry = db->fetchInt("symmetry");
  const G4double rInnerBarrel  = 
    env.GetParameterAsDouble("Yoke_barrel_inner_radius");
  const G4double rInnerEndcap  = 
    env.GetParameterAsDouble("Yoke_endcap_inner_radius");
  const G4double zStartEndcap  = 
    env.GetParameterAsDouble("Yoke_Z_start_endcaps");

  db->exec("SELECT * FROM `muon`;");
  db->getTuple();
  iron_thickness               = db->fetchDouble("iron_thickness");
  G4double gap_thickness       = db->fetchDouble("layer_thickness");
  number_of_layers             = db->fetchInt("number_of_layers");
  G4double yokeBarrelEndcapGap = db->fetchInt("barrel_endcap_gap");
  G4double cell_dim_x          = db->fetchDouble("cell_size");
  G4double cell_dim_z          = db->fetchDouble("cell_size"); 
  G4double chamber_thickness   = 10*mm;   

  //... calculations named constants
  const G4double fullAngle        = 360*deg;
  const G4double tiltAngle        = 180*deg/symmetry - 90*deg; 
  //... the yoke should always rest on a flat side
  //... const G4double tiltAngle2          = 180 * deg / symmetry;

  //... Barrel parameters: 
  //... tolerance 1 mm
  G4double yokeBarrelThickness    = gap_thickness 
    + number_of_layers*(iron_thickness + gap_thickness) 
    + 3*(5.6*iron_thickness + gap_thickness) 
    + 1*mm;
  G4double rOuterBarrel           =    rInnerBarrel + yokeBarrelThickness;    
  G4double z_halfBarrel           =    zStartEndcap - yokeBarrelEndcapGap;    

  //... Endcap parameters:
  G4double yokeEndcapThickness    =   number_of_layers*(iron_thickness 
							+ gap_thickness) + 2*(5.6*iron_thickness + gap_thickness);
  const G4double zEndcap          =   zStartEndcap + yokeEndcapThickness/2;

  G4double  Angle = 360*deg / symmetry;
  G4double Angle2 = acos(-1.)/symmetry;

  //	 const G4double zEndEndcap    =  zStartEndcap + yokeEndcapThickness ;

  //... Print Info: 
  cout << "  ...Yoke  db: symmetry             " << symmetry <<endl;
  cout << "  ...Yoke  db: rInnerBarrel         " << rInnerBarrel <<endl;
  cout << "  ...Yoke  db: rInnerEndcap         " << rInnerEndcap <<endl;
  cout << "  ...Yoke  db: zStartEndcap         " << zStartEndcap <<endl;

  cout << "  ...Muon  db: iron_thickness       " << iron_thickness <<endl;
  cout << "  ...Muon  db: gap_thickness        " << gap_thickness <<endl;
  cout << "  ...Muon  db: number_of_layers     " << number_of_layers <<endl;
  cout << "  ...Muon  db: yokeBarrelEndcapGap  " << yokeBarrelEndcapGap <<endl;

  cout << "  ...Muon par: yokeBarrelThickness  " << yokeBarrelThickness <<endl;
  cout << "  ...Muon par: Barrel_half_z        " << z_halfBarrel <<endl;

  cout << "  ...Muon par: yokeBarrelEndcapGap  " << yokeBarrelEndcapGap <<endl;
  cout << "  ...Muon par: zStartEndcap         " << zStartEndcap <<endl;
  cout << "  ...Muon par: yokeEndcapThickness  " << yokeEndcapThickness <<endl;

  cout << "  ...Muon par: cell_dim_x           " << cell_dim_x <<endl;
  cout << "  ...Muon par: cell_dim_z           " << cell_dim_z <<endl;
  //...

  const G4double zBarrelArray[2]      = 
    { -(zStartEndcap-yokeBarrelEndcapGap)/3.0, 
      +(zStartEndcap-yokeBarrelEndcapGap)/3.0};  
  const G4double rInnerBarrelArray[2] =     { rInnerBarrel, rInnerBarrel };
  const G4double rOuterBarrelArray[2] =     { rOuterBarrel, rOuterBarrel };

  //...  
#ifdef MOKKA_GEAR
  gear::CalorimeterParametersImpl* barrelParam =
    new gear::CalorimeterParametersImpl(rInnerBarrel, 
					zStartEndcap,
					symmetry,
					0.0);

  gear::CalorimeterParametersImpl* endcapParam =
    new gear::CalorimeterParametersImpl(    rInnerEndcap, 
					    rOuterBarrel,
					    zStartEndcap,
					    2, //fwd/bckwd for endcap
					    0.0);

  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setYokeBarrelParameters( barrelParam ) ;
  gearMgr->setYokeEndcapParameters( endcapParam ) ;		  
#endif
	
  //... Materials
  G4Material *yokeMaterial = CGAGeometryManager::GetMaterial("iron");
  
  //... User Limits 
  const G4double maxStep  = 1.0*mm;  // max allowed step size in this volume
  const G4double maxTrack = DBL_MAX; // max total track length
  const G4double maxTime  = DBL_MAX; // max time
  const G4double minEkine = 0;       // min kinetic energy (charged particles)
  const G4double minRange = 0;       // min remaining range (charged particles)

  G4UserLimits *userLimits = new G4UserLimits(maxStep,maxTrack,
                                              maxTime,minEkine,minRange);

  //...########################################################################
  //... Plug Construction 
  //... ******* plug detector idea see TESLA-tdr IV p.122, now is solid

  G4String is_plug=env.GetParameterAsString("Yoke_with_plug");

  cout << " Plug is  " << is_plug << endl;

  if(is_plug=="true")
    {
      //       G4double shift_in_layer=0.0*mm; 
      //       muonSD* muonPlugSD =  
      //       new muonSD(cell_dim_x ,cell_dim_z, chamber_thickness,2,"MuonPlug",1);
      //       RegisterSensitiveDetector(muonPlugSD);

      const G4double HCAL_z = 
	atof(((*Control::globalModelParameters)["calorimeter_region_zmax"]).c_str())*mm;

      db->exec("SELECT * FROM `muon`;");
      db->getTuple();

      const G4double HCAL_plug_gap= db->fetchDouble("Hcal_plug_gap")*mm;
      const G4double plug_thickness = zStartEndcap-HCAL_z-HCAL_plug_gap;
      HCAL_R_max=atof(((*Control::globalModelParameters)["Hcal_R_max"]).c_str())*mm;

      cout << "  ...Plug par: HCAL_half_z          " << HCAL_z <<endl;
      cout << "  ...Plug par: HCAL_Plug_Gap        " << HCAL_plug_gap <<endl;
      cout << "  ...Plug par: Plug Thickness       " << plug_thickness <<endl;
      cout << "  ...Plug par: Plug Radius          " << HCAL_R_max <<endl;

      G4cout   <<HCAL_z<<" "<<HCAL_plug_gap<<" "<<zStartEndcap
	       <<" "<<HCAL_R_max<<" "<<plug_thickness<<G4endl;
      // fg: the plug should have same outer radius as the hcal endcap 
      // note: HCAL_R_max is outer edge radius of the barrel with 16-fold symmentry
      // the hcal endcap only extends to an 'assumed' octagonal barrel 
      // that fits into the 16-fold shape -> cos(pi/8)

      HCAL_R_max= HCAL_R_max*cos( 180.*deg/16.) *cos( 180.*deg/8.) ;

      // cout << "HCal_R_max "<< HCAL_R_max << endl;

      if (plug_thickness<0.0*mm) 
	Control::Abort("Yoke05: Plug thickness negative \n Check the geometry! Sorry for abort :( ",MOKKA_OTHER_ERRORS);

      const G4double zPosPlugArray[2]   = 
	{ -plug_thickness/2.0-0.5*mm, +plug_thickness/2.0-0.5*mm };
      const G4double rInnerPlugArray[2] = { rInnerEndcap, rInnerEndcap };
      const G4double rOuterPlugArray[2] = { HCAL_R_max, HCAL_R_max };
  
 
      G4Polyhedra *plugSolid0 = new G4Polyhedra("YokePlugSolid0", 
						tiltAngle, 
						fullAngle, 
						symmetry, 
						2,
						zPosPlugArray, 
						rInnerPlugArray, 
						rOuterPlugArray);

      G4LogicalVolume *plugLog0 = new G4LogicalVolume(plugSolid0, 
						      yokeMaterial, 
						      "YokePlugLog0", 
						      0, 
						      0, 
						      0);
      G4Polyhedra *plugSolid1 = new G4Polyhedra("YokePlugSolid1", 
						tiltAngle, 
						fullAngle, 
						symmetry, 
						2,
						zPosPlugArray, 
						rInnerPlugArray, 
						rOuterPlugArray);

      G4LogicalVolume *plugLog1 = new G4LogicalVolume(plugSolid1, 
						      yokeMaterial, 
						      "YokePlugLog1", 
						      0, 
						      0, 
						      0);
 
      G4VisAttributes *plugVisAttributes = 
	new G4VisAttributes(G4Colour(1.0, 0.0, 0.0)); // red!
      plugVisAttributes->SetForceWireframe(true);
      plugVisAttributes->SetDaughtersInvisible(false);
      plugLog0->SetVisAttributes(plugVisAttributes);
      plugLog1->SetVisAttributes(plugVisAttributes);

#ifdef MOKKA_GEAR 
      gear::CalorimeterParametersImpl* plugParam =
	new gear::CalorimeterParametersImpl(rInnerEndcap, HCAL_R_max, 
					    zStartEndcap-plug_thickness, 
					    2, //fwd/bckwd for endcap/plug
					    0. );
      plugParam -> setDoubleVal("YokePlugThickness", plug_thickness ) ;
      gearMgr   -> setYokePlugParameters( plugParam ) ;
#endif

      //... 
      const G4double shift_plug=zStartEndcap-plug_thickness/2; 
    
      new G4PVPlacement(                                  0, 
							  G4ThreeVector(0, 0, +shift_plug), 
							  plugLog0, 
							  "YokePlug0", 
							  worldLog, 
							  false, 
							  0);
      new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(180 * deg), 
				      G4ThreeVector(0, 0, -shift_plug)),
			plugLog1, 
			"YokePlug1", 
			worldLog, 
			false, 
			0);
   
    } 
  //... end of plug

  //...#######################################################################
  //... muonSD - Muon Sensitive Detectors

  muonSD *muonBarrelSD = new muonSD(cell_dim_x,cell_dim_z,
				    chamber_thickness,1,"MuonBarrel",1);
  RegisterSensitiveDetector(muonBarrelSD);

  muonSD * muonECSD =  new muonSD(cell_dim_x,cell_dim_z,
				  chamber_thickness,2,"MuonEndCap",1);
  RegisterSensitiveDetector(muonECSD);


  //...#######################################################################
  //... Barrel Yoke Construction

  G4Polyhedra *barrelSolid = new G4Polyhedra("YokeBarrelSolid", 
					     tiltAngle,fullAngle,symmetry,2, 
					     zBarrelArray,rInnerBarrelArray,rOuterBarrelArray);

  G4LogicalVolume *barrelLog1 = new G4LogicalVolume(barrelSolid, 
						    yokeMaterial, 
						    "YokeBarrelSolid", 
						    0, 0, 0);
  G4LogicalVolume *barrelLog2 = new G4LogicalVolume(barrelSolid, 
						    yokeMaterial, 
						    "YokeBarrelSolid", 
						    0, 0, 0);
  G4LogicalVolume *barrelLog3 = new G4LogicalVolume(barrelSolid, 
						    yokeMaterial, 
						    "YokeBarrelSolid", 
						    0, 0, 0);

  G4VisAttributes *VisAtt_barrel 
    = new G4VisAttributes(G4Colour(0.0,1.0,0.0));
  VisAtt_barrel->SetForceWireframe(true);
  barrelLog1->SetVisAttributes(VisAtt_barrel);
  barrelLog2->SetVisAttributes(VisAtt_barrel);
  barrelLog3->SetVisAttributes(VisAtt_barrel);
  VisAtt_barrel->SetDaughtersInvisible(false);

       
  new G4PVPlacement(                                               0, 
								   G4ThreeVector(0.,0.,-2*(zStartEndcap-yokeBarrelEndcapGap)/3), 
								   barrelLog1,"YokeBarrelSolid",worldLog,false,0);

  new G4PVPlacement(                                               0, 
								   G4ThreeVector(0.,0.,0.),barrelLog2,"YokeBarrelSolid",worldLog, 
								   false,0);

  new G4PVPlacement(                                              0, 
								  G4ThreeVector(0.,0.,2*(zStartEndcap-yokeBarrelEndcapGap)/3), 
								  barrelLog3,"YokeBarrelSolid",worldLog,false,0);

  //...########################################################################
  //... Barrel Instrumentation
  G4double radius_low, radius_mid, radius_sensitive;

  //... addition of 1+10+2+1 layers of detectors (06.2012) in barel Yoke 

  for(int i=0; i<=number_of_layers+3 ;i++)
    {
      radius_low = 
	rInnerBarrel+ 0.5*mm + i*gap_thickness + i*iron_thickness; 
      radius_mid       = radius_low+0.5*gap_thickness;  
      radius_sensitive = radius_mid;

      if( i>=10 )
	{ radius_low =  
	    rInnerBarrel + 0.5*mm + i*gap_thickness 
	    + (i+(i-10)*4.6)*iron_thickness;
	  radius_mid       = radius_low+0.5*gap_thickness;  
	  radius_sensitive = radius_mid;
	}

      if(i==0)
	{
#ifdef MOKKA_GEAR
	  barrelParam->layerLayout().addLayer(gap_thickness, 
					      cell_dim_x,cell_dim_z,0.0 );
#endif	
	}
      if( i>0&&i<=10 )
	{
#ifdef MOKKA_GEAR
	  barrelParam->layerLayout().addLayer(iron_thickness + gap_thickness, 
					      cell_dim_x,cell_dim_z,iron_thickness );
#endif	
	}
      if( i>10 )
	{ 
#ifdef MOKKA_GEAR
	  barrelParam->layerLayout().addLayer(5.6*iron_thickness+gap_thickness, 
					      cell_dim_x,cell_dim_z,5.6*iron_thickness );
#endif	
	}


      //... safety margines of 0.1 mm for x,y of chambers
      G4double dx = radius_low*tan(Angle2)-0.1*mm;
      G4double dy = (zStartEndcap-yokeBarrelEndcapGap)/3.0-0.1*mm; 

      G4Box* ChamberSolid1b = new G4Box("layer1",dx,gap_thickness/2.,dy);
      G4Box* ChamberSolid2b = new G4Box("layer2",dx,gap_thickness/2.,dy);
      G4Box* ChamberSolid3b = new G4Box("layer3",dx,gap_thickness/2.,dy);

      cout << "  ...Barrel i, position: "<<i <<" "<<radius_sensitive<<endl;
      muonBarrelSD->AddLayer(i+1,dx,radius_sensitive,0.0); 


      for(int j=0;j<symmetry;j++)
	{
	  G4int id1=1+i+1000*(j+1)+100000*2;
	  G4int id2=1+i+1000*(j+1)+100000*3;
	  G4int id3=1+i+1000*(j+1)+100000*4;
 
	  G4LogicalVolume *ChamberLogic1b = BuildRPC1Box(ChamberSolid1b,
							 muonBarrelSD,id1,userLimits,db);

	  G4LogicalVolume *ChamberLogic2b = BuildRPC1Box(ChamberSolid2b, 
							 muonBarrelSD,id2,userLimits,db);

	  G4LogicalVolume *ChamberLogic3b = BuildRPC1Box(ChamberSolid3b, 
							 muonBarrelSD,id3,userLimits,db);

	  G4VisAttributes *VisAtt_chamber_b = 
	    new G4VisAttributes(G4Colour(0.,0.,1.));
	  VisAtt_chamber_b->SetForceWireframe(true);
	  ChamberLogic1b->SetVisAttributes(VisAtt_chamber_b);
	  ChamberLogic2b->SetVisAttributes(VisAtt_chamber_b);
	  ChamberLogic3b->SetVisAttributes(VisAtt_chamber_b);


	  G4double phirot=Angle*(j+1); 
	  G4double phirotp=phirot+90*deg;
	  muonBarrelSD->SetStaveRotationMatrix(j+1, phirot);

	  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateZ(phirot), 
					  G4ThreeVector(radius_mid*cos(phirotp),
							radius_mid*sin(phirotp),
							0.)),
			    ChamberLogic1b, 
			    "chamber1", 
			    barrelLog1, 
			    false,
			    0 );

	  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateZ(phirot),
					  G4ThreeVector(radius_mid*cos(phirotp),
							radius_mid*sin(phirotp),
							0.)), 
			    ChamberLogic2b, 
			    "chamber2", 
			    barrelLog2, 
			    false, 
			    0);

	  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateZ(phirot),
					  G4ThreeVector(radius_mid*cos(phirotp),
							radius_mid*sin(phirotp),
							0.)), 
			    ChamberLogic3b, 
			    "chamber3", 
			    barrelLog3, 
			    false, 
			    0);
	}
    }
  //... end of barrel


  //...#####################################################################
  //... EndCap Yoke Construction

  const G4double   zEndcapArray[2]    =    
    { -yokeEndcapThickness/2+1.*mm, +yokeEndcapThickness/2+1.*mm};  
  const G4double rInnerEndcapArray[2] =     { rInnerEndcap, rInnerEndcap };
  const G4double rOuterEndcapArray[2] =     { rOuterBarrel, rOuterBarrel };

  G4Polyhedra *endcapSolid =  new G4Polyhedra(    "endcapSolid", 
						  tiltAngle, 
						  fullAngle, 
						  symmetry, 
						  2,
						  zEndcapArray, 
						  rInnerEndcapArray, 
						  rOuterEndcapArray ); 
  //... End Cap's Log Volumes
  G4LogicalVolume *endcapLog1 =  new G4LogicalVolume( endcapSolid, 
						      yokeMaterial, 
						      "endcapLog1", 
						      0, 0, 0 );

  G4LogicalVolume *endcapLog2 =  new G4LogicalVolume( endcapSolid, 
						      yokeMaterial, 
						      "endcapLog2", 
						      0, 0, 0 );

  G4VisAttributes *VisAtt_endcup = 
    new G4VisAttributes(G4Colour(0.5,0.5,.5));
  VisAtt_endcup->SetForceWireframe(true);
  endcapLog1->SetVisAttributes(VisAtt_endcup);
  endcapLog2->SetVisAttributes(VisAtt_endcup);
  VisAtt_endcup->SetDaughtersInvisible(false);
  //... Positioning 
  new G4PVPlacement(0,G4ThreeVector(0, 0, +zEndcap), 
		    endcapLog1,"YokeEndcap1",worldLog,false,0);

  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(180*deg), 
				  G4ThreeVector(0, 0,-zEndcap)),endcapLog2,
		    "YokeEndcap2",worldLog,false,0);

  //...#####################################################################
  //... EndCap's Instrumentation
  G4double shift_middle; 
  G4double shift_sensitive;

  //... # of layers in EndCup's  10 + 2 
  for(int i=0; i<=number_of_layers+1; i++)
    {
      shift_middle    = - yokeEndcapThickness/2 +0.5*mm 
	+ iron_thickness*(i+1) 
	+ (i+0.5)*gap_thickness; 
      shift_sensitive = zEndcap+shift_middle; 

      if( i>= 10)
	{
	  shift_middle    = - yokeEndcapThickness/2 +0.5*mm 
	    + iron_thickness*(i+1+(i-9)*4.6) + (i+0.5)*gap_thickness; 
	  shift_sensitive =  zEndcap+shift_middle; 

	}

      //...
      if( i<10)
	{
#ifdef MOKKA_GEAR
	  endcapParam->layerLayout().addLayer(iron_thickness+gap_thickness, 
					      cell_dim_x,cell_dim_z, iron_thickness);
#endif	
	}
      if( i>= 10)
	{
#ifdef MOKKA_GEAR
	  endcapParam->layerLayout().addLayer(5.6*iron_thickness+gap_thickness, 
					      cell_dim_x,cell_dim_z, 5.6*iron_thickness);
#endif	
	}
      //...
      const G4double zGapEndcapArrayc[2]   = 
	{ -gap_thickness/2.0, gap_thickness/2.0 };
      const G4double rInnerEndcapArrayc[2] = 
	{ rInnerEndcap+1.*mm, rInnerEndcap+1.*mm };
      const G4double rOuterEndcapArrayc[2] =     
	{ rOuterBarrel-1.*mm, rOuterBarrel-1.*mm };

      G4Polyhedra* ChamberSolide = new G4Polyhedra(          "layer1e", 
							     tiltAngle, 
							     fullAngle, 
							     symmetry,
							     2, 
							     zGapEndcapArrayc, 
							     rInnerEndcapArrayc, 
							     rOuterEndcapArrayc);
      //...
      G4int id1=i+1+1000*(0+1)+100000*1;
      G4int id2=i+1+1000*(0+1)+100000*5;

      G4LogicalVolume *ChamberLogic1e = BuildRPC1ECShape( ChamberSolide,
							  muonECSD,id1,userLimits,db,env);

      G4LogicalVolume *ChamberLogic2e = BuildRPC1ECShape( ChamberSolide,
							  muonECSD,id2,userLimits,db,env);
      //...
      muonECSD->AddLayer(i+1,0.0,0.0,shift_sensitive);
      cout << "  ...Endcap i, position: "<<i <<" "<<shift_sensitive<<endl;

      G4VisAttributes *VisAtt_chamber_e = 
	new G4VisAttributes(G4Colour(0.,0.0,1.0));
      VisAtt_chamber_e->SetForceWireframe(true);
      ChamberLogic1e->SetVisAttributes(VisAtt_chamber_e);
      ChamberLogic2e->SetVisAttributes(VisAtt_chamber_e);
      //...
      new G4PVPlacement(G4Transform3D(G4RotationMatrix(),
				      G4ThreeVector(0,0,shift_middle)),ChamberLogic1e,
			"chamber1",endcapLog1,false,0);

      new G4PVPlacement(G4Transform3D(G4RotationMatrix(),
				      G4ThreeVector(0,0,shift_middle)),ChamberLogic2e, 
			"chamber2",endcapLog2,false,0);
    }      
  delete db;
  return true;

}
//... end of endcap
//... end of Construction

//...######################################################################
//... BSciS Barrel Scintillation Sensor
G4LogicalVolume * Yoke05::BuildRPC1Box(              G4Box *ChamberSolid,
						     muonSD *theSD, 
						     G4int layer_id,
						     G4UserLimits *pULimits,
						     Database *db )
{
  G4Material *Air= CGAGeometryManager::GetMaterial("Air");
  G4Material *Sci= CGAGeometryManager::GetMaterial("polystyrene");
  G4double gap_thickness       = db->fetchDouble("layer_thickness");
  G4double chamber_thickness   =  gap_thickness/4;

  if(ChamberSolid->GetEntityType()=="G4Box")
    {
      //... fill the ChamberSolid with air
      G4LogicalVolume *ChamberLog = new G4LogicalVolume
	(ChamberSolid,Air,"muonSci", 0, 0, 0 );

      //... Assembly of the layer
      {
	G4Box *pieceSolid = new G4Box( "RPCLayerPieceSolid", 
				       ChamberSolid->GetXHalfLength(), chamber_thickness/2,
				       ChamberSolid->GetZHalfLength());

	G4LogicalVolume *pieceLog = new G4LogicalVolume(pieceSolid, 
							Sci,"RPCLayerPieceLog",0,theSD,pULimits );

	new G4PVPlacement(0, G4ThreeVector(0,0,0),pieceLog,
			  "RPCLayerPiecegas",ChamberLog,false,layer_id );
	G4VisAttributes *VisAtt_pieceLog = 
	  new G4VisAttributes(G4Colour(1.0,0.0,0.0));
	VisAtt_pieceLog->SetForceWireframe(true);
	pieceLog->SetVisAttributes(VisAtt_pieceLog);
	VisAtt_pieceLog->SetDaughtersInvisible(false);

      }
      return ChamberLog;  
    }
  Control::Abort("Yoke05::BuildRPC1Box: invalid ChamberSolidEnvelope",MOKKA_OTHER_ERRORS);
  return NULL; 
}

//...##########################################################################
//... ESciS Endcap Scintillaton Sensors 

G4LogicalVolume * Yoke05::BuildRPC1ECShape(G4Polyhedra *ChamberSolide,
					   muonSD* theSD,
					   G4int layer_id,
					   G4UserLimits* pULimits, 
					   Database *db,
					   const CGAGeometryEnvironment &env)
{ 

  G4Material * Air= CGAGeometryManager::GetMaterial("Air");
  G4Material * Sci = CGAGeometryManager::GetMaterial("polystyrene");

  db->exec("SELECT * FROM `yoke`;");
  db->getTuple();
  const G4int symmetry               = db->fetchInt("symmetry");
  const G4double rInnerEndcap        = 
    env.GetParameterAsDouble("Yoke_endcap_inner_radius");
  const G4double rInnerBarrel        = 
    env.GetParameterAsDouble("Yoke_barrel_inner_radius");

  db->exec("SELECT * FROM `muon`;");
  db->getTuple();
  G4double iron_thickness      = db->fetchDouble("iron_thickness");
  G4double gap_thickness       = db->fetchDouble("layer_thickness");
  G4int number_of_layers       = db->fetchInt("number_of_layers");
  //...
  G4double chamber_thickness   = 10* mm;   
  //...
  const G4double rOuterBarrel  = rInnerBarrel
    + gap_thickness 
    + number_of_layers*(iron_thickness + gap_thickness) 
    + 3*(5.6*iron_thickness + gap_thickness) 
    + 1*mm;
  //...
  const G4double fullAngle           = 360*deg;
  const G4double tiltAngle           = 180*deg / symmetry - 90*deg; 

  if(ChamberSolide->GetEntityType()=="G4Polyhedra")
    {
      G4LogicalVolume *ChamberLoge = new G4LogicalVolume(ChamberSolide,
							 Air,"muonSci",0,0,0);
      //... Assembly of Detector
      {
	const G4double zPosEndcapArray[2]   = 
	  { - chamber_thickness/ 2, + chamber_thickness / 2 };
	const G4double rInnerEndcapArray[2] =   
	  { rInnerEndcap+1.1*mm, rInnerEndcap+1.1*mm };
	const G4double rOuterEndcapArray[2] =   
	  { rOuterBarrel-1.1*mm, rOuterBarrel-1.1*mm };
  
	G4Polyhedra *pieceSolid = new G4Polyhedra("RPCLayerPieceSolid", 
						  tiltAngle,fullAngle,symmetry,2, 
						  zPosEndcapArray,
						  rInnerEndcapArray,
						  rOuterEndcapArray);

	G4LogicalVolume *pieceLog = new G4LogicalVolume(pieceSolid, 
							Sci,"RPCLayerPieceLog",0,theSD,pULimits );

        new G4PVPlacement(0, G4ThreeVector(0,0,0),pieceLog,
			  "SciPlane",ChamberLoge,false,layer_id );

	G4VisAttributes *VisAtt_pieceLog = 
	  new G4VisAttributes(G4Colour(1.0,0.0,0.0));
	VisAtt_pieceLog->SetForceWireframe(true);
	pieceLog->SetVisAttributes(VisAtt_pieceLog);
	VisAtt_pieceLog->SetDaughtersInvisible(false);

      }      
      return ChamberLoge;  
    }

  Control::Abort("Yoke05::BuildRPC1ECBox: invalid ChamberSolidEnvelope",MOKKA_OTHER_ERRORS);
  return NULL; 
}
