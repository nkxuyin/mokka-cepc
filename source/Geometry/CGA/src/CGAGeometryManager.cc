// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: CGAGeometryManager.cc,v 1.46 2009/04/23 15:42:41 mora Exp $
// $Name: mokka-07-00 $
//
// History
// - first implementation for the Mokka Common Geometry Access (CGA)
//   by Gabriel Musat (musat@poly.in2p3.fr), July 2002
// - updated to make use of G4NistManager, Adrian Vogel, 2006-07-11
// - updated to implement new GEAR interface -- K.Harder, T.Pinto Jayawardena  2007-07-31
//
// see CGA documentation at 
// http://mokka.in2p3.fr/software/doc/CGADoc/CGAIndex.html

#include "Control.hh"
#include "CGAGeometryManager.hh"
#include "CGAGeometryEnvironment.hh"
#include "VSubDetectorDriver.hh"
#include "VSuperSubDetectorDriver.hh"
#include "VSensitiveDetector.hh"
#include "G4UnitsTable.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4Element.hh"
#include "G4ElementTable.hh"
#include "G4Isotope.hh"
#include "G4ProductionCuts.hh"

#include "G4Box.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4PVPlacement.hh"  
#include "MyPlacement.hh"
#include "MySQLWrapper.hh"
#include <unistd.h>
#include "G4SDManager.hh"
#include "UserInit.hh"


CGAGeometryManager* 
CGAGeometryManager::theCGAGeometryManager = NULL;

//-------------------------------------------
// auxiliary class to keep the Model recipe
//-------------------------------------------
class SubDetector
{
 public:
  SubDetector(G4String name, G4String db, G4String driver,
	      G4String description, G4String subdriver,
	      G4int buildOrder)
    :   _name(name), _db(db), _driver(driver), 
	_description(description), _subdriver(subdriver), 
	_buildOrder(buildOrder) {}
  
  G4String _name;
  G4String _db;
  G4String _driver;
  G4String _description;
  G4String _subdriver;
  G4int _buildOrder;
};

//---------------------------------------------------
// function to sort the Model recipe by subdetector 
// build order.
//---------------------------------------------------
inline 
bool lt_buildOrder(SubDetector *s1, SubDetector *s2) 
{ 
return s1->_buildOrder < s2->_buildOrder; 
}

#ifdef MOKKA_GEAR
void CGAGeometryManager::GearSetup()
{
  for(size_t i=0;i<theRegisteredDrivers.size();i++)
    {
      theRegisteredDrivers[i]->GearSetupIfConstructed();
    }
}
#endif

CGAGeometryManager* 
CGAGeometryManager::GetCGAGeometryManager()
{
  if(theCGAGeometryManager == NULL) 
    theCGAGeometryManager  = new CGAGeometryManager();
  return theCGAGeometryManager;
}

void 
CGAGeometryManager::RegisterGeometryDriver(VSubDetectorDriver* aSubDetectorDriver)
{
  for (unsigned int idriver=0;idriver<theRegisteredDrivers.size();idriver++)
    if(theRegisteredDrivers[idriver]->IsApplicable(aSubDetectorDriver->GetName()))
      {
	G4cout << "Trying to register twice the same geometry driver name "
	       << aSubDetectorDriver->GetName()
	       << " !!!" 
	       << G4endl;
	Control::Abort("Trying to register twice the same geometry driver name!",MOKKA_OTHER_ERRORS);
      }
  theRegisteredDrivers.push_back (aSubDetectorDriver);
}

void 
CGAGeometryManager::RegisterGeometryRegion(G4Region* aRegion, G4double aCut)
{
  theRegisteredRegions.push_back(aRegion);
  theRegionCuts.push_back(aCut);
  G4ProductionCuts* cuts = new G4ProductionCuts;
  cuts->SetProductionCut(aCut*mm);
  aRegion->SetProductionCuts(cuts);

  G4cout << "CGAGeometryManager registered region : " << aRegion->GetName();
  G4cout << " with cut = " << aCut << "[mm]" << G4endl;
}

G4int
CGAGeometryManager::GetNumberOfRegions()
{
  return theRegisteredRegions.size();
}

G4Region*
CGAGeometryManager::GetGeometryRegion(G4int index)
{
  return theRegisteredRegions[index];
}

G4double
CGAGeometryManager::GetRegionCutValue(G4int index)
{
  return theRegionCuts[index];
}

void 
CGAGeometryManager::RegisterGeometryDriver(VSuperSubDetectorDriver* aSuperSubDetectorDriver)
{
  for (unsigned int idriver=0;idriver<theRegisteredSuperSubDrivers.size();idriver++)
    if(theRegisteredSuperSubDrivers[idriver]->IsApplicable(aSuperSubDetectorDriver->GetName()))
      {
	G4cout << "Trying to register twice the same geometry super driver name "
	       << aSuperSubDetectorDriver->GetName()
	       << " !!!" 
	       << G4endl;
	Control::Abort("Trying to register twice the same geometry super driver name!",MOKKA_OTHER_ERRORS);
      }
  theRegisteredSuperSubDrivers.push_back (aSuperSubDetectorDriver);
}


