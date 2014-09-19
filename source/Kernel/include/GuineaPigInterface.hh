// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: GuineaPigInterface.hh,v 1.1 2005/09/29 16:26:55 adrian Exp $
// $Name: mokka-07-00 $

#ifndef GuineaPigInterface_hh
#define GuineaPigInterface_hh 1

#include "G4VPrimaryGenerator.hh"
#include <fstream>

class G4Event;

class GuineaPigInterface: public G4VPrimaryGenerator
{
public:
  GuineaPigInterface(G4String inputFilename);
  ~GuineaPigInterface(void);

  void GeneratePrimaryVertex(G4Event *event);

private:
  std::ifstream fInputFile;
};

#endif
