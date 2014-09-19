// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: VXD00.cc,v 1.10 2008/10/06 12:58:35 musat Exp $
// $Name: mokka-07-00 $
//
// History:  
// - first implementation -- Paulo Mora de Freitas, September 2002
// - fixed geometry overlap -- Adrian Vogel, 2005-12-12
// - added optional GEAR output -- R. Lippe, DESY, 2006-09-04

#include "Control.hh"
#include "G4PVPlacement.hh"
#include "VXD00.hh"
#include "TRKSiSD00.hh"
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
#include "gear/VXDParameters.h" 
#include "gearimpl/ZPlanarParametersImpl.h" 
#include "gearimpl/GearParametersImpl.h"
#include "gearimpl/ZPlanarLayerLayoutImpl.h" 
#include "MokkaGear.h"
#include "G4EmCalculator.hh"
#include "G4ParticleTable.hh"
#endif

INSTANTIATE(VXD00)

VXD00::~VXD00()
{
}

G4bool VXD00::construct(const G4String &dbName, G4LogicalVolume *worldLog)
{
  G4VisAttributes *VisAtt;
  G4PVPlacement *Phys;

  G4double sPhi =   0 * deg;
  G4double dPhi = 360 * deg;

  db = new Database(dbName.data());

  //****************************************
  // Layers
  //****************************************
  //
  // Common layer thickness parameters
  db->exec("select * from layers_common_parameters;");
  db->getTuple();
  G4double  support_structure_thickness,active_silicon_thickness,support_structure_radial_thickness;
   
  support_structure_thickness = 
    db->fetchDouble("support_structure_thickness");
  electronics_structure_thickness = 
    db->fetchDouble("electronics_structure_thickness");
  active_silicon_thickness = 
    db->fetchDouble("active_silicon_thickness");
  strip_lines_thickness = 
    db->fetchDouble("strip_lines_thickness");
  support_structure_radial_thickness = 
    db->fetchDouble("support_structure_radial_thickness");
  end_electronics_half_z = 
    db->fetchDouble("end_electronics_half_z");
  strip_final_beampipe_radious = 
    db->fetchDouble("strip_final_beampipe_radious");

  // support shell parameters
  db->exec("select * from support_shell;");
  db->getTuple();
  G4double  shell_inner_radious,shell_half_z,
    shell_thickess;
  shell_inner_radious = db->fetchDouble("inner_radious");
  shell_half_z = db->fetchDouble("half_z");
  shell_thickess = db->fetchDouble("thickess");
  support_endplate_inner_radious = db->fetchDouble("endplate_inner_radious");

  // The VXD Sensitive detector
  // Threshold is 20% of a MIP. For Si we have 
  // 340 KeV/mm as MIP.
  theVXDSD = 
    new TRKSiSD00("VXD", 
		active_silicon_thickness * mm 
		* 340 * keV
		* 0.2);
  RegisterSensitiveDetector(theVXDSD);

#ifdef MOKKA_GEAR
  // some variables for storing information for MOKKA_GEAR
  // during the loop
  std::vector<helpLayer> gearHelpLadders ;
  std::vector<helpLayer> gearHelpSensitives ;
  std::vector<int> gearHelpNumberLadders ;
  std::vector<double> gearHelpPhi0 ;
  G4double gearHelpGap = 0. ;
  G4int gearHelpCount = 0 ;
  G4int gearHelpFraction = 1 ;
#endif

  db->exec("select * from layer;");
  db->getTuple();
  do 
    {
      G4int LayerId;
      G4double Z;
      G4double layer_radios, ladder_dim_z,ladder_gap,
	strip_line_final_z;
      LayerId = db->fetchInt("id");
      layer_radios = db->fetchDouble("layer_radios");
      ladder_dim_z  = db->fetchDouble("ladder_dim_z");
      ladder_gap = db->fetchDouble("ladder_gap");
      strip_line_final_z = db->fetchDouble("strip_line_final_z");
#ifdef LCIO_MODE
      ladder_gapVec.push_back(ladder_gap);
      StripLineFinalZ_Vec.push_back( strip_line_final_z);
#endif
      // Beryllium support tube
      G4Tubs *BerylliumSupportSolid
	= new G4Tubs("BerylliumSupport",
		     layer_radios,
		     layer_radios+support_structure_thickness,
		     ladder_dim_z+ladder_gap,
		     sPhi, 
		     dPhi);
      
      VisAtt = new G4VisAttributes(G4Colour(1.,0.,0.));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
        
      supportMaterial = CGAGeometryManager::GetMaterial("beryllium");

      G4LogicalVolume *BerylliumSupportLogical=
	new G4LogicalVolume(BerylliumSupportSolid,
			    supportMaterial,
			    "BerylliumSupport", 
			    0, 
			    0, 
			    0);
      
      BerylliumSupportLogical->SetVisAttributes(VisAtt);

      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., 0.),
			  BerylliumSupportLogical,
			  "BerylliumSupport",
			  worldLog,
			  false,0);  
      
