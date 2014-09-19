#ifndef TBSD_VCell_h
#define TBSD_VCell_h 1

#include "TBCellReadout.hh"

#include "VSensitiveDetector.hh"
#include "CalHit.hh"
#include "G4TouchableHistory.hh"

typedef G4THitsCollection<CalHit> TBHitsCollection;

// hits map = temporary solution to avoid index calculations
typedef std::map<G4ThreeVector,CalHit*> TBHitMap;
typedef TBHitMap::iterator TBHitMapIterator;
typedef std::pair<G4ThreeVector,CalHit*> TBHitMapPair;

class TBSD_VCell : public VSensitiveDetector
{
public:

  TBSD_VCell(G4String sdname, G4double cx, G4double cz, G4double calhx, G4double calhz);
  ~TBSD_VCell();

  void Initialize(G4HCofThisEvent* HCE);
  void EndOfEvent(G4HCofThisEvent* HCE);
  G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);

  inline G4String GetName() const { return SDName;}

private:
  G4bool FindHit(G4ThreeVector cellPos, G4double edep, G4int PDG, G4double time );
  
  G4int HCID;
  G4String SDName;  
  
  TBHitsCollection *hitsColl;
  TBHitMap *hitMap;

  G4ThreeVector origin;

  G4int moduleID;
  G4int depthToLayer;

  TBCellReadout cellRO;
  G4double cell_x, cell_z;  
  G4double cal_hx, cal_hz;
};

#endif
