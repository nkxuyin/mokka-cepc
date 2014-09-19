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
// $Id: Proto01.cc,v 1.4 2005/04/08 14:37:15 musat Exp $
// $Name: mokka-07-00 $
//
//
// Proto01.cc
//
// History:  

#include "G4PVPlacement.hh"
#include "G4Material.hh"

#include "Proto01.hh"
#include "CGAGeometryManager.hh"

#include "globals.hh"
#include "G4Box.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4UserLimits.hh"
#include "MySQLWrapper.hh"
#include "Control.hh"

#include "ProtoSD.hh"

#include <algorithm>

#include "CGADefs.h"

INSTANTIATE(Proto01)

G4bool Proto01::construct(const G4String &aSubDetectorName,
			  G4LogicalVolume *WorldLog)

{
  G4cout << "\nBuilding Proto release 01" << G4endl;
  db = new Database(aSubDetectorName.data());
  
  // BuildElements takes the main parameters and builds 
  // the basic logical volumes
  BuildElements();
  
  G4int i_plate=0;
  
  for (unsigned int i_group=0;i_group<PlateGroups.size();i_group++)
    {
      BuildDeadPlate(PlateGroups[i_group]);
      BuildAlveolus(PlateGroups[i_group]);
      for (i_plate=0;i_plate<PlateGroups[i_group]->n_layers;i_plate++)
	Plates.push_back(PlateGroups[i_group]);
    }

  //--------------
  // Detector:
  //--------------

  G4double HalfDetectorY=0;
  for(i_plate=0;i_plate<total_W_plates;i_plate++)
    if(i_plate%2==0)
      HalfDetectorY+=Plates[i_plate]->AsDeadTotalHalfY;
    else
      HalfDetectorY+=Plates[i_plate]->AsAlveolusTotalHalfY;

  // N times an half fiber_thickness + an half at 
  // entry and exit module faces.
  HalfDetectorY+=total_W_plates*fiber_thickness/2. + fiber_thickness;

  // the number of W dead plates is the enough to cover
  // all alveolus X dimension
  G4double totalAlveolusX = 2*(HalfAlveolusX+HalfDetectorY*tan_rate);  
  n_dead_w_plates = 
    int(totalAlveolusX / (2*(HalfDeadWX+fiber_thickness))) + 1;

  G4double HalfDetectorX=
    n_dead_w_plates*(HalfDeadWX+fiber_thickness);
  
  G4double HalfDetectorZ= // plus 1 mm de chaque cote
    n_towers*(HalfAlveolusZ+inter_tower_fiber_thickness)+1.;
  
  G4Box *DetectorSolid= new G4Box("DetectorBox",
				  HalfDetectorX,
				  HalfDetectorY,
				  HalfDetectorZ);
  
  //>>>>>>>>>>>>>>>>>> VER MATERIAL !!!!!!!!!!!!
  DetectorLogical=
    new G4LogicalVolume(DetectorSolid,
			CGAGeometryManager::GetMaterial("g10"),
			"DetectorLogical", 
			0, 
			0, 
			0);
  G4VisAttributes * VisAttDetector = new G4VisAttributes(G4Colour(0.,1.,0.));
  VisAttDetector->SetForceWireframe(true);
  DetectorLogical->SetVisAttributes(VisAttDetector);

  //--------------
  // Detector Placement
  //--------------
  G4double x_center,y_center,z_center;
  x_center=y_center=z_center=0;
  db->exec("select x_center,y_center,z_center from proto;");
  if(db->getTuple())
    {
      x_center = db->fetchDouble("x_center");
      y_center = db->fetchDouble("y_center");
      z_center = db->fetchDouble("z_center");
    }
  else
    {
      Control::Log("proto01 Warning: old database release without (x_center,y_center,z_center) values, assuming (0.,2*HalfDetectorY,0.).");
      y_center = 2*HalfDetectorY;
    }
  char buff[80];
  sprintf(buff,"proto01: proto size is (%f,%f,%f) mm",2*HalfDetectorX,
	  2*HalfDetectorY,2*HalfDetectorZ);
  Control::Log(buff);
  sprintf(buff,"proto01: placing prototype at (%f,%f,%f) mm",x_center,y_center,z_center);
  Control::Log(buff);
  
  new G4PVPlacement(0,
		    G4ThreeVector(x_center, y_center, z_center),
		    DetectorLogical,
		    "FiberBlock",
		    WorldLog,
		    false,0);

  //----------------------
  // Tower Placements
  //----------------------

  G4double DispZTower=0.;
  // si pair, paroi central
  if(n_towers%2==0)
    DispZTower=inter_tower_fiber_thickness/2+HalfAlveolusZ+
      (n_towers/2-1)*(2*HalfAlveolusZ+inter_tower_fiber_thickness);
  else
    if(n_towers>1)
      DispZTower=n_towers/2*
	(2*HalfAlveolusZ+inter_tower_fiber_thickness);

  G4double DispZ=0;
  for(G4int i_tower=0;i_tower<n_towers;i_tower++) 
    {
      //----------------------
      // Layers inside the tower Placements
      //----------------------
      G4LogicalVolume *NextLayer;
      G4double DeltaY=0;
      G4int CopyNumber=0;
      G4double DispY=-HalfDetectorY+ fiber_thickness ;  
      for(i_plate=0;i_plate<total_W_plates;i_plate++)
	{
	  //if(i_plate == 4 ) return true; // for debug
	  if(i_plate%2==0) // Dead plates
	    {
	      NextLayer=Plates[i_plate]->AsDeadWLogical;
	      DispY+=DeltaY=Plates[i_plate]->AsDeadTotalHalfY
		+ fiber_thickness / 2.;
	      DispZ=0;
	      CopyNumber=0;
	      G4double DeadDispX=0;
	      if(n_dead_w_plates%2==0)
		DeadDispX = fiber_thickness+HalfDeadWX+
		  (n_dead_w_plates/2-1)*2.*(HalfDeadWX+fiber_thickness);
	      else
		if(n_dead_w_plates>1)
		  DeadDispX = n_dead_w_plates/2*(2*HalfDeadWX+2*fiber_thickness);
	      if(i_tower==0) // just one time in this direction!!!
		for(G4int i_dead_plate=0;i_dead_plate<n_dead_w_plates;i_dead_plate++)
		  {
		    new G4PVPlacement(0,
				      G4ThreeVector(DeadDispX, DispY, DispZ),
				      NextLayer,
				      "DeadW",
				      DetectorLogical,
				      false,CopyNumber);
		    DeadDispX-=2*(HalfDeadWX+fiber_thickness);
		  }
	    }
	  else
	    {
	      NextLayer=Plates[i_plate]->AsAlveolusLogical;
	      DispY+=DeltaY=Plates[i_plate]->AsAlveolusTotalHalfY
		+ fiber_thickness  / 2. ;
	      DispZ=DispZTower;
	      CopyNumber= i_tower*100 + i_plate;
	      new G4PVPlacement(0,
				G4ThreeVector(DispY*tan_rate, DispY, DispZ),
				NextLayer,
				"Alveolus",
				DetectorLogical,
				false,CopyNumber);
	    }
	  DispY+=DeltaY;
	}
      DispZTower=DispZTower-(2*HalfAlveolusZ+inter_tower_fiber_thickness);
    }
  // Closes Database connection
  delete db;
  db = 0;
  G4cout << "Proto done.\n" << G4endl;
  return true;
}
Proto01::~Proto01()
{
//   if(theProtoSD!=0) delete theProtoSD;
}

