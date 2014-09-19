// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: FieldX01.hh,v 1.2 2006/09/12 18:01:34 adrian Exp $
// $Name: mokka-07-00 $

#ifndef FieldX01_hh
#define FieldX01_hh 1

#include "G4MagneticField.hh"
#include "VSubDetectorDriver.hh"

#include <vector>
#include <map>

class CGAGeometryEnvironment;
class G4LogicalVolume;
class FieldX01VMap;

class FieldX01: public VSubDetectorDriver, public G4MagneticField
{
public:
  FieldX01(void): VSubDetectorDriver("fieldX01", "field") {};
  ~FieldX01(void) {};

  G4bool ContextualConstruct(const CGAGeometryEnvironment &geometryEnv, G4LogicalVolume *worldLog);
  void GetFieldValue(const double point[4], double *bField) const;

private:
  typedef enum {          // These constants are also used in the MySQL database:
    kCenterQuad = 0,      // ideal quadrupole, centered on the z-axis
    kUpstreamQuad = 1,    // ideal quadrupole, on the upstream branch
    kDnstreamQuad = 2,    // ideal quadrupole, on the downstream branch
    kSolenoid = 3,        // ideal solenoid
    kDID = 4,             // ideal dipole
    kMapSolenoid = 5,     // solenoid from a field map
    kMapDID = 6           // dipole from a field map
  } EFieldType;
  
  typedef struct {
    EFieldType fieldType;
    G4double zMin;
    G4double zMax;
    G4double rMinSqr;
    G4double rMaxSqr;
    G4double fieldValue;
    FieldX01VMap *fieldMap;
  } TFieldRegion;
  
  typedef std::vector<TFieldRegion> TFieldStorage;
  typedef std::map<G4String, G4double> TReferenceMap;

private:
  TFieldStorage _fieldStorage;
  G4double _crossingAngle;
};

#define MAP_BINS 1001 // or more, if you like
#define MAP_STEP (1.0 * cm) // as of August 2005

class FieldX01VMap
{
public:
  FieldX01VMap(G4String tableName);
  virtual ~FieldX01VMap(void);
  
  virtual void GetFieldValue(const G4ThreeVector &pos, G4ThreeVector &value) = 0;
  
protected:
  G4float _fieldMap[MAP_BINS];
};

class FieldX01MapSolenoid: public FieldX01VMap
{
public:
  FieldX01MapSolenoid(G4String tableName): FieldX01VMap(tableName) {}
  void GetFieldValue(const G4ThreeVector &pos, G4ThreeVector &value);
};

class FieldX01MapDID: public FieldX01VMap
{
public:
  FieldX01MapDID(G4String tableName): FieldX01VMap(tableName) {}
  void GetFieldValue(const G4ThreeVector &pos, G4ThreeVector &value);
};

#endif // FieldX01_hh
