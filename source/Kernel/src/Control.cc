// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: Control.cc,v 1.67 2009/05/13 08:22:57 steve Exp $
// $Name: mokka-07-00 $

#include "Control.hh"

#include "ControlMessenger.hh"
#include "Visu.hh"
#include "VSubDetectorDriver.hh"
#include "CGAGeometryManager.hh"
#include "G4Event.hh"
#include "Trajectory.hh"
#include "UserInit.hh"
#include "G4EventManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4UImanager.hh"
#include "G4VVisManager.hh"

#include "G4Step.hh"

#include <sstream>
// using std::istringstream;
//FG  switched back to use  strstream (needed for gcc2.95)
// #include <strstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#ifdef LCIO_MODE
#include "lcio.h"
#include "IO/LCWriter.h"
#include "IMPL/LCEventImpl.h" 
#include "IMPL/LCCollectionVec.h"
#include "IMPL/MCParticleImpl.h" 
#include "HepLCIOInterfaceNew.hh"
#include "UTIL/Operators.h"
using namespace lcio ;
#endif

#include "G4Track.hh"
#include "UserTrackInformation.hh"

using namespace mokka ;
 
G4bool   Control::BATCH_MODE=false;
G4String Control::DBHOST="pollin1.in2p3.fr";
G4String Control::USER="consult";
G4String Control::DBPASSWD="consult";

G4String Control::DETECTOR_MODEL="";
G4String Control::DETECTOR_CONCEPT="";
G4String Control::DETECTOR_SETUP = "";

G4String Control::SUB_DETECTOR="";
G4String Control::MODELS_DBNAME="models03";

G4String Control::MATERIALS_DBNAME="materials02";
G4String Control::LOGFILEPATH="";
G4String Control::LASTEVTFILEPATH="";

G4String Control::IntialMacroFile="";
G4bool   Control::DUMPG3 = false;

PERSISTENCY_MODE Control::PersistencyMode = mokka::NONE;

G4String Control::OutDirName;

G4String Control::LCIOWRITEMODE="";

#ifdef LCIO_MODE
G4String Control::LCIOFILENAME="";
LCWriter* Control::lcWrt=NULL;
LCEventImpl* Control::lcEvt=NULL;
LCRunHeaderImpl* Control::lcRunHdr=NULL ;
LCCollectionVec* Control::lcMCVec=NULL;
MCParticleMap_type Control::MCParticleMap;
LCIOEventParameterMap Control::lcioEvtParamMap ;
#endif

G4bool Control::USE_OLD_HEPLCIO = false ; 

// allways define this boolean as it is used also
// for ASCII output format (PMdeF).
G4bool  Control::LCIOWriteCompleteHepEvt = true;
G4bool  Control::LCIOWriteParentsForBackscatter = true ;
G4bool   Control::SavingTrajectories=true;
G4bool   Control::SavingPrimaries=true;
G4bool   Control::drawFlag = false;

G4bool   Control::RESTART = false;
G4bool   Control::SKIP = false;
G4bool   Control::VISUMODE  = false;
G4int    Control::SYNCEVT = 0;
G4String Control::PhytiaFileName; 

G4String Control::PhysicsListName = "QGSP_BERT" ; 

G4double Control::RadiatorRangeCut = 0.005 * mm;
G4double Control::PCBRangeCut = 0.005 * mm;
G4double Control::ActiveRangeCut = 0.005 * mm;
G4double Control::RangeCut = 0.005 * mm;
G4double Control::TPCCut = 10 * MeV;

//SJA: Added limitation of the step length in the TPC for very low pt charged particles
G4double Control::TPCLowPtCut = 10.0 * MeV;
G4bool   Control::TPCLowPtStepLimit = false;
G4bool   Control::TPCLowPtStoreMCPForHits = false;
G4double Control::TPCLowPtMaxStepLength = 1.0 * mm;
G4double Control::TPCLowPtMaxHitSeparation = 5.0 * mm;

//SJA: Add control over TrackingPhysicsList
G4bool   Control::TrackingPhysicsListMSOn    = true;
G4bool   Control::TrackingPhysicsListELossOn = true;

G4double Control::BFactor = 1.;

G4int    Control::stepNumber = 0;
G4int    Control::primaryId = -1;
G4bool   Control::TrackingPrimary = false;

G4ThreeVector Control::VertexPosition;
G4double Control::VertexEnergy;
G4ThreeVector Control::VertexMomentum;

Visu*     Control::VisuManager = NULL;
Control* Control::theControl = NULL;

G4bool  Control::LCIODetailedShowerMode = false ;

G4bool  Control::FixStdHepLeptonFSR = true ;

G4double Control::ConfigAngle = 0.0;

G4double Control::LorentzTransformationAngle = 0;
G4double Control::PrimaryVertexSpreadZ = 0;

CGASetupParameters* Control::globalModelParameters = NULL;

std::vector<VSubDetectorDriver*> Control::DETECTOR_DRIVERS;

G4int Control::PrintLevel = 2;

G4int Control::RandomSeed = 1234567890;
G4bool Control::RandomSeedSetViaCommandLine = false;

G4int Control::PairParticlesPerEvent = 1 ;

G4bool Control::LCIOStorePosition = true;

std::vector<GeometryEdition*> Control::GeometryEditions;

std::vector<G4String> Control::lcioStoreTRKHitMomentum;

std::vector<G4String> Control::detailedHitsStoring;

G4int Control::DataRunNumber = 0;
G4bool Control::DataRunNumberSetViaCommandLine = false;

G4int Control::mcRunNumber = 0;
G4bool Control::mcRunNumberSetViaCommandLine = false;

G4String Control::ConfDataTag = "";

G4double Control::UserDeltaIntersection=-1.;
G4double Control::UserDeltaOneStep=-1.;
G4double Control::G4DefaultDeltaIntersection=-1.;
G4double Control::G4DefaultDeltaOneStep=-1.;

