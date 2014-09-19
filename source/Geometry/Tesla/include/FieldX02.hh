// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: FieldX02.hh,v 1.1 2008/12/05 17:19:36 mora Exp $
// $Name: mokka-07-00 $

#ifndef FieldX02_hh
#define FieldX02_hh 1

#include "G4MagneticField.hh"
#include "VSubDetectorDriver.hh"

#include <vector>
#include <map>

class CGAGeometryEnvironment;
class G4LogicalVolume;
class FieldX02VMap;

class FieldX02: public VSubDetectorDriver, public G4MagneticField
{
public:
  FieldX02(void): VSubDetectorDriver("fieldX02", "field") {};
  ~FieldX02(void) {};

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
    FieldX02VMap *fieldMap;
  } TFieldRegion;
  
  typedef std::vector<TFieldRegion> TFieldStorage;
  typedef std::map<G4String, G4double> TReferenceMap;

private:
  TFieldStorage _fieldStorage;
  G4double _crossingAngle;
};

#define MAP_BINS 1001 // or more, if you like
#define MAP_STEP (1.0 * cm) // as of August 2005

class FieldX02VMap
{
public:
  FieldX02VMap(G4String tableName, G4double theModelBFactor);
  virtual ~FieldX02VMap(void);
  
  virtual void GetFieldValue(const G4ThreeVector &pos, G4ThreeVector &value) = 0;
  
protected:
  G4float _fieldMap[MAP_BINS];
};

class FieldX02MapSolenoid: public FieldX02VMap
{
public:
  FieldX02MapSolenoid(G4String tableName, G4double theModelBFactor): FieldX02VMap(tableName,theModelBFactor) {}
  void GetFieldValue(const G4ThreeVector &pos, G4ThreeVector &value);
};

class FieldX02MapDID: public FieldX02VMap
{
public:
  FieldX02MapDID(G4String tableName, G4double theModelBFactor): FieldX02VMap(tableName,theModelBFactor) {}
  void GetFieldValue(const G4ThreeVector &pos, G4ThreeVector &value);
};

#endif // FieldX02_hh