void Proto01::BuildElements() 
{
  G4VisAttributes * VisAtt;
  
  //---------------------------------
  // Prend la config de plaques
  //---------------------------------
  total_W_plates = 0;
  db->exec("select * from w_layers;");
  while (db->getTuple())
    {
      G4int n_layers = db->fetchInt("n_layers");
      PlateGroups.
	push_back(new WLAYERS(n_layers,
			      db->fetchInt("plate_multiplicity")));
      total_W_plates+=n_layers;
    }
  G4cout << "total_W_layers = " << total_W_plates << G4endl;
  
  //---------------------------------
  // Take general parameters
  //---------------------------------
  db->exec("select * from proto;");
  db->getTuple();
  
  G4double cell_dim_x = db->fetchDouble("cell_dim_x");
  G4double cell_dim_z = db->fetchDouble("cell_dim_z");
  // The cell boundaries does not really exist as G4 volumes. So,
  // to avoid long steps over running  several cells, the 
  // theMaxStepAllowed inside the sensitive material is the
  // pad smaller x or z dimension.
  theMaxStepAllowed= std::min(cell_dim_x,cell_dim_z);

  //---------------------------
  // Waffer, partie Sensitive
  //---------------------------
  G4double HalfSensitiveWafferX = 
    cell_dim_x/2*db->fetchInt("n_cell_x");
  G4double HalfSensitiveWafferY = 
    db->fetchDouble("si_thickness")/2;
  G4double HalfSensitiveWafferZ = 
    cell_dim_z/2*db->fetchInt("n_cell_z");
  
  //---------------------------
  // Waffer, total
  //---------------------------
  HalfWafferX = 
    HalfSensitiveWafferX+db->fetchDouble("garde_size");
  HalfWafferY = 
    HalfSensitiveWafferY;
  HalfWafferZ = 
    HalfSensitiveWafferZ+db->fetchDouble("garde_size");
  
  //----------------------------------------------
  // Dimensions X x Z du support pour les waffers
  // (version deux waffers par support)
  //----------------------------------------------
  HalfSuppX = 
    // 2 fois la taille du waffer fois le nombre (si multiple)
    2*db->fetchInt("multiplicity_waffers")*HalfWafferX +
    // gap inter waffers (si NxN par module)
    (db->fetchInt("multiplicity_waffers")-1)*db->fetchDouble("inter_waffer_gap")/2 +
    // gap inter les deux jeux de waffers sur le support
    db->fetchDouble("inter_waffer_gap") +
    // gap inter waffers sur les bords du support (2 fois)
    db->fetchDouble("inter_waffer_gap");
  
  HalfSuppZ = 
    // taille du waffer fois le nombre (si NxN par module)
    db->fetchInt("multiplicity_waffers")*HalfWafferZ +
    // gap inter waffers (si NxN par module)
    (db->fetchInt("multiplicity_waffers")-1)*db->fetchDouble("inter_waffer_gap")/2 +
    // gap inter waffers sur les bords du support (2 fois)
    db->fetchDouble("inter_waffer_gap");    
  

  //--------------------------------------------
  // angle en radiens d'inclination des alveolus
  //--------------------------------------------
  G4double x_offset_rate = db->fetchDouble("x_offset_rate");
  tan_rate = tan(pi/2-x_offset_rate);

  //-------------------------------------
  // Dimensions de l'alveolus
  //-------------------------------------
  HalfAlveolusX =
    HalfSuppX * db->fetchInt("n_elements_x") + 
    (db->fetchInt("n_elements_x")-1)*db->fetchDouble("inter_element_gap")+
    db->fetchDouble("wafer_shift") +         // space for wafer uper/under shift 
    db->fetchDouble("lateral_alveolus_gap"); // jeu 2 * chaque cote
  
  HalfAlveolusZ =
    HalfSuppZ+
    db->fetchDouble("lateral_alveolus_gap"); // jeu 2 * chaque cote
  
  //------------------------------------
  // Dimensions W support des Si (WSlab)
  //------------------------------------
  HalfWSlabX = HalfAlveolusX - db->fetchDouble("lateral_alveolus_gap");
  HalfWSlabZ = HalfSuppZ;
  
  //---------------------------------------------
  // Building basic elements
  //---------------------------------------------
  //---------------------------------------------
  // Volume Waffer mort, avec l'anneau de garde
  //---------------------------------------------
  G4Box *WafferSolid  = new G4Box("WafferSolid",
				  HalfWafferX,
				  HalfWafferY,
				  HalfWafferZ);
  
  WafferLogical=
    new G4LogicalVolume(WafferSolid,
			CGAGeometryManager::GetMaterial("silicon_2.33gccm"),
			"WafferLogical", 
			0, 
			0, 
			0);
  
  VisAtt = new G4VisAttributes(G4Colour(0.,1.,1.));
  VisAtt->SetForceWireframe(true);
  WafferLogical->SetVisAttributes(VisAtt);
  
  //----------------------------------------------------------
  // Placement Volume Waffer sensitive, dedans le waffer mort
  //----------------------------------------------------------
  G4Box *SensWafferSolid  = new G4Box("SensWafferSolid",
				      HalfSensitiveWafferX,
				      HalfSensitiveWafferY,
				      HalfSensitiveWafferZ);
  
  G4UserLimits* pULimits=
    new G4UserLimits(theMaxStepAllowed);

  G4LogicalVolume *SensWafferLogical=
    new G4LogicalVolume(SensWafferSolid,
			CGAGeometryManager::GetMaterial("silicon_2.33gccm"),
			"SensWafferLogical", 
			0, 
			0, 
			pULimits);

  VisAtt = new G4VisAttributes(G4Colour(1.,1.,1.));
  //VisAtt->SetForceWireframe(true);
  VisAtt->SetForceSolid(true);

  SensWafferLogical->SetVisAttributes(VisAtt);

  // sensitive detector plugin
  theProtoSD = new
    ProtoSD(db->fetchDouble("cell_dim_x"),
	    db->fetchDouble("si_thickness"),
	    db->fetchDouble("cell_dim_z"),
	    db->fetchInt("n_cell_x"),
	    db->fetchInt("n_cell_z"),
	    db->fetchInt("multiplicity_waffers"),
	    db->fetchInt("n_elements_x"),
	    db->fetchInt("n_towers"),
	    "ProtoSD");
  RegisterSensitiveDetector(theProtoSD);
  SensWafferLogical->SetSensitiveDetector(theProtoSD);

  new G4PVPlacement(0,
		    G4ThreeVector(0., 0., 0.),
		    SensWafferLogical,
		    "SensWafferPhys",
		    WafferLogical,
		    false,0);

  //--------------------------
  // Cu entre Si et Kapton
  //--------------------------
  G4double HalfCuX = HalfWSlabX;
  HalfCuY = db->fetchDouble("cu_thickness")/2;
  G4double HalfCuZ = HalfWSlabZ;
  
  G4Box *CuSolid  = new G4Box("CuSolid",
			      HalfCuX,
			      HalfCuY,
			      HalfCuZ);
  
  CuLogical=
    new G4LogicalVolume(CuSolid,
			CGAGeometryManager::GetMaterial("copper"),
			"CuLogical", 
			0, 
			0, 
			0);
  VisAtt = new G4VisAttributes(G4Colour(1.,0.,0.));
  VisAtt->SetForceWireframe(true);
  CuLogical->SetVisAttributes(VisAtt);
  
  //----------
  // Mix with G10/Cu/Kapton/W/Kapton/Al/Air
  //----------
  HalfMixY = 
    (db->fetchDouble("g10_thickness")+
     db->fetchDouble("cu2_thickness")+
     db->fetchDouble("kapton1_thickness")+
     db->fetchDouble("w_wires_thickness")+
     db->fetchDouble("kapton2_thickness")+
     db->fetchDouble("al_thickness")+
     db->fetchDouble("air_thickness")
     )/2;

  // masses calculees pour un box de 1 mm**2 de base
  G4double Base = 1. * 1.;

  G4double G10Mass = CGAGeometryManager::GetMaterial("g10")->GetDensity()*
    Base*db->fetchDouble("g10_thickness");
  G4double CuMass = CGAGeometryManager::GetMaterial("copper")->GetDensity()*
    Base*db->fetchDouble("cu2_thickness");
  G4double KaptonMass = CGAGeometryManager::GetMaterial("kapton")->GetDensity()*
    Base*
    (db->fetchDouble("kapton1_thickness")+
     db->fetchDouble("kapton2_thickness"));
  G4double WMass = CGAGeometryManager::GetMaterial("tungsten_19.3gccm")->GetDensity()*
    Base*db->fetchDouble("w_wires_thickness");
  G4double AlMass = CGAGeometryManager::GetMaterial("aluminium")->GetDensity()*
    Base*db->fetchDouble("al_thickness");
  G4double AirMass = CGAGeometryManager::GetMaterial("air")->GetDensity()*
    Base*db->fetchDouble("air_thickness");

  // masse totale pour calculer les fractions
  G4double TotalMass = 
    G10Mass+CuMass+KaptonMass+WMass+AlMass+AirMass;

  // densite du melange = masse totale/ volume
  G4double MixDensite = TotalMass/(2*HalfMixY*Base);
  G4cout << "MixDensite = " 
	 << MixDensite / g * cm3 
	 << " g/cm3" << G4endl;

  // definition du melange
  Mix = new G4Material("Mix",MixDensite,6);
  Mix->AddMaterial(CGAGeometryManager::GetMaterial("g10"),G10Mass/TotalMass);
  Mix->AddMaterial(CGAGeometryManager::GetMaterial("copper"),CuMass/TotalMass);
  Mix->AddMaterial(CGAGeometryManager::GetMaterial("kapton"),KaptonMass/TotalMass);
  Mix->AddMaterial(CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),WMass/TotalMass);
  Mix->AddMaterial(CGAGeometryManager::GetMaterial("aluminium"),AlMass/TotalMass);
  Mix->AddMaterial(CGAGeometryManager::GetMaterial("air"),AirMass/TotalMass);
  G4cout << "Mix->GetRadlen()= " 
	 << Mix->GetRadlen() /mm   << " mm\n";

  fiber_thickness=db->fetchDouble("fiber_thickness");
  inter_tower_fiber_thickness=
    db->fetchDouble("inter_tower_fiber_thickness");

  // number of towers
  n_towers=db->fetchInt("n_towers");

  //-----------------------------------
  // Dead W Plates common parameters
  //-----------------------------------
  //---------------------------------
  // W basic plate thickness
  //---------------------------------
  nominal_w_thickness = 
    db->fetchDouble("nominal_w_thickness");

  //HalfDeadWX = db->fetchDouble("dead_w_dimx")/2.;
  HalfDeadWX = HalfSuppZ;

  HalfDeadWZ=
    (n_towers-1)*
    (HalfAlveolusZ+inter_tower_fiber_thickness/2)+
    HalfAlveolusZ;
}

