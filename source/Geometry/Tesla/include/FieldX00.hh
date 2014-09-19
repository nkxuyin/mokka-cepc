// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: FieldX00.hh,v 1.1 2005/10/10 17:19:11 adrian Exp $
// $Name: mokka-07-00 $

#ifndef FieldX00_hh
#define FieldX00_hh 1

#include "G4MagneticField.hh"
#include "VSubDetectorDriver.hh"

#include <vector>

class CGAGeometryEnvironment;
class G4LogicalVolume;
class FieldX00Map;

class FieldX00: public VSubDetectorDriver, public G4MagneticField
{
public:
  FieldX00(): VSubDetectorDriver("fieldX00") {};
  ~FieldX00() {};

  G4bool ContextualConstruct(const CGAGeometryEnvironment &geometryEnv, G4LogicalVolume *worldLog);
  void GetFieldValue(const double point[4], double *bField) const;

private:
  typedef enum {          // These constants are also used in the MySQL database:
    kCenterQuad = 0,      // ideal quadrupole, centered on the z-axis
    kUpstreamQuad = 1,    // ideal quadrupole, on the upstream branch
    kDnstreamQuad = 2,    // ideal quadrupole, on the downstream branch
    kSolenoid = 3,        // ideal solenoid
    kSolenoidDID = 4,     // ideal solenoid with a superimposed ideal dipole
    kMapSolenoid = 5,     // solenoid from a field map
    kMapSolenoidDID = 6   // solenoid from a field map with a superimposed dipole
  } EFieldType;
  
  typedef struct {
    EFieldType fieldType;
    G4double zMin;
    G4double zMax;
    G4double rMinSqr;
    G4double rMaxSqr;
    G4double fieldValue;
    FieldX00Map *fieldMap;
  } TFieldRegion;
  
  typedef std::vector<TFieldRegion> TFieldStorage;

private:
  TFieldStorage fFieldStorage;
  G4double fCrossingAngle;
};

#define MAP_BINS 1001 // or more, if you like
#define MAP_STEP (1.0 * cm) // as of August 2005

class FieldX00Map
{
public:
  FieldX00Map(G4String databaseName, G4String tableName);
  ~FieldX00Map() {};
  
  void GetFieldValue(const G4ThreeVector &pos, G4ThreeVector &value, G4bool useDID);
  
private:
  G4float fFieldMapBz[MAP_BINS];
  G4float fFieldMapBx[MAP_BINS];
};

#endif // FieldX00_hh