G4bool   Control::ModelOpened = false;
G4bool   Control::EditGeometry = false;

G4String Control::MokkaVersionName = "";

#ifdef MOKKA_GEAR
G4String Control::mokkaGearFileName = "GearOutput.xml" ;
G4String Control::mokkaGearMergeSource = ""  ;
#endif

G4bool Control::RunAborted = false;
G4String Control::InputFileGeneratorWarning = "";

G4String Control::PDGFile = "particle.tbl";

Control* Control::GetControl()
{
  if (theControl == 0)
    theControl = new Control();
  return theControl;
}

//FG: now static  Control::Control() : MCCutInRange(5 * cm), MCCutInEnergy(1000 * MeV)
Control::Control()
{
  ResetPIDForCalHit(-1);
  trackedtracks = new TrackSummaryVector();
  theControlMessenger = new ControlMessenger(this);
  VisuManager = new (Visu);
  globalModelParameters = new CGASetupParameters();
}

Control::~Control()
{
delete theControlMessenger;
}

//-----------------------------------------------------------------------------
void Control::Usage(const char *name, const char *str)
{
  if (strcmp(str, "")) // there is an error message
    G4cout << name << ": " << str << "\n" << G4endl;

  G4cout
    << "Usage:          " << name << " [options] steeringfile\n"
    << "\n"
    << "-H              print this help message.\n"
    << "-m <filename>   specifies a macro file to be executed before running.\n"
    << "\n"
    << "-l <filename>   specifies the filename for LCIO output.\n"
    << "-o <dirname>    specifies the directory for ASCII output.\n"
    << "\n"
    << "If neither -l nor -o are given, Mokka runs in transient mode.\n"
    << "\n"
    << "-v              specifies that the -o option above points to an old output\n"
    << "                directory with data just to be reloaded for visualisation.\n"
    << "                (no physics in this case)\n"
    << "-P              specifies NOT to save event primaries.\n"
    << "                (event primaries are saved by default in persistent mode)\n"
    << "-T              specifies NOT to save primary trajectories.\n"
    << "                (primary trajectories are saved by default in persistent mode)\n"
    << "-c <double>     specifies the Geant 4 production range cut in mm.\n"
    << "                (default is " << Control::RangeCut / mm << " mm)\n"
    << "-C <double>     specifies the range cut in the sensitive materials in mm.\n"
    << "                (default is " << Control::ActiveRangeCut / mm << " mm)\n"
    << "-B <double>     specifies a magnetic field factor.\n"
    << "                (default is " << Control::BFactor << ")\n"
    << "-t <double>     specifies the TPC primary energy cut in MeV.\n"
    << "                (enables the user to control the TPC output file length)\n"
    << "                (default is " << Control::TPCCut / MeV << " MeV)\n"
    << "-b              specifies BRAHMS backward facility. (generates GEANT3 code)\n"
    << "-M <model>      specifies the detector model to be simulated.\n"
    << "                (default is \"" << Control::DETECTOR_MODEL << "\")\n"
    << "-s <setup>      specifies the detector setup to be simulated.\n"
    << "                (default is \"" << Control::DETECTOR_SETUP << "\")\n"
    << "-S <subdet>     specifies to build only one subdetector.\n"
    << "                (overrides the -M option)\n"
    << "-h <hostname>   specifies the hostname of the MySQL server.\n"
    << "                (default is \"" << Control::DBHOST << "\")\n"
    << "-u <user>       specifies the MySQL user.\n"
    << "                (default is \"" << Control::USER << "\")\n"
    << "-p <password>   specifies the MySQL password.\n"
    << "-p <password>   specifies the MySQL password.\n"
    << "-e <pdgfile>    specifies pdg file with extra particles (e.g. particles.tbl)\n"
    << "-r <seed>       specifies the random seed. Overides any value specified in the steering file.\n"
    << "-n <runNumber>  specifies the mcRunNumber. Overides any value specified in the steering file.\n"
    << "-N <runNumber>  specifies the DataRunNumber. Overides any value specified in the steering file.\n"
#ifdef MOKKA_GEAR
    << "-g <filename>   specifies the filename for Gear geometry output.\n"
    << "                (default is \"" << Control::mokkaGearFileName << "\")\n"
    << "-G <filename>   specifies filename for XML that merges into regular output.\n"
    << "                (merges dominantly into output)\n"
#endif 
    << G4endl;
  exit(EXIT_FAILURE);
}

//-----------------------------------------------------------------------------

