// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: TPC10.cc,v 1.1 2009/05/13 08:22:57 steve Exp $
// $Name: mokka-07-00 $
//
// History:  
// - modified version of TPC driver by Ties Behnke
// - modified version of TPC02 as TPC03 with selectable chamber gas -- Adrian Vogel, 2005-06-09
// - modified version of TPC03 as TPC04 with limit of step length   -- Adrian Vogel, 2006-02-01
// - introduced self-scalability, no superdriver needed anymore     -- Adrian Vogel, 2006-03-11
// - modified version of TPC04 as TPC05 in order to have full MC
//   information both at entry and exit hits in the TPC ,
//   more realistic central electrode and endplate             -- Predrag Krstonosic, 2006-07-12
// - implemented new GEAR interface -- K. Harder, T. Pinto Jayawardena                2007-07-31
// - TPC10 implemented readout within the Gas volume and layered inner and outer wall -- SJA -- 2010-11-19

            
#include "TPC10.hh"

#include "TPCSD04.hh"

#include "CGAGeometryManager.hh"
#include "MySQLWrapper.hh"
#include "Control.hh"
#include "CGADefs.h"

#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4PVPlacement.hh"
#include "G4UserLimits.hh"
#include "G4SubtractionSolid.hh"
#include "G4ThreeVector.hh"

#ifdef MOKKA_GEAR
#include "MokkaGear.h"
#include "gear/TPCParameters.h"
#include "gearimpl/TPCParametersImpl.h"
#include "gearimpl/FixedPadSizeDiskLayout.h"
#include "G4EmCalculator.hh"
#include "G4ParticleTable.hh"



#endif



#include <iomanip>
#include <vector>
INSTANTIATE(TPC10)