G4VPhysicalVolume* CGAGeometryManager::Construct()
{
  G4String message,query;

  //---------------------------------------------
  // Material definitions are the same for PROTO et 
  // full simulation
  //---------------------------------------------

  //--------------------------------------------------------
  // Open the geometry database connections
  //--------------------------------------------------------

  db = new Database(Control::MODELS_DBNAME.data());
  db_aux = new Database(Control::MODELS_DBNAME.data());
  
  //--------------------------------------------------------
  // Build the world volume, set some attributes and key parameters
  //--------------------------------------------------------
  WorldPhys = BuildWorldVolume();

  //--------------------------------------------------------
  // If backward compatible opens the g4g3.f new file
  //--------------------------------------------------------
  if(Control::DUMPG3)MyPlacement::Open();

  //--------------------------------------------------------
  // Initialize the LCIO Header, if any
  //--------------------------------------------------------
#ifdef LCIO_MODE
  InitializeLCIOHeader();
#endif

  //--------------------------------------------------------
  // Look for the Model in the models database to retrieve
  // the ingredient list
  //--------------------------------------------------------
  BuildRecipe();
  
  G4bool found=false;
  CGASetupParameters *SetupParameters;
  CGASetupParameters *GlobalParameters = Control::globalModelParameters;

  G4String the_name;
  G4String the_db;
  G4String the_driver;
  G4String the_description;
  G4String the_subdriver;
  G4int the_buildOrder;

  for (unsigned isub = 0;
       isub < Ingredients.size();
       isub++)
    {
      the_name = Ingredients[isub]->_name;
      the_db = Ingredients[isub]->_db;
      the_driver = Ingredients[isub]->_driver;
      the_description = Ingredients[isub]->_description;
      the_subdriver = Ingredients[isub]->_subdriver;
      the_buildOrder = Ingredients[isub]->_buildOrder;
      
      found=true; // okay, sub_detector exists in the model database
      // log it
      message = "\nBuilding sub_detector ";
      message += the_name;
      message += ", geometry db ";
      message += the_db;
      message += ", driver ";
      message += the_driver;
      message +=":\n";
      message += the_description;
      Control::Log(message);

      // Initialize the SetupParameters for this detector driver given the actual setup.
      // - initialize all sub_detector parameters with the default values
      query = "select parameter, driver_default_value, default_value from sharing as s,parameters where parameter = name and driver='";
      query += the_driver;
      query += "';";
      db_aux->exec(query.data());
      char **tuple;
      G4String parameter_default_value_source;
      G4String parameterName;
      SetupParameters = new CGASetupParameters();
      while((tuple=db_aux->getTuple()))
	{
	  if (tuple[1])
	    parameter_default_value_source = "driver_default_value";
	  else
	    parameter_default_value_source = "default_value";
	  
	  parameterName = db_aux->fetchString("parameter");

// 	  G4cout << "parameterName = "
// 		 << parameterName 
// 		 << ", parameter_default_value_source = " 
// 		 << parameter_default_value_source 
// 		 << G4endl;
// 	  exit(1);


	  (*SetupParameters)[parameterName]=
	    db_aux->fetchString(parameter_default_value_source);
	}      
      // - now overwrite it with the special detector model parameters, if any
      query = "select parameter, default_value from model_parameters where model = '";
      query += Control::DETECTOR_MODEL;
      query += "';";
      db_aux->exec(query.data());
      while(db_aux->getTuple())
	{
	  parameterName = db_aux->fetchString("parameter");
	  if(SetupParameters->find(parameterName) != SetupParameters->end())
	    (*SetupParameters)[parameterName]=
	      db_aux->fetchString("default_value");
	}

      // - now overwrite it with the current values in the given SETUP.
      query = "select p.parameter,p.value from setup_parameters as p left join sharing as r using (parameter) where r.driver = '";
      query += the_driver;
      query += "' and p.setup = '";
      query += Control::DETECTOR_SETUP;
      query += "';";
      db_aux->exec(query.data());
      while(db_aux->getTuple())
	{
	  parameterName = db_aux->fetchString("parameter");
	  if(SetupParameters->find(parameterName) != SetupParameters->end())
	    (*SetupParameters)[parameterName]=
	      db_aux->fetchString("value");
	}
      // and now overwrite it with the run time (from steering files and previous superdrivers)
      // global parameters.
      //
      for (std::map < G4String, G4String >::iterator 
	     par = (*Control::globalModelParameters).begin();
	   par != (*Control::globalModelParameters).end();
	   par++)
	if(SetupParameters->find((*par).first) != SetupParameters->end())
	  (*SetupParameters)[(*par).first] =  (*par).second;
      
      if(SetupParameters->size()>0)
	{
	  G4cout << "  Current parameters for the " 
		 << the_name << " detector : \n";
	  for (std::map < G4String, G4String >::iterator 
		 par = (*SetupParameters).begin();
	       par != (*SetupParameters).end();
	       par++)
	    G4cout << "   - " << (*par).first << " = " 
		   << (*par).second << G4endl;  
	}

      CGAGeometryEnvironment *aGeometryEnvironment =
	new CGAGeometryEnvironment(the_db,
				   detector_concept,
				   SetupParameters, 
				   GlobalParameters);
      
#ifdef LCIO_MODE
      if(runHdr) 
	runHdr->addActiveSubdetector( std::string(the_name) ) ;
#endif

      // Is it a SuperDriver?
      G4String sub_driver = the_driver; // the driver name, sub or super
      if(the_subdriver != "" )          // if !="" it's a super driver
	{
	  // builds the tmp database
	  BuildSubDriverEnvironment(sub_driver,*aGeometryEnvironment); 
	  // set sub_driver which the one which should actually build the
	  // device.
	  sub_driver = the_subdriver ;  
	}
      
      G4bool done,found;
      done = found = false;
      for (unsigned int idriver=0;
	   idriver<theRegisteredDrivers.size();
	   idriver++)
	if((found = theRegisteredDrivers[idriver]->
	   IsApplicable(sub_driver)))
	  {
	    done=theRegisteredDrivers[idriver]->
	      basic_construct(*aGeometryEnvironment,
			      WorldLog
#ifdef LCIO_MODE
			      , the_name
#endif
);
	    if(done)
	      done = theRegisteredDrivers[idriver]->
		PostConstructAction(*aGeometryEnvironment);
	    
	    theRegisteredDrivers[idriver]->setID(Control::
			                  DETECTOR_DRIVERS.size());
	    Control::
	      DETECTOR_DRIVERS.push_back(theRegisteredDrivers[idriver]);

	    
// 	    cout << " number of SD " << theRegisteredDrivers[idriver]->GetNumberOfSD()<<endl;
	    for(int isd=0;isd<theRegisteredDrivers[idriver]->GetNumberOfSD();isd++)
	      { 
// 		cout << " Ncoll " 
// 		     << theRegisteredDrivers[idriver]->getSD(isd)->GetNumberOfCollections() 
// 		     << endl;
		for (G4int i_coll=0; 
		     i_coll<theRegisteredDrivers[idriver]->
		       getSD(isd)->GetNumberOfCollections();
		     i_coll++)
		  {    
		    G4String ime=theRegisteredDrivers[idriver]->
		      getSD(isd)->GetCollectionName(i_coll);
// 		    cout << " ime " << ime << endl;
		    
		    std::vector<G4String>::iterator naso= 
		      find(Control::lcioStoreTRKHitMomentum.begin(),
			   Control::lcioStoreTRKHitMomentum.end(),
			   ime);
		    
		    if( naso!=Control::lcioStoreTRKHitMomentum.end() )
		      {
			theRegisteredDrivers[idriver]->SetSaveTRKHitMomentum(isd);
			Control::lcioStoreTRKHitMomentum.erase(naso);
		      }
		  }
		
	      }
	    BuildGeometrySubDetectorThree(theRegisteredDrivers[idriver]
					  ->GetBaseName());
	    
	    break;
	  }
      TmpDBCleanup();
      if(!found)
	Control::Abort("sub_detector driver doesn't exist! Construct failed",
		MOKKA_ERROR_SUBDETECTOR_CONSTRUCTION_FAILED);
      if(!done)
	Control::Abort("sub_detector construction failed",
		MOKKA_ERROR_SUBDETECTOR_CONSTRUCTION_FAILED);
      
      message = "Sub_detector ";
      message += the_name;
      message +=" DONE!\n";
      Control::Log(message);
      
      delete aGeometryEnvironment;
      if(dbtmp) 
	{
	  delete dbtmp;
	  dbtmp = NULL;
	}
    }
  if(!found)
    {
      if(Control::SUB_DETECTOR=="") Control::Abort("Model not found!",
		MOKKA_ERROR_MODEL_NOT_FOUND);
      else
	Control::Abort("Sub detector not found!",MOKKA_ERROR_SUBDETECTOR_NOT_FOUND);
    }
  
#ifdef LCIO_MODE
    if(Control::lcWrt) Control::lcWrt->writeRunHeader( runHdr ) ;
#endif

  if(Control::DUMPG3)MyPlacement::Close();
  
  
  // Update the tracker and calorimeter region parameters if changed by super drivers.
  G4double val = 0.;
  if(tracker_region_rmax == 0)
    {
      tracker_region_rmax = model_tracker_region_rmax;
      std::istringstream 
	is1((*Control::globalModelParameters)["tracker_region_rmax"]);
      is1 >> val;
      G4cout << "model's tracker_region_rmax = " 
	     << tracker_region_rmax << G4endl;
      if (val > 1.) 
	{
	  tracker_region_rmax = val;
	  G4cout << "===> tracker_region_rmax set to " 
		 << tracker_region_rmax << G4endl;
	}
    }
  else 
      G4cout << "From steering file : tracker_region_rmax = " 
	     << tracker_region_rmax << G4endl;
    
  if(tracker_region_zmax == 0)
    {
      tracker_region_zmax = model_tracker_region_zmax;
      val = 0;
      std::istringstream 
	is2((*Control::globalModelParameters)["tracker_region_zmax"]);
      is2 >> val;
      G4cout << "model's tracker_region_zmax = " 
	     << tracker_region_zmax << G4endl;
      if (val > 1.) 
	{
	  tracker_region_zmax = val;
	  G4cout << "===> tracker_region_zmax set to " 
		 << tracker_region_zmax << G4endl;
	}
    }
  else 
      G4cout << "From steering file : tracker_region_zmax = " 
	     << tracker_region_zmax << G4endl;

  if(calorimeter_region_rmax == 0)
    {
      calorimeter_region_rmax = model_calorimeter_region_rmax;
      val = 0;
      std::istringstream 
	is3((*Control::globalModelParameters)["calorimeter_region_rmax"]);
      is3 >> val;
      G4cout << "models's calorimeter_region_rmax = " 
	     << calorimeter_region_rmax << G4endl;
      if (val > 1.) 
	{
	  calorimeter_region_rmax = val;
	  G4cout << "===> calorimeter_region_rmax set to " 
		 << calorimeter_region_rmax << G4endl;
	}
    }
  else
      G4cout << "From steering file : calorimeter_region_rmax = " 
	     << calorimeter_region_rmax  << G4endl;

  if(calorimeter_region_zmax == 0)
    {
      calorimeter_region_zmax = model_calorimeter_region_zmax;
      val = 0;
      std::istringstream 
	is4((*Control::globalModelParameters)["calorimeter_region_zmax"]);
      is4 >> val;
      G4cout << "models's calorimeter_region_zmax = " 
	     << calorimeter_region_zmax << G4endl;
      if (val > 1.) 
	{
	  calorimeter_region_zmax = val;
	  G4cout << "===> calorimeter_region_zmax set to " 
		 << calorimeter_region_zmax << G4endl;
	}
    }
  else
      G4cout << "From steering file : calorimeter_region_zmax = " 
	     << calorimeter_region_zmax  << G4endl;
  
  G4cout << "\nDetector construction done.\n";

  // Warning fake subdetector names in the 
  // /Mokka/init/lcioStoreTRKHitMomentum command.
  for (std::vector<G4String>::iterator iDetectorName = 
	 Control::lcioStoreTRKHitMomentum.begin(); 
       iDetectorName < Control::lcioStoreTRKHitMomentum.end();
       iDetectorName++)
    G4cout << "WARNING: subdetector name '" 
	   << *iDetectorName
	   << "' given in /Mokka/init/lcioStoreTRKHitMomentum not found."
	   << G4endl;
  

  delete db;
  delete db_aux;

  // Returns the World Physical Volume to Run Manager
  return WorldPhys;
}

