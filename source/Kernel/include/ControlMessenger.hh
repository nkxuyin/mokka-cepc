// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: ControlMessenger.hh,v 1.34 2009/05/13 08:22:57 steve Exp $
// $Name: mokka-07-00 $

#ifndef ControlMessenger_h
#define ControlMessenger_h 1

#include "globals.hh"
#include "G4UImessenger.hh"
class G4UIdirectory;
class G4UIcmdWithABool;
class G4UIcmdWithAnInteger;
class G4UIcmdWithADouble;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithAString;

class Control;

class ControlMessenger: public G4UImessenger
{
public:
  ControlMessenger(Control *pControl);
  ~ControlMessenger(void);
  void SetNewValue(G4UIcommand *command, G4String newValues);
private:
  Control *theControl;

  G4UIdirectory        *MokkaDirectory;
  G4UIcmdWithABool     *drawEventCmd;
  G4UIcmdWithAnInteger *loadFileCmd;

  G4UIdirectory        *VisuDirectory;

  // commands for steering files
  G4UIdirectory        *_initDir;
  G4UIcmdWithAString   *_dbHostCmd;
  G4UIcmdWithAString   *_userCmd;
  G4UIcmdWithAString   *_dbPasswdCmd;
  G4UIcmdWithAString   *_detectorModelCmd;
  G4UIcmdWithAString   *_modelsDBNameCmd;
  G4UIcmdWithAString   *_materialsDBNameCmd;
  G4UIcmdWithAString   *_detectorSetupCmd;
  G4UIcmdWithAString   *_subDetectorCmd;
  G4UIcmdWithAString   *_initialMacroFileCmd;
  G4UIcmdWithAString   *_outDirNameCmd;
  G4UIcmdWithAString   *_lcioFilenameCmd;
  G4UIcmdWithAString   *_lcioEventParameterCmd;
  G4UIcmdWithAString   *_lcioWriteModeCmd;
  G4UIcmdWithAString   *_pythiaFilenameCmd; 

  G4UIcmdWithABool     * _batchMode;// Control::BATCH_MODE = false;
  G4UIcmdWithABool     *_dumpG3Cmd; // Control::DUMPG3 = false;
  G4UIcmdWithABool     *_savingTrajectoriesCmd; // Control::SavingTrajectories = true;
  G4UIcmdWithABool     *_savingPrimariesCmd; // Control::SavingPrimaries = true;
  G4UIcmdWithABool     *_visumodeCmd; // Control::VISUMODE = false;
  G4UIcmdWithADouble   *_BFactorCmd; // Control::BFactor = 1;
  G4UIcmdWithADoubleAndUnit *_rangeCutCmd; // Control::RangeCut = 0.005 * mm;
  G4UIcmdWithADoubleAndUnit *_activeRangeCutCmd;//Control::ActiveRangeCut=0.005 * mm;
  G4UIcmdWithADoubleAndUnit *_radiatorRangeCutCmd;//Control::RadiatorRangeCut=0.005 * mm;
  G4UIcmdWithADoubleAndUnit *_pcbRangeCutCmd;//Control::PCBRangeCut=0.005 * mm;
  G4UIcmdWithADoubleAndUnit *_TPCCutCmd; // Control::TPCCut = 10 * MeV;

  //SJA: Added limitation of the step length in the TPC for very low pt charged particles
  G4UIcmdWithADoubleAndUnit *_TPCLowPtCutCmd; // Control::TPCLowPtCut = 10.0 * MeV;
  G4UIcmdWithABool *_TPCLowPtStepLimitCmd; // Control::TPCLowPtStepLimit = false; 
  G4UIcmdWithABool *_TPCLowPtStoreMCPForHitsCmd; // Control::TPCLowPtStoreMCPForHits = false; 
  G4UIcmdWithADoubleAndUnit *_TPCLowPtMaxStepLengthCmd; // Control::TPCLowPtMaxStepLength = 1.0 * mm;
  G4UIcmdWithADoubleAndUnit *_TPCLowPtMaxHitSeparationCmd; // Control::TPCLowPtMaxHitSeparation = 5.0 * mm;
  
  //SJA: Add control over TrackingPhysicsList
  G4UIcmdWithABool *_TrackingPhysicsListMSOnCmd; // Control::TrackingPhysicsListMSOn = true;
  G4UIcmdWithABool *_TrackingPhysicsListELossOnCmd; // Control::TrackingPhysicsListELossOn = true;
  
  G4UIcmdWithAString   *_activatePluginCmd;
  G4UIcmdWithAString   *_physicsListNameCmd; 
  G4UIcommand          *_userInitStringCmd; 
  G4UIcommand          *_userInitDoubleCmd; 
  G4UIcommand          *_userInitIntCmd;
  G4UIcommand          *_userInitBoolCmd;
  G4UIcmdWithABool     *_lcioDetailedShowerModeCmd; // Control::LCIODetailedShowerMode

  G4UIcmdWithABool     *_useOldHEPLCIOCmd; // Control::USE_OLD_HEPLCIO

  G4UIcmdWithABool     *_fixStdHepLeptonFSRCmd ; // Control::FixStdHepLeptonFSR

  G4UIcmdWithABool     *_lcioWriteCompleteHepEvt; // Control::LCIOWriteCompleteHepEvt
  G4UIcmdWithABool     *_lcioWriteParentsForBackscatter ;
  G4UIcmdWithADoubleAndUnit *_configAngleCmd; // Control::ConfigAngle = 100000;
  G4UIcmdWithADoubleAndUnit *_lorentzTransformationAngleCmd;
  G4UIcmdWithADoubleAndUnit *_primaryVertexSpreadZCmd;
  G4UIcommand          *_CGAGeometryGlobalSetupCmd; // setup global geometry parameters
  G4UIcmdWithAnInteger *_startEventNumberCmd;
  G4UIcmdWithAnInteger *_printLevelCmd;
  G4UIcommand   *_outputMomentumInTrackerHits;
  G4UIcommand   *_detailedHitCollection; //Lukasz Maczewski
  G4UIcmdWithABool     *_lcioStorePositionCmd; // Control::LCIOStorePosition = true;

  // commands to change the detector geometry 
  G4UIcommand   *_addSubDetectorCmd;
  G4UIcommand   *_rmSubDetectorCmd;

  // commands for MokkaGear (not in ifdef enviroment so 
  // user can be informed about building w/o GEAR
  G4UIcmdWithAString   *_mokkaGearFileNameCmd ;
  G4UIcmdWithAString   *_mokkaGearMergeSourceCmd ;

  G4UIcmdWithAnInteger *_randomSeedCmd;
  G4UIcmdWithAnInteger *_pairParticlesPerEventCmd ;

  G4UIcmdWithAnInteger *_dataRunNumberCmd;
  G4UIcmdWithAnInteger *_mcRunNumberCmd;
  G4UIcmdWithAString   *_confDataTagCmd;

  G4UIcmdWithADoubleAndUnit *_userDeltaIntersectionCmd; 
  G4UIcmdWithADoubleAndUnit *_userDeltaOneStepCmd; 

  // for custom particle table
  G4UIcmdWithAString   *_PDGFileCmd;

};

#endif