#ifdef MOKKA_GEAR
      helpLayer thisLadder ;
      thisLadder.distance  = layer_radios ;
      thisLadder.offset    = 0 ;
      thisLadder.thickness = support_structure_thickness ;
      thisLadder.length    = ladder_dim_z ;
      thisLadder.width     = 2*M_PI / gearHelpFraction * layer_radios ;
      thisLadder.radLength = (BerylliumSupportLogical->GetMaterial())->GetRadlen()/mm ;
#endif

      // Electronics end
      // (two dead Si tubes at Si layer ends)
      //
      G4Tubs *ElectronicsEndSolid
	= new G4Tubs("ElectronicsEnd",
		     layer_radios, 
		     layer_radios+electronics_structure_thickness,
		     end_electronics_half_z,
		     sPhi, 
		     dPhi);
      
      VisAtt = new G4VisAttributes(G4Colour(0.,1.,0.));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      
      G4LogicalVolume *ElectronicsEndLogical=
	new G4LogicalVolume(ElectronicsEndSolid,
			    CGAGeometryManager::GetMaterial("silicon_2.33gccm"),
			    "ElectronicsEnd", 
			    0, 
			    0, 
			    0);
      ElectronicsEndLogical->SetVisAttributes(VisAtt);

      Z= ladder_dim_z + ladder_gap/2. + end_electronics_half_z;
      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., Z),
			  ElectronicsEndLogical,
			  "ElectronicsEnd",
			  worldLog,
			  false,0);      
      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., -Z),
			  ElectronicsEndLogical,
			  "ElectronicsEnd",
			  worldLog,
			  false,0);      

      // Strip lines 
      G4double strip_line_start_z,strip_line_half_z;
      strip_line_start_z = ladder_dim_z + 
	ladder_gap/2. + end_electronics_half_z * 2.
	+ shell_thickess; // to avoid overlaps


      strip_line_half_z = 
	(strip_line_final_z - strip_line_start_z) / 2.;
      assert (strip_line_half_z>0);
      
       G4Cons *StripLinesSolid
	= new G4Cons("StripLines",
		     layer_radios, // inside radius at  -fDz
		     layer_radios + strip_lines_thickness, // outside radius at -fDz
		     strip_final_beampipe_radious, // inside radius at  +fDz
		     strip_final_beampipe_radious + 
		     strip_lines_thickness, // outside radius at +fDz
		     strip_line_half_z,
		     sPhi, 
		     dPhi);
 
      VisAtt = new G4VisAttributes(G4Colour(1.,1.,1.));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      

      stripMaterial = CGAGeometryManager::GetMaterial("kapton"); 
      G4LogicalVolume *StripLinesLogical=
	new G4LogicalVolume(StripLinesSolid,
			    stripMaterial,
			    "StripLines", 
			    0, 
			    0,
			    0);
      
      StripLinesLogical->SetVisAttributes(VisAtt);

      Z = strip_line_start_z + strip_line_half_z;
      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., Z),
			  StripLinesLogical,
			  "StripLines",
			  worldLog,
			  false,0);      
      G4RotationMatrix *rot=new G4RotationMatrix();
      rot->rotateX(pi); // the same but other side
      Phys=
	new G4PVPlacement(rot,
			  G4ThreeVector(0., 0., -Z),
			  StripLinesLogical,
			  "StripLines",
			  worldLog,
			  false,0);

      // Si Active layer 
      // (two tubes with the ladder gap in the center)
      
      activeMaterial =CGAGeometryManager::GetMaterial("silicon_2.33gccm"); 

      G4Tubs *SiActiveLayerSolid
	= new G4Tubs("SiActiveLayer",
		     layer_radios+support_structure_thickness,
		     layer_radios+ support_structure_thickness +
		      active_silicon_thickness,
		     ladder_dim_z/2,
		     sPhi, 
		     dPhi);
      
      VisAtt = new G4VisAttributes(G4Colour(0.,1.,1.));
      VisAtt->SetForceWireframe(true);
      //VisAtt->SetForceSolid(true);
      
      G4LogicalVolume *SiActiveLayerLogical=
	new G4LogicalVolume(SiActiveLayerSolid,
			    activeMaterial,
			    "SiActiveLayer", 
			    0, 
			    0, 
			    0);
      SiActiveLayerLogical->SetVisAttributes(VisAtt);
      SiActiveLayerLogical->SetSensitiveDetector(theVXDSD);

      Z= ladder_dim_z/2 + ladder_gap;
      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., Z),
			  SiActiveLayerLogical,
			  "SiActiveLayer",
			  worldLog,
			  false,LayerId);      

      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., -Z),
			  SiActiveLayerLogical,
			  "SiActiveLayer",
			  worldLog,
			  false,LayerId);      

