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
// $Id: MyPlacement.hh,v 1.3 2008/10/21 15:38:42 engels Exp $
// $Name: mokka-07-00 $
//
#ifndef MYPLACEMENT_h
#define MYPLACEMENT_h 1

#include "G4Material.hh"
#include "globals.hh"
#include "G4PVPlacement.hh"

#include "G4VSolid.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4Tubs.hh"
#include "G4Torus.hh"
#include "G4Trap.hh"
#include "G4Trd.hh"
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"

#include <stdio.h>

class G4DisplacedSolid;
class SD;

#define MAX_LVS 1000
#define MAX_BOOL_COMP 5

#define MAX_SD 10

class Composant {
public:
  Composant() : theDisplacedSolid(0){}
  ~Composant() {}
  G4DisplacedSolid* theDisplacedSolid;
  G4String shapeName;
  G4int shapeNumber;
  G4double XH,YH,ZH; // max to take place
};

class MyBoolShape 
{
public:
  MyBoolShape() : nComp(0), envelope(0) {}
  ~MyBoolShape() {}
  G4VSolid* BuildEnvelopeShape();
  void FillEnvelope(G4int EnvelopeSolidNumber,G4bool IsSensitive);
  Composant composants [MAX_BOOL_COMP];
  G4int nComp;
  G4Box* envelope;
  G4ThreeVector deltaCenter;
};
  

typedef struct {
  G4LogicalVolume* LogVol;
  G4int nCopy;
  G4String G3GSName;
  G4int G3GSNumber;
  MyBoolShape* boolShape;
  SD* theSD;
} LV;


class MyPlacement : public G4PVPlacement
{
public:
  MyPlacement(G4RotationMatrix *pRot, 
	 const G4ThreeVector &tlate,
	 G4LogicalVolume *pCurrentLogical,
	 const G4String& pName,
	 G4LogicalVolume *pMotherLogical,
	 G4bool pMany,
	 G4int pCopyNo,
     G4bool pSurfCheck=false); 
  ~MyPlacement(){};
  
  static LV LVs [MAX_LVS];
  static G4int n_LVs;

  static G4int n_MATs;
  static G4int SolidNumber;
  static FILE * FVOLS;
  //  static FILE * FLAYERS;
  
  static void Open();
  static void Close();
  static void Init(G4String Detector, G4String database);
  static void DescribeRotation(G4RotationMatrix *pRot);

  static void InsertComment(const char* txt);

private:
  G4int DescribeLogical(G4LogicalVolume* LV);
  G4int DescribeSolid(G4VSolid* theSolid,
		      G4String MatName,
		      G4bool IsSensitive,
		      MyBoolShape* boolShape=0);
  void PlaceLogical(G4String pName, 
		    G4RotationMatrix *pRot,
		    const G4ThreeVector &tlate,
		    G4int pCopyNo,
		    G4int theMotherLV,
		    G4int theLV);
  G4int BoxParameters(G4VSolid* theSolid,MyBoolShape* boolShape=0);
  G4int TrdParameters(G4VSolid* theSolid,MyBoolShape* boolShape=0);
  G4int TrapParameters(G4VSolid* theSolid,MyBoolShape* boolShape=0);
  G4int TubParameters(G4VSolid* theSolid,MyBoolShape* boolShape=0);

  static void WriteCellMapping();

  static G4double ToDegrees(G4double angle);
};  

#endif