void
CGAGeometryManager::BuildSubDriverEnvironment(G4String aSuperDriverName,
					      CGAGeometryEnvironment& 
					      aGeometryEnvironment)
{
  G4String query,querydelete;
  G4bool done,found;
  done = found = false;
  
  for (unsigned int idriver=0;idriver<theRegisteredSuperSubDrivers.size();idriver++)
    if((found = theRegisteredSuperSubDrivers[idriver]->
       IsApplicable(aSuperDriverName)))
      {
	// check the parameters
	if(!theRegisteredSuperSubDrivers[idriver]->
	   CheckParameters(aGeometryEnvironment))
	  Control::Abort("Bad parameters for super driver.",
		MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);

	// Creates a tmp database for the Super driver tables
	//

	// Get connection_id
	dbtmp = new Database(Control::MODELS_DBNAME.data());
	G4String connection_id,unix_timestamp;
	dbtmp->exec("select CONNECTION_ID() as con;");
	if(!dbtmp->getTuple()) Control::Abort("Cannot identify connection_id!",
		MOKKA_ERROR_DATABASE_SELECT_ERROR);
	connection_id = dbtmp->fetchString("con");
	
	// Loops until a tmp database is available
	//
	// TIMEOUT_TO_RELAX_TMP fixes the time out to consider obsolete
	// old locks found in the tmp_databases tables. It depends on your
	// installation. Normally 10'' should be enough for stand-alone
	// installations, more when submitting to bath systems.
	//
	// SLEEP_BEFORE_RETRY defines the time to sleep before retry. Play
	// with this parameter to reduce the network load with retries.
	//
// #define TIMEOUT_TO_RELAX_TMP "120"
// #define SLEEP_BEFORE_RETRY 5

	std::string TIMEOUT_TO_RELAX_TMP = 
	  UserInit::getInstance()->getString("TIMEOUT_TO_RELAX_TMP")  ;
	if(  TIMEOUT_TO_RELAX_TMP.size() == 0 )  // not set in steering file
	  TIMEOUT_TO_RELAX_TMP = "120" ;
 	
	int SLEEP_BEFORE_RETRY = 
	  UserInit::getInstance()->getInt("SLEEP_BEFORE_RETRY")  ;
	if( SLEEP_BEFORE_RETRY == 0 ) // not set in steering file
	  SLEEP_BEFORE_RETRY = 5 ;
	
	G4bool waiting = true;
	while (waiting)
	  { 
	    // first we look for a released tmp database
	    dbtmp->exec("select UNIX_TIMESTAMP() as time;");
	    if(!dbtmp->getTuple()) Control::Abort("Cannot get unix_timestamp!",
		MOKKA_ERROR_DATABASE_SELECT_ERROR);
	    unix_timestamp= dbtmp->fetchString("time");
	    
	    G4cout << "CONNECTION_ID = " << connection_id
		   << ", UNIX_TIMESTAMP = " << unix_timestamp
		   << ", waiting for an available temporary database..."
		   << G4endl;
	    
	    dbtmp->exec("lock table tmp_databases WRITE;");
	    dbtmp->exec("select * from tmp_databases where `connection` = 0;");
	    if(!dbtmp->getTuple())
	      {
		// if not found, we try to release a obsolete lock
		query = "select * from tmp_databases where `connection` = 0 or ";
		query += unix_timestamp;
		query += " -  time_stamp > ";
		query += TIMEOUT_TO_RELAX_TMP;
		query += ";";
		dbtmp->exec(query.data());
		if(!dbtmp->getTuple())
		  {
		    // It seems really busy, lets wait a bit.
		    dbtmp->exec("unlock tables;");
		    G4cout << "All temporary databases are busy, sleeping for a while..." << G4endl;
		    sleep(SLEEP_BEFORE_RETRY);
		    continue;
		  }
	      }
	    tmpDBName = dbtmp->fetchString("name");
	    query = "update tmp_databases set `connection` = ";
	    query += connection_id;
	    query += ", time_stamp = ";
	    query += unix_timestamp;
	    query += " where name = '";
	    query += tmpDBName;
	    query += "';";
	    dbtmp->exec(query.data());
	    dbtmp->exec("unlock tables;");
	    waiting = false;
	  }
	G4cout << "tmp database name is " << tmpDBName << G4endl;
	TmpDBCleanup(false);
	query = "use " + tmpDBName + ";" ;
	dbtmp->exec(query.data());	

	// calls the PreLoadScriptAction to initialize the database
	done = theRegisteredSuperSubDrivers[idriver]->
	  PreLoadScriptAction(dbtmp,
			      aGeometryEnvironment);
	// runs the MySQL script which populates the database
	if (done && 
	    theRegisteredSuperSubDrivers[idriver]->NeedRunMysqlScript()) 
	  done = 
	    LoadandExecMySQLScript(theRegisteredSuperSubDrivers[idriver]->
				   GetStringName ());
	
	// calls the PostLoadScriptAction if any to retrieve global parameters
        // to be postponed.
	if (done) 
	  done = theRegisteredSuperSubDrivers[idriver]->
	    PostLoadScriptAction(dbtmp,
				 aGeometryEnvironment);
	break;
      }
  if(!found)Control::Abort("Super detector driver doesn't exist! Construct failed",MOKKA_ERROR_SUBDETECTOR_CONSTRUCTION_FAILED);
  if(!done)
    {
      TmpDBCleanup();
      Control::Abort("Super detector construction failed",MOKKA_ERROR_SUBDETECTOR_CONSTRUCTION_FAILED);
    }
  aGeometryEnvironment.SetDBName(tmpDBName);
  G4String message;
  message = "Super detector ";
  message += aSuperDriverName;
  message +=" DONE!\n";
  Control::Log(message);
}