void Control::setup(int argc, char *argv[])
{
  // decodes the command line and prepares the run environment

  G4String RunControlFileName = "Run.control";
  time_t now = time(NULL);
  G4cout << "\n**** Mokka started at " << ctime(&now) << G4endl;

  std::string steeringFileName = std::string();

  char errbuff[80];
  char *Me = strrchr(argv[0], '/');
  if (Me == NULL) Me = argv[0];
  else Me++;

  extern char *optarg;       // provided by <unistd.h>
  extern int optind, optopt; // provided by <unistd.h>

  while (true) { // exit with "break" if no more options are found
    const int option = getopt(argc, argv, ":Hm:o:vPTc:C:B:t:bM:s:S:h:u:Up:l:g:G:e:r:n:N:");
    // Options requiring an argument are followed by a colon.
    // See the manpage of getopt(3) for details.
    if (option == -1) break; // no more options found: exit the loop

    switch (option) {
      case 'H': Usage("tata", ""); break;
      case 'm': Control::IntialMacroFile = optarg; break;
      case 'o':
        Control::OutDirName = optarg;
        Control::PersistencyMode = ASCII_FILES;
        break;
      case 'v': Control::VISUMODE = true; break;
      case 'P': Control::SavingPrimaries = false; break;
      case 'T': Control::SavingTrajectories = false; break;
      case 'c': Control::RangeCut = std::strtod(optarg, 0); break;
      case 'C': Control::ActiveRangeCut = std::strtod(optarg, 0); break;
      case 'B': 
        Control::BFactor = std::strtod(optarg, 0);
        if (Control::BFactor < 0) Usage(Me, "Argument of option -B must not be negative.");
        break;
      case 't': Control::TPCCut = std::strtod(optarg, 0); break;
      case 'b': Control::DUMPG3 = true; break;
      case 'M': Control::DETECTOR_MODEL = optarg; break;
      case 's': Control::DETECTOR_SETUP = optarg; break;
      case 'S': Control::SUB_DETECTOR = optarg; break;
      case 'h': Control::DBHOST = optarg; break;
      case 'u': Control::USER = optarg; break;
      case 'U': Control::ModelOpened = true; break;
      case 'p': Control::DBPASSWD = optarg; break;
      case 'e': Control::PDGFile = optarg; break;
      case 'r': Control::RandomSeed = std::atoi(optarg); Control::RandomSeedSetViaCommandLine = true ; break;
      case 'n': Control::mcRunNumber = std::atoi(optarg); Control::mcRunNumberSetViaCommandLine = true ; break;
      case 'N': Control::DataRunNumber = std::atoi(optarg); Control::DataRunNumberSetViaCommandLine = true ; break;
      case 'l':
#ifdef LCIO_MODE
        Control::LCIOFILENAME = optarg;
        Control::PersistencyMode = LCIO_FILE;
#else
        Usage(Me, "Option -l is not available because Mokka was compiled without LCIO support.");
#endif
        break;
      case 'g':
#ifdef MOKKA_GEAR
        Control::mokkaGearFileName = optarg;
#else
        Usage(Me, "Option -g is not available because Mokka was compiled without Gear support.");
#endif
        break;
      case 'G':
#ifdef MOKKA_GEAR
        Control::mokkaGearMergeSource = optarg;
#else
        Usage(Me, "Option -G is not available because Mokka was compiled without Gear support.");
#endif
        break;
          
      case ':': // getopt(3) returns a colon if the argument of an option is missing
        G4cout << "Option -" << char(optopt) << " requires an argument.\n" << G4endl;
        Usage(Me, "");
        break;
      case '?': // getopt(3) returns a question mark if an option is unknown
        G4cout << "Unrecognized option: -" << char(optopt) << "\n" << G4endl;
        Usage(Me, "");
        break;
    } // switch (option)
  } // while (true)

  if (optind < argc - 1) { // there are several command line parameters left
    Usage(Me, "Cannot use more than one steering file at a time.");

  } else if (optind == argc - 1) { // steering file is given on the command line
    steeringFileName = argv[optind];
    G4UImanager::GetUIpointer()->ApplyCommand(G4String("/control/execute " + steeringFileName));
    if (ConfigAngle == 100000) ConfigAngle = 0;
  }

  // check parameters
  if (OutDirName.empty() && VISUMODE) {
    sprintf(errbuff, "The -v option asks for the -o <DirectoryName> parameter!\n");
    Usage(Me, errbuff);
  } 

  // deal with the native persistency scheme (ASCII files)
  G4String RunControlPath = Control::OutDirName+"/";
  RunControlPath+=RunControlFileName;
  FILE *RunControlFile=0;

  // If given a output directory name, lets see if it exists
  if(OutDirName.length()!=0 && !VISUMODE) 
    {
      G4String header= "MOKKA started";
      FILE *LastEvtFile;
      LASTEVTFILEPATH=Control::OutDirName+"/.LASTEVT";
      
      G4cout<< "Creating the output directory " << OutDirName << G4endl;
      if(mkdir(OutDirName,
	       S_IRUSR|S_IWUSR|S_IXUSR|
	       S_IRGRP|S_IXGRP|
	       S_IROTH|S_IXOTH) == -1)
	{
	  // It already exists, it means we are in restart mode or reload
	  G4cout<< "Output directory already exists, assuming RESTART mode" 
		<< G4endl;

	  // flag it!
	  Control::RESTART = true;

	  // Recover the last sucefully simulated event in the directory
	  if((LastEvtFile=fopen(LASTEVTFILEPATH,"r"))==NULL)
	    Abort("RESTART mode but .LASTEVT file not found!",
			MOKKA_ERROR_RESTART_MODE_NOT_POSSIBLE);
	  fscanf(LastEvtFile,"%d",&SYNCEVT);
	  G4cout << "SYNCEVT = " << SYNCEVT << G4endl;
	}
      else
	{
	  // the directory doesn't exist. So it's a true new Run
	  // Creates the LastEvtFile file to keep the last sucefully 
	  // simulated event in the directory, inits it to zero
	  if((LastEvtFile=fopen(LASTEVTFILEPATH,"w"))==NULL) {
	    perror(LASTEVTFILEPATH);
	    exit(1);
	  }
	  fprintf(LastEvtFile,"%d\n",0);

	  // Creates the Run.control describing the main run
	  // options (geometrie, saving policies, etc)
	  if((RunControlFile =fopen(RunControlPath,"w"))==NULL) {
	    perror(RunControlPath);
	    exit(1);
	  }
	  G4String TmpSUB_DETECTOR=SUB_DETECTOR;
	  if(TmpSUB_DETECTOR =="") TmpSUB_DETECTOR="VOID";
	  G4String TmpDETECTOR_SETUP=DETECTOR_SETUP;
	  if(TmpDETECTOR_SETUP =="") TmpDETECTOR_SETUP="VOID";

	  fprintf(RunControlFile,"%s\n%s\n%s\n%s\n%15e\n%d\n%d\n%15e\n%f\n",
		  DETECTOR_MODEL.data(),
		  TmpDETECTOR_SETUP.data(),
		  TmpSUB_DETECTOR.data(),
		  DBHOST.data(),
		  RangeCut,
		  SavingTrajectories,
		  SavingPrimaries,
		  BFactor,
		  ConfigAngle);
	  fclose(RunControlFile);
	}
      fclose(LastEvtFile);
      
      // Log it
      LOGFILEPATH=Control::OutDirName+"/Mokka.log";
      if(Control::RESTART) 
	{
	  header = "MOKKA re-started";
	}
      header += ", command line is:\n";
      char **v=argv;
      while((*v)!=NULL) 
	{
	  header+=(*v);
	  header+=+" ";
	  v++;
	}
      Log(header.data());
    }

  // If VISUMODE or RESTART mode, a Run.control file has to exist in the
  // data directory. Reload it to recover the true run parameters.
  // (all the other options in the command line are overhead here)
  if(VISUMODE || RESTART)
    {
      G4cout << "Reloading " << RunControlPath << " Mokka control file\n";
      if((RunControlFile=fopen(RunControlPath,"r"))==NULL)
	Abort("RESTART mode but Run.control file not found!",
			MOKKA_ERROR_RESTART_MODE_NOT_POSSIBLE);
      char buff[80];
      G4bool coherence = true;
      fscanf(RunControlFile,"%s",&buff[0]);
      if(DETECTOR_MODEL!="D09" && 
	 DETECTOR_MODEL!=buff)
	{
	  G4cout << "The line option -M says " 
		 <<  DETECTOR_MODEL
		 << " but it's not coherent with the Run.control file contents!"
		 << G4endl;
	  coherence = false;
	}
      DETECTOR_MODEL=buff;
      // reading setup name
      fscanf(RunControlFile,"%s",&buff[0]);
      if(DETECTOR_SETUP!="" && 
	 DETECTOR_SETUP!=buff)
	{
	  G4cout << "The line option -s says " 
		 <<  DETECTOR_SETUP
		 << " but it's not coherent with the Run.control file contents!"
		 << G4endl;
	  coherence = false;
	}
      DETECTOR_SETUP=buff;
      if(DETECTOR_SETUP=="VOID")DETECTOR_SETUP="";
      fscanf(RunControlFile,"%s",&buff[0]);
      if(SUB_DETECTOR!="" && 
	 SUB_DETECTOR!=buff)
	{
	  G4cout << "The line option -S says " 
		 << SUB_DETECTOR
		 << " but it's not coherent with the Run.control file contents!"
		 << G4endl;
	  coherence = false;
	}
      SUB_DETECTOR=buff;
      if(SUB_DETECTOR=="VOID")SUB_DETECTOR="";
      fscanf(RunControlFile,"%s",&buff[0]);
      DBHOST=buff;
      fscanf(RunControlFile,"%lf",&RangeCut);
      G4int tmpVal;
      fscanf(RunControlFile,"%d",&tmpVal);
      SavingTrajectories= tmpVal==1 ;
      fscanf(RunControlFile,"%d",&tmpVal);
      SavingPrimaries= tmpVal==1 ;
      fscanf(RunControlFile,"%lf",&BFactor);
      G4cout << "*****************************************************"
	     << "\n* Run.control file contents:"
	     << "\n* DETECTOR_MODEL= " << DETECTOR_MODEL 
	     << "\n* DETECTOR_SETUP= " << DETECTOR_SETUP 
	     << "\n* SUB_DETECTOR= " << SUB_DETECTOR 
	     << "\n* DBHOST= " << DBHOST
	     << "\n* RangeCut = " << RangeCut
	     << "\n* SavingTrajectories= " << SavingTrajectories
	     << "\n* SavingPrimaries= " << SavingPrimaries 
	     << "\n* Magnetic field factor= " << BFactor
	     << "\n*****************************************************\n"
	     << G4endl;
      fclose(RunControlFile);
      if(!coherence)
	Abort("Fatal error: line command and Run.control file are not coherent.",MOKKA_ERROR_RESTART_MODE_NOT_POSSIBLE);
    }
#ifdef LCIO_MODE
  if(PersistencyMode == LCIO_FILE)
    {
      if(VISUMODE || RESTART) 
	Abort("Fatal error: LCIO output mode not yet available in visualisation or restart mode.",MOKKA_OTHER_ERRORS);
    }
  if(LCIOFILENAME != "")
    {
      lcWrt = LCFactory::getInstance()->createLCWriter();

      try
	{
	  const std::string FILEN = LCIOFILENAME;
	  
	  // 	  lcWrt->open( LCIOFILENAME );
	  
	  if( Control::LCIOWRITEMODE == "WRITE_APPEND" ) {
	    
	    lcWrt->open( LCIOFILENAME , LCIO::WRITE_APPEND ) ;
	  }
	  else if( Control::LCIOWRITEMODE == "WRITE_NEW" ) {
	    
	    lcWrt->open( LCIOFILENAME , LCIO::WRITE_NEW ) ;
	  }
	  else {
	    lcWrt->open( LCIOFILENAME ) ;
	  }
	  
	  if( steeringFileName.size() > 0 ) {

	    // copy the full steering file (if any) to the LCIO run header for documentation
	    lcRunHdr = new LCRunHeaderImpl ;
	    
	    std::ifstream inFile( steeringFileName.c_str()  ) ;
	    
	    std::stringstream steeringFile  ;

	    steeringFile  << std::endl 
			  << "# ****** begin *** begin * Mokka steering file ** begin **** begin ****"  
			  << std::endl ;

	    std::string dummy ;

	    while(  ! inFile.eof()  ) {

	      getline( inFile, dummy ) ;

	      steeringFile << dummy << std::endl ;
	    }    


	    steeringFile << "# ********* end *** end *** Mokka steering file **** end ***** end *****"  
			 << std::endl ;
	    
	    lcRunHdr->parameters().setValue( "MOKKA_SteeringFile",  steeringFile.str()  ) ;
	  }
          if (!Control::IntialMacroFile.empty())
	  {

	  if(lcRunHdr == NULL)
		lcRunHdr = new LCRunHeaderImpl ;

	  std::ifstream macroFile( Control::IntialMacroFile.c_str()  ) ;
	  std::stringstream macroFileContent  ;

	  macroFileContent  << std::endl
                          << "# ****** begin *** begin * Mokka macro file ** begin **** begin ****"
                          << std::endl ;

	  std::string dummy ;

	  while(  ! macroFile.eof()  ) {

              getline( macroFile, dummy ) ;

              macroFileContent  << dummy << std::endl ;
	  }


	  macroFileContent  << "# ********* end *** end *** Mokka macro file **** end ***** end *****"
                         << std::endl ;

	  lcRunHdr->parameters().setValue( "MOKKA_MacroFile",  macroFileContent.str()  ) ;

	  }

	}
      catch(IOException& e)
	{
	  G4cout << "Couldn't open file " 
		 << LCIOFILENAME 
		 << "\n" 
		 << e.what() 
		 << G4endl ;
	  Abort("Fatal error: LCIO couldn't open file",
		MOKKA_ERROR_CANNOT_OPEN_LCIO_FILE);
	}
    }
#endif
}

