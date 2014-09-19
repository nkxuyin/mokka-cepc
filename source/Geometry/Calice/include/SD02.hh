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
// $Id: SD02.hh,v 1.2 2004/07/26 11:37:23 musat Exp $
// $Name: mokka-07-00 $
//

#ifndef SD02_h
#define SD02_h 1

#include "CalHit.hh"
#include "VSensitiveDetector.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"

#define MAX_STAVES 8
#define MAX_LAYERS 40
#define MAX_MODULES 7
#define ENDCAP_SD_PLATE_FLAG MAX_LAYERS+1

class SD02LayerRef {
public:
  SD02LayerRef (G4double X,G4double Y,G4double Z) :
    X0(X),Y0(Y),Z0(Z) {}
  ~SD02LayerRef() {}
  G4double X0,Y0,Z0;
};

class SD02 : public VSensitiveDetector
{
  
public:
  SD02(G4double Idim,G4double Jdim,G4double radius, G4double Thickness,
     G4int Piece,G4String SD02name, G4bool divideFlag);
  virtual ~SD02();
  
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

  void SetModuleRotationMatrix(G4double thetarot);
  void SetStaveRotationMatrix(G4int staveNumber, G4double phirot);
  void AddLayer(G4int layerNumber, G4double X,G4double Y,G4double Z);
  void AddLayerOffset(G4int layerNumber, G4double X,G4double Y,G4double Z);
  void SetModuleZOffset(G4int moduleNumber,G4double Zoff);
  void TouchCell(G4int theSDPiece, G4int theStave, G4int theModule,
		  G4int I, G4int J, G4int theLayer, G4ThreeVector* thePosition,
		  G4Step *aStep, G4double edep);

  HitsCollection *CalCollection;

  G4RotationMatrix * StavesRotationMatrices [MAX_STAVES];
  G4RotationMatrix * InverseStavesRotationMatrices [MAX_STAVES];
  G4RotationMatrix * ModuleRotationMatrix;
  G4RotationMatrix * InverseModuleRotationMatrix;

  G4double * StavesPhirots [MAX_STAVES];
  G4double * ModulesZOffsets [MAX_MODULES];

  SD02LayerRef* Layers [MAX_LAYERS] ;
  G4ThreeVector CellDim;
  G4double theRadius;
 
  G4int SDPiece;
  G4int HCID;
  G4bool divide;
};

#endif

