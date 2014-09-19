// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: MaskX00.cc,v 1.1 2005/10/10 17:19:11 adrian Exp $
// $Name: mokka-07-00 $
//
// History:
// - first implementation as LAT00: Peter Wienemann, Jun 2003
// - modified for a crossing angle as MaskX00: Adrian Vogel, 2005-05-19

#include "MaskX00.hh"
#include "MySQLWrapper.hh"
#include "CGAGeometryManager.hh"
#include "CGAGeometryEnvironment.hh"
#include "CGADefs.h"
#include "Control.hh"

#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"

#include "G4VisAttributes.hh"
#include "G4Material.hh"
#include "G4Cons.hh"
#include "G4Tubs.hh"
#include "G4SubtractionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

INSTANTIATE(MaskX00)

G4bool MaskX00::ContextualConstruct(const CGAGeometryEnvironment &geometryEnv, G4LogicalVolume *worldLog)
{
  // useful values for construction of tubes and cones
  const G4double phi1 =   0.0 * deg; // all cones start at zero...
  const G4double phi2 = 360.0 * deg; // ...and cover the whole 360 degrees

  Database *componentsDB = new Database(geometryEnv.GetDBName()); // which components are to be built?
  Database *consDB = new Database(geometryEnv.GetDBName()); // which G4Cons belong to each component?

  componentsDB->exec("SELECT value FROM _parameters WHERE name='crossingAngle';");
  if (!componentsDB->getTuple())
    Control::Abort("Parameter \"crossingAngle\" not found.",MOKKA_ERROR_DATABASE_SELECT_ERROR);
  const G4double crossingAngle = componentsDB->fetchDouble("value") / 2 * mrad; // only half the angle
  
  componentsDB->exec("SELECT * FROM _components;"); // get all components which should be built
  while (componentsDB->getTuple()) { // build each single component with...

    const G4String name        = componentsDB->fetchString("name"); // its own table in the database (with this name)
    const G4double colorR      = componentsDB->fetchDouble("colorR"); // a common colour for all its G4Cons
    const G4double colorG      = componentsDB->fetchDouble("colorG"); // (three RGB colour components)
    const G4double colorB      = componentsDB->fetchDouble("colorB");
    const G4String visAttStr   = componentsDB->fetchString("visAttrib"); // common G4VisAttributes for all G4Cons

    G4VisAttributes *visAttrib = new G4VisAttributes(G4Colour(colorR, colorG, colorB)); // initialize with a colour
    switch (visAttStr(0)) { // this string (only its first character, to be precise) can force a certain behaviour:
      case 's': visAttrib->SetForceSolid(true); break;
      case 'w': visAttrib->SetForceWireframe(true); break;
      case 'i': visAttrib->SetVisibility(false); break;
    } // all other values are silently ignored

    consDB->exec(G4String("SELECT * FROM " + name + ";").data()); // query the table that belongs to this component
    while (consDB->getTuple()) {
      const ECrossType crossType  = ECrossType(consDB->fetchInt("crossType")); // center, upstream, downstream, or punched
      const G4double zStart       = consDB->fetchDouble("zStart") * mm; // all these values are used directly...
      const G4double zEnd         = consDB->fetchDouble("zEnd") * mm; // ...in the G4Cons constructor below
      const G4double rInnerStart  = consDB->fetchDouble("rInnerStart") * mm;
      const G4double rInnerEnd    = consDB->fetchDouble("rInnerEnd") * mm;
      const G4double rOuterStart  = consDB->fetchDouble("rOuterStart") * mm;
      const G4double rOuterEnd    = consDB->fetchDouble("rOuterEnd") * mm;
      const G4String materialName = consDB->fetchString("material"); // literal name for CGAGeometryManager::GetMaterial
      const G4String volName      = "mask_" + consDB->fetchString("name"); // literal volume name for later usage

      // things which can be calculated immediately
      const G4double zHalf        = fabs(zStart - zEnd) / 2; // half z length of the cone
      const G4double zPosition    = fabs(zStart + zEnd) / 2; // middle z position
      G4Material *volMaterial     = CGAGeometryManager::GetMaterial(materialName);

      if (crossingAngle == 0 && crossType != kCenter) {
        Control::Log("You are trying to build a crossing geometry without a crossing angle.\n"
          "This is probably not what you want - better check your geometry data!");
        return false;
      }

      if (crossType != kPunched) {
        register G4double tmpAngle = 0;
        if      (crossType == kUpstream || crossType == kUpstreamClipped) tmpAngle = -crossingAngle;
        else if (crossType == kDnstream || crossType == kDnstreamClipped) tmpAngle = +crossingAngle;

        const G4double rotateAngle = tmpAngle; // for the placement at +z (better make it const now)
        const G4double mirrorAngle = 180 * deg - rotateAngle; // for the "mirrored" placement at -z

        G4Transform3D transformer(G4RotationMatrix().rotateY(rotateAngle), G4ThreeVector(0, 0, zPosition).rotateY(rotateAngle));
        G4Transform3D transmirror(G4RotationMatrix().rotateY(mirrorAngle), G4ThreeVector(0, 0, zPosition).rotateY(mirrorAngle));
        // the "mirroring" in fact is done by a rotation of (almost) 180 degrees around the y-axis

        // Create a solid with the given dimensions

        G4Cons *volSolid = new G4Cons(volName + "_solid" , rInnerStart, rOuterStart, rInnerEnd, rOuterEnd, zHalf, phi1, phi2);

        G4LogicalVolume *volLog = new G4LogicalVolume(volSolid, volMaterial, volName + "_log", 0, 0, 0, true);
        volLog->SetVisAttributes(visAttrib);

        G4PVPlacement *volPhys;
        volPhys = new G4PVPlacement(transformer, volLog, volName, worldLog, false, 0);
        volPhys = new G4PVPlacement(transmirror, volLog, volName, worldLog, false, 1);

      } else { // (crossType == kPunched)
        // a cone with two inner holes (two tubes are punched out)
      
        const G4double &rUpstreamPunch = rInnerStart; // just alias names denoting what is meant here
        const G4double &rDnstreamPunch = rInnerEnd; // (the database entries are "abused" in this case)
    
        // relative transformations for the composition of the G4SubtractionVolumes
        G4Transform3D upstreamTransformer(G4RotationMatrix().rotateY(-crossingAngle), G4ThreeVector(zPosition * tan(-crossingAngle), 0, 0));
        G4Transform3D dnstreamTransformer(G4RotationMatrix().rotateY(+crossingAngle), G4ThreeVector(zPosition * tan(+crossingAngle), 0, 0));

        // absolute transformations for the final placement in the world
        G4Transform3D placementTransformer(G4RotationMatrix().rotateY(  0 * deg), G4ThreeVector(0, 0, zPosition).rotateY(  0 * deg));
        G4Transform3D placementTransmirror(G4RotationMatrix().rotateY(180 * deg), G4ThreeVector(0, 0, zPosition).rotateY(180 * deg));

        // the main solid and the two pieces (only tubes, for the moment) which will be punched out
        G4Cons *wholeSolid = new G4Cons(volName + "_whole", 0 * mm, rOuterStart, 0 * mm, rOuterEnd, zHalf, phi1, phi2);
        G4Tubs *upstreamPunch = new G4Tubs(volName + "_punch_up", 0 * mm, rUpstreamPunch, 2 * zHalf, phi1, phi2);
        G4Tubs *dnstreamPunch = new G4Tubs(volName + "_punch_dn", 0 * mm, rDnstreamPunch, 2 * zHalf, phi1, phi2);

        // the punched subtraction solids can be asymmetric and therefore have to be created twice:
        // one time in the "right" way, another time in the "reverse" way, because the "mirroring"
        // rotation around the y-axis will not only exchange +z and -z, but also +x and -x
        G4SubtractionSolid *tmpSolid0 = new G4SubtractionSolid(volName + "_tmp_0", wholeSolid, upstreamPunch, upstreamTransformer);
        G4SubtractionSolid *tmpSolid1 = new G4SubtractionSolid(volName + "_tmp_1", wholeSolid, upstreamPunch, dnstreamTransformer); // [sic]
        G4SubtractionSolid *finalSolid0 = new G4SubtractionSolid(volName + "_solid_0", tmpSolid0, dnstreamPunch, dnstreamTransformer);
        G4SubtractionSolid *finalSolid1 = new G4SubtractionSolid(volName + "_solid_1", tmpSolid1, dnstreamPunch, upstreamTransformer); // [sic]

        G4LogicalVolume *finalLog0 = new G4LogicalVolume(finalSolid0, volMaterial, volName + "_log_0", 0, 0, 0, true);
        G4LogicalVolume *finalLog1 = new G4LogicalVolume(finalSolid1, volMaterial, volName + "_log_1", 0, 0, 0, true);
        finalLog0->SetVisAttributes(visAttrib);
        finalLog1->SetVisAttributes(visAttrib);

        G4PVPlacement *finalPhys;
        finalPhys = new G4PVPlacement(placementTransformer, finalLog0, volName, worldLog, false, 0);
        finalPhys = new G4PVPlacement(placementTransmirror, finalLog1, volName, worldLog, false, 1);
      }
    } // no more G4Cons for this component
  } // no more components to be built
  delete consDB;
  delete componentsDB;
  return true;
}