void Control::Log(const char * message)
{
  time_t now = time(NULL);
  G4cout << message << G4endl;
  if(LOGFILEPATH=="" || VISUMODE)return;
  FILE *Log;
  if((Log=fopen(LOGFILEPATH.data(),"a"))==NULL) 
    {
      perror(LOGFILEPATH.data());
      exit(1);
    }
  fprintf(Log,"%s%s\n",ctime(&now),message) ;
  fclose(Log);
}

void Control::Abort(const char * message, int errorCode)
{
  Log("==========> MOKKA ABORTING <============");
  Log(message);
  // last try to release tmp databases, if any... 
  CGAGeometryManager::GetCGAGeometryManager()->TmpDBCleanup();
  exit(errorCode);
}

void Control::BeginOfEventAction(const G4Event* anEvent)
{

  // Clear the Event secondaries list
  size_t itr;
  for(itr=0;itr<trackedtracks->size();itr++)
    { 
      delete (*trackedtracks)[itr];
    }
  trackedtracks->clear();  



#ifdef LCIO_MODE

  if(lcWrt) 
    {
      lcEvt = new LCEventImpl();
      lcEvt->setRunNumber( (int) Control::mcRunNumber  ); 
      lcEvt->setEventNumber( anEvent->GetEventID()+SYNCEVT );
      lcEvt->setDetectorName( DETECTOR_MODEL );
    }
#endif
  // propagates the begin-of-event for sub-detector drivers to 
  // initialise.
  for (unsigned int i_coll=0;i_coll<DETECTOR_DRIVERS.size();i_coll++)
    DETECTOR_DRIVERS[i_coll]->BeginOfEventAction(anEvent);
}

