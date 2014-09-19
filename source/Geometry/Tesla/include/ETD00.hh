// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: ETD00.hh,v 1.1 2006/03/16 17:47:36 adrian Exp $
// $Name: mokka-07-00 $

#ifndef ETD00_hh
#define ETD00_hh 1

#include "VSubDetectorDriver.hh"

class ETD00: public VSubDetectorDriver
{
public:
  ETD00(void): VSubDetectorDriver("etd00", "etd") {}
  ~ETD00(void) {}
  
  G4bool ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog);
};

#endif
