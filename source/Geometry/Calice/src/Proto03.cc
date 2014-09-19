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
// $Id: Proto03.cc,v 1.22 2009/02/11 13:32:46 musat Exp $
// $Name: mokka-07-00 $
//
//
// Proto03.cc
//
// History:  

#include "MyPlacement.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4Material.hh"
#include "G4SubtractionSolid.hh"

#include "Proto03.hh"
#include "CGAGeometryManager.hh"

#include "globals.hh"
#include "G4GeometryTolerance.hh"
#include "G4Box.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4UserLimits.hh"
#include "MySQLWrapper.hh"
#include "Control.hh"

#include "G4Region.hh"
#include "ProtoSD03.hh"
#include "TRKSD00.hh"

#include <algorithm>

#include "CGADefs.h"

INSTANTIATE(Proto03)

G4bool Proto03::ContextualConstruct(
		const CGAGeometryEnvironment &aGeometryEnvironment,
		G4LogicalVolume *theWorld) {

       theG10MaterialName =
           aGeometryEnvironment.GetParameterAsString("g10MaterialName");

	config_angle = aGeometryEnvironment.GetParameterAsDouble("configuration_angle");

	G4String trkUse = aGeometryEnvironment.GetParameterAsString("use_tracker");

	if(trkUse == "true")
		useTracker = true;
	else
		useTracker = false;

	theCellProtoSD = 0;
	theGRProtoSD = 0;
	theTRKSD = 0;
	return construct (aGeometryEnvironment.GetDBName(),theWorld);
}
	
G4bool Proto03::construct(const G4String &aSubDetectorName,
			  G4LogicalVolume *WorldLog)

{
  G4cout << "\nBuilding Proto release 03" << G4endl;
  this->WorldLog = WorldLog;
  PlateGroups.clear();
  Plates.clear();
  theSubDetectorName = aSubDetectorName;

  G10Region = new G4Region("PCB");

  WRegion = new G4Region("Radiator");

  db = new Database(aSubDetectorName.data());
  
  if(Control::DUMPG3) MyPlacement::Init("PROTO",aSubDetectorName);

  DefineMaterial();

  // BuildElements takes the main parameters and builds 
  // the basic logical volumes
  BuildElements();
  
  G4bool useID1 = true;
  if(((2*(HalfWafferX-garde_size)/cell_dim_x) <= 511) &&
  ((2*(HalfWafferZ-garde_size)/cell_dim_z) <= 511))
	useID1 = false;

  G4int i_plate=0;
  
  for (unsigned int i_group=0;i_group<PlateGroups.size();i_group++)
    {
      // to be replaced in the future by the real w thicknesses for Plates[i]
      PlateGroups[i_group]->real_w_thickness = 
	      PlateGroups[i_group]->nominal_w_thickness;
      BuildDeadPlate(PlateGroups[i_group]);
      for (i_plate=0;i_plate<PlateGroups[i_group]->n_layers;i_plate++)
	Plates.push_back(new WLAYERS(*PlateGroups[i_group]));
    }
  
  db->exec("select count(*) AS N_SLABS from slabs where position = 'center';");
  db->getTuple();
  unsigned int n_slabs = db->fetchInt("N_SLABS");

  if(n_slabs != Plates.size()) {
	  G4cout << "Mismatch between w_layers table and slabs table\n" 
		  << "concerning the number of slabs. Exiting."
		  << G4endl;
	  exit(1);
  }

  i_plate=0;
  db->exec("select slab_shift, waffer_shift from slabs where position ='center' order by id;");
  while (db->getTuple()) {
      Plates[i_plate]->center_slab_shift = db->fetchDouble("slab_shift");
      Plates[i_plate]->center_waffer_shift = db->fetchDouble("waffer_shift");
      i_plate++;
  }

  i_plate=0;
  db->exec("select slab_shift, waffer_shift from slabs where position ='right' order by id;");
  while (db->getTuple()) {
      Plates[i_plate]->right_slab_shift = db->fetchDouble("slab_shift");
      Plates[i_plate]->right_waffer_shift = db->fetchDouble("waffer_shift");
      i_plate++;
  }

  CalculateSlabShifts();

  for (unsigned int i=0; i<Plates.size();i++) {
      BuildAlveola(Plates[i]);
  }
  for (unsigned int i=0; i<Plates.size();i++) {
      std::pair<G4double, G4double> element(
		      Plates[i]->center_slab_shift + endcap_x,
		      Plates[i]->center_waffer_shift);
      slab_shifts_vector.push_back(element);
  }
  for (unsigned int i=0; i<Plates.size();i++) {
      std::pair<G4double, G4double> element(
		      Plates[i]->right_slab_shift + endcap_x,
		      Plates[i]->right_waffer_shift);
      slab_shifts_vector.push_back(element);
  }
  //--------------
  // Detector:
  //--------------

  HalfEcalY = 0;
  for(unsigned int i = 0; i < Plates.size(); i++) {
      HalfEcalY+=Plates[i]->nominal_w_thickness;
      HalfEcalY+=2*deadw_fiber_thickness;
      HalfEcalY+=Plates[i]->AsAlveolusTotalHalfY * 2 + al_cf_y_gap;
  }

  HalfEcalY+=exit_fiber_thickness * PlateGroups.size();
  HalfEcalY+=inter_structures_gap * (PlateGroups.size() - 1);
  HalfEcalY /= 2;
  HalfEcalY+=trkHalfY;

#ifdef MOKKA_DEBUG
  if (HalfEcalX <= HalfDeadWX * 3) {
	G4cout << "======= ASSERT WILL CRASH :\n"
		<< " X dimension of Ecal is not big enough\n"
		<< " to contain the dead W plates and carbon fiber\n"
		<< " needed minimum X dimension = " 
		<< HalfDeadWX * 6
		<< " DB X dimension of Ecal = "
		<< HalfEcalX * 2 << G4endl;
        Control::Abort("Proto03::construct: Assertion failed (HalfEcalX <= HalfDeadWX * 3)",MOKKA_OTHER_ERRORS);
  }
#endif

  HalfEcalZ = n_towers * (2 * HalfAlveolusZ + al_cf_z_gap) +
  	inter_tower_fiber_thickness * (n_towers - 1) +
	2 * lateral_fiber_thickness;

  HalfEcalZ /= 2;
  
#ifdef MOKKA_DEBUG
  if (HalfDeadWZ  > HalfEcalZ) {
	G4cout << "======= ASSERT WILL CRASH :\n"
		<< " Y dimension of Ecal is less than\n"
		<< " Y dimension of dead w\n"
		<< G4endl;
        Control::Abort("Proto03::construct: Assertion failed (HalfDeadWZ  > HalfEcalZ)",MOKKA_OTHER_ERRORS);
  }
#endif

  BuildStructures();

  if(c_angle_from_steering){
    struct_shift[0] += (struct_shift[1] + struct_shift[2]);
    struct_shift[1] += struct_shift[2];
  }

  G4double max = 0;
  for(unsigned int iShift = 0; iShift < theStructuresVector.size(); iShift++) {
	G4LogicalVolume *StructureLogical = theStructuresVector[iShift].first;
	G4double HalfStructureX = ((G4Box*)(StructureLogical->GetSolid()))->
		GetXHalfLength();
	StructHalfX[iShift] = HalfStructureX;
	if(max < fabs(struct_shift[iShift]) + HalfStructureX)
		max = fabs(struct_shift[iShift]) + HalfStructureX;
  }
  
  G4Box *DetectorSolid= new G4Box("DetectorBox",
		  			max, 
					HalfEcalY, 
					HalfEcalZ);
  
  //>>>>>>>>>>>>>>>>>> VER MATERIAL !!!!!!!!!!!!
  DetectorLogical=
    new G4LogicalVolume(DetectorSolid,
			CGAGeometryManager::GetMaterial("air"),
			"DetectorLogical", 
			0, 
			0, 
			0);

  DetectorLogical->SetVisAttributes(VisAttAir);
  
  G4double YPlacement = 0;
  for(unsigned int i_group=0; i_group<theStructuresVector.size(); i_group++){
    
    G4LogicalVolume * StructureLogical = theStructuresVector[i_group].first;
    
    G4double HalfStructureY = theStructuresVector[i_group].second;
    
    new MyPlacement(0,
		    G4ThreeVector(-max + StructHalfX[i_group] +
			    fabs(struct_shift[0]) +
			    struct_shift[i_group], 
		      - HalfEcalY + YPlacement + HalfStructureY, 0),
		    StructureLogical,
		    "EnvStructurePhys",
		    DetectorLogical,
		    false,i_group);

    YPlacement += HalfStructureY * 2 + inter_structures_gap;
  }

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
      Control::Log("proto03 Warning: old database release without (x_center,y_center,z_center) values, assuming (0.,0., 2*HalfEcalZ).");
      z_center = 2*HalfEcalY;
    }

  EcalPosition = G4ThreeVector(x_center, y_center, z_center);

  EcalRotation = new G4RotationMatrix();
  EcalRotation->rotateX(-pi*0.5);

  G4ThreeVector displacement(max-HalfEcalX-fabs(struct_shift[0]), HalfEcalY, 
		  HalfWafferZ + inter_waffer_gap / 2); 
  displacement = (*EcalRotation) * displacement;
  EcalPosition += displacement;

  start_layer_number=0;

  db->exec("select start_layer_number from proto;");
  db->getTuple();
  start_layer_number = db->fetchInt("start_layer_number");
  
  // sensitive detector plugin
  //  G4String theSDName = G4String("ProtoSD") + theSubDetectorName.data();
  G4String theCellSDName = G4String("ProtoSD03");
  G4String theGRSDName = G4String("ProtoSD03GuardRing");

  if(theCellProtoSD==0)
    {
      theCellProtoSD = new
	ProtoSD03(cell_dim_x, cell_dim_z, n_cell_x, n_cell_z, 
		n_waffers_x, n_waffers_z + 1, 
		upper_waffer_shift + wafer_x_shift,
		garde_size, cell_y_pos_in_alveolus, alveolus_y_spacing,
		exit_fiber_thickness,
		slab_shifts_vector,// waffer et slab shifts
		struct_shift,StructHalfX,
		&EcalPosition, EcalRotation,
		theCellSDName, start_layer_number,
		inter_waffer_gap, HalfAlveolusX,
		max, HalfEcalX, HalfEcalY, 
		inter_tower_fiber_thickness + al_cf_z_gap,
		inter_structures_gap, n_guard_ring_zones,
		lateralWaferGap, endcap_x, useID1);
      RegisterSensitiveDetector(theCellProtoSD);
    }

  siCellLogical->SetSensitiveDetector(theCellProtoSD);

  if(theGRProtoSD==0)
    {
      theGRProtoSD = new
	ProtoSD03(cell_dim_x, cell_dim_z, n_cell_x, n_cell_z, 
		n_waffers_x, n_waffers_z + 1, 
		upper_waffer_shift + wafer_x_shift,
		garde_size, cell_y_pos_in_alveolus, alveolus_y_spacing,
		exit_fiber_thickness,
		slab_shifts_vector,// waffer et slab shifts
		struct_shift,StructHalfX,
		&EcalPosition, EcalRotation,
		theGRSDName, start_layer_number,
		inter_waffer_gap, HalfAlveolusX,
		max, HalfEcalX, HalfEcalY, 
		inter_tower_fiber_thickness + al_cf_z_gap,
		inter_structures_gap, n_guard_ring_zones,
		lateralWaferGap, endcap_x,true);
      RegisterSensitiveDetector(theGRProtoSD);
    }

  SiWafferLogical->SetSensitiveDetector(theGRProtoSD); // for the guard-ring


  if((useTracker) && (theTRKSD == 0)){
	theTRKSD = new TRKSD00("ProtoTRKSD03", 0, Control::TPCCut);
	RegisterSensitiveDetector(theTRKSD);
	TrackerLogical->SetSensitiveDetector(theTRKSD);
  }

  //--------------
  // Detector Placement
  //--------------
  char buff[80];
  sprintf(buff,"proto03: proto size is (%f,%f,%f) mm",2*HalfEcalX,
	  2*HalfEcalZ,2*HalfEcalY);
  Control::Log(buff);
  sprintf(buff,"proto03: placing prototype at (%f,%f,%f) mm",
		  EcalPosition(0), EcalPosition(1), EcalPosition(2));
  Control::Log(buff);
  
  new MyPlacement(EcalRotation,
  		    EcalPosition,
		    DetectorLogical,
		    "DetectorPhys",
		    WorldLog,
		    false,0);

  CGAGeometryManager::GetCGAGeometryManager()->
	RegisterGeometryRegion(G10Region, Control::PCBRangeCut);

  CGAGeometryManager::GetCGAGeometryManager()->
	RegisterGeometryRegion(WRegion, Control::RadiatorRangeCut);

  // Closes Database connection
  delete db;
  db = 0;
  G4cout << "Proto done.\n" << G4endl;
  return true;
}