void Control::EndOfEventAction(const G4Event* evt)
{
  // primaryId reset 
  ResetPIDForCalHit(-1);

  if(RunAborted)
	return;

  // Save primaries and Trajectories if needed
  SaveTracks(evt);
  
  // Setup the visualization current event
  // before it'll be too late
  VisuManager->SetCurrentEvt(evt);
  
  // Message of end of event
  if(Control::PrintLevel > 0) {
	  G4cout << ">>> Event " << evt->GetEventID()+SYNCEVT
		 << ", scanning sub-detectors \n\n";
  }
  // propagates the end-of-event for sub-detector drivers to sumarize 
  // and save data.
  for (unsigned int i_coll=0;i_coll<DETECTOR_DRIVERS.size();i_coll++)
	DETECTOR_DRIVERS[i_coll]->EndOfEventAction(evt);
  
  if(Control::PrintLevel > 1) {
	  G4cout << G4endl;
  }

  if( InputFileGeneratorWarning.size() != 0 && Control::PrintLevel > 0 ) 
	G4cout << "Event " << evt->GetEventID()+SYNCEVT << ", " 
		<< InputFileGeneratorWarning << G4endl;

  InputFileGeneratorWarning.clear();

#ifdef LCIO_MODE
  // Deal with LCIO persistency mode
  if(lcWrt) 
    {

      //fg: copy the event weight and process id from the mcp collection - if set:
      LCCollection* mcCol = lcEvt->getCollection("MCParticle" ) ;
      float evtWgt = mcCol->getParameters().getFloatVal("_weight") ;

      if( evtWgt != 0.0 )
	lcEvt->setWeight( evtWgt ) ;

      int idrup =  mcCol->getParameters().getIntVal("_idrup") ;

      if( idrup != 0 )
	lcEvt->parameters().setValue( "_idrup" , idrup ) ;


      //fg: add event parameters if set through /Mokka/init/lcioEventParameter
      typedef  LCIOEventParameterMap::iterator MIT ;
      typedef std::pair< MIT, MIT > MITP ; 
      MITP itp = lcioEvtParamMap.equal_range("int") ;
      for( MIT it = itp.first ; it != itp.second ; ++it ){
	lcEvt->parameters().setValue(  it->second.first ,  
				       atoi( it->second.second.c_str() ) )  ;
      }
      itp = lcioEvtParamMap.equal_range("float") ;
      for( MIT it = itp.first ; it != itp.second ; ++it ){
	lcEvt->parameters().setValue(  it->second.first ,  
				       (float) atof( it->second.second.c_str() ) )  ;
      }
      itp = lcioEvtParamMap.equal_range("string") ;
      for( MIT it = itp.first ; it != itp.second ; ++it ){
	lcEvt->parameters().setValue(  it->second.first ,  it->second.second  )  ;
      }
      
      try
	{
	  lcWrt->writeEvent( lcEvt ) ;
	} 
      catch(IOException& e)
	{
	  G4cout << "IO Error while write LCIO event:\n" 
		 << e.what() 
		 << G4endl ;
	  Abort("Fatal error: IO Error while write LCIO event.",
			MOKKA_ERROR_CANNOT_WRITE_TO_LCIO_FILE);
	}
      delete lcEvt;
    }
#endif
  
  // if persistent mode, keeps the last evt number
  if(OutDirName.length()!=0 && !VISUMODE)
    {
      FILE *LastEvtFile;
      if((LastEvtFile=fopen(LASTEVTFILEPATH,"w"))==NULL) 
	{
	  perror(LASTEVTFILEPATH);
	  exit(1);
	}
      fprintf(LastEvtFile,"%d\n",evt->GetEventID()+SYNCEVT+1);
      fclose(LastEvtFile); 
    }

  // Draw Hits if draw flag is true
  if(drawFlag) 
    {
      G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
      if(pVVisManager)
	{
	  G4UImanager::GetUIpointer()->ApplyCommand("/vis~/draw/current");
	}
      VisuManager->Refresh();  
    }
}

