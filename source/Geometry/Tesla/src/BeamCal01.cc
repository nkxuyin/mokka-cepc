// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: BeamCal01.cc,v 1.7 2008/11/12 16:05:32 steve Exp $
// $Name:  $
//
// History:
// - first implementation, code from Fcal collaboration,
//     * position of sensitive layer corrected
//     * several constants moved to beamcal table
//     * superdriver by A. Sapronov, A. Rosca and A. Popescu
//     * implemented in Mokka by A. Hartin and S. Aplin, Oct 2008
// - Changes to allow a maximum of 1024 layers numbers by A. Sailer
// 
//


#include "BeamCal01.hh"

#include "BeamCalSD01.hh"
#include "CGAGeometryManager.hh"
#include "MySQLWrapper.hh"
#include "Control.hh"
#include "CGADefs.h"

#include "assert.h"

#include "G4Tubs.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4PVPlacement.hh"
#include "G4ThreeVector.hh"
#include "G4Transform3D.hh"
#include "G4SubtractionSolid.hh"


#ifdef MOKKA_GEAR
#include "gear/CalorimeterParameters.h"
#include "gearimpl/CalorimeterParametersImpl.h"
#include "gearimpl/LayerLayoutImpl.h"
#include "MokkaGear.h"
#endif

//Encoder32Fcal uses 10 bit for layers, this defines the maximum, one additional number is taken for the PairMonitor
#define BEAMCAL_MAX_LAYERS 1023
#define CHECKSURFACE false

INSTANTIATE(BeamCal01)