void Proto03::BuildStructures(void) {

unsigned int i_plate1=0, i_plate2=0;
G4double YPlacement = 0;
for (unsigned int i_group=0;i_group<PlateGroups.size();i_group++) {

  G4double layerThickness = Plates[i_plate2]->nominal_w_thickness +
	  2*deadw_fiber_thickness +
	  Plates[i_plate2]->AsAlveolusTotalHalfY * 2 +
	  al_cf_y_gap;
  std::pair<G4double, G4double> element(layerThickness, 
		  Plates[i_plate2]->AsAlveolusTotalHalfY * 2 +
		  al_cf_y_gap);
  alveolus_y_spacing.push_back(element);
  cell_y_pos_in_alveolus.push_back(Plates[i_plate2]->cell_y_pos_in_alveolus);

  G4double HalfStructureY = 0;

  for (i_plate1=i_plate2 ; 
		  i_plate1< i_plate2 + 
		  (unsigned int)(PlateGroups[i_group]->n_layers); 
 		  i_plate1++) {
      HalfStructureY+=Plates[i_plate1]->nominal_w_thickness;
      HalfStructureY+=2*deadw_fiber_thickness;
      HalfStructureY+=Plates[i_plate1]->AsAlveolusTotalHalfY * 2 
	      + al_cf_y_gap;
  }
  HalfStructureY+=exit_fiber_thickness;
  HalfStructureY /= 2;

  G4Box *StructureBox= new G4Box("StructureSolid",
		  			HalfEcalX, 
					HalfStructureY, 
					HalfEcalZ);
  
  G4VSolid * StructureSolid = static_cast<G4VSolid*>(StructureBox);

  for (i_plate1=i_plate2 ; 
		  i_plate1< i_plate2 + 
		  (unsigned int)(PlateGroups[i_group]->n_layers); 
 		  i_plate1++) {

	  G4double YDisp = - HalfStructureY + 
		 Plates[i_plate1]->nominal_w_thickness +
      		 deadw_fiber_thickness * 2 +
		 Plates[i_plate1]->AsAlveolusTotalHalfY + al_cf_y_gap /2 +
		 (i_plate1 - i_plate2) 
		 *(Plates[i_plate1]->nominal_w_thickness +
      		 deadw_fiber_thickness * 2 +
		 Plates[i_plate1]->AsAlveolusTotalHalfY * 2 + al_cf_y_gap);

	  G4ThreeVector position(0, YDisp,
		      - HalfEcalZ + HalfAlveolusZ + lateral_fiber_thickness +
		      al_cf_z_gap / 2);

	  G4Box * volumeToSubtract = (G4Box*)(Plates[i_plate1]->
			  AsLeftAlveolusLogical->GetSolid());

	  G4SubtractionSolid * newSolid = 
		  new G4SubtractionSolid("Struct-Left",
				  StructureSolid,
				  volumeToSubtract,
				  0,
				  position);

	  position = G4ThreeVector(0, YDisp,
		      - HalfEcalZ + 3*(HalfAlveolusZ + al_cf_z_gap/2) +
		      lateral_fiber_thickness + inter_tower_fiber_thickness);

	  StructureSolid = static_cast<G4VSolid*>(newSolid);
	  newSolid = new G4SubtractionSolid("Struct-Center",
			  StructureSolid,
			  volumeToSubtract,
			  0,
			  position);

	  position = G4ThreeVector(0, YDisp,
		      HalfEcalZ - HalfAlveolusZ - lateral_fiber_thickness -
		      al_cf_z_gap/2);
	  
	  StructureSolid = static_cast<G4VSolid*>(newSolid);
	  newSolid = new G4SubtractionSolid("Struct-Right",
			  StructureSolid,
			  volumeToSubtract,
			  0,
			  position);

	  StructureSolid = static_cast<G4VSolid*>(newSolid);
  }

  G4LogicalVolume * StructureLogical =
    new G4LogicalVolume(StructureSolid,
			CarbonFiber,
			"StructureLogical", 
			0, 
			0, 
			0);
  
  StructureLogical->SetVisAttributes(VisAttCF);

  G4double HalfEnvAlveolusX = HalfAlveolusX + endcap_x/2 + g10_x_out/2;
  G4double HalfEnvStructX = HalfEnvAlveolusX+
	  PlateGroups[i_group]->maxSlabShift/2;

  trkHalfY = 0;
  if(useTracker && (i_group == 0)) trkHalfY = 1000*
	G4GeometryTolerance::GetInstance()->GetSurfaceTolerance();
		
  G4Box *EnvStructureBox= new G4Box("EnvStructureSolid",
		  		  HalfEnvStructX,
			    	  HalfStructureY + trkHalfY, 
				  HalfEcalZ);

  G4LogicalVolume * EnvStructureLogical =
	  new G4LogicalVolume(EnvStructureBox,
			  CGAGeometryManager::GetMaterial("air"),
			  "EnvStructureLogical",
			  0,
			  0,
			  0);

  EnvStructureLogical->SetVisAttributes(VisAttAir);

#ifdef MOKKA_DEBUG
    if (HalfEnvStructX < HalfEcalX) {
	        G4cout << "======= ASSERT WILL CRASH :\n" 
		<< " Structure " << i_group 
		<< " doesn't fit inside enveloppe\n"
		<< " in the X direction\n" << G4endl;
        Control::Abort("Proto03::BuildStructures: Assertion failed (HalfEnvStructX < HalfEcalX)",MOKKA_OTHER_ERRORS);
    }
#endif

  if((useTracker) && (i_group == 0)) {
  	G4Box * trkSolid = new G4Box("Proto03TRKSolid",
					HalfEcalX,
					trkHalfY,
					HalfEcalZ);

  	TrackerLogical = new G4LogicalVolume(trkSolid,
			  CGAGeometryManager::GetMaterial("air"),
			  "Proto03TRKLogical",
			  0,
			  0,
			  0);

  	TrackerLogical->SetVisAttributes(VisAttAir);

  	new MyPlacement(0,
		    G4ThreeVector(-HalfEnvStructX + HalfEcalX, 
						-HalfStructureY, 0),
		    TrackerLogical,
		    "Proto03TRK_P",
		    EnvStructureLogical,
		    false,1);
  }

  new MyPlacement(0,
		    G4ThreeVector(-HalfEnvStructX + HalfEcalX, trkHalfY, 0),
		    StructureLogical,
		    "Structure_P",
		    EnvStructureLogical,
		    false,i_group);

  std::pair<G4LogicalVolume *, G4double> 
	  anElement(EnvStructureLogical,HalfStructureY+trkHalfY);
  theStructuresVector.push_back(anElement);

  if(c_angle_from_steering)
    struct_shift[i_group] = -(2*HalfStructureY) * tan(config_angle*deg);

  for ( i_plate1 = i_plate2; 
	i_plate1 < i_plate2 + (unsigned int)(PlateGroups[i_group]->n_layers); 
		  i_plate1++) {

	G4double YDisp = -HalfStructureY + 
		      	    Plates[i_plate1]->nominal_w_thickness / 2 +
      			    deadw_fiber_thickness +
			    (i_plate1 - i_plate2) 
			    *(Plates[i_plate1]->nominal_w_thickness +
			    2*deadw_fiber_thickness + 
			    Plates[i_plate1]->AsAlveolusTotalHalfY * 2
			    + al_cf_y_gap);
  	new MyPlacement(0,
		    G4ThreeVector(-HalfEcalX + HalfDeadWX + 
			    front_rear_fiber_thickness, 
			    YDisp, 0),				
		    Plates[i_plate1]->AsLeftDeadWLogical,
		    "LeftDeadWPhys",
		    StructureLogical,
		    false,0);

  	new MyPlacement(0,
		    G4ThreeVector(-HalfEcalX + 3*HalfDeadWX + 
			    front_rear_fiber_thickness +
			    inter_deadw_fiber_thickness, 
			    YDisp, 0),				
		    Plates[i_plate1]->AsCenterDeadWLogical,
		    "CenterDeadWPhys",
		    StructureLogical,
		    false,0);

  	new MyPlacement(0,
		    G4ThreeVector(HalfEcalX - HalfDeadWX - 
			    front_rear_fiber_thickness, 
			    YDisp, 0),				
		    Plates[i_plate1]->AsRightDeadWLogical,
		    "RightDeadWPhys",
		    StructureLogical,
		    false,0);

	YDisp = - HalfStructureY + 
		 Plates[i_plate1]->nominal_w_thickness +
      		 deadw_fiber_thickness * 2 +
		 Plates[i_plate1]->AsAlveolusTotalHalfY + al_cf_y_gap /2 +
		 (i_plate1 - i_plate2) 
		 *(Plates[i_plate1]->nominal_w_thickness +
      		 deadw_fiber_thickness * 2 +
		 Plates[i_plate1]->AsAlveolusTotalHalfY * 2 + al_cf_y_gap);

	G4double xSlab = ((G4Box*)(Plates[i_plate1]->AsCenterAlveolusLogical->
			GetSolid()))->GetXHalfLength();

	G4double xDisp = -HalfEnvStructX + xSlab + 
		Plates[i_plate1]->center_slab_shift;
	
  	new MyPlacement(0,
		    G4ThreeVector(xDisp, YDisp,
		      - HalfEcalZ + 3*(HalfAlveolusZ + al_cf_z_gap/2) +
		      lateral_fiber_thickness + inter_tower_fiber_thickness),
		    Plates[i_plate1]->AsCenterAlveolusLogical,
		    "CenterAlveolus",
		    EnvStructureLogical,
		    false,100 * i_group + i_plate1 + 1);

	xSlab = ((G4Box*)(Plates[i_plate1]->AsRightAlveolusLogical->
			GetSolid()))->GetXHalfLength();

	xDisp = -HalfEnvStructX + xSlab + 
		Plates[i_plate1]->right_slab_shift;
	
  	new MyPlacement(0,
		    G4ThreeVector(xDisp, YDisp,
		      HalfEcalZ - HalfAlveolusZ - lateral_fiber_thickness -
		      al_cf_z_gap/2),
		    Plates[i_plate1]->AsRightAlveolusLogical,
		    "RightAlveolus",
		    EnvStructureLogical,
		    false,100 * i_group + i_plate1 + 1);
  }
  i_plate2 = i_plate1;
  YPlacement += HalfStructureY * 2 + inter_structures_gap;
 }
}

