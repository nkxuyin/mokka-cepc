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
// $Id: Proto00.cc,v 1.3 2005/04/08 14:37:15 musat Exp $
// $Name: mokka-07-00 $
//
//
// Proto00.cc
//
// History:  

#include "G4PVPlacement.hh"
#include "G4Material.hh"

#include "Proto00.hh"
#include "CGAGeometryManager.hh"

#include "globals.hh"
#include "G4Box.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"

#include "MySQLWrapper.hh"
#include "Control.hh"

#include "ProtoSD.hh"

#include "CGADefs.h"

INSTANTIATE(Proto00)

G4bool Proto00::construct(const G4String &aSubDetectorName,
			  G4LogicalVolume *WorldLog)

{
  G4cout << "\nBuilding Proto..." << G4endl;
  db = new Database(aSubDetectorName.data());
  
  // BuildElements takes the main parameters and builds 
  // the basic logical volumes
  BuildElements();
  
  G4int i_plate=0;
  for (unsigned int i_group=0;i_group<PlateGroups.size();i_group++)
    {
      BuildAlveolus(PlateGroups[i_group]);
      BuildDeadPlate(PlateGroups[i_group]);
      for (i_plate=0;i_plate<PlateGroups[i_group]->n_plates;i_plate++)
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

  HalfDetectorY+=fiber_thickness;


  // on deborde les plaques en W mortes
  n_dead_w_plates = 
    int(2*(HalfAlveolusX+HalfDetectorY*tan_rate) /
	(db->fetchDouble("dead_w_dimx")+2*fiber_thickness))+1;
  G4double HalfDetectorX=
    n_dead_w_plates*(HalfDeadWX+fiber_thickness);
  
  G4double HalfDetectorZ= // plus 1 mm de chaque cote
    n_towers*(HalfAlveolusZ+inter_tower_fiber_thickness)+1.;
  
  G4cout << "HalfDetectorX= "
	 << HalfDetectorX
	 << ", HalfDetectorY= "
	 << HalfDetectorY
	 << ", HalfDetectorZ= "
	 << HalfDetectorZ
	 << G4endl;

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
  // Global World:
  //--------------
//   G4Box *WorldBox= new G4Box("WorldBox",
// 			     4*HalfDetectorX,
// 			     4*HalfDetectorY,
// 			     4*HalfDetectorZ);
  
//   WorldLog=new G4LogicalVolume(WorldBox,
// 			       CGAGeometryManager::GetMaterial("air"),
// 			       "WorldLogical", 0, 0, 0);

//   WorldPhys=new G4PVPlacement(0,G4ThreeVector(),
// 			      "ProtoWorldPhysical",
// 			      WorldLog,
// 			      0,false,0);
//   G4VisAttributes * experimantalHallVisAtt
//     = new G4VisAttributes(G4Colour(0.0,0.0,0.0));
//   experimantalHallVisAtt->SetVisibility(false);
//   WorldLog->SetVisAttributes(experimantalHallVisAtt);

  //--------------
  // Detector Placement
  //--------------
  new G4PVPlacement(0,
		    G4ThreeVector(0.,2*HalfDetectorY , 0.),
		    DetectorLogical,
		    "DetectorPhys",
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
      G4double DispY=-HalfDetectorY+fiber_thickness;  
      for(i_plate=0;i_plate<total_W_plates;i_plate++)
	{
	  if(i_plate%2==0)
	    {
	      NextLayer=Plates[i_plate]->AsDeadWLogical;
	      DispY+=DeltaY=Plates[i_plate]->AsDeadTotalHalfY;
	      DispZ=0;
	      CopyNumber=0;
	      G4double DeadDispX=0;
	      if(n_dead_w_plates%2==0)
		DeadDispX = fiber_thickness+HalfDeadWX+
		  (n_dead_w_plates/2-1)*2.*(HalfDeadWX+fiber_thickness);
	      else
		if(n_dead_w_plates>1)
		  DeadDispX = n_dead_w_plates/2*(2*HalfDeadWX+2*fiber_thickness);
	      for(G4int i_dead_plate=0;i_dead_plate<n_dead_w_plates;i_dead_plate++)
		{
		  new G4PVPlacement(0,
				      G4ThreeVector(DeadDispX, DispY, DispZ),
				      NextLayer,
				      "LayerPhys",
				      DetectorLogical,
				      false,CopyNumber);
		  DeadDispX-=2*(HalfDeadWX+fiber_thickness);
		}
	    }
	  else
	    {
	      NextLayer=Plates[i_plate]->AsAlveolusLogical;
	      DispY+=DeltaY=Plates[i_plate]->AsAlveolusTotalHalfY;
	      DispZ=DispZTower;
	      CopyNumber= i_tower*100 + i_plate;
	      new G4PVPlacement(0,
				G4ThreeVector(DispY*tan_rate, DispY, DispZ),
				NextLayer,
				"LayerPhys",
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
Proto00::~Proto00()
{
//   if(theProtoSD!=0) delete theProtoSD;
}

void Proto00::BuildElements() 
{
  G4VisAttributes * VisAtt;
  
  //---------------------------------
  // Prend la config de plaques
  //---------------------------------
  total_W_plates = 0;
  db->exec("select * from w_plates;");
  while (db->getTuple())
    {
      G4int n_plates = db->fetchInt("n_plates");
      G4double w_thickness = db->fetchDouble("w_thickness");
      PlateGroups.push_back(new PLATEGROUP(n_plates,
					   w_thickness));
      total_W_plates+=n_plates;
    }
  G4cout << "total_W_plates = " << total_W_plates << G4endl;
  
  //---------------------------
  // Waffer, partie Sensitive
  //---------------------------
  db->exec("select * from proto;");
  db->getTuple();

  G4double HalfSensitiveWafferX = 
    db->fetchDouble("cell_dim_x")/2*db->fetchInt("n_cell_x");
  G4double HalfSensitiveWafferY = 
    db->fetchDouble("si_thickness")/2;
  G4double HalfSensitiveWafferZ = 
    db->fetchDouble("cell_dim_z")/2*db->fetchInt("n_cell_z");
  
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
  
  G4double HalfSuppZ = 
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
  
  G4LogicalVolume *SensWafferLogical=
    new G4LogicalVolume(SensWafferSolid,
			CGAGeometryManager::GetMaterial("silicon_2.33gccm"),
			"SensWafferLogical", 
			0, 
			0, 
			0);

  VisAtt = new G4VisAttributes(G4Colour(1.,1.,1.));
  VisAtt->SetForceWireframe(true);
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

  //-----------------------------------
  // Dead W Plates common parameters
  //-----------------------------------
  n_towers=db->fetchInt("n_towers");

  HalfDeadWX = db->fetchDouble("dead_w_dimx")/2.;
  
  HalfDeadWZ=
    (n_towers-1)*
    (HalfAlveolusZ+inter_tower_fiber_thickness/2)+
    HalfAlveolusZ;
}

void Proto00::BuildDeadPlate(PLATEGROUP* aPlate)
{
  G4VisAttributes * VisAttDeadW = new G4VisAttributes(G4Colour(1.,0.2,1.));
  VisAttDeadW->SetForceWireframe(true);
  
  //-----------------------------------
  // Logical type WPlate
  //-----------------------------------
  G4double HalfDeadWY =
    aPlate->W_thickness/2;    // 1 fois

  // DeadW
  G4Box *DeadWSolid  = new G4Box("DeadWSolid",
				    HalfDeadWX,
				    HalfDeadWY,
				    HalfDeadWZ);
  
  aPlate->AsDeadWLogical=
    new G4LogicalVolume(DeadWSolid,
			CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
			"DeadWLogical", 
			0, 
			0, 
			0);
  
  aPlate->AsDeadWLogical->SetVisAttributes(VisAttDeadW);
  
  //-----------------------------------------------------------
  // TotalHalfY = HalfDeadWY + fibre 2 fois (1 chaque cote)
  //-----------------------------------------------------------
  aPlate->AsDeadTotalHalfY=HalfDeadWY + fiber_thickness;
}


void Proto00::BuildAlveolus(PLATEGROUP* aPlate)
{

  //------------------------------------
  // VisAttributes pour Alevolus et Slab
  //------------------------------------
  G4VisAttributes * VisAttAlv = new G4VisAttributes(G4Colour(1.,1.,0.2));
  VisAttAlv->SetForceWireframe(true);
  G4VisAttributes * VisAttWSlab = new G4VisAttributes(G4Colour(1.,0.2,1.));
  VisAttWSlab->SetForceWireframe(true);
  
  db->exec("select * from proto;");
  db->getTuple();

  //-----------------------------------
  // Logical type Alveolus
  //-----------------------------------
  G4double HalfAlveolusY =
    aPlate->W_thickness/2+    // 1 fois
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
  // TotalHalfY = HalfAlveolusY + fibre 2 fois (1 chaque cote)
  //-----------------------------------------------------------
  aPlate->AsAlveolusTotalHalfY=HalfAlveolusY + fiber_thickness;

  //-----------------------------------------
  // WSlab
  //-----------------------------------------

  G4double HalfWSlabY = aPlate->W_thickness/2;
  G4Box *WSlabSolid  = new G4Box("WSlabSolid",
				 HalfWSlabX,
				 HalfWSlabY,
				 HalfWSlabZ);
  
  G4LogicalVolume * WSlabLogical=
    new G4LogicalVolume(WSlabSolid,
			CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
			"WSlabLogical", 
			0, 
			0, 
			0);
  WSlabLogical->SetVisAttributes(VisAttWSlab);
  
  new G4PVPlacement(0,
		    G4ThreeVector(0., 0., 0.),
		    WSlabLogical,
		    "WSlabPhys",
		    aPlate->AsAlveolusLogical,
		    false,0);
  
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