G4bool 
CGAGeometryManager::LoadandExecMySQLScript(G4String aScriptName)
{  
  G4String message;
  Database* db = new Database(Control::MODELS_DBNAME.data());
  G4String query;
  query = "select TRIM(script) as script from scripts where name='";
  query += aScriptName;
  query +="';";
  db->exec(query);
  if(!db->getTuple())
    {
      message = "MySQL script not found: ";
      message += aScriptName;
      message +="\n";
      Control::Abort(message,MOKKA_ERROR_MYSQL_SCRIPT_NOT_FOUND);
    }
  
  G4String Script = db->fetchString("script");
  delete db;

  if(Script(Script.length()-1) != ';' && Script(Script.length()-1) != '\n' )
    Script += ";";
  
  G4int i = 0;
  while (Script.contains(';'))
    {
      i++;
      str_size i_virg = Script.index(';');
      G4String cmd = Script;
      cmd.remove(i_virg+1,Script.length());
      dbtmp->exec(cmd.data());
      
      Script.remove(0,i_virg+1);
      i_virg = Script.index(';');
    }
    return true;
}

void
CGAGeometryManager::TmpDBCleanup(G4bool releaseDB)
{
  if(tmpDBName != "") 
    {
      G4String query,tableName;
      query = "use " + tmpDBName + ";" ;
      dbtmp->exec(query.data());	  
      dbtmp->exec("show tables;");
      while (dbtmp->getTuple())
	{
	  tableName = dbtmp->fetchString(0); // the table names appear in the first (and only) column
	  query = "drop table `" + tableName + "`;";
	  dbtmp->exec(query.data());
	  dbtmp->exec("show tables;");
	}
      if(releaseDB)
	{
	  query = "use " + Control::MODELS_DBNAME + ";";
	  dbtmp->exec(query.data());
	  query = "update tmp_databases set `connection` = 0, time_stamp = 0 where name = '";
	  query += tmpDBName;
	  query += "';";
	  dbtmp->exec(query.data());
	  tmpDBName = "";
	}
    }
}