void Proto03::DefineMaterial(void) {
	
	G4double density;
	G4String name, symbol;
	G4int nel;
	G4double fractionmass, volumefraction;

	volumefraction = 0.5;
	density = (1.3 + volumefraction / 3 ) * g/cm3;
        fractionmass = 1 - 1.3 * (1 - volumefraction) / (density / (g/cm3));


	CarbonFiber = new G4Material(name="CarbonFiber", density, nel=2);
	CarbonFiber->AddElement(CGAGeometryManager::GetElement("C"), fractionmass);
	CarbonFiber->AddMaterial(CGAGeometryManager::GetMaterial("epoxy"), 1 - fractionmass);
	G4cout << "CarbonFiber->GetRadlen() = " 
		<< CarbonFiber->GetRadlen() /mm   << " mm\n";
}

Proto03::~Proto03()
{
//   if(THEpROTOsD!=0) delete theProtoSD;
}

void Proto03::BuildElements() 
{
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
			      db->fetchDouble("nominal_w_thickness")));
      G4cout << "w_thickness = " << db->fetchDouble("nominal_w_thickness")
	     << G4endl;
      total_W_plates+=n_layers;
    }
  G4cout << "total_W_layers = " << total_W_plates << G4endl;
  
  //---------------------------------
  // Take general parameters
  //---------------------------------
  db->exec("select * from carbon_fiber;");
  db->getTuple();
  
  inter_deadw_fiber_thickness = 
	  db->fetchDouble("inter_deadw_fiber_thickness");

  front_rear_fiber_thickness = 
	  db->fetchDouble("deadw_outer_fiber_thickness");

  waffer_fiber_vertical_thickness = 
	  db->fetchDouble("waffer_fiber_z_thickness");

  waffer_fiber_lateral_thickness = 
	  db->fetchDouble("waffer_fiber_y_thickness");

  deadw_fiber_thickness = 
	  db->fetchDouble("deadw_fiber_thickness");

  exit_fiber_thickness = 
	  db->fetchDouble("exit_fiber_thickness");

  lateral_fiber_thickness=
    db->fetchDouble("y_fiber_thickness");

  inter_tower_fiber_thickness=
    db->fetchDouble("inter_tower_fiber_thickness");

  db->exec("select * from proto;");
  db->getTuple();

  struct_shift[0] = 
	  db->fetchDouble("struct1_shift");

  struct_shift[1] = 
	  db->fetchDouble("struct2_shift");
  
  G4String flag = db->fetchString("c_angle_from_steering");
  if(flag == "true")
    c_angle_from_steering = true;
  else
    c_angle_from_steering = false;
  
  struct_shift[2] = 0.;

  al_cf_z_gap = 
	  db->fetchDouble("al_cf_y_gap");

  al_cf_y_gap = 
	  db->fetchDouble("al_cf_z_gap");

  inter_structures_gap = 
	  db->fetchDouble("inter_structures_gap");

  n_towers=db->fetchInt("n_towers");

  if(n_towers != 3) {
	  G4cout << " n_towers != 3 not possible in this release"
		  << G4endl;
	  exit(1);
  }

  HalfEcalX = db->fetchDouble("ecal_x") / 2; 

  db->exec("select * from alveolus;");
  db->getTuple();

  g10_thickness = 
	  db->fetchDouble("g10_thickness");

  g10_x_out = 
	  db->fetchDouble("g10_x_out");

  al_g10_gap = 
	  db->fetchDouble("al_g10_gap");

  al_thickness = 
	  db->fetchDouble("al_thickness");

  endcap_x = 
	  db->fetchDouble("endcap_x");

  wafer_x_shift = 
	  db->fetchDouble("wafer_x_shift");

  upper_waffer_shift = 
	  db->fetchDouble("upper_waffer_shift");

  inter_waffer_gap = 
	  db->fetchDouble("inter_waffer_gap");

  n_waffers_x = 
	  db->fetchInt("n_waffers_x");

  n_waffers_z = 
	  db->fetchInt("n_waffers_y");

  if((n_waffers_x%2) == 0) {
      G4cout 
	<< "even n_waffers_x not possible in this release!!!!!!!" 
	<< G4endl;
      exit(1);
    }

  if(n_waffers_z>2) {
      G4cout 
	<< "n_waffers_z>2 not possible in this release!!!!!!!" 
	<< G4endl;
      exit(1);
    }

  //-------------------------------------
  // Dimensions de l'alveolus
  //-------------------------------------
  HalfAlveolusX = db->fetchDouble("al_dim_x") / 2;
  
  HalfAlveolusZ = db->fetchDouble("al_dim_y") / 2;
  
  db->exec("select * from tungsten;");
  db->getTuple();

  //------------------------------------
  // Dimensions W support des Si (WSlab)
  //------------------------------------
  HalfWSlabX = db->fetchDouble("wslab_x") / 2;
  HalfWSlabZ = db->fetchDouble("wslab_y") / 2;

  
