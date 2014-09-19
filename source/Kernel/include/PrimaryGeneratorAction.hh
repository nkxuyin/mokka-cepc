// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: PrimaryGeneratorAction.hh,v 1.7 2007/06/21 16:14:45 musat Exp $
// $Name: mokka-07-00 $

#ifndef PrimaryGeneratorAction_hh
#define PrimaryGeneratorAction_hh 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

class PrimaryGeneratorMessenger;
class VPrimaryGenerator;
class G4Event;

class PrimaryGeneratorAction: public G4VUserPrimaryGeneratorAction
{
public:
  PrimaryGeneratorAction(void);
  ~PrimaryGeneratorAction(void);

  void GeneratePrimaries(G4Event *evt);
  void SetGeneratorWithName(G4String generatorName);



  VPrimaryGenerator* GetPrimaryGenerator(void) { return fPrimaryGenerator; }

private:
  void ApplyLorentzTransformation(G4Event *evt);

  PrimaryGeneratorMessenger *fMessenger;
  VPrimaryGenerator *fPrimaryGenerator;

};

#endif
