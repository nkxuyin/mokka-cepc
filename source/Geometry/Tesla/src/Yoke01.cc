// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: Yoke01.cc,v 1.1 2005/10/10 17:19:11 adrian Exp $
// $Name: mokka-07-00 $
//
// History:  
// - first implementation as Yoke00.cc -- Paulo Mora de Freitas, May 01
// - rewritten, modified for octagon as Yoke01.cc -- Adrian Vogel, 2005-08-09

#include "Yoke01.hh"
#include "MySQLWrapper.hh"
#include "Control.hh"
#include "CGAGeometryManager.hh"
#include "CGADefs.h"

#include "globals.hh"
#include "G4Polyhedra.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

INSTANTIATE(Yoke01)

G4bool Yoke01::ContextualConstruct(const CGAGeometryEnvironment &geometryEnv, G4LogicalVolume *worldLog)
{
  const G4int nSides = 8; // (was 12 in earlier times)
  const G4double angleSpan = 360 * deg; // construct the full circle
  const G4double angleTilt = 180 * deg / nSides; // put the polyhedron on its flat side

  G4VisAttributes *yokeVisAttrib = new G4VisAttributes(G4Colour(0.5, 0.8, 0.8));
  yokeVisAttrib->SetForceWireframe(true); // otherwise you'll hardly see anything else
  
  G4Material *yokeMaterial = CGAGeometryManager::GetMaterial("iron"); 

  Database *db = new Database(geometryEnv.GetDBName());
  db->exec("SELECT * FROM yoke;");
  while (db->getTuple()) {
    const G4double zStart        = db->fetchDouble("zStart") * mm;
    const G4double zEnd          = db->fetchDouble("zEnd") * mm;
    const G4double rInnerStart   = db->fetchDouble("rInnerStart") * mm;
    const G4double rInnerEnd     = db->fetchDouble("rInnerEnd") * mm;
    const G4double rOuterStart   = db->fetchDouble("rOuterStart") * mm;
    const G4double rOuterEnd     = db->fetchDouble("rOuterEnd") * mm;
    const G4String partName      = "yoke_" + db->fetchString("partName");
    
    const G4bool atZero = (zStart == 0 && rInnerStart == rInnerEnd && rOuterStart == rOuterEnd);
    // we can construct one single solid reaching from -zEnd to +zEnd and don't have to mirror it

    const G4double zPosition     = (atZero) ?    (0) : ((zEnd + zStart) / 2);
    const G4double zHalf         = (atZero) ? (zEnd) : ((zEnd - zStart) / 2);
    
    const G4double zPlanePoly[2] = { -zHalf, +zHalf };
    const G4double rInnerPoly[2] = { rInnerStart, rInnerEnd }; // needed in this array format...
    const G4double rOuterPoly[2] = { rOuterStart, rOuterEnd }; // ...for the G4Polyhedra constructor
    
    const G4Transform3D transformer(G4RotationMatrix().rotateY(  0 * deg), G4ThreeVector(0, 0, zPosition).rotateY(  0 * deg));
    const G4Transform3D transmirror(G4RotationMatrix().rotateY(180 * deg), G4ThreeVector(0, 0, zPosition).rotateY(180 * deg));
    
    G4Polyhedra *yokeSolid = new G4Polyhedra(partName + "_solid", angleTilt, angleSpan, nSides, 2, zPlanePoly, rInnerPoly, rOuterPoly);
    G4LogicalVolume *yokeLog = new G4LogicalVolume(yokeSolid, yokeMaterial, partName + "_log", 0, 0, 0);
    yokeLog->SetVisAttributes(yokeVisAttrib);
    G4PVPlacement *yokePhys;
    yokePhys              = new G4PVPlacement(transformer, yokeLog, partName, worldLog, false, 0);
    if (!atZero) yokePhys = new G4PVPlacement(transmirror, yokeLog, partName, worldLog, false, 1);
  }
  delete db;
  return true;
}