#ifdef MOKKA_DEBUG
  if ((HalfWSlabX > HalfAlveolusX) || (HalfWSlabZ > (HalfAlveolusZ - al_thickness))) {
	G4cout << "======= ASSERT WILL CRASH :\n"
		<< " WSlab doesn't fit inside Al box\n"
		<< " in the X-Y plane\n" << G4endl;
        Control::Abort("Proto03::BuildElements: Assertion failed ((HalfWSlabX > HalfAlveolusX) || (HalfWSlabZ > (HalfAlveolusZ - al_thickness)))",MOKKA_OTHER_ERRORS);
  }
#endif

  //-----------------------------------
  // Dead W Plates common parameters
  //-----------------------------------
  HalfDeadWX = db->fetchDouble("deadw_x")/2.;

  HalfDeadWZ = db->fetchDouble("deadw_y")/2.;

  db->exec("select * from silicon;");
  db->getTuple();

  cell_dim_x = db->fetchDouble("cell_dim_x");
  cell_dim_z = db->fetchDouble("cell_dim_y");

  //---------------------------
  // Waffer, partie Sensitive
  //---------------------------
  n_cell_x = db->fetchInt("n_cell_x");
  G4double HalfSensitiveWafferX = 
    cell_dim_x*n_cell_x/2;
  G4double HalfSensitiveWafferY = 
    db->fetchDouble("si_thickness")/2;
  n_cell_z = db->fetchInt("n_cell_y");
  G4double HalfSensitiveWafferZ = 
    cell_dim_z*n_cell_z/2;
  n_guard_ring_zones = db->fetchInt("n_guard_ring_zones");
  
  //---------------------------
  // Waffer, total
  //---------------------------
  garde_size = db->fetchDouble("garde_size");
  HalfWafferX = 
    HalfSensitiveWafferX+garde_size;
  HalfWafferY = 
    HalfSensitiveWafferY;
  HalfWafferZ = 
    HalfSensitiveWafferZ+garde_size;
  
  lateralWaferGap = HalfAlveolusZ - 2*HalfWafferZ - 
	  inter_waffer_gap/2;
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
  
  SiWafferLogical=
    new G4LogicalVolume(WafferSolid,
			CGAGeometryManager::GetMaterial("silicon_2.33gccm"),
			"SiWafferLogical", 
			0, 
			0, 
			0);

  G4Region* GRRegion = new G4Region("GuardRing");
  SiWafferLogical->SetRegion(GRRegion);
  GRRegion->AddRootLogicalVolume(SiWafferLogical);
  CGAGeometryManager::GetCGAGeometryManager()->
	RegisterGeometryRegion(GRRegion, Control::ActiveRangeCut);

  
  SiO2WafferLogical=
    new G4LogicalVolume(WafferSolid,
			CGAGeometryManager::GetMaterial("quartz"),
			"SiO2WafferLogical", 
			0, 
			0, 
			0);
  
  VisAttSi = new G4VisAttributes(G4Colour(0.,1.,1.));
  VisAttSi->SetForceWireframe(true);
  G4VisAttributes * VisAttSiO2;
  VisAttSiO2 = new G4VisAttributes(G4Colour(1.,1.,1.));
  VisAttSiO2->SetForceWireframe(true);
  SiWafferLogical->SetVisAttributes(VisAttSi);
  SiO2WafferLogical->SetVisAttributes(VisAttSiO2);
  
  //----------------------------------------------------------
  // Placement Volume Waffer sensitive, dedans le waffer mort
  //----------------------------------------------------------
  G4Box *SensWafferSolid  = new G4Box("SensWafferSolid",
				      HalfSensitiveWafferX,
				      HalfSensitiveWafferY,
				      HalfSensitiveWafferZ);

  G4LogicalVolume * SensWafferLogical=
    new G4LogicalVolume(SensWafferSolid,
			CGAGeometryManager::GetMaterial("silicon_2.33gccm"),
			"SensWafferLogical", 
			0, 
			0, 
			0);

  SensWafferLogical->SetVisAttributes(VisAttSi);

  new MyPlacement(0,
		    G4ThreeVector(0., 0., 0.),
		    SensWafferLogical,
		    "SensWafferPhys",
		    SiWafferLogical,
		    false,0);

  G4Box *siStrip  = new G4Box("SiStripSolid",
				      HalfSensitiveWafferX,
				      HalfSensitiveWafferY,
				      cell_dim_z/2);

  G4LogicalVolume *siStripLogical=
    new G4LogicalVolume(siStrip,
			CGAGeometryManager::GetMaterial("silicon_2.33gccm"),
			"SiStripLogical", 
			0, 
			0, 
			0);

  siStripLogical->SetVisAttributes(VisAttSi);

  new G4PVReplica("SiStrips",
			  siStripLogical,
			  SensWafferLogical,
			  kZAxis,
			  n_cell_z,
			  cell_dim_z,
			  0);

  G4Box *siCell  = new G4Box("SiCellSolid",
				      cell_dim_x/2,
				      HalfSensitiveWafferY,
				      cell_dim_z/2);

  // The cell boundaries really exist as G4 volumes. 
  // So, there's no use for step limits defined like this:
  theMaxStepAllowed= std::min(cell_dim_x,cell_dim_z);
  
  // G4UserLimits* pULimits=
  // 	new G4UserLimits(theMaxStepAllowed);

  siCellLogical=
    new G4LogicalVolume(siCell,
			CGAGeometryManager::GetMaterial("silicon_2.33gccm"),
			"SiCellLogical", 
			0, 
			0, 
			0);
  //			pULimits);

  siCellLogical->SetVisAttributes(VisAttSi);

  G4Region* CellRegion = new G4Region("SiCell");
  siCellLogical->SetRegion(CellRegion);
  CellRegion->AddRootLogicalVolume(siCellLogical);
  CGAGeometryManager::GetCGAGeometryManager()->
	RegisterGeometryRegion(CellRegion, Control::ActiveRangeCut);

  new G4PVReplica("SiCells",
		  	  siCellLogical,
			  siStripLogical,
			  kXAxis,
			  n_cell_x,
			  cell_dim_x,
			  0);
  
}

