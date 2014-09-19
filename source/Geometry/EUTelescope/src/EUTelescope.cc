// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: EUTelescope.cc,v 1.1 2008/10/24 11:45:20 tatsiana Exp $
// $Name: mokka-07-00 $
//

#include "EUTelescope.hh"

#include "CGAGeometryManager.hh"
#include "MySQLWrapper.hh" 
#include "TRKSiSD00.hh" 
#include "CGADefs.h" 

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4Transform3D.hh"
#include "G4PVPlacement.hh"

#ifdef MOKKA_GEAR
#include "gear/SiPlanesParameters.h" 
#include "gearimpl/SiPlanesParametersImpl.h" 
#include "gearimpl/SiPlanesLayerLayoutImpl.h" 
#include "MokkaGear.h"
#endif

INSTANTIATE(EUTelescope)
  
    G4bool EUTelescope::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog){
    
    G4VisAttributes *sensitiveVisAttributes = new G4VisAttributes(G4Colour(0.8, 0.8, 0.0)); 
    G4VisAttributes *insensitiveVisAttributes = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5));
    
#ifdef MOKKA_GEAR
    //------------------------------------------------------
    // some variables for storing information for MOKKA_GEAR
    // during the loop
    //------------------------------------------------------
    std::vector<helpLayer> gearHelpLayers ;
    std::vector<helpSensitive> gearHelpSensitives ;
    helpLayer gearHelpDUTLayer ;
    helpSensitive gearHelpDUTSensitive ;
    G4int gearHelpNumberPlanes ;
    G4int gearHelpCount = 0 ;
    G4int gearHelpType = 0 ;
    G4int gearHelpSetupID = 0 ;
    
#endif
    
    Database *db = new Database(env.GetDBName());
    db->exec("SELECT * FROM `common parameters`;");
    db->getTuple();
    const G4int setupID   = db->fetchInt("id");    
    const G4int dut_tag   = db->fetchInt("DUT_presence"); 
    const G4int nb_planes = db->fetchInt("numberTelPlanes");
    const G4double telPlaneThickness     = db->fetchDouble("telPlaneThickness") * mm; 
    const G4double dutPlaneThickness     = db->fetchDouble("DUTPlaneThickness") * mm; 
    
    //-------------------------------------------
    // The Telescope Sensitive detector
    // Threshold is 20% of a MIP. For Si we have
    // 340 KeV/mm as MIP.
    //-------------------------------------------
    
    TRKSiSD00 *tel_sensitiveDetector =
	new TRKSiSD00("telEUTelescope",
		    telPlaneThickness * mm
		    * 340 * keV
		    * 0.2);
    
    TRKSiSD00 *dut_sensitiveDetector =
	new TRKSiSD00("dutEUTelescope",
		    dutPlaneThickness * mm
		    * 340 * keV
		    * 0.2);
    
    RegisterSensitiveDetector(tel_sensitiveDetector); 
    RegisterSensitiveDetector(dut_sensitiveDetector); 
    
#ifdef MOKKA_GEAR
    // setup ID
    gearHelpSetupID = setupID;
    // type of telescope setup: with or without DUT 
    if( dut_tag == 0 ) gearHelpType = gear::SiPlanesParametersImpl::TelescopeWithoutDUT  ;
    if( dut_tag == 1 ) gearHelpType = gear::SiPlanesParametersImpl::TelescopeWithDUT ;
     // number of Si planes
    gearHelpNumberPlanes = nb_planes;
#endif
    
