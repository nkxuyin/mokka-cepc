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
// $Id: SLHcal01.cc,v 1.6 2008/11/12 16:42:24 mora Exp $
// $Name: mokka-07-00 $
//
// 
//
// SLHcal01.cc
//

#include "Control.hh"
#include "MyPlacement.hh"
#include "G4UserLimits.hh"
#include "SLHcal01.hh"
#include "CGAGeometryManager.hh"
#include "SEcalSDRing02.hh"

#include "globals.hh"
#include "G4Box.hh"
#include "G4SubtractionSolid.hh"
#include "G4Trd.hh"
#include "G4Polyhedra.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
    
#include "CGADefs.h"
    
#ifdef MOKKA_GEAR
#include "gear/CalorimeterParameters.h"
#include "gearimpl/CalorimeterParametersImpl.h"
#include "gearimpl/LayerLayoutImpl.h"
#include "MokkaGear.h"
#endif
    
INSTANTIATE(SLHcal01)
#define VERBOSE 1

#define N_FIBERS_ALVOULUS 3
#define N_FIBERS_W_STRUCTURE 2

G4bool SLHcal01::
ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
		    G4LogicalVolume *theWorld)
{
  G4cout << "\nBuilding SLHcal01"<< G4endl;
  
  theLHcalSD=0;
  // Initialize the Geant3 interface
  if(Control::DUMPG3) MyPlacement::Init("ECAL","SLHcal01");

  // Initialize the driver
  if (!Setup(aGeometryEnvironment)) return false;
  if (!Build(theWorld))  return false;
  return true;
}

G4bool 
SLHcal01::Setup(const CGAGeometryEnvironment &theGeometryEnvironment)
{
  // retrieve the setup parameters and initialize the
  // driver
  
  Ecal_fiber_thickness =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_fiber_thickness");
  Ecal_Alveolus_Air_Gap =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_Alveolus_Air_Gap");
  Ecal_Slab_shielding =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_Slab_shielding");
  Ecal_Slab_copper_thickness =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_Slab_copper_thickness");
  Ecal_Slab_PCB_thickness =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_Slab_PCB_thickness");
  Ecal_Slab_glue_gap =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_Slab_glue_gap");
  Ecal_Slab_ground_thickness =
    theGeometryEnvironment.GetParameterAsDouble("Ecal_Slab_ground_thickness");
  Ecal_Si_thickness = 
    theGeometryEnvironment.GetParameterAsDouble("Ecal_Si_thickness");

  Hcal_endcap_center_box_size =     
    theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_center_box_size");
  Hcal_endcap_zmin = 
   theGeometryEnvironment.GetParameterAsDouble("Hcal_endcap_zmin");
  LHcal_zmin_displacement = 
   theGeometryEnvironment.GetParameterAsDouble("LHcal_zmin_displacement");

  RadiatorMaterial =  CGAGeometryManager::GetMaterial("tungsten_19.3gccm");
  G4String MaterialWarning =  "Radiator material for LHcal " 
    +  RadiatorMaterial->GetName();
  Control::Log(MaterialWarning.data());

  LHcal_Electronics_space =
    theGeometryEnvironment.GetParameterAsDouble("LHcal_Electronics_space");

  LHcal_radiator_thickness =
    theGeometryEnvironment.GetParameterAsDouble("LHcal_radiator_thickness");

  LHcal_cell_size = 
    theGeometryEnvironment.GetParameterAsDouble("LHcal_cell_size");

  LHcal_inner_radius =
    theGeometryEnvironment.GetParameterAsDouble("LHcal_inner_radius");

  TUBE_crossing_angle = 
    theGeometryEnvironment.GetParameterAsDouble("TUBE_crossing_angle");

  LHcal_nlayers =
    theGeometryEnvironment.GetParameterAsInt("LHcal_nlayers");

  LHcal_lateral_face_thickness =
    theGeometryEnvironment.GetParameterAsDouble("LHcal_lateral_face_thickness");

  LHcal_total_Slab_thickness = 
    Ecal_Slab_shielding + 
    Ecal_Slab_copper_thickness + 
    Ecal_Slab_PCB_thickness +
    Ecal_Slab_glue_gap + 
    Ecal_Si_thickness + 
    Ecal_Slab_ground_thickness +
    Ecal_Alveolus_Air_Gap / 2;

  G4double WRadLayerThickness = LHcal_radiator_thickness 
    +  2 * N_FIBERS_W_STRUCTURE * Ecal_fiber_thickness;
    
  G4double AlveoulusLayerThickness = 
    2 * (LHcal_total_Slab_thickness + Ecal_fiber_thickness) 
    + LHcal_radiator_thickness
    + 2 * N_FIBERS_ALVOULUS * Ecal_fiber_thickness;

  module_thickness = 
    LHcal_nlayers/2 * (WRadLayerThickness + AlveoulusLayerThickness);
 
  G4cout << "\nFor information : LHcal thickness = " << module_thickness  
	 << "mm" << G4endl;
  G4cout << "Electronics thickness = " << LHcal_total_Slab_thickness  
	 <<  "mm" << G4endl;

  LhcalBoxSize = Hcal_endcap_center_box_size/2 
    - LHcal_Electronics_space;

  LhcalAvailableBoxSize = LhcalBoxSize
    - LHcal_lateral_face_thickness;

  sensitive_Si_size = LhcalAvailableBoxSize
    - N_FIBERS_ALVOULUS * Ecal_fiber_thickness;

  G4cout << "With the given parameters the Si sensitive plate has the size = " 
	 << sensitive_Si_size <<  "mm" << G4endl;

  // initialize the central box in endcaps
  // Central box become a Tub...
  CenterECTub =
    new G4Tubs ("CenterECTub",
		0.,
		LHcal_inner_radius,
		module_thickness,
		0.,
		2 * pi);

  module_z_offset = 
    Hcal_endcap_zmin + LHcal_zmin_displacement
    + module_thickness /2;
  
  G4double centerTubDispl = 
    module_z_offset * tan(TUBE_crossing_angle /2000);

#ifdef VERBOSE
  G4cout << "module_z_offset = " << module_z_offset 
	 << ", centerTubDispl = " << centerTubDispl << G4endl;
#endif
  
  FollowLcal = new G4TranslateX3D (centerTubDispl);
  FollowLcalZminus = new G4TranslateX3D (-centerTubDispl);
  
  return true;
}