void 
Control::SaveTracks(const G4Event* anEvent)
{
  if(PersistencyMode == mokka::NONE) return;
  
#ifdef LCIO_MODE

  //  static bool writeCompleteHepEvt =  UserInit::getInstance()->getBool("WriteCompleteHepEvt") ;

  if( Control::LCIOWriteCompleteHepEvt ) {

    //fg: loop over MCParticle/G4DynamicParticle map and update the TrackSummary objects with 
    // existing McParticles
    typedef HepLCIOInterfaceNew::LCIO2Geant4Map::iterator IT ;
    
    //   std::cout << "Control::SaveTracks -------  HepLCIOInterface::Map --------- : " << std::endl ;
    
    for(IT it = HepLCIOInterfaceNew::Map.begin() ; it != HepLCIOInterfaceNew::Map.end() ; ++it ){
      
      //     std::cout << " pdg: "      <<  it->second->GetPDGcode() 
      // 	      << " trackid:  " <<  it->second->GetTrackID()  
      // 	      << std::endl ;
      
      if( it->second->GetTrackID() < 0 ) continue ;
      
      for(int itr = trackedtracks->size()-1; itr>=0; itr-- ){
	
	if( trackedtracks->operator[](itr)->GetTrackID() == it->second->GetTrackID()  ) {
	  trackedtracks->operator[](itr)->SetMCParticle( dynamic_cast<MCParticleImpl*>( it->first ) ) ; 
	  break ;
	}
      }
      
    }
  }
#endif
  

  // force to save all paths only for interesting secondaries
  G4int itr;
  for(itr=trackedtracks->size()-1;itr>=0;itr--)
    if(((*trackedtracks)[itr])->GetToBeSaved())
      ((*trackedtracks)[itr])->SetParentToBeSaved();
  
  // Mokka ASCII native mode
  // Save the primaries only if the user didn't set 
  // up the -P option
  if(OutDirName.length()!=0)
    {
      if(SavingPrimaries)
	{
	  // Opens the eventxxxxxx.kin file
	  FILE *oFile = NULL;
	  char buffer[40];
	  sprintf(buffer,"/event%.6d.kin",
		  anEvent->GetEventID() + SYNCEVT);
	  G4String FileName;  
	  FileName=OutDirName+buffer;
	  if((oFile=fopen(FileName,"w"))==NULL) 
	    {
	      perror(FileName);
	      exit(1);
	    }

	  // Loops on the TrackSummary vector
	  // For each track to be saved saves the track ID, the PDG code,
	  // the start position, the momentum and the particle charge,
	  // the initial energy, the parent ID and the end position.

	  size_t itr;
	  TrackSummary* theTrackSummary;
	  for(itr=0;itr<trackedtracks->size();itr++)
	    {
	      theTrackSummary = (*trackedtracks)[itr];
	      if (theTrackSummary->GetToBeSaved())
		{
		  if(theTrackSummary->GetMomentum().x() == 0. &&
		     theTrackSummary->GetMomentum().y() == 0. &&
		     theTrackSummary->GetMomentum().z() == 0. )
		    {
		      G4cout << "P = (0.,0.,0.): " 
			     << theTrackSummary->GetTrackID()
			     << G4endl;

		    }
		  fprintf(oFile,"%d %d %15e %15e %15e %15e %15e %15e %15e %15e %d %15e %15e %15e %d %X\n",
			  theTrackSummary->GetTrackID(),
			  theTrackSummary->GetPDG(),
			  theTrackSummary->GetVertex().x(),
			  theTrackSummary->GetVertex().y(),
			  theTrackSummary->GetVertex().z(),
			  theTrackSummary->GetMomentum().x() / GeV,
			  theTrackSummary->GetMomentum().y() / GeV,
			  theTrackSummary->GetMomentum().z() / GeV,
			  theTrackSummary->GetCharge(),
			  theTrackSummary->GetEnergy() / GeV,
			  theTrackSummary->GetParentID(),
			  theTrackSummary->GetEndPoint().x(),
			  theTrackSummary->GetEndPoint().y(),
			  theTrackSummary->GetEndPoint().z(),
			  theTrackSummary->GetHepEvtStatus(),
			  theTrackSummary->GetSimulatorStatus());
		}
	    }
	  //  closes the eventxxxxxx.kin file
	  fclose(oFile);
	}
      
      // Save the primariestrajectories  only if the user 
      // didn't set up the -P option.
      if(SavingTrajectories)
	{
	  // Opens the eventxxxxxx.steps output file
	  FILE *oFile = NULL;
	  char buffer[40];
	  sprintf(buffer,"/event%.6d.steps",
		  anEvent->GetEventID() + SYNCEVT);
	  G4String FileName;  
	  FileName=OutDirName+buffer;
	  if((oFile=fopen(FileName,"w"))==NULL) 
	    {
	      perror(FileName);
	      exit(1);
	    }
	  
	  // Looks for the theTrajectory container to retrieve trajectories
	  G4TrajectoryContainer *theTrajectoryContainer=
	    anEvent->GetTrajectoryContainer();
	  Trajectory* aTrajectory;
	  
	  // For each trajectory, saves only the steps attached to the
	  // primaries and at least 100 mm far each one.
	  for(int i_cont = 0; 
	      i_cont < theTrajectoryContainer->entries();
	      i_cont++)
	    {
	      aTrajectory= (Trajectory*)
		theTrajectoryContainer->operator[](i_cont);
	      
	      G4ThreeVector LastWrotePoint;
	      
	      // Tests if it's a primary, so parent ID == 0
	      if(aTrajectory->GetParentID()==0)
		for(int i_point = 0; 
		    i_point < aTrajectory->GetPointEntries();
		    i_point++)
		  {
		    G4TrajectoryPoint* thePoint= 
		      (G4TrajectoryPoint*)
		      ((Trajectory*) (aTrajectory))->GetPoint(i_point);
		    
		    G4ThreeVector DeltaVector;
		    G4ThreeVector thePosition = thePoint->GetPosition();
		    DeltaVector=thePosition-LastWrotePoint;
		    
		    if(i_point==0 || 
		       i_point==aTrajectory->GetPointEntries()-1 ||
		       DeltaVector.mag() > 100.)
		      {
			// Saves the track ID and the step point.
			fprintf(oFile,"%d %15e %15e %15e\n",
				aTrajectory->GetTrackID(),
				thePosition.x(),		      
				thePosition.y(),		      
				thePosition.z());
			LastWrotePoint=thePosition;
		      }
		  }
	    }
	  // closes the eventxxxxxx.steps file.
	  fclose(oFile);
	}
    }
#ifdef LCIO_MODE
  // Mokka LCIO interface
  // Create and register the MCParticle collection
  // of this event.

 
  if(lcEvt)
    {
      

      MCParticleMap.clear();

      if( lcMCVec == 0 ) 
	lcMCVec = new LCCollectionVec( LCIO::MCPARTICLE );

      size_t itr;
      TrackSummary* theTrackSummary;
      for(itr=0;itr<trackedtracks->size();itr++)
	{
	  theTrackSummary = (*trackedtracks)[itr];
	  if (theTrackSummary->GetToBeSaved())
	    {
	      MCParticleMap.
		insert(PIDMC(theTrackSummary->GetTrackID(),
			     theTrackSummary->GetMCParticle()));

	      // //trackerRMax=1842.9 trackerZMax = 2350
	      // const double* vtx = theTrackSummary->GetMCParticle()->getVertex() ;
	      // double rad = sqrt( vtx[0] * vtx[0] + vtx[1] * vtx[1] ) ;
	      // if( rad > 1842.9 || abs( vtx[2] )  > 2350. ){
	      // 	std::cout << " ########## WARNING - writing MCParticle outside of Tracking Volume - trackid: " << theTrackSummary->GetTrackID()<< "\n"
	      // 		  << * theTrackSummary->GetMCParticle() << std::endl ;
	      // }

 	      //fg: if we write the complete HepEvt we have to add only particles created in simulation ...
	      if( ! LCIOWriteCompleteHepEvt ||  theTrackSummary->GetMCParticle()->getGeneratorStatus() == 0 ){ 
		lcMCVec->push_back(theTrackSummary->GetMCParticle()) ;
	      }
	      
	    }
	}
      lcEvt->addCollection( (LCCollection*) lcMCVec , "MCParticle" ) ;

      lcMCVec = 0 ;
    }
#endif
}

