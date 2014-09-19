#ifndef BeamCal00_hh
#define BeamCal00_hh 1
/* 
 * First implementation of BeamCal detector 
 * A.Hartin
 * Oct. 2008 */

class G4LogicalVolume;
class Database;

#include "VSubDetectorDriver.hh"
#include "G4Material.hh"

class BeamCal00 : public VSubDetectorDriver
{
public:
  BeamCal00(void): VSubDetectorDriver("BeamCal00", "BeamCal") {}
  ~BeamCal00(void) {}

  G4bool ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog);

//private:
	G4double tiltForw, tiltBack;
	G4double SegmdR;
	G4double SegmnRs;
	G4double SegmdPhi[256];
	G4double SegmdPhiDA[256];
	G4double SegmnPhis[256];
	G4double SegmnPhisDA[256];
	G4double dR, r, DArinner, DAStart;
        G4int nRs,j;

	G4double zCenter;
	G4ThreeVector posBack, posForw;
	G4double dLayer, length;


	G4Material *materialAir;
	G4Material *materialDiamond;
	G4Material *materialTungsten;
	G4Material *materialGold;
	G4Material *materialKapton;
	G4Material *materialGraphite;

	G4double posSens, pos_sensBack;
	G4double posAbs, pos_absBack;
	G4double posElectrode, pos_electrodeBack;
	G4double posPCB, pcbBack;


};


  

#endif