#ifdef MOKKA_GEAR
  // sensitive layer
  helpLayer thisSens ;
  thisSens.distance  = layer_radios + support_structure_thickness ;
  thisSens.offset    = 0. ;
  thisSens.thickness = active_silicon_thickness ;
  thisSens.length    = ladder_dim_z/2. ;
  thisSens.width     = 2*M_PI / gearHelpFraction * layer_radios ;
  thisSens.radLength = (SiActiveLayerLogical->GetMaterial())->GetRadlen()/mm ;
 
  // save information for gear
  gearHelpLadders.push_back( thisLadder );
  gearHelpSensitives.push_back( thisSens ) ;
  gearHelpNumberLadders.push_back( 0 ) ;
  gearHelpPhi0.push_back( 0. ) ;
  gearHelpGap = std::max( gearHelpGap , ladder_gap ) ;
  gearHelpCount ++ ;
#endif

    }
  while(db->getTuple()!=NULL);

  //****************************************
  // Outer support shell
  //****************************************
  // central tube
  G4Tubs *SupportShellSolid
    = new G4Tubs("SupportShell",
		 shell_inner_radious,
		 shell_inner_radious+shell_thickess,
		 shell_half_z,
		 sPhi, 
		 dPhi);
  
  VisAtt = new G4VisAttributes(G4Colour(0.,0.,1.));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  
  G4LogicalVolume *SupportShellLogical=
    new G4LogicalVolume(SupportShellSolid,
			supportMaterial,
			"SupportShell", 
			0, 
			0, 
			0);
  
  SupportShellLogical->SetVisAttributes(VisAtt);


  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., 0.),
		      SupportShellLogical,
		      "SupportShell",
		      worldLog,
		      false,0);

  // support endplates
  G4double support_endplate_half_z;
  support_endplate_half_z = shell_thickess/2;

  G4Tubs *EndPlateShellSolid
    = new G4Tubs("EndPlateShell",
		 support_endplate_inner_radious,
		 shell_inner_radious+shell_thickess,
		 support_endplate_half_z,
		 sPhi, 
		 dPhi);
  
  VisAtt = new G4VisAttributes(G4Colour(0.,0.,1.));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  
  G4LogicalVolume *EndPlateShellLogical=
    new G4LogicalVolume(EndPlateShellSolid,
			supportMaterial,
			"EndPlateShell", 
			0, 
			0, 
			0);
  
  EndPlateShellLogical->SetVisAttributes(VisAtt);

  G4double ZEndPlateShell;
  ZEndPlateShell = shell_half_z + shell_thickess/2.;

  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., ZEndPlateShell),
		      EndPlateShellLogical,
		      "EndPlateShell",
		      worldLog,
		      false,0);
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., -ZEndPlateShell),
		      EndPlateShellLogical,
		      "EndPlateShell",
		      worldLog,
		      false,0);

  //*** Cryostat ***************************************************************
  
  db->exec("SELECT * FROM cryostat;");
  db->getTuple();
  
  rAlu   = db->fetchDouble("alu_skin_inner_radious") * mm;
  drAlu  = db->fetchDouble("alu_skin_tickness") * mm;
  const G4double rSty   = db->fetchDouble("foam_inner_radious") * mm;
  const G4double drSty  = db->fetchDouble("foam_tickness") * mm;
  const G4double dzSty  = db->fetchDouble("foam_half_z") * mm;
  rInner = db->fetchDouble("endplate_inner_radious") * mm;
  
  aluEndcapZ = dzSty + drSty + drAlu / 2;
  const G4double styEndcapZ = dzSty + drSty / 2;

  aluHalfZ = dzSty + drSty;

  aluMaterial = CGAGeometryManager::GetMaterial("aluminium");
  G4VisAttributes *aluVisAttributes = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5));
  aluVisAttributes->SetForceWireframe(true);

  G4Material *styMaterial = CGAGeometryManager::GetMaterial("styropor");
  G4VisAttributes *styVisAttributes = new G4VisAttributes(G4Colour(0.9, 0.9, 0.9));
  styVisAttributes->SetForceWireframe(true);

  G4Tubs *aluBarrelSolid = new G4Tubs("CryostatAluSkinBarrel", rAlu, rAlu + drAlu,aluHalfZ, sPhi, dPhi);
  G4LogicalVolume *aluBarrelLog = new G4LogicalVolume(aluBarrelSolid, aluMaterial, "CryostatAluSkinBarrel", 0, 0, 0);
  aluBarrelLog->SetVisAttributes(aluVisAttributes);
  new G4PVPlacement(0, G4ThreeVector(), aluBarrelLog, "CryostatAluSkinBarrel", worldLog, false, 0);

  G4Tubs *styBarrelSolid = new G4Tubs("CryostatFoamBarrel", rSty, rSty + drSty, dzSty, sPhi, dPhi);
  G4LogicalVolume *styBarrelLog = new G4LogicalVolume(styBarrelSolid, styMaterial, "CryostatFoamBarrel", 0, 0, 0);
  styBarrelLog->SetVisAttributes(styVisAttributes);
  new G4PVPlacement(0, G4ThreeVector(), styBarrelLog, "CryostatFoamBarrel", worldLog, false, 0);

  G4Tubs *aluEndcapSolid = new G4Tubs("CryostatAluSkinEndPlate", rInner, rAlu + drAlu, drAlu / 2, sPhi, dPhi);
  G4LogicalVolume *aluEndcapLog = new G4LogicalVolume(aluEndcapSolid, aluMaterial, "CryostatAluSkinEndPlate", 0, 0, 0);
  aluEndcapLog->SetVisAttributes(aluVisAttributes);
  new G4PVPlacement(0, G4ThreeVector(0, 0, +aluEndcapZ), aluEndcapLog, "CryostatAluSkinEndPlate", worldLog, false, +1);
  new G4PVPlacement(0, G4ThreeVector(0, 0, -aluEndcapZ), aluEndcapLog, "CryostatAluSkinEndPlate", worldLog, false, -1);

  G4Tubs *styEndcapSolid = new G4Tubs("CryostatFoamEndPlate", rInner, rSty + drSty, drSty / 2, sPhi, dPhi);
  G4LogicalVolume *styEndcapLog = new G4LogicalVolume(styEndcapSolid, styMaterial, "CryostatFoamEndPlate", 0, 0, 0);
  styEndcapLog->SetVisAttributes(styVisAttributes);
  new G4PVPlacement(0, G4ThreeVector(0, 0, +styEndcapZ), styEndcapLog, "CryostatFoamEndPlate", worldLog, false, +1);
  new G4PVPlacement(0, G4ThreeVector(0, 0, -styEndcapZ), styEndcapLog, "CryostatFoamEndPlate", worldLog, false, -1);

