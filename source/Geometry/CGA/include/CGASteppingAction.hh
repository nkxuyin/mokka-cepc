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
//*    Mokka home page.                                 *
//*                                                     *
//*******************************************************
//
// $Id: CGASteppingAction.hh,v 1.3 2008/10/14 16:27:17 musat Exp $
// $Name: mokka-07-00 $
//
// History
// first implementation for the 
// Mokka Common Geometry Access (CGA) by 
// Gabriel Musat (musat@poly.in2p3.fr), July 2002
//
// see CGA documentation at 
// http://polype.in2p3.fr/geant4/tesla/www/mokka/
//        software/doc/CGADoc/CGAIndex.html
//-------------------------------------------------------

#ifndef CGASteppingAction_h
#define CGASteppingAction_h 1

#include "SteppingAction.hh"
#include "G4UserSteppingAction.hh"
#include "G4ThreeVector.hh"
#include "G4TouchableHistory.hh"
#include "G4String.hh"
#include "globals.hh"
#include <vector>

typedef struct {
	G4String nomVol;
	G4String nomMat;
	G4double parcours;
	G4double nbX0;
	G4double nInterLen;
	G4ThreeVector preStepPoint;
	std::vector<G4String> logicals;
} CGAStep;

class CGASteppingAction : public SteppingAction
{
public:
  CGASteppingAction();
  virtual ~CGASteppingAction() { resetCGAVector(); };
  void UserSteppingAction(const G4Step* aStep);

private:
  static G4ThreeVector finalPoint;
  static G4ThreeVector startPoint;
  static G4String finalLVName;
  static std::vector<CGAStep*> volData;
  static G4double maxDist;

public:
  typedef std::vector<CGAStep*>::const_iterator iterator;
  static CGASteppingAction::iterator begin(void);
  static CGASteppingAction::iterator end(void);
  static void resetCGAVector(void);
  static void setEndPoints(G4double xi, 
	G4double yi, G4double zi, G4double xf, 
	G4double yf, G4double zf);
};

#endif