G4bool SLHcal01::Build(G4LogicalVolume* WorldLog)
{
  // Fit the cell size to have a interger number os cells
  // in the given Si plates
  G4double cell_size =  sensitive_Si_size 
    / floor(sensitive_Si_size/LHcal_cell_size);
  LHcal_cell_size = cell_size;
  G4int N_cells = int(sensitive_Si_size / LHcal_cell_size);
  
  G4cout << "\n***** Forcing cell size to " << LHcal_cell_size 
	 << "mm to insure an integer \nnumber of cells."
	 << " With these values the LHcal plates will have \n" << N_cells
	 << " X " << N_cells << " identique cells of " <<LHcal_cell_size
	 << " mm as size. *****\n" << G4endl;
  
  theMaxStepAllowed= LHcal_cell_size;
  
  // Sensitive detector for LHcal : use the same as for the Ecal 
  // endcap ring but without preshower 
  theLHcalSD = 
    new SEcalSDRing02 (LHcal_cell_size,
		       LHcal_cell_size,
		       Ecal_Si_thickness,
		       ECALENDCAPMINUS,
		       "LHcal",
		       false,
		       false);
  RegisterSensitiveDetector(theLHcalSD);

  // End cap ring doesn't rotate and has just one stave
  theLHcalSD->SetStaveRotationMatrix(1,0.);

  // As the central hole is not symmetric and SEcalSDRing02 is a bit stupid,
  // we have to build two independent modules, Z+ and Z-

  // 
  // LHcal envelope
  //
  // Common Z+ & Z- vis attributs
  G4VisAttributes* VisAtt = 
    new G4VisAttributes(G4Colour(0.7,0.7,0.7));
  VisAtt->SetForceSolid(false);
  VisAtt->SetVisibility(true);
  VisAtt->SetDaughtersInvisible(true);

  // Zplus fiber block
  G4Box *LhcalBox = 
    new G4Box ("LhcalSolidBox", 
	       LhcalBoxSize,
	       LhcalBoxSize,
	       module_thickness/2);
  
  G4SubtractionSolid *LhcalSolidZplus =
    new G4SubtractionSolid("LhcalSolidZplus", 
			   LhcalBox, 
			   CenterECTub,
			   *FollowLcal);
  
  LhcalLogZplus =
    new G4LogicalVolume(LhcalSolidZplus,
			CGAGeometryManager::
			GetMaterial("g10"),
			"LhcalLogZPlus", 
			0, 0, 0);
  LhcalLogZplus->SetVisAttributes(VisAtt);
  
  // Zminus fiber block
  G4SubtractionSolid *LhcalSolidZminus =
    new G4SubtractionSolid("LhcalSolidZminus", 
			   LhcalBox, 
			   CenterECTub,
			   *FollowLcalZminus);
  
  LhcalLogZminus =
    new G4LogicalVolume(LhcalSolidZminus,
			CGAGeometryManager::
			GetMaterial("g10"),
			"LhcalLogZminus", 
			0, 0, 0);
  LhcalLogZminus->SetVisAttributes(VisAtt);
  
  // Radiator plates
  // Common Radiator vis attributs
  G4VisAttributes* VisAttRad = 
    new G4VisAttributes(G4Colour(0.7,0.,0.));
  VisAttRad->SetForceSolid(false);
  VisAttRad->SetVisibility(true);
   
  G4Box *LhcalRadBox = 
    new G4Box ("LhcalRadSolid", 
	       LhcalAvailableBoxSize
	       - N_FIBERS_W_STRUCTURE * Ecal_fiber_thickness,
	       LhcalAvailableBoxSize
	       - N_FIBERS_W_STRUCTURE * Ecal_fiber_thickness,
	       LHcal_radiator_thickness /2);

  G4SubtractionSolid *LhcalRadSolidPlus =
    new G4SubtractionSolid("LhcalRadSolid", 
			   LhcalRadBox, 
			   CenterECTub,
			   *FollowLcal);

  LhcalRadiator =
    new G4LogicalVolume(LhcalRadSolidPlus,
			RadiatorMaterial,
			"LhcalRadLog", 
			0, 0, 0);
  LhcalRadiator->SetVisAttributes(VisAttRad);

  // Si sensitive surfaces
  // Common VIS attributs
  G4VisAttributes* VisAttSi = 
    new G4VisAttributes(G4Colour(0.,0.7,0.));
  VisAttSi->SetForceSolid(false);
  VisAttSi->SetVisibility(true);
  
  LhcalSiBox = 
    new G4Box ("LhcalSiSolid", 
	       sensitive_Si_size,
	       sensitive_Si_size, 
	       Ecal_Si_thickness/2);
  
  // Zplus
  G4SubtractionSolid *LhcalSiSolidZplus =
    new G4SubtractionSolid("LhcalSiSolidZplus", 
			   LhcalSiBox, 
			   CenterECTub,
			   *FollowLcal);
  
  G4UserLimits* pULimits=
    new G4UserLimits(theMaxStepAllowed);
  
  LhcalSiLogZplus =
    new G4LogicalVolume(LhcalSiSolidZplus,
			CGAGeometryManager::
			GetMaterial("silicon_2.33gccm"),
			"LhcalSiLogZplus", 
			0, 0, pULimits);  
  LhcalSiLogZplus->SetSensitiveDetector(theLHcalSD);
  LhcalSiLogZplus->SetVisAttributes(VisAttSi);

  // Z minus -> hole not symetric
  G4SubtractionSolid *LhcalSiSolidZminus =
    new G4SubtractionSolid("LhcalSiSolidZminus", 
			   LhcalSiBox, 
			   CenterECTub,
			   *FollowLcalZminus);
  
  LhcalSiLogZminus =
    new G4LogicalVolume(LhcalSiSolidZminus,
			CGAGeometryManager::
			GetMaterial("silicon_2.33gccm"),
			"LhcalSiLogZminus", 
			0, 0, pULimits);  
  LhcalSiLogZminus->SetSensitiveDetector(theLHcalSD);
  LhcalSiLogZminus->SetVisAttributes(VisAtt);

  MyPlacement::InsertComment("Building LHcal"); 

  // Building modules
  //
  // Zplus
  G4double z_floor = - module_thickness / 2.+
    N_FIBERS_W_STRUCTURE * Ecal_fiber_thickness;

  G4int layer_id;
  for (layer_id = 1; layer_id < LHcal_nlayers +1; layer_id +=2 )
    {
      z_floor = BuildLhcalAlveolus (layer_id,
				     z_floor,
				     LhcalLogZplus,
				     false);
    }
  
  // Zminus
  z_floor = - module_thickness / 2.+
    N_FIBERS_W_STRUCTURE * Ecal_fiber_thickness;
  
  for (layer_id = 1; layer_id < LHcal_nlayers +1; layer_id +=2 )
    {
      z_floor = BuildLhcalAlveolus (layer_id,
				     z_floor,
				     LhcalLogZminus,
				     true);
    }

  // Placing modules
  //
  // Zplus
  new MyPlacement(0,
		  G4ThreeVector(0.,
				0.,
				module_z_offset),
		  LhcalLogZplus,
		  "EndCapPhys",
		  WorldLog,
		  false,
		  ECALENDCAPPLUS);
  theLHcalSD->SetModuleZOffset(6,module_z_offset);

  // Zminus
  // rotate to place it on the -Z side
  G4RotationMatrix *rot =  new G4RotationMatrix();
  rot->rotateY(pi);
  new MyPlacement(rot,
		  G4ThreeVector(0.,
				0.,
				-module_z_offset),
		  LhcalLogZminus,
		  "EndCapPhys",
		  WorldLog,
		  false,
		  ECALENDCAPMINUS);

  // module = 6 to flag as endcap module in SD
  theLHcalSD->
    SetModuleZOffset(6,
 		     module_z_offset);  

  
#ifdef MOKKA_GEAR
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++
  // +  MOKKA GEAR                                      +
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++


  // get the information that are not yet included
  helpLHcal.phi0 = 0;
  G4double calcThick = 0;

  helpLHcal.outerRadius = LhcalBoxSize;
  helpLHcal.innerRadius = LHcal_inner_radius;
  helpLHcal.leastZ = module_z_offset - module_thickness / 2.;

  gear::CalorimeterParametersImpl* lhcalParam =
    new gear::CalorimeterParametersImpl(helpLHcal.innerRadius,
					helpLHcal.outerRadius,
					helpLHcal.leastZ,
					2,
					helpLHcal.phi0);
  
  for (int i=1; i < helpLHcal.count; i++)
    {
      calcThick = helpLHcal.layerPos[i] - helpLHcal.layerPos[i-1] ;    
      lhcalParam->layerLayout().positionLayer
	(0, calcThick, LHcal_cell_size, LHcal_cell_size, 
	 helpLHcal.radiThickness[i-1]);
    }

  // the last layer...
  lhcalParam->layerLayout().positionLayer
    (0, calcThick, LHcal_cell_size, LHcal_cell_size, 
     helpLHcal.radiThickness[helpLHcal.count-1]);
  
  lhcalParam->setDoubleVal("beam_crossing_angle", TUBE_crossing_angle ) ;
  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setLHcalParameters ( lhcalParam );
#endif

  G4cout << "LHcal done.\n" << G4endl;
  return true;  
}

