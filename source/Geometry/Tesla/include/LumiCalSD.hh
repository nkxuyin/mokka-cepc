/*
$Id$
$Name$
*/
/* This is the sensitive detector class for the LumiCal */
/* Author Bogdan Pawlik INP PAS Krakow */
/* initial version                            Feb. 2005 */
/* handling of virtual cells implemented  B.P Feb. 2010 */
#ifndef LUMICALSD_H
#define LUMICALSD_H 1

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




typedef G4THitsCollection<CalHit> LCHitsCollection;
typedef std::map<G4int,CalHit*> LCHitMap;
typedef LCHitMap::iterator LCHitMapIterator;
typedef std::pair<G4int,CalHit*> LCHitMapPair;






class LumiCalSD : public VSensitiveDetector {


public:

  LumiCalSD(G4String sdname,     // name of SD
	    G4String LumiCal_Type, // type of design ( pads or strips )
	    G4double CalRhoMin,  // LumiCal inner radius
	    G4double PhiOffset,  // phi angle offset
	    G4double gridRho,    // grid dimension along radius
	    G4double gridPhi,    // grid dimension along phi angle
	    G4int nCellRho,      // n cells along radius
	    G4int nCellPhi,      // n cells along phi
	    G4int modID);        // module  ID
  // overloaded for virtual cell
 
  LumiCalSD(G4String sdname,     
	    G4String LumiCal_Type,
	    G4double CalRhoMin, G4double PhiOffset,  
	    G4double gridRho,   G4double gridPhi,   
	    G4int nCellRho,     G4int nCellPhi,     
	    G4int modID,
	    G4bool VirtualCell);        
  
  ~LumiCalSD();

inline G4String GetName() const { return SDName; }

  // impl. of VSensitiveDetector virtual methods
  void Initialize(G4HCofThisEvent* HCE);
  void EndOfEvent(G4HCofThisEvent* HCE);
  G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);
  //


private:

  // Set methods for SD primary parameters
  void SetNCellPhi(G4int nx);
  void SetNCellRho( G4int ny);
  void SetRhoCellDim(G4double c1);
  void SetPhiCellDim( G4double c2);
  void SetModuleID(G4int m);
  void SetRhoMin(G4double c);
  void SetPhiOffset(G4double phi); 
  //
  // find if hit exists already and ++edep if it does
  G4bool FindHit(G4int, G4double, G4int pdg , G4double time);

  // hits collection id
  G4int HCID;

  // name of SD
  G4String SDName;  

  // ID of module = LUMICALO
    G4int moduleID;

  // hits collection
  LCHitsCollection *hitsColl;

  // map of hits
  LCHitMap *hitMap;

  // origin point from transformations = 0, 0, 0
  G4ThreeVector origin;

  // Type of design
  G4String LumiCal_Type;

  // Virtual cells mode
  G4bool VirtualCell;

  // num cells in rho/phi -directions
  G4int NstripRho, NstripPhi;

    // grid size
  G4double cellDimRho, cellDimPhi;

    // inner radius and phi offset size
  G4double CalRhoMin, PhiOffset;

};
#endif
