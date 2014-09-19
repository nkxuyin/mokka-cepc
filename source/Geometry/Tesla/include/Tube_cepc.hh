// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: Tube_cepc.hh,v 1.2 2007/08/13 08:12:42 kristian Exp $
// $Name: mokka-07-00 $
//
// History:
// - first implementation as Tube00: Paulo Mora de Freitas, Sep 2002
// - modified from Tube00 to Tube01DT: Ties Behnke, 2003-02-11
// - modified for a crossing angle as TubeX00: Adrian Vogel, 2005-05-18
// - modified for fancier geometries as Tube_cepc: Adrian Vogel, 2006-04-20
// - modified gear parameters: Robin Glattauer, 2012-01-24

#ifndef Tube_cepc_hh
#define Tube_cepc_hh 1

class CGAGeometryEnvironment;
class G4LogicalVolume;

#include "VSubDetectorDriver.hh"
#include <map>
#include <string>

class Tube_cepc: public VSubDetectorDriver
{
public:
  Tube_cepc(void): VSubDetectorDriver("Tube_cepc", "tube") {}
  ~Tube_cepc(void) {}

  G4bool ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog);

#ifdef MOKKA_GEAR
    void GearSetup();
#endif

private:
  typedef enum {                // These constants are also used in the MySQL database:
    kCenter               =  0, // centered on the z-axis
    kUpstream             =  1, // on the upstream branch, rotated by half the crossing angle
    kDnstream             =  2, // on the downstream branch, rotated by half the crossing angle

    kPunchedCenter        =  3, // centered, with one or two inner holes
    kPunchedUpstream      =  4, // on the upstream branch, with two inner holes
    kPunchedDnstream      =  5, // on the downstrem branch, with two inner holes

    kUpstreamClippedFront =  6, // upstream, with the front face parallel to the xy-plane
    kDnstreamClippedFront =  7, // downstream, with the front face parallel to the xy-plane
    kUpstreamClippedRear  =  8, // upstream, with the rear face parallel to the xy-plane
    kDnstreamClippedRear  =  9, // downstream, with the rear face parallel to the xy-plane
    kUpstreamClippedBoth  = 10, // upstream, with both faces parallel to the xy-plane
    kDnstreamClippedBoth  = 11, // downstream, with both faces parallel to the xy-plane

    kUpstreamSlicedFront  = 12, // upstream, with the front face parallel to a tilted piece
    kDnstreamSlicedFront  = 13, // downstream, with the front face parallel to a tilted piece
    kUpstreamSlicedRear   = 14, // upstream, with the rear face parallel to a tilted piece
    kDnstreamSlicedRear   = 15, // downstream, with the rear face parallel to a tilted piece
    kUpstreamSlicedBoth   = 16, // upstream, with both faces parallel to a tilted piece
    kDnstreamSlicedBoth   = 17  // downstream, with both faces parallel to a tilted piece
  } ECrossType;

  std::string material;
  G4double beam_inner_radius;
  G4double beam_thickness;
  G4double zHalf;
  typedef std::map<G4String, G4double> TReferenceMap;
  
  // For gear: the beampipe is assumed to be made of cone frustums (cones with the spikey bit cut off), 
  // that are connected to each other. 
  // So the radius of the end of one bit is supposed to be the radius of the
  // start of the next one. RStart[i] = REnd[i-1]
  std::vector<double> gearValRInner;
  std::vector<double> gearValROuter;
  std::vector<double> gearValZ;
};

#endif // Tube_cepc_hh