G4bool TPC10::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog)
{
  const G4double phi1 =   0.0 * deg;
  const G4double phi2 = 360.0 * deg;

  const G4double dzTotal           = 2.0 * env.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");
  const G4double rInner            = env.GetParameterAsDouble("TPC_inner_radius") * mm;
  const G4double rOuter            = env.GetParameterAsDouble("TPC_outer_radius") * mm;
  const G4double padHeight         = env.GetParameterAsDouble("TPC_pad_height") * mm;
  const G4double padWidth          = env.GetParameterAsDouble("TPC_pad_width") * mm;
  const G4double TPCMaxStepLength  = env.GetParameterAsDouble("TPC_max_step_length") * mm;


  // Geometry parameters from the geometry environment and from the database

  Database *db = new Database(env.GetDBName());

  db->exec("SELECT * FROM `global`;");
  db->getTuple();

  const G4double dr_InnerWall        = db->fetchDouble("dr_InnerWall") * mm;
  const G4double dr_InnerServiceArea = db->fetchDouble("dr_InnerServiceArea") * mm;
  const G4double dr_OuterServiceArea = db->fetchDouble("dr_OuterServiceArea") * mm;
  const G4double dr_OuterWall        = db->fetchDouble("dr_OuterWall") * mm;
  const G4double dz_Readout          = db->fetchDouble("dz_Readout") * mm;
  const G4double dz_Endplate         = db->fetchDouble("dz_Endplate") * mm;
  G4Material* const material_TPC_Gas = CGAGeometryManager::GetMaterial(db->fetchString("chamber_Gas"));
#ifdef MOKKA_GEAR
  _gear_gas_material = material_TPC_Gas; 
#endif

  const G4double sensitive_threshold_eV  = db->fetchDouble("sensitive_threshold_eV") * eV;


  db->exec("SELECT * FROM `cathode`;");
  db->getTuple();

  const G4double dz_Cathode_Insulator          = db->fetchDouble("dz_Cathode_Insulator") * mm;
  const G4double dz_Cathode_Conductor          = db->fetchDouble("dz_Cathode_Conductor") * mm;
  G4Material* const material_Cathode_Insulator = CGAGeometryManager::GetMaterial(db->fetchString("material_Cathode_Insulator"));
  G4Material* const material_Cathode_Conductor = CGAGeometryManager::GetMaterial(db->fetchString("material_Cathode_Conductor"));

  const G4double dr_Cathode_Grip               = db->fetchDouble("dr_Cathode_Grip") * mm;
  const G4double dz_Cathode_Grip               = db->fetchDouble("dz_Cathode_Grip") * mm;
  G4Material* const material_Cathode_Grip      = CGAGeometryManager::GetMaterial(db->fetchString("material_Cathode_Grip"));

  G4cout << " Cathode Grip Ring Material: " << material_Cathode_Grip->GetName() << " : Rad length = " << material_Cathode_Grip->GetRadlen() / mm << " mm." << std::endl;

  const G4double dz_Cathode = 2*(dz_Cathode_Insulator+dz_Cathode_Conductor);
  
  G4double tracking_tpc_ecal_gap = env.GetParameterAsDouble("Ecal_Tpc_gap") * mm;;
  G4double tracking_region_rmax = rOuter + tracking_tpc_ecal_gap - 0.1*mm; // give 100 micron clearance 

  std::stringstream tracking_region_rmax_as_string;
  tracking_region_rmax_as_string <<  tracking_region_rmax;

  (*Control::globalModelParameters)["tracker_region_rmax"] = tracking_region_rmax_as_string.str();
  (*Control::globalModelParameters)["tracker_region_zmax"] = env.GetParameterAsString("TPC_Ecal_Hcal_barrel_halfZ");


  // Calculate Dimentions needed for later. Note gas volume and endplate will be mirror placed ...
  const G4double rMin_GasVolume      = rInner + dr_InnerWall;
  const G4double rMax_GasVolume      = rOuter - dr_OuterWall;

// note the will be two gas volumes one in each z-half. The cathode and readout are considered to be placed inside the Gas volume
  const G4double dz_GasVolume        = ( dzTotal/2.0 ) - dz_Endplate; 

  const G4double rMin_Sensitive      = rMin_GasVolume + dr_InnerServiceArea;
  const G4double rMax_Sensitive      = rMax_GasVolume - dr_OuterServiceArea;
  const G4double dz_Sensitive        = dz_GasVolume - ( dz_Cathode/2.0 + dz_Readout ); // the d_Cathode spans both halfs of the TPC 
  const G4double dz_Wall        = dzTotal - 2.0 * dz_Endplate ; // note field cage spans the complete length of the TPC Gas volume


  const G4int numberPadRows = (int)((rMax_Sensitive-rMin_Sensitive)/padHeight) ;


  // Materials to be used
  G4Material * const materialAir     = CGAGeometryManager::GetMaterial("air");

  // Material mixture for end of endplate zone 
  db->exec("SELECT * FROM `endplate_mixture`;");

  G4double endplate_mixture_total = 0.0;
  G4double densityTotal = 0.0 ;

  std::map<G4Material* const, G4double> material_fractions;
  while (db->getTuple()) 
    {
      G4Material* const material = CGAGeometryManager::GetMaterial(db->fetchString("material"));
      material_fractions[material]    = db->fetchDouble("percentage") * perCent; // fraction of material mix
      endplate_mixture_total         += material_fractions[material];
      densityTotal                   += material->GetDensity() * material_fractions[material]; 
    }
  
  if (fabs( endplate_mixture_total - 1) > 1E-06 || (fabs( 1 - endplate_mixture_total) > 1E-06 )) 
    {
      G4cout << "endplate_mixture_total = " << endplate_mixture_total << G4endl;
      Control::Abort("TPC endplate material fractions do not add up to 100%",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
    }
  
  G4Material *endplate_MaterialMix = new G4Material("TPC_endplate_mix", densityTotal, material_fractions.size());
 
  for(  std::map<G4Material* const, G4double>::iterator it=material_fractions.begin(); it!=material_fractions.end();++it )
    {
      endplate_MaterialMix->AddMaterial(it->first, it->second);
    }

  G4cout << "Endplate material mix Density   = " << endplate_MaterialMix->GetDensity() / g * cm3 << " g/cm3" << G4endl;
  G4cout << "Endplate material mix Radlength = " << endplate_MaterialMix->GetRadlen() / mm << " mm" << G4endl;


  // Visualisation attributes

  G4VisAttributes *wallVisAttributes = new G4VisAttributes(G4Colour(0.0, 0.5, 0.5)); // dull cyan
  wallVisAttributes->SetForceWireframe(false);
  wallVisAttributes->SetDaughtersInvisible(true);

  G4VisAttributes *cathodeVisAttributes = new G4VisAttributes(G4Colour(0.9, 0.3, 0.1)); // coppery brown
  cathodeVisAttributes->SetForceWireframe(false);


  // Some verbose output
  G4cout << "TPC10: Inner radius of the gas volume is " << std::setw(4) << rMin_GasVolume / mm << " mm." << G4endl;
  G4cout << "TPC10: Outer radius of the gas volume is " << std::setw(4) << rMax_GasVolume / mm << " mm." << G4endl;
  G4cout << "TPC10: Inner wall thickness is " << std::setw(4) << dr_InnerWall / mm << " mm." << G4endl;
  G4cout << "TPC10: Outer wall thickness is " << std::setw(4) << dr_OuterWall / mm << " mm." << G4endl;
  G4cout << "TPC10: Outer wall thickness is " << std::setw(4) << dr_OuterWall / mm << " mm." << G4endl;

  G4cout << "TPC10: Inner radius of the sensitive volume is " << std::setw(4) << rMin_Sensitive / mm << " mm." << G4endl;
  G4cout << "TPC10: Outer radius of the sensitive volume is " << std::setw(4) << rMax_Sensitive / mm << " mm." << G4endl;
  G4cout << "TPC10: Number of Pad Rows in the TPC  " << std::setw(4) << numberPadRows << G4endl;
  G4cout << "TPC10: Limiting the step length in the TPC to  " << std::setw(4) << TPCMaxStepLength / mm << " mm." << G4endl;

  //-------------------------------------------------------------------------------------------------------//

  //-------------------------------- TPC mother volume ----------------------------------------------------//
  //------------ Volume for the whole TPC, Field Cage, Cathode, and Endplate and Sensitive ----------------//

  G4Tubs *motherSolid = new G4Tubs("TPCSolid" , 
				   rInner , 
				   rOuter , 
				   dzTotal/2.0 , 
				   phi1 , 
				   phi2) ;

  G4LogicalVolume *motherLog = new G4LogicalVolume(motherSolid, material_TPC_Gas, "TPCLog", 0, 0, 0);
  //  motherLog->SetVisAttributes(G4VisAttributes::Invisible);
  new G4PVPlacement(0, G4ThreeVector(), motherLog, "TPC", worldLog, false, 0);

  G4VisAttributes* motherVisAttributes = new G4VisAttributes(G4Colour(0.0, 0.5, 0.5)); // dull cyan
  motherVisAttributes->SetVisibility(false);
  motherVisAttributes->SetDaughtersInvisible(true);
  motherLog->SetVisAttributes(motherVisAttributes);
  
  
  G4cout << "TPC10: Total Gas material corresponds to " << ( ( (rOuter-dr_OuterWall) - (rInner + dr_InnerWall) ) / (material_TPC_Gas->GetRadlen() / mm ) * 100.0 ) << "% of a radiation length." << G4endl;

  //-------------------------------------------------------------------------------------------------------//


  //-------------------------------- inner wall construction ----------------------------------------//

  G4Tubs *innerWallSolid = new G4Tubs("TPCInnerWallSolid" ,
					   rInner ,
					   rInner + dr_InnerWall ,
					   dz_Wall / 2.0 ,
					   phi1 , 
					   phi2) ;

  G4LogicalVolume *innerWallLog = new G4LogicalVolume( innerWallSolid, materialAir, "TPCInnerWallLog", 0, 0, 0 ) ; 
  innerWallLog->SetVisAttributes(wallVisAttributes);
  new G4PVPlacement( 0, G4ThreeVector(), innerWallLog, "TPCInnerWall", motherLog, false, 0 );

  G4int layerCounter = 0;
  G4double fracRadLengthInnerWall = 0;
  G4double rCursor = rInner ;
  gear_inner_wall_material_total_density = 0.0;

  db->exec("SELECT * FROM `innerWall`;");
  while (db->getTuple()) {
    const G4double dr = db->fetchDouble("dr") * mm;
    G4Material *layerMaterial = CGAGeometryManager::GetMaterial(db->fetchString("material"));
    G4Tubs *layerSolid = new G4Tubs("TPCInnerWallLayerSolid", rCursor, rCursor + dr , dz_Wall / 2.0, phi1, phi2);
    G4LogicalVolume *layerLog = new G4LogicalVolume(layerSolid, layerMaterial, "TPCInnerWallLayerLog", 0, 0, 0);
    layerLog->SetVisAttributes(G4VisAttributes::Invisible);
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), layerLog, "TPCInnerWallLayer", innerWallLog, false, layerCounter);
    ++layerCounter;
    rCursor += dr ;
    fracRadLengthInnerWall += dr / layerMaterial->GetRadlen();

    G4cout << "TPC10: Add Material to Inner Wall: dr =  " << std::setw(4) << dr / mm << " mm. Material = " << layerMaterial->GetName() << " X0 = " << layerMaterial->GetRadlen() << "  " <<  dr / layerMaterial->GetRadlen() << "% X0" << G4endl;

#ifdef MOKKA_GEAR

    gear_inner_wall_material_total_density += layerMaterial->GetDensity() * (dr / dr_InnerWall);    
    std::string material_name = db->fetchString("material");

    G4cout << "TPC10: gear_inner_wall_material_total_density = " << std::setw(4) << gear_inner_wall_material_total_density << G4endl;   

    if( gear_inner_wall_material_thicknesses.find( material_name ) ==  gear_inner_wall_material_thicknesses.end() ) {
      gear_inner_wall_material_thicknesses[ material_name ] = dr;
      }
    else {
      gear_inner_wall_material_thicknesses[ material_name ] += dr;
      }
      
#endif

  }

  G4cout << "TPC10: Inner wall material corresponds to " << G4int( fracRadLengthInnerWall * 1000) / 10.0 << "% of a radiation length." << G4endl;
  G4cout << "TPC10: Inner wall effective X0 = " << std::setw(4) << dr_InnerWall / fracRadLengthInnerWall<< G4endl;  


  //-------------------------------------------------------------------------------------------------------//


  //-------------------------------- outer wall construction ----------------------------------------//

  G4Tubs *outerWallSolid = new G4Tubs("TPCOuterWallSolid" ,
					   rOuter - dr_OuterWall ,
					   rOuter ,
					   dz_Wall / 2.0 ,
					   phi1 , 
					   phi2) ;

  G4LogicalVolume *outerWallLog = new G4LogicalVolume( outerWallSolid, materialAir, "TPCOuterWallLog", 0, 0, 0 ) ; 
  outerWallLog->SetVisAttributes(wallVisAttributes);
  new G4PVPlacement( 0, G4ThreeVector(), outerWallLog, "TPCOuterWall", motherLog, false, 0 );


  layerCounter = 0;
  G4double fracRadLengthOuterWall = 0;
  rCursor = rOuter - dr_OuterWall ;
  gear_outer_wall_material_total_density = 0.0;
 
  db->exec("SELECT * FROM `outerWall`;");
  while (db->getTuple()) {
    const G4double dr = db->fetchDouble("dr") * mm;
    G4Material *layerMaterial = CGAGeometryManager::GetMaterial(db->fetchString("material"));
    G4Tubs *layerSolid = new G4Tubs("TPCOuterWallLayerSolid", rCursor, rCursor + dr , dz_Wall / 2.0, phi1, phi2);
    G4LogicalVolume *layerLog = new G4LogicalVolume(layerSolid, layerMaterial, "TPCOuterWallLayerLog", 0, 0, 0);
    layerLog->SetVisAttributes(G4VisAttributes::Invisible);
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), layerLog, "TPCOuterWallLayer", outerWallLog, false, layerCounter);
    ++layerCounter;
    rCursor += dr ;
    fracRadLengthOuterWall += dr / layerMaterial->GetRadlen();

    G4cout << "TPC10: Add Material to Outer Wall: dr =  " << std::setw(4) << dr / mm << " mm. Material = " << layerMaterial->GetName() << " X0 = " << layerMaterial->GetRadlen() << "  " <<  dr / layerMaterial->GetRadlen() << "% X0" << G4endl;

