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
// $Id: SD.hh,v 1.8 2009/02/27 14:45:57 musat Exp $
// $Name: mokka-07-00 $
//

#ifndef SD_h
#define SD_h 1

#include "CalHit.hh"
#include "VSensitiveDetector.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"

#if G4_VERSION_GE( 920 )
#include "G4EmSaturation.hh"
#else
#include "../../Kernel/G4EmSaturation.hh"
#endif

#define MAX_STAVES 8
#define MAX_LAYERS 100
#define MAX_MODULES 7
#define ENDCAP_SD_PLATE_FLAG MAX_LAYERS+1

class LayerRef {
public:
  LayerRef (G4double X,G4double Y,G4double Z) :
    X0(X),Y0(Y),Z0(Z) {}
  ~LayerRef() {}
  G4double X0,Y0,Z0;
};

class SD : public VSensitiveDetector
{
  
public:
  SD(G4double Idim,G4double Jdim, G4double Thickness,
     G4int Piece,G4String SDname,G4bool id1Flag = false);
  virtual ~SD();
  
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
  
  HitsCollection *CalCollection;

  G4RotationMatrix * StavesRotationMatrices [MAX_STAVES];
  G4RotationMatrix * InverseStavesRotationMatrices [MAX_STAVES];

  G4double * StavesPhirots [MAX_STAVES];
  G4double * ModulesZOffsets [MAX_MODULES];

  LayerRef* Layers [MAX_LAYERS] ;
  G4ThreeVector CellDim;
 
  G4int SDPiece;
  G4int HCID;

  G4EmSaturation *emSaturation;
  /**Apply Birks attenuation law (example given by Vladimir Ivantchenko, from CERN)*/
  G4double BirkAttenuation(const G4Step* aStep);


};

#endif