//--------------------
// DUT plane sensitive
//--------------------
    
    
    if( dut_tag == 1 ){
	
	db->exec("SELECT * FROM `dut sensitive`;");
	db->getTuple();
	const G4int    dut_ID        = db->fetchInt("dut_id");    
	const G4double dut_sizeX     = db->fetchDouble("dut_sizeX") * mm; 
	const G4double dut_sizeY     = db->fetchDouble("dut_sizeY") * mm; 
	const G4double dut_sizeZ     = db->fetchDouble("dut_sizeZ") * mm;
	const G4double dut_positionX = db->fetchDouble("dut_positionX") * mm;
	const G4double dut_positionY = db->fetchDouble("dut_positionY") * mm;
	const G4double dut_positionZ = db->fetchDouble("dut_positionZ") * mm;
	const G4double dut_shiftX    = db->fetchDouble("dut_shiftX") * mm;
	const G4double dut_shiftY    = db->fetchDouble("dut_shiftY") * mm;
	const G4double dut_shiftZ    = db->fetchDouble("dut_shiftZ") * mm;
	const G4double dut_tiltX     = db->fetchDouble("dut_tiltX") * deg;
	const G4double dut_tiltY     = db->fetchDouble("dut_tiltY") * deg;
	const G4double dut_tiltZ     = db->fetchDouble("dut_tiltZ") * deg;    
	const G4int dut_npixelX      = db->fetchInt("dut_npixelX");  
	const G4int dut_npixelY      = db->fetchInt("dut_npixelY");  
	const G4double dut_pitchX    = db->fetchDouble("dut_pitchX") * um;  
	const G4double dut_pitchY    = db->fetchDouble("dut_pitchY") * um;  
	const G4double dut_resolution= db->fetchDouble("dut_resolution") * um;  
	const G4double dut_rotation1 = db->fetchDouble("dut_rotation1");  
	const G4double dut_rotation2 = db->fetchDouble("dut_rotation2");  
	const G4double dut_rotation3 = db->fetchDouble("dut_rotation3");  
	const G4double dut_rotation4 = db->fetchDouble("dut_rotation4");  
	const G4String dut_sensitive = db->fetchString("dut_sensitive");
	const G4String dut_name      = db->fetchString("dut_name");
	G4Material *dut_material     = CGAGeometryManager::GetMaterial(db->fetchString("dut_material"));
	
	G4Box *dutsensBox = new G4Box(dut_name, dut_sizeX / 2, dut_sizeY / 2, dut_sizeZ / 2);
	G4LogicalVolume *dutsensLog = new G4LogicalVolume(dutsensBox, dut_material, dut_name, 0, 0, 0);
	
	if (dut_sensitive == "true") {
	    dutsensLog->SetSensitiveDetector(dut_sensitiveDetector);
	    dutsensLog->SetVisAttributes(sensitiveVisAttributes);
	} else {
	    dutsensLog->SetVisAttributes(insensitiveVisAttributes);
	}
	
	const G4RotationMatrix dutsensRotation = G4RotationMatrix().rotateX(dut_tiltX).rotateY(dut_tiltY).rotateZ(dut_tiltZ);
	const G4ThreeVector dutsensTranslation = G4ThreeVector(dut_positionX + dut_shiftX, dut_positionY + dut_shiftY, dut_positionZ + dut_shiftZ);
	new G4PVPlacement(G4Transform3D(dutsensRotation, dutsensTranslation), dutsensLog,dut_name,worldLog, false,dut_ID);
	
#ifdef MOKKA_GEAR
        //--------------------
        // DUT sensitive layer
        //--------------------
	helpSensitive dutSens ;
	dutSens.ID         =  dut_ID;
	dutSens.positionX  =  dut_positionX;
	dutSens.positionY  =  dut_positionY;
	dutSens.positionZ  =  dut_positionZ;
	dutSens.sizeX      =  dut_sizeX;
	dutSens.sizeY      =  dut_sizeY;
	dutSens.thickness  =  dut_sizeZ;
	dutSens.npixelX    =  dut_npixelX;
	dutSens.npixelY    =  dut_npixelY;
	dutSens.pitchX     =  dut_pitchX;
	dutSens.pitchY     =  dut_pitchY;
	dutSens.resolution =  dut_resolution;
	dutSens.rotation1  =  dut_rotation1;
	dutSens.rotation2  =  dut_rotation2;
	dutSens.rotation3  =  dut_rotation3;
	dutSens.rotation4  =  dut_rotation4;
	dutSens.radLength  =  (dutsensLog->GetMaterial())->GetRadlen()/mm ;
	// save information for gear
	gearHelpDUTSensitive = dutSens;
#endif
	
//-----------------------
// DUT nonsensitive layer
//-----------------------
	
	db->exec("SELECT * FROM `dut nonsensitive`;");
	db->getTuple();
	const G4int    dut_nonsens_ID        = db->fetchInt("dut_id");    
	const G4double dut_nonsens_sizeX     = db->fetchDouble("dut_sizeX") * mm;
	const G4double dut_nonsens_sizeY     = db->fetchDouble("dut_sizeY") * mm; 
	const G4double dut_nonsens_sizeZ     = db->fetchDouble("dut_sizeZ") * mm;
	const G4double dut_nonsens_positionX = db->fetchDouble("dut_positionX") * mm;
	const G4double dut_nonsens_positionY = db->fetchDouble("dut_positionY") * mm;
	const G4double dut_nonsens_positionZ = db->fetchDouble("dut_positionZ") * mm;
	const G4double dut_nonsens_shiftX    = db->fetchDouble("dut_shiftX") * mm;
	const G4double dut_nonsens_shiftY    = db->fetchDouble("dut_shiftY") * mm;
	const G4double dut_nonsens_shiftZ    = db->fetchDouble("dut_shiftZ") * mm;
	const G4double dut_nonsens_tiltX     = db->fetchDouble("dut_tiltX") * deg;
	const G4double dut_nonsens_tiltY     = db->fetchDouble("dut_tiltY") * deg;
	const G4double dut_nonsens_tiltZ     = db->fetchDouble("dut_tiltZ") * deg;    
	const G4String dut_nonsens_sensitive = db->fetchString("dut_sensitive");
	const G4String dut_nonsens_name      = db->fetchString("dut_name");
	G4Material *dut_nonsens_material     = CGAGeometryManager::GetMaterial(db->fetchString("dut_material"));
	
	G4Box *dut_nonsens_Box = new G4Box(dut_nonsens_name, dut_nonsens_sizeX / 2, dut_nonsens_sizeY / 2, dut_nonsens_sizeZ / 2);
	G4LogicalVolume *dut_nonsens_Log = new G4LogicalVolume(dut_nonsens_Box, dut_nonsens_material, dut_nonsens_name, 0, 0, 0);
	
	if (dut_nonsens_sensitive == "true") {
	    dut_nonsens_Log->SetSensitiveDetector(dut_sensitiveDetector);
	    dut_nonsens_Log->SetVisAttributes(sensitiveVisAttributes);
	} else {
	    dut_nonsens_Log->SetVisAttributes(insensitiveVisAttributes);
	}
	
	const G4RotationMatrix dut_nonsens_Rotation = G4RotationMatrix().rotateX(dut_nonsens_tiltX).rotateY(dut_nonsens_tiltY).rotateZ(dut_nonsens_tiltZ);
	const G4ThreeVector dut_nonsens_Translation = G4ThreeVector(dut_nonsens_positionX + dut_nonsens_shiftX, dut_nonsens_positionY + dut_nonsens_shiftY, dut_nonsens_positionZ + dut_nonsens_shiftZ);
	new G4PVPlacement(G4Transform3D(dut_nonsens_Rotation, dut_nonsens_Translation), dut_nonsens_Log,dut_nonsens_name,worldLog, false,dut_nonsens_ID);
	
#ifdef MOKKA_GEAR
	//------------------------
	// DUT nonsensitive layer
	//------------------------
	helpLayer dutLayer ;
      	dutLayer.ID         =  dut_nonsens_ID;
	dutLayer.positionX  =  dut_nonsens_positionX;
	dutLayer.positionY  =  dut_nonsens_positionY;
	dutLayer.positionZ  =  dut_nonsens_positionZ;
	dutLayer.sizeX      =  dut_nonsens_sizeX;
	dutLayer.sizeY      =  dut_nonsens_sizeY;
	dutLayer.thickness  =  dut_nonsens_sizeZ;
	dutLayer.radLength  =  (dut_nonsens_Log->GetMaterial())->GetRadlen()/mm ;
	
	// save information for gear
	gearHelpDUTLayer =  dutLayer;
#endif
	
    }
    
