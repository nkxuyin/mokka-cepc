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
// $Id: SD03.hh,v 1.4 2006/03/01 14:13:30 musat Exp $
// $Name: mokka-07-00 $
//

#ifndef SD03_h
#define SD03_h 1

#include "CalHit.hh"
#include "VSensitiveDetector.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"

#define MAX_STAVES 8
#define MAX_LAYERS 40
#define MAX_MODULES 7
#define ENDCAP_SD_PLATE_FLAG MAX_LAYERS+1

class LayerRef {
public:
  LayerRef (G4double X,G4double Y,G4double Z) :
    X0(X),Y0(Y),Z0(Z) {}
  ~LayerRef() {}
  G4double X0,Y0,Z0;
};

class SD03 : public VSensitiveDetector
{
  
public:
  SD03(G4double Idim,G4double Jdim, G4double Thickness,
     G4double guard_ring_size, G4double inter_wafer_gap,
     G4int nmax_cell_x, G4int nmax_cell_z, G4int n_guard_ring_zones,
     G4int Piece,G4String SDname,G4bool useID1 = false);
  virtual ~SD03();
  
  void Initialize(G4HCofThisEvent*HCE);
  G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
  G4ThreeVector GetCellCenter(G4int pP,G4int pS,G4int pM,
		  		G4int pI,G4int pJ,G4int pK);
  cell_ids GetCellIndex(double X, double Y, double Z,
            int & flag, double xDir=0, double yDir=0, double zDir=1);
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
  
protected:
  void GetCellIndices(const G4VTouchable * theTouchable,
#ifdef MOKKA_DEBUG
		G4ThreeVector& thePosition, 
#endif
		G4int& theSDPiece, G4int& theStave, 
                G4int & theModule, G4int & I, G4int & J, G4int & theLayer);

  void GetNearestCell(const G4VTouchable * theTouchable,
		G4ThreeVector thePosition,
		G4int& P, G4int& S, G4int& M, G4int& I, G4int& J, G4int& K,
		G4int & zone);


  HitsCollection *CalCollection;

  G4RotationMatrix * StavesRotationMatrices [MAX_STAVES];
  G4RotationMatrix * InverseStavesRotationMatrices [MAX_STAVES];

  G4double * StavesPhirots [MAX_STAVES];
  G4double * ModulesZOffsets [MAX_MODULES];

  LayerRef* Layers [MAX_LAYERS] ;
  G4ThreeVector CellDim;
 
  G4int SDPiece;
  G4int HCID;
  G4double theGuardRingSize, theInterWaferGap;
  G4int theNMaxCellX, theNMaxCellZ, theNGuardRingZones;
};

#endif

