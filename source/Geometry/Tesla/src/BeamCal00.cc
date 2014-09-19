// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: BeamCal00.cc,v 1.7 2008/11/12 16:05:32 steve Exp $
// $Name: mokka-07-00 $
//
// History:
// - first implementation, code from Fcal collaboration,
//     * position of sensitive layer corrected
//     * several constants moved to beamcal table
//     * superdriver by A. Sapronov, A. Rosca and A. Popescu
//     * implemented in Mokka by A. Hartin and S. Aplin, Oct 2008

#include "BeamCal00.hh"

#include "BeamCalSD00.hh"
#include "CGAGeometryManager.hh"
#include "MySQLWrapper.hh"
#include "Control.hh"
#include "CGADefs.h"

#include "assert.h"

#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4PVPlacement.hh"
#include "G4ThreeVector.hh"

#ifdef MOKKA_GEAR
#include "gear/CalorimeterParameters.h"
#include "gearimpl/CalorimeterParametersImpl.h"
#include "gearimpl/LayerLayoutImpl.h"
#include "MokkaGear.h"
#endif
    

INSTANTIATE(BeamCal00)

G4bool BeamCal00::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog)
{
  //worldLog--->The world volume
  //used for the placement of the detectors elements

  // Geometry parameters from the geometry environment and from the database
  
  G4cout << "BeamCal00: Starting" << G4endl;

  //global geometry parameter-->from the environment
  const G4double crossingAngle = env.GetParameterAsDouble("ILC_Main_Crossing_Angle") * mrad; //the xAngle

  //open the appropriate data base according to the crossing angle

  const G4String dbName = env.GetDBName();
  Database *db = new Database(dbName.c_str());

  //get the variabiles from the db

  db->exec("SELECT * FROM `beamcal`;");
  db->getTuple();
  //inner radius of the calorimeter. 10 microns has been added to ensure no overlap with the beampipe due to numerical instability
  const G4double Rinner = db->fetchDouble("Rinner")*mm + 0.01*mm; 
  const G4double Router = db->fetchDouble("Router")*mm; //outer radius of the calorimeter
  const G4double sPhi = db->fetchDouble("sPhi")*deg; //staring angle of the cylinder
  const G4double dPhi = db->fetchDouble("dPhi")*deg; //spanning angle of the cylinder	  
  const G4double nWafers = db->fetchDouble("nWafers"); //number of wafers
  const G4double BPmaxR = db->fetchDouble("BPmaxR")*mm; //beam pipe maximum radius used to define the dead area dimensions
  const G4double zStart = db->fetchDouble("zStart")*mm;
  const G4double dAbsorber = db->fetchDouble("dAbsorber")*mm; // Tungsten thickness
  const G4double dSensor = db->fetchDouble("dSensor")*mm; // Diamond thickness
  const G4double dAirgap = db->fetchDouble("dAirgap")*mm; // Layer gap (air)
  const G4double dElboard = db->fetchDouble("dElboard")*mm; // PCB (Kapton)
  const G4double dElectrMet = db->fetchDouble("dElectrMet")*mm; //Sensor Electrode Metalisation (gold)
  const G4int nLayers = db->fetchInt("nLayers"); //number of layers
  const G4double Segm = db->fetchDouble("Segm")*mm; //dimension of the segments
  const G4double dGraphite = db->fetchDouble("dGraphite")*mm;//dimension of the graphite layer


  G4cout << " BeamCal inner radius : "<< Rinner << " mm " << G4endl;
  G4cout << " BeamCal outer radius : "<< Router << " mm " << G4endl;


  //some constants

  //	dAbsorber = 3.5*mm; // Tungsten thickness
  //	dSensor = 0.3*mm - 4.0E-4*mm; // Diamond thickness
  //	dAirgap = 0.05*mm; // Layer gap (air)
  //	dElboard = 0.15*mm; // PCB (Kapton)
  //	dElectrMet = 4.0E-4*mm; //Sensor Electrode Metalisation (gold)
  //	nLayers = 30; //number of layers
  //	Segm = 8*mm; //dimension of the segments
  //	dGraphite = 100*mm;//dimension of the graphite layer

  if(crossingAngle>=14*mrad)
    {
      tiltForw = -crossingAngle/2.;
      tiltBack = crossingAngle/2.;
    }
  else
    {
      tiltForw = 0.;
      tiltBack = 0.;
    }

  //calculation

  dLayer = dAbsorber+dSensor+dAirgap+dElboard+dElectrMet; //layer thickness
  length = dLayer*nLayers; //total length of the calorimeter
  zCenter = zStart + length/2.;
  
  //SJA: create and overall envelope volume which will include the pair monitor + 3mm clearance + graphite shield + 6mm clearance + calorimeter    
  G4double envVolumeLength = dSensor + 3.*mm  + dGraphite + 6.*mm + length ;
  G4double envVolumeCenterZ = zStart + length - (envVolumeLength/2.0);

  posForw = G4ThreeVector(zCenter*sin(crossingAngle/2.0), 0., zCenter*cos(crossingAngle/2.0)); // x/y/z position of the forward cal
  posBack = G4ThreeVector(zCenter*sin(crossingAngle/2.0), 0., -zCenter*cos(crossingAngle/2.0));// x/y/z position of the backward cal

  G4ThreeVector envVolposForw = G4ThreeVector(envVolumeCenterZ*sin(crossingAngle/2.0), 0., envVolumeCenterZ*cos(crossingAngle/2.0)); // x/y/z position of the forward cal
  G4ThreeVector envVolposBack = G4ThreeVector(envVolumeCenterZ*sin(crossingAngle/2.0), 0., -envVolumeCenterZ*cos(crossingAngle/2.0));// x/y/z position of the backward cal

  dR=split_segm((Router-Rinner),Segm); //distance between 2 rings
  SegmdR=dR;
  nRs=split_n((Router-Rinner),Segm);//number of rings
  SegmnRs=nRs;
  DAStart=0, DArinner=0;

  if(crossingAngle>=14*mrad)
    {
      for(j=0;j<nRs;j++)
	{
	  r=Rinner + j*dR;
	  if(r>=BPmaxR)
	    {
	      DArinner = r;//inner radius of the DA
	      DAStart=j;//number where rings are complete
	      break;
	    }
	
	}
    }

  //  G4cout << "SJA:TAG: DArinner:"  << DArinner << G4endl;

  //materials to be used
  materialAir = CGAGeometryManager::GetMaterial("air");
  materialTungsten  = CGAGeometryManager::GetMaterial("tungsten_19.3gccm");
  materialKapton = CGAGeometryManager::GetMaterial("kapton");
  materialGold = CGAGeometryManager::GetMaterial("gold");
  materialDiamond = CGAGeometryManager::GetMaterial("diamond");
  materialGraphite = CGAGeometryManager::GetMaterial("graphite"); 


  //visualisation attributes
  G4VisAttributes *caloVisAttributes = new G4VisAttributes(G4Colour(0.1, 0.6, 1));//calorimeter valume -->DodgerBlue1
  G4VisAttributes *sensVisAttributes = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0));//sensor-->RED
  G4VisAttributes *absVisAttributes = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0));//absorber-->BLUE
  G4VisAttributes *electrodeVisAttributes = new G4VisAttributes(G4Colour(1.0, 1.0, 0.0));//electrode-->YELLOW
  G4VisAttributes *pcbVisAttributes = new G4VisAttributes(G4Colour(1.0, 0.6, 0.0));//pcb-->ORANGE
  G4VisAttributes *grVisAttributes = new G4VisAttributes(G4Color(0.5,0.5,0.5));//graphite-->GRAY

  //variables
  int i; 
  char index[10];
  const G4int nlayers=30;

  //rotation in the forward & backward region
  G4RotationMatrix *bcRotForw = new G4RotationMatrix();
  G4RotationMatrix *bcRotBack = new G4RotationMatrix();
  bcRotForw->rotateY(tiltForw);
  bcRotBack->rotateY(tiltBack);

  //offset of the graphite shield and of the firs diamond layer
  //G4double grOft= length/2 + dGraphite/2 + 3.*mm;
  G4double grOft= -envVolumeLength/2.0 + dSensor + 3.0*mm + dGraphite/2.0 ;
  G4double pairsMonitorZ = envVolumeCenterZ - envVolumeLength/2.0 + dSensor/2.0 ;

  //position of the graphite
  //  G4ThreeVector grPosForw = G4ThreeVector((grOft)*sin(crossingAngle/2.0), 0., (grOft)*cos(crossingAngle/2.0));
  //  G4ThreeVector grPosBack = G4ThreeVector((grOft)*sin(crossingAngle/2.0), 0., -(grOft)*cos(crossingAngle/2.0));

  //======================================================== Sensitive Detector =================================================================================
  BeamCalSD00*  bcSD =  new BeamCalSD00("BeamCal", Rinner,Router,crossingAngle,zStart,sPhi,dPhi,nWafers,DAStart,dLayer,nLayers,dSensor,dAbsorber,Segm,pairsMonitorZ,envVolumeLength,envVolumeCenterZ);
  RegisterSensitiveDetector(bcSD);
  //=============================================================================================================================================================

  //-------------------------------------------
  //build the calorimeter in the forward region
  //-------------------------------------------

  //Calorimeter volume - a tube filled with air that contains all the layers
  G4Tubs *CaloVolumeSol = new G4Tubs("CaloVolumeSol",Rinner, Router, envVolumeLength/2.0, sPhi, dPhi);
  G4LogicalVolume *CaloVolumeLogForw = new G4LogicalVolume (CaloVolumeSol, materialAir, "CaloVolumeLogForw",0,0);
  new G4PVPlacement(bcRotForw, envVolposForw, CaloVolumeLogForw, "CaloVolumeForw", worldLog,false, 0);
  CaloVolumeLogForw->SetVisAttributes(caloVisAttributes);
  //	caloVisAttributes->SetDaughtersInvisible(true);

  //the graphite layer in the forward region
  G4Tubs *GrVolumeSol = new G4Tubs("GrVolumeSol", Rinner, Router, dGraphite/2, sPhi,dPhi);
  G4LogicalVolume *GrVolumeLogForw = new G4LogicalVolume(GrVolumeSol,materialGraphite, "GrVolumeLogForw",0,0,0);
  new G4PVPlacement(0, G4ThreeVector(0,0,grOft), GrVolumeLogForw, "GrVolumeForw", CaloVolumeLogForw, false,0);
  GrVolumeLogForw->SetVisAttributes(grVisAttributes);  


  //Absorber-->30 layers of tungsten

  G4Tubs *AbsorberLayerSol = new G4Tubs("AbsorberLayerSol", Rinner, Router, dAbsorber/2., sPhi, dPhi);
  G4LogicalVolume *AbsorberLayerLogForw[nlayers];
  for(i=0;i<nlayers;i++)
    {
      sprintf(index,"%d",i);
      AbsorberLayerLogForw[i] = new G4LogicalVolume(AbsorberLayerSol,materialTungsten, G4String("AbsorberLayerLogForw").append(index), 0,0);
      AbsorberLayerLogForw[i]->SetVisAttributes(absVisAttributes);
      posAbs = envVolumeLength/2.0 -length + dAbsorber/2. + i*dLayer;
      new G4PVPlacement(0,G4ThreeVector(0,0,posAbs),AbsorberLayerLogForw[i],G4String("AbsorberLayerForw").append(index),CaloVolumeLogForw,false, 0);
    }

  //Sensor-->30 layers of diamond + the Pairs Monitor in front of the graphite block

  G4Tubs *SensorLayerSol = new G4Tubs("SensorLayerSol", Rinner, Router, dSensor/2., sPhi, dPhi);
  
  G4LogicalVolume *SensorLayerLogForw[nlayers];

  // for the pairs monitor in front to the beamCal set the copy number to nlayers+1 as we number layers from 1 (layer number is from 0)
  G4LogicalVolume *PairsMonitorForwLog = new G4LogicalVolume(SensorLayerSol, materialDiamond, G4String("PairsMonitorForwLog"),0,0);
  PairsMonitorForwLog->SetVisAttributes(sensVisAttributes);
  G4double z =  -envVolumeLength/2.0 + dSensor/2.0 ;
  new G4PVPlacement(0,G4ThreeVector(0,0,z), PairsMonitorForwLog,G4String("PairsMonitorForw"), CaloVolumeLogForw,false, nlayers+1);
  PairsMonitorForwLog->SetSensitiveDetector(bcSD);

  for (i=0;i<nlayers; i++)
    {
      sprintf(index,"%d",i);	       
      SensorLayerLogForw[i] = new G4LogicalVolume(SensorLayerSol, materialDiamond, G4String("SensorLayerForw").append(index),0,0);
      SensorLayerLogForw[i]->SetVisAttributes(sensVisAttributes);
      posSens = envVolumeLength/2.0 - length + dAbsorber + dSensor/2. +i*dLayer;
      new G4PVPlacement(0,G4ThreeVector(0,0,posSens), SensorLayerLogForw[i],G4String("SensorLayerForw").append(index), CaloVolumeLogForw,false, i+1);
      SensorLayerLogForw[i]->SetSensitiveDetector(bcSD);
    } 

  // Electrode metalisation -->30 layers of gold

  G4Tubs *ElectrodeSolid = new G4Tubs("ElectrodeSolid", Rinner, Router, dElectrMet/2., sPhi, dPhi);
  G4LogicalVolume *ElectrodeLogicalForw[nlayers];
  for(i=0;i<nlayers;i++)
    {
      sprintf(index,"%d",i);
      ElectrodeLogicalForw[i] = new G4LogicalVolume(ElectrodeSolid, materialGold,G4String("ElectrodeForw").append(index),0,0);
      ElectrodeLogicalForw[i]->SetVisAttributes(electrodeVisAttributes);
      posElectrode = envVolumeLength/2.0 -length + dAbsorber + dSensor +dElectrMet/2. + i*dLayer;
      new G4PVPlacement (0, G4ThreeVector(0,0,posElectrode), ElectrodeLogicalForw[i],G4String("Electrode").append(index),CaloVolumeLogForw,false, 0);
    }
	
  //PCB--> 30 layers of Kapton

  G4Tubs *PCBSolid = new G4Tubs("PCBSolid",Rinner, Router, dElboard/2.,sPhi,dPhi);
  G4LogicalVolume *PCBLogicalForw[nlayers];
  for(i=0;i<nlayers;i++)
    {
      sprintf(index,"%d",i);
      PCBLogicalForw[i] = new G4LogicalVolume(PCBSolid,materialKapton,G4String("PCBForw").append(index),0,0);
      PCBLogicalForw[i]->SetVisAttributes(pcbVisAttributes);
      posPCB = envVolumeLength/2.0 -length + dAbsorber + dSensor + dElectrMet + dElboard/2. + i*dLayer;
      new G4PVPlacement(0,G4ThreeVector(0,0,posPCB),PCBLogicalForw[i],G4String("PCB").append(index),CaloVolumeLogForw,false,0);
    }

  //=========================================
  //build the detector in the backward region
  //=========================================

  //Calorimeter volume --> a tube filled with air that contains all the layers

  G4LogicalVolume *CaloVolumeLogBack = new G4LogicalVolume(CaloVolumeSol,materialAir,"CaloVolumeLogBack",0,0);
  new G4PVPlacement(bcRotBack,envVolposBack,CaloVolumeLogBack,"CaloVolumeBack",worldLog,false,0);
  CaloVolumeLogBack->SetVisAttributes(caloVisAttributes);

  //graphite layer
  G4LogicalVolume *GrVolumeLogBack = new G4LogicalVolume(GrVolumeSol,materialGraphite, "GrVolumeLogBack",0,0,0);	
  new G4PVPlacement(0,G4ThreeVector(0,0,-grOft),GrVolumeLogBack,"GrVolumeBack", CaloVolumeLogBack, false,0);
  GrVolumeLogBack->SetVisAttributes(grVisAttributes);

  //Absorber -->30 layers of tungsten

  G4LogicalVolume *AbsorberLayerLogBack[nlayers];
  for(i=0;i<nlayers;i++)
    {
      sprintf(index,"%d",i);
      AbsorberLayerLogBack[i] = new G4LogicalVolume(AbsorberLayerSol,materialTungsten,G4String("AbsorberLayerLogBack").append(index),0,0);
      AbsorberLayerLogBack[i]->SetVisAttributes(absVisAttributes);
      pos_absBack = -envVolumeLength/2.0 + length - dAbsorber/2. - i*dLayer;      
      new G4PVPlacement(0,G4ThreeVector(0,0,pos_absBack),AbsorberLayerLogBack[i],G4String("AbsorberLayerBack").append(index),CaloVolumeLogBack, false,0);
    }

  //Sensor-->30 layers of diamond + the Pairs Monitor in front of the graphite block

  G4LogicalVolume *SensorLayerLogBack[nlayers];

  // for the pairs monitor in front to the beamCal set the copy number to nlayers+1 as we number layers from 1 (layer number is from 0)
  G4LogicalVolume *PairsMonitorBackLog = new G4LogicalVolume(SensorLayerSol, materialDiamond, G4String("PairsMonitorBackLog"),0,0);
  PairsMonitorBackLog->SetVisAttributes(sensVisAttributes);
  z =  envVolumeLength/2.0 - dSensor/2.0 ;
  new G4PVPlacement(0,G4ThreeVector(0,0,z), PairsMonitorBackLog,G4String("PairsMonitorBack"), CaloVolumeLogBack,false, -1*(nlayers+1));
  PairsMonitorBackLog->SetSensitiveDetector(bcSD);

  for(i=0;i<nlayers;i++)
    {
      sprintf(index,"%d",i);
      SensorLayerLogBack[i] = new G4LogicalVolume(SensorLayerSol,materialDiamond,G4String("SensorLayerLogBack").append(index),0,0);
      SensorLayerLogBack[i]->SetVisAttributes(sensVisAttributes);
      pos_sensBack = -envVolumeLength/2.0 + length - dAbsorber - dSensor/2. -i*dLayer;
      new G4PVPlacement (0,G4ThreeVector(0,0,pos_sensBack),SensorLayerLogBack[i],G4String("SensorLayerBack").append(index),CaloVolumeLogBack, false,-1*(i+1)); 
      SensorLayerLogBack[i]->SetSensitiveDetector(bcSD);
    }

  //Electrode metalisation --> 30 layers of gold 

  G4LogicalVolume *ElectrodeLayerLogBack[nlayers];
  for(i=0;i<nlayers;i++)
    {
      sprintf(index,"%d",i);
      ElectrodeLayerLogBack[i] = new G4LogicalVolume(ElectrodeSolid,materialGold,G4String("ElectrodeLayerLogBack").append(index),0,0);
      ElectrodeLayerLogBack[i]->SetVisAttributes(electrodeVisAttributes);
      pos_electrodeBack = -envVolumeLength/2.0 + length - dAbsorber - dSensor - dElectrMet/2. - i*dLayer; //????
      new G4PVPlacement(0, G4ThreeVector(0,0,pos_electrodeBack), ElectrodeLayerLogBack[i],G4String("ElectrodeBack").append(index),CaloVolumeLogBack, false,0);
    }

  //PCB --> 30 layers of Kapton

  G4LogicalVolume *PCBLayerLogBack[nlayers];
  for(i=0;i<nlayers;i++)
    {
      sprintf(index,"%d",i);
      PCBLayerLogBack[i] = new G4LogicalVolume(PCBSolid,materialKapton,G4String("PCBLayerLogBack").append(index),0,0);
      PCBLayerLogBack[i]->SetVisAttributes(pcbVisAttributes);
      pcbBack = -envVolumeLength/2.0 + length - dAbsorber - dSensor - dElectrMet/2. - i*dLayer - dElboard/2.;
      new G4PVPlacement(0, G4ThreeVector(0,0,pcbBack), PCBLayerLogBack[i],G4String("PCBBack").append(index),CaloVolumeLogBack, false,0);
    }

  //=================================================
  //DEAD AREA (for the 14mrad and 20mrad xangle !!!)
  //=================================================

  if(crossingAngle>=14*mrad){
    cout<<"DAStart="<<DAStart<<"\n"<<G4endl;

    //=====================================
    //	Forward Calorimeter DA
    //=====================================

    //Calorimeter volume DA

    //    G4VPhysicalVolume *CaloVolumeDAForw;
    G4Tubs *CaloVolumeDASol = new G4Tubs("CaloVolumeDASol",DArinner,Router, envVolumeLength/2.0, sPhi+dPhi, 360*deg-dPhi);
    G4LogicalVolume *CaloVolumeDALogForw = new G4LogicalVolume(CaloVolumeDASol,materialAir, "CaloVolumeDALogForw", 0,0);
    CaloVolumeDALogForw->SetVisAttributes(caloVisAttributes);
    new G4PVPlacement(bcRotForw, envVolposForw, CaloVolumeDALogForw,"CaloVolumeDAForw",worldLog,false,0);
   
    //graphite DA
    G4Tubs *GrVolumeDASol = new G4Tubs("GrVolumeSol", DArinner, Router, dGraphite/2., sPhi+dPhi, 360*deg -dPhi);
    G4LogicalVolume *GrVolumeDALogForw = new G4LogicalVolume(GrVolumeDASol,materialGraphite, "GrVolumeDALogForw",0,0);
    new G4PVPlacement(0, G4ThreeVector(0,0,grOft), GrVolumeDALogForw,"GrVolumeDAForw", CaloVolumeDALogForw, false,0);
    GrVolumeDALogForw->SetVisAttributes(grVisAttributes);
   
    //Absorber DA

    G4Tubs *AbsorberDALayerSol = new G4Tubs("AbsorberDALayerSol",DArinner,Router,dAbsorber/2.,sPhi+dPhi,360*deg-dPhi);
    G4LogicalVolume *AbsorberDALayerLogForw[nlayers];
    for(i=0;i<nlayers;i++)
      {
	sprintf(index,"%d",i);
	AbsorberDALayerLogForw[i] = new G4LogicalVolume(AbsorberDALayerSol,materialTungsten,G4String("AbsorberDALayerLogForw").append(index),0,0,0);
	AbsorberDALayerLogForw[i]->SetVisAttributes(absVisAttributes);
	posAbs = envVolumeLength/2.0 -length + dAbsorber/2. + i*dLayer; 
	new G4PVPlacement(0, G4ThreeVector(0,0,posAbs),AbsorberDALayerLogForw[i],G4String("AbsorberDALayerForw").append(index), CaloVolumeDALogForw,false,0);
      }

   
    //Sensor DA + the Pairs Monitor in front of the graphite block

    G4Tubs *SensorDALayerSol = new G4Tubs("SensorDALayerSol", DArinner, Router, dSensor/2., sPhi+dPhi, 360*deg-dPhi);
    G4LogicalVolume *SensorDALayerLogForw[nlayers]; 

    // for the pairs monitor in front to the beamCal set the copy number to nlayers+1 as we number layers from 1 (layer number is from 0)
    G4LogicalVolume *PairsMonitorDAForwLog  = new G4LogicalVolume(SensorDALayerSol, materialDiamond, G4String("PairsMonitorDAForwLog"),0,0);
    PairsMonitorDAForwLog->SetVisAttributes(sensVisAttributes);
    z = -envVolumeLength/2.0 + dSensor/2.0;
    new G4PVPlacement(0,G4ThreeVector(0,0,z), PairsMonitorDAForwLog,G4String("PairsMonitorDAForw"), CaloVolumeDALogForw,false, nlayers+1);
    PairsMonitorDAForwLog->SetSensitiveDetector(bcSD);

   
    for(i=0;i<nlayers;i++)
      {
	sprintf(index,"%d",i);
	SensorDALayerLogForw[i]=new G4LogicalVolume(SensorDALayerSol,materialDiamond,G4String("SensorDALayerLogForw").append(index),0,0,0);
	SensorDALayerLogForw[i]->SetVisAttributes(sensVisAttributes);
	posSens = envVolumeLength/2.0 - length + dAbsorber + dSensor/2. +i*dLayer;
	new G4PVPlacement(0, G4ThreeVector(0,0,posSens), SensorDALayerLogForw[i], G4String("SensorDALayerForw").append(index), CaloVolumeDALogForw, false, i+1);
	SensorDALayerLogForw[i]->SetSensitiveDetector(bcSD);
      }


    //Electrode metalisation DA

    G4Tubs *ElectrodeDASolid = new G4Tubs("ElectrodeDASolid",DArinner,Router,dElectrMet/2., sPhi+dPhi,360*deg-dPhi);
    G4LogicalVolume *ElectrodeDALogicalForw[nlayers];
    for (i=0;i<nlayers;i++)
      {
	sprintf(index,"%d",i);
	ElectrodeDALogicalForw[i] = new G4LogicalVolume(ElectrodeDASolid,materialGold,G4String("ElectrodeDALogicalForw").append(index),0,0,0);
	ElectrodeDALogicalForw[i]->SetVisAttributes(electrodeVisAttributes);
	posElectrode = envVolumeLength/2.0 -length + dAbsorber + dSensor +dElectrMet/2. + i*dLayer;
	new G4PVPlacement(0,G4ThreeVector(0,0,posElectrode), ElectrodeDALogicalForw[i], G4String("ElectodeDAForw").append(index), CaloVolumeDALogForw, false, 0);
      }

    //PCB DA

    G4Tubs *PCBDASolid = new G4Tubs("PCBDASolid",DArinner,Router,dElboard/2.,sPhi+dPhi,360*deg-dPhi);
    G4LogicalVolume *PCBDALogicalForw[nlayers];
    for(i=0;i<nlayers;i++)
      {
	sprintf(index,"%d",i);
	PCBDALogicalForw[i] = new G4LogicalVolume(PCBDASolid,materialKapton,G4String("PCBDALogicalForw").append(index), 0,0,0);
	PCBDALogicalForw[i]->SetVisAttributes(pcbVisAttributes);
	posPCB = envVolumeLength/2.0 -length + dAbsorber + dSensor + dElectrMet + dElboard/2. + i*dLayer;
	new G4PVPlacement(0,G4ThreeVector(0,0,posPCB),PCBDALogicalForw[i],G4String("PCBForw").append(index), CaloVolumeDALogForw, false, 0);
      }

    //=========================
    //Backward Calorimeter DA
    //=========================

    //Calorimeter volume DA

    G4LogicalVolume *CaloVolumeDALogBack = new G4LogicalVolume(CaloVolumeDASol,materialAir,"CaloVolumeDALogBack",0,0,0);
    CaloVolumeDALogBack->SetVisAttributes(caloVisAttributes);
    new G4PVPlacement(bcRotBack,envVolposBack,CaloVolumeDALogBack,"CaloVolumeDABack",worldLog,false,0);

    //graphite DA
    G4LogicalVolume *GrVolumeDALogBack = new G4LogicalVolume(GrVolumeDASol,materialGraphite, "GrVolumeDALogBack",0,0);
    new G4PVPlacement(0,G4ThreeVector(0,0,-grOft),GrVolumeDALogForw,"GrVolumeDAForw", CaloVolumeDALogBack, false,0);
    GrVolumeDALogBack->SetVisAttributes(grVisAttributes);

    //Absorber DA
    G4LogicalVolume *AbsorberDALayerLogBack[nlayers];
    for(i=0;i<nlayers;i++)
      {
	sprintf(index,"%d",i);
	AbsorberDALayerLogBack[i] = new G4LogicalVolume(AbsorberDALayerSol,materialTungsten,G4String("AbsorberDALayerLogBack").append(index),0,0,0);
	AbsorberDALayerLogBack[i]->SetVisAttributes(absVisAttributes);
	posAbs = -envVolumeLength/2.0 + length - dAbsorber/2. - i*dLayer;    
	new G4PVPlacement (0,G4ThreeVector(0,0,posAbs),AbsorberDALayerLogBack[i],G4String("AbsorberDALayer").append(index),CaloVolumeDALogBack, false,0);
      }

    //Sensor DA + the Pairs Monitor in front of the graphite block

    G4LogicalVolume *SensorDALayerLogBack[nlayers];

    // for the pairs monitor in front to the beamCal set the copy number to nlayers+1 as we number layers from 1 (layer number is from 0)
    G4LogicalVolume *PairsMonitorDABackLog  = new G4LogicalVolume(SensorDALayerSol, materialDiamond, G4String("PairsMonitorDABackLog"),0, 0);
    PairsMonitorDABackLog->SetVisAttributes(sensVisAttributes);
    G4double z =  envVolumeLength/2.0 - dSensor/2.0 ;
    new G4PVPlacement(0,G4ThreeVector(0,0,z), PairsMonitorDABackLog,G4String("PairsMonitorDABack"), CaloVolumeDALogBack,false,-1*(nlayers+1));
    PairsMonitorDABackLog->SetSensitiveDetector(bcSD);


    for(i=0;i<nlayers;i++)
      {
	sprintf(index,"%d",i);
	SensorDALayerLogBack[i] = new G4LogicalVolume(SensorDALayerSol, materialDiamond, G4String("SensorDALayerLogBack").append(index),0,0,0);
	SensorDALayerLogBack[i]->SetVisAttributes(sensVisAttributes);
	posSens = -envVolumeLength/2.0 + length - dAbsorber - dSensor/2. -i*dLayer;
	new G4PVPlacement(0, G4ThreeVector(0,0,posSens), SensorDALayerLogBack[i], G4String("SensorDALayerBack").append(index),CaloVolumeDALogBack, false,-1*(i+1));
	SensorDALayerLogBack[i]->SetSensitiveDetector(bcSD);
      }

    //Electrode DA

    G4LogicalVolume *ElectrodeDALogBack[nlayers];
    for(i=0;i<nlayers;i++)
      {
	sprintf(index,"%d",i);
	ElectrodeDALogBack[i] = new G4LogicalVolume(ElectrodeDASolid,materialGold,G4String("ElecrodeDALogBack").append(index),0,0,0);
	ElectrodeDALogBack[i]->SetVisAttributes(electrodeVisAttributes);
	posElectrode = -envVolumeLength/2.0 + length - dAbsorber - dSensor - dElectrMet/2. - i*dLayer;
	new G4PVPlacement(0, G4ThreeVector(0,0,posElectrode), ElectrodeDALogBack[i],G4String("ElectrodeDA").append(index),CaloVolumeDALogBack, false,0);
      }

    //PCB DA

    G4LogicalVolume *PCBDALogBack[nlayers];
    for(i=0;i<nlayers;i++)
      {
	sprintf(index,"%d",i);
	PCBDALogBack[i] = new G4LogicalVolume(PCBDASolid, materialKapton, G4String("PCBDALogBack").append(index),0,0,0);
	PCBDALogBack[i]->SetVisAttributes(pcbVisAttributes);
	posPCB = -envVolumeLength/2.0 + length - dAbsorber - dSensor - dElectrMet/2. - i*dLayer - dElboard/2.;
	new G4PVPlacement(0, G4ThreeVector(0,0,posPCB), PCBDALogBack[i], G4String("PCBDABack").append(index),CaloVolumeDALogBack, false,0);
      }

    //============================================
    //set the sensitive detector for the dead area
    //============================================
//    for(i=0; i<nLayers; i++)
//      {
//	SensorDALayerLogForw[i]->SetSensitiveDetector(bcSD);
//	SensorDALayerLogBack[i]->SetSensitiveDetector(bcSD);
//      }

  }//end if

  //=====
  // set SD
  //=====
