// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: ETD00.cc,v 1.3 2006/03/21 16:41:39 adrian Exp $
// $Name: mokka-07-00 $
//
// History:  
// - modified version of TPC driver by Ties Behnke
// - detached from TPC03 as ETD00, stand-alone self-scaling driver -- Adrian Vogel, 2006-03-16

#include "ETD00.hh"

#include "CGAGeometryManager.hh"
#include "MySQLWrapper.hh"
#include "TRKSD00.hh"
#include "Control.hh"
#include "CGADefs.h"

#include "G4Tubs.hh"
#include "G4ThreeVector.hh"
#include "G4Transform3D.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

#include <sstream>

INSTANTIATE(ETD00)

G4bool ETD00::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog)
{
  const G4double phi1 =   0 * deg;
  const G4double phi2 = 360 * deg;

  // Geometry parameters from the geometry environment and from the database

  Database *db = new Database(env.GetDBName());
  db->exec("SELECT * FROM `disk`;");
  db->getTuple();

  const G4double rInnerTPC  = env.GetParameterAsDouble("TPC_inner_radius");
  const G4double rOuterTPC  = env.GetParameterAsDouble("TPC_outer_radius");
  const G4double zEndTPC    = env.GetParameterAsDouble("TPC_Ecal_Hcal_barrel_halfZ");
  const G4double drInnerGap = db->fetchDouble("drInnerGap") * mm;
  const G4double drOuterGap = db->fetchDouble("drOuterGap") * mm;
  const G4double dzETD      = env.GetParameterAsDouble("FCH_thickness");
  G4double       dzCables   = env.GetParameterAsDouble("Ecal_cables_gap");

  // check if there is enough space for the ETD
  // if not, create it and update the size of the gap between TPC and ECAL
  if (dzCables < dzETD) {
    dzCables = dzETD;
    std::ostringstream Ecal_cables_gap;
    Ecal_cables_gap << dzCables;
    (*Control::globalModelParameters)["Ecal_cables_gap"] = Ecal_cables_gap.str();
  }
  
  // Material for the ETD
  
  G4Material *etdMaterial   = CGAGeometryManager::GetMaterial("silicon_2.33gccm");
  
  // Simple calculations and some named constants

  const G4double rInnerETD  = rInnerTPC + drInnerGap;
  const G4double rOuterETD  = rOuterTPC - drOuterGap;
  const G4double zEndETD    = zEndTPC + dzCables; // glued on the ECAL endcap
  const G4double zETD       = zEndETD - dzETD / 2; // centre of the volume

  // Update the tracker region
  
  std::ostringstream tracker_region_zmax;
  tracker_region_zmax << zEndETD;
  (*Control::globalModelParameters)["tracker_region_zmax"] = tracker_region_zmax.str();

  // Visualisation attributes

  G4VisAttributes *etdVisAttributes = new G4VisAttributes(G4Colour(0.2, 0.0, 0.8)); // rich blue
  etdVisAttributes->SetForceWireframe(true);

  // Sensitive detector
  
  // in Si you get 340 keV/mm for a MIP, threshold is 20% of this
  TRKSD00 *sensitiveDetector = new TRKSD00("ETD", 0.2 * 340 * keV/mm * dzETD);
  RegisterSensitiveDetector(sensitiveDetector);

  // The disks
  
  G4Tubs *etdSolid = new G4Tubs("ETDSolid", rInnerETD, rOuterETD, dzETD / 2, phi1, phi2);
  G4LogicalVolume *etdLog = new G4LogicalVolume(etdSolid, etdMaterial, "ETDLog", 0, sensitiveDetector, 0);
  etdLog->SetVisAttributes(etdVisAttributes);
  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(  0 * deg), G4ThreeVector(0, 0, +zETD)), etdLog, "ETD", worldLog, false, 1000);
  new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(180 * deg), G4ThreeVector(0, 0, -zETD)), etdLog, "ETD", worldLog, false, 1001);

  delete db;
  return true;
}
