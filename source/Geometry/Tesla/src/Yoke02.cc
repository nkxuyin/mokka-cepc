// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: Yoke02.cc,v 1.2 2006/03/22 15:27:55 adrian Exp $
// $Name: mokka-07-00 $
//
// History:  
// - first implementation P. Mora de Freitas (May 2001)
// - selectable symmetry, self-scaling, removed pole tips -- Adrian Vogel, 2006-03-17

#include "Yoke02.hh"

#include "G4Polyhedra.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

#include "MySQLWrapper.hh"
#include "Control.hh"
#include "CGAGeometryManager.hh"

#include "CGADefs.h"

INSTANTIATE(Yoke02)

G4bool Yoke02::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog)
{
  Database *db = new Database(env.GetDBName());
  db->exec("SELECT * FROM `yoke`;");
  db->getTuple();
  
  // Geometry parameters from the geometry environment and from the database

  const G4int symmetry                = db->fetchInt("symmetry");

  const G4double rInnerBarrel         = env.GetParameterAsDouble("Yoke_barrel_inner_radius");
  const G4double rInnerEndcap         = env.GetParameterAsDouble("Yoke_endcap_inner_radius");
  const G4double drBarrel             = env.GetParameterAsDouble("Yoke_thickness");
  
  const G4double zStartEndcap         = env.GetParameterAsDouble("Yoke_Z_start_endcaps");
  const G4double zEndYoke             = env.GetParameterAsDouble("Yoke_Barrel_half_z");
  
  // Simple calculations and some named constants

  const G4double fullAngle            = 360 * deg;
  const G4double tiltAngle            = 180 * deg / symmetry - 90 * deg; // the yoke should always rest on a flat side
  
  const G4double rOuterBarrel         = rInnerBarrel + drBarrel;
  const G4double rOuterEndcap         = rInnerBarrel;
  const G4double dzEndcap             = zEndYoke - zStartEndcap;
  const G4double zEndcap              = zEndYoke - dzEndcap / 2;
  
  // These arrays are needed for the G4Polyhedra constructor
  
  const G4double zPosBarrelArray[2]   = { -zEndYoke, +zEndYoke };
  const G4double rInnerBarrelArray[2] = { rInnerBarrel, rInnerBarrel };
  const G4double rOuterBarrelArray[2] = { rOuterBarrel, rOuterBarrel };
  
  const G4double zPosEndcapArray[2]   = { -dzEndcap / 2, +dzEndcap / 2 };
  const G4double rInnerEndcapArray[2] = { rInnerEndcap, rInnerEndcap };
  const G4double rOuterEndcapArray[2] = { rOuterEndcap, rOuterEndcap };
  
  // Materials

  G4Material *yokeMaterial = CGAGeometryManager::GetMaterial("iron");
  
  // Visualisation attributes

  G4VisAttributes *yokeVisAttributes = new G4VisAttributes(G4Colour(0.1, 0.8, 0.8)); // light cyan
  yokeVisAttributes->SetForceWireframe(true);
  
  // Volumes for the barel and the endcap
  
  G4Polyhedra *barrelSolid = new G4Polyhedra("YokeBarrelSolid", tiltAngle, fullAngle, symmetry, 2, zPosBarrelArray, rInnerBarrelArray, rOuterBarrelArray);
  G4LogicalVolume *barrelLog = new G4LogicalVolume(barrelSolid, yokeMaterial, "YokeBarrelLog", 0, 0, 0);
  barrelLog->SetVisAttributes(yokeVisAttributes);
  new G4PVPlacement(0, G4ThreeVector(), barrelLog, "YokeBarrel", worldLog, false, 0);

  G4Polyhedra *endcapSolid = new G4Polyhedra("YokeEndcapSolid", tiltAngle, fullAngle, symmetry, 2, zPosEndcapArray, rInnerEndcapArray, rOuterEndcapArray);
  G4LogicalVolume *endcapLog = new G4LogicalVolume(endcapSolid, yokeMaterial, "YokeEndcapLog", 0, 0, 0);
  endcapLog->SetVisAttributes(yokeVisAttributes);
  new G4PVPlacement(0, G4ThreeVector(0, 0, +zEndcap), endcapLog, "YokeEndcap", worldLog, false, 0);
  new G4PVPlacement(0, G4ThreeVector(0, 0, -zEndcap), endcapLog, "YokeEndcap", worldLog, false, 1);
  
  delete db;
  return true;
}