//----------------------------
// Telescope sensitive layers
//----------------------------
    
    db->exec("SELECT * FROM `layers sensitive`;");
    
    while (db->getTuple()) {
	const G4int    layer     = db->fetchInt("layer");
	const G4double sizeX     = db->fetchDouble("sizeX") * mm; 
	const G4double sizeY     = db->fetchDouble("sizeY") * mm; 
	const G4double sizeZ     = db->fetchDouble("sizeZ") * mm;
	const G4double positionX = db->fetchDouble("positionX") * mm;
	const G4double positionY = db->fetchDouble("positionY") * mm;
	const G4double positionZ = db->fetchDouble("positionZ") * mm;
	const G4double shiftX    = db->fetchDouble("shiftX") * mm;
	const G4double shiftY    = db->fetchDouble("shiftY") * mm;
	const G4double shiftZ    = db->fetchDouble("shiftZ") * mm;
	const G4double tiltX     = db->fetchDouble("tiltX") * deg;
	const G4double tiltY     = db->fetchDouble("tiltY") * deg;
	const G4double tiltZ     = db->fetchDouble("tiltZ") * deg;    
	const G4int npixelX      = db->fetchInt("npixelX");  
	const G4int npixelY      = db->fetchInt("npixelY");  
	const G4double pitchX    = db->fetchDouble("pitchX") * um;  
	const G4double pitchY    = db->fetchDouble("pitchY") * um;  
	const G4double resolution= db->fetchDouble("resolution") * um;  
	const G4double rotation1 = db->fetchDouble("rotation1");  
	const G4double rotation2 = db->fetchDouble("rotation2");  
	const G4double rotation3 = db->fetchDouble("rotation3");  
	const G4double rotation4 = db->fetchDouble("rotation4");
	const G4String sensitive = db->fetchString("sensitive"); 
	const G4String name      = db->fetchString("name"); 
	G4Material *material     = CGAGeometryManager::GetMaterial(db->fetchString("material"));  
	
	G4Box *boxsens = new G4Box(name, sizeX / 2, sizeY / 2, sizeZ / 2); 
	G4LogicalVolume *logsens = new G4LogicalVolume(boxsens, material, name, 0, 0, 0); 
	
	if (sensitive == "true") { 
	    logsens->SetSensitiveDetector(tel_sensitiveDetector);
	    logsens->SetVisAttributes(sensitiveVisAttributes);
	} else {
	    logsens->SetVisAttributes(insensitiveVisAttributes);
	}
	
	const G4RotationMatrix rotationsens = G4RotationMatrix().rotateX(tiltX).rotateY(tiltY).rotateZ(tiltZ);
	const G4ThreeVector translationsens = G4ThreeVector(positionX + shiftX, positionY + shiftY, positionZ + shiftZ);
	new G4PVPlacement(G4Transform3D(rotationsens, translationsens), logsens, name, worldLog, false, layer);
	
#ifdef MOKKA_GEAR
	//---------------------------
	// Telescope sensitive layers
	//---------------------------
	helpSensitive thisSens ;
	thisSens.ID         =  layer;
	thisSens.positionX  =  positionX;
	thisSens.positionY  =  positionY;
	thisSens.positionZ  =  positionZ;
	thisSens.sizeX      =  sizeX;
	thisSens.sizeY      =  sizeY;
	thisSens.thickness  =  sizeZ;
	thisSens.npixelX    =  npixelX;
	thisSens.npixelY    =  npixelY;
	thisSens.pitchX     =  pitchX;
	thisSens.pitchY     =  pitchY;
	thisSens.resolution =  resolution;
	thisSens.rotation1  =  rotation1;
	thisSens.rotation2  =  rotation2;
	thisSens.rotation3  =  rotation3;
	thisSens.rotation4  =  rotation4;
	thisSens.radLength  = (logsens->GetMaterial())->GetRadlen()/mm ;
	
	// save information for gear
	gearHelpSensitives.push_back( thisSens ) ;
	gearHelpCount ++ ;
#endif
    } 
    
