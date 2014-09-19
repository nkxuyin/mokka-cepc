// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: ControlMessenger.cc,v 1.47 2009/05/13 08:22:57 steve Exp $
// $Name: mokka-07-00 $

#include "ControlMessenger.hh"
#include "Control.hh"

#include "globals.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithADouble.hh" 
#include "G4UIcmdWithADoubleAndUnit.hh" 
#include "G4UIcmdWithAString.hh"
#include "G4Tokenizer.hh"

#include "Randomize.hh"

#include "PluginManager.hh"
#include "UserInit.hh"


ControlMessenger::ControlMessenger(Control *pControl): theControl(pControl)
{
  MokkaDirectory = new G4UIdirectory("/Mokka/");
  MokkaDirectory->SetGuidance("Mokka specific commands.");

  drawEventCmd = new G4UIcmdWithABool("/Mokka/Draw", this);
  drawEventCmd->SetGuidance("Set drawFlag for end of events. (default is false)");
  drawEventCmd->SetParameterName("Draw", false);
  drawEventCmd->SetDefaultValue(false);

  loadFileCmd  = new G4UIcmdWithAnInteger("/Mokka/LoadEvent", this);
  loadFileCmd->SetGuidance("Loads a simulated event. (depending on the persistency mode)");
  loadFileCmd->SetParameterName("EventNumber", false);

  VisuDirectory  = new G4UIdirectory("/Mokka/Visu/");
  VisuDirectory->SetGuidance("Visualization commands.");

  _initDir = new G4UIdirectory("/Mokka/init/");
  _initDir->SetGuidance("Mokka initialization commands.");

  _detectorModelCmd = new G4UIcmdWithAString("/Mokka/init/detectorModel", this);
  _detectorModelCmd->SetGuidance("The detector model to be used as defined in the models database.");
  _detectorModelCmd->SetParameterName("detectorModel", false);
  _detectorModelCmd->AvailableForStates(G4State_PreInit);
  
  _modelsDBNameCmd = new G4UIcmdWithAString("/Mokka/init/modelsDBName", this);
  _modelsDBNameCmd->SetGuidance("The models database name on the MySQL server to be used.");
  _modelsDBNameCmd->SetParameterName("modelsDBName", false);
  _modelsDBNameCmd->AvailableForStates(G4State_PreInit);

  _materialsDBNameCmd = new G4UIcmdWithAString("/Mokka/init/materialsDBName", this);
  _materialsDBNameCmd->SetGuidance("The models database name on the MySQL server to be used.");
  _materialsDBNameCmd->SetParameterName("materialsDBName", false);
  _materialsDBNameCmd->AvailableForStates(G4State_PreInit);

  _detectorSetupCmd = new G4UIcmdWithAString("/Mokka/init/detectorSetup", this);
  _detectorSetupCmd->SetGuidance("The detector setup to be used as defined in the models database.");
  _detectorSetupCmd->SetParameterName("detectorSetup", false);
  _detectorSetupCmd->AvailableForStates(G4State_PreInit);
  
  _dbHostCmd = new G4UIcmdWithAString("/Mokka/init/dbHost", this);
  _dbHostCmd->SetGuidance("The host machine where the MySQL server is running.");
  _dbHostCmd->SetParameterName("dbHost", false);
  _dbHostCmd->AvailableForStates(G4State_PreInit);

  _userCmd = new G4UIcmdWithAString("/Mokka/init/user", this);
  _userCmd->SetGuidance("The user name to connect to the MySQL server.");
  _userCmd->SetParameterName("user", false);
  _userCmd->AvailableForStates(G4State_PreInit);

  _dbPasswdCmd = new G4UIcmdWithAString("/Mokka/init/dbPasswd", this);
  _dbPasswdCmd->SetGuidance("The password to connect to the MySQL server.");
  _dbPasswdCmd->SetParameterName("dbPasswd", false);
  _dbPasswdCmd->AvailableForStates(G4State_PreInit);
  
  _subDetectorCmd = new G4UIcmdWithAString("/Mokka/init/subDetector", this);
  _subDetectorCmd->SetGuidance("Specifies just a sub detector name to be built.");
  _subDetectorCmd->SetParameterName("subDetector", false);
  _subDetectorCmd->AvailableForStates(G4State_PreInit);

  _initialMacroFileCmd = new G4UIcmdWithAString("/Mokka/init/initialMacroFile", this);
  _initialMacroFileCmd->SetGuidance("Name of the initial macro file to be executed after startup.");
  _initialMacroFileCmd->SetParameterName("initialMacroFile", false);
  _initialMacroFileCmd->AvailableForStates(G4State_PreInit);
  
  _outDirNameCmd = new G4UIcmdWithAString("/Mokka/init/outDirName", this);
  _outDirNameCmd->SetGuidance("Name of the output directory - this implies ASCII output!");
  _outDirNameCmd->SetParameterName("outDirName", false);
  _outDirNameCmd->AvailableForStates(G4State_PreInit);

  _lcioFilenameCmd = new G4UIcmdWithAString("/Mokka/init/lcioFilename", this);
  _lcioFilenameCmd->SetGuidance("Name of LCIO output file - this implies LCIO output!");
  _lcioFilenameCmd->SetParameterName("lcioFilename", false);
  _lcioFilenameCmd->AvailableForStates(G4State_PreInit);

  _lcioEventParameterCmd = new G4UIcmdWithAString("/Mokka/init/lcioEventParameter", this);
  _lcioEventParameterCmd->SetGuidance("parameter added to every LCIO event - usage: type[int,float,string] name value");
  _lcioEventParameterCmd->SetParameterName("lcioEventParameter", false);
  _lcioEventParameterCmd->AvailableForStates(G4State_PreInit);
  
  
  _lcioWriteModeCmd = new G4UIcmdWithAString("/Mokka/init/lcioWriteMode", this);
  _lcioWriteModeCmd->SetGuidance("Write mode of LCIO output file: \"WRITE_APPEND\" or \"WRITE_NEW\".");
  _lcioWriteModeCmd->SetGuidance("If not specified, an existing file with the given name causes an abort.");
  _lcioWriteModeCmd->SetParameterName("lcioWriteMode", false);
  _lcioWriteModeCmd->SetCandidates("WRITE_APPEND WRITE_NEW");
  _lcioWriteModeCmd->AvailableForStates(G4State_PreInit);

  _pythiaFilenameCmd = new G4UIcmdWithAString("/Mokka/init/pythiaFilename", this);
  _pythiaFilenameCmd->SetGuidance("Name of PYTHIA input file.");
  _pythiaFilenameCmd->SetParameterName("pythiaFilename", false);
  _pythiaFilenameCmd->AvailableForStates(G4State_PreInit);

  _batchMode = new G4UIcmdWithABool("/Mokka/init/BatchMode", this);
  _batchMode->SetGuidance("No interactive session if in batch mode. Just executes a given macro file, if any.");
  _batchMode->SetParameterName("batchMode", false);
  _batchMode->AvailableForStates(G4State_PreInit);

  _dumpG3Cmd = new G4UIcmdWithABool("/Mokka/init/dumpG3", this);
  _dumpG3Cmd->SetGuidance("Specifies BRAHMS backward facility. (generates Geant3 Fortran code)");
  _dumpG3Cmd->SetParameterName("dumpG3", false);
  _dumpG3Cmd->AvailableForStates(G4State_PreInit);

  _savingTrajectoriesCmd = new G4UIcmdWithABool("/Mokka/init/savingTrajectories", this);
  _savingTrajectoriesCmd->SetGuidance("Specifies whether to save primary trajectories. (default is true)");
  _savingTrajectoriesCmd->SetGuidance("Only if \"/init/outDirName\" is set (ASCII mode)");
  _savingTrajectoriesCmd->SetParameterName("savingTrajectories", false);
  _savingTrajectoriesCmd->AvailableForStates(G4State_PreInit);

  _savingPrimariesCmd = new G4UIcmdWithABool("/Mokka/init/savingPrimaries", this);
  _savingPrimariesCmd->SetGuidance("Specifies whether to save primaries. (default is true)");
  _savingPrimariesCmd->SetGuidance("Only if \"/init/outDirName\" is set (ASCII mode)");
  _savingPrimariesCmd->SetParameterName("savingPrimaries", false);
  _savingPrimariesCmd->AvailableForStates(G4State_PreInit);

  _visumodeCmd = new G4UIcmdWithABool("/Mokka/init/visumode", this);
  _visumodeCmd->SetGuidance("Specifies whether to start in visualization mode.");
  _visumodeCmd->SetGuidance("Only if \"/init/outDirName\" is set to an existing output directory (ASCII mode)");
  _visumodeCmd->SetParameterName("visumode", false);
  _visumodeCmd->AvailableForStates(G4State_PreInit);

  _BFactorCmd = new G4UIcmdWithADouble("/Mokka/init/BFactor", this);
  _BFactorCmd->SetGuidance("Specifies a magnetic field factor. (0 to 1)");
  _BFactorCmd->SetParameterName("BFactor", false);
  _BFactorCmd->SetRange("BFactor >= 0.");
  _BFactorCmd->AvailableForStates(G4State_PreInit);

//GM: new cmds requested by Steve for the FieldMgr
  _userDeltaIntersectionCmd=
      new G4UIcmdWithADoubleAndUnit("/Mokka/init/userDeltaIntersection",this);
  _userDeltaIntersectionCmd->SetGuidance("Changes delta intersection parameter of the field manager");
  _userDeltaIntersectionCmd->SetParameterName("UserDeltaIntersection",false);
  _userDeltaIntersectionCmd->SetDefaultUnit("mm");
  _userDeltaIntersectionCmd->AvailableForStates(G4State_PreInit);

  _userDeltaOneStepCmd=
      new G4UIcmdWithADoubleAndUnit("/Mokka/init/userDeltaOneStep",this);
  _userDeltaOneStepCmd->SetGuidance("Changes DeltaOneStep parameter of the field manager");
  _userDeltaOneStepCmd->SetParameterName("UserDeltaOneStep",false);
  _userDeltaOneStepCmd->SetDefaultUnit("mm");
  _userDeltaOneStepCmd->AvailableForStates(G4State_PreInit);

  _rangeCutCmd = new G4UIcmdWithADoubleAndUnit("/Mokka/init/rangeCut", this);
  _rangeCutCmd->SetGuidance(G4String("Specifies the production Geant4 range cut. "
    "(default is " + DtoS(Control::RangeCut / mm) + " mm)"));
  _rangeCutCmd->SetParameterName("rangeCut", false);
  _rangeCutCmd->AvailableForStates(G4State_PreInit);

  _pcbRangeCutCmd = new G4UIcmdWithADoubleAndUnit("/Mokka/init/pcbRangeCut", this);
  _pcbRangeCutCmd->SetGuidance(G4String("Specifies the production Geant4 range cut in the PCB. "
    "(default is " + DtoS(Control::PCBRangeCut / mm) + " mm)"));
  _pcbRangeCutCmd->SetParameterName("pcbRangeCut", false);
  _pcbRangeCutCmd->AvailableForStates(G4State_PreInit);

  _radiatorRangeCutCmd = new G4UIcmdWithADoubleAndUnit("/Mokka/init/radiatorRangeCut", this);
  _radiatorRangeCutCmd->SetGuidance(G4String("Specifies the production Geant4 range cut in the radiator. "
    "(default is " + DtoS(Control::RadiatorRangeCut / mm) + " mm)"));
  _radiatorRangeCutCmd->SetParameterName("radiatorRangeCut", false);
  _radiatorRangeCutCmd->AvailableForStates(G4State_PreInit);

  _activeRangeCutCmd = new G4UIcmdWithADoubleAndUnit("/Mokka/init/activeRangeCut", this);
  _activeRangeCutCmd->SetGuidance(G4String("Specifies the production Geant4 range cut in the active material. "
    "(default is " + DtoS(Control::ActiveRangeCut / mm) + " mm)"));
  _activeRangeCutCmd->SetParameterName("activeRangeCut", false);
  _activeRangeCutCmd->AvailableForStates(G4State_PreInit);

  _TPCCutCmd = new G4UIcmdWithADoubleAndUnit("/Mokka/init/TPCCut", this);
  _TPCCutCmd->SetGuidance(G4String("Specifies the TPC primary energy cut. "
    "(default is " + DtoS(Control::TPCCut / MeV) + " MeV)"));
  _TPCCutCmd->SetParameterName("TPCCut", false);
  _TPCCutCmd->AvailableForStates(G4State_PreInit);

  _TrackingPhysicsListMSOnCmd = new G4UIcmdWithABool("/Mokka/init/TrackingPhysicsListMSOn", this) ;
  _TrackingPhysicsListMSOnCmd->SetGuidance(G4String("Turns on Multiple Scattering in the TrackingPhysicsList, default is ON"));
  _TrackingPhysicsListMSOnCmd->SetParameterName("TrackingPhysicsListMSOn", true);
  _TrackingPhysicsListMSOnCmd->AvailableForStates(G4State_PreInit);
  
  _TrackingPhysicsListELossOnCmd = new G4UIcmdWithABool("/Mokka/init/TrackingPhysicsListELossOn", this) ;
  _TrackingPhysicsListELossOnCmd->SetGuidance(G4String("Turns on EM Energy Loss in the TrackingPhysicsList, default is ON"));
  _TrackingPhysicsListELossOnCmd->SetParameterName("TrackingPhysicsListELossOn", true);
  _TrackingPhysicsListELossOnCmd->AvailableForStates(G4State_PreInit);
  
  _TPCLowPtCutCmd = new G4UIcmdWithADoubleAndUnit("/Mokka/init/TPCLowPtCut", this);
  _TPCLowPtCutCmd->SetGuidance(G4String("Specifies the Pt threshold above which hits are produced using the pad-ring doublet volumes in the TPC. Below this value of Pt, hits will only be produced if TPCLowPtStepLimit is true. In which case a step limiter will be employed for these low Pt tracks, and a hit created once TPCLowPtHitSeparation is exceeded"
                                                 "(default is " + DtoS(Control::TPCLowPtCut / MeV) + " MeV)"));
  _TPCLowPtCutCmd->SetParameterName("TPCLowPtCut", false);
  _TPCLowPtCutCmd->AvailableForStates(G4State_PreInit);

  _TPCLowPtStepLimitCmd = new G4UIcmdWithABool("/Mokka/init/TPCLowPtStepLimit", this);
  _TPCLowPtStepLimitCmd->SetGuidance(G4String("Turns on the creation of hits caused by tracks with Pt less than TPCLowPtCut, and activates the Pt dependant Step Limmiter in the TPC. default is false"));
  _TPCLowPtStepLimitCmd->SetParameterName("TPCLowPtStepLimit", false);
  _TPCLowPtStepLimitCmd->AvailableForStates(G4State_PreInit);

  _TPCLowPtStoreMCPForHitsCmd = new G4UIcmdWithABool("/Mokka/init/TPCLowPtStoreMCPForHits", this);
  _TPCLowPtStoreMCPForHitsCmd->SetGuidance(G4String("Turns on the storing of MCParticles for hits caused by tracks with Pt less than TPCLowPtCut, default is false"));
  _TPCLowPtStoreMCPForHitsCmd->SetGuidance("Only active if \"/Mokka/init/TPCLowPtStepLimit\" is set true");
  _TPCLowPtStoreMCPForHitsCmd->SetParameterName("TPCLowPtStoreMCPForHits", false);
  _TPCLowPtStoreMCPForHitsCmd->AvailableForStates(G4State_PreInit);

  _TPCLowPtMaxStepLengthCmd = new G4UIcmdWithADoubleAndUnit("/Mokka/init/TPCLowPtMaxStepLength", this);
  _TPCLowPtMaxStepLengthCmd->SetGuidance(G4String("Specifies the Max step length for the Pt dependant Step Limmiter. "
                                                  "(default is " + DtoS(Control::TPCLowPtMaxStepLength / mm) + " mm)"));
  _TPCLowPtMaxStepLengthCmd->SetGuidance("Only active if \"/Mokka/init/TPCLowPtStepLimit\" is set true");
  _TPCLowPtMaxStepLengthCmd->SetParameterName("TPCLowPtMaxStepLength", false);
  _TPCLowPtMaxStepLengthCmd->AvailableForStates(G4State_PreInit);
  
  _TPCLowPtMaxHitSeparationCmd = new G4UIcmdWithADoubleAndUnit("/Mokka/init/TPCLowPtMaxHitSeparation", this);
  _TPCLowPtMaxHitSeparationCmd->SetGuidance(G4String("Specifies the max distance between those hits with pt less than TPCLowPtCut. "
                                                  "(default is " + DtoS(Control::TPCLowPtMaxHitSeparation / mm) + " mm)"));
  _TPCLowPtMaxHitSeparationCmd->SetGuidance("Only active if \"/Mokka/init/TPCLowPtStepLimit\" is set true");
  _TPCLowPtMaxHitSeparationCmd->SetParameterName("TPCLowPtMaxHitSeparation", false);
  _TPCLowPtMaxHitSeparationCmd->AvailableForStates(G4State_PreInit);

  _activatePluginCmd = new G4UIcmdWithAString("/Mokka/init/registerPlugin", this);
  _activatePluginCmd->SetGuidance("Activate the plugin with the given name to be executed at runtime.");
  _activatePluginCmd->SetGuidance("All plugins will be called in the order they have been activated.");
  _activatePluginCmd->SetParameterName("PluginName", false);
  _activatePluginCmd->AvailableForStates(G4State_PreInit);

  _physicsListNameCmd = new G4UIcmdWithAString("/Mokka/init/physicsListName", this);
  _physicsListNameCmd->SetGuidance("Specify the name of the physics list to be used for the simulation.");
  _physicsListNameCmd->SetGuidance("Available are all default physics lists provided by Geant4,");
  _physicsListNameCmd->SetGuidance("e.g. LHEP, QGSP, ... (default is \"QGSP_BERT\")");
  _physicsListNameCmd->SetParameterName("PhysicsListName", false);
  _physicsListNameCmd->AvailableForStates(G4State_PreInit);

  _userInitStringCmd = new G4UIcommand("/Mokka/init/userInitString", this);
  _userInitStringCmd->SetGuidance("Define a user variable of type string.");
  _userInitStringCmd->SetParameter(new G4UIparameter("name", 's', false));
  _userInitStringCmd->SetParameter(new G4UIparameter("value", 's', false));
  _userInitStringCmd->AvailableForStates(G4State_PreInit);

  _userInitDoubleCmd = new G4UIcommand("/Mokka/init/userInitDouble", this);
  _userInitDoubleCmd->SetGuidance("Define a user variable of type double. (with optional unit)");
  _userInitDoubleCmd->SetParameter(new G4UIparameter("name", 's', false));
  _userInitDoubleCmd->SetParameter(new G4UIparameter("value", 'd', false));
  _userInitDoubleCmd->SetParameter(new G4UIparameter("unit", 's', true));
  _userInitDoubleCmd->AvailableForStates(G4State_PreInit);

  _userInitIntCmd = new G4UIcommand("/Mokka/init/userInitInt", this);
  _userInitIntCmd->SetGuidance("Define a user variable of type int.");
  _userInitIntCmd->SetParameter(new G4UIparameter("name", 's', false));
  _userInitIntCmd->SetParameter(new G4UIparameter("value", 'i', false));
  _userInitIntCmd->AvailableForStates(G4State_PreInit);
  
  _userInitBoolCmd = new G4UIcommand("/Mokka/init/userInitBool", this);
  _userInitBoolCmd->SetGuidance("Define a user variable of type bool.");
  _userInitBoolCmd->SetParameter(new G4UIparameter("name", 's', false));
  _userInitBoolCmd->SetParameter(new G4UIparameter("value", 'b', false));
  _userInitBoolCmd->AvailableForStates(G4State_PreInit);

  _lcioDetailedShowerModeCmd = new G4UIcmdWithABool("/Mokka/init/lcioDetailedShowerMode", this);
  _lcioDetailedShowerModeCmd->SetGuidance("If true, LCIO file will contain detailed MC contribution");
  _lcioDetailedShowerModeCmd->SetGuidance("from secondaries in calorimeter showers,");
  _lcioDetailedShowerModeCmd->SetGuidance("i.e. energy, PDG and time of the secondary contributing to the hit.");
  _lcioDetailedShowerModeCmd->SetParameterName("LCIODetailedShowerMode", false);
  _lcioDetailedShowerModeCmd->AvailableForStates(G4State_PreInit);


  _useOldHEPLCIOCmd = new G4UIcmdWithABool("/Mokka/init/useOldHEPLCIO", this);
  _useOldHEPLCIOCmd->SetGuidance("If true, we use the old scheme of reading hepevt/stdhep files - debug option");
  _useOldHEPLCIOCmd->SetParameterName("UseOldHEPLCIO", false);
  _useOldHEPLCIOCmd->AvailableForStates(G4State_PreInit);


  _fixStdHepLeptonFSRCmd = new G4UIcmdWithABool("/Mokka/init/FixStdHepLeptonFSR", this);
  _fixStdHepLeptonFSRCmd->SetGuidance("If true, a fix is applied when reading in stdhep files,");
  _fixStdHepLeptonFSRCmd->SetGuidance("that avoids double counting of leptons in the case of");
  _fixStdHepLeptonFSRCmd->SetGuidance("FSR on the lepton. Default value is on.");
  _fixStdHepLeptonFSRCmd->SetParameterName("FixStdHepLeptonFSR", true );
  _fixStdHepLeptonFSRCmd->AvailableForStates(G4State_PreInit);


  _lcioWriteCompleteHepEvt = new G4UIcmdWithABool("/Mokka/init/lcioWriteCompleteHepEvt", this);
  _lcioWriteCompleteHepEvt->SetGuidance("If true, the output LCIO file will contain the complete list of particles present in the event file");
  _lcioWriteCompleteHepEvt->SetGuidance("with their original generator (HepEvt) status preserved.");
  _lcioWriteCompleteHepEvt->SetParameterName("LCIOWriteCompleteHepEvtMode", true);
  _lcioWriteCompleteHepEvt->AvailableForStates(G4State_PreInit);

  _lcioWriteParentsForBackscatter = new G4UIcmdWithABool("/Mokka/init/lcioWriteParentsForBackscatter", this);
  _lcioWriteParentsForBackscatter->SetGuidance("If true, the output LCIO MCParticle collection will have shower particles stored if they are parents of back scattered particles. ");
  _lcioWriteParentsForBackscatter->SetGuidance("This is the old (pre DBD) behaviour.");
  _lcioWriteParentsForBackscatter->SetParameterName("LCIOWriteParentsForBackscatterMode", true);
  _lcioWriteParentsForBackscatter->AvailableForStates(G4State_PreInit);

  _configAngleCmd = new G4UIcmdWithADoubleAndUnit("/Mokka/init/CONFIG_ANGLE", this);
  _configAngleCmd->SetGuidance("Specify the configuration angle for test beam setups.");
  _configAngleCmd->SetParameterName("ConfigAngle", false);
  _configAngleCmd->SetDefaultUnit("deg");
  _configAngleCmd->SetRange("ConfigAngle >= -180. && ConfigAngle <= 180.") ;
  _configAngleCmd->AvailableForStates(G4State_PreInit);

  _lorentzTransformationAngleCmd = new G4UIcmdWithADoubleAndUnit("/Mokka/init/lorentzTransformationAngle", this);
  _lorentzTransformationAngleCmd->SetGuidance("Specify the angle for the Lorentz transformation of primary particles.");
  _lorentzTransformationAngleCmd->SetGuidance("This angle corresponds to half the beam crossing angle.");
  _lorentzTransformationAngleCmd->SetParameterName("Angle", false);
  _lorentzTransformationAngleCmd->SetDefaultUnit("mrad");
  _lorentzTransformationAngleCmd->AvailableForStates(G4State_PreInit);

  _primaryVertexSpreadZCmd = new G4UIcmdWithADoubleAndUnit("/Mokka/init/primaryVertexSpreadZ", this);
  _primaryVertexSpreadZCmd->SetGuidance("Specify the spread of z for primary vertex.");
  _primaryVertexSpreadZCmd->SetGuidance("This value corresponds to the width of a Gaussian distribution along z.");
  _primaryVertexSpreadZCmd->SetParameterName("PrimaryVertexSpreadZ", false);
  _primaryVertexSpreadZCmd->SetDefaultUnit("mm");
  _primaryVertexSpreadZCmd->AvailableForStates(G4State_PreInit);

  _CGAGeometryGlobalSetupCmd = new G4UIcommand("/Mokka/init/globalModelParameter", this);
  _CGAGeometryGlobalSetupCmd->SetGuidance("Define a global parameter for the geometry model.");
  _CGAGeometryGlobalSetupCmd->SetParameter(new G4UIparameter("name", 's', false));
  _CGAGeometryGlobalSetupCmd->SetParameter(new G4UIparameter("value", 's', false));
  _CGAGeometryGlobalSetupCmd->AvailableForStates(G4State_PreInit);

  _startEventNumberCmd = new G4UIcmdWithAnInteger("/Mokka/init/startEventNumber", this);
  _startEventNumberCmd->SetGuidance("Begins the run from the given event number.");
  _startEventNumberCmd->SetParameterName("StartEventNumber", false);
  _startEventNumberCmd->AvailableForStates(G4State_PreInit);

  _printLevelCmd = new G4UIcmdWithAnInteger("/Mokka/init/printLevel", this);
  _printLevelCmd->SetGuidance("Sets the verbosity level of Mokka.");
  _printLevelCmd->SetParameterName("PrintLevel", false);
  _printLevelCmd->AvailableForStates(G4State_PreInit);

  _randomSeedCmd = new G4UIcmdWithAnInteger("/Mokka/init/randomSeed", this);
  _randomSeedCmd->SetGuidance("Sets the random seed of the CLHEP generator. Overidden by command line option -r <seed>");
  _randomSeedCmd->SetParameterName("RandomSeed", false);
  _randomSeedCmd->AvailableForStates(G4State_PreInit);

  _pairParticlesPerEventCmd = new G4UIcmdWithAnInteger("/Mokka/init/pairParticlesPerEvent", this);
  _pairParticlesPerEventCmd->SetGuidance("Number of pair particles to be simulayted in on (LCIO) event with GuineaPig pair background files.");
  _pairParticlesPerEventCmd->SetParameterName("PairParticlesPerEvent", false);
  _pairParticlesPerEventCmd->AvailableForStates(G4State_PreInit);

  _outputMomentumInTrackerHits = new G4UIcommand("/Mokka/init/lcioDetailedTRKHitMode", this);
  _outputMomentumInTrackerHits->SetGuidance("Enable momentum information in tracker hits for a given hits collection");
  _outputMomentumInTrackerHits->SetParameter(new G4UIparameter("collectionName",'s',false));
  _outputMomentumInTrackerHits->AvailableForStates(G4State_PreInit);

  _detailedHitCollection = new G4UIcommand("/Mokka/init/detailedHitsStoring", this);
  _detailedHitCollection->SetGuidance("Store all hits in the sensitive volume");
  _detailedHitCollection->SetParameter(new G4UIparameter("collectionName",'s',false));
  _detailedHitCollection->AvailableForStates(G4State_PreInit);

  _lcioStorePositionCmd = new G4UIcmdWithABool("/Mokka/init/lcioStoreCalHitPosition", this);
  _lcioStorePositionCmd->SetGuidance("Specifies whether to save CalHit position in LCIO mode. (default is true)");
  _lcioStorePositionCmd->SetParameterName("lcioStoreCalHitPosition", false);
  _lcioStorePositionCmd->AvailableForStates(G4State_PreInit);


  // commands to change the detector geometry

  MokkaDirectory = new G4UIdirectory("/Mokka/init/EditGeometry/");
  MokkaDirectory->SetGuidance("Special geometry editor to modify on the fly the model composition.");

  _addSubDetectorCmd = new G4UIcommand("/Mokka/init/EditGeometry/addSubDetector", this);
  _addSubDetectorCmd->SetGuidance("Add a Sub-Detector to the current Detector Model in a given build-order.");
  _addSubDetectorCmd->SetParameter(new G4UIparameter("SubDetectorAdded",'s',false));
  G4UIparameter* BuildOrderPar = new G4UIparameter("BuildOrder",'i',true);
  BuildOrderPar->SetDefaultValue(0);
  _addSubDetectorCmd->SetParameter(BuildOrderPar);

  _addSubDetectorCmd->AvailableForStates(G4State_PreInit);

  _rmSubDetectorCmd = new G4UIcommand("/Mokka/init/EditGeometry/rmSubDetector", this);  
  _rmSubDetectorCmd->SetGuidance("Remove a subdetector (or all) from the current Detector Model.");
  G4UIparameter* nameToRemove = new G4UIparameter("SubDetectorToRemoved",'s', false);
  nameToRemove->SetGuidance("The subdetector name or \"all\" to erase all ingredients.");
  _rmSubDetectorCmd->SetParameter(nameToRemove);
  //  _rmSubDetectorCmd->SetParameterName("SubDetectorToRemoved", false);
  _rmSubDetectorCmd->AvailableForStates(G4State_PreInit);

  // MokkaGear Commands 
  _mokkaGearFileNameCmd = new G4UIcmdWithAString("/Mokka/init/MokkaGearFileName", this);
  _mokkaGearFileNameCmd->SetGuidance("Set file name for Geometry xml output file.");
  _mokkaGearFileNameCmd->SetParameterName("MokkaGearFileName",false);
  _mokkaGearFileNameCmd->AvailableForStates(G4State_PreInit);

  _mokkaGearMergeSourceCmd = new G4UIcmdWithAString("/Mokka/init/MokkaGearMergeSource", this);
  _mokkaGearMergeSourceCmd->SetGuidance( "Set file name for xml file.");
  _mokkaGearMergeSourceCmd->SetGuidance( "It will dominantly merge with regular MokkaGear output." );
  _mokkaGearMergeSourceCmd->SetParameterName("MokkaGearMergeSource",false);
  // end MokkaGear Commands


  //Nem TB cmds
  _confDataTagCmd = new G4UIcmdWithAString("/Mokka/init/confDataTag", this);
  _confDataTagCmd->SetGuidance( "Configuration of corresponding data run.");
  _confDataTagCmd->SetGuidance( "Required by the Calice collaboration." );
  _confDataTagCmd->SetParameterName("confDataTag",false);
  _confDataTagCmd->AvailableForStates(G4State_PreInit);
  
  _dataRunNumberCmd = new G4UIcmdWithAnInteger("/Mokka/init/dataRunNumber", this);
  _dataRunNumberCmd->SetGuidance("Sets the Test Beam data run number.");
  _dataRunNumberCmd->SetGuidance( "Required by the Calice collaboration." );
  _dataRunNumberCmd->SetGuidance( "Overidden by command line option -N <runnumber>");
  _dataRunNumberCmd->SetParameterName("dataRunNumber", false);
  _dataRunNumberCmd->AvailableForStates(G4State_PreInit);
  
  _mcRunNumberCmd = new G4UIcmdWithAnInteger("/Mokka/init/mcRunNumber", this);
  _mcRunNumberCmd->SetGuidance("Sets the Monte Carlo run number.");
  _mcRunNumberCmd->SetGuidance( "Overidden by command line option -n <runnumber>");
  _mcRunNumberCmd->SetParameterName("mcRunNumber", false);
  _mcRunNumberCmd->AvailableForStates(G4State_PreInit);

  // for custom particle table
  _PDGFileCmd = new G4UIcmdWithAString("/Mokka/init/PDGFile",this);
  _PDGFileCmd->SetGuidance("Sets the particle data for HepPDT, probably called particle.tbl.");
  _PDGFileCmd->SetParameterName("PDGFile",false);
  _PDGFileCmd->AvailableForStates(G4State_PreInit);

}

