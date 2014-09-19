// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: GPSGenerator.hh,v 1.1 2007/06/21 16:14:45 musat Exp $
// $Name: mokka-07-00 $

#ifndef GPSGenerator_hh
#define GPSGenerator_hh 1

#include "G4ThreeVector.hh"
#include "globals.hh"

#include "VPrimaryGenerator.hh"

class G4GeneralParticleSource;
class G4Event;

class GPSGenerator: public VPrimaryGenerator
{
public:
  GPSGenerator(void);
  ~GPSGenerator(void);

  void GeneratePrimaryVertex(G4Event *evt);
  void PrintGeneratorInfo(void);

private:
  G4GeneralParticleSource *fGenerator;
};

#endif
