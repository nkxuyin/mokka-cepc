// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: Yoke02.hh,v 1.2 2006/03/22 15:27:55 adrian Exp $
// $Name: mokka-07-00 $

#ifndef Yoke02_hh
#define Yoke02_hh 1

#include "VSubDetectorDriver.hh"

class Yoke02: public VSubDetectorDriver
{
public:
  Yoke02(void): VSubDetectorDriver("yoke02", "yoke") {}
  ~Yoke02(void) {}
  
  G4bool ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog);
};

#endif