#ifdef MOKKA_GEAR
  // -------write data to gear

  // get gear manager
  MokkaGear* gearMgr = MokkaGear::getMgr() ;

  // construct VXDParameters
  gear::ZPlanarParametersImpl* vxdParams = 
    new gear::ZPlanarParametersImpl(gear::ZPlanarParametersImpl::CCD ,                // vxd type 
				shell_inner_radious ,                                 // inner radius
				shell_inner_radious+shell_thickess ,                  // outer radius
				shell_half_z ,                                        // half length
				gearHelpGap ,                                         // shell gap
				(SupportShellLogical->GetMaterial())->GetRadlen()/mm ) ; // shell rad length

  // add all layers
  for( int i = 0 ; i < gearHelpCount ; i++ ) {
    vxdParams->addLayer( gearHelpNumberLadders[i] , 0. ,
			 gearHelpLadders[i].distance , gearHelpLadders[i].offset,gearHelpLadders[i].thickness ,
			 gearHelpLadders[i].length , gearHelpLadders[i].width , gearHelpLadders[i].radLength ,
			 gearHelpSensitives[i].distance, gearHelpSensitives[i].offset , gearHelpSensitives[i]. thickness , 
    			 gearHelpSensitives[i].length , gearHelpSensitives[i].width , gearHelpSensitives[i].radLength ) ;

    gearMgr->setVXDParameters( vxdParams ) ;
  }