G4bool BeamCal01::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog)
{
  //worldLog--->The world volume
  //used for the placement of the detectors elements

  // Geometry parameters from the geometry environment and from the database
  
  G4cout << "BeamCal01: Starting" << G4endl;

  //global geometry parameter-->from the environment
  const G4double crossingAngle = env.GetParameterAsDouble("ILC_Main_Crossing_Angle") * mrad; //the xAngle

  //open the appropriate data base , no longer depends on crossing anle!

  const G4String dbName = env.GetDBName();

  Database *db = new Database(dbName.c_str());

  BeamCalValueMap BeamCalValues;
  //get the variabiles from the db
  db->exec("SELECT * FROM beamcal;");
  while (db->getTuple()) {
    G4String globalName   = db->fetchString("globalName");
    G4double value        = db->fetchDouble("beamcalvalue");
    BeamCalValues[globalName]   = value;
  }
  delete db;

  //Parameters from Database
  const G4double dSensor        = BeamCalValues["dSensor"];            // Diamond thickness
  const G4double dAirgap        = BeamCalValues["dAirgap"];            // Layer gap (air)
  const G4double dElboard       = BeamCalValues["dElboard"];           // PCB (Kapton)
  const G4double dElectrMet     = BeamCalValues["dElectrMet"];         //Sensor Electrode Metalisation (gold)

  //Global Parameters
  //inner radius of the calorimeter. 10 microns has been added to ensure no overlap with the beampipe due to numerical instability
  const G4double Rinner			= env.GetParameterAsDouble("BCal_rInner") + 0.01*mm;
  const G4double Router			= env.GetParameterAsDouble("BCal_rOuter");
  const G4double dAbsorber		= env.GetParameterAsDouble("BCal_dAbsorber");
  const G4double dGraphite		= env.GetParameterAsDouble("BCal_dGraphite");
  const G4double incomingTubeRadius	= env.GetParameterAsDouble("BCal_TubeIncomingRadius");
  const G4double RSegmentation		= env.GetParameterAsDouble("BCal_rSegmentation");
  const G4int    nLayers		= env.GetParameterAsInt("BCal_nLayers");
  const G4double nWafers		= env.GetParameterAsDouble("BCal_nWafers");
  const G4int    pairmonitorOnOff	= env.GetParameterAsInt("BCal_PairMonitor");
  const G4double dPhi			= env.GetParameterAsDouble("BCal_SpanningPhi")*deg;
  const G4double zStart			= env.GetParameterAsDouble("LHcal_zend") + env.GetParameterAsDouble("LHcal_BCal_clearance");

  //Necessary but calculable parameters
  const G4double sPhi			= 360*deg - dPhi/2.;

  // G4cout << "LHCal Zend:           " << env.GetParameterAsDouble("LHcal_zend") << G4endl;
  // G4cout << "BeamCal inner radius: " << Rinner     << " mm " << G4endl;
  // G4cout << "BeamCal outer radius: " << Router     << " mm " << G4endl;




  if(nLayers > BEAMCAL_MAX_LAYERS) {
    Control::Log("BeamCal01: You are using too many Layers in BeamCal, maximum is 1023 (one is reserved for PairMonitor)!");
    return false; // premature exit, Mokka will abort now
  }
  //this is always true, even if crossing anlge is 0
  tiltForw = crossingAngle/2.;
  tiltBack = 180 * deg - crossingAngle/2.;

  //calculation
  dLayer  = dAbsorber+dSensor+dElectrMet+dElboard+dAirgap; //layer thickness
  length  = dLayer*nLayers; //total length of the calorimeter


  //SJA: create and overall envelope volume which will include the pair monitor + 3mm clearance + graphite shield + 6mm clearance + calorimeter    
  G4double envVolumeLength  = dSensor + 3.*mm  + dGraphite + 6.*mm + length ;
  G4double PMLength =  dSensor + 3.*mm  + dGraphite;
  G4double BCLength =  length;
  G4double GlobalStart = zStart - (PMLength +6*mm);
  G4double BCStart = zStart; //This is were BeamCal starts!
  //  G4double envVolumeCenterZ = GlobalStart + length - (envVolumeLength/2.0);
  G4double envVolumeCenterZ = GlobalStart + envVolumeLength/2.0;
  G4double PMVolumeCenterZ = GlobalStart + PMLength/2.0;
  // G4cout << "BeamCal GlobalStart: " << GlobalStart     << "\n" 
  // 	 << "BeamCal zStart:      " << zStart          << "\n" 
  // 	 << "BeamCal EnvLength:   " << envVolumeLength << "\n" 
  // 	 << "BeamCal LayerLength: " << dLayer          << "\n" 
  // 	 << G4endl;

  //
  // Dead Area Calculations
  // BPMaxRcalc
  //  G4double BPMaxRCalc = (-1*G4ThreeVector(-incomingTubeRadius, incomingTubeRadius, BCStart+BCLength).rotateY(-crossingAngle)[0])+0.1 * mm ;
  G4double BPMaxRCalc = ((G4ThreeVector(-incomingTubeRadius, incomingTubeRadius, BCStart+BCLength).rotateY(-crossingAngle)).getRho())+0.1 * mm ;
  G4cout << "BPMaxRCalc:            " << BPMaxRCalc << " mm " << G4endl;


  G4Transform3D transformer(G4RotationMatrix().rotateY(tiltForw),                  G4ThreeVector(0, 0, BCStart+BCLength/2.).rotateY(tiltForw));
  G4Transform3D transmirror(G4RotationMatrix().rotateZ(180*deg).rotateY(tiltBack), G4ThreeVector(0, 0, BCStart+BCLength/2.).rotateY(tiltBack));

  G4Transform3D transformerPM(G4RotationMatrix().rotateY(tiltForw),                  G4ThreeVector(0, 0, PMVolumeCenterZ).rotateY(tiltForw));
  G4Transform3D transmirrorPM(G4RotationMatrix().rotateZ(180*deg).rotateY(tiltBack), G4ThreeVector(0, 0, PMVolumeCenterZ).rotateY(tiltBack));


  //inline functions from BeamCalSD01.hh
  dR  = split_segm((Router-Rinner),RSegmentation); //distance between 2 rings
  nRs = split_n((Router-Rinner),RSegmentation);//number of rings
  //DA is for DEAD AREA
  DAStart=0, DArinner=-1;

  if(crossingAngle>=14*mrad) {
    for(int j = 0; j < nRs; j++){
      r = Rinner + j*dR;
      if(r >= BPMaxRCalc){
	DArinner = r; //inner radius of the DA
	DAStart  = j; //number where rings are complete
	break;
      }
    }
  }//if the crossing angle is above 14mrad
  
  // G4cout << "SJA:TAG: DArinner:"  << DArinner << G4endl;

  //materials to be used
  materialAir      = CGAGeometryManager::GetMaterial("air");
  materialTungsten = CGAGeometryManager::GetMaterial("tungsten_19.3gccm");
  materialKapton   = CGAGeometryManager::GetMaterial("kapton");
  materialGold     = CGAGeometryManager::GetMaterial("gold");
  materialDiamond  = CGAGeometryManager::GetMaterial("diamond");
  materialSilicon  = CGAGeometryManager::GetMaterial("silicon");
  materialGraphite = CGAGeometryManager::GetMaterial("graphite"); 


  //visualisation attributes
  G4VisAttributes *caloVisAttributes      = new G4VisAttributes(G4Colour(0.1, 0.6, 1));//calorimeter valume -->DodgerBlue1
  G4VisAttributes *sensVisAttributes      = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0));//sensor-->RED
  G4VisAttributes *absVisAttributes       = new G4VisAttributes(G4Colour(1.0, 0.0, 1.0));//absorber-->BLUE, not blue, purple??
  G4VisAttributes *electrodeVisAttributes = new G4VisAttributes(G4Colour(1.0, 1.0, 0.0));//electrode-->YELLOW
  G4VisAttributes *pcbVisAttributes       = new G4VisAttributes(G4Colour(1.0, 0.6, 0.0));//pcb-->ORANGE
  G4VisAttributes *grVisAttributes        = new G4VisAttributes(G4Color(0.5,0.5,0.5));//graphite-->GRAY


  G4double pairsMonitorZ = GlobalStart + dSensor/2.0 ;


  //======================================================== Sensitive Detector =================================================================================
  BeamCalSD01*  bcSD =  new BeamCalSD01("BeamCal", Rinner,Router,crossingAngle,GlobalStart,sPhi,dPhi,nWafers,DAStart,dLayer,nLayers,dSensor,dAbsorber,RSegmentation,pairsMonitorZ,envVolumeLength,envVolumeCenterZ);
  RegisterSensitiveDetector(bcSD);
  //=============================================================================================================================================================

  //-------------------------------------------
  // Build the PairMonitor and Graphite Layer
  //-------------------------------------------

  //The Space for the incoming beampipe
  G4Tubs *CutOutGr = new G4Tubs("CutOutGr",0*mm, incomingTubeRadius+0.2*mm, envVolumeLength, 0*deg, 360*deg);
  G4Tubs *CutOutGr2 = new G4Tubs("CutOutGr2",0*mm, incomingTubeRadius+0.3*mm, envVolumeLength, 0*deg, 360*deg);

  // G4cout << "cutoutRgr: " << cutoutRgr << "   "	 << "cutoutZgr: " << cutoutZgr << "   "	 << G4endl;
  G4Transform3D PMTransform = G4Transform3D(
					    G4RotationMatrix().rotateY(-crossingAngle), 
					    G4ThreeVector(G4ThreeVector(0, 0, GlobalStart+PMLength/2.).rotateY(-crossingAngle)[0],0,0)
					    );

  G4Transform3D PMTransformSens = G4Transform3D(
						G4RotationMatrix().rotateY(-crossingAngle), 
						G4ThreeVector(G4ThreeVector(0, 0, GlobalStart+dSensor/2.).rotateY(-crossingAngle)[0],0,0)
						);
  G4Transform3D PMTransformGraphite = G4Transform3D(
						    G4RotationMatrix().rotateY(-crossingAngle), 
						    G4ThreeVector(G4ThreeVector(0, 0, GlobalStart+PMLength-dGraphite/2.).rotateY(-crossingAngle)[0],0,0)
						    );



  //The PairMonitor will be placed in its own volume together with the graphite 6 mm before BeamCal
  G4Tubs *PairMonitorVolumeSol_1 = new G4Tubs("PairMonitorVolumeSol_1",Rinner, Router, PMLength/2., sPhi, 360*deg);

    //Must use full crossing angle, because we are centering beamcal on outgoing beamline
  G4SubtractionSolid *PairMonitorVolumeSol = new G4SubtractionSolid("PairMonitorVolumeSol", PairMonitorVolumeSol_1, CutOutGr, PMTransform);
  G4LogicalVolume *PairMonitorVolumeLog = new G4LogicalVolume (PairMonitorVolumeSol, materialAir, "PairMonitorVolumeLog",0,0);
  PairMonitorVolumeLog->SetVisAttributes(caloVisAttributes);
  caloVisAttributes->SetDaughtersInvisible(true);
  //  PairMonitorVolumeLog->SetVisAttributes(G4VisAttributes::Invisible);

  if(pairmonitorOnOff == 1) {
    G4Tubs *PairMonitorLayerSol_1 = new G4Tubs("PairMonitorLayerSol_1", Rinner, Router, dSensor/2., sPhi, 360*deg);
    G4SubtractionSolid *PairMonitorLayerSol = new G4SubtractionSolid("PairMonitorLayerSol", PairMonitorLayerSol_1, CutOutGr2, PMTransformSens);
    G4LogicalVolume *PairMonitorLayerLog = new G4LogicalVolume(PairMonitorLayerSol, materialSilicon, "PairMonitorLayerSol",0,0);
    PairMonitorLayerLog->SetVisAttributes(sensVisAttributes);
    //    PairMonitorLayerLog->SetVisAttributes(G4VisAttributes::Invisible);
    new G4PVPlacement(0,G4ThreeVector(0,0, (-PMLength+dSensor)/2.0), PairMonitorLayerLog,"PairMonitorLayerPlaced", PairMonitorVolumeLog,false, nLayers+1, CHECKSURFACE);
    PairMonitorLayerLog->SetSensitiveDetector(bcSD);
  }

  //the graphite layer in the forward region
  if(dGraphite > 0) {
    G4Tubs *GrVolumeSol_1 = new G4Tubs("GrVolumeSol_1", Rinner, Router, dGraphite/2., sPhi,360*deg);
    G4SubtractionSolid *GrVolumeSol = new G4SubtractionSolid("GrVolumeSol", GrVolumeSol_1, CutOutGr2, PMTransformGraphite);
    G4LogicalVolume *GrVolumeLog = new G4LogicalVolume(GrVolumeSol,materialGraphite, "GrVolumeLog",0,0,0);
    new G4PVPlacement(0, G4ThreeVector(0,0, (PMLength-dGraphite)/2.), GrVolumeLog, "GrVolumePlaced", PairMonitorVolumeLog, false, 0, CHECKSURFACE);
    GrVolumeLog->SetVisAttributes(grVisAttributes);  
    //    GrVolumeLog->SetVisAttributes(G4VisAttributes::Invisible);
  }
  if(dGraphite > 0 || pairmonitorOnOff == 1) {
    new G4PVPlacement(transformerPM, PairMonitorVolumeLog, "PairMonitorVolumeForw", worldLog,true, 1, CHECKSURFACE);
    //Place the backward PM stuff			
    new G4PVPlacement(transmirrorPM, PairMonitorVolumeLog, "PairMonitorVolumeBack", worldLog,true, 2, CHECKSURFACE);
  }

  //-------------------------------------------
  //build the calorimeter in the forward region
  //-------------------------------------------

  //Check that the spanning angle is not too large and overlaps with the cutout for the incoming beampipe
  G4double bpminrcalc = (-1*G4ThreeVector(0, 0, BCStart).rotateY(-crossingAngle)[0]);
  G4double cutoutangle = atan(incomingTubeRadius/bpminrcalc);

  if(360*deg-2*cutoutangle < dPhi) {
    Control::Log("BeamCal01: Spanning Angle is too large!");
    G4cout << "ERROR: Spanning Angle too large! Maximum possible angle is " << 360-2*cutoutangle/deg << G4endl;
    return false;
  }

  G4Transform3D  BCtransform(G4RotationMatrix().rotateY(-crossingAngle), G4ThreeVector(G4ThreeVector(0, 0, BCStart+BCLength/2.).rotateY(-crossingAngle)[0],0,0));

  //Don't need the space for the airgap in BeamCalLayer, because we don't place anything there! Taking Tungsten out of beamCalLayer
  G4double dLayerBC = (dLayer-dAirgap-dAbsorber)/2.;

  //Calorimeter volume - a tube filled with air that contains all the layers
  G4Tubs *CaloVolumeSol_1 = new G4Tubs("CaloVolumeSol_1",Rinner, Router, BCLength/2.0, sPhi, 360*deg);
  G4SubtractionSolid *CaloVolumeSol = new G4SubtractionSolid("CaloVolumeSol", CaloVolumeSol_1, CutOutGr, BCtransform);
  G4LogicalVolume *CaloVolumeLog = new G4LogicalVolume (CaloVolumeSol, materialAir, "CaloVolumeLog",0,0);
  CaloVolumeLog->SetVisAttributes(caloVisAttributes);
  //  CaloVolumeLog->SetVisAttributes(G4VisAttributes::Invisible);
  //ONLY FOR DEBUGGING, TURN TO TRUE AFTER!!
  caloVisAttributes->SetDaughtersInvisible(true);
  //  CaloVolumeSol->DumpInfo();


  G4Tubs *BeamCalLayerSol_1 = new G4Tubs("BeamCalLayerSol_1",Rinner, Router, dLayerBC, sPhi, 360*deg);
  G4Tubs *BeamCalLayerSol_DA = new G4Tubs("BeamCalLayerSol_DA",Rinner-1*mm, DArinner, dLayerBC, 360*deg-sPhi, 360*deg-dPhi);
  //  G4SubtractionSolid *BeamCalLayerSol = new G4SubtractionSolid("BeamCalLayerHoled", BeamCalLayerSol_1, CutOutBox, BCtransform);
  G4SubtractionSolid *BeamCalLayerSol = new G4SubtractionSolid("BeamCalLayerHoled", BeamCalLayerSol_1, BeamCalLayerSol_DA, G4Transform3D());
  G4LogicalVolume *BeamCalLayerLog = new G4LogicalVolume (BeamCalLayerSol, materialAir, "BeamCalLayerLog",0,0);
  BeamCalLayerLog->SetVisAttributes(sensVisAttributes);
  //  BeamCalLayerLog->SetVisAttributes(G4VisAttributes::Invisible);

  //Sensor-->nLayers layers of diamond + the Pairs Monitor in front of the graphite block
  G4Tubs *SensorLayerSol_1 = new G4Tubs("SensorLayerSol_1", Rinner, Router, dSensor/2., sPhi, 360*deg);
  G4SubtractionSolid *SensorLayerSol = new G4SubtractionSolid("SensorLayerHoled", SensorLayerSol_1, BeamCalLayerSol_DA, G4Transform3D());
  G4LogicalVolume *SensorLayerLog = new G4LogicalVolume(SensorLayerSol, materialDiamond, "SensorLayerSol",0,0);
  new G4PVPlacement(0,G4ThreeVector(0,0,-dLayerBC+dSensor/2.), SensorLayerLog,"SensorLayerPlaced", BeamCalLayerLog, false, 0, CHECKSURFACE);
  SensorLayerLog->SetVisAttributes(sensVisAttributes);
  //  SensorLayerLog->SetVisAttributes(G4VisAttributes::Invisible);
  SensorLayerLog->SetSensitiveDetector(bcSD);

  // Electrode metalisation of gold
  G4Tubs *ElectrodeLayerSol_1 = new G4Tubs("ElectrodeLayerSol_1", Rinner, Router, dElectrMet/2., sPhi, 360*deg);
  G4SubtractionSolid *ElectrodeLayerSol = new G4SubtractionSolid("ElectrodeLayerHoled", ElectrodeLayerSol_1, BeamCalLayerSol_DA, G4Transform3D());
  G4LogicalVolume *ElectrodeLayerLog = new G4LogicalVolume(ElectrodeLayerSol, materialGold,"ElectrodeLayerLog",0,0);
  new G4PVPlacement (0, G4ThreeVector(0,0,-dLayerBC+dSensor+dElectrMet/2.), ElectrodeLayerLog,"ElectrodeLayerPlaced",BeamCalLayerLog,false, 0, CHECKSURFACE);
  ElectrodeLayerLog->SetVisAttributes(electrodeVisAttributes);
  //  ElectrodeLayerLog->SetVisAttributes(G4VisAttributes::Invisible);
  // KaptonBoard
  G4Tubs *PCBLayerSol_1 = new G4Tubs("PCBLayerSol_1",Rinner, Router, dElboard/2.,sPhi,360*deg);
  G4SubtractionSolid *PCBLayerSol = new G4SubtractionSolid("PCBLayerHoled", PCBLayerSol_1, BeamCalLayerSol_DA, G4Transform3D());
  G4LogicalVolume *PCBLayerLog = new G4LogicalVolume(PCBLayerSol, materialKapton, "PCBLayerLog",0,0);
  new G4PVPlacement(0,G4ThreeVector(0,0,-dLayerBC+dSensor+dElectrMet+dElboard/2.),PCBLayerLog,"PCBLayerPlaced",BeamCalLayerLog,false, 0, CHECKSURFACE);
  PCBLayerLog->SetVisAttributes(pcbVisAttributes);
  //  PCBLayerLog->SetVisAttributes(G4VisAttributes::Invisible);
  //TungstenLayer
  G4Tubs *AbsorberLayerSol_1 = new G4Tubs("AbsorberLayerSol", Rinner, Router, dAbsorber/2., sPhi, 360*deg);

  //Place the BeamCalLayer and the absorber plates in the CaloVolumeLog
  for(int i = 1; i <= nLayers; i++) {
    char index[5]; sprintf(index,"%i",i);
    //    new G4PVPlacement(0, G4ThreeVector(0,0, -BCLength/2. + dLayer*G4double(i-1) + dAbsorber + dLayerBC ), BeamCalLayerLog, G4String("BeamCalLayerPlaced").append(index), CaloVolumeLog, true, i, CHECKSURFACE);
    //Position of the hole depends on layer position!
    G4Transform3D BCtransform2(G4RotationMatrix().rotateY(-crossingAngle), G4ThreeVector(G4ThreeVector(0, 0, BCStart + dAbsorber/2. + dLayer*G4double(i-1)).rotateY(-crossingAngle)[0],0,0));
    G4SubtractionSolid *AbsorberLayerSol = new G4SubtractionSolid(G4String("AbsorberLayerHoled").append(index), AbsorberLayerSol_1, CutOutGr2, BCtransform2);
    G4LogicalVolume *AbsorberLayerLog = new G4LogicalVolume(AbsorberLayerSol,materialTungsten, G4String("AbsorberLayerLog").append(index), 0,0);
    //Place TunsgtenLayer as daughter to CaloVolumeLog
    AbsorberLayerLog->SetVisAttributes(absVisAttributes);  
    //    AbsorberLayerLog->SetVisAttributes(G4VisAttributes::Invisible);
    new G4PVPlacement(0, G4ThreeVector(0,0, -BCLength/2. + dAbsorber/2. + dLayer*G4double(i-1)), AbsorberLayerLog, G4String("AbsorberLayerPlaced").append(index), CaloVolumeLog, true, i, CHECKSURFACE);
    new G4PVPlacement(0, G4ThreeVector(0,0, -BCLength/2. + dLayer*G4double(i-1) + dAbsorber + dLayerBC ), BeamCalLayerLog, G4String("BeamCalLayerPlaced").append(index), CaloVolumeLog, true, i, CHECKSURFACE);
  }

  //Place the Forward calorimeter
  //use copy number to decide which side we are on in SD 1 for forward, 2 for backward
  new G4PVPlacement(transformer, CaloVolumeLog, "CaloVolumeForw", worldLog,true, 1, CHECKSURFACE);
  //Place the backward calorimeter
  new G4PVPlacement(transmirror, CaloVolumeLog, "CaloVolumeBack", worldLog,true, 2, CHECKSURFACE);

  return true;
}