/********* Elements and Materials *******************************************/

G4Material *CGAGeometryManager::GetMaterial(const G4String &materialName)
{
  G4Material *materialDefinition = 0;
  G4NistManager *nistManager = G4NistManager::Instance();
  static std::map<G4String, G4String> mokka2NistNameMap;
  
  if (mokka2NistNameMap.find(materialName) != mokka2NistNameMap.end()) // mapped to a NIST material with another name?
    materialDefinition = G4Material::GetMaterial(mokka2NistNameMap[materialName]); // NIST material has been constructed
  else
    materialDefinition = G4Material::GetMaterial(materialName, false); // is the material already known? (without warning)
  if (materialDefinition) return materialDefinition; // material has been constructed before, we're already done

  materialDefinition = nistManager->FindOrBuildMaterial(materialName); // does the material exist in the NIST database?
  if (materialDefinition) return materialDefinition; // NIST manager knows this material, we're already done
  
  Database *materialsDB = new Database(Control::MATERIALS_DBNAME.c_str());
  materialsDB->exec(G4String("SELECT * FROM `materials` WHERE `name` = '" + materialName + "';").c_str());
  if (!materialsDB->getTuple()) { // material not found in the database
    G4cout << "CGAGeometryManager::GetMaterial: The material \"" << materialName << "\" is not in the materials database." << G4endl;
    Control::Abort("CGAGeometryManager::GetMaterial failed.",
		MOKKA_ERROR_CANNOT_BUILD_MATERIAL);
  }

  G4String nistName    = materialsDB->fetchString("nistName");
  if (!nistName.empty()) { // material should be in the NIST database with another name
    materialDefinition = nistManager->FindOrBuildMaterial(nistName);
    if (!materialDefinition) {
      G4cout << "CGAGeometryManager::GetMaterial: The material \"" << nistName << "\" (\"" << materialName << "\") is unknown to the G4NistManager." << G4endl;
      Control::Abort("CGAGeometryManager::GetMaterial failed.",
		MOKKA_ERROR_CANNOT_BUILD_MATERIAL);
    }
    mokka2NistNameMap[materialName] = nistName; // remember this mapping for subsequent calls to this function

  } else { // material is constructed from the Mokka database
    G4double density     = materialsDB->fetchDouble("density") * g/cm3;
    G4double temperature = materialsDB->fetchDouble("temperature") * kelvin;
    G4double pressure    = materialsDB->fetchDouble("pressure") * bar;
    G4String stateStr    = materialsDB->fetchString("state");

    G4State                        state = kStateUndefined; // same as the default value in the constructor
    if      (stateStr == "solid")  state = kStateSolid;
    else if (stateStr == "liquid") state = kStateLiquid;
    else if (stateStr == "gas")    state = kStateGas;
  
    if (!density) { // a density must be given, otherwise the constructor will complain
      G4cout << "CGAGeometryManager::GetMaterial: The material \"" << materialName << "\" has no density in the materials database." << G4endl;
      Control::Abort("CGAGeometryManager::GetMaterial failed.",
		MOKKA_ERROR_CANNOT_BUILD_MATERIAL);
    }
    if (!temperature) temperature = STP_Temperature; // same as the default value in the constructor
    if (!pressure)    pressure    = STP_Pressure;    // same as the default value in the constructor

    materialsDB->exec(G4String("SELECT * FROM `components` WHERE `material` = '" + materialName + "';").c_str());
    const G4int nComponents = materialsDB->nrTuples();
    if (nComponents == 0) { // no components found in the database
      G4cout << "CGAGeometryManager::GetMaterial: Composition of the material \"" << materialName << "\" is not in the materials database." << G4endl;
      Control::Abort("CGAGeometryManager::GetMaterial failed.",
		MOKKA_ERROR_CANNOT_BUILD_MATERIAL);
    }
    
    materialDefinition = new G4Material(materialName, density, nComponents, state, temperature, pressure);
    while (materialsDB->getTuple()) {
      const G4String componentName = materialsDB->fetchString("component");
      const G4int    nAtoms        = materialsDB->fetchInt("nAtoms");
      const G4double fraction      = materialsDB->fetchDouble("fraction");
    
      if ((nAtoms && fraction) || (!nAtoms && !fraction)) { // either nAtoms or fraction must be given
        G4cout << "CGAGeometryManager::GetMaterial: The composition of the material \"" << materialName << "\" is invalid in the materials database." << G4endl;
        Control::Abort("CGAGeometryManager::GetMaterial failed.",
		MOKKA_ERROR_CANNOT_BUILD_MATERIAL);
      }
      if (nAtoms) { // "nAtoms" is sensible only for elements
        materialDefinition->AddElement(GetElement(componentName), nAtoms);
      } else { // "fraction" can be sensible both for elements and materials - what is it?
        G4Element *componentDefinition = GetElement(componentName, false); // without warning
        if (componentDefinition) materialDefinition->AddElement(componentDefinition, fraction);
        else                     materialDefinition->AddMaterial(GetMaterial(componentName), fraction);
      }
    } // while (materialsDB->getTuple())
  } // if (nistName)
  G4cout << materialName << "->GetRadlen() = " << G4BestUnit(materialDefinition->GetRadlen(), "Length") << G4endl;
  
  delete materialsDB;
  return materialDefinition;
}

