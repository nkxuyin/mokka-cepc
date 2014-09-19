// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: Mask_cepc.cc,v 1.2 2006/05/21 00:35:13 adrian Exp $
// $Name: mokka-07-00 $
//
// History:
// - first implementation as LAT00: Peter Wienemann, Jun 2003
// - modified for a crossing angle as MaskX00: Adrian Vogel, 2005-05-19
// - modified for fancier geometries as Mask_cepc: Adrian Vogel, 2006-04-20

#include "Mask_cepc.hh"
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

INSTANTIATE(Mask_cepc)

G4bool Mask_cepc::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog)
{
  // useful values for construction of tubes and cones
  const G4double phi1 =   0.0 * deg; // all cones start at zero...
  const G4double phi2 = 360.0 * deg; // ...and cover the whole 360 degrees

  const G4double crossingAngle = env.GetParameterAsDouble("ILC_Main_Crossing_Angle") / 2 * mrad; // only half the angle

  //modify by xuyin: change db name
  //const G4String dbName = env.GetDBName() + "_" + env.GetParameterAsString("ILC_Main_Crossing_Angle");
  const G4String dbName = env.GetDBName();

  Database *componentsDB = new Database(dbName.c_str()); // which components are to be built?
  Database *consDB = new Database(dbName.c_str()); // which G4Cons belong to each component?
  
  G4bool usingOffsets = false;
  TReferenceMap referenceOffsets;
  componentsDB->exec("SELECT * FROM `_references`;");
  while (componentsDB->getTuple()) {
    const G4String globalName   = componentsDB->fetchString("globalName");
    const G4String localName    = componentsDB->fetchString("localName");
    const G4double assumedValue = componentsDB->fetchDouble("assumption") * mm;
    const G4double currentValue = env.GetParameterAsDouble(globalName);
    const G4double offsetValue  = currentValue - assumedValue;
    referenceOffsets[localName] = offsetValue;

    if (offsetValue != 0) {
      G4cout
        << "Mask_cepc: Using " << globalName << " = "
        << currentValue / mm << " mm instead of "
        << assumedValue / mm << " mm" << G4endl;
      usingOffsets = true;
    }
  }
  if (usingOffsets) Control::Log("Mask_cepc: Be sure you know what you're doing!");

  componentsDB->exec("SELECT * FROM `_components`;"); // get all components which should be built
  while (componentsDB->getTuple()) { // build each single component with...

    const G4String name        = componentsDB->fetchString("name"); // its own table in the database (with this name)
    const G4double colorR      = componentsDB->fetchDouble("colorR"); // a common colour for all its G4Cons
    const G4double colorG      = componentsDB->fetchDouble("colorG"); // (three RGB colour components)
    const G4double colorB      = componentsDB->fetchDouble("colorB");
    const G4String visAttStr   = componentsDB->fetchString("visAttrib"); // common G4VisAttributes for all G4Cons

    G4VisAttributes *visAttrib = new G4VisAttributes(G4Colour(colorR, colorG, colorB)); // initialize with a colour
    switch (visAttStr(0)) { // this string (only its first character, to be precise) can force a certain behaviour:
      case 's': visAttrib->SetForceSolid(true);     break;
      case 'w': visAttrib->SetForceWireframe(true); break;
      case 'i': visAttrib->SetVisibility(false);    break;
    } // all other values are silently ignored

    consDB->exec(G4String("SELECT * FROM `" + name + "`;").c_str()); // query the table that belongs to this component
    while (consDB->getTuple()) {
      // reference values for r- and z-values
      const G4String zStartRef         = consDB->fetchString("zStartRef");
      const G4String zEndRef           = consDB->fetchString("zEndRef");
      const G4String rInnerStartRef    = consDB->fetchString("rInnerStartRef");
      const G4String rInnerEndRef      = consDB->fetchString("rInnerEndRef");
      const G4String rOuterStartRef    = consDB->fetchString("rOuterStartRef");
      const G4String rOuterEndRef      = consDB->fetchString("rOuterEndRef");
    
      const G4double zStartOffset      = (zStartRef      == "") ? (0) : (referenceOffsets[zStartRef]);
      const G4double zEndOffset        = (zEndRef        == "") ? (0) : (referenceOffsets[zEndRef]);
      const G4double rInnerStartOffset = (rInnerStartRef == "") ? (0) : (referenceOffsets[rInnerStartRef]);
      const G4double rInnerEndOffset   = (rInnerEndRef   == "") ? (0) : (referenceOffsets[rInnerEndRef]);
      const G4double rOuterStartOffset = (rOuterStartRef == "") ? (0) : (referenceOffsets[rOuterStartRef]);
      const G4double rOuterEndOffset   = (rOuterEndRef   == "") ? (0) : (referenceOffsets[rOuterEndRef]);
  
      const ECrossType crossType  = ECrossType(consDB->fetchInt("crossType")); // center, upstream, downstream, or punched
      const G4double zStart       = consDB->fetchDouble("zStart")      * mm + zStartOffset;
      const G4double zEnd         = consDB->fetchDouble("zEnd")        * mm + zEndOffset;
      const G4double rInnerStart  = consDB->fetchDouble("rInnerStart") * mm + rInnerStartOffset;
      const G4double rInnerEnd    = consDB->fetchDouble("rInnerEnd")   * mm + rInnerEndOffset;
      const G4double rOuterStart  = consDB->fetchDouble("rOuterStart") * mm + rOuterStartOffset;
      const G4double rOuterEnd    = consDB->fetchDouble("rOuterEnd")   * mm + rOuterEndOffset;
      const G4String materialName = consDB->fetchString("material");
      const G4String volName      = "mask_" + consDB->fetchString("name");

      // things which can be calculated immediately
      const G4double zHalf        = fabs(zStart - zEnd) / 2; // half z length of the cone
      const G4double zPosition    = fabs(zStart + zEnd) / 2; // middle z position
      G4Material *volMaterial     = CGAGeometryManager::GetMaterial(materialName);

      if (crossingAngle == 0 && crossType != kCenter) {
        Control::Log("Mask_cepc: You are trying to build a crossing geometry without a crossing angle.\n"
          "This is probably not what you want - better check your geometry data!");
        return false;
      }

      register G4double tmpAngle;
      switch (crossType) {
        case kUpstream:
        case kPunchedUpstream:
          tmpAngle = -crossingAngle;
          break;
        case kDnstream:
        case kPunchedDnstream:
          tmpAngle = +crossingAngle;
          break;
        default:
          tmpAngle = 0;
          break;
      }

      const G4double rotateAngle = tmpAngle; // for the placement at +z (better make it const now)
      const G4double mirrorAngle = 180 * deg - rotateAngle; // for the "mirrored" placement at -z

      switch (crossType) {
        case kCenter:
        case kUpstream:
        case kDnstream: {
          G4Transform3D transformer(G4RotationMatrix().rotateY(rotateAngle), G4ThreeVector(0, 0, zPosition).rotateY(rotateAngle));
          G4Transform3D transmirror(G4RotationMatrix().rotateY(mirrorAngle), G4ThreeVector(0, 0, zPosition).rotateY(mirrorAngle));
          // the "mirroring" in fact is done by a rotation of (almost) 180 degrees around the y-axis
  
          // Create a solid with the given dimensions
          G4Cons *volSolid = new G4Cons(volName, rInnerStart, rOuterStart, rInnerEnd, rOuterEnd, zHalf, phi1, phi2);
  
          G4LogicalVolume *volLog = new G4LogicalVolume(volSolid, volMaterial, volName, 0, 0, 0, true);
          volLog->SetVisAttributes(visAttrib);
  
          new G4PVPlacement(transformer, volLog, volName, worldLog, false, 0);
          new G4PVPlacement(transmirror, volLog, volName, worldLog, false, 1);
          break;
        }
        case kPunchedCenter: {
          // a cone with one or two inner holes (two tubes are punched out)
        
          const G4double rUpstreamPunch = rInnerStart; // just alias names denoting what is meant here
          const G4double rDnstreamPunch = rInnerEnd; // (the database entries are "abused" in this case)
      
          // relative transformations for the composition of the G4SubtractionVolumes
          G4Transform3D upstreamTransformer(G4RotationMatrix().rotateY(-crossingAngle), G4ThreeVector(zPosition * tan(-crossingAngle), 0, 0));
          G4Transform3D dnstreamTransformer(G4RotationMatrix().rotateY(+crossingAngle), G4ThreeVector(zPosition * tan(+crossingAngle), 0, 0));
  
          // absolute transformations for the final placement in the world
          G4Transform3D placementTransformer(G4RotationMatrix().rotateY(rotateAngle), G4ThreeVector(0, 0, zPosition).rotateY(rotateAngle));
          G4Transform3D placementTransmirror(G4RotationMatrix().rotateY(mirrorAngle), G4ThreeVector(0, 0, zPosition).rotateY(mirrorAngle));
  
          // the main solid and the two pieces (only tubes, for the moment) which will be punched out
          G4Cons *wholeSolid = new G4Cons(volName + "_whole", 0, rOuterStart, 0, rOuterEnd, zHalf, phi1, phi2);
          G4VSolid *tmpSolid0, *tmpSolid1, *finalSolid0, *finalSolid1;
  
          // the punched subtraction solids can be asymmetric and therefore have to be created twice:
          // one time in the "right" way, another time in the "reverse" way, because the "mirroring"
          // rotation around the y-axis will not only exchange +z and -z, but also +x and -x
          if (rUpstreamPunch) { // do we need a hole on the upstream branch?
            G4Tubs *upstreamPunch = new G4Tubs(volName + "_punch_up", 0 * mm, rUpstreamPunch, 2 * zHalf, phi1, phi2);
            tmpSolid0 = new G4SubtractionSolid(volName + "_tmp_0", wholeSolid, upstreamPunch, upstreamTransformer);
            tmpSolid1 = new G4SubtractionSolid(volName + "_tmp_1", wholeSolid, upstreamPunch, dnstreamTransformer); // [sic]
          } else {
            tmpSolid0 = wholeSolid;
            tmpSolid1 = wholeSolid;
          }
          
          if (rDnstreamPunch) { // do we need a hole on the downstream branch?
            G4Tubs *dnstreamPunch = new G4Tubs(volName + "_punch_dn", 0 * mm, rDnstreamPunch, 2 * zHalf, phi1, phi2);
            finalSolid0 = new G4SubtractionSolid(volName + "_0", tmpSolid0, dnstreamPunch, dnstreamTransformer);
            finalSolid1 = new G4SubtractionSolid(volName + "_1", tmpSolid1, dnstreamPunch, upstreamTransformer); // [sic]
          } else {
            finalSolid0 = tmpSolid0;
            finalSolid1 = tmpSolid1;
          }
  
          G4LogicalVolume *finalLog0 = new G4LogicalVolume(finalSolid0, volMaterial, volName + "_0", 0, 0, 0, true);
          G4LogicalVolume *finalLog1 = new G4LogicalVolume(finalSolid1, volMaterial, volName + "_1", 0, 0, 0, true);
          finalLog0->SetVisAttributes(visAttrib);
          finalLog1->SetVisAttributes(visAttrib);
  
          new G4PVPlacement(placementTransformer, finalLog0, volName, worldLog, false, 0);
          new G4PVPlacement(placementTransmirror, finalLog1, volName, worldLog, false, 1);
          break;
        }
        case kPunchedUpstream:
        case kPunchedDnstream: {
          // a volume on the upstream or downstream branch with two inner holes
          // (implemented as a cone from which another tube is punched out)
        
          const G4double rCenterPunch = (crossType == kPunchedUpstream) ? (rInnerStart) : (rInnerEnd); // radius of the central hole
          const G4double rOffsetPunch = (crossType == kPunchedDnstream) ? (rInnerStart) : (rInnerEnd); // radius of the off-axis hole
        
          // relative transformations for the composition of the G4SubtractionVolumes
          G4Transform3D punchTransformer(G4RotationMatrix().rotateY(-2 * rotateAngle), G4ThreeVector(zPosition * tan(-2 * rotateAngle), 0, 0));
          G4Transform3D punchTransmirror(G4RotationMatrix().rotateY(+2 * rotateAngle), G4ThreeVector(zPosition * tan(+2 * rotateAngle), 0, 0));
    
          // absolute transformations for the final placement in the world
          G4Transform3D placementTransformer(G4RotationMatrix().rotateY(rotateAngle), G4ThreeVector(0, 0, zPosition).rotateY(rotateAngle));
          G4Transform3D placementTransmirror(G4RotationMatrix().rotateY(mirrorAngle), G4ThreeVector(0, 0, zPosition).rotateY(mirrorAngle));
    
          // the main solid and the piece (only a tube, for the moment) which will be punched out
          G4Cons *wholeSolid = new G4Cons(volName + "_whole", rCenterPunch, rOuterStart, rCenterPunch, rOuterEnd, zHalf, phi1, phi2);
          G4Tubs *punchSolid = new G4Tubs(volName + "_punch", 0 * mm, rOffsetPunch, 2 * zHalf, phi1, phi2);
    
          // the punched subtraction solids can be asymmetric and therefore have to be created twice:
          // one time in the "right" way, another time in the "reverse" way, because the "mirroring"
          // rotation around the y-axis will not only exchange +z and -z, but also +x and -x
          G4SubtractionSolid *finalSolid0 = new G4SubtractionSolid(volName + "_0", wholeSolid, punchSolid, punchTransformer);
          G4SubtractionSolid *finalSolid1 = new G4SubtractionSolid(volName + "_1", wholeSolid, punchSolid, punchTransmirror);
  
          G4LogicalVolume *finalLog0 = new G4LogicalVolume(finalSolid0, volMaterial, volName + "_0", 0, 0, 0, true);
          G4LogicalVolume *finalLog1 = new G4LogicalVolume(finalSolid1, volMaterial, volName + "_1", 0, 0, 0, true);
          finalLog0->SetVisAttributes(visAttrib);
          finalLog1->SetVisAttributes(visAttrib);
  
          new G4PVPlacement(placementTransformer, finalLog0, volName, worldLog, false, 0);
          new G4PVPlacement(placementTransmirror, finalLog1, volName, worldLog, false, 1);
          break;
        }
        default: {
          Control::Log("Mask_cepc: Unimplemented \"crossType\" code.");
          return false; // fatal failure
        }
      } // switch (crossType)
    } // no more G4Cons for this component
  } // no more components to be built
  delete consDB;
  delete componentsDB;
  return true;
}