//------------------------------
// Telescope nonsensitive layers
//------------------------------
    
    db->exec("SELECT * FROM `layers nonsensitive`;");
    
    while (db->getTuple()) {
	const G4int    layer     = db->fetchInt("layer");
	const G4double sizeX     = db->fetchDouble("sizeX") * mm;
	const G4double sizeY     = db->fetchDouble("sizeY") * mm;
	const G4double sizeZ     = db->fetchDouble("sizeZ") * mm;
	const G4double positionX = db->fetchDouble("positionX") * mm;
	const G4double positionY = db->fetchDouble("positionY") * mm;
	const G4double positionZ = db->fetchDouble("positionZ") * mm;
	const G4double shiftX    = db->fetchDouble("shiftX") * mm;
	const G4double shiftY    = db->fetchDouble("shiftY") * mm;
	const G4double shiftZ    = db->fetchDouble("shiftZ") * mm;
	const G4double tiltX     = db->fetchDouble("tiltX") * deg;
	const G4double tiltY     = db->fetchDouble("tiltY") * deg;
	const G4double tiltZ     = db->fetchDouble("tiltZ") * deg;    
	const G4String sensitive = db->fetchString("sensitive");
	const G4String name      = db->fetchString("name"); 
	G4Material *material     = CGAGeometryManager::GetMaterial(db->fetchString("material"));  
	
	G4Box *box = new G4Box(name, sizeX / 2, sizeY / 2, sizeZ / 2); 
	G4LogicalVolume *log = new G4LogicalVolume(box, material, name, 0, 0, 0); 
	
	if (sensitive == "true") { 
	    log->SetSensitiveDetector(tel_sensitiveDetector);
	    log->SetVisAttributes(sensitiveVisAttributes);
	} else {
	    log->SetVisAttributes(insensitiveVisAttributes);
	}
	
	const G4RotationMatrix rotation = G4RotationMatrix().rotateX(tiltX).rotateY(tiltY).rotateZ(tiltZ);
	const G4ThreeVector translation = G4ThreeVector(positionX + shiftX, positionY + shiftY, positionZ + shiftZ);
	new G4PVPlacement(G4Transform3D(rotation, translation), log, name, worldLog, false, layer);
	
#ifdef MOKKA_GEAR
	//------------------------------
	// Telescope nonsensitive layers
	//------------------------------
	helpLayer thisLayer ;
	thisLayer.ID         =  layer;
	thisLayer.positionX  =  positionX;
	thisLayer.positionY  =  positionY;
	thisLayer.positionZ  =  positionZ;
	thisLayer.sizeX      =  sizeX;
	thisLayer.sizeY      =  sizeY;
	thisLayer.thickness  =  sizeZ;
	thisLayer.radLength  = (log->GetMaterial())->GetRadlen()/mm ;
	
	// save information for gear
	gearHelpLayers.push_back( thisLayer );
#endif
    } 
    
