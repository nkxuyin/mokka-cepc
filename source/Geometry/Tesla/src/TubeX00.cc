// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: TubeX00.cc,v 1.2 2007/08/13 08:12:42 kristian Exp $
// $Name: mokka-07-00 $
//
// History:
// - first implementation of Tube00: P. Mora de Freitas, Sep 2002
// - modified from Tube00 to TubeDT01: Ties Behnke, 11-2-2003
// - modified for a crossing angle as TubeX00: Adrian Vogel, 2005-05-18

#include "TubeX00.hh"
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

#ifdef MOKKA_GEAR
#include "gearimpl/GearParametersImpl.h"
#include "MokkaGear.h"
#include "G4EmCalculator.hh"
#include "G4ParticleTable.hh"
#endif 


INSTANTIATE(TubeX00)

G4bool TubeX00::ContextualConstruct(const CGAGeometryEnvironment &geometryEnv, G4LogicalVolume *worldLog)
{
  // useful values for construction of tubes and cones
  const G4double phi1 =   0.0 * deg; // all cones start at zero...
  const G4double phi2 = 360.0 * deg; // ...and cover the whole 360 degrees

  // some visualization attributes for the tube wall and the vacuum inside
  G4VisAttributes *wallVisAttrib = new G4VisAttributes(G4Colour(1.0, 0.75, 0.5)); // light brown
  //wallVisAttrib->SetForceSolid(true);
  G4VisAttributes *vacuumVisAttrib = new G4VisAttributes(G4Colour(0.0, 0.0, 0.5)); // dark blue
  vacuumVisAttrib->SetVisibility(false); // there isn't anything, so what do you expect?
  
  bool firstPiece = true;
  material = "";
  beam_inner_radius = -99999;
  beam_thickness = -99999;
  
  Database *db = new Database(geometryEnv.GetDBName());
  db->exec("SELECT value FROM parameters WHERE name='crossingAngle';");
  if (!db->getTuple())
    Control::Abort("Parameter \"crossingAngle\" not found.",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
  const G4double crossingAngle = db->fetchDouble("value") / 2 * mrad; // only half the angle
  
  db->exec("SELECT * FROM tube;");
  while (db->getTuple()) {
    // fields in the data tuple
    const ECrossType crossType  = ECrossType(db->fetchInt("crossType")); // positioning of the volume
    const G4double zStart       = db->fetchDouble("zStart") * mm; // all these values are used directly...
    const G4double zEnd         = db->fetchDouble("zEnd") * mm; // ...in the G4Cons constructor below
    const G4double rInnerStart  = db->fetchDouble("rInnerStart") * mm;
    const G4double rInnerEnd    = db->fetchDouble("rInnerEnd") * mm;
    const G4double rOuterStart  = db->fetchDouble("rOuterStart") * mm;
    const G4double thickness    = rOuterStart - rInnerStart;
    const G4double rOuterEnd    = db->fetchDouble("rOuterEnd") * mm;
    const G4String materialName = db->fetchString("material"); // literal name for CGAGeometryManager::GetMaterial
    const G4String volName      = "tube_" + db->fetchString("name"); // literal name from the database

    if(firstPiece)
      { 
	firstPiece = false;
	material = materialName;
	beam_inner_radius = rInnerStart;
	beam_thickness = thickness;
	beamPipe_zHalf = fabs(zEnd - zStart);
      }
    
    // things which can be calculated immediately
    const G4double zHalf        = fabs(zEnd - zStart) / 2; // half z length of the cone
    const G4double zPosition    = fabs(zEnd + zStart) / 2; // middle z position
    G4Material *coreMaterial    = CGAGeometryManager::GetMaterial("beam"); // always the same
    G4Material *wallMaterial    = CGAGeometryManager::GetMaterial(materialName);

    // this could mess up your geometry, so better check it
    if (crossingAngle == 0 && crossType != kCenter) {
      Control::Log("You are trying to build a crossing geometry without a crossing angle.\n"
        "This is probably not what you want - better check your geometry data!");
      return false; // premature exit, Mokka will abort now
    }

    register G4double tmpAngle = 0; // default value, for kCenter and kPunched
    if      (crossType == kUpstream || crossType == kUpstreamClipped) tmpAngle = -crossingAngle;
    else if (crossType == kDnstream || crossType == kDnstreamClipped) tmpAngle = +crossingAngle;

    const G4double rotateAngle = tmpAngle; // for the placement at +z (better make it const now)
    const G4double mirrorAngle = 180 * deg - rotateAngle; // for the "mirrored" placement at -z
    // the "mirroring" in fact is done by a rotation of (almost) 180 degrees around the y-axis
    
    if (crossType == kCenter || crossType == kUpstream || crossType == kDnstream) {
      // a volume on the z-axis, on the upstream branch, or on the downstream branch
    
      // absolute transformations for the placement in the world
      G4Transform3D transformer(G4RotationMatrix().rotateY(rotateAngle), G4ThreeVector(0, 0, zPosition).rotateY(rotateAngle));
      G4Transform3D transmirror(G4RotationMatrix().rotateY(mirrorAngle), G4ThreeVector(0, 0, zPosition).rotateY(mirrorAngle));
      
      // solid for the tube (including vacuum and wall): a solid cone
      G4Cons *tubeSolid = new G4Cons(volName + "_solid", 0, rOuterStart, 0, rOuterEnd, zHalf, phi1, phi2);
      
      // tube consists of vacuum
      G4LogicalVolume *tubeLog = new G4LogicalVolume(tubeSolid, coreMaterial, volName + "_log", 0, 0, 0, true);
      tubeLog->SetVisAttributes(vacuumVisAttrib);
      
      // placement of the tube in the world, both at +z and -z
      G4PVPlacement *tubePhys = 0;
      tubePhys = new G4PVPlacement(transformer, tubeLog, volName, worldLog, false, 0);
      tubePhys = new G4PVPlacement(transmirror, tubeLog, volName, worldLog, false, 1);
      
      // the wall solid: a tubular cone
      G4Cons *wallSolid = new G4Cons(volName + "_wall_solid", rInnerStart, rOuterStart, rInnerEnd, rOuterEnd, zHalf, phi1, phi2);
      
      // the wall consists of the material given in the database
      G4LogicalVolume *wallLog = new G4LogicalVolume(wallSolid, wallMaterial, volName + "_wall_log", 0, 0, 0, true);
      wallLog->SetVisAttributes(wallVisAttrib);
      
      // placement as a daughter volume of the tube, will appear in both placements of the tube
      G4PVPlacement *wallPhys = 0; // one-liner would give a compiler warning
      wallPhys = new G4PVPlacement(0, G4ThreeVector(), wallLog, volName + "_wall", tubeLog, false, 0);
    
    } else if (crossType == kUpstreamClipped || crossType == kDnstreamClipped) {
      // a volume on the upstream or donwstream branch, but with the front face parallel to the xy-plane (!)
      // (implemented as a slightly longer cone from which the end is clipped off)
    
      // the volume which will be used for clipping: a solid tube
      const G4double &clipSize = rOuterStart; // the right order of magnitude for the clipping volume (alias name)
      G4Tubs *clipSolid = new G4Tubs(volName + "_clip", 0, 2 * clipSize, clipSize, phi1, phi2); // must be large enough
      
      // relative transformations for the composition of the G4SubtractionVolumes
      const G4double clipShift = (zPosition - clipSize / 2) - (zStart - clipSize) / cos(crossingAngle); // question: why is this correct?
      G4Transform3D clipTransformer(G4RotationMatrix().rotateY(-rotateAngle), G4ThreeVector(0, 0, -clipShift));
      G4Transform3D clipTransmirror(G4RotationMatrix().rotateY(+rotateAngle), G4ThreeVector(0, 0, -clipShift));

      // absolute transformations for the final placement in the world
      G4Transform3D placementTransformer(G4RotationMatrix().rotateY(rotateAngle), G4ThreeVector(0, 0, zPosition - clipSize / 2).rotateY(rotateAngle));
      G4Transform3D placementTransmirror(G4RotationMatrix().rotateY(mirrorAngle), G4ThreeVector(0, 0, zPosition - clipSize / 2).rotateY(mirrorAngle));

      // solid for the tube (including vacuum and wall): a solid cone
      G4Cons *wholeSolid = new G4Cons(volName + "_whole", 0, rOuterStart, 0, rOuterEnd, zHalf + clipSize / 2, phi1, phi2); // a bit longer

      // clip away the protruding end
      G4SubtractionSolid *tubeSolid0 = new G4SubtractionSolid(volName + "_solid_0", wholeSolid, clipSolid, clipTransformer);
      G4SubtractionSolid *tubeSolid1 = new G4SubtractionSolid(volName + "_solid_1", wholeSolid, clipSolid, clipTransmirror);
      
      // tube consists of vacuum (will later have two different daughters)
      G4LogicalVolume *tubeLog0 = new G4LogicalVolume(tubeSolid0, coreMaterial, volName + "_log_0", 0, 0, 0, true);
      G4LogicalVolume *tubeLog1 = new G4LogicalVolume(tubeSolid1, coreMaterial, volName + "_log_1", 0, 0, 0, true);
      tubeLog0->SetVisAttributes(vacuumVisAttrib);
      tubeLog1->SetVisAttributes(vacuumVisAttrib);
      
      // placement of the tube in the world, both at +z and -z
      G4PVPlacement *tubePhys = 0;
      tubePhys = new G4PVPlacement(placementTransformer, tubeLog0, volName, worldLog, false, 0);
      tubePhys = new G4PVPlacement(placementTransmirror, tubeLog1, volName, worldLog, false, 1);
     
      // the wall solid: a tubular cone
      G4Cons *wallWholeSolid = new G4Cons(volName + "_wall_whole", rInnerStart, rOuterStart, rInnerEnd, rOuterEnd, zHalf + clipSize / 2, phi1, phi2); // a bit longer
      
      // clip away the protruding end
      G4SubtractionSolid *wallSolid0 = new G4SubtractionSolid(volName + "_wall_solid_0", wallWholeSolid, clipSolid, clipTransformer);
      G4SubtractionSolid *wallSolid1 = new G4SubtractionSolid(volName + "_wall_solid_1", wallWholeSolid, clipSolid, clipTransmirror);
      
      // the wall consists of the material given in the database
      G4LogicalVolume *wallLog0 = new G4LogicalVolume(wallSolid0, wallMaterial, volName + "_wall_log_0", 0, 0, 0, true);
      G4LogicalVolume *wallLog1 = new G4LogicalVolume(wallSolid1, wallMaterial, volName + "_wall_log_1", 0, 0, 0, true);
      wallLog0->SetVisAttributes(wallVisAttrib);
      wallLog1->SetVisAttributes(wallVisAttrib);
      
      // placement as a daughter volumes of the tube
      G4PVPlacement *wallPhys = 0;
      wallPhys = new G4PVPlacement(0, G4ThreeVector(), wallLog0, volName + "_wall", tubeLog0, false, 0);
      wallPhys = new G4PVPlacement(0, G4ThreeVector(), wallLog1, volName + "_wall", tubeLog1, false, 1);
      
    } else if (crossType == kPunched) {
      // a volume on the z-axis with two inner holes
      // (implemented as a cone from which two tubes are punched out)
    
      const G4double &rUpstreamPunch = rInnerStart; // just alias names denoting what is meant here
      const G4double &rDnstreamPunch = rInnerEnd; // (the database entries are "abused" in this case)
    
      // relative transformations for the composition of the G4SubtractionVolumes
      G4Transform3D upstreamTransformer(G4RotationMatrix().rotateY(-crossingAngle), G4ThreeVector(zPosition * tan(-crossingAngle), 0, 0));
      G4Transform3D dnstreamTransformer(G4RotationMatrix().rotateY(+crossingAngle), G4ThreeVector(zPosition * tan(+crossingAngle), 0, 0));

      // absolute transformations for the final placement in the world (angles always equal zero and 180 deg)
      G4Transform3D placementTransformer(G4RotationMatrix().rotateY(rotateAngle), G4ThreeVector(0, 0, zPosition).rotateY(rotateAngle));
      G4Transform3D placementTransmirror(G4RotationMatrix().rotateY(mirrorAngle), G4ThreeVector(0, 0, zPosition).rotateY(mirrorAngle));

      // solid for the tube (including vacuum and wall): a solid cone
      G4Cons *tubeSolid = new G4Cons(volName + "_solid", 0, rOuterStart, 0, rOuterEnd, zHalf, phi1, phi2);
      
      // tube consists of vacuum (will later have two different daughters)
      G4LogicalVolume *tubeLog0 = new G4LogicalVolume(tubeSolid, coreMaterial, volName + "_log_0", 0, 0, 0, true);
      G4LogicalVolume *tubeLog1 = new G4LogicalVolume(tubeSolid, coreMaterial, volName + "_log_1", 0, 0, 0, true);
      tubeLog0->SetVisAttributes(vacuumVisAttrib);
      tubeLog1->SetVisAttributes(vacuumVisAttrib);
      
      // placement of the tube in the world, both at +z and -z
      G4PVPlacement *tubePhys = 0;
      tubePhys = new G4PVPlacement(placementTransformer, tubeLog0, volName, worldLog, false, 0);
      tubePhys = new G4PVPlacement(placementTransmirror, tubeLog1, volName, worldLog, false, 1);
      
      // the wall solid and the two pieces (only tubes, for the moment) which will be punched out
      G4Cons *wholeSolid = new G4Cons(volName + "_wall_whole", 0, rOuterStart, 0, rOuterEnd, zHalf, phi1, phi2);
      G4Tubs *upstreamPunch = new G4Tubs(volName + "_wall_punch_up", 0, rUpstreamPunch, 3 * zHalf, phi1, phi2); // a bit longer
      G4Tubs *dnstreamPunch = new G4Tubs(volName + "_wall_punch_dn", 0, rDnstreamPunch, 3 * zHalf, phi1, phi2); // a bit longer

      // the punched subtraction solids can be asymmetric and therefore have to be created twice:
      // one time in the "right" way, another time in the "reverse" way, because the "mirroring"
      // rotation around the y-axis will not only exchange +z and -z, but also +x and -x
      G4SubtractionSolid *tmpSolid0 = new G4SubtractionSolid(volName + "_wall_tmp_0", wholeSolid, upstreamPunch, upstreamTransformer);
      G4SubtractionSolid *tmpSolid1 = new G4SubtractionSolid(volName + "_wall_tmp_1", wholeSolid, upstreamPunch, dnstreamTransformer); // [sic]
      G4SubtractionSolid *wallSolid0 = new G4SubtractionSolid(volName + "_wall_solid_0", tmpSolid0, dnstreamPunch, dnstreamTransformer);
      G4SubtractionSolid *wallSolid1 = new G4SubtractionSolid(volName + "_wall_solid_1", tmpSolid1, dnstreamPunch, upstreamTransformer); // [sic]

      // the wall consists of the material given in the database
      G4LogicalVolume *wallLog0 = new G4LogicalVolume(wallSolid0, wallMaterial, volName + "_wall_log_0", 0, 0, 0, true);
      G4LogicalVolume *wallLog1 = new G4LogicalVolume(wallSolid1, wallMaterial, volName + "_wall_log_1", 0, 0, 0, true);
      wallLog0->SetVisAttributes(wallVisAttrib);
      wallLog1->SetVisAttributes(wallVisAttrib);

      // placement as a daughter volumes of the tube
      G4PVPlacement *wallPhys;
      wallPhys = new G4PVPlacement(0, G4ThreeVector(), wallLog0, volName + "_wall", tubeLog0, false, 0);
      wallPhys = new G4PVPlacement(0, G4ThreeVector(), wallLog1, volName + "_wall", tubeLog1, false, 1);
    }
  }
  delete db;
  return true;
}


#ifdef MOKKA_GEAR

void TubeX00::GearSetup()
{
  
  G4double CurrentdEdx, BeamPipe_RadLen, BeamPipe_dEdx;
  G4EmCalculator findDEdx;
  G4ParticleTable*theParticleTable=G4ParticleTable::GetParticleTable();


  G4Material *pipeMaterial = CGAGeometryManager::GetMaterial(material);

  BeamPipe_RadLen = pipeMaterial->GetRadlen();

  //Looping over bins in the DEDX table to obtain the mip DEDX 
  //From energy 0.0001MeV to 1000MeV in steps of 10
  G4double step_size=10,step,mindEdx=99999;
  
  
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx = 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  pipeMaterial);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }

  BeamPipe_dEdx=(mindEdx)/1000;

  gear::GearParametersImpl* gearParameters = new gear::GearParametersImpl;

  gearParameters -> setDoubleVal( "BeamPipeRadius", beam_inner_radius  ) ;
  gearParameters -> setDoubleVal( "BeamPipeHalfZ" ,  beamPipe_zHalf ) ;
  gearParameters -> setDoubleVal( "BeamPipeThickness" ,  beam_thickness) ;
  gearParameters -> setDoubleVal( "BeamPipeProperties_dEdx" , BeamPipe_dEdx ) ;
  gearParameters -> setDoubleVal( "BeamPipeProperties_RadLen" , BeamPipe_RadLen ) ;


  // Write gearParameters to GearMgr
  // Parameters for SIT
  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setGearParameters("BeamPipe", gearParameters ) ;
}


#endif 