#endif				


  delete db;
  return true;
}


#ifdef MOKKA_GEAR
 
void VXD00::GearSetup()
{
  G4double electronic_end_length, alu_RadLen,StripLine_RadLen;
  G4double CurrentdEdx,ActiveLayer_dEdx,SupportLayer_dEdx,StripLine_dEdx,Cryostat_dEdx;
  G4double mindEdx = 99999,step,step_size=10;
  G4EmCalculator findDEdx;
  G4ParticleTable*theParticleTable=G4ParticleTable::GetParticleTable();

  electronic_end_length = 2*end_electronics_half_z;


  StripLine_RadLen = stripMaterial->GetRadlen();
  alu_RadLen = aluMaterial->GetRadlen();
  

  //Looping over bins in the DEDX table to obtain the mip DEDX 
  //From energy 0.0001MeV to 1000MeV in steps of 10
  
  mindEdx=99999;
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx = 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  activeMaterial);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }

  ActiveLayer_dEdx=(mindEdx)/1000;
  

  //Looping over bins in the DEDX table to obtain the mip DEDX 
  //From energy 0.0001MeV to 1000MeV in steps of 10
  
  mindEdx=99999;
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx = 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  supportMaterial);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }

  SupportLayer_dEdx=(mindEdx)/1000;
  

  //Looping over bins in the DEDX table to obtain the mip DEDX 
  //From energy 0.0001MeV to 1000MeV in steps of 10
  
  mindEdx=99999;
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx = 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  stripMaterial);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }

  StripLine_dEdx=(mindEdx)/1000;

  //Looping over bins in the DEDX table to obtain the mip DEDX 
  //From energy 0.0001MeV to 1000MeV in steps of 10
  
  mindEdx=99999;
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx = 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  aluMaterial);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }

  Cryostat_dEdx=(mindEdx)/1000;


  gear::GearParametersImpl* gearParameters = new gear::GearParametersImpl;
  
#ifdef LCIO_MODE
  gearParameters -> setDoubleVals("LadderGaps" , ladder_gapVec ) ;
  gearParameters -> setDoubleVals("StripLineFinalZ" , StripLineFinalZ_Vec  ) ;
#endif
  gearParameters -> setDoubleVal( "ElectronicEndThickness" , electronics_structure_thickness ) ;
  gearParameters -> setDoubleVal( "ElectronicEndLength" ,electronic_end_length); 
  gearParameters -> setDoubleVal( "StripLineThickness" , strip_lines_thickness); 
  gearParameters -> setDoubleVal( "StripLineBeamPipeRadius" ,strip_final_beampipe_radious  ); 
  gearParameters -> setDoubleVal( "VXDEndPlateInnerRadius" , support_endplate_inner_radious ); 
  gearParameters -> setDoubleVal( "CryostatAlRadius" ,rAlu  ); 
  gearParameters -> setDoubleVal( "CryostatAlThickness" , drAlu ); 
  gearParameters -> setDoubleVal( "CryostatAlInnerR" , rInner ); 
  gearParameters -> setDoubleVal( "CryostatAlZEndCap" ,aluEndcapZ ); 
  gearParameters -> setDoubleVal( "CryostatAlHalfZ" ,aluHalfZ ); 
  gearParameters -> setDoubleVal( "Cryostat_RadLen" ,alu_RadLen );
  gearParameters -> setDoubleVal( "Cryostat_dEdx" ,Cryostat_dEdx );
  gearParameters -> setDoubleVal( "StripLineProperties_RadLen" ,StripLine_RadLen );
  gearParameters -> setDoubleVal( "ActiveLayerProperties_dEdx" ,ActiveLayer_dEdx );
  gearParameters -> setDoubleVal( "SupportLayerProperties_dEdx" ,SupportLayer_dEdx );
  gearParameters -> setDoubleVal( "StripLineProperties_dEdx" ,StripLine_dEdx );


  // Write gearParameters to GearMgr
  // Parameters for SIT
  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setGearParameters("VXDInfra", gearParameters);
     
 
}
#endif