void 
Control::LoadTracks(G4Event* anEvent)
{
  // Loads only if it's in persistency mode
  if(OutDirName.length()==0) return;

  // And only if the primaries and trajectories exist
  if(SavingPrimaries && SavingTrajectories)
    {
      G4cout << "Loading trajectories..." << G4endl;
      
      // Open the eventxxxxxx.kin input file
      char buffer[40];
      G4String FileName;  
      FILE *iFilePrimaries = NULL;
      sprintf(buffer,"/event%.6d.kin",
	      anEvent->GetEventID());
      FileName=OutDirName+buffer;
      if((iFilePrimaries=fopen(FileName,"r"))==NULL) 
	{
	  // If not found, just message it and returns
	  perror(FileName);
	  return;
	}
      
      // Open the eventxxxxxx.steps
      FILE *iFileSteps = NULL;
      sprintf(buffer,"/event%.6d.steps",
	      anEvent->GetEventID());
      FileName=OutDirName+buffer;
      if((iFileSteps=fopen(FileName,"r"))==NULL) 
	{
	  // If not found, just message it and returns
	  perror(FileName);
	  return;
	}
      
      // Creates a new trajectory container and 
      // a new Trajectory Point Container
      G4TrajectoryContainer *theTrajectoryContainer=
	new G4TrajectoryContainer();
      TrajectoryPointContainer* aTrajectoryPointContainer;
      
      // scratch
      Trajectory* aTrajectory;      
      G4int theTrackID, theStepTrackID,thePDGEncoding;
      G4double theCharge,x,y,z,px,py,pz;
      G4double theE,theEndPointX,theEndPointY,theEndPointZ;
      G4int theParentID,theHepEvtStatus;
      unsigned int theSimulatorStatus;

      // Load the file while we find a new primary
      while (fscanf(iFilePrimaries,"%d%d%le%le%le%le%le%le%le%le%d%le%le%le%d%X",
		    &theTrackID,
		    &thePDGEncoding,
		    &x,&y,&z,
		    &px,&py,&pz,
		    &theCharge,
		    &theE,
		    &theParentID,
		    &theEndPointX,&theEndPointY,&theEndPointZ,
		    &theHepEvtStatus,
		    &theSimulatorStatus) == 16)
	{
	  // Ignore not tracked primaries 
	  // 
	  if (theTrackID<0) continue;

	  // We need the Particle Definition to retrieve the 
	  // Particle Name. It's possible thanks to the 
	  // Particle Table FindParticle() method.
	  G4ParticleTable* theParticleTable = 
	    G4ParticleTable::GetParticleTable();
	  G4ParticleDefinition* theparticle =
	    theParticleTable->FindParticle(thePDGEncoding);
	  G4String theParticleName = "dummy";
	
	  if(theparticle) theParticleName = theparticle->GetParticleName();
	  else continue; 
	  //Abort("LoadTracks, FindParticle(thePDGEncoding) returns NULL !");

	  // Create, creates the new trajctory and initialize it.
	  aTrajectory = new Trajectory(theTrackID,0,
				       thePDGEncoding,theCharge,
				       theParticleName,
				       G4ThreeVector(px,py,pz));

	  // Attach it to the Trajectory Container
	  theTrajectoryContainer->push_back(aTrajectory);

	  // Creates the Trajectory Point Container for this primary
	  aTrajectoryPointContainer=
	    aTrajectory->GetTrajectoryPointContainer();

	  // Put the vertex point as the first
	  aTrajectoryPointContainer->
	    push_back(new G4TrajectoryPoint(G4ThreeVector(x,y,z)));
	  
	  // Re-read the steps file to scan the steps for this primary,
	  // attaching each one found to the Trajectory Point Container.
	  rewind(iFileSteps);
	  while (fscanf(iFileSteps,"%d%le%le%le",
			&theStepTrackID,&x,&y,&z)==4)
	    if(theStepTrackID==theTrackID)
	      aTrajectoryPointContainer->
		push_back(new G4TrajectoryPoint(G4ThreeVector(x,y,z)));
	}
      
      // CLose the input files
      fclose(iFilePrimaries);
      fclose(iFileSteps);
      
      // Attach the Trajectory Container to the event received as 
      // parameter
      anEvent->SetTrajectoryContainer(theTrajectoryContainer);
    }
}