ControlMessenger::~ControlMessenger(void)
{
  /* should delete all the created commands and directories */
  delete MokkaDirectory;
  delete drawEventCmd;
  delete loadFileCmd;
  delete VisuDirectory;
  delete _initDir;
  delete _dbHostCmd;
  delete _userCmd;
  delete _dbPasswdCmd;
  delete _detectorModelCmd;
  delete _modelsDBNameCmd;
  delete _materialsDBNameCmd;
  delete _detectorSetupCmd;
  delete _subDetectorCmd;
  delete _initialMacroFileCmd;
  delete _outDirNameCmd;
  delete _lcioFilenameCmd;
  delete _lcioEventParameterCmd;
  delete _lcioWriteModeCmd;
  delete _pythiaFilenameCmd; 
  delete _batchMode;// Control::BATCH_MODE = false;
  delete _dumpG3Cmd; // Control::DUMPG3 = false;
  delete _savingTrajectoriesCmd; // Control::SavingTrajectories = true;
  delete _savingPrimariesCmd; // Control::SavingPrimaries = true;
  delete _visumodeCmd; // Control::VISUMODE = false;
  delete _BFactorCmd; // Control::BFactor = 1;
  delete _rangeCutCmd; // Control::RangeCut = 0.005 * mm;
  delete _activeRangeCutCmd;//Control::ActiveRangeCut=0.005 * mm;
  delete _radiatorRangeCutCmd;//Control::RadiatorRangeCut=0.005 * mm;
  delete _pcbRangeCutCmd;//Control::PCBRangeCut=0.005 * mm;
  delete _TPCCutCmd; // Control::TPCCut = 10 * MeV;
  delete _TrackingPhysicsListMSOnCmd;
  delete _TrackingPhysicsListELossOnCmd;
  delete _TPCLowPtCutCmd; // Control::TPCLowPtCut = 10.0 * MeV;
  delete _TPCLowPtStepLimitCmd; // Control::TPCLowPtStepLimit = false; 
  delete _TPCLowPtStoreMCPForHitsCmd; // Control::TPCLowPtStoreMCPForHits = false; 
  delete _TPCLowPtMaxStepLengthCmd; // Control::TPCLowPtMaxStepLength = 1.0 * mm;
  delete _TPCLowPtMaxHitSeparationCmd; // Control::TPCLowPtMaxHitSeparation = 5.0 * mm;
  delete _activatePluginCmd; 
  delete _physicsListNameCmd; 
  delete _userInitStringCmd; 
  delete _userInitDoubleCmd; 
  delete _userInitIntCmd;
  delete _userInitBoolCmd;
  delete _lcioDetailedShowerModeCmd; // Control::LCIODetailedShowerMode
  delete _useOldHEPLCIOCmd; // Control::USE_OLD_HEPLCIO
  delete _fixStdHepLeptonFSRCmd ; // Control::FixStdHepLeptonFSR
  delete _lcioWriteCompleteHepEvt; // Control::LCIOWriteCompleteHepEvt
  delete _lcioWriteParentsForBackscatter; // Control::LCIOWriteParentsForBackscatter
  delete _configAngleCmd; // Control::ConfigAngle = 100000;
  delete _lorentzTransformationAngleCmd;
  delete _CGAGeometryGlobalSetupCmd; // setup global geometry parameters
  delete _startEventNumberCmd;
  delete _printLevelCmd;
  delete _outputMomentumInTrackerHits;
  delete _detailedHitCollection; //Lukasz Maczewski
  delete _lcioStorePositionCmd; // Control::LCIOStorePosition = true;
  delete _addSubDetectorCmd;
  delete _rmSubDetectorCmd;
  delete _mokkaGearFileNameCmd ;
  delete _mokkaGearMergeSourceCmd ;
  delete _randomSeedCmd;
  delete _pairParticlesPerEventCmd ;
  delete _dataRunNumberCmd;
  delete _mcRunNumberCmd;
  delete _confDataTagCmd;
  delete _userDeltaIntersectionCmd; 
  delete _userDeltaOneStepCmd; 
  delete _PDGFileCmd;
  delete _primaryVertexSpreadZCmd;
}

