// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: TubeX00.hh,v 1.3 2007/08/13 08:12:42 kristian Exp $
// $Name: mokka-07-00 $
//
// History:
// - first implementation of Tube00: P. Mora de Freitas (Sept 02)
// - modified from Tube00 to Tube01DT: Ties Behnke, 11-2-2003
// - modified for a crossing angle as TubeX00: Adrian Vogel, 2005-05-18

#ifndef TubeX00_hh
#define TubeX00_hh 1

class CGAGeometryEnvironment;
class G4LogicalVolume;

#include "VSubDetectorDriver.hh"
#include <string>

class TubeX00: public VSubDetectorDriver
{
public:
  TubeX00(): VSubDetectorDriver("tubeX00","tube") {}
  ~TubeX00() {}

  G4bool ContextualConstruct(const CGAGeometryEnvironment &geometryEnv, G4LogicalVolume *worldLog);

#ifdef MOKKA_GEAR
  void GearSetup();
#endif

private:
  typedef enum {          // These constants are also used in the MySQL database:
    kCenter = 0,          // centered on the z-axis
    kUpstream = 1,        // on the upstream branch, rotated by half the crossing angle
    kDnstream = 2,        // on the downstream branch, rotated by half the crossing angle
    kPunched = 3,         // centered, with two inner holes instead of one
    kUpstreamClipped = 4, // upstream, with one face parallel to the xy-plane
    kDnstreamClipped = 5  // downstream, with one face parallel to the xy-plane
  } ECrossType;
  
  std::string material;
  G4double beam_inner_radius;
  G4double beam_thickness;
  G4double beamPipe_zHalf;
  };

#endif // TubeX00_hh
