// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: VXD04.cc,v 1.7 2008/11/13 08:25:41 steve Exp $
// $Name: mokka-07-00 $
//
// History:
// - first implementation -- Damien Grandjean, April 2003
// - fixed geometry overlap -- Adrian Vogel, 2005-12-12
// - added optional GEAR output -- R. Lippe, DESY, 2006-09-04
// -modification for double layer geometry -- Damien Grandjean, February 2008
// -increased realism in the description of the ladders, the Be support and the cabling, added cooling tubes Y. Voutsinas, September 2011

#include "Control.hh"
#include "G4PVPlacement.hh"
#include "VXD04.hh"
#include "TRKSiSD00.hh"
#include "CGAGeometryManager.hh"

#include "globals.hh"

#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Tubs.hh"
#include "G4Torus.hh"
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
#include "gearimpl/ZPlanarLayerLayoutImpl.h"
#include "gearimpl/GearParametersImpl.h"
#include "MokkaGear.h"
#include "G4EmCalculator.hh"
#include "G4ParticleTable.hh"

#endif

#include <UTIL/BitField64.h>
#include <UTIL/ILDConf.h>


INSTANTIATE(VXD04)

VXD04::~VXD04()
{
  //delete VXDSupportMaterial;
}

G4bool VXD04::construct(const G4String &dbName, G4LogicalVolume *worldLog)
{
  //  G4VisAttributes *VisAtt;
  G4PVPlacement *Phys;

  G4double sPhi = 0 * deg;
  G4double dPhi = 360 * deg;

  db = new Database(dbName.data());
  
  //****************************************
  // Layers
  //****************************************
  //
  // Common layer thickness parameters
  db->exec("select * from layers_common_parameters;");
  db->getTuple();
  G4double  foam_spacer_thickness,active_silicon_thickness,support_structure_radial_thickness, side_band_electronics_width, side_band_electronics_thickness, layer_gap, flex_cable_thickness, cool_pipe_inner_radius, cool_pipe_outer_radius, metal_traces_thickness, external_kapton_thickness, external_metal_thickness;
  G4String flex_cable_material;
  G4String foam_spacer_material;
  G4String cool_pipe_material;
  G4String metal_traces_material;
  G4int side_band_electronics_option , end_ladd_electronics_option, active_side_band_electronics_option;
  
  foam_spacer_thickness =
    db->fetchDouble("foam_spacer_thickness");
  flex_cable_thickness = 
    db->fetchDouble("flex_cable_thickness");
  metal_traces_thickness = 
    db->fetchDouble("metal_traces_thickness");
  electronics_structure_thickness =
    db->fetchDouble("electronics_structure_thickness");
  active_silicon_thickness =
    db->fetchDouble("active_silicon_thickness");
  support_structure_radial_thickness =
    db->fetchDouble("support_structure_radial_thickness");
  end_electronics_half_z=
    db->fetchDouble("end_electronics_half_z");
  strip_final_beampipe_radious =
    db->fetchDouble("strip_final_beampipe_radious");
  side_band_electronics_option=
    db->fetchInt("side_band_electronics_option");
  flex_cable_material=
    db->fetchString("flex_cable_material");
  metal_traces_material=
    db->fetchString("metal_traces_material");
  foam_spacer_material=
    db->fetchString("foam_spacer_material");
  cool_pipe_material=
    db->fetchString("cool_pipe_material");
  end_ladd_electronics_option=
    db->fetchInt("end_ladd_electronics_option"); 
  side_band_electronics_width=
    db->fetchDouble("side_band_electronics_width");
  side_band_electronics_thickness=
    db->fetchDouble("side_band_electronics_thickness");
  active_side_band_electronics_option=     
    db->fetchInt("active_side_band_electronics_option");
  layer_gap=
    db->fetchDouble("layer_gap");
  cool_pipe_outer_radius=
    db->fetchDouble("cool_pipe_outer_radius");
  cool_pipe_inner_radius=
    db->fetchDouble("cool_pipe_inner_radius");
  external_kapton_thickness=
    db->fetchDouble("external_kapton_thickness");
  external_metal_thickness=
    db->fetchDouble("external_metal_thickness");
  //Cryostat parameters

  db->exec("SELECT * FROM cryostat;");
  db->getTuple();
  
  rAlu   = db->fetchDouble("alu_skin_inner_radious") * mm;
  drAlu  = db->fetchDouble("alu_skin_tickness") * mm;
  const G4double rSty   = db->fetchDouble("foam_inner_radious") * mm;
  const G4double drSty  = db->fetchDouble("foam_tickness") * mm;
  const G4double dzSty  = db->fetchDouble("foam_half_z") * mm;
  const G4double cryostat_apperture  = db->fetchDouble("cryostat_apperture") * mm;
  const G4double cryostat_apperture_radius  = db->fetchDouble("cryostat_apperture_radius") * mm;
  rInner = db->fetchDouble("endplate_inner_radious") * mm;
  useCryo  = G4bool(db->fetchInt("cryostat_option"));

  // support shell parameters
  db->exec("select * from support_shell;");
  db->getTuple();
  G4double  shell_inner_radious,shell_half_z,
    shell_thickess,
    support_endplate_inner_radious_L1,support_endplate_outer_radious_L1,
    offset_ladder_block, beryllium_ladder_block_length , beryllium_ladder_block_length2,
    beryllium_ladder_block_thickness, forward_shell_half_z ;
  shell_inner_radious = db->fetchDouble("inner_radious");
  shell_half_z = db->fetchDouble("half_z");
  shell_thickess = db->fetchDouble("thickess");
  support_endplate_inner_radious = db->fetchDouble("endplate_inner_radious");
  support_endplate_inner_radious_L1 = db->fetchDouble("endplate_inner_radius_L1");
  support_endplate_outer_radious_L1 = db->fetchDouble("endplate_outer_radius_L1");
  offset_ladder_block = db->fetchDouble("offset_ladder_block");
  beryllium_ladder_block_length = db->fetchDouble("beryllium_ladder_block_length");
  beryllium_ladder_block_thickness = db->fetchDouble("beryllium_ladder_block_thickness");
  beryllium_ladder_block_length2=0.;
  shell_endplate_thickness = db->fetchDouble("shell_endplate_thickness");
  forward_shell_half_z = db->fetchDouble("forward_shell_half_z");

  // The VXD Sensitive detector
  // Threshold is 20% of a MIP. For Si we have
  // 340 KeV/mm as MIP.
  theVXDSD =
    new TRKSiSD00("VXD",
		  active_silicon_thickness * mm
		  * 340 * keV
		  * 0.2);
  RegisterSensitiveDetector(theVXDSD);

  // setup the encoder 
  UTIL::BitField64 encoder( ILDCellID0::encoder_string ) ; 
  
  encoder.reset() ;  // reset to 0
  
  encoder[ILDCellID0::subdet] = ILDDetID::VXD ;
  encoder[ILDCellID0::side] = 0 ;
  encoder[ILDCellID0::layer]  = 0 ;
  encoder[ILDCellID0::module] = 0 ;
  encoder[ILDCellID0::sensor] = 0 ;
  int cellID0 = encoder.lowWord() ;

  activeMaterial =CGAGeometryManager::GetMaterial("silicon_2.33gccm"); 
  
#ifdef MOKKA_GEAR
  // some variables for storing information for MOKKA_GEAR
  // during the loop
  std::vector<helpLayer> gearHelpLadders ;
  std::vector<helpLayer> gearHelpSensitives ;
  std::vector<int> gearHelpNumberLadders ;
  std::vector<double> gearHelpPhi0 ;
  G4double gearHelpGap = 0. ;
  G4int gearHelpCount = 0 ;
  G4int gearHelpType = 0 ;
#endif
  
  db->exec("select * from layer;");
  db->getTuple();
  do
    {
      G4int LayerId;
      G4double Z;
      G4double layer_radius, ladder_length, ladder_width, support_width, ladder_gap,strip_line_final_z, nb_ladder, phirot, phirot2, initial_kapton_striplines_thickness, final_kapton_striplines_thickness, initial_metal_striplines_thickness, final_metal_striplines_thickness;
      //	end_electronics_width;
      LayerId = db->fetchInt("id");
      layer_radius = db->fetchDouble("layer_radius");
      ladder_length  = db->fetchDouble("ladder_length");
      ladder_width = db->fetchDouble("ladder_width");
      support_width = db->fetchDouble("support_width");
      ladder_gap = db->fetchDouble("ladder_gap");
      strip_line_final_z = db->fetchDouble("strip_line_final_z");
      initial_kapton_striplines_thickness = db->fetchDouble("initial_kapton_striplines_thickness");
      final_kapton_striplines_thickness = db->fetchDouble("final_kapton_striplines_thickness");
      initial_metal_striplines_thickness = db->fetchDouble("initial_metal_striplines_thickness");
      final_metal_striplines_thickness = db->fetchDouble("final_metal_striplines_thickness");
#ifdef LCIO_MODE
      ladder_gapVec.push_back(ladder_gap);
      StripLineFinalZ_Vec.push_back(strip_line_final_z);
#endif
      nb_ladder = db->fetchDouble("nb_ladder");
      //      end_electronics_width= db->fetchDouble("end_electronics_width");

      phirot = 0.;
      phirot2 = 0.;
      
      //replacing support ladder with flex cable (kapton+metal) & adding a foam spacer


      // ****************************************************************************************
      // **********************   flex  cable *****************************************
      // ****************************************************************************************
      
      flexCableMaterial = CGAGeometryManager::GetMaterial(flex_cable_material);
      
      G4Box *FlexCableSolid
	= new G4Box("FlexCable",
		     ladder_width+(side_band_electronics_option*side_band_electronics_width/2.),
		    ladder_length+(end_ladd_electronics_option*(2*end_electronics_half_z)) + beryllium_ladder_block_length*2.,
		    flex_cable_thickness/2.);
      
      G4VisAttributes* flex_cableVisAtt = new G4VisAttributes(G4Colour(1.,0.,0.,1.0));   //red
      //VisAtt->SetForceWireframe(false);
      //SJA: ladder_supportVisAtt->SetForceSolid(true);
      //VisAtt->SetDaughtersInvisible(true);
      
      
      G4LogicalVolume *FlexCableLogical=
	new G4LogicalVolume(FlexCableSolid,
			    flexCableMaterial,
			    "FlexCable",
			    0,
			    0,
			    0);
      
      FlexCableLogical->SetVisAttributes(flex_cableVisAtt);
      

      // ****************************************************************************************
      // **********************   metal traces  *****************************************
      // ****************************************************************************************


      metalTracesMaterial = CGAGeometryManager::GetMaterial(metal_traces_material);

      G4Box *MetalTracesSolid
	= new G4Box("MetalTraces",
		     ladder_width+(side_band_electronics_option*side_band_electronics_width/2.),
		    ladder_length+(end_ladd_electronics_option*(2*end_electronics_half_z)) + beryllium_ladder_block_length*2.,
		    metal_traces_thickness/2.);
      
      G4VisAttributes* metal_tracesVisAtt = new G4VisAttributes(G4Colour(0.5,0.5,0.5));   //gray
      //VisAtt->SetForceWireframe(false);
      //SJA: ladder_supportVisAtt->SetForceSolid(true);
      //VisAtt->SetDaughtersInvisible(true);
      
      
      G4LogicalVolume *MetalTracesLogical=
	new G4LogicalVolume(MetalTracesSolid,
			    metalTracesMaterial,
			    "MetalTraces",
			    0,
			    0,
			    0);
      
      MetalTracesLogical->SetVisAttributes(metal_tracesVisAtt);

      // ****************************************************************************************
      // **********************   foam spacer n support  *****************************************
      // ****************************************************************************************

      foamSpacerMaterial = CGAGeometryManager::GetMaterial(foam_spacer_material);

      G4Box *FoamSpacerSolid
	= new G4Box("FoamSpacer",
		     support_width+(side_band_electronics_option*side_band_electronics_width/2.),
		    ladder_length+(end_ladd_electronics_option*(2*end_electronics_half_z)) + beryllium_ladder_block_length*2. ,
		    foam_spacer_thickness/2.);
      
      G4VisAttributes* foam_spacerVisAtt = new G4VisAttributes(G4Colour(1.,1.,0.));  //yellow
      
      G4LogicalVolume *FoamSpacerLogical=
	new G4LogicalVolume(FoamSpacerSolid,
			    foamSpacerMaterial,
			    "FoamSpacer",
			    0,
			    0,
			    0);
      
      FoamSpacerLogical->SetVisAttributes(foam_spacerVisAtt);

      //here we place the physical volumes of both the flex cable (kapton & metal traces) and the foam spacer
      
      phirot = (2*pi)/nb_ladder;
      
      G4double ladder_clothest_approch = beryllium_ladder_block_thickness*2 +0.1;

      // calculate optimal offset, such that there is 0.1mm space between to the edge and the surface of two adjacent ladders.
      // in the case of ladders overlapped per superlayer
      /*
      G4double offset_phi=(1-cos(phirot))/sin(phirot)*layer_radius  
	-((ladder_width+(side_band_electronics_option*side_band_electronics_width/2.))
	  +(ladder_clothest_approch+cos(phirot)*2*(foam_spacer_thickness+active_silicon_thickness+flex_cable_thickness+metal_traces_thickness))/sin(phirot));
      */
      
      // in the case of ladders overlapped per layer
      G4double offset_phi=(1-cos(phirot))/sin(phirot)*layer_radius  
	-((ladder_width+(side_band_electronics_option*side_band_electronics_width/2.))
	  +(ladder_clothest_approch+cos(phirot)*2*(active_silicon_thickness+flex_cable_thickness+metal_traces_thickness))/sin(phirot));

      if (LayerId==1||LayerId==3||LayerId==5) 
	{
	  
	  for (G4double ladder_loop=0;ladder_loop<nb_ladder;ladder_loop++) {
	    
	    G4double phirot2 = ladder_loop*phirot;
	    G4RotationMatrix *rot = new G4RotationMatrix();
	    rot->rotateX(pi*0.5);
	    rot->rotateY(phirot2);
	    
	    
	    Phys=
	      new G4PVPlacement(rot,
				G4ThreeVector((layer_radius + metal_traces_thickness + (flex_cable_thickness/2.))*sin(phirot2)+offset_phi*cos(phirot2),
					      -(layer_radius + metal_traces_thickness + (flex_cable_thickness/2.))*cos(phirot2)+offset_phi*sin(phirot2),
					      0.),
				FlexCableLogical,
				"FlexCable",
				worldLog,
				false,
				0);
	       
	    Phys=
	      new G4PVPlacement(rot,
				G4ThreeVector((layer_radius + flex_cable_thickness + metal_traces_thickness + foam_spacer_thickness/2.)*sin(phirot2)+offset_phi*cos(phirot2),
					      -(layer_radius + flex_cable_thickness + metal_traces_thickness +  foam_spacer_thickness/2.)*cos(phirot2)+offset_phi*sin(phirot2),
					      0.),
				FoamSpacerLogical,
				"FoamSpacer",
				worldLog,
				false,
				0);
	    

	    Phys=
	      new G4PVPlacement(rot,
				G4ThreeVector((layer_radius + (metal_traces_thickness/2))*sin(phirot2)+offset_phi*cos(phirot2),
					      -(layer_radius + (metal_traces_thickness/2.))*cos(phirot2)+offset_phi*sin(phirot2),
					      0.),
				MetalTracesLogical,
				"MetalTraces",
				worldLog,
				false,
				0); 
	    
	  }
	  
	}	
      
          
      if (LayerId==2||LayerId==4||LayerId==6) 
	{
	  
	  for (G4double ladder_loop=0;ladder_loop<nb_ladder;ladder_loop++) {
	    
	    G4double phirot2 = ladder_loop*phirot;
	    G4RotationMatrix *rot = new G4RotationMatrix();
	    rot->rotateX(pi*0.5);
	    rot->rotateY(phirot2);

	    
	    Phys=
	      new G4PVPlacement(rot,
				G4ThreeVector((layer_radius-(metal_traces_thickness + flex_cable_thickness/2.)+layer_gap)*sin(phirot2)+offset_phi*cos(phirot2),
					      -(layer_radius-(metal_traces_thickness + flex_cable_thickness/2.)+layer_gap)*cos(phirot2)+offset_phi*sin(phirot2),
					      0.),
				FlexCableLogical,
				"FlexCable",
				worldLog,
				false,
				0);
	    
	    Phys=
	      new G4PVPlacement(rot,
				G4ThreeVector((layer_radius + layer_gap - flex_cable_thickness -  metal_traces_thickness - foam_spacer_thickness/2.)*sin(phirot2)+offset_phi*cos(phirot2),
					      -(layer_radius + layer_gap - flex_cable_thickness - metal_traces_thickness - foam_spacer_thickness/2.)*cos(phirot2)+offset_phi*sin(phirot2),
					      0.),
				FoamSpacerLogical,
				"FoamSpacer",
				worldLog,
				false,
				0);
	    
	    Phys=
	      new G4PVPlacement(rot,
				G4ThreeVector((layer_radius-(metal_traces_thickness/2)+layer_gap)*sin(phirot2)+offset_phi*cos(phirot2),
					      -(layer_radius-(metal_traces_thickness/2.)+layer_gap)*cos(phirot2)+offset_phi*sin(phirot2),
					      0.),
				MetalTracesLogical,
				"MetalTraces",
				worldLog,
				false,
				0);
	    
	  }
	}

      

#ifdef MOKKA_GEAR

      //Definition of the VXDSupport composite material. It is going to be used only during the reconstruction stage, for tracking purposes. It consists by three layers: metal traces, flex cable and the foam spacer support with user defined materials and thicknesses. Here we define the element and calculate its effective radiation length, atomic number and atomic mass. For the simulation, the more realistic 3 different layers structure is being used.   

      G4double MetalDensity = metalTracesMaterial->GetDensity()/(g/mm3);
      G4double KaptonDensity = flexCableMaterial->GetDensity()/(g/mm3);
      G4double FoamDensity = foamSpacerMaterial->GetDensity()/(g/mm3);

      G4double VXDSupportThickness = metal_traces_thickness + flex_cable_thickness + foam_spacer_thickness;

      //calculations of thickness fractions of each layer of the support
      metalTF = metal_traces_thickness / VXDSupportThickness;
      foamTF = foam_spacer_thickness / VXDSupportThickness;
      flexTF = flex_cable_thickness / VXDSupportThickness;

      G4double elemVol = 1/(mm2);

      G4double VXDSupportMass = foam_spacer_thickness*(elemVol)*FoamDensity + flex_cable_thickness*(elemVol)*KaptonDensity + metal_traces_thickness*(elemVol)*MetalDensity;

      G4double VXDSupportDensity = VXDSupportMass/1/(mm3) ;

      G4double foamFM = 100. * ((foam_spacer_thickness*(elemVol)*FoamDensity) / VXDSupportMass) ;
      G4double kaptonFM = 100. * ((flex_cable_thickness*(elemVol)*KaptonDensity) / VXDSupportMass) ;
      G4double metalFM = 100. * ((metal_traces_thickness*(elemVol)*MetalDensity) / VXDSupportMass) ;

      //Calculation of an effective radiation length for the support based on the mass fraction of each material

      G4double VXDSupportRadLen = 1. / ((metalTF/metalTracesMaterial->GetRadlen()) + (flexTF/flexCableMaterial->GetRadlen()) + (foamTF/foamSpacerMaterial->GetRadlen()));

      //Calculation of the effective atomic number of the VXD support. The Z effectives are obtained from the formula: Zeff = Sum(Wi*Zi) where Wi are the mass fractions of the elements that consist the material 

      G4Material *carbon = CGAGeometryManager::GetMaterial("carbon");
      G4Material *silicon = CGAGeometryManager::GetMaterial("silicon");
      G4Material *hydrogen = CGAGeometryManager::GetMaterial("H2");
      G4Material *nitro = CGAGeometryManager::GetMaterial("N2");
      G4Material *oxygen = CGAGeometryManager::GetMaterial("oxygen");

      G4double C_Z = carbon->GetZ();
      G4double Si_Z = silicon->GetZ();
      G4double C_A = carbon->GetA()/g;
      G4double Si_A = silicon->GetA()/g;
      G4double H_Z = hydrogen->GetZ();
      G4double H_A = hydrogen->GetA()/g;
      G4double N_Z = nitro->GetZ();
      G4double N_A = nitro->GetA()/g;
      G4double O_Z = oxygen->GetZ();
      G4double O_A = oxygen->GetA()/g;


      G4double foamZeff = C_Z*(C_A/(C_A+Si_A)) + Si_Z*(Si_A/(C_A+Si_A));


      G4double metalZ = metalTracesMaterial->GetZ();
      G4double metalA = metalTracesMaterial->GetA()/g;
      
      //Calculation of kapton effective Z - weight fractions for each element taken from NIST dB

      G4double flexZeff = H_Z*0.026362 + C_Z*0.691133 + N_Z*0.073270 + O_Z*0.209235;

      G4double VXDSupportZeff = (metalFM/100.)*metalZ + (kaptonFM/100.)*flexZeff + (foamFM/100.)*foamZeff;


      //Calculation of the effective atomic mass of the VXD support. The Z effectives are obtained from the formula: Aeff = Zeff / (Z/A)eff where (Z/A)eff = Sum Wi*Zi/Ai

      G4double metalZA = metalZ/metalA;
      G4double foamZAeff = (C_A/(C_A+Si_A))*(C_Z/C_A) + (Si_A/(C_A+Si_A))*(Si_Z/Si_A);
      G4double flexZAeff = (H_Z/H_A)*0.026362 + (C_Z/C_A)*0.691133 + (N_Z/N_A)*0.073270 + (O_Z/O_A)*0.209235;

      G4double VXDSupportZAeff = (metalFM/100.)*metalZA + (kaptonFM/100.)*flexZAeff + (foamFM/100.)*foamZAeff;

      G4double VXDSupportAeff = VXDSupportZeff / VXDSupportZAeff;

      //Calculation of the effective nuclear interaction length of the VXD support

      G4double VXDSupportIntLength = 1. / ((metalTF/metalTracesMaterial->GetNuclearInterLength()) + (flexTF/flexCableMaterial->GetNuclearInterLength()) + (foamTF/foamSpacerMaterial->GetNuclearInterLength()));

      //Here we call the SimpleMaterial class of gear. The density should be converted to kg/m3
      VXDSupportDensity = 1000000*VXDSupportDensity;

      VXDSupportMaterial = new gear::SimpleMaterialImpl("VXDSupportMaterial", VXDSupportAeff, VXDSupportZeff, VXDSupportDensity, VXDSupportRadLen, VXDSupportIntLength );

      //_________________________________________________________________________________________________________
      //

      helpLayer thisLadder ;
      if (LayerId==2||LayerId==4||LayerId==6) 
	{ 
	  thisLadder.distance  = layer_radius + layer_gap * 0.5 ;
	}
      if (LayerId==1||LayerId==3||LayerId==5) 
	{ 
	  thisLadder.distance  = layer_radius  ;
	}      
      //      thisLadder.distance  = layer_radius ;
      thisLadder.offset    = offset_phi ;
      thisLadder.thickness = VXDSupportThickness ;
      thisLadder.length    = ladder_length ;
      thisLadder.width     = (ladder_width*2.)+(side_band_electronics_option*side_band_electronics_width) ;
      thisLadder.radLength = VXDSupportMaterial->getRadLength()/mm ;

 
      // find out type
      if( side_band_electronics_option == 0 &&  end_ladd_electronics_option == 1) gearHelpType = gear::ZPlanarParametersImpl::CCD  ;
      if( side_band_electronics_option == 1 &&  end_ladd_electronics_option == 0 ) gearHelpType = gear::ZPlanarParametersImpl::CMOS ;
      if( side_band_electronics_option == 1 &&  end_ladd_electronics_option == 1) gearHelpType = gear::ZPlanarParametersImpl::HYBRID ;

#endif

      
      // ****************************************************************************************
      // **********************   Berylium annulus block *****************************************
      // ****************************************************************************************

      //only one block per superlayer

      if (LayerId==2)
	{
	  G4Box *BerylliumAnnulusBlockSolid
	    = new G4Box("BerylliumAnnulusBlock",
			ladder_width,
			beryllium_ladder_block_length,
			beryllium_ladder_block_thickness);
	  
	  G4VisAttributes* beryllium_ladder_blockVisAtt = new G4VisAttributes(G4Colour(0.,0.,1.,1.));
	  //beryllium_ladder_blockVisAtt->SetForceWireframe(false);
	  //SJA: beryllium_ladder_blockVisAtt->SetForceSolid(true);
	  //beryllium_ladder_blockVisAtt->SetDaughtersInvisible(true);
	  
	  
	  G4LogicalVolume *BerylliumAnnulusBlockLogical=
	    new G4LogicalVolume(BerylliumAnnulusBlockSolid,
				CGAGeometryManager::GetMaterial("beryllium"),
				"BerylliumAnnulusBlock",
				0,
				0,
				0);
	  BerylliumAnnulusBlockLogical->SetVisAttributes(beryllium_ladder_blockVisAtt);
	  
	  
	  for (G4double AnnulusBlock_loop=0;AnnulusBlock_loop<nb_ladder;AnnulusBlock_loop++) {
	    
	    phirot2 = phirot*AnnulusBlock_loop;
	    G4RotationMatrix *rot = new G4RotationMatrix();
	    rot->rotateX(pi*0.5);
	    rot->rotateY(phirot2);

	    G4double ZAnnulusBlock;
	    ZAnnulusBlock= ladder_length + end_electronics_half_z + (beryllium_ladder_block_length*2.);
	    
	    Phys=
	      new G4PVPlacement(rot,
				G4ThreeVector((layer_radius+beryllium_ladder_block_thickness+layer_gap)*sin(phirot2)+offset_phi*cos(phirot2),
					      -(layer_radius+beryllium_ladder_block_thickness+layer_gap)*cos(phirot2)+offset_phi*sin(phirot2),
					      ZAnnulusBlock),
				BerylliumAnnulusBlockLogical,
				"BerylliumAnnulusBlock",
				worldLog,
				false,
				0);
	    
	    
	    Phys=
	      new G4PVPlacement(rot,
				G4ThreeVector((layer_radius+beryllium_ladder_block_thickness+layer_gap)*sin(phirot2)+offset_phi*cos(phirot2),
					      -(layer_radius+beryllium_ladder_block_thickness+layer_gap)*cos(phirot2)+offset_phi*sin(phirot2),
					      -ZAnnulusBlock),
				BerylliumAnnulusBlockLogical,
				"BerylliumAnnulusBlock",
				worldLog,
				false,
				0);
	  }	
	}
      

      if (LayerId==4||LayerId==6) 
	{ 
	  beryllium_ladder_block_length2 = beryllium_ladder_block_length + (shell_half_z - (end_electronics_half_z *3.* end_ladd_electronics_option)-ladder_length);
	  
	  for (G4double AnnulusBlock_loop=0;AnnulusBlock_loop<nb_ladder;AnnulusBlock_loop++) {
	    
	    G4Box *BerylliumAnnulusBlockSolid
	      = new G4Box("BerylliumAnnulusBlock",
			  ladder_width,
			  beryllium_ladder_block_length2/2.,
			  beryllium_ladder_block_thickness);
	    
	    G4VisAttributes* beryllium_ladder_blockVisAtt = new G4VisAttributes(G4Colour(0.,0.,1.,1.));
	    
	    //VisAtt->SetForceWireframe(false);
	    //SJA: beryllium_ladder_blockVisAtt->SetForceSolid(true);
	    //VisAtt->SetDaughtersInvisible(true);
	    
	    
	    G4LogicalVolume *BerylliumAnnulusBlockLogical=
	      new G4LogicalVolume(BerylliumAnnulusBlockSolid,
				  CGAGeometryManager::GetMaterial("beryllium"),
				  "BerylliumAnnulusBlock",
				  0,
				  0,
				  0);
	    
	    
	    BerylliumAnnulusBlockLogical->SetVisAttributes(beryllium_ladder_blockVisAtt);
	    phirot2 = phirot*AnnulusBlock_loop;
	    G4RotationMatrix *rot = new G4RotationMatrix();
	    rot->rotateX(pi*0.5);
	    rot->rotateY(phirot2);
	    
	    G4double ZAnnulusBlock2;
	    ZAnnulusBlock2=shell_half_z -(beryllium_ladder_block_length2/2.);// - (shell_thickess/2.); 
	    
	    Phys=
	      new G4PVPlacement(rot,
				G4ThreeVector((layer_radius+beryllium_ladder_block_thickness+layer_gap)*sin(phirot2)+offset_phi*cos(phirot2),
					      -(layer_radius+beryllium_ladder_block_thickness+layer_gap)*cos(phirot2)+offset_phi*sin(phirot2),
					      ZAnnulusBlock2),
				BerylliumAnnulusBlockLogical,
				"BerylliumAnnulusBlock",
				worldLog,
				false,
				0);
	    
	    
	    Phys=
	      new G4PVPlacement(rot,
				G4ThreeVector((layer_radius+beryllium_ladder_block_thickness+layer_gap)*sin(phirot2)+offset_phi*cos(phirot2),
					      -(layer_radius+beryllium_ladder_block_thickness+layer_gap)*cos(phirot2)+offset_phi*sin(phirot2),
					      -ZAnnulusBlock2),
				BerylliumAnnulusBlockLogical,
				"BerylliumAnnulusBlock",
				worldLog,
				false,
				0);
	    
	    
	  }
	}
      
      //****************************************************************************************
      // *********************************  Electronics   **********************************
      // ******************************  (dead Si layer ends)   ********************************
      //****************************************************************************************
      
      
      // *********************************  Electronics at the end of the ladder  **********************************
      
      if(end_ladd_electronics_option==1){
	
	G4Box *ElectronicsEndSolid
	  = new G4Box("ElectronicsEnd",
		      ladder_width,
		      end_electronics_half_z,
		      electronics_structure_thickness/2.);
	
	G4VisAttributes* ElectronicsEndVisAtt = new G4VisAttributes(G4Colour(0.,1.,0.));
	//VisAtt->SetForceWireframe(false);
	// VisAtt->SetForceSolid(true);
	//VisAtt->SetDaughtersInvisible(true);

	G4LogicalVolume *ElectronicsEndLogical=
	  new G4LogicalVolume(ElectronicsEndSolid,
			    CGAGeometryManager::GetMaterial("silicon_2.33gccm"),
			      "ElectronicsEnd",
			      0,
			      0,
			      0);
	ElectronicsEndLogical->SetVisAttributes(ElectronicsEndVisAtt);
	
	
	G4double end_ladd_electronic_offset_phi = offset_phi +(side_band_electronics_option * side_band_electronics_width/2.);
	
	if (LayerId==2||LayerId==4||LayerId==6) 
	  {       
	    
	    
	    for (G4double elec_loop=0; elec_loop<nb_ladder;elec_loop++) {
	      
	      phirot2 = phirot*elec_loop;
	      G4RotationMatrix *rot = new G4RotationMatrix();
	      rot->rotateX(pi*0.5);
	      rot->rotateY(phirot2);
	      
	      
	      Z= ladder_length +end_electronics_half_z + (ladder_gap/2.);
	      
	      Phys=
		new G4PVPlacement(rot,
				  G4ThreeVector((layer_radius+(electronics_structure_thickness/2.)+layer_gap)*sin(phirot2)+ end_ladd_electronic_offset_phi*cos(phirot2),
						-(layer_radius+(electronics_structure_thickness/2.)+layer_gap)*cos(phirot2)+ end_ladd_electronic_offset_phi*sin(phirot2),
						Z),
				  ElectronicsEndLogical,
				  "ElectronicsEnd",
				  worldLog,
				  false,
				  0);
	      Phys=
		new G4PVPlacement(rot,
				  G4ThreeVector((layer_radius+(electronics_structure_thickness/2.)+layer_gap)*sin(phirot2)+ end_ladd_electronic_offset_phi*cos(phirot2),
						-(layer_radius+(electronics_structure_thickness/2.)+layer_gap)*cos(phirot2)+ end_ladd_electronic_offset_phi*sin(phirot2),
						-Z),
			  ElectronicsEndLogical,
				  "ElectronicsEnd",
				  worldLog,
				  false,
				  0);
	    }
	    
	  }
	
	if (LayerId==1||LayerId==3||LayerId==5) 
	  {       

	    
	    for (G4double elec_loop=0; elec_loop<nb_ladder;elec_loop++) {
	      
	      phirot2 = phirot*elec_loop;
	      G4RotationMatrix *rot = new G4RotationMatrix();
	      rot->rotateX(pi*0.5);
	      rot->rotateY(phirot2);
	      
	      
	      Z= ladder_length +end_electronics_half_z + (ladder_gap/2.);

	      Phys=
		new G4PVPlacement(rot,
				  G4ThreeVector((layer_radius-(electronics_structure_thickness/2.))*sin(phirot2)+ end_ladd_electronic_offset_phi*cos(phirot2),
			               -(layer_radius-(electronics_structure_thickness/2.))*cos(phirot2)+ end_ladd_electronic_offset_phi*sin(phirot2),
						Z),
				  ElectronicsEndLogical,
				  "ElectronicsEnd",
				  worldLog,
				  false,
				  0);
	      Phys=
		new G4PVPlacement(rot,
				  G4ThreeVector((layer_radius-(electronics_structure_thickness/2.))*sin(phirot2)+ end_ladd_electronic_offset_phi*cos(phirot2),
						-(layer_radius-(electronics_structure_thickness/2.))*cos(phirot2)+ end_ladd_electronic_offset_phi*sin(phirot2),
						-Z),
				  ElectronicsEndLogical,
				  "ElectronicsEnd",
				  worldLog,
				  false,
				  0);
	    }
	    
	  }
	
      }
      // *********************************  Electronics a long  the ladder  **********************************
      
      if(side_band_electronics_option==1){
	
	G4Box *ElectronicsBandSolid
	  = new G4Box("ElectronicsBand",
		      side_band_electronics_width/2.,
		      ladder_length/2.,
		      side_band_electronics_thickness/2.);

	G4VisAttributes* ElectronicsEndVisAtt = new G4VisAttributes(G4Colour(0.,1.,0.));
	//VisAtt->SetForceWireframe(false);
	// VisAtt->SetForceSolid(true);
	//VisAtt->SetDaughtersInvisible(true);
	
	G4LogicalVolume *ElectronicsBandLogical=
	  new G4LogicalVolume(ElectronicsBandSolid,
			      activeMaterial,
			      "ElectronicsBand",
			      0,
			      0,
			      0);
	ElectronicsBandLogical->SetVisAttributes(ElectronicsEndVisAtt);
	if (active_side_band_electronics_option==1)  ElectronicsBandLogical->SetSensitiveDetector(theVXDSD);
	

	G4double side_band_electronic_offset_phi = offset_phi - (side_band_electronics_option * ladder_width);

  

  if (LayerId==2||LayerId==4||LayerId==6) 
	  {       
	    
	    for (G4double elec_loop=0; elec_loop<nb_ladder;elec_loop++) {   
	      
	      phirot2 = phirot*elec_loop;
	      G4RotationMatrix *rot = new G4RotationMatrix();
	      rot->rotateX(pi*0.5);
	      rot->rotateY(phirot2);
        
        
	      
	      Z= (ladder_length* (1-side_band_electronics_option/2.)) + ladder_gap/2.;
	      
        encoder[ILDCellID0::layer]  =  LayerId -1;
        encoder[ILDCellID0::module] = elec_loop ;
        cellID0 = encoder.lowWord() ;  
        
	      Phys=
		new G4PVPlacement(rot,
				  G4ThreeVector((layer_radius+(side_band_electronics_thickness/2.)+layer_gap)*sin(phirot2)+side_band_electronic_offset_phi*cos(phirot2),
			               -(layer_radius+(side_band_electronics_thickness/2.)+layer_gap)*cos(phirot2)+side_band_electronic_offset_phi*sin(phirot2),
						Z),
				  ElectronicsBandLogical,
				  "ElectronicsBand",
				  worldLog,
				  false,
				  cellID0);
        
	      Phys=
		new G4PVPlacement(rot,
				  G4ThreeVector((layer_radius+(side_band_electronics_thickness/2.)+layer_gap)*sin(phirot2)+side_band_electronic_offset_phi*cos(phirot2),
						-(layer_radius+(side_band_electronics_thickness/2.)+layer_gap)*cos(phirot2)+side_band_electronic_offset_phi*sin(phirot2),
						-Z),
				  ElectronicsBandLogical,
				  "ElectronicsBand",
				  worldLog,
				  false,
				  cellID0);
	      
	    }
	  }
	
	if (LayerId==1||LayerId==3||LayerId==5) 
	  {       
	    
	    for (G4double elec_loop=0; elec_loop<nb_ladder;elec_loop++) { 
	      
	      phirot2 = phirot*elec_loop;
	      G4RotationMatrix *rot = new G4RotationMatrix();
	      rot->rotateX(pi*0.5);
	      rot->rotateY(phirot2);
	      
	      
	      Z= (ladder_length* (1-side_band_electronics_option/2.)) + ladder_gap/2.;
	      
        encoder[ILDCellID0::layer]  =  LayerId -1;
        encoder[ILDCellID0::module] = elec_loop ;
        cellID0 = encoder.lowWord() ;  
        
	Phys=
	  new G4PVPlacement(rot,
			    G4ThreeVector((layer_radius-(side_band_electronics_thickness/2.))*sin(phirot2)+side_band_electronic_offset_phi*cos(phirot2),
					  -(layer_radius-(side_band_electronics_thickness/2.))*cos(phirot2)+side_band_electronic_offset_phi*sin(phirot2),
					  Z),
			    ElectronicsBandLogical,
			    "ElectronicsBand",
			    worldLog,
			    false,
			    0);
	Phys=
	  new G4PVPlacement(rot,
			    G4ThreeVector((layer_radius-(side_band_electronics_thickness/2.))*sin(phirot2)+side_band_electronic_offset_phi*cos(phirot2),
					  -(layer_radius-(side_band_electronics_thickness/2.))*cos(phirot2)+side_band_electronic_offset_phi*sin(phirot2),
					  -Z),
			    ElectronicsBandLogical,
			    "ElectronicsBand",
			    worldLog,
			    false,
			    0);
	
	    }
	  }
	
	
      }
      

      //****************************************************************************************
      //*******************************  Strip lines (Kapton + metal)  *************************
      //************ here the strip lines are still simulate by conical geometry ***************
      //************ the thickness varies linearly with z **************************************
      //****************************************************************************************


      G4double strip_line_start_z=0.;
      
      if (LayerId==1||LayerId==2) 
	{
	  strip_line_start_z = ladder_length + ladder_gap/2. +( end_electronics_half_z * 2.)+ shell_thickess + beryllium_ladder_block_length*2 ; // to avoid overlaps
	}
      else
	{
	  strip_line_start_z = shell_half_z + shell_endplate_thickness;//ladder_length + ladder_gap/2. - end_electronics_half_z * 2.+ shell_thickess  ; // to avoid overlaps
	}
      
      //G4double strip_line_half_z = (strip_line_final_z - strip_line_start_z) / 2.;

      G4double strip_line_half_z = (dzSty - strip_line_start_z) / 2.;
      assert (strip_line_half_z>0);


      if (LayerId==1||LayerId==3||LayerId==5) 
	{      

	  //Here we define the solid and logical volumes of the kapton strip lines
	  G4Cons *KaptonLinesSolid
	    = new G4Cons("KaptonLines",
			 layer_radius, // inside radius at  -fDz
			 layer_radius + initial_kapton_striplines_thickness, // outside radius at -fDz
			 cryostat_apperture + LayerId*final_kapton_striplines_thickness, // inside radius at  +fDz
			 cryostat_apperture + LayerId*final_kapton_striplines_thickness + final_kapton_striplines_thickness,
			 strip_line_half_z,
			 sPhi,
			 dPhi);
	  
	  G4VisAttributes* KaptonLinesVisAtt = new G4VisAttributes(G4Colour(1.,1.,1.));  //white
	  KaptonLinesVisAtt->SetForceWireframe(true);
	  //VisAtt->SetForceSolid(true);
	  
	  //stripMaterial = CGAGeometryManager::GetMaterial("kapton"); 
	  //here we use the flex cable material since is kapton for both of them	  

	  G4LogicalVolume *KaptonLinesLogical=
	    new G4LogicalVolume(KaptonLinesSolid,
				flexCableMaterial,
				"KaptonLines",
				0,
				0,
				0);
	  
	  KaptonLinesLogical->SetVisAttributes( KaptonLinesVisAtt);
	  
	  //Here we define the solid and logical volumes of the metal traces of the strip lines
	  G4Cons *MetalLinesSolid
	    = new G4Cons("MetalLines",
			 layer_radius + initial_kapton_striplines_thickness, // inside radius at  -fDz
			 layer_radius + initial_kapton_striplines_thickness + initial_metal_striplines_thickness, // outside radius at -fDz
			 cryostat_apperture + LayerId*final_kapton_striplines_thickness + final_kapton_striplines_thickness, // inside radius at  +fDz
			 cryostat_apperture + LayerId*final_kapton_striplines_thickness + final_kapton_striplines_thickness + final_metal_striplines_thickness,
			 strip_line_half_z,
			 sPhi,
			 dPhi);


	  G4VisAttributes* MetalLinesVisAtt = new G4VisAttributes(G4Colour(0.5,0.5,0.5));  //gray

	  G4LogicalVolume *MetalLinesLogical=
	    new G4LogicalVolume(MetalLinesSolid,
				metalTracesMaterial,
				"MetalLines",
				0,
				0,
				0);
	  
	  MetalLinesLogical->SetVisAttributes( MetalLinesVisAtt);
	  MetalLinesVisAtt->SetForceWireframe(true);

	  //here we place both the kapton and copper part of the strip lines

	  Z = strip_line_start_z + strip_line_half_z;
	  Phys=
	    new G4PVPlacement(0,
			      G4ThreeVector(0., 0., Z),
			      KaptonLinesLogical,
			      "KaptonLines",
			      worldLog,
			      false,0);

	  G4RotationMatrix *rot=new G4RotationMatrix();
	  rot->rotateX(pi); // the same but other side
	  
	  Phys=
	    new G4PVPlacement(rot,
			  G4ThreeVector(0., 0., -Z),
			      KaptonLinesLogical,
			      "KaptonLines",
			      worldLog,
			      false,0);

	  
	  Phys=
	    new G4PVPlacement(0,
			      G4ThreeVector(0., 0., Z),
			      MetalLinesLogical,
			      "MetalLines",
			      worldLog,
			      false,0);
	  
	  Phys=
	    new G4PVPlacement(rot,
			  G4ThreeVector(0., 0., -Z),
			      MetalLinesLogical,
			      "MetalLines",
			      worldLog,
			      false,0);
	  
	}


      //*** Here we place the cabling going outside the VXD ****************************************************

      G4double external_cable_length = (drAlu + drSty)/2.;
      G4double ExternalCablesZ = dzSty + drSty/2. + drAlu/2. ;

      //kapton part
      G4Tubs *ExternalKaptonCablesSolid = 
	new G4Tubs("ExternalKaptonCables",
		   cryostat_apperture,
		   cryostat_apperture + 3*external_kapton_thickness/2.,
		   external_cable_length,
		   sPhi,
		   dPhi);

      //The reason for the factor three is that the thickness refer to the thickness of each single cable, and we have three cables in total, one per layer

      G4VisAttributes* ExternalKaptonCablesVisAtt = new G4VisAttributes(G4Colour(1.,1.,1.));   //white
      ExternalKaptonCablesVisAtt->SetForceWireframe(true);
      
      G4LogicalVolume *ExternalKaptonCablesLogical=
	new G4LogicalVolume(ExternalKaptonCablesSolid,
			    CGAGeometryManager::GetMaterial("kapton"),
			    "ExternalKaptonCables",
			    0,
			    0,
			    0);
      
      ExternalKaptonCablesLogical->SetVisAttributes(ExternalKaptonCablesVisAtt);


      //metal part
      G4Tubs *ExternalMetalCablesSolid = 
	new G4Tubs("ExternalMetalCables",
		   cryostat_apperture - 3*external_metal_thickness/2.,
		   cryostat_apperture,
		   external_cable_length,
		   sPhi,
		   dPhi);


      G4VisAttributes* ExternalMetalCablesVisAtt = new G4VisAttributes(G4Colour(0.5,0.5,0.5));   //gray
      ExternalMetalCablesVisAtt->SetForceWireframe(true);
      
      G4LogicalVolume *ExternalMetalCablesLogical=
	new G4LogicalVolume(ExternalMetalCablesSolid,
			    CGAGeometryManager::GetMaterial("copper"),
			    "ExternalMetalCables",
			    0,
			    0,
			    0);
      
      ExternalMetalCablesLogical->SetVisAttributes(ExternalMetalCablesVisAtt);

      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., ExternalCablesZ),
			  ExternalKaptonCablesLogical,
			  "ExternalKaptonCables",
			  worldLog,
			  false,0);

      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., -ExternalCablesZ),
			  ExternalKaptonCablesLogical,
			  "ExternalKaptonCables",
			  worldLog,
			  false,0);

      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., ExternalCablesZ),
			  ExternalMetalCablesLogical,
			  "ExternalMetalCables",
			  worldLog,
			  false,0);

      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., -ExternalCablesZ),
			  ExternalMetalCablesLogical,
			  "ExternalMetalCables",
			  worldLog,
			  false,0);

      //****************************************************************************************
      //*******************************  Cooling Pipes (Titanium )  ********************************
      //****************************************************************************************      
     
      //endplate cooling pipes

      G4double ZEndPlateCoolPipes;
      ZEndPlateCoolPipes = shell_half_z + shell_endplate_thickness;

      G4double ZEndPlateCoolPipesL1;
      ZEndPlateCoolPipesL1  = ladder_length +  ((end_electronics_half_z*end_ladd_electronics_option) * 2) + shell_thickess + (beryllium_ladder_block_length*2) ;

      G4Torus *CoolPipeSolid = 
	new G4Torus("CoolPipe",
		    cool_pipe_inner_radius,
		    cool_pipe_outer_radius,
		    layer_radius + layer_gap + cool_pipe_outer_radius/2.,
		    sPhi,
		    dPhi);
      
      
      G4VisAttributes* CoolPipeVisAtt = new G4VisAttributes(G4Colour(1.,0.,1.));   //magenta
      //CoolPipeVisAtt->SetForceWireframe(true);
      
      G4LogicalVolume *CoolPipeLogical=
	new G4LogicalVolume(CoolPipeSolid,
			    CGAGeometryManager::GetMaterial("titanium"),
			    "CoolPipe",
			    0,
			    0,
			    0);
      
      CoolPipeLogical->SetVisAttributes(CoolPipeVisAtt);

      //one cooling pipe for each double layer
 
      if (LayerId==4 || LayerId==6) 
	{ 
	  
	  Phys=
	    new G4PVPlacement(0,
			      G4ThreeVector(0., 0., ZEndPlateCoolPipes + cool_pipe_outer_radius),
			      CoolPipeLogical,
			      "CoolPipe",
			      worldLog,
			      false,0);      
      
	  Phys=
	    new G4PVPlacement(0,
			      G4ThreeVector(0., 0., -(ZEndPlateCoolPipes + cool_pipe_outer_radius)),
			      CoolPipeLogical,
			      "CoolPipe",
			      worldLog,
			      false,0);
	}


      if (LayerId==2) 
	{ 
	  
	  Phys=
	    new G4PVPlacement(0,
			      G4ThreeVector(0., 0., ZEndPlateCoolPipesL1 + cool_pipe_outer_radius + shell_thickess),
			      CoolPipeLogical,
			      "CoolPipe",
			      worldLog,
			      false,0);      
      
	  Phys=
	    new G4PVPlacement(0,
			      G4ThreeVector(0., 0., -(ZEndPlateCoolPipesL1 + cool_pipe_outer_radius + shell_thickess)),
			      CoolPipeLogical,
			      "CoolPipe",
			      worldLog,
			      false,0);
	}


      // *** cooling pipe connecting the pipes at the central be support endplate to the layer 1 support endplate  ****


      G4double thetaTube = atan((support_endplate_inner_radious - (layer_radius + layer_gap + 2*cool_pipe_outer_radius)) / (shell_half_z - ZEndPlateCoolPipesL1)) ;
      if (LayerId==2) 
	{ 
	  G4double CoolPipeLength = (shell_half_z - shell_thickess/2.) - ZEndPlateCoolPipesL1;
	

	  G4Tubs *CoolPipeTubeSolid = 
	    new G4Tubs("CoolPipeTube",
		       cool_pipe_inner_radius,
		       cool_pipe_outer_radius,
		       CoolPipeLength/2.,
		       sPhi,
		       dPhi);
	  
	  G4VisAttributes* CoolPipeTubeVisAtt = new G4VisAttributes(G4Colour(1.,0.,1.));   //magenta
	  //CoolPipeTubeVisAtt->SetForceWireframe(false);
	  
	  G4LogicalVolume *CoolPipeTubeLogical = 
	    new G4LogicalVolume(CoolPipeTubeSolid,
				CGAGeometryManager::GetMaterial("titanium"),
				"CoolPipeTube",
				0,
				0,
				0);
	  
	  CoolPipeTubeLogical->SetVisAttributes(CoolPipeTubeVisAtt);
	  
	  G4RotationMatrix * rm = new G4RotationMatrix;
	  rm->rotateX(thetaTube);

	  G4RotationMatrix * rm2 = new G4RotationMatrix;
	  rm2->rotateX(-thetaTube);
	  
	  Phys=
	    new G4PVPlacement(rm,
			      G4ThreeVector(0., 
					    //(layer_radius + layer_gap + cool_pipe_outer_radius/2. + support_endplate_inner_radious)/2., 
					    (layer_radius + layer_gap + support_endplate_inner_radious)/2.,
					    (ZEndPlateCoolPipesL1 + 3*cool_pipe_outer_radius + CoolPipeLength/2.)),
			      CoolPipeTubeLogical,
			      "CoolPipeTube",
			      worldLog,
			      false,0); 

	  Phys=
	    new G4PVPlacement(rm2,
			      G4ThreeVector(0., 
					    //-(layer_radius + layer_gap + cool_pipe_outer_radius/2. + support_endplate_inner_radious)/2., 
					    -(layer_radius + layer_gap + support_endplate_inner_radious)/2.,
					    (ZEndPlateCoolPipesL1 + 3*cool_pipe_outer_radius + CoolPipeLength/2.)),
			      CoolPipeTubeLogical,
			      "CoolPipeTube",
			      worldLog,
			      false,0);

	  Phys=
	    new G4PVPlacement(rm2,
			      G4ThreeVector(0., 
					    //(layer_radius + layer_gap + cool_pipe_outer_radius/2. + support_endplate_inner_radious)/2., 
					    (layer_radius + layer_gap + support_endplate_inner_radious)/2.,
					    -(ZEndPlateCoolPipesL1 + 3*cool_pipe_outer_radius + CoolPipeLength/2.)),
			      CoolPipeTubeLogical,
			      "CoolPipeTube",
			      worldLog,
			      false,0); 

	  Phys=
	    new G4PVPlacement(rm,
			      G4ThreeVector(0., 
					    //-(layer_radius + layer_gap + cool_pipe_outer_radius/2. + support_endplate_inner_radious)/2., 
					    -(layer_radius + layer_gap + support_endplate_inner_radious)/2.,
					    -(ZEndPlateCoolPipesL1 + 3*cool_pipe_outer_radius + CoolPipeLength/2.)),
			      CoolPipeTubeLogical,
			      "CoolPipeTube",
			      worldLog,
			      false,0);
	}

      //****************************************************************************************
      // *******************************  Si Active layer  *************************************
      //****************************************************************************************


      G4Box *SiActiveLayerSolid
	= new G4Box("SiActiveLayer",
		    ladder_width,
		    ladder_length/2.,
		    active_silicon_thickness/2.);
      
      G4VisAttributes* SiActiveLayerVisAtt = new G4VisAttributes(G4Colour(0.,1.,1.,1.));
      //VisAtt->SetForceWireframe(false);
      //SJA: SiActiveLayerVisAtt->SetForceSolid(true);
      //VisAtt->SetDaughtersInvisible(true);
      
      G4LogicalVolume *SiActiveLayerLogical=
	new G4LogicalVolume(SiActiveLayerSolid,
			    activeMaterial,
			    "SiActiveLayer",
			    0,
			    0,
			    0);
      SiActiveLayerLogical->SetVisAttributes(SiActiveLayerVisAtt);
      SiActiveLayerLogical->SetSensitiveDetector(theVXDSD);
      
      
      G4double active_offset_phi = offset_phi +(side_band_electronics_option * side_band_electronics_width/2.); 
      
      for (G4double active_loop=0;active_loop<nb_ladder;active_loop++){
	
	phirot2 =  phirot*active_loop;
	G4RotationMatrix *rot = new G4RotationMatrix();
	rot->rotateX(pi*0.5);
	rot->rotateY(phirot2);
	
	Z= ladder_length/2.+ ladder_gap;
	
  encoder[ILDCellID0::layer]  =  LayerId -1;
  encoder[ILDCellID0::module] = active_loop ;
  cellID0 = encoder.lowWord() ;  

        
	if (LayerId==2||LayerId==4||LayerId==6) 
	  { 
  
	    Phys=
	      new G4PVPlacement(rot,
				G4ThreeVector((layer_radius+(active_silicon_thickness/2.)+layer_gap)*sin(phirot2)+active_offset_phi*cos(phirot2),
					      -(layer_radius+(active_silicon_thickness/2.)+layer_gap)*cos(phirot2)+active_offset_phi*sin(phirot2),
					      Z),
				SiActiveLayerLogical,
				"SiActiveLayer",
				worldLog,
				false,
				cellID0);
	    
	    Phys=
	      new G4PVPlacement(rot,
				G4ThreeVector((layer_radius+(active_silicon_thickness/2.)+layer_gap)*sin(phirot2)+active_offset_phi*cos(phirot2),
					      -(layer_radius+(active_silicon_thickness/2.)+layer_gap)*cos(phirot2)+active_offset_phi*sin(phirot2),
					      -Z),
				SiActiveLayerLogical,
				"SiActiveLayer",
				worldLog,
				false,
				cellID0);
	  }		  
	
	if (LayerId==1||LayerId==3||LayerId==5) 
	  { 
	    Phys=
	      new G4PVPlacement(rot,
				G4ThreeVector((layer_radius-(active_silicon_thickness/2.))*sin(phirot2)+active_offset_phi*cos(phirot2),
                                       -(layer_radius-(active_silicon_thickness/2.))*cos(phirot2)+active_offset_phi*sin(phirot2),
					      Z),
				SiActiveLayerLogical,
				"SiActiveLayer",
				worldLog,
				false,
				cellID0);
	    
	    Phys=
	      new G4PVPlacement(rot,
				G4ThreeVector((layer_radius-(active_silicon_thickness/2.))*sin(phirot2)+active_offset_phi*cos(phirot2),
					      -(layer_radius-(active_silicon_thickness/2.))*cos(phirot2)+active_offset_phi*sin(phirot2),
					      -Z),
				SiActiveLayerLogical,
				"SiActiveLayer",
				worldLog,
				false,
				cellID0);
	  }		  
	
	
      }
      