void Proto03::BuildDeadPlate(WLAYERS* aPlate)
{
  //-----------------------------------
  // Build the W dead matter for dead layers
  // and Slabs
  //-----------------------------------

  G4double HalfDeadWY = aPlate->real_w_thickness / 2;
  
  G4VisAttributes * VisAttDeadW = new G4VisAttributes(G4Colour(0.,0.,1.));
  VisAttDeadW->SetVisibility(false);
  VisAttDeadW->SetForceWireframe(true);
  
  G4Box *DeadWSolid  = new G4Box("DeadWSolid",
				    HalfDeadWX,
				    HalfDeadWY,
				    HalfDeadWZ);

  // Logical for dead matter
  aPlate->AsLeftDeadWLogical =
    new G4LogicalVolume(DeadWSolid,
		    	CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
			"DeadWLogical", 
			0, 
			0, 
			0);
  
  aPlate->AsLeftDeadWLogical->SetRegion(WRegion);
  WRegion->AddRootLogicalVolume(aPlate->AsLeftDeadWLogical);

  aPlate->AsLeftDeadWLogical->SetVisAttributes(VisAttDeadW);

  aPlate->AsCenterDeadWLogical =
    new G4LogicalVolume(DeadWSolid,
		    	CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
			"DeadWLogical", 
			0, 
			0, 
			0);
  
  aPlate->AsCenterDeadWLogical->SetRegion(WRegion);
  WRegion->AddRootLogicalVolume(aPlate->AsCenterDeadWLogical);

  aPlate->AsCenterDeadWLogical->SetVisAttributes(VisAttDeadW);

  aPlate->AsRightDeadWLogical =
    new G4LogicalVolume(DeadWSolid,
		    	CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
			"DeadWLogical", 
			0, 
			0, 
			0);
  
  aPlate->AsRightDeadWLogical->SetRegion(WRegion);
  WRegion->AddRootLogicalVolume(aPlate->AsRightDeadWLogical);

  aPlate->AsRightDeadWLogical->SetVisAttributes(VisAttDeadW);

  G4double HalfWSlabY = aPlate->real_w_thickness / 2;

  G4Box *SlabWSolid  = new G4Box("SlabWSolid",
				    HalfWSlabX,
				    HalfWSlabY,
				    HalfWSlabZ);

  // Logical for slab dead matter
  aPlate->AsCenterSlabWLogical =
    new G4LogicalVolume(SlabWSolid,
		    	CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
			"SlabWLogical", 
			0, 
			0, 
			0);
  
  aPlate->AsCenterSlabWLogical->SetRegion(WRegion);
  WRegion->AddRootLogicalVolume(aPlate->AsCenterSlabWLogical);

  aPlate->AsCenterSlabWLogical->SetVisAttributes(VisAttDeadW);

  aPlate->AsRightSlabWLogical =
    new G4LogicalVolume(SlabWSolid,
		    	CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
			"SlabWLogical", 
			0, 
			0, 
			0);
  
  aPlate->AsRightSlabWLogical->SetRegion(WRegion);
  WRegion->AddRootLogicalVolume(aPlate->AsRightSlabWLogical);

  aPlate->AsRightSlabWLogical->SetVisAttributes(VisAttDeadW);


}

void Proto03::CalculateSlabShifts(void) {

  unsigned int i_plate1=0, i_plate2=0;

  for (unsigned int i_group=0;i_group<PlateGroups.size();i_group++){

    PlateGroups[i_group]->maxSlabShift = 0;

    for (i_plate1=i_plate2 ; i_plate1< i_plate2 +
		(unsigned int)(PlateGroups[i_group]->n_layers); i_plate1++)
    {
	WLAYERS* aPlate = Plates[i_plate1];

	/*
  	G4double HalfAlveolusY =
    		aPlate->nominal_w_thickness / 2 +    // 1 fois
    		waffer_fiber_vertical_thickness +
		HalfWafferY * 2 +    // 2 fois
		g10_thickness +
    		al_g10_gap +
    		al_thickness;

  	G4double shift = 0;
  	switch(i_plate1%5) {
	  case 0:
	    shift = -(4*HalfAlveolusY - exit_fiber_thickness/2 + al_cf_y_gap*2 +
		     aPlate->nominal_w_thickness *5/2 + 
	     	deadw_fiber_thickness * 5)* tan(config_angle*deg);
    	    break;
  	  case 1:
    	     shift = -(2*HalfAlveolusY - exit_fiber_thickness /2 + al_cf_y_gap +
	   	aPlate->nominal_w_thickness *3/2 +
	   	deadw_fiber_thickness * 3)* tan(config_angle*deg);
    	     break;
  	  case 2:
    		shift =-(-exit_fiber_thickness/2 
				+ aPlate->nominal_w_thickness /2 +
	    	deadw_fiber_thickness)* tan(config_angle*deg);
    	     break;
  	  case 3:
    	     shift = (2*HalfAlveolusY - exit_fiber_thickness /2 + al_cf_y_gap +
	   	aPlate->nominal_w_thickness /2 +
	   	deadw_fiber_thickness)* tan(config_angle*deg);
    	     break;
  	  case 4:
    	     shift = (4*HalfAlveolusY - exit_fiber_thickness/2 + al_cf_y_gap*2 +
	     	aPlate->nominal_w_thickness *3/2 + 
	     	deadw_fiber_thickness * 3)* tan(config_angle*deg);
    	     break;  
  	}; 

  	if(c_angle_from_steering) {
    		aPlate->center_slab_shift += shift;
    		aPlate->right_slab_shift += shift;
  	}

  	if(aPlate->center_slab_shift < 0.) {
		G4cout << "Center Slab No " << i_plate1 << 
			" has got a negative shift: " << 
			aPlate->center_slab_shift << " reseting it to zero!" <<
			G4endl;
    		aPlate->center_slab_shift = 0.;
	}
	
  	if(aPlate->right_slab_shift < 0.){
		G4cout << "Right Slab No " << i_plate1 << 
			" has got a negative shift: " << 
			aPlate->right_slab_shift << " reseting it to zero!" <<
			G4endl;
    		aPlate->right_slab_shift = 0.;
	}
	*/
	if(aPlate->center_slab_shift >
			PlateGroups[i_group]->maxSlabShift)
		PlateGroups[i_group]->maxSlabShift = 
			aPlate->center_slab_shift;

	if(aPlate->right_slab_shift >
			PlateGroups[i_group]->maxSlabShift)
		PlateGroups[i_group]->maxSlabShift = 
			aPlate->right_slab_shift;
    } 
    i_plate2 = i_plate1;
  }
}

