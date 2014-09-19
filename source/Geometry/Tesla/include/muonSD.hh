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
// $Id: muonSD.hh,v 1.3 2008/10/21 15:09:58 musat Exp $
// $Name: mokka-07-00 $
//

#ifndef muonSD_h
#define muonSD_h 1

#include "CalHit.hh"
#include "VSensitiveDetector.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"

#define MAX_STAVES_MUON 16
#define MAX_LAYERS_MUON 40
#define MAX_MODULES 7
#define ENDCAP_muonSD_PLATE_FLAG MAX_LAYERS_MUON+1

class muLayerRef {
public:
  muLayerRef (G4double X,G4double Y,G4double Z) :
    X0(X),Y0(Y),Z0(Z) {}
  ~muLayerRef() {}
  G4double X0,Y0,Z0;
};

class muonSD : public VSensitiveDetector
{
  
public:
  muonSD(G4double Idim,G4double Jdim, G4double Thickness,
     G4int Piece,G4String SDname,G4bool id1Flag = false);
  virtual ~muonSD();
  
  void Initialize(G4HCofThisEvent*HCE);
  G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
  G4ThreeVector GetCellCenter(G4int pP,G4int pS,G4int pM,
		  		G4int pI,G4int pJ,G4int pK);
  void EndOfEvent(G4HCofThisEvent*HCE);
  void clear();
  void DrawAll();
  void PrintAll();
  std::vector<bool>saveHitMomentum;
  // Basic reload hits from Ascii Files
  void LoadEvent(FILE* theSubDetectorEventHitsFileInput);
  void SetSymmetry(G4int symmetry);
  void SetInnerbox(G4double inbox);
  void SetStaveRotationMatrix(G4int staveNumber, G4double phirot);
  void AddLayer(G4int layerNumber, G4double X,G4double Y,G4double Z);
  void AddLayerOffset(G4int layerNumber, G4double X,G4double Y,G4double Z);
  void SetModuleZOffset(G4int moduleNumber,G4double Zoff);
  
  HitsCollection *CalCollection;

  G4RotationMatrix * StavesRotationMatrices [MAX_STAVES_MUON];
  G4RotationMatrix * InverseStavesRotationMatrices [MAX_STAVES_MUON];

  G4double * StavesPhirots [MAX_STAVES_MUON];
  G4double * ModulesZOffsets [MAX_MODULES];

  muLayerRef* Layers [MAX_LAYERS_MUON] ;
  muLayerRef* Layers2 [MAX_LAYERS_MUON] ;
  G4ThreeVector CellDim;
 
  G4int Symmetry;
  G4double Inner_box;




  G4int SDPiece;
  G4int HCID;
};

#endif