SLHcal01::~SLHcal01() 
{
}  

void 
SLHcal01::LoadEvent(FILE* )
{
  // Note sure that it still works fine...
  //theLHcalSD->LoadEvent(theSubDetectorEventHitsFileInput);
}

G4double
SLHcal01::BuildLhcalAlveolus (G4int layer_id,
			       G4double z_floor,
			       G4LogicalVolume* ECLog,
			       G4bool Zminus)
{
  // If Zminus, rotate radiator plates as it the same for both modules
  G4RotationMatrix *rot =  new G4RotationMatrix();
  if(Zminus) rot->rotateY(pi);

  // LHcal starts by the radiator 
  new MyPlacement(rot,
		  G4ThreeVector (0,
				 0,
				 z_floor + LHcal_radiator_thickness / 2),
		  LhcalRadiator,
		  "RadLhcal",
		  ECLog,
		  false,
		  0);

#ifdef MOKKA_GEAR
  // get positions of Layer as the middle of the radiator layer
  if(!Zminus)
    {
      helpLHcal.layerPos.push_back(z_floor 
				  + LHcal_radiator_thickness / 2);
      // get radiator thickness
      helpLHcal.radiThickness.push_back(LHcal_radiator_thickness);
      // count layers
      helpLHcal.count ++;
    }
#endif

  z_floor +=  LHcal_radiator_thickness 
    + N_FIBERS_W_STRUCTURE * Ecal_fiber_thickness 
    + N_FIBERS_ALVOULUS * Ecal_fiber_thickness;
  
  // Then slab
  G4LogicalVolume *SiLog = LhcalSiLogZplus;
  if(Zminus) SiLog = LhcalSiLogZminus;
  
  // place Si 1 (in z_floor + Ecal_total_Slab_thickness / 2)
  new MyPlacement(0,
		  G4ThreeVector (0,
				 0,
				 z_floor + LHcal_total_Slab_thickness / 2),
		  SiLog,
		  "SlabSiLhcal",
		  ECLog,
		  false,
		  layer_id);
  
  // set layer in SD
  if(!Zminus)
    theLHcalSD->
      AddLayer(layer_id,
 	       - LhcalSiBox->GetXHalfLength(),
 	       - LhcalSiBox->GetYHalfLength(),
 	       z_floor + LHcal_total_Slab_thickness / 2);
    
    z_floor += LHcal_total_Slab_thickness + Ecal_fiber_thickness;

  // place Rad (in z_floor + Ecal_total_Slab_thickness +
  // N X fiber + RadThick / 2)
  new MyPlacement(rot,
		  G4ThreeVector (0,
				 0,
				 z_floor + LHcal_radiator_thickness / 2),
		  LhcalRadiator,
		  "RadLhcal",
		  ECLog,
		  false,
		  0);
  
#ifdef MOKKA_GEAR
  // get positions of Layer as the middle of the radiator layer
  if(!Zminus)
    {
      helpLHcal.layerPos.push_back(z_floor 
				  + LHcal_radiator_thickness / 2);
      // get radiator thickness
      helpLHcal.radiThickness.push_back(LHcal_radiator_thickness);
      // count layers
      helpLHcal.count ++;
    }
#endif
  
  z_floor +=  LHcal_radiator_thickness + Ecal_fiber_thickness;

  // place 2th Si in slab 
  //
  new MyPlacement(0,
		  G4ThreeVector (0,
				 0,
				 z_floor + LHcal_total_Slab_thickness / 2),
		  SiLog,
		  "SlabSiLhcal",
		  ECLog,
		  false,
		  layer_id + 1);
 
  if(!Zminus)
    theLHcalSD->
      AddLayer(layer_id+1,
	       - LhcalSiBox->GetXHalfLength(),
	       - LhcalSiBox->GetYHalfLength(),
	       z_floor + LHcal_total_Slab_thickness / 2);

  z_floor +=  LHcal_total_Slab_thickness
    + N_FIBERS_ALVOULUS * Ecal_fiber_thickness 
    + N_FIBERS_W_STRUCTURE * Ecal_fiber_thickness;
   return z_floor;
}

G4bool 
SLHcal01::PostConstructAction(CGAGeometryEnvironment& )
{
  // None to be propagate for the moment... 
  std::ostringstream oss;  
  oss << module_z_offset + module_thickness /2;
  (*Control::globalModelParameters)["LHcal_zend"] = oss.str();
  G4cout << "SLHcal information:  = setting LHcal_zend to " 
	 <<  module_z_offset + module_thickness /2
	 << " mm"  <<  G4endl;
  return true;
}