#ifdef MOKKA_GEAR

    gear_outer_wall_material_total_density += layerMaterial->GetDensity() * (dr / dr_OuterWall);    
    std::string material_name = db->fetchString("material");

    G4cout << "TPC10: gear_outer_wall_material_total_density = " << std::setw(4) << gear_outer_wall_material_total_density << G4endl;   

    if( gear_outer_wall_material_thicknesses.find( material_name ) ==  gear_outer_wall_material_thicknesses.end() )
      {
	gear_outer_wall_material_thicknesses[ material_name ] = dr;
      }
    else
      {
      	gear_outer_wall_material_thicknesses[ material_name ] += dr;
      }

#endif  

}
  G4cout << "TPC10: Outer wall material corresponds to " << G4int( fracRadLengthOuterWall * 1000) / 10.0 << "% of a radiation length." << G4endl;
  G4cout << "TPC10: Outer wall effective X0 = " << std::setw(4) << dr_OuterWall / fracRadLengthOuterWall   << G4endl;  
  //-----------------------------------------------------------------------------------------------//  


  //-------------------------------- cathode grip ring construction ----------------------------------------//
  // inner grip ring
  G4Tubs *cathodeInnerGripSolid = new G4Tubs("TPCCathodeInnerGripSolid", rMin_GasVolume , rMin_GasVolume + dr_Cathode_Grip, dz_Cathode_Grip / 2.0 , phi1, phi2);
  G4LogicalVolume *cathodeInnerGripLog = new G4LogicalVolume(cathodeInnerGripSolid, material_Cathode_Grip, "TPCCathodeInnerGripLog", 0, 0, 0);
  cathodeInnerGripLog->SetVisAttributes(cathodeVisAttributes);
  new G4PVPlacement(0, G4ThreeVector(), cathodeInnerGripLog, "TPCCathodeInnerGrip", motherLog, false, 0);

  // outer grip ring
  G4Tubs *cathodeOuterGripSolid = new G4Tubs("TPCCathodeOuterGripSolid", rMax_GasVolume - dr_Cathode_Grip, rMax_GasVolume, dz_Cathode_Grip / 2.0 , phi1, phi2);
  G4LogicalVolume *cathodeOuterGripLog = new G4LogicalVolume(cathodeOuterGripSolid, material_Cathode_Grip, "TPCCathodeOuterGripLog", 0, 0, 0);
  cathodeOuterGripLog->SetVisAttributes(cathodeVisAttributes);
  new G4PVPlacement(0, G4ThreeVector(), cathodeOuterGripLog, "TPCCathodeOuterGrip", motherLog, false, 0);

  //-----------------------------------------------------------------------------------------------//  


  //-------------------------------- cathode construction ----------------------------------------//
  G4Tubs *cathodeSolid = new G4Tubs("TPCCathodeSolid", rMin_Sensitive, rMax_Sensitive, dz_Cathode / 2.0, phi1, phi2);
  G4LogicalVolume *cathodeLog = new G4LogicalVolume(cathodeSolid, materialAir, "TPCCathodeLog", 0, 0, 0);
  cathodeLog->SetVisAttributes(cathodeVisAttributes);
  new G4PVPlacement(0, G4ThreeVector(0.,0.,0.), cathodeLog, "TPCCathode", motherLog, false, 0);

  // insulator 
  G4Tubs *cathodeInsulatorSolid = new G4Tubs("TPCcathodeInsulatorSolid", rMin_Sensitive, rMax_Sensitive, (dz_Cathode_Insulator / 2.0)-0.00000001*mm, phi1, phi2);
  G4LogicalVolume *cathodeInsulatorLog = new G4LogicalVolume(cathodeInsulatorSolid, material_Cathode_Insulator, "TPCcathodeInsulatorLog", 0, 0, 0);
  
  // place plus and minus z 
  new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, dz_Cathode_Insulator / 2.0), cathodeInsulatorLog, "TPCCathodeInsulator_+z",cathodeLog, false, 0);
  new G4PVPlacement(0, G4ThreeVector(0.0, 0.0,-dz_Cathode_Insulator / 2.0), cathodeInsulatorLog, "TPCCathodeInsulator_-z",cathodeLog, false, 0);

  G4cout << "Cathode dz = " <<  dz_Cathode_Insulator << G4endl; 
  G4cout << "Place cathode +z at " <<  dz_Cathode_Insulator / 2.0 << G4endl; 
  G4cout << "Place cathode -z at " << -dz_Cathode_Insulator / 2.0 << G4endl;
  
  // conductor 
  G4Tubs *cathodeConductorSolid = new G4Tubs("TPCCathodeConductorSolid", rMin_Sensitive, rMax_Sensitive, dz_Cathode_Conductor / 2.0, phi1, phi2);
  G4LogicalVolume *cathodeConductorLog = new G4LogicalVolume(cathodeConductorSolid, material_Cathode_Conductor, "TPCCathodeConductorLog", 0, 0, 0);
  
  // place plus and minus z 
  new G4PVPlacement(0, G4ThreeVector(0.0, 0.0,  dz_Cathode_Insulator + (dz_Cathode_Conductor / 2.0)) , cathodeConductorLog, "TPCCathodeConductor_+z",cathodeLog, false, 0);
  new G4PVPlacement(0, G4ThreeVector(0.0, 0.0,-(dz_Cathode_Insulator + (dz_Cathode_Conductor / 2.0))), cathodeConductorLog, "TPCCathodeConductor_-z",cathodeLog, false, 0);
  
  //-----------------------------------------------------------------------------------------------//



  //----------------------------------------------- TPC Sensitive Detector (Pad Rings) ---------------------------------------------------------------//
  

  TPCSD04 *sensitiveDetector = new TPCSD04("TPC", sensitive_threshold_eV);

  RegisterSensitiveDetector(sensitiveDetector);

  G4UserLimits *userLimits = new G4UserLimits(TPCMaxStepLength);

  G4Tubs * senstiveGasSolid = new G4Tubs("TPCSensitiveGasSolid", rMin_Sensitive, rMax_Sensitive, dz_Sensitive / 2.0, phi1, phi2);

  G4ThreeVector translation(0,0,0);
  G4RotationMatrix rot;
  G4Transform3D transform(rot,translation);

  G4LogicalVolume *sensitiveGasLog = new G4LogicalVolume(senstiveGasSolid, material_TPC_Gas, "TPCSensitiveLog", 0, 0, userLimits);

  sensitiveGasLog->SetVisAttributes(G4VisAttributes::Invisible);
  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(   0 * deg), G4ThreeVector(0, 0, +( dz_Cathode/2.0 + dz_Sensitive/2.0 ) )), sensitiveGasLog, "TPCSensitiveLog_+z", motherLog, false, 0);
  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY( 180 * deg), G4ThreeVector(0, 0, -( dz_Cathode/2.0 + dz_Sensitive/2.0 ) )), sensitiveGasLog, "TPCSensitiveLog_-z", motherLog, false, 1);

 
  //---------------------------------------------------- Pad row doublets -------------------------------------------------------------------------------//
  for (G4int layer = 0; layer < numberPadRows; layer++) {

    // create twice the number of rings as there are pads, producing an lower and upper part of the pad with the boundry between them the pad-ring centre

    const G4double inner_lowerlayer_radius = rMin_Sensitive + (layer * (padHeight));
    const G4double outer_lowerlayer_radius = inner_lowerlayer_radius + (padHeight/2.0);

    const G4double inner_upperlayer_radius = outer_lowerlayer_radius ;
    const G4double outer_upperlayer_radius = inner_upperlayer_radius + (padHeight/2.0);

    G4Tubs *lowerlayerSolid = new G4Tubs("TPC_lowerlayer_solid", inner_lowerlayer_radius, outer_lowerlayer_radius, dz_Sensitive / 2.0, phi1, phi2);
    G4Tubs *upperlayerSolid = new G4Tubs("TPC_upperlayer_solid", inner_upperlayer_radius, outer_upperlayer_radius, dz_Sensitive / 2.0, phi1, phi2);

    G4LogicalVolume *lowerlayerLog = new G4LogicalVolume(lowerlayerSolid, material_TPC_Gas, "TPC_lowerlayer_log", 0, sensitiveDetector, userLimits);
    G4LogicalVolume *upperlayerLog = new G4LogicalVolume(upperlayerSolid, material_TPC_Gas, "TPC_upperlayer_log", 0, sensitiveDetector, userLimits);

    lowerlayerLog->SetVisAttributes(G4VisAttributes::Invisible);
    upperlayerLog->SetVisAttributes(G4VisAttributes::Invisible);

    new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(  0 * deg), G4ThreeVector(0, 0, 0)), 
		      lowerlayerLog, "TPCLowerLayer", sensitiveGasLog, false, layer+1);

    new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(  0 * deg), G4ThreeVector(0, 0, 0)), 
		      upperlayerLog, "TPCUpperLayer", sensitiveGasLog, false, layer+1);

  }


  // Assembly of the TPC Readout
  
  G4Tubs *readoutSolid = new G4Tubs("TPCReadoutSolid", rMin_GasVolume, rMax_GasVolume, dz_Readout / 2.0, phi1, phi2);
  G4LogicalVolume *readoutLog = new G4LogicalVolume(readoutSolid, material_TPC_Gas, "TPCReadoutLog", 0, 0, 0);
  readoutLog->SetVisAttributes(wallVisAttributes);
 
  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(  0 * deg), G4ThreeVector( 0, 0, +(dz_GasVolume - (dz_Readout / 2.0) ))), readoutLog, "TPCReadout_+z", motherLog, false, 0);
  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(180 * deg), G4ThreeVector( 0, 0, -(dz_GasVolume - (dz_Readout / 2.0) ))), readoutLog, "TPCReadout_-z", motherLog, false, 1);

  G4int pieceCounter = 0;
  G4double fracRadLengthReadout = 0;
  G4double zCursor = -dz_Readout / 2;
  
  db->exec("SELECT * FROM `readout`;");
  while (db->getTuple()) {
    const G4double dzPiece = db->fetchDouble("dz") * mm;
    G4Material *pieceMaterial = CGAGeometryManager::GetMaterial(db->fetchString("material"));
    
    G4Tubs *pieceSolid = new G4Tubs("TPCReadoutPieceSolid", rMin_GasVolume, rMax_GasVolume, dzPiece / 2, phi1, phi2);
    G4LogicalVolume *pieceLog = new G4LogicalVolume(pieceSolid, pieceMaterial, "TPCReadoutPieceLog", 0, 0, 0);
    pieceLog->SetVisAttributes(G4VisAttributes::Invisible);
    new G4PVPlacement(0, G4ThreeVector(0, 0, zCursor + dzPiece / 2), pieceLog, "TPCReadoutPiece", readoutLog, false, pieceCounter);
    
    ++pieceCounter;
    fracRadLengthReadout += dzPiece / pieceMaterial->GetRadlen();
    zCursor += dzPiece;

    if (zCursor > +dz_Readout / 2) Control::Abort("TPC10: Overfull TPC readout.",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

  }

  
  // Some verbose output
  
  G4cout << "TPC10: Readout material corresponds to " << G4int(fracRadLengthReadout * 1000) / 10.0 << "% of a radiation length." << G4endl;

  
  G4Tubs *endplateSolid = new G4Tubs("TPCEndplateSolid", rInner, rOuter, dz_Endplate / 2.0, phi1, phi2);
  G4LogicalVolume *endplateLog = new G4LogicalVolume(endplateSolid, endplate_MaterialMix, "TPCEndplateLog", 0, 0, 0);
  endplateLog->SetVisAttributes(wallVisAttributes);
 
  // note: as opposed to the readout, the endpate is not placed inside the gas volume
  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(  0 * deg), G4ThreeVector( 0, 0, +(dz_GasVolume + (dz_Endplate / 2.0) ))), endplateLog, "TPCEndplate_+z", motherLog, false, 0);
  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(180 * deg), G4ThreeVector( 0, 0, -(dz_GasVolume + (dz_Endplate / 2.0) ))), endplateLog, "TPCEndplate_-z", motherLog, false, 1);

  G4cout << "TPC10: Total Endplate material corresponds to " << (fracRadLengthReadout * 100.0) + (dz_Endplate / (endplate_MaterialMix->GetRadlen() / mm) * 100.0 ) << "% of a radiation length." << G4endl;
  
