//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for Tesla -      *
//*                                                     *
//* For more information about Mokka, please, go to the *
//*                                                     *
//*  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
//*                                                     *
//* Mokka home page.                                    *
//*                                                     *
//*******************************************************
//
// $Id: ProtoSD.hh,v 1.2 2004/04/19 13:45:20 mora Exp $
// $Name: mokka-07-00 $
//

#ifndef ProtoSD_h
#define ProtoSD_h 1

#include "CalHit.hh"
#include "VSensitiveDetector.hh"
#include "G4Step.hh"

class ProtoSD : public VSensitiveDetector
{
  
public:
  ProtoSD(G4double dimX,G4double dimY,G4double dimZ,
	  G4int n_cell_x, G4int n_cell_z,
	  G4int aMultiplicity,G4int aNumberOfElements,
	  G4int aNumberOfTowers,G4String ProtoSDname);
  virtual ~ProtoSD();
  
  void Initialize(G4HCofThisEvent*HCE);
  G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
  void EndOfEvent(G4HCofThisEvent*HCE);
  void clear();
  void DrawAll();
  void PrintAll();

  // Basic reload hits from Ascii Files
  void LoadEvent(FILE* theSubDetectorEventHitsFileInput);

  HitsCollection *CalCollection;
  G4int HCID;

private:
  G4ThreeVector CellDim;
  G4int theNumberOfTowers,theNCell_x, 
    theNCell_z,theMultiplicity,theNumberOfElements;	    
};

#endif