void Proto03::BuildAlveola(WLAYERS* aPlate) {

  //------------------------------------
  // VisAttributes pour Alevolus et Slab
  //------------------------------------

  VisAttCF = new G4VisAttributes(G4Colour(1.,0.1,1.));
  VisAttCF->SetForceWireframe(true);
  VisAttCF->SetDaughtersInvisible(true);
  G4VisAttributes * VisAttWSlab = new G4VisAttributes(G4Colour(1.,0.2,1.));
  VisAttWSlab->SetForceWireframe(true);
  VisAttWSlab->SetDaughtersInvisible(false);
  
  VisAttAir = new G4VisAttributes(G4Colour(1.,1.,1.));
  VisAttAir->SetForceWireframe(true);
  VisAttAir->SetVisibility(false);

  VisAttAl = new G4VisAttributes(G4Colour(1.,0.5,1.));
  VisAttAl->SetForceWireframe(true);

  VisAttG10 = new G4VisAttributes(G4Colour(0.1,1.,1.));
  VisAttG10->SetForceWireframe(true);

  //-----------------------------------
  // Logical type Alveolus
  //-----------------------------------
  G4double HalfAlveolusY =
    aPlate->nominal_w_thickness / 2 +    // 1 fois
    waffer_fiber_vertical_thickness +
    HalfWafferY * 2 +    // 2 fois
    g10_thickness +
    al_g10_gap +
    al_thickness;


  aPlate->AsAlveolusTotalHalfY=HalfAlveolusY;
  // Alveolus
  G4Box *LeftAlveolusSolid  = new G4Box("AlveolusSolid",
				    HalfEcalX + 
	G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(),
				    HalfAlveolusY + al_cf_y_gap/2,
				    HalfAlveolusZ + 
	G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()
					);
  
  aPlate->AsLeftAlveolusLogical=
    new G4LogicalVolume(LeftAlveolusSolid,
		    	CGAGeometryManager::GetMaterial("air"),
			"LeftAlveolusLogical", 
			0, 
			0, 
			0);
  
  aPlate->AsLeftAlveolusLogical->SetVisAttributes(VisAttAir);
  
  G4double HalfEnvAlveolusX = HalfAlveolusX + endcap_x/2 + g10_x_out/2;

  G4Box *EnvAlveolusSolid  = new G4Box("EnvAlveolusSolid",
				    HalfEnvAlveolusX,
				    HalfAlveolusY,
				    HalfAlveolusZ);
  
  aPlate->AsCenterAlveolusLogical=
    new G4LogicalVolume(EnvAlveolusSolid,
		    	CGAGeometryManager::GetMaterial("air"),
			"CenterAlveolusLogical", 
			0, 
			0, 
			0);
  
  aPlate->AsCenterAlveolusLogical->SetVisAttributes(VisAttAir);
  
  aPlate->AsRightAlveolusLogical=
    new G4LogicalVolume(EnvAlveolusSolid,
		    	CGAGeometryManager::GetMaterial("air"),
			"RightAlveolusLogical", 
			0, 
			0, 
			0);
  
  aPlate->AsRightAlveolusLogical->SetVisAttributes(VisAttAir);
  
  G4Box *AlSolid  = new G4Box("AlSolid",
				    HalfAlveolusX,
				    HalfAlveolusY,
				    HalfAlveolusZ);
  
  G4LogicalVolume * alCenterLogical = 
    new G4LogicalVolume(AlSolid,
		    	CGAGeometryManager::GetMaterial("aluminium"),
			"CenterAlLogical", 
			0, 
			0, 
			0);
  
  alCenterLogical->SetVisAttributes(VisAttAl);
  
  G4LogicalVolume * alRightLogical =
    new G4LogicalVolume(AlSolid,
		    	CGAGeometryManager::GetMaterial("aluminium"),
			"RightAlLogical", 
			0, 
			0, 
			0);
  
  alRightLogical->SetVisAttributes(VisAttAl);
  
  new MyPlacement(0,
		    G4ThreeVector(-HalfEnvAlveolusX + HalfAlveolusX + 
			  endcap_x, 0., 0.),
		    alCenterLogical,
		    "CenterAlPhys",
		    aPlate->AsCenterAlveolusLogical,
		    false,0);
  
  new MyPlacement(0,
		    G4ThreeVector(-HalfEnvAlveolusX + HalfAlveolusX + 
			  endcap_x, 0., 0.),
		    alRightLogical,
		    "RightAlPhys",
		    aPlate->AsRightAlveolusLogical,
		    false,0);
  
  G4Box *CFSolid  = new G4Box("CarbonFiberSolid",
				    HalfAlveolusX,
				    HalfAlveolusY - al_thickness,
				    HalfAlveolusZ - al_thickness);
  
  
  G4LogicalVolume * CFCenterLogical = 
    new G4LogicalVolume(CFSolid,
		    	CarbonFiber, 
			"CarbonFiberCenterLogical",
			0, 
			0, 
			0);
  
  CFCenterLogical->SetVisAttributes(VisAttCF);
  
  G4LogicalVolume * CFRightLogical = 
    new G4LogicalVolume(CFSolid,
		    	CarbonFiber, 
			"CarbonFiberRightLogical",
			0, 
			0, 
			0);
  
  CFRightLogical->SetVisAttributes(VisAttCF);
  
  new MyPlacement(0,
		    G4ThreeVector(0., 0., 0.),
		    CFCenterLogical,
		    "CenterCFPhys",
		    alCenterLogical,
		    false,0);
  
  new MyPlacement(0,
		    G4ThreeVector(0., 0., 0.),
		    CFRightLogical,
		    "RightCFPhys",
		    alRightLogical,
		    false,0);
 
  //-----------------------------------------
  // WSlab
  //-----------------------------------------

  new MyPlacement(0,
		    G4ThreeVector(0., 0., 0.),
		    aPlate->AsCenterSlabWLogical,
		    "CenterWSlabPhys",
		    CFCenterLogical,
		    false,0);
  
  new MyPlacement(0,
		    G4ThreeVector(0., 0., 0.),
		    aPlate->AsRightSlabWLogical,
		    "RightWSlabPhys",
		    CFRightLogical,
		    false,0);
  
  FillAlveola(aPlate, CFCenterLogical, CFRightLogical);

  G4Box *alEndCap  = new G4Box("alEndCap",
			    endcap_x/2,
			    HalfAlveolusY,
			    HalfAlveolusZ);
  
  G4LogicalVolume * alEndCapLogical = 
    new G4LogicalVolume(alEndCap,
		    	CGAGeometryManager::GetMaterial("aluminium"),
			"AlEndCapLogical", 
			0, 
			0, 
			0);
  
  alEndCapLogical->SetVisAttributes(VisAttAl);
  
  new MyPlacement(0,
		    G4ThreeVector(-HalfEnvAlveolusX + endcap_x/2, 0., 0.),
		    alEndCapLogical,
		    "CenterAlEndCapPhys",
		    aPlate->AsCenterAlveolusLogical,
		    false,0);
  
  new MyPlacement(0,
		    G4ThreeVector(-HalfEnvAlveolusX + endcap_x/2, 0., 0.),
		    alEndCapLogical,
		    "RightAlEndCapPhys",
		    aPlate->AsRightAlveolusLogical,
		    false,0);
  
  G4double yDisp =  2*HalfWafferY + g10_thickness/2 + 
    		aPlate->nominal_w_thickness / 2 
		+ waffer_fiber_vertical_thickness;

  G4double HalfG10OutZ = (HalfAlveolusZ - al_thickness -
		          waffer_fiber_lateral_thickness)/2;

  G4Box *G10Solid = new G4Box("G10Solid",
		g10_x_out/2,
		g10_thickness/2.,
		HalfG10OutZ);
  
  G4LogicalVolume * G10Logical = 
    new G4LogicalVolume(G10Solid,
			CGAGeometryManager::GetMaterial(theG10MaterialName),
			"G10Logical",
			0, 
			0, 
			0);
  
  G10Logical->SetRegion(G10Region);
  G10Region->AddRootLogicalVolume(G10Logical);

  G10Logical->SetVisAttributes(VisAttG10);


  new MyPlacement(0,
		    G4ThreeVector(HalfEnvAlveolusX - g10_x_out/2, yDisp, 
			    HalfG10OutZ),
		    G10Logical,
		    "CenterTopG10Phys",
		    aPlate->AsCenterAlveolusLogical,
		    false,0);
  
  new MyPlacement(0,
		    G4ThreeVector(HalfEnvAlveolusX - g10_x_out/2, -yDisp, 
			    -HalfG10OutZ),
		    G10Logical,
		    "CenterBottomG10Phys",
		    aPlate->AsCenterAlveolusLogical,
		    false,0);

  new MyPlacement(0,
		    G4ThreeVector(HalfEnvAlveolusX - g10_x_out/2, yDisp, 
			    HalfG10OutZ),
		    G10Logical,
		    "RightTopG10Phys",
		    aPlate->AsRightAlveolusLogical,
		    false,0);
  
  new MyPlacement(0,
		    G4ThreeVector(HalfEnvAlveolusX - g10_x_out/2, -yDisp, 
			    -HalfG10OutZ),
		    G10Logical,
		    "RightBottomG10Phys",
		    aPlate->AsRightAlveolusLogical,
	       	    false,0);
}