#ifdef MOKKA_GEAR
  
  // save the parameters needed to write the gear file ...

  _gear_r_min = rInner;
  _gear_r_max = rOuter;
  _gear_inner_wall_thickness = dr_InnerWall;
  _gear_outer_wall_thickness = dr_OuterWall;

  _gear_r_min_readout = rMin_Sensitive;
  _gear_r_max_readout = rMin_Sensitive + numberPadRows*padHeight;
  _gear_n_rows_readout = numberPadRows;
  _gear_pad_height = padHeight;
  _gear_pad_width = padWidth;
  
  _gear_max_drift_length = dz_Sensitive + dz_Cathode/2.0; // SJA: cathode has to be added as the sensitive region does not start at 0.00    
  _gear_z_anode = dzTotal - dz_Endplate; // the edge of the readout terminating the drift volume
  
#endif

  delete db;
  return true;
}


//new routine to ensure all data is obtained before called to
//be written to the gear xml file

#ifdef MOKKA_GEAR

void TPC10::GearSetup()
{ 

  G4EmCalculator findDEdx;
  G4ParticleTable* theParticleTable=G4ParticleTable::GetParticleTable();

  // create material mix for inner and outer field cage

  G4Material *inner_wall_material_mix = new G4Material("TPC_inner_wall_material_mix", gear_inner_wall_material_total_density, gear_inner_wall_material_thicknesses.size());
  
  G4cout << "TPC10: Create Inner Wall Material " << G4endl;
  
  for(  std::map<std::string, G4double>::iterator it=gear_inner_wall_material_thicknesses.begin(); it!=gear_inner_wall_material_thicknesses.end();++it )
    {
      G4Material* const material = CGAGeometryManager::GetMaterial( it->first );
      G4double fraction = it->second / _gear_inner_wall_thickness;
      inner_wall_material_mix->AddMaterial(material, fraction);
  G4cout << "TPC10: Add Material to Inner Wall: material " << material->GetName() << " X0 = " << material->GetRadlen() << " dr = "<< it->second << "  fraction " <<  fraction  << G4endl;
    }
  
  G4Material *outer_wall_material_mix = new G4Material("TPC_outer_wall_material_mix", gear_outer_wall_material_total_density, gear_outer_wall_material_thicknesses.size());

  for(  std::map<std::string, G4double>::iterator it=gear_outer_wall_material_thicknesses.begin(); it!=gear_outer_wall_material_thicknesses.end();++it )
    {
      G4Material* const material = CGAGeometryManager::GetMaterial( it->first );
      G4double fraction = it->second / _gear_outer_wall_thickness;
      outer_wall_material_mix->AddMaterial(material, fraction);
  G4cout << "TPC10: Add Material to Outer Wall: material " << material->GetName() << " X0 = " << material->GetRadlen() << " dr = "<< it->second << "  fraction " <<  fraction  << G4endl;
    }
  
  
  //Radiation Length from the Material database of materialMix
  G4double TPCInnerWallProperties_RadLen = inner_wall_material_mix->GetRadlen();
  G4cout << "TPC10: Inner Wall: material X0 = " << inner_wall_material_mix->GetRadlen() / cm 
  << " cm Density = " <<  inner_wall_material_mix->GetDensity()/(g/cm3) << " g/cm3" 
  << std::endl;

  //Radiation Length from the Material database of materialMix
  G4double TPCOuterWallProperties_RadLen = outer_wall_material_mix->GetRadlen();
  G4cout << "TPC10: Outer Wall: material X0 = " << outer_wall_material_mix->GetRadlen() / cm 
  << " cm Density = " <<  outer_wall_material_mix->GetDensity()/(g/cm3) << " g/cm3" 
  << std::endl;

  //Looping over bins in the DEDX table to obtain the mip DEDX 

  //From energy 0.0001MeV to 1000MeV in steps of 10
  G4double step_size = 10.0;
  G4double step = 0.0;
  G4double mindEdx=99999;
  G4double CurrentdEdx;


  // inner wall  
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx= 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  inner_wall_material_mix);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }
  G4double TPCInnerWallProperties_dEdx=(mindEdx)/1000;


  // outer wall  
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx= 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  outer_wall_material_mix);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }

  G4double TPCOuterWallProperties_dEdx=(mindEdx)/1000;
  

  //Obtaining the Radiation Length for the sensitive gas volume from the Materials database
  G4double TPCGasProperties_RadLen = _gear_gas_material-> GetRadlen();

  mindEdx=99999;
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx=
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  _gear_gas_material);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }
  
  
  //the DEDX values in GeV/mm (converted from MeV to GeV)
  G4double TPCGasProperties_dEdx=(mindEdx)/1000;

 
  // write data to MokkaGear that will print out as XML