#ifdef MOKKA_GEAR
      // sensitive layer
      helpLayer thisSens ;
      if (LayerId==2||LayerId==4||LayerId==6) 
	{ 
	  thisSens.distance  = layer_gap + layer_radius;
	}
      if (LayerId==1||LayerId==3||LayerId==5) 
	{ 
	  thisSens.distance  = layer_radius  - active_silicon_thickness ;
	}
      thisSens.offset    = active_offset_phi ;
      thisSens.thickness = active_silicon_thickness ;
      thisSens.length    = ladder_length ;
      if (active_side_band_electronics_option==1) {
	thisSens.width     = ladder_width*2.+side_band_electronics_width ;
      }
      else  {
	thisSens.width     = ladder_width*2.; 
      }
      thisSens.radLength = (SiActiveLayerLogical->GetMaterial())->GetRadlen()/mm ;
      
      // save information for gear
      gearHelpLadders.push_back( thisLadder );
      gearHelpSensitives.push_back( thisSens ) ;
      gearHelpNumberLadders.push_back( (int) nb_ladder ) ;
      
      // fg: here we start with the first ladder at -pi/2 (i.e. the negative y-axis)
      gearHelpPhi0.push_back( -pi/2. ) ;
      
      gearHelpGap = std::max( gearHelpGap , ladder_gap ) ;
      gearHelpCount ++ ;