G4Element *CGAGeometryManager::GetElement(const G4String &elementName, G4bool warning)
{
  G4NistManager *theNistManager = G4NistManager::Instance();
  G4Element *elementDefinition = theNistManager->FindOrBuildElement(elementName, true); // with isotopes
  if (!elementDefinition && warning) {
    G4cout
      << "CGAGeometryManager::GetElement: The element \"" << elementName
      << "\" is unknown to the G4NistManager." << G4endl;
    Control::Abort("CGAGeometryManager::GetElement failed.",
		MOKKA_ERROR_CANNOT_BUILD_MATERIAL);
  }
  return elementDefinition;
}

CGAGeometryManager::~CGAGeometryManager()
{
   unsigned int i = 0;
   for( i = 0; i < theGeometryThree->size(); i++)
	delete (*theGeometryThree)[i];

   theGeometryThree->clear();
   delete theGeometryThree;

   for( i = 0; i < Ingredients.size(); i++)
	delete Ingredients[i];
   Ingredients.clear();
}

void
CGAGeometryManager::BuildGeometrySubDetectorThree(G4String aSubDetectorName)
{
  G4int i_daughter, i_level = 0;
  G4bool found;
  unsigned int i_vol;

//   G4cout << "Vols du sous detector "
// 	 << aSubDetectorName
// 	 << " : \n";

  unsigned int first_new_GT_entry = theGeometryThree->size();
  
  // register the LV from this sub-detector
  for (i_daughter = last_world_daughter;
       i_daughter < WorldLog->GetNoDaughters();
       i_daughter ++)
    {
      // books just one time the LV
      found = false;
      for (i_vol = 0;
	   i_vol < theGeometryThree->size();
	   i_vol ++)
	if(theGeometryThree->operator[](i_vol)->LV ==
	   WorldLog->GetDaughter(i_daughter)->GetLogicalVolume())
	  {
	    found = true;
	    break;
	  }
      if(!found)
	{
	  // If not set creates a dummy vis attribute
	  if(WorldLog->GetDaughter(i_daughter)
	     ->GetLogicalVolume()->GetVisAttributes() == 0)
	    WorldLog->GetDaughter(i_daughter)
	      ->GetLogicalVolume()->SetVisAttributes(new G4VisAttributes());
	  theGeometryThree->
	    push_back(new 
		      LV_level(aSubDetectorName,
			       WorldLog->GetDaughter(i_daughter)
			       ->GetLogicalVolume(),
			       i_level));
	}
      
    }
  last_world_daughter = WorldLog->GetNoDaughters();
  
  // go down in the sub detector three
  while (first_new_GT_entry < theGeometryThree->size())
    {
      i_level++;
      unsigned int local_first_new_GT_entry = first_new_GT_entry;
      unsigned int local_last_entry;
      local_last_entry = first_new_GT_entry = theGeometryThree->size();
      for (unsigned int i_mother = local_first_new_GT_entry;
	   i_mother < local_last_entry;
	   i_mother ++)
	{
	  LV_level *aLV_level = theGeometryThree->operator[](i_mother);
	  G4LogicalVolume* aLV = aLV_level->LV;
	  for (i_daughter = 0;
	       i_daughter < aLV->GetNoDaughters();
	       i_daughter ++)
	    {
	      // books just one time the LV
	      G4bool found = false;
	      for (unsigned int i_vol = 0;
		   i_vol < theGeometryThree->size();
		   i_vol ++)
		if(theGeometryThree->operator[](i_vol)->LV ==
		   aLV->GetDaughter(i_daughter)->GetLogicalVolume())
		  {
		    found = true;
		    break;
		  }
	      if(!found)
		{
		  // If not set creates a dummy vis attribute
		  if(aLV->GetDaughter(i_daughter)
		     ->GetLogicalVolume()->GetVisAttributes() == 0)
		    aLV->GetDaughter(i_daughter)
		      ->GetLogicalVolume()->SetVisAttributes(new G4VisAttributes());
		  theGeometryThree->
		    push_back(new 
			      LV_level(aSubDetectorName,
				       aLV->GetDaughter(i_daughter)
				       ->GetLogicalVolume(),
				       i_level));
		}
	    }
	}

    }
}

