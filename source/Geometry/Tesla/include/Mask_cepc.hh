// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: Mask_cepc.hh,v 1.1 2006/04/25 17:41:26 adrian Exp $
// $Name: mokka-07-00 $
//
// History:
// - first implementation as LAT00: Peter Wienemann, Jun 2003
// - modified for a crossing angle as MaskX00: Adrian Vogel, 2005-05-19
// - modified for fancier geometries as Mask_cepc: Adrian Vogel, 2006-04-20

#ifndef Mask_cepc_hh
#define Mask_cepc_hh 1

class CGAGeometryEnvironment;
class G4LogicalVolume;

#include "VSubDetectorDriver.hh"
#include <map>

class Mask_cepc: public VSubDetectorDriver
{
public:
  Mask_cepc(void): VSubDetectorDriver("Mask_cepc", "mask") {}
  ~Mask_cepc(void) {}

  G4bool ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog);

private:
  typedef enum {          // These constants are also used in the MySQL database:
    kCenter          = 0, // centered on the z-axis
    kUpstream        = 1, // on the upstream branch, rotated by half the crossing angle
    kDnstream        = 2, // on the downstream branch, rotated by half the crossing angle
    kPunchedCenter   = 3, // centered, with one or two inner holes
    kPunchedUpstream = 4, // on the upstream branch, with two inner holes
    kPunchedDnstream = 5  // on the downstream branch, with two inner holes
  } ECrossType;
  
  typedef std::map<G4String, G4double> TReferenceMap;
};

#endif // Mask_cepc_hh