//---------------
// Other material
//---------------
    
    db->exec("SELECT * FROM `other material`;");
    
    while (db->getTuple()) {
	const G4int    layer     = db->fetchInt("layer");
	const G4double sizeX     = db->fetchDouble("sizeX") * mm;
	const G4double sizeY     = db->fetchDouble("sizeY") * mm;
	const G4double sizeZ     = db->fetchDouble("sizeZ") * mm;
	const G4double positionX = db->fetchDouble("positionX") * mm;
	const G4double positionY = db->fetchDouble("positionY") * mm;
	const G4double positionZ = db->fetchDouble("positionZ") * mm;
	const G4double shiftX    = db->fetchDouble("shiftX") * mm;
	const G4double shiftY    = db->fetchDouble("shiftY") * mm;
	const G4double shiftZ    = db->fetchDouble("shiftZ") * mm;
	const G4double tiltX     = db->fetchDouble("tiltX") * deg;
	const G4double tiltY     = db->fetchDouble("tiltY") * deg;
	const G4double tiltZ     = db->fetchDouble("tiltZ") * deg;    
	const G4String sensitive = db->fetchString("sensitive"); 
	const G4String name      = db->fetchString("name"); 
	G4Material *material     = CGAGeometryManager::GetMaterial(db->fetchString("material"));  
	
	G4Box *box = new G4Box(name, sizeX / 2, sizeY / 2, sizeZ / 2); 
	G4LogicalVolume *log = new G4LogicalVolume(box, material, name, 0, 0, 0);
	
	if (sensitive == "true") { 
	    log->SetSensitiveDetector(tel_sensitiveDetector);
	    log->SetVisAttributes(sensitiveVisAttributes);
	} else {
	    log->SetVisAttributes(insensitiveVisAttributes);
	}
	
	const G4RotationMatrix rotation = G4RotationMatrix().rotateX(tiltX).rotateY(tiltY).rotateZ(tiltZ);
	const G4ThreeVector translation = G4ThreeVector(positionX + shiftX, positionY + shiftY, positionZ + shiftZ);
	new G4PVPlacement(G4Transform3D(rotation, translation), log, name, worldLog, false, layer);
	
#ifdef MOKKA_GEAR
	helpLayer thisLayer ;
	thisLayer.ID         =  layer;
	thisLayer.positionX  =  positionX;
	thisLayer.positionY  =  positionY;
	thisLayer.positionZ  =  positionZ;
	thisLayer.sizeX      =  sizeX;
	thisLayer.sizeY      =  sizeY;
	thisLayer.thickness  =  sizeZ;
	thisLayer.radLength  = (log->GetMaterial())->GetRadlen()/mm ;
	
	helpSensitive thisSens ;
	thisSens.ID          =  0;
	thisSens.positionX   =  0.;
	thisSens.positionY   =  0.;
	thisSens.positionZ   =  0.;
	thisSens.sizeX       =  0.;
	thisSens.sizeY       =  0.;
	thisSens.thickness   =  0.;
	thisSens.npixelX     =  0;
	thisSens.npixelY     =  0;
	thisSens.pitchX      =  0.;
	thisSens.pitchY      =  0.;
	thisSens.resolution  =  0.;
	thisSens.rotation1   =  0.;
	thisSens.rotation2   =  0.;
	thisSens.rotation3   =  0.;
	thisSens.rotation4   =  0.;   
	thisSens.radLength   =  0.;
	
	// save information for gear
	gearHelpSensitives.push_back( thisSens ) ;
	gearHelpLayers.push_back( thisLayer );
	gearHelpCount ++ ;
#endif
    } 
    
    
