// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: InputFileGenerator.hh,v 1.1 2007/06/21 16:14:45 musat Exp $
// $Name: mokka-07-00 $

#ifndef InputFileGenerator_hh
#define InputFileGenerator_hh 1

#include "VPrimaryGenerator.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

class G4VPrimaryGenerator;
class G4Event;

class InputFileGenerator: public VPrimaryGenerator
{
public:
  InputFileGenerator(G4String);
  ~InputFileGenerator(void);

  void GeneratePrimaryVertex(G4Event *evt);
  void PrintGeneratorInfo(void);
  void SetupFileGenerator(G4String);

  bool AppliesLorentzTransform() ;

private:
  G4VPrimaryGenerator *fGenerator;
};

#endif
