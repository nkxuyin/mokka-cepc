/*
 * BCal SuperDriver for Mokka
 *
 * SBcal01.cc - v1.0 Oct. 2008
 *
 * Version history:
 *   1.0 - first implementation. 
 *    Scales to the LHcal by using a z clearance parameter
 *    A.Hartin 23/10/2008
 *
 */

#include "MySQLWrapper.hh"
#include "Control.hh"
#include "SBcal01.hh"

#include "globals.hh"
#include "CGADefs.h"



INSTANTIATE(SBcal01)


SBcal01::SBcal01() : VSuperSubDetectorDriver("SBcal01")
{

}

SBcal01::~SBcal01()
{
}

G4bool SBcal01::PreLoadScriptAction(Database* dbtmp,
			     CGAGeometryEnvironment& theGeometryEnvironment)
{
  G4String query;

  G4String dbName = theGeometryEnvironment.GetDBName();
  
  G4String crossingAngle = theGeometryEnvironment.GetParameterAsString("ILC_Main_Crossing_Angle");

  G4cout << G4endl <<"SBcal01 - the BeamCal super driver v1.0 with DB:" << dbName << G4endl;

////////// create the beamcal table ////////////////

  query = "CREATE TABLE beamcal SELECT * FROM "+dbName+"_"+crossingAngle+".beamcal;";
  G4cout << query << G4endl;
  dbtmp->exec(query.data());

/////

  G4cout << "SBcal01 : temp table created " << G4endl;
//////////// get values from beamcal table, scale zStart to end of LHcal with specified clearance/////////////

  query = "SELECT * FROM beamcal;";
  dbtmp->exec(query.data());
  dbtmp->getTuple();
  G4cout << "SBcal01 : DB init done " << G4endl;

  G4double Rinner = dbtmp->fetchDouble("Rinner");
  G4double Router = dbtmp->fetchDouble("Router");
  G4double sPhi = dbtmp->fetchDouble("sPhi");
  G4double dPhi = dbtmp->fetchDouble("dPhi");
  G4double nWafers = dbtmp->fetchDouble("nWafers");
  G4double BPmaxR = dbtmp->fetchDouble("BPmaxR"); 
  G4double LHcal_Bcal_clr = dbtmp->fetchDouble("LHcal_Bcal_clr");
  G4double dAbsorber = dbtmp->fetchDouble("dAbsorber");
  G4double dSensor = dbtmp->fetchDouble("dSensor");
  G4double dAirgap = dbtmp->fetchDouble("dAirgap");
  G4double dElboard = dbtmp->fetchDouble("dElboard");
  G4double dElectrMet = dbtmp->fetchDouble("dElectrMet");
  G4double nLayers = dbtmp->fetchDouble("nLayers");
  G4double Segm = dbtmp->fetchDouble("Segm");
  G4double dGraphite = dbtmp->fetchDouble("dGraphite");

  G4double zStart = theGeometryEnvironment.GetParameterAsDouble("LHcal_zend") + LHcal_Bcal_clr;
  G4cout << "SBcal01 : zStart  " << zStart << G4endl;

 ////////////fill in the beamcal table with values above///////////// 
  G4cout << "SBcal01: fill in the beamcal table with values above" << G4endl;

  std::ostringstream querystr;  
  querystr << "UPDATE beamcal Set zStart=" << zStart << ";" ;    
  query = querystr.str();    
  dbtmp->exec(query.data());

  G4cout << "SBcal01::PreLoadScriptAction() done." << G4endl;
  return true;
}


G4bool SBcal01::PostLoadScriptAction(Database*  dbtmp,
			     CGAGeometryEnvironment& )
{

  //show what actually has been set
  dbtmp->exec("SELECT * FROM beamcal;");
  dbtmp->getTuple();

  G4cout << "SBcal01 has set the following values :" << G4endl 
 	 << "    Rinner  = " << dbtmp->fetchDouble("Rinner") << G4endl
 	 << "    Router  = " << dbtmp->fetchDouble("Router") << G4endl
 	 << "    sPhi    = " << dbtmp->fetchDouble("sPhi") << G4endl
 	 << "    dPhi    = " << dbtmp->fetchDouble("dPhi") << G4endl
 	 << "    nWafers = " << dbtmp->fetchDouble("nWafers") << G4endl
 	 << "    BPmaxR  = " << dbtmp->fetchDouble("BPmaxR") << G4endl
 	 << "    zStart  = " << dbtmp->fetchDouble("zStart") << G4endl
 	 << "    LHcal_Bcal_clr  = " << dbtmp->fetchDouble("LHcal_Bcal_clr") << G4endl
         << "    dAbsorber = " << dbtmp->fetchDouble("dAbsorber") << G4endl
         << "    dSensor = " << dbtmp->fetchDouble("dSensor") << G4endl
         << "    dAirgap = " << dbtmp->fetchDouble("dAirgap")<< G4endl
         << "    dElboard = " << dbtmp->fetchDouble("dElboard")<< G4endl
         << "    dElectrMet = " << dbtmp->fetchDouble("dElectrMet")<< G4endl
         << "    nLayers = " << dbtmp->fetchDouble("nLayers") << G4endl
         << "    Segm = " << dbtmp->fetchDouble("Segm") << G4endl
         << "    dGraphite = " << dbtmp->fetchDouble("dGraphite") << G4endl
         << G4endl;

  return true;    
}

