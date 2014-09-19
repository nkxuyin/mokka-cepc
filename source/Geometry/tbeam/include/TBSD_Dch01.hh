// $Header: /home/flc/cvs/Mokka/source/Geometry/tbeam/include/TBSD_Dch01.hh,v 1.3 2006/03/02 22:02:49 fabrizio Exp $

#ifndef TBSD_Dch01_h
#define TBSD_Dch01_h 1

#include "TRKHit.hh"

#include "VSensitiveDetector.hh"
#include "G4TouchableHistory.hh"

typedef G4THitsCollection<TRKHit> TBHitsCollection;

class TBSD_Dch01 : public VSensitiveDetector
{

public:

  TBSD_Dch01(G4String sdname, G4double eThreshold);   // name of SD

  ~TBSD_Dch01();

public:

  inline G4String GetName() const { return SDName; }

  // impl. of VSensitiveDetector public methods
  void Initialize(G4HCofThisEvent* HCE);
  void EndOfEvent(G4HCofThisEvent* HCE);
  G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);
  //

private:

  // hits collection id
  G4int HCID;

  // name of SD
  G4String SDName;  

  // Energy threshold to accept a hit
  G4double hits_eThreshold;

  // hits collection
  TBHitsCollection *hitsColl;

  // origin point from transformations = 0, 0, 0
  G4ThreeVector origin;

};

#endif