// SJA:Note: this model cannot have gaps between pads, last parameter (pad gap) set to 0.0
  gear::PadRowLayout2D *padLayout = new gear::FixedPadSizeDiskLayout(_gear_r_min_readout, _gear_r_max_readout, _gear_pad_height, _gear_pad_width, _gear_n_rows_readout, 0.0);

  // parameters for TPCParameters
  gear::TPCParametersImpl *tpcParameters = new gear::TPCParametersImpl();
  tpcParameters->setPadLayout(padLayout);
  tpcParameters->setMaxDriftLength(   _gear_max_drift_length );
  tpcParameters->setDriftVelocity(    0.0); // SJA: not set in Mokka so set to 0.0
  tpcParameters->setReadoutFrequency( 0.0); // SJA: not set in Mokka so set to 0.0

  // set non-standardized parameters in map
  tpcParameters -> setDoubleVal( "tpcOuterRadius" , _gear_r_max ) ;
  tpcParameters -> setDoubleVal( "tpcInnerRadius", _gear_r_min ) ;
  tpcParameters -> setDoubleVal( "tpcInnerWallThickness",  _gear_inner_wall_thickness ) ;
  tpcParameters -> setDoubleVal( "tpcOuterWallThickness",  _gear_outer_wall_thickness ) ;
  tpcParameters -> setDoubleVal( "tpcZAnode", _gear_z_anode);
  
  tpcParameters -> setDoubleVal( "TPCInnerWallProperties_RadLen",TPCInnerWallProperties_RadLen ) ;
  tpcParameters -> setDoubleVal( "TPCInnerWallProperties_dEdx",TPCInnerWallProperties_dEdx) ;
  tpcParameters -> setDoubleVal( "TPCOuterWallProperties_RadLen",TPCOuterWallProperties_RadLen ) ;
  tpcParameters -> setDoubleVal( "TPCOuterWallProperties_dEdx",TPCOuterWallProperties_dEdx) ;
//SJA:Note: for backward compatibility for now set the TPCWallProperties to the inner wall
  tpcParameters -> setDoubleVal( "TPCWallProperties_RadLen",TPCInnerWallProperties_RadLen ) ;
  tpcParameters -> setDoubleVal( "TPCWallProperties_dEdx",TPCInnerWallProperties_dEdx) ;

  tpcParameters -> setDoubleVal( "TPCGasProperties_RadLen",TPCGasProperties_RadLen);
  tpcParameters -> setDoubleVal( "TPCGasProperties_dEdx", TPCGasProperties_dEdx);


  // write TPCParameters to GearMgr
  gear::GearMgr *gearMgr = MokkaGear::getMgr();
  gearMgr->setTPCParameters(tpcParameters);


}
#endif // MOKKA_GEAR
