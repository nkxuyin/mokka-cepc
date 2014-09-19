// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: Yoke01.hh,v 1.2 2006/02/14 15:28:25 mora Exp $
// $Name: mokka-07-00 $

#ifndef Yoke01_hh
#define Yoke01_hh 1

class CGAGeometryEnvironment;
class G4LogicalVolume;

#include "VSubDetectorDriver.hh"

class Yoke01: public VSubDetectorDriver
{
public:
  Yoke01(): VSubDetectorDriver("yoke01","yoke") {}
  ~Yoke01() {}
  
  G4bool ContextualConstruct(const CGAGeometryEnvironment &geometryEnv, G4LogicalVolume *worldLog);
};

#endif