void ControlMessenger::SetNewValue(G4UIcommand *command, G4String newValues)
{
  if      (command == drawEventCmd)               theControl->SetDrawFlag(drawEventCmd->GetNewBoolValue(newValues));
  else if (command == loadFileCmd) {
    if (!Control::VISUMODE)
      G4cout << "LoadEvent is only available in visualisation mode!" << G4endl;
    else
      theControl->LoadEvent(loadFileCmd->GetNewIntValue(newValues));
  }

  // steering commands:
  G4cout << "Initialisation: " << command->GetCommandPath() << " " << newValues << G4endl;

  if      (command == _dbHostCmd)                 Control::DBHOST = newValues;
  else if (command == _userCmd)                   Control::USER = newValues;
  else if (command == _dbPasswdCmd)               Control::DBPASSWD = newValues;
  else if (command == _detectorModelCmd)          Control::DETECTOR_MODEL = newValues;
  else if (command == _modelsDBNameCmd)           Control::MODELS_DBNAME= newValues;
  else if (command == _materialsDBNameCmd)        Control::MATERIALS_DBNAME = newValues;
  else if (command == _detectorSetupCmd)          Control::DETECTOR_SETUP = newValues;
  else if (command == _subDetectorCmd)            Control::SUB_DETECTOR = newValues;
  else if (command == _initialMacroFileCmd)       Control::IntialMacroFile = newValues;
  else if (command == _outDirNameCmd) {
    Control::OutDirName = newValues;
    Control::PersistencyMode = mokka::ASCII_FILES;
  }
  else if (command == _lcioFilenameCmd) {
#ifdef LCIO_MODE
    Control::LCIOFILENAME = newValues;
    Control::PersistencyMode = mokka::LCIO_FILE;
#else
    G4cout << " LCIO not supported!" << G4endl
      << " Please set the LCIO environment variable to build Mokka with LCIO." << G4endl;
#endif
  } 

#ifdef LCIO_MODE
  else if( command == _lcioEventParameterCmd ){         
    
    G4Tokenizer nextToken(newValues);

    const G4String type = nextToken();
    const G4String name = nextToken();
    const G4String value= nextToken();

    if( name.size() > 0 && type.size() > 0 && value.size() > 0 && 
	( type == "int" || type == "float" || type == "string" )  ) {
      
      Control::lcioEvtParamMap.insert(  std::make_pair( type,  std::make_pair(name,value) )  ) ;
      
    } else {
      
      G4cout << " lcioEventParameterCmd - illegal format : '" << newValues << "' -> ignored ! " 
	     << std::endl ; 
    }
  }
#endif

  else if (command == _useOldHEPLCIOCmd)          Control::USE_OLD_HEPLCIO = newValues;
  else if (command == _lcioWriteModeCmd)          Control::LCIOWRITEMODE = newValues;
  else if (command == _pythiaFilenameCmd)         Control::PhytiaFileName = newValues;
  else if (command == _dumpG3Cmd)                 Control::DUMPG3 = _dumpG3Cmd->GetNewBoolValue(newValues);
  else if (command == _batchMode)                 Control::BATCH_MODE = _batchMode->GetNewBoolValue(newValues);
  else if (command == _savingTrajectoriesCmd)     Control::SavingTrajectories = _savingTrajectoriesCmd->GetNewBoolValue(newValues);
  else if (command == _savingPrimariesCmd)        Control::SavingPrimaries = _savingPrimariesCmd->GetNewBoolValue(newValues);
  else if (command == _visumodeCmd)               Control::VISUMODE = _visumodeCmd->GetNewBoolValue(newValues);
  else if (command == _BFactorCmd)                Control::BFactor = _BFactorCmd->GetNewDoubleValue(newValues);
  else if (command == _rangeCutCmd)               Control::RangeCut = _rangeCutCmd->GetNewDoubleValue(newValues);
  else if (command == _pcbRangeCutCmd)            Control::PCBRangeCut = _pcbRangeCutCmd->GetNewDoubleValue(newValues);
  else if (command == _radiatorRangeCutCmd)       Control::RadiatorRangeCut = _radiatorRangeCutCmd->GetNewDoubleValue(newValues);
  else if (command == _activeRangeCutCmd)         Control::ActiveRangeCut = _activeRangeCutCmd->GetNewDoubleValue(newValues);
  else if (command == _TPCCutCmd)                 Control::TPCCut = _TPCCutCmd->GetNewDoubleValue(newValues);

  //SJA: Add control over TrackingPhysicsList
  else if (command == _TrackingPhysicsListMSOnCmd)    Control::TrackingPhysicsListMSOn = _TrackingPhysicsListMSOnCmd->GetNewBoolValue(newValues) ;
  else if (command == _TrackingPhysicsListELossOnCmd) Control::TrackingPhysicsListELossOn = _TrackingPhysicsListELossOnCmd->GetNewBoolValue(newValues) ;

  else if (command == _TPCLowPtCutCmd)              Control::TPCLowPtCut = _TPCLowPtCutCmd->GetNewDoubleValue(newValues);
  else if (command == _TPCLowPtStepLimitCmd)        Control::TPCLowPtStepLimit = _TPCLowPtStepLimitCmd->GetNewBoolValue(newValues);
  else if (command == _TPCLowPtStoreMCPForHitsCmd)  Control::TPCLowPtStoreMCPForHits = _TPCLowPtStoreMCPForHitsCmd->GetNewBoolValue(newValues);
  else if (command == _TPCLowPtMaxStepLengthCmd)    Control::TPCLowPtMaxStepLength = _TPCLowPtMaxStepLengthCmd->GetNewDoubleValue(newValues);
  else if (command == _TPCLowPtMaxHitSeparationCmd) Control::TPCLowPtMaxHitSeparation = _TPCLowPtMaxHitSeparationCmd->GetNewDoubleValue(newValues);

  else if (command == _activatePluginCmd)         PluginManager::getInstance()->activatePlugin(newValues);
  else if (command == _physicsListNameCmd)        Control::PhysicsListName = newValues;
  else if (command == _userInitStringCmd) {
    G4Tokenizer nextToken(newValues);
    const G4String name = nextToken();
    const G4String value = nextToken();
    UserInit::getInstance()->setUserVariable(name, value);
  } 
  else if (command == _userInitDoubleCmd) {
    G4Tokenizer nextToken(newValues);
    const G4String name = nextToken();
    const G4String value = nextToken();
    const G4String unit = nextToken();
    if (unit.isNull())
      UserInit::getInstance()->setUserVariable(name, StoD(value));
    else
      UserInit::getInstance()->setUserVariable(name, StoD(value) * G4UIcommand::ValueOf(unit));
  } 
  else if (command == _userInitIntCmd) {
    G4Tokenizer nextToken(newValues);
    const G4String name = nextToken();
    const G4String value = nextToken();
    UserInit::getInstance()->setUserVariable(name, StoI(value));
  }
  else if (command == _userInitBoolCmd) {
    G4Tokenizer nextToken(newValues);
    const G4String name = nextToken();
    const G4String value = nextToken();
    UserInit::getInstance()->setUserVariable(name, StoB(value));
  }
  else if (command == _lcioDetailedShowerModeCmd) Control::LCIODetailedShowerMode = _lcioDetailedShowerModeCmd->GetNewBoolValue(newValues);

  else if (command == _fixStdHepLeptonFSRCmd) Control::FixStdHepLeptonFSR = _fixStdHepLeptonFSRCmd->GetNewBoolValue(newValues);

  else if (command == _lcioWriteCompleteHepEvt) Control::LCIOWriteCompleteHepEvt = _lcioWriteCompleteHepEvt->GetNewBoolValue(newValues);
  else if (command == _lcioWriteParentsForBackscatter) Control::LCIOWriteParentsForBackscatter = _lcioWriteParentsForBackscatter->GetNewBoolValue(newValues);
  else if(command == _userDeltaIntersectionCmd)
	Control::UserDeltaIntersection= _userDeltaIntersectionCmd->GetNewDoubleValue(newValues) / mm;
  else if(command == _userDeltaOneStepCmd)
	Control::UserDeltaOneStep= _userDeltaOneStepCmd->GetNewDoubleValue(newValues) / mm;
  else if (command == _configAngleCmd)            Control::ConfigAngle = _configAngleCmd->GetNewDoubleValue(newValues) / deg; // they store it in units of degrees (not beautiful)
  else if (command == _lorentzTransformationAngleCmd) Control::LorentzTransformationAngle = _lorentzTransformationAngleCmd->GetNewDoubleValue(newValues);
  else if (command == _primaryVertexSpreadZCmd) Control::PrimaryVertexSpreadZ = _primaryVertexSpreadZCmd->GetNewDoubleValue(newValues);
  else if (command == _CGAGeometryGlobalSetupCmd) {
 
    Control::EditGeometry = true;

    G4Tokenizer nextToken(newValues);
    const G4String name = nextToken();
    const G4String value = nextToken();
    G4cout << " Global model parameter \"" << name << "\" set to \"" << value << "\"" << G4endl;
    (*Control::globalModelParameters)[name] = value;
  }
  else if (command == _startEventNumberCmd) {
    Control::SYNCEVT = _startEventNumberCmd->GetNewIntValue(newValues);
    Control::SKIP = true;
  }
  else if (command == _printLevelCmd) {
    Control::PrintLevel = _printLevelCmd->GetNewIntValue(newValues);
  }
  else if (command == _dataRunNumberCmd && Control::DataRunNumberSetViaCommandLine == false) {
    Control::DataRunNumber = _dataRunNumberCmd->GetNewIntValue(newValues);
  }
  else if (command == _mcRunNumberCmd && Control::mcRunNumberSetViaCommandLine == false) {
    Control::mcRunNumber = _mcRunNumberCmd->GetNewIntValue(newValues);
  }
  else if (command == _confDataTagCmd) {
    Control::ConfDataTag = newValues;
  }
  else if (command == _randomSeedCmd && Control::RandomSeedSetViaCommandLine == false) {
    Control::RandomSeed = _randomSeedCmd->GetNewIntValue(newValues);
    G4RandGauss::getTheEngine()->setSeed(Control::RandomSeed,0);
  }
  else if (command == _pairParticlesPerEventCmd) {
    Control::PairParticlesPerEvent = _pairParticlesPerEventCmd->GetNewIntValue(newValues);
  }
  else if (command == _lcioStorePositionCmd) {
    Control::LCIOStorePosition = _lcioStorePositionCmd->GetNewBoolValue(newValues);
  }
  else if (command == _addSubDetectorCmd) {

    Control::EditGeometry = true;

    G4Tokenizer nextToken(newValues);
    const G4String name = nextToken();
    const G4String order = nextToken();
    Control::GeometryEditions.
      push_back(new GeometryEdition(ADD,
				    name,
				    StoI(order)
				    )
		);
  }
  else if (command == _rmSubDetectorCmd) {

    Control::EditGeometry = true;

    Control::GeometryEditions.
      push_back(new GeometryEdition(REMOVE,
				    newValues,
				    0)
		);
  }
  else if (command == _outputMomentumInTrackerHits) {
    Control::lcioStoreTRKHitMomentum.push_back(newValues);
  }
  else if (command == _detailedHitCollection) {
    Control::detailedHitsStoring.push_back(newValues);
  }
  else if (command == _mokkaGearFileNameCmd ) {
  
#ifdef MOKKA_GEAR
    Control::mokkaGearFileName = newValues ;
#else
    G4cout << " MokkaGear not supported!" << G4endl
      << " Please set the GEAR environment variable to build Mokka with GEAR." << G4endl ;
#endif
  }
  else if (command == _mokkaGearMergeSourceCmd ) {
#ifdef MOKKA_GEAR
    Control::mokkaGearMergeSource = newValues ;
#else
    G4cout << " MokkaGear not supported!" << G4endl
      << " Please set the GEAR environment variable to build Mokka with GEAR." << G4endl ;
#endif
  }
  else if (command == _PDGFileCmd) {
    Control::PDGFile = newValues;
  }
}
