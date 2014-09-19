#ifndef WSCALSD_H
#define WSCALSD_H 1
/* This is the sensitive detector class for the workshop calorimeter */
/* Author Roman Poeschl DESY Hamburg */
/* Dec. 2004 */

//Some Mokka includes
// module enums
#include "Control.hh"
//base class for this class
#include "VSensitiveDetector.hh"

//G4includes
#include "G4SDManager.hh"
#include "G4TouchableHandle.hh"
#include "G4StepPoint.hh"
#include "G4VPhysicalVolume.hh"
#include "G4NavigationHistory.hh"
#include "G4TouchableHistory.hh"
#include "G4PVReplica.hh"

//include file to enable a reasonable representation
//of the cellID in the LCIO output file
#include "CalHit.hh"




typedef G4THitsCollection<CalHit> WSHitsCollection;
typedef std::map<G4int,CalHit*> WSHitMap;
typedef WSHitMap::iterator WSHitMapIterator;
typedef std::pair<G4int,CalHit*> WSHitMapPair;






class wscalSD : public VSensitiveDetector {


public:

  wscalSD(G4String sdname,   // name of SD
	  G4double gridDim,  // grid dimension XY
	  G4int nCellX,      // n cells along X
	  G4int nCellY,      // n cells along Y
	  G4int modID);      // ID of module: TBHCAL or TBCATCHER
  
  ~wscalSD();

inline G4String GetName() const { return SDName; }

  // impl. of VSensitiveDetector virtual methods
  void Initialize(G4HCofThisEvent* HCE);
  void EndOfEvent(G4HCofThisEvent* HCE);
  G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);
  //

  // Set methods for SD primary parameters
  void SetNCellXY(G4int nx, G4int ny);
  void SetCellDim(G4double c);
  void SetModuleID(G4int m); 
  //

private:

  // find if hit exists already and ++edep if it does
  G4bool FindHit(G4int, G4double, G4int pdg , G4double time);

  // hits collection id
  G4int HCID;

  // name of SD
  G4String SDName;  

  // ID of module = WSCALO
    G4int moduleID;

  // hits collection
  WSHitsCollection *hitsColl;

  // map of hits
  WSHitMap *hitMap;

  // origin point from transformations = 0, 0, 0
  G4ThreeVector origin;

  // num cells in xy-directions
  G4int ncell_xy[2];

    // grid size
  G4double cellDim;

};
#endif
