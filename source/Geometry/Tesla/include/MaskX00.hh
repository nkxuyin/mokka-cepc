// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: MaskX00.hh,v 1.2 2006/02/14 15:28:25 mora Exp $
// $Name: mokka-07-00 $
//
// History:
// - first implementation as LAT00: Peter Wienemann, Jun 2003
// - modified for a crossing angle as MaskX00: Adrian Vogel, 2005-05-19

#ifndef MaskX00_hh
#define MaskX00_hh 1

class CGAGeometryEnvironment;
class G4LogicalVolume;

#include "VSubDetectorDriver.hh"

class MaskX00: public VSubDetectorDriver
{
public:
  MaskX00(): VSubDetectorDriver("maskX00","mask") {}
  ~MaskX00() {}

  G4bool ContextualConstruct(const CGAGeometryEnvironment &geometryEnv, G4LogicalVolume *worldLog);

private:
  typedef enum {          // These constants are also used in the MySQL database:
    kCenter = 0,          // centered on the z-axis
    kUpstream = 1,        // on the upstream branch, rotated by half the crossing angle
    kDnstream = 2,        // on the downstream branch, rotated by half the crossing angle
    kPunched = 3,         // centered, with two inner holes instead of one
    kUpstreamClipped = 4, // upstream, with one face parallel to the xy-plane (not implemented yet)
    kDnstreamClipped = 5  // downstream, with one face parallel to the xy-plane (not implemented yet)
  } ECrossType;
};

#endif // MaskX00_hh