#endif
    }
  while(db->getTuple()!=NULL);
  
  
  //****************************************
  // Outer support shell
  //****************************************
  
  // ************central tube************
  
  G4Tubs *SupportShellSolid
    = new G4Tubs("SupportShell",
		 shell_inner_radious,
		 shell_inner_radious+shell_thickess,
		 shell_half_z,
		 sPhi,
		 dPhi);
  
  G4VisAttributes* SupportShellVisAtt = new G4VisAttributes(G4Colour(0.,0.,1.));
  SupportShellVisAtt->SetForceWireframe(true);
  //SJA: SupportShellVisAtt->SetForceWireframe(false);
  //VisAtt->SetForceSolid(true);
  
  G4LogicalVolume *SupportShellLogical=
    new G4LogicalVolume(SupportShellSolid,
			CGAGeometryManager::GetMaterial("beryllium"),
			"SupportShell",
			0,
			0,
			0);
  
  SupportShellLogical->SetVisAttributes(SupportShellVisAtt);
  
  
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., 0.),
		      SupportShellLogical,
		      "SupportShell",
		      worldLog,
		      false,0);
  
  // ************support endplates************
  G4double support_endplate_half_z;
  support_endplate_half_z = shell_endplate_thickness/2;
  
  G4Tubs *EndPlateShellSolid
    = new G4Tubs("EndPlateShell",
		 support_endplate_inner_radious,
		 shell_inner_radious+shell_thickess,
		 support_endplate_half_z,
		 sPhi,
		 dPhi);
  
  G4VisAttributes* EndPlateShellVisAtt = new G4VisAttributes(G4Colour(0.,0.,1.));
  EndPlateShellVisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  
  G4LogicalVolume *EndPlateShellLogical=
    new G4LogicalVolume(EndPlateShellSolid,
			CGAGeometryManager::GetMaterial("beryllium"),
			"EndPlateShell",
			0,
			0,
			0);
  
  EndPlateShellLogical->SetVisAttributes(EndPlateShellVisAtt);
  
  G4double ZEndPlateShell;
  ZEndPlateShell = shell_half_z + shell_endplate_thickness/2.;// + (beryllium_ladder_block_length*2);
  
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
  

  // ************support endplates for the layer 1************
  
  G4double support_endplate_half_z_L1;
  support_endplate_half_z_L1 = shell_thickess/2;

  db->exec("select * from layer;");
  db->getTuple();
  
  G4int LayerId;
  G4double ladder_length;
  G4double ZEndPlateShell2;
  LayerId = db->fetchInt("id");
  ladder_length  = db->fetchDouble("ladder_length");
  
  
  G4Tubs *EndPlateShellSolidL1
    = new G4Tubs("EndPlateShellL1",
		 support_endplate_inner_radious_L1,
		 support_endplate_outer_radious_L1,
		 support_endplate_half_z_L1,
		 sPhi,
		 dPhi);
  
  //G4VisAttributes* EndPlateShell_L1VisAtt = new G4VisAttributes(G4Colour(0.,0.,1.,1.));
  //EndPlateShellVisAtt->SetForceWireframe(false);
  //VisAtt->SetForceSolid(true);
  
  G4LogicalVolume *EndPlateShellLogicalL1=
    new G4LogicalVolume(EndPlateShellSolidL1,
			CGAGeometryManager::GetMaterial("beryllium"),
			"EndPlateShellL1",
			0,
			0,
			0);
  
  EndPlateShellLogicalL1->SetVisAttributes(EndPlateShellVisAtt);
  
  if (LayerId==1) {
  
    ZEndPlateShell2 = ladder_length +  ((end_electronics_half_z*end_ladd_electronics_option) * 2) + shell_thickess/2. + (beryllium_ladder_block_length*2) ;
    
    Phys=
      new G4PVPlacement(0,
			G4ThreeVector(0., 0., ZEndPlateShell2),
			EndPlateShellLogicalL1,
			"EndPlateShellL1",
			worldLog,
			false,0);
    Phys=
      new G4PVPlacement(0,
			G4ThreeVector(0., 0., -ZEndPlateShell2),
			EndPlateShellLogicalL1,
			"EndPlateShellL1",
			worldLog,
			false,0);
  }
  

  //**** beryllium support shell cone ************************************************

  G4double support_cone_half_z = (shell_half_z - (ZEndPlateShell2 + shell_thickess/2.))/2.;
  

  G4Cons *SupportShellCone = 
    new G4Cons("SupportCone", 
	       support_endplate_outer_radious_L1, 
	       support_endplate_outer_radious_L1 + shell_thickess, 
	       support_endplate_inner_radious, 
	       support_endplate_inner_radious + shell_thickess, 
	       support_cone_half_z, 
	       sPhi, 
	       dPhi );

  G4VisAttributes* SupportConeVisAtt = new G4VisAttributes(G4Colour(0.,0.,1.));
  SupportConeVisAtt->SetForceWireframe(true);
  
  G4LogicalVolume *SupportConeLogical=
    new G4LogicalVolume(SupportShellCone,
			CGAGeometryManager::GetMaterial("beryllium"),
			"SupportCone",
			0,
			0,
			0);
  
  SupportConeLogical->SetVisAttributes(SupportConeVisAtt);
  
  G4double ZCone = ladder_length +  ((end_electronics_half_z*end_ladd_electronics_option) * 2) + shell_thickess + (beryllium_ladder_block_length*2) + support_cone_half_z;


  G4RotationMatrix *rot=new G4RotationMatrix();
  rot->rotateX(pi); // the same but other side

  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., ZCone),
		      SupportConeLogical,
		      "SupportCone",
		      worldLog,
		      false,0);
  Phys=
    new G4PVPlacement(rot,
		      G4ThreeVector(0., 0., -ZCone),
		      SupportConeLogical,
		      "SupportCone",
		      worldLog,
		      false,0);


  //*** beryllium support forward part **************************************************************************
        
  G4double supportForZ = shell_half_z + shell_endplate_thickness + forward_shell_half_z;

  G4Tubs *SupportForSolid
    = new G4Tubs("SupportFor",
		 support_endplate_inner_radious,
		 support_endplate_inner_radious + shell_endplate_thickness,
		 forward_shell_half_z,
		 sPhi,
		 dPhi);
  
  G4VisAttributes* SupportForVisAtt = new G4VisAttributes(G4Colour(0.,0.,1.));
  //SJA: SupportShellVisAtt->SetForceWireframe(false);
  //VisAtt->SetForceSolid(true);
  
  G4LogicalVolume *SupportForLogical=
    new G4LogicalVolume(SupportForSolid,
			CGAGeometryManager::GetMaterial("beryllium"),
			"SupportFor",
			0,
			0,
			0);
  
  SupportForLogical->SetVisAttributes(SupportForVisAtt);

  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., supportForZ),
		      SupportForLogical,
		      "SupportFor",
		      worldLog,
		      false,0);


  Phys=
    new G4PVPlacement(rot,
		      G4ThreeVector(0., 0., - supportForZ),
		      SupportForLogical,
		      "SupportFor",
		      worldLog,
		      false,0);
  
  //*** Cryostat ***************************************************************


  aluEndcapZ = dzSty + drSty + drAlu / 2;
  const G4double styEndcapZ = dzSty + drSty / 2;
  
  aluHalfZ = dzSty + drSty;
  
  if (useCryo) {
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

    //Aluminium + styropor endplates for the cryostat
    //Create an apperture at the cryostat endcap for the cabling and the cooling pipes
    
    G4Tubs *aluEndcapSolidInner = new G4Tubs("CryostatAluSkinEndPlateInner", rInner, cryostat_apperture - cryostat_apperture_radius, drAlu / 2, sPhi, dPhi);
    G4LogicalVolume *aluEndcapInnerLog = new G4LogicalVolume(aluEndcapSolidInner, aluMaterial, "CryostatAluSkinEndPlateInner", 0, 0, 0);
    aluEndcapInnerLog->SetVisAttributes(aluVisAttributes);
    new G4PVPlacement(0, G4ThreeVector(0, 0, +aluEndcapZ), aluEndcapInnerLog, "CryostatAluSkinEndPlateInner", worldLog, false, +1);
    new G4PVPlacement(0, G4ThreeVector(0, 0, -aluEndcapZ), aluEndcapInnerLog, "CryostatAluSkinEndPlateInner", worldLog, false, -1);

    G4Tubs *aluEndcapSolidOuter = new G4Tubs("CryostatAluSkinEndPlateOuter", cryostat_apperture + cryostat_apperture_radius, rAlu + drAlu, drAlu / 2, sPhi, dPhi);
    G4LogicalVolume *aluEndcapOuterLog = new G4LogicalVolume(aluEndcapSolidOuter, aluMaterial, "CryostatAluSkinEndPlateOuter", 0, 0, 0);
    aluEndcapOuterLog->SetVisAttributes(aluVisAttributes);
    new G4PVPlacement(0, G4ThreeVector(0, 0, +aluEndcapZ), aluEndcapOuterLog, "CryostatAluSkinEndPlateOuter", worldLog, false, +1);
    new G4PVPlacement(0, G4ThreeVector(0, 0, -aluEndcapZ), aluEndcapOuterLog, "CryostatAluSkinEndPlateOuter", worldLog, false, -1);
    


    G4Tubs *styEndcapSolidInner = new G4Tubs("CryostatFoamEndPlateInner", rInner, cryostat_apperture - cryostat_apperture_radius, drSty / 2, sPhi, dPhi);
    G4LogicalVolume *styEndcapInnerLog = new G4LogicalVolume(styEndcapSolidInner, styMaterial, "CryostatFoamEndPlateInner", 0, 0, 0);
    styEndcapInnerLog->SetVisAttributes(styVisAttributes);
    new G4PVPlacement(0, G4ThreeVector(0, 0, +styEndcapZ), styEndcapInnerLog, "CryostatFoamEndPlateInner", worldLog, false, +1);
    new G4PVPlacement(0, G4ThreeVector(0, 0, -styEndcapZ), styEndcapInnerLog, "CryostatFoamEndPlateInner", worldLog, false, -1);


    G4Tubs *styEndcapSolidOuter = new G4Tubs("CryostatFoamEndPlateOuter", cryostat_apperture + cryostat_apperture_radius, rSty + drSty, drSty / 2, sPhi, dPhi);
    G4LogicalVolume *styEndcapOuterLog = new G4LogicalVolume(styEndcapSolidOuter, styMaterial, "CryostatFoamEndPlateOuter", 0, 0, 0);
    styEndcapOuterLog->SetVisAttributes(styVisAttributes);
    new G4PVPlacement(0, G4ThreeVector(0, 0, +styEndcapZ), styEndcapOuterLog, "CryostatFoamEndPlateOuter", worldLog, false, +1);
    new G4PVPlacement(0, G4ThreeVector(0, 0, -styEndcapZ), styEndcapOuterLog, "CryostatFoamEndPlateOuter", worldLog, false, -1);

  }
  
  G4cout<<"rAlu "<< rAlu<<G4endl;
  G4cout<<"drAlu "<< drAlu<<G4endl;
  G4cout<<"aluHalfZ "<< aluHalfZ<<G4endl;
  G4cout<<"drSty "<< drSty<<G4endl;
  G4cout<<"rInner "<<rInner <<G4endl;
  G4cout<<"+aluEndcapZ "<<+aluEndcapZ <<G4endl;
  G4cout<<"shell_inner_radious "<<shell_inner_radious <<G4endl; 
  G4cout<<"foam inner radius "<<rSty <<G4endl; 
  G4cout << "database name =" << dbName << G4endl;
  
