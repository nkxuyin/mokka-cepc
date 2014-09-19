// $Header: /home/flc/cvs/Mokka/source/Geometry/tbeam/include/TBSD_VCell4d.hh,v 1.6 2009/02/27 14:45:57 musat Exp $

#ifndef TBSD_VCell4d_h
#define TBSD_VCell4d_h 1

#include "CalHit.hh"

#include "VSensitiveDetector.hh"
#include "G4TouchableHistory.hh"

#if G4_VERSION_GE( 920 )
#include "G4EmSaturation.hh"
#else
#include "../../Kernel/G4EmSaturation.hh"
#endif

typedef G4THitsCollection<CalHit> TBHitsCollection;

typedef std::map<G4int,CalHit*> TBHitMap;
typedef TBHitMap::iterator TBHitMapIterator;
typedef std::pair<G4int,CalHit*> TBHitMapPair;

class TBSD_VCell4d : public VSensitiveDetector
{

public:

  TBSD_VCell4d(G4String sdname,   // name of SD
	       G4double gridDim,  // grid dimension XY
	       G4int nCellX,      // n cells along X
	       G4int nCellY,      // n cells along Y
	       G4int dLayer,      // depth to layer from leaf volume
	       G4int modID,      // ID of module: TBHCAL or TBCATCHER
	       G4int applyBirksLawTemp=0, //flag to apply Birks law for HCAL
	       G4double hcalTimeCutTemp=0,//HCAL time cut (in ns)
	       G4double zBeginDetectorTemp=0);//temporary variable to store z-start of the HCAL detector

  ~TBSD_VCell4d();

public:

  inline G4String GetName() const { return SDName; }

  // impl. of VSensitiveDetector public methods
  void Initialize(G4HCofThisEvent* HCE);
  void EndOfEvent(G4HCofThisEvent* HCE);
  G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);
  //

  // Set methods for SD primary parameters
  void SetNCellXY(G4int nx, G4int ny);
  void SetGridDim(G4double g);
  void SetDepthToLayer(G4int d);
  void SetModuleID(G4int m); 
  //
  // Get methods for SD primary parameters
  G4int* GetNCellXY();
  G4double GetGridDim();
  G4int GetDepthToLayer();

  G4int GetApplyBirksLaw();
  G4double GetZBeginDetector();
  G4double GetHcalTimeCut();  
  //
private:

  // find if hit exists already and ++edep if it does
  G4bool FindHit(G4int, G4int, G4double, G4int pdg , G4double time, float* sp);

  // set the cell coordinate member var
  void SetCellID( G4float localPosX, G4float localPosY);

  // hits collection id
  G4int HCID;

  // name of SD
  G4String SDName;  

  // ID of module = TBHCAL, TBCATCHER
  G4int moduleID;

  // face dimension
  G4double xDim, yDim;

  // hits collection
  TBHitsCollection *hitsColl;

  // map of hits
  TBHitMap *hitMap;

  // origin point from transformations = 0, 0, 0
  G4ThreeVector origin;

  // num cells in XY directions
  G4int ncell_xy[2];

  // grid size
  G4double gridDim;

  // depth to the layer from leaf volume
  G4int depthToLayer;

  // array for CellIds 
  G4int cellID[3];

    /******************************************************************************/
   //z-start of the HCAL detector (in mm)
    G4double zBeginDetector;
    G4double hcalTimeCut;
    G4int applyBirksLaw;

    //G4EmSaturation implements Birks' law
    G4EmSaturation *emSaturation;
    //Get attenuated energy for densely ionising particles, due to Birks law
    G4double GetBirksAttenuatedEnergy(const G4Step* aStep);
    /******************************************************************************/


};

#endif