void Proto01::BuildDeadPlate(WLAYERS* aPlate)
{
  
  //-----------------------------------
  // Build the W dead matter for dead layers
  // and Slabs
  //-----------------------------------

  // axys X : several lanes
  // axys Z : several lego blocks

  // get the number of lego blocks per lane
  // (we assume here that the lane number 1 exists and that
  //  all the lanes have the same number of blocks)
  db->exec("select count(*) AS N_BLOCKS from w_profile where lane = 1;");
  db->getTuple();
  G4int n_blocks_per_lane = db->fetchInt("N_BLOCKS");

  // get the number of lanes
  // (as the total number of lines in the table / number of 
  // lego blocks per lane)
  db->exec("select distinct count(*) AS N_TOTAL from w_profile;");
  G4int n_lanes = db->fetchInt("N_TOTAL")/n_blocks_per_lane;

  // HalfX for each lane (half of the block size in X)
  G4double HalfLaneX = HalfDeadWX / n_lanes;

  // get the lego block size in X will be adapted to keep
  // blocks of egal size.
  //
  // original block size in Z = profile_step_size
  //
  db->exec("select profile_step_size from proto;");
  db->getTuple();
  G4double  HalfBlockZ = db->fetchDouble("profile_step_size") / 2.;

  // adapt the block size in Z to be a integer number of
  // lego blocks
  G4int n_Z_blocks_std = (int) (HalfDeadWZ/HalfBlockZ);
  HalfBlockZ = HalfDeadWZ/n_Z_blocks_std;

  // For Y, we look for the bigger plate thickness
  G4double HalfMaxPlateY = 0;
  db->exec("SELECT thickness FROM w_profile;");
  while(db->getTuple())
    if(db->fetchDouble("thickness") > HalfMaxPlateY)
      HalfMaxPlateY=db->fetchDouble("thickness");

  // and we divise by 2 because it's "half" value
  HalfMaxPlateY/=2;
  
  // For the total Y half size, we multiply it by the 
  // plate_multiplicity
  G4double HalfDeadWY = 
    HalfMaxPlateY * aPlate->plate_multiplicity;
  
  // we have to be sure the all keeps inside the mother block
  if(HalfDeadWY> 
     (nominal_w_thickness * aPlate->plate_multiplicity) + fiber_thickness)
    Control::Abort("nominal_w_thickness smaller than the biggest block in profile!",MOKKA_OTHER_ERRORS);
  
  // We put all the lego blocks inside a 
  // fiber mother box "DeadMotherBox"
  G4Box *DeadWSolid  = new G4Box("DeadWSolid",
				    HalfLaneX*n_lanes,
				    HalfDeadWY,
				    HalfDeadWZ);

  // Logical for dead matter
  aPlate->AsDeadWLogical=
    new G4LogicalVolume(DeadWSolid,
			CGAGeometryManager::GetMaterial("g10"),
			"DeadWLogical", 
			0, 
			0, 
			0);
  
  G4VisAttributes * VisAttDeadW = new G4VisAttributes(G4Colour(1.,1.,1.));
  //VisAttDeadW->SetVisibility(false);
  VisAttDeadW->SetForceWireframe(true);
  aPlate->AsDeadWLogical->SetVisAttributes(VisAttDeadW);

  // 
  // building the standard W lego blocks, they will be
  // keep in the LaneWBlocks array.
  //
  G4VisAttributes * VisAttWBlock = new G4VisAttributes(G4Colour(1.,0.2,1.));
  //VisAttWBlock->SetForceWireframe(true);
  VisAttWBlock->SetForceSolid(true);
  std::vector<G4LogicalVolume*> LaneWBlocks;
  db->exec("SELECT id,lane,thickness FROM w_profile order by id;");
  while(db->getTuple())
    {
      DeadWSolid  = new G4Box("DeadWBlock",
			      HalfLaneX,
			      db->fetchDouble("thickness")/2.,
			      HalfBlockZ);
      G4LogicalVolume *LV =
	new G4LogicalVolume(DeadWSolid,
			    CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
			    "DeadWBlockLogical", 
			    0, 
			    0, 
			    0);
      LV->SetVisAttributes(VisAttWBlock);
      LaneWBlocks.push_back(LV);
    }
  
  // 
  // For each level in multiplicity we put the lanes.
  // For each lane, we place the lego blocks
  // in ciclic way
  //
  G4double firstOffY = ( - aPlate->plate_multiplicity + 1) * HalfMaxPlateY;
  if((aPlate->plate_multiplicity % 2) != 0)
	    firstOffY = -(int(aPlate->plate_multiplicity/2)) 
	      * 2*HalfMaxPlateY;
  G4int ilevel;
  for(ilevel=0;ilevel < aPlate->plate_multiplicity;ilevel++)
    {    
      for(G4int ilane=0; ilane < n_lanes; ilane++)
	{
	  // firstOffX = more left lego block X coordinate
	  // (it's not the same if pair/impair number of blocks)
	  G4double firstOffX = (-n_lanes + 1) * HalfLaneX;
	  if((n_lanes % 2) != 0)
	    firstOffX = -(int(n_lanes/2)) * 2*HalfLaneX;
	  
	  // firstOffZ = more left lego block Z coordinate
	  // (it's not the same if pair/impair number of blocks)
	  G4double firstOffZ = (-n_Z_blocks_std + 1) * HalfBlockZ;
	  if((n_Z_blocks_std % 2) != 0) 
	    firstOffZ = -(int(n_Z_blocks_std/2)) * 2*HalfBlockZ;
	  
	  // placing
	  for(G4int iBlock=0; iBlock < n_Z_blocks_std; iBlock++)
	    {
	      G4int stdBlockIndex = (iBlock % n_blocks_per_lane);	  
	      new G4PVPlacement(0,
				G4ThreeVector(firstOffX + ilane * 2*HalfLaneX,
					      firstOffY + ilevel * 
					      2*HalfMaxPlateY,
					      firstOffZ + iBlock * 
					      2*HalfBlockZ),
				LaneWBlocks[stdBlockIndex],
				"DeadWBlock",
				aPlate->AsDeadWLogical,
				false,0);
	    }
	}
    }
  
  //-----------------------------------------------------------
  // TotalHalfY = HalfDeadWY
  //-----------------------------------------------------------
  aPlate->AsDeadTotalHalfY= 
    nominal_w_thickness / 2.  * aPlate->plate_multiplicity; 

  //------------------------------------------
  // W for Slabs
  //------------------------------------------
  //
  // the size in Z is the same as for the alveolus

  // Air mother box but we change X <-> Z
  // to avoid rotation
  DeadWSolid  = new G4Box("AirSlabWSolid",
			  HalfWSlabX,
			  HalfDeadWY,
			  HalfLaneX*n_lanes);
  // Logical for Slab
  aPlate->AsSlabWLogical=
    new G4LogicalVolume(DeadWSolid,
			CGAGeometryManager::GetMaterial("air"),
			"WSlabLogical", 
			0, 
			0, 
			0);
  aPlate->AsSlabWLogical->SetVisAttributes(VisAttDeadW);

  //
  // We keep the same 2*HalfBlockZ block size but the last
  // one will be small to fill all the alveolus
  G4int n_X_blocks_std = (int) (HalfWSlabX/HalfBlockZ);

  // Dimension of the last block
  G4double HalfLastBlockX = 
    HalfWSlabX - (n_X_blocks_std*HalfBlockZ);
    
  // builds the last block
  DeadWSolid  = new G4Box("LastWSolid",
			  HalfLaneX,
			  HalfMaxPlateY,
			  HalfLastBlockX);

  G4LogicalVolume* LastBk = 
    new G4LogicalVolume(DeadWSolid,
			CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
			"LastWBlockLogical", 
			0, 
			0, 
			0);
  G4VisAttributes * VisAttLastWBlock = new G4VisAttributes(G4Colour(0.0,0.7,0.0));
  VisAttLastWBlock->SetForceSolid(true);
  LastBk->SetVisAttributes(VisAttLastWBlock);

  // 
  // For each level in multiplicity we put the lanes.
  // For each lane, we place the lego blocks
  // in ciclic way and then the last one.
  //

  // remember, X<->Z !!!
  // firstOffZ = more left lego block Z coordinate
  // (it's not the same if pair/impair number of blocks)
  G4double firstOffZ = (-n_lanes + 1) * HalfLaneX;
  if((n_lanes % 2) != 0)
    firstOffZ = -(int(n_lanes/2)) * 2*HalfLaneX;
  
  // firstOffX = more left lego block X coordinate
  // (it's not the same if pair/impair number of blocks)
  G4double firstOffX = (-n_X_blocks_std + 1) * HalfBlockZ;
  if((n_X_blocks_std % 2) != 0) 
    firstOffX = -(int(n_X_blocks_std/2)) * 2*HalfBlockZ;
  firstOffX -= HalfLastBlockX;
  
  G4RotationMatrix *rot=new G4RotationMatrix();
  rot->rotateY(pi*0.5); // to change the block direction

  for(ilevel=0;ilevel < aPlate->plate_multiplicity;ilevel++)
    {    
      for(G4int ilane=0; ilane < n_lanes; ilane++)
	{
	  // placing
	  G4int iBlock = 0;
	  for(iBlock=0; iBlock < n_X_blocks_std; iBlock++)
	    {
	      G4int stdBlockIndex = (iBlock % n_blocks_per_lane);
	      G4ThreeVector Pos =
		G4ThreeVector(firstOffX + iBlock * 2*HalfBlockZ,
			      firstOffY + ilevel * 2*HalfMaxPlateY,
			      firstOffZ + ilane * 2*HalfLaneX);
	      new G4PVPlacement(rot,
				Pos,
				LaneWBlocks[stdBlockIndex],
				"SlabWBlock",
				aPlate->AsSlabWLogical,
				false,0);
	    }
	  new G4PVPlacement(rot,
			    G4ThreeVector(firstOffX + 
					  (n_X_blocks_std-1)*2*HalfBlockZ +
					  HalfBlockZ + HalfLastBlockX,
					  firstOffY + ilevel * 2*HalfMaxPlateY,
					  firstOffZ + ilane * 2*HalfLaneX),
			    LastBk,
			    "LastSlabWBlock",
			    aPlate->AsSlabWLogical,
			    false,0);
	}
    }
  
  //-----------------------------------------------------------
  // TotalHalfY = HalfDeadWY + fibre 2 fois (1 chaque cote)
  //-----------------------------------------------------------
  aPlate->AsSlabWTotalHalfY= HalfDeadWY;
}