#ifdef MOKKA_GEAR
  // -------write data to gear
  
  // get gear manager
  MokkaGear* gearMgr = MokkaGear::getMgr() ;
  
  // construct VXDParameters
  gear::ZPlanarParametersImpl* vxdParams = 
    new gear::ZPlanarParametersImpl(gearHelpType ,                                        // vxd type 
				shell_inner_radious ,                                 // inner radius
				shell_inner_radious+shell_thickess ,                  // outer radius
				shell_half_z ,                                        // half length
				gearHelpGap ,                                         // shell gap
				(SupportShellLogical->GetMaterial())->GetRadlen()/mm ) ; // shell rad length
  
  // add all layers
  for( int i = 0 ; i < gearHelpCount ; i++ ) {
    vxdParams->addLayer( gearHelpNumberLadders[i] , gearHelpPhi0[i] ,
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
 
void VXD04::GearSetup()
{
  G4double electronic_end_length, alu_RadLen;
  G4double CurrentdEdx, ActiveLayer_dEdx, VXDSupport_dEdx, Cryostat_dEdx, metalTraces_dEdx, foamSpacer_dEdx, flexCable_dEdx, BeSupport_dEdx ;
  G4double mindEdx = 99999,step,step_size=10;
  G4EmCalculator findDEdx;
  G4ParticleTable*theParticleTable=G4ParticleTable::GetParticleTable();



  electronic_end_length = 2*end_electronics_half_z;


  //StripLine_RadLen = stripMaterial->GetRadlen();


  if (useCryo){  
    alu_RadLen = aluMaterial->GetRadlen();
  }
  

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
  //for the Be support  
  G4Material *BeSupportMaterial = CGAGeometryManager::GetMaterial("beryllium");


  mindEdx=99999;
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx = 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  BeSupportMaterial);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }

  BeSupport_dEdx  =(mindEdx)/1000;

  //Looping over bins in the DEDX table to obtain the mip DEDX 
  //From energy 0.0001MeV to 1000MeV in steps of 10
  
  mindEdx=99999;
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx = 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  foamSpacerMaterial);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }

  foamSpacer_dEdx = (mindEdx)/1000;

  mindEdx=99999;
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx = 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  flexCableMaterial);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }

  flexCable_dEdx = (mindEdx)/1000;

  mindEdx=99999;
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx = 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  metalTracesMaterial);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }

  metalTraces_dEdx = (mindEdx)/1000;

  //The dE/dx of the VXD support is calculated as a weighted average of the dE/dx of the 3 different materials that it is constituted

  VXDSupport_dEdx=foamTF*foamSpacer_dEdx + flexTF*flexCable_dEdx + metalTF*metalTraces_dEdx;
  

  //Looping over bins in the DEDX table to obtain the mip DEDX 
  //From energy 0.0001MeV to 1000MeV in steps of 10

  if (useCryo){
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
  }

  gear::GearParametersImpl* gearParameters = new gear::GearParametersImpl;
  
