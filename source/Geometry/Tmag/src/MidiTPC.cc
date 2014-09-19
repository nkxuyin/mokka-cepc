
// History:  
// MIDI TPC - DESY prototype tpc  -- Predrag Krstonosic, 2005-06-09

#include "MidiTPC.hh"

#include "TPCSD01.hh"
#include "CGAGeometryManager.hh"
#include "MySQLWrapper.hh"
#include "Control.hh"
#include "CGADefs.h"

#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4PVPlacement.hh"
#include "G4UserLimits.hh"

#include <iomanip>

INSTANTIATE(MidiTPC)
G4bool MidiTPC::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *WorldLog)
{ 
  G4cout << "\nBuilding MidiTPC..." << G4endl;
  const G4double phi1 =   0 * deg;
  const G4double phi2 = 360 * deg;

  Database *db = new Database(env.GetDBName());
  db->exec("SELECT * FROM `tpc`;");
  db->getTuple();


  G4String matname=db->fetchString("material");

  G4Material *materialGas = CGAGeometryManager::GetMaterial(matname);


  // threshold is twice the energy for Ar ionisation, that is 2 * 16 eV.
 
  const G4double trs  = db->fetchDouble("treshold") * eV;

  TPCSD01 *sensitiveDetector = new TPCSD01("TPC", trs, Control::TPCCut);
  RegisterSensitiveDetector(sensitiveDetector);

  // possible user limits which can be assigned to the logical TPC volume (from G4UserLimits.hh)
  // if database entries are NULL, then use the same default values as in the G4UserLimits constructor

  const G4double step  = db->fetchDouble("step") * mm;

  const G4double maxStep  = step;// max allowed step size in this volume
  const G4double maxTrack = DBL_MAX*mm; // max total track length
  const G4double maxTime  = DBL_MAX*s; // max time
  const G4double minEkine = 0.0*eV;       // min kinetic energy  (only for charged particles)
  const G4double minRange = 0.0*mm;       // min remaining range (only for charged particles)
  G4UserLimits *userLimits = new G4UserLimits(maxStep, maxTrack, maxTime, minEkine, minRange);

  const G4double radius  = db->fetchDouble("radius") * mm;
  const G4double z_half  = db->fetchDouble("length") * mm;
  G4Tubs *sensitiveSolid = new G4Tubs("TPCSensitiveSolid", 0.0, radius, z_half, phi1, phi2);

  G4LogicalVolume *sensitiveLog = new G4LogicalVolume(sensitiveSolid, materialGas, 
						      "TPCSensitiveLog", 0, sensitiveDetector, userLimits);
  sensitiveLog->SetVisAttributes(G4VisAttributes::Invisible);
 
  const G4double z_shift  = db->fetchDouble("shift") * mm;

  new G4PVPlacement(0, G4ThreeVector(0., 0., z_shift),  sensitiveLog,"TPC",WorldLog,false,0);

  delete db; 
  G4cout << "MidiTPC done.\n" << G4endl;
  return true;
}
