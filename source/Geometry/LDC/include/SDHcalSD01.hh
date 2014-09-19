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
// $Id: SDHcalSD01.hh,v 1.3 2008/12/03 13:54:33 musat Exp $
// $Name: mokka-07-00 $
//

#ifndef SDHcalSD01_h
#define SDHcalSD01_h 1

#include "CalHit.hh"
#include "VSensitiveDetector.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"

#define DHCAL_MAX_STAVES 8
#define DHCAL_MAX_LAYERS 48
#define DHCAL_MAX_MODULES 5

class DhcalLayerRef {
public:
  DhcalLayerRef (G4double X,G4double Y, G4double hY, G4double hZ) :
    X0(X),Y0(Y),hY0(hY),hZ0(hZ){}
  ~DhcalLayerRef() {}
  G4double X0,Y0,hY0,hZ0;
};

class SDHcalSD01 : public VSensitiveDetector
{
  
public:
  SDHcalSD01(G4double Idim,G4double Jdim, G4double Thickness,
     G4int Piece,G4String SDname,G4double PadSeparation,G4bool id1Flag = false);
  virtual ~SDHcalSD01();
  
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

  void SetStaveRotationMatrix(G4int staveNumber, G4RotationMatrix*  rot);
  void AddLayer(G4int layerNumber, G4double X,G4double Y,G4double hY,G4double hZ);
  void SetModuleZOffset(G4int moduleNumber,G4double Zoff);
  
  HitsCollection *CalCollection;

  G4RotationMatrix * StavesRotationMatrices [DHCAL_MAX_STAVES];
  G4RotationMatrix * InverseStavesRotationMatrices [DHCAL_MAX_STAVES];

  G4double * StavesPhirots [DHCAL_MAX_STAVES];
  G4double * ModulesZOffsets [DHCAL_MAX_MODULES];

  DhcalLayerRef* Layers [DHCAL_MAX_LAYERS] ;
  G4ThreeVector CellDim;

  G4int SDPiece;
  G4double PadSpacing;
  G4int HCID;
  
};

#endif