void Proto03::FillAlveola(WLAYERS* aPlate, G4LogicalVolume * CFCenterLogical,
				G4LogicalVolume * CFRightLogical) {

  G4Box *AirSolid  = new G4Box("AirSolid",
				    HalfAlveolusX,
				    HalfWafferY + 
				    (g10_thickness + al_g10_gap)/2.,
				    HalfAlveolusZ - al_thickness -
				    waffer_fiber_lateral_thickness);
  
  G4LogicalVolume * AirCenterTopLogical = 
    new G4LogicalVolume(AirSolid,
		    	CGAGeometryManager::GetMaterial("air"),
			"AirCenterTopLogical",
			0, 
			0, 
			0);
  
  AirCenterTopLogical->SetVisAttributes(VisAttAir);
  
  G4LogicalVolume * AirCenterBottomLogical = 
    new G4LogicalVolume(AirSolid,
		    	CGAGeometryManager::GetMaterial("air"),
			"AirCenterBottomLogical",
			0, 
			0, 
			0);
  
  AirCenterBottomLogical->SetVisAttributes(VisAttAir);
  
  G4LogicalVolume * AirRightTopLogical = 
    new G4LogicalVolume(AirSolid,
			CGAGeometryManager::GetMaterial("air"),
			"AirRightTopLogical",
			0, 
			0, 
			0);
  
  AirRightTopLogical->SetVisAttributes(VisAttAir);
  
  G4LogicalVolume * AirRightBottomLogical = 
    new G4LogicalVolume(AirSolid,
			CGAGeometryManager::GetMaterial("air"),
			"AirRightBottomLogical",
			0, 
			0, 
			0);
  
  AirRightBottomLogical->SetVisAttributes(VisAttAir);
  
  G4double yDispAir =  HalfWafferY +(g10_thickness + al_g10_gap)/2 + 
    		aPlate->nominal_w_thickness / 2 + 
		waffer_fiber_vertical_thickness;

  new MyPlacement(0,
		    G4ThreeVector(0., -yDispAir, 0.),
		    AirCenterTopLogical,
		    "CenterTopAirPhys",
		    CFCenterLogical,
		    false,0);
  new MyPlacement(0,
		    G4ThreeVector(0., yDispAir, 0.),
		    AirCenterBottomLogical,
		    "CenterBottomAirPhys",
		    CFCenterLogical,
		    false,0);
  
  new MyPlacement(0,
		    G4ThreeVector(0., -yDispAir, 0.),
		    AirRightTopLogical,
		    "RightTopAirPhys",
		    CFRightLogical,
		    false,0);
  
  new MyPlacement(0,
		    G4ThreeVector(0., yDispAir, 0.),
		    AirRightBottomLogical,
		    "RightBottomAirPhys",
		    CFRightLogical,
		    false,0);

  G4Box *G10SolidTop  = new G4Box("G10SolidTop",
				    HalfAlveolusX - upper_waffer_shift/2.,
				    g10_thickness/2.,
				    HalfAlveolusZ - al_thickness -
				    waffer_fiber_lateral_thickness);
  
  G4Box *G10SolidCenterBottom  = new G4Box("G10SolidCenterBottom",
		HalfAlveolusX - 
			(upper_waffer_shift + aPlate->center_waffer_shift)/2.,
		g10_thickness/2.,
	       	HalfAlveolusZ - al_thickness -
				    waffer_fiber_lateral_thickness);
  
  G4Box *G10SolidRightBottom  = new G4Box("G10SolidRightBottom",
		HalfAlveolusX - 
			(upper_waffer_shift + aPlate->right_waffer_shift)/2.,
		g10_thickness/2.,
	       	HalfAlveolusZ - al_thickness -
				    waffer_fiber_lateral_thickness);
  
  G4LogicalVolume * G10LogicalTop = 
    new G4LogicalVolume(G10SolidTop,
			CGAGeometryManager::GetMaterial(theG10MaterialName),
			"G10LogicalTop",
			0, 
			0, 
			0);
  
  G10LogicalTop->SetRegion(G10Region);
  G10Region->AddRootLogicalVolume(G10LogicalTop);

  G10LogicalTop->SetVisAttributes(VisAttG10);

  G4LogicalVolume * G10LogicalCenterBottom = 
    new G4LogicalVolume(G10SolidCenterBottom,
			CGAGeometryManager::GetMaterial(theG10MaterialName),
			"G10LogicalCenterBottom",
			0, 
			0, 
			0);
  
  G10LogicalCenterBottom->SetRegion(G10Region);
  G10Region->AddRootLogicalVolume(G10LogicalCenterBottom);

  G10LogicalCenterBottom->SetVisAttributes(VisAttG10);

  G4LogicalVolume * G10LogicalRightBottom = 
    new G4LogicalVolume(G10SolidRightBottom,
			CGAGeometryManager::GetMaterial(theG10MaterialName),
			"G10LogicalRightBottom",
			0, 
			0, 
			0);
  
  G10LogicalRightBottom->SetRegion(G10Region);
  G10Region->AddRootLogicalVolume(G10LogicalRightBottom);

  G10LogicalRightBottom->SetVisAttributes(VisAttG10);

  G4double yDisp = HalfWafferY - al_g10_gap/2.;

  new MyPlacement(0,
		    G4ThreeVector(upper_waffer_shift/2., -yDisp, 0.),
		    G10LogicalTop,
		    "CenterTopG10Phys",
		    AirCenterTopLogical,
		    false,0);
  
  new MyPlacement(0,
		    G4ThreeVector(upper_waffer_shift/2., -yDisp, 0.),
		    G10LogicalTop,
		    "RightTopG10Phys",
		    AirRightTopLogical,
		    false,0);

  new MyPlacement(0,
		    G4ThreeVector(
			 (upper_waffer_shift+aPlate->center_waffer_shift)/2., 
			 yDisp, 0.),
		    G10LogicalCenterBottom,
		    "CenterBottomG10Phys",
		    AirCenterBottomLogical,
		    false,0);

  new MyPlacement(0,
		    G4ThreeVector(
			    (upper_waffer_shift+aPlate->right_waffer_shift)/2., 
			    yDisp, 0.),
		    G10LogicalRightBottom,
		    "RightBottomG10Phys",
		    AirRightBottomLogical,
		    false,0);
  
  //-------------------------
  // Placement des waffers
  //-------------------------
  // Placement des waffers dans les alveoles du centre et de droite. 

  G4double check = 2*HalfAlveolusX - (2*n_waffers_x * HalfWafferX 
		  + (n_waffers_x - 1) * inter_waffer_gap 
		  + upper_waffer_shift + aPlate->center_waffer_shift
		  + wafer_x_shift);

#ifdef MOKKA_DEBUG
  if (check < 0.) {
	G4cout << "======= ASSERT WILL CRASH :\n"
		<< " X dimension of alveolus is not big enough\n"
		<< " to contain all waffers and upper and lower\n"
		<< " center waffer shifts as defined by endcap dimensions\n" 
		<< G4endl;
        Control::Abort("Proto03::FillAlveola: Assertion failed (check < 0.) (ID 1)",MOKKA_OTHER_ERRORS);
  }
#endif

  check = 2*HalfAlveolusX - (2*n_waffers_x * HalfWafferX 
		  + (n_waffers_x - 1) * inter_waffer_gap 
		  + upper_waffer_shift + aPlate->right_waffer_shift
		  + wafer_x_shift);

#ifdef MOKKA_DEBUG
  if(check < 0.) {
	G4cout << "======= ASSERT WILL CRASH :\n"
		<< " X dimension of alveolus is not big enough\n"
		<< " to contain all waffers and upper and lower\n"
		<< " right waffer shifts as defined by endcap dimensions\n" 
		<< G4endl;
        Control::Abort("Proto03::FillAlveola: Assertion failed (check < 0.) (ID 2)",MOKKA_OTHER_ERRORS);
  }
#endif

  G4double VerticalDisp = - (g10_thickness + al_g10_gap) / 2;

  aPlate->cell_y_pos_in_alveolus = yDispAir + VerticalDisp;

  G4double siLayerCenterX = - HalfAlveolusX + (n_waffers_x * HalfWafferX 
	  + (n_waffers_x - 1) * inter_waffer_gap / 2 + upper_waffer_shift
	  + wafer_x_shift);

  G4double xDisp= (2*HalfWafferX+inter_waffer_gap) * (n_waffers_x-1)/2.;

  PlaceWafers(AirCenterTopLogical, AirCenterBottomLogical, SiO2WafferLogical,
	  SiO2WafferLogical, siLayerCenterX + xDisp, 
	  aPlate->center_waffer_shift, VerticalDisp, 0, 0);

  PlaceWafers(AirRightTopLogical, AirRightBottomLogical, SiO2WafferLogical,
	  SiO2WafferLogical, siLayerCenterX + xDisp,
	  aPlate->right_waffer_shift, VerticalDisp, 0, 0);

  PlaceWafers(AirCenterTopLogical, AirCenterBottomLogical, SiO2WafferLogical,
	  SiO2WafferLogical, siLayerCenterX - xDisp, 
	  aPlate->center_waffer_shift, VerticalDisp, 0, 0);

  PlaceWafers(AirRightTopLogical, AirRightBottomLogical, SiO2WafferLogical,
	  SiO2WafferLogical, siLayerCenterX - xDisp,
	  aPlate->right_waffer_shift, VerticalDisp, 0, 0);

  for(G4int i = n_waffers_x - 2; i > 0; i--) {
  
      xDisp -= (2*HalfWafferX+inter_waffer_gap);

      PlaceWafers(AirCenterTopLogical, AirCenterBottomLogical, 
		   SiWafferLogical, SiWafferLogical, 
		   siLayerCenterX + xDisp, 
		   aPlate->center_waffer_shift, 
		   VerticalDisp, n_waffers_z, i);

      PlaceWafers(AirRightTopLogical, AirRightBottomLogical, 
                   SiWafferLogical, SiO2WafferLogical, 
	           siLayerCenterX + xDisp,
		   aPlate->right_waffer_shift, 
                   VerticalDisp, 0, i);
  }

  G4Box *alTop  = new G4Box("alTop",
			    upper_waffer_shift/2.,
			    HalfWafferY + 
				    (g10_thickness + al_g10_gap)/2.,
			    HalfAlveolusZ - al_thickness -
			    waffer_fiber_lateral_thickness);
  
  G4LogicalVolume * alTopLogical =
    new G4LogicalVolume(alTop,
		    	CGAGeometryManager::GetMaterial("aluminium"),
			"TopAlEndcapLogical", 
			0, 
			0, 
			0);
  
  alTopLogical->SetVisAttributes(VisAttAl);
  
  new MyPlacement(0,
	    G4ThreeVector(-HalfAlveolusX + upper_waffer_shift/2., 0., 0.),
	    alTopLogical,
	    "CenterAlTopEndcapPhys",
	    AirCenterTopLogical,
	    false,0);
  
  new MyPlacement(0,
	    G4ThreeVector(-HalfAlveolusX + upper_waffer_shift/2., 0., 0.),
	    alTopLogical,
	    "RightAlTopEndcapPhys",
	    AirRightTopLogical,
	    false,0);
  
  G4Box *alCenterBottom  = new G4Box("alCenterBottom",
		    (upper_waffer_shift + aPlate->center_waffer_shift)/2.,
		    HalfWafferY + 
			    (g10_thickness + al_g10_gap)/2.,
		    HalfAlveolusZ - al_thickness -
			    waffer_fiber_lateral_thickness);
  
  G4LogicalVolume * alCenterBottomLogical =
    new G4LogicalVolume(alCenterBottom,
		    	CGAGeometryManager::GetMaterial("aluminium"),
			"BottomAlEndcapLogical", 
			0, 
			0, 
			0);
  
  alCenterBottomLogical->SetVisAttributes(VisAttAl);
  
  G4Box *alRightBottom  = new G4Box("alRightBottom",
		    (upper_waffer_shift + aPlate->right_waffer_shift)/2.,
		    HalfWafferY + 
			    (g10_thickness + al_g10_gap)/2.,
		    HalfAlveolusZ - al_thickness -
			    waffer_fiber_lateral_thickness);
  

  G4LogicalVolume * alRightBottomLogical =
    new G4LogicalVolume(alRightBottom,
		    	CGAGeometryManager::GetMaterial("aluminium"),
			"BottomAlEndcapLogical", 
			0, 
			0, 
			0);
  
  alRightBottomLogical->SetVisAttributes(VisAttAl);
  
  new MyPlacement(0,
	 G4ThreeVector(-HalfAlveolusX + 
	    (upper_waffer_shift + aPlate->center_waffer_shift)/2., 0., 0.),
         alCenterBottomLogical,
         "CenterAlBottomEndcapPhys",
         AirCenterBottomLogical,
         false,0);
  
  new MyPlacement(0,
	 G4ThreeVector(-HalfAlveolusX + 
	    (upper_waffer_shift + aPlate->right_waffer_shift)/2., 0., 0.),
         alRightBottomLogical,
         "RightAlBottomEndcapPhys",
         AirRightBottomLogical,
         false,0);
}

		      
void Proto03::PlaceWafers(G4LogicalVolume * motherTop, 
		G4LogicalVolume * motherBottom,
		G4LogicalVolume * leftWaffer, G4LogicalVolume * rightWaffer, 
		G4double xDisp, G4double bottomShift, G4double yDisp, 
		G4int delta_j, G4int i) {  //center delta_i=0; right delta_i=n_waffers_z


  G4String WafferName("WafferPhysical");

  G4double zDisp = (n_waffers_z-1)*
	(HalfWafferZ+inter_waffer_gap/2.);

  G4LogicalVolume * wafferToPlace = rightWaffer;
	      
  for(G4int j=0;j<n_waffers_z;j++) {
	G4int CopyNumber=10000+ i*10 + j + delta_j; //center = 0; right = n_waffers_z
	new MyPlacement(0, G4ThreeVector(xDisp, -yDisp, zDisp),
				    wafferToPlace,
				    WafferName,
				    motherTop,
				    false,-CopyNumber);
	new MyPlacement(0,G4ThreeVector(xDisp + bottomShift,
				    yDisp, zDisp),
				    wafferToPlace,
				    WafferName,
				    motherBottom,
				    false,CopyNumber);
	zDisp -= (2*HalfWafferZ+inter_waffer_gap);  
	wafferToPlace = leftWaffer;
  }
} 
  
void  
Proto03::BeginOfEventAction(const G4Event*)
{
  Dumped=false;
}

void 
Proto03::EndOfEventAction(const G4Event* evt)
{
  if(!Dumped) 
    VSubDetectorDriver::EndOfEventAction(evt);
  Dumped = true;
}