#ifdef LCIO_MODE
  gearParameters -> setDoubleVals("LadderGaps" , ladder_gapVec ) ;
  gearParameters -> setDoubleVals("StripLineFinalZ" , StripLineFinalZ_Vec  ) ;
#endif

  gearParameters -> setDoubleVal( "ElectronicEndThickness" , electronics_structure_thickness ) ;
  gearParameters -> setDoubleVal( "ElectronicEndLength" ,electronic_end_length); 
  gearParameters -> setDoubleVal( "StripLineBeamPipeRadius" ,strip_final_beampipe_radious  ); 
  gearParameters -> setDoubleVal( "VXDEndPlateInnerRadius" , support_endplate_inner_radious ); 
  if (useCryo){
    gearParameters -> setDoubleVal( "CryostatAlRadius" ,rAlu  ); 
    gearParameters -> setDoubleVal( "CryostatAlThickness" , drAlu ); 
    gearParameters -> setDoubleVal( "CryostatAlInnerR" , rInner ); 
    gearParameters -> setDoubleVal( "CryostatAlZEndCap" ,aluEndcapZ ); 
    gearParameters -> setDoubleVal( "CryostatAlHalfZ" ,aluHalfZ ); 
    gearParameters -> setDoubleVal( "Cryostat_RadLen" ,alu_RadLen );
    gearParameters -> setDoubleVal( "Cryostat_dEdx" ,Cryostat_dEdx );
  }
  gearParameters -> setDoubleVal( "ActiveLayerProperties_dEdx" ,ActiveLayer_dEdx );
  gearParameters -> setDoubleVal( "VXDSupport_dEdx" ,VXDSupport_dEdx );
  gearParameters -> setDoubleVal( "BeSupport_dEdx" ,BeSupport_dEdx );
  gearParameters -> setDoubleVal( "BeSupportEndplateThickness" ,shell_endplate_thickness );


  // Write gearParameters to GearMgr
  // Parameters for SIT
  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  gearMgr->registerSimpleMaterial(VXDSupportMaterial); 
  gearMgr->setGearParameters("VXDInfra", gearParameters);
     
 
}
#endif


