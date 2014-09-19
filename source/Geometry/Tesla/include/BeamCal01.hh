#ifndef BeamCal01_hh
#define BeamCal01_hh 1
/* 
 * Second implementation of BeamCal detector based on the First implementation
 * A.Sailer
 * Apr. 2010 */

class G4LogicalVolume;
class Database;

#include "VSubDetectorDriver.hh"
#include "G4Material.hh"
#include <map>

class BeamCal01 : public VSubDetectorDriver
{
public:
  BeamCal01(void): VSubDetectorDriver("BeamCal01", "BeamCal") {}
  ~BeamCal01(void) {}

  G4bool ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog);

private:
  G4double tiltForw, tiltBack;
  G4double SegmdR;
  G4double SegmnRs;
  //  G4double SegmdPhi[256];
  // G4double SegmdPhiDA[256];
  // G4double SegmnPhis[256];
  // G4double SegmnPhisDA[256];

  //std::vector<double> SegmdPhi, SegmdPhiDA, SegmnPhis, SegmnPhisDA;
  G4double dR, r, DArinner, DAStart;
  G4int nRs;

  G4double zCenter;
  G4ThreeVector posBack, posForw;
  G4double dLayer, length;

  G4Material *materialAir;
  G4Material *materialDiamond;
  G4Material *materialTungsten;
  G4Material *materialGold;
  G4Material *materialKapton;
  G4Material *materialGraphite;
  G4Material *materialSilicon;

  G4double posSens, pos_sensBack;
  G4double posAbs, pos_absBack;
  G4double posElectrode, pos_electrodeBack;
  G4double posPCB, pcbBack;


  typedef std::map<G4String, G4double> BeamCalValueMap;

};


  

#endif
