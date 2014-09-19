#ifndef TBSDVCellscecal01_h
#define TBSDVCellscecal01_h 1

#include "TBCellReadout.hh"

#include "VSensitiveDetector.hh"
// CRP Use CellHit class defined for the protoype, i.e. Proto_CellHit.hh
// later typedefs are defined accordingly
//#include "CellHit.hh"
#include "CalHit.hh"
#include "G4TouchableHistory.hh"

//include class VSubdetectorDriver in order to have access to the 
//parameters of the detector
#include "VSubDetectorDriver.hh"
//#include "TBhcal02.hh"
//#include "TBecal02.hh"
//#include "TBcatcher02.hh"

#define NROWSTRIP 4
#define NCOLOMN 18

//typedef G4THitsCollection<CellHit> TBHitsCollection;
typedef G4THitsCollection<CalHit> TBHitsCollection;

// hits map = temporary solution to avoid index calculations
//typedef std::map<G4int,CellHit*> TBHitMap;
typedef std::map<G4int,CalHit*> TBHitMap;
typedef TBHitMap::iterator TBHitMapIterator;
//typedef std::pair<G4int,CellHit*> TBHitMapPair;
typedef std::pair<G4int,CalHit*> TBHitMapPair;

//class TBhcal02;
//class TBecal02;
//class TBcatcher02;

class TBSDVCellscecal01 : public VSensitiveDetector
{
public:

  // This should take:
  // sdname, vsubdriver, grid_size, cell_xz pntr, sd enum
  // G4String sdname, G4double &grid_ptr, G4double &cellxz_ptr, G4int modid
  // (removes dep on subdet driver classes)
  TBSDVCellscecal01(G4String sdname, VSubDetectorDriver*);
  ~TBSDVCellscecal01();

  // CRP define required realization of virtual methods 
  //     in G4VSensitiveDetector.hh (or) G4SDManager.hh"
  void Initialize(G4HCofThisEvent* HCE);
  void EndOfEvent(G4HCofThisEvent* HCE);
  G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);

  inline G4String GetName() const { return SDName;}

private:
  G4bool FindHit(G4int, G4double, G4int pdg , G4double time);
  //CRP Declare the routine which set cell coordinates
  void SetCellCoordinates( G4float x, G4float y, G4int n_lay);
  //CRP Declare the routine which calculate Position of virtual cell
  void SetCellPosition( G4int );
  //CRP Rearranged due to compiler warning
  G4int HCID;
  G4String SDName;  
  G4int moduleID;
  G4int depthToLayer;

  TBHitsCollection *hitsColl;
  TBHitMap *hitMap;

  G4ThreeVector origin;
  //CRP Number of Cells in xz-direction
  const G4int* ncell_xz;
  //CRP Gridsize
  const G4double* grid_sizePtr;
  G4double cell_x, cell_z;  
  G4double cal_hx, cal_hz;
  //CRP Array for CellIds 
  G4int cellID[3];
  G4double cellPos[3];

//TODO to let this be a parameter
  G4int _nstrip_xz[2]; // = {NCOLOMN,NROWSTRIP};

};

#endif