G4VPhysicalVolume*
CGAGeometryManager::BuildWorldVolume()
{
  //--------------------------------------------------------
  // Check the detector model
  //--------------------------------------------------------
  
  if( Control::DETECTOR_MODEL == "" )  
    {
      Control::Abort("No Mokka Model specified: use either command-line option -M or specify it in the steering file using /Mokka/init/detectorModel [Model_Name] ",MOKKA_ERROR_WRONG_USAGE);
    } 
  
  G4cout << "CGAGeometryManager starting the detector construction: " << G4endl;
  G4cout << "Asking for the model " << Control::DETECTOR_MODEL << ":";
  G4String query("select * from model where name='");
  query+=Control::DETECTOR_MODEL;
  query+="';";
  db->exec(query.data());
  if(!db->getTuple()) Control::Abort("Model not found!",
		MOKKA_ERROR_MODEL_NOT_FOUND);

  detector_concept = db->fetchString("detector_concept");
  G4cout << " found. \nDetector concept is \"" << detector_concept 
	 << "\", detector setup is \"" << Control::DETECTOR_SETUP 
	 << "\"."  << G4endl;

  
  //--------------------------------------------------------
  // Check the  model status
  //--------------------------------------------------------
  G4String model_status = db->fetchString("model_status");
  if (model_status == "frozen" ) {
    if((Control::SUB_DETECTOR == "") && Control::EditGeometry && !Control::ModelOpened)
	Control::Abort("To modify a frozen model use the command-line option -U",MOKKA_ERROR_WRONG_USAGE);
    G4cout << "Database says that this model is frozzen and good for production." << G4endl;
  }
  else
    {
      G4cout << "**************************************************\n"
	     << "*                  BE CAREFUL                    *\n"
	     << "*    This detector model is still unstable!!!    *\n"
	     << "**************************************************\n"
	     << G4endl;
    } 
  
  //--------------------------------------------------------
  // Check the setup name 
  //--------------------------------------------------------
  if(Control::DETECTOR_SETUP != "")
    {
      query="select * from setup where name ='";
      query += Control::DETECTOR_SETUP.data();
      query += "';";
      db->exec(query.data());
      if(db->nrTuples() == 0)
	{
	  G4String message = "Setup \"" 
	    + Control::DETECTOR_SETUP 
	    + "\" not found in the models database!";
	  Control::Abort(message,MOKKA_ERROR_DETECTOR_SETUP_NOT_FOUND);
	}      
    }

  //--------------------------------------------------------
  // Check the detector concept
  //--------------------------------------------------------
  query =  G4String("select * from detector_concept where name ='") ;
  query += detector_concept.data();
  query+="';";
  db->exec(query.data());
  if(!db->getTuple()) Control::Abort("detector concept not found in database!",
		MOKKA_ERROR_DETECTOR_CONCEPT_NOT_FOUND);

  //--------------------------------------------------------
  // Keep the defaults for tracker and calorimeter regions
  //--------------------------------------------------------

  G4double val = 0.;
  std::istringstream 
    is1((*Control::globalModelParameters)["tracker_region_rmax"]);
  is1 >> val;
  if (val > 1.) 
    {
      tracker_region_rmax = val;
      G4cout << "===> sterring file sets tracker_region_rmax to " 
	     << tracker_region_rmax << G4endl;
    }

  val = 0;
  std::istringstream 
    is2((*Control::globalModelParameters)["tracker_region_zmax"]);
  is2 >> val;
  if (val > 1.) 
    {
      tracker_region_zmax = val;
      G4cout << "===> sterring file sets tracker_region_zmax to " 
	     << tracker_region_zmax << G4endl;
    }

  val = 0;
  std::istringstream 
    is3((*Control::globalModelParameters)["calorimeter_region_rmax"]);
  is3 >> val;
  if (val > 1.) 
    {
      calorimeter_region_rmax = val;
      G4cout << "===> sterring file sets calorimeter_region_rmax to " 
	     << calorimeter_region_rmax << G4endl;
    }

  val = 0;
  std::istringstream 
    is4((*Control::globalModelParameters)["calorimeter_region_zmax"]);
  is4 >> val;
  if (val > 1.) 
    {
      calorimeter_region_zmax = val;
      G4cout << "===> sterring file sets calorimeter_region_zmax to " 
	     << calorimeter_region_zmax << G4endl;
    }

  model_tracker_region_rmax =
    db->fetchDouble("tracker_region_rmax");
  if(model_tracker_region_rmax < 1.) tracker_region_rmax = 0.;
  model_tracker_region_zmax =
    db->fetchDouble("tracker_region_zmax");
  if(model_tracker_region_zmax < 1.) tracker_region_zmax = 0.;
  model_calorimeter_region_rmax =
    db->fetchDouble("calorimeter_region_rmax");
  model_calorimeter_region_zmax =
    db->fetchDouble("calorimeter_region_zmax");
  
  

  //--------------------------------------------------------
  // Build the world volume
  //--------------------------------------------------------
  G4double world_box_hx,world_box_hy,world_box_hz;
  world_box_hx=db->fetchDouble("world_box_hx");
  world_box_hy=db->fetchDouble("world_box_hy");
  world_box_hz=db->fetchDouble("world_box_hz");

  G4Box *WorldBox= new G4Box("WorldBox",world_box_hx, world_box_hy, world_box_hz);
  WorldLog=new G4LogicalVolume(WorldBox,GetMaterial("air"),
			       "WorldLogical", 0, 0, 0);
  G4PVPlacement *WorldPhys=new G4PVPlacement(0,G4ThreeVector(),
					     "WorldPhysical",
					     WorldLog,
					     0,false,0);
  G4VisAttributes * experimantalHallVisAtt
    = new G4VisAttributes(G4Colour(0.0,0.0,0.0));
  experimantalHallVisAtt->SetVisibility(false);
  WorldLog->SetVisAttributes(experimantalHallVisAtt);
  return WorldPhys;
}