#ifdef MOKKA_GEAR
    // write data to gear
    
    // get gear manager
    MokkaGear* gearMgr = MokkaGear::getMgr() ;
    
    // construct SiPlanesParameters
    gear::SiPlanesParametersImpl* siplanesParams = 
	new gear::SiPlanesParametersImpl(gearHelpSetupID, gearHelpType, gearHelpNumberPlanes);
    
    
    // add all layers
    for( int i = 0 ; i < gearHelpCount ; i++ ) {
	siplanesParams->addLayer(gearHelpLayers[i].ID ,
				 gearHelpLayers[i].positionX ,
				 gearHelpLayers[i].positionY ,
				 gearHelpLayers[i].positionZ ,
				 gearHelpLayers[i].sizeX ,
				 gearHelpLayers[i].sizeY ,
				 gearHelpLayers[i].thickness ,
				 gearHelpLayers[i].radLength ,
				 gearHelpSensitives[i].ID ,
				 gearHelpSensitives[i].positionX ,
				 gearHelpSensitives[i].positionY ,
				 gearHelpSensitives[i].positionZ ,
				 gearHelpSensitives[i].sizeX ,
				 gearHelpSensitives[i].sizeY ,
				 gearHelpSensitives[i].thickness ,
				 gearHelpSensitives[i].npixelX ,
				 gearHelpSensitives[i].npixelY ,
				 gearHelpSensitives[i].pitchX ,
				 gearHelpSensitives[i].pitchY ,
				 gearHelpSensitives[i].resolution ,
				 gearHelpSensitives[i].rotation1 ,
				 gearHelpSensitives[i].rotation2 ,
				 gearHelpSensitives[i].rotation3 ,
				 gearHelpSensitives[i].rotation4 ,
				 gearHelpSensitives[i].radLength ) ;
    }
    
    if( dut_tag == 1 ){
	
	// add DUT layer
	siplanesParams->addDUT(gearHelpDUTLayer.ID ,
			       gearHelpDUTLayer.positionX ,
			       gearHelpDUTLayer.positionY ,
			       gearHelpDUTLayer.positionZ ,
			       gearHelpDUTLayer.sizeX ,
			       gearHelpDUTLayer.sizeY ,
			       gearHelpDUTLayer.thickness ,
			       gearHelpDUTLayer.radLength , 
			       gearHelpDUTSensitive.ID ,
			       gearHelpDUTSensitive.positionX ,
			       gearHelpDUTSensitive.positionY ,
			       gearHelpDUTSensitive.positionZ ,
			       gearHelpDUTSensitive.sizeX ,
			       gearHelpDUTSensitive.sizeY ,
			       gearHelpDUTSensitive.thickness ,
			       gearHelpDUTSensitive.npixelX ,
			       gearHelpDUTSensitive.npixelY ,
			       gearHelpDUTSensitive.pitchX ,
			       gearHelpDUTSensitive.pitchY ,
			       gearHelpDUTSensitive.resolution ,
			       gearHelpDUTSensitive.rotation1 ,
			       gearHelpDUTSensitive.rotation2 ,
			       gearHelpDUTSensitive.rotation3 ,
			       gearHelpDUTSensitive.rotation4 ,
			       gearHelpDUTSensitive.radLength ) ;
    }
    
    gearMgr->setSiPlanesParameters( siplanesParams ) ;
    
#endif
    
    delete db;
    return true;
}