//  for(i=0; i<nLayers; i++)
//    {
//      SensorLayerLogForw[i]->SetSensitiveDetector(bcSD);
//      SensorLayerLogBack[i]->SetSensitiveDetector(bcSD);
//    }
	
  delete db;


//#ifdef MOKKA_GEAR
//  // ++++++++++++++++++++++++++++++++++++++++++++++++++++
//  // +  MOKKA GEAR                                      +
//  // ++++++++++++++++++++++++++++++++++++++++++++++++++++
//	
//  const G4double zMin = posForw.z() - length/2.0; 
//
//  gear::CalorimeterParametersImpl* bcalParam = new gear::CalorimeterParametersImpl(Rinner, Router, zMin, 2, 0.);
//	
//  for (int i=0; i < nlayers; i++)
//    {
//      //SJA:FIXME: Phi segmentation still to be set. Nominally set to 1.0 here for now.
//      bcalParam->layerLayout().positionLayer(0.0, dLayer, Segm, 1.0, dAbsorber);
//    }
//	
//  bcalParam->setDoubleVal("beam_crossing_angle", crossingAngle ) ;
//  bcalParam->setDoubleVal("cylinder_staring_phi", sPhi ) ;
//  bcalParam->setDoubleVal("cylinder_spanning_phi", dPhi ) ;
//  bcalParam->setDoubleVal("dead_area_outer_r", DArinner ) ;
//  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
//  gearMgr->setBeamCalParameters ( bcalParam );
//#endif



  return true;
}
