// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: MidiTPC.hh,v 1.1 2007/02/09 15:57:48 predrag Exp $
// $Name: mokka-07-00 $

#ifndef MidiTPC_hh
#define MidiTPC_hh 1

#include "VSubDetectorDriver.hh"

class MidiTPC: public VSubDetectorDriver
{
public:
  MidiTPC(void): VSubDetectorDriver("MidiTPC", "tpc") {}
  ~MidiTPC(void) {}
  G4bool ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *WorldLog);

};

#endif