void 
CGAGeometryManager::BuildRecipe()
{
  //--------------------------------------------------------
  // Look for the Model in the models database, get
  // and build the ingredient list
  //--------------------------------------------------------
  G4String query;
  G4String the_name;
  G4String the_db;
  G4String the_driver;
  G4String the_description;
  G4String the_subdriver;
  G4int the_buildOrder;

  if(Control::SUB_DETECTOR=="") 
    {
      // check database consistence
      query="select sub_detector,name from ingredients left join sub_detector on name=sub_detector where name is NULL and model = '";
      query+=Control::DETECTOR_MODEL;
      query+="' order by ingredients.sub_detector;";
      db->exec(query.data());
      if(db->nrTuples()>0)
	{ 
	  G4cout << " Detector model asks for missing components:\n";
	  while(db->getTuple())
	    G4cout << "Missing sub-detector "
		   << db->fetchString("sub_detector") 
		   << " in sub-detector table!"
		   << G4endl;
	  Control::Abort("Construct failed!",MOKKA_ERROR_SUBDETECTOR_NOT_FOUND);
	}
      
      // okay, lets try to load it
      query="select model.name as model,sub_detector.name as sub_detector,db,driver,sub_detector.description as description,sub_detector.subdriver as subdriver, build_order from model,sub_detector,ingredients where model.name=ingredients.model and sub_detector.name=ingredients.sub_detector and  model.name='";
      query+=Control::DETECTOR_MODEL;
      query+="' order by ingredients.build_order;";
    }
  else 
    {
      query="select name as sub_detector,db,driver,description,subdriver, 0 as build_order from sub_detector where name='";
      query+=Control::SUB_DETECTOR;
      query+="';";
    }
  db->exec(query.data());
  while(db->getTuple())
    {
      the_name = db->fetchString("sub_detector");
      the_db = db->fetchString("db");
      the_driver = db->fetchString("driver");
      the_description = db->fetchString("description");
      the_subdriver = db->fetchString("subdriver");
      the_buildOrder =  db->fetchInt("build_order");
      
      Ingredients.
	push_back(new 
		  SubDetector(the_name,the_db,the_driver,the_description,
			      the_subdriver,the_buildOrder));
    }
  
  if(Control::GeometryEditions.size()>0)
    {
      G4cout << "\nCooking the geometry, original model recipe in database:\n";
      G4cout << "(subdetector/database/driver/sub_driver/build_order)\n";
      for (unsigned isub = 0;
	   isub < Ingredients.size();
	   isub++)
	G4cout << Ingredients[isub]->_name << " / "
	       << Ingredients[isub]->_db << " / "
	       << Ingredients[isub]->_driver << " / "
	       << Ingredients[isub]->_subdriver << " / "
	       << Ingredients[isub]->_buildOrder << G4endl;
      
      G4cout << "\nEdition commands\n(0=add, 1=rm / name / build_order)\n";
      for (unsigned iedit = 0;
	   iedit < Control::GeometryEditions.size();
	   iedit++)
	{
	  G4cout << Control::GeometryEditions[iedit]->_Op << " / "
		 << Control::GeometryEditions[iedit]->_subDetectorName << " / "
		 << Control::GeometryEditions[iedit]->_buildOrder << G4endl;
	  switch (Control::GeometryEditions[iedit]->_Op)
	    {
	    case ADD:
	      query="select name as sub_detector,db,driver,description,subdriver, 0 as build_order from sub_detector where name='";
	      query+=Control::GeometryEditions[iedit]->_subDetectorName;
	      query+="';";
	      db->exec(query.data());
	      if(db->getTuple()==0)
		{
		  Control::Log("In command addSubDetector, sub detector:");
		  Control::Log(Control::GeometryEditions[iedit]->_subDetectorName);
		  Control::Log("unknown.");
		  Control::Abort("",MOKKA_ERROR_SUBDETECTOR_NOT_FOUND);
		}
	      the_name = db->fetchString("sub_detector");
	      the_db = db->fetchString("db");
	      the_driver = db->fetchString("driver");
	      the_description = db->fetchString("description");
	      the_subdriver = db->fetchString("subdriver");
	      the_buildOrder = Control::GeometryEditions[iedit]->_buildOrder;
	      
	      Ingredients.
		push_back(new 
			  SubDetector(the_name,the_db,the_driver,the_description,
				      the_subdriver,the_buildOrder));
	      break;
	    case REMOVE:
	      if(Control::GeometryEditions[iedit]->_subDetectorName == "all")
		Ingredients.clear();
	      else
		for (std::vector<SubDetector*>::iterator isub 
		       = Ingredients.begin();
		     isub < Ingredients.end();
		     isub++)
		  if((*isub)->_name == 
		     Control::GeometryEditions[iedit]->_subDetectorName)
		    Ingredients.erase(isub);
	      break;
	    default:
	      Control::Abort("Unknown operation in GeometryEditions!",
			MOKKA_ERROR_WRONG_USAGE);
	    }
	}
      
      std::sort(Ingredients.begin(),Ingredients.end(),lt_buildOrder);
      
      G4cout << "\n\nFinal model recipe after cooking it:\n";
      G4cout << "(subdetector/database/driver/sub_driver/build_order)\n";
      for (unsigned isub = 0;
	   isub < Ingredients.size();
	   isub++)
	G4cout << Ingredients[isub]->_name << " / "
	       << Ingredients[isub]->_db << " / "
	       << Ingredients[isub]->_driver << " / "
	       << Ingredients[isub]->_subdriver << " / "
	       << Ingredients[isub]->_buildOrder << G4endl;
    }
}

#ifdef LCIO_MODE
void 
CGAGeometryManager::InitializeLCIOHeader()
{
  runHdr = Control::lcRunHdr ; 
  if(Control::lcWrt) 
    {
      if( runHdr == 0 ) 
	runHdr = new LCRunHeaderImpl;
      
      runHdr->setRunNumber( Control::mcRunNumber );
      runHdr->setDetectorName( Control::DETECTOR_MODEL ) ;
      
      std::string description("LCIO output file created by Mokka") ;
      runHdr->setDescription( description ) ;
      
      // set a few parameters in the LCIO file
      if (LCIO::MAJORVERSION > 0 || LCIO::MINORVERSION > 1)
	{
	  runHdr->parameters().setValue( "SimulatorName", "Mokka" ) ;
	  runHdr->parameters().setValue( "SimulatorVersion", Control::MokkaVersionName ) ;
	  runHdr->parameters().setValue( "PhysicsList", Control::PhysicsListName ) ;
	  // Mokka specif parameters
	  runHdr->parameters().setValue( "MOKKA_DetectorSetup", Control::DETECTOR_SETUP );
	  runHdr->parameters().setValue( "MOKKA_SubDetector", Control::SUB_DETECTOR );
	  runHdr->parameters().setValue( "MOKKA_DBHost" , Control::DBHOST );
	  runHdr->parameters().setValue( "MOKKA_BFactor" , (float)Control::BFactor);
	  runHdr->parameters().setValue( "MOKKA_ConfigAngle" , (float)Control::ConfigAngle);

//GM: new parames as suggested by Anne-Marie Magnan:
	  runHdr->parameters().setValue( "DataRunNumber" , (int)Control::DataRunNumber);
	  runHdr->parameters().setValue( "ConfDataTag" , Control::ConfDataTag);

//GM: new param suggested by Angela Lucaci
	  runHdr->parameters().setValue( "GEANT4version", G4Version);
    
    runHdr->parameters().setValue( "MOKKA_RandomSeed", Control::RandomSeed);
    
	}
    }
}
#endif