void Proto01::BuildAlveolus(WLAYERS* aPlate)
{

  //------------------------------------
  // VisAttributes pour Alevolus et Slab
  //------------------------------------
  G4VisAttributes * VisAttAlv = new G4VisAttributes(G4Colour(1.,1.,0.2));
  VisAttAlv->SetForceWireframe(true);
  G4VisAttributes * VisAttWSlab = new G4VisAttributes(G4Colour(1.,0.2,1.));
  VisAttWSlab->SetForceWireframe(true);
  VisAttWSlab->SetDaughtersInvisible(false);
  
  db->exec("select * from proto;");
  db->getTuple();

  //-----------------------------------
  // Logical type Alveolus
  //-----------------------------------
  G4double HalfAlveolusY =
    aPlate->plate_multiplicity * nominal_w_thickness / 2 +    // 1 fois
    HalfWafferY*2+    // 2 fois
    HalfCuY*2+        // 2 fois
    HalfMixY*2;    // 2 fois

  // Alveolus
  G4Box *AlveolusSolid  = new G4Box("AlveolusSolid",
				    HalfAlveolusX,
				    HalfAlveolusY,
				    HalfAlveolusZ);
  
  aPlate->AsAlveolusLogical=
    new G4LogicalVolume(AlveolusSolid,
			Mix,
			"AlveolusLogical", 
			0, 
			0, 
			0);
  
  aPlate->AsAlveolusLogical->SetVisAttributes(VisAttAlv);
  
  //-----------------------------------------------------------
  // TotalHalfY = HalfAlveolusY 
  //-----------------------------------------------------------
  aPlate->AsAlveolusTotalHalfY=HalfAlveolusY;

  //-----------------------------------------
  // WSlab
  //-----------------------------------------

  new G4PVPlacement(0,
		    G4ThreeVector(0., 0., 0.),
		    aPlate->AsSlabWLogical,
		    "WSlabPhys",
		    aPlate->AsAlveolusLogical,
		    false,0);
  
  G4double HalfWSlabY =aPlate->AsSlabWTotalHalfY;
  
  //-------------------------
  // Placement des waffers
  //-------------------------
  G4String WafferName("WafferPhysical");

  G4double VerticalDisp = HalfWSlabY+HalfWafferY;
  G4double XDisp=0,XLocalDisp=0,
    XInterElementDisp=0,ZDisp=0,Wafer_shift=0;

  Wafer_shift=db->fetchDouble("wafer_shift");

  XDisp= (db->fetchInt("n_elements_x")-1)*
    (HalfSuppX+db->fetchDouble("inter_element_gap")/2)-
    db->fetchDouble("inter_waffer_gap")/2;

  G4int multiplicity_waffers = db->fetchInt("multiplicity_waffers");
  if(multiplicity_waffers>2)
    {
      G4cout 
	<< "multiplicity_waffers>2 not possible in this release!!!!!!!" 
	<< G4endl;
      exit(1);
    }
  for(G4int j=0;j<db->fetchInt("n_elements_x");j++)
    {
      XInterElementDisp=
	HalfWafferX*multiplicity_waffers+
	(multiplicity_waffers-1)*db->fetchDouble("inter_waffer_gap")+
	db->fetchDouble("inter_waffer_gap")/2;
	
      for(G4int k=0;k<2;k++)
	{
	  XLocalDisp=
	    (db->fetchInt("multiplicity_waffers")-1)*
	    (HalfWafferX+db->fetchDouble("inter_waffer_gap")/2);
	  for(G4int i2=0;i2<db->fetchInt("multiplicity_waffers");i2++)
	    {
	      ZDisp = 
		(db->fetchInt("multiplicity_waffers")-1)*
		(HalfWafferZ+db->fetchDouble("inter_waffer_gap")/2);
	      
	      for(G4int i=0;i<db->fetchInt("multiplicity_waffers");i++)
		{
		  G4int CopyNumber=10000+j*1000 + k*100 + i2*10 + i;
		  new G4PVPlacement(0,
				    G4ThreeVector(Wafer_shift+
						  XDisp+XInterElementDisp+
						  XLocalDisp, 
						  VerticalDisp, 
						  ZDisp),
				    WafferLogical,
				    WafferName,
				    aPlate->AsAlveolusLogical,
				    false,CopyNumber);
		  new G4PVPlacement(0,
				    G4ThreeVector(-Wafer_shift+
						  XDisp+XInterElementDisp+
						  XLocalDisp, 
						  -VerticalDisp, 
						  ZDisp),
				    WafferLogical,
				    WafferName,
				    aPlate->AsAlveolusLogical,
				    false,-CopyNumber);
		  ZDisp-= 2*(HalfWafferZ+db->fetchDouble("inter_waffer_gap")/2);  
		}
	      XLocalDisp-=2*(HalfWafferX+db->fetchDouble("inter_waffer_gap")/2);
	    }
	  XInterElementDisp-=2*(HalfWafferX*multiplicity_waffers+
				db->fetchDouble("inter_waffer_gap"));
	}
      XDisp-=2*(HalfSuppX+db->fetchDouble("inter_element_gap")/2);
    }
  
  
  //--------------------------------
  // Placement des 2 plaques de Cu
  //--------------------------------
  VerticalDisp+=HalfCuY+HalfWafferY;
  
  new G4PVPlacement(0,
		    G4ThreeVector(0.,VerticalDisp, 0.),
		    CuLogical,
		    "CuPhys",
		    aPlate->AsAlveolusLogical,
		    false,0);
  new G4PVPlacement(0,
		    G4ThreeVector(0.,-VerticalDisp, 0.),
		    CuLogical,
		    "CuPhys",
		    aPlate->AsAlveolusLogical,
		    false,0);
}