void 
Control::LoadEvent(G4int EventNumber)
{
  G4Event* currentEvent = new G4Event(EventNumber);

  // About the hits:
  // We simulate here the normal sensitive detector protocol:
  // 1) initialisation to create the HitsCollections;
  // 2) load the hits in the files instead of getting it from
  //    the tracking;
  // 3) endofevent to put togheter the HitsCollections in the
  //    evt object.
  
  G4SDManager* sdManager = G4SDManager::GetSDMpointerIfExist();
  if(sdManager)
    currentEvent->SetHCofThisEvent(sdManager->PrepareNewEvent()); 

  for (unsigned int i_coll=0;i_coll<DETECTOR_DRIVERS.size();i_coll++)
    DETECTOR_DRIVERS[i_coll]->LoadEvent(currentEvent);

  sdManager->TerminateCurrentEvent(currentEvent->GetHCofThisEvent());

  // About the trajectories and tracks, reload it if saved.
  LoadTracks(currentEvent);

  // Sets the current event to be viewed.
  G4Event* LastEvt = VisuManager->GetCurrentEvt();
  if(LastEvt) delete LastEvt;
  VisuManager->SetCurrentEvt(currentEvent);

  // Refresh the screen
  G4UImanager* UImanager = G4UImanager::GetUIpointer();
  UImanager->ApplyCommand("/vis/viewer/refresh");
  VisuManager->Refresh();
}

G4int 
Control::GetPIDForCalHit(G4Step *)
{ 
// AZ: In old logic there was a bug - in case some back scattered track
// has no hits in CAL, it's child is marked as original track
// for hits.
// So, method must be completely rewritten with for example:
  if (FirstPIDInCal>primaryId){
    FirstPIDInCal=primaryId;
    
    //fg----------------------------------------------
    //     this causes very large numbers of MCParticles 
    //     from calorimeter showers to be stored - 
    //     if it turns out to be needed for dedicated calo studies
    //     activate old code with /Mokka/init/LCIOWriteParentsForBackscatter true
    if( LCIOWriteParentsForBackscatter) {

      TrackSummary* aTrackSummary = NULL;
      G4int itr;
      for(itr=trackedtracks->size()-1;itr>=0;itr--)
	{
	  aTrackSummary=(*trackedtracks)[itr];
	  if(aTrackSummary && 
	     aTrackSummary->GetTrackID()==primaryId)
	    break;
	}
      if(itr<0)
	Control::Abort("AZ: Tracks tree is broken... :(", MOKKA_OTHER_ERRORS);
      
       // std::cout << " **** Control::GetPIDForCalHit set track to be saved trackID: " << aTrackSummary->GetTrackID() 
       // 		<< " current toBeSaved: "  <<  aTrackSummary->GetToBeSaved()  << std::endl ;
      
      aTrackSummary->SetToBeSaved();
    }
    //fg---------------------------------------------------------------------
  }
  return FirstPIDInCal; 

//   G4Track* theTrack = aStep->GetTrack();
//   G4int currentPID = theTrack->GetTrackID();
//   if (FirstPIDInCal>currentPID)
//     {
// //       G4cout << "GetPIDForCalHit: FirstPIDInCal was " 
// // 	     << FirstPIDInCal 
// // 	     << " and it becomes "
// // 	     << currentPID
// // 	     <<  G4endl;
// //       char a;
// //       G4cin >> a;
      
//       FirstPIDInCal = currentPID;
//       // If the UserTrackInformation exists just flag
//       // tobesaved,
//       UserTrackInformation* theUserTrackInformation =
// 	(UserTrackInformation*) (theTrack->GetUserInformation());
//       if(theUserTrackInformation) 
// 	theUserTrackInformation->GetTheTrackSummary()->SetToBeSaved();
//       // else create it and attache it to track.
//       else
// 	{
// 	  TrackSummary* theTrackSummary =
// 	    new TrackSummary(theTrack,true);
// 	  GetTrackedtracks()->
// 	    push_back(theTrackSummary);
// 	  theTrack->
// 	    SetUserInformation(new 
// 			       UserTrackInformation(theTrackSummary));
// 	}
//     }
//   return FirstPIDInCal; 
}
