#ifndef TBSD_h
#define TBSD_h 1

#include "VSensitiveDetector.hh"
#include "CalHit.hh"
#include "G4TouchableHistory.hh"

typedef G4THitsCollection<CalHit> TBHitsCollection;

// hits map = temporary solution to avoid index calculations
typedef std::map<G4ThreeVector,CalHit*> TBHitMap;
typedef TBHitMap::iterator TBHitMapIterator;
typedef std::pair<G4ThreeVector,CalHit*> TBHitMapPair;

class TBSD : public VSensitiveDetector
{
public:

  TBSD(G4String sdname);
  ~TBSD();

  void Initialize(G4HCofThisEvent* HCE);
  void EndOfEvent(G4HCofThisEvent* HCE);
  G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);

  inline G4String GetName() const { return SDName;}

private:
  G4bool FindHit(G4ThreeVector cellPos, G4double edep, G4int PDG, G4double time);
  
  G4int HCID;
  G4String SDName;  
  
  TBHitsCollection *hitsColl;

  TBHitMap *hitMap;

  G4ThreeVector origin;
 
  G4int moduleID;
  G4int depthToLayer;
};

#endif
