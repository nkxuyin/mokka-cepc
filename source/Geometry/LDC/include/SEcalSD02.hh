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
// $Id: SEcalSD02.hh,v 1.3 2009/06/04 11:40:42 musat Exp $
// $Name: mokka-07-00 $
//

#ifndef SEcalSD02_h
#define SEcalSD02_h 1

#include "CalHit.hh"
#include "VSensitiveDetector.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"

#define MAX_STAVES 8
#define MAX_LAYERS 100
#define MAX_MODULES 7

class LayerRef {
public:
  LayerRef (G4double X,G4double Y,G4double Z) :
    X0(X),Y0(Y),Z0(Z) {}
  ~LayerRef() {}
  G4double X0,Y0,Z0;
};

class SEcalSD02 : public VSensitiveDetector
{
  
public:
  SEcalSD02(G4double Idim, G4double Jdim, G4double Thickness,
	    G4int n_cells_i, G4int n_cells_j,
	    G4double GuardRingSize, G4double HWallSize,
	    G4double TowerWallSize,
	    G4int Piece,G4String SDname,
	    G4bool id1Flag = false,
	    G4String BarrelSlabMode = "0110",
	    G4String ECSlabMode = "0110");
  virtual ~SEcalSD02();
  
  void Initialize(G4HCofThisEvent*HCE);
  G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
  G4ThreeVector GetCellCenter(G4int pP,G4int pS,G4int pM,
		  		G4int pI,G4int pJ,G4int pK);
  void EndOfEvent(G4HCofThisEvent*HCE);
  void clear();
  void DrawAll();
  void PrintAll();

  // Basic reload hits from Ascii Files
  void LoadEvent(FILE* theSubDetectorEventHitsFileInput);

  void SetStaveRotationMatrix(G4int staveNumber, G4double phirot);
  void AddLayer(G4int layerNumber, G4double X,G4double Y,G4double Z);
  void AddLayerOffset(G4int layerNumber, G4double X,G4double Y,G4double Z);
  void SetModuleZOffset(G4int moduleNumber,G4double Zoff);
  void SetStandardXOffset(G4double Xoff)
  { StandardXOffset = Xoff; }
  
  HitsCollection *NormalCalCollection;
  HitsCollection *FirstLayerCalCollection;

  G4RotationMatrix * StavesRotationMatrices [MAX_STAVES];
  G4RotationMatrix * InverseStavesRotationMatrices [MAX_STAVES];

  G4double * StavesPhirots [MAX_STAVES];
  G4double * ModulesZOffsets [MAX_MODULES];
  G4double   StandardXOffset;

  LayerRef* Layers [MAX_LAYERS] ;
  G4ThreeVector CellDim;
  G4int n_cells_i, n_cells_j;
  G4double GuardRingSize, HWallSize, TowerWallSize;
  
  G4int SDPiece;
  G4int HCID1,HCID2;
  G4String BarrelSlabMode;
  G4String ECSlabMode;

  G4double total_wafer_size_x, total_wafer_size_z;
  G4double total_tower_size_z;

};

#endif

