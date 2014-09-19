// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//

/** A field driver for the ILD detector that uses a 2D field map 
 *  for the solenoid component - the other fields (anti-DID and 
 *  MDI focus magnets are taken from the previous driver FieldX02
 *  by A.Vogel and P. Mora de Freitas.
 * 
 * @version $Id: FieldX03.hh,v 1.1 2009/05/07 16:04:20 frank Exp $
 */

#ifndef FieldX03_hh
#define FieldX03_hh 1

#include "G4MagneticField.hh"
#include "VSubDetectorDriver.hh"

#include <vector>
#include <map>

class CGAGeometryEnvironment;
class G4LogicalVolume;
class FieldX03VMap;

class FieldX03: public VSubDetectorDriver, public G4MagneticField
{
public:
  FieldX03(void): VSubDetectorDriver("fieldX03", "field") {};
  ~FieldX03(void) {};

  G4bool ContextualConstruct(const CGAGeometryEnvironment &geometryEnv, G4LogicalVolume *worldLog);
  void GetFieldValue(const double point[4], double *bField) const;

private:
  typedef enum {          // These constants are also used in the MySQL database:
    kCenterQuad = 0,      // ideal quadrupole, centered on the z-axis
    kUpstreamQuad = 1,    // ideal quadrupole, on the upstream branch
    kDnstreamQuad = 2,    // ideal quadrupole, on the downstream branch
    kSolenoid = 3,        // ideal solenoid
    kDID = 4,             // ideal dipole
    kMapSolenoid = 5,     // solenoid from a 2D-field map
    kMapDID = 6           // dipole from a field map
  } EFieldType;
  
  typedef struct {
    EFieldType fieldType;
    G4double zMin;
    G4double zMax;
    G4double rMinSqr;
    G4double rMaxSqr;
    G4double fieldValue;
    FieldX03VMap *fieldMap;
  } TFieldRegion;
  
  typedef std::vector<TFieldRegion> TFieldStorage;
  typedef std::map<G4String, G4double> TReferenceMap;

private:
  TFieldStorage _fieldStorage;
  G4double _crossingAngle;
};



class FieldX03VMap {
public:
  virtual void GetFieldValue(const G4ThreeVector &pos, G4ThreeVector &value) = 0;
};

class FieldX03MapSolenoid : public FieldX03VMap{
public:
  virtual ~FieldX03MapSolenoid(){}
  FieldX03MapSolenoid(G4String tableName, G4double theModelBFactor) ;
  void GetFieldValue(const G4ThreeVector &pos, G4ThreeVector &value);
protected:
  std::vector<G4float> _Brho ;
  std::vector<G4float> _Bz ;

  G4double _rhoMin ;
  G4double _rhoMax ;
  G4double _drho ;
  G4double _zMin ;
  G4double _zMax ;
  G4double _dz ;
  G4int  _nrho ;
  G4int  _nz ;
};



#define MAP_BINS 1001 // or more, if you like
#define MAP_STEP (1.0 * cm) // as of August 2005

class FieldX03MapDID : public FieldX03VMap{


public:
  virtual ~FieldX03MapDID(){}
  FieldX03MapDID(G4String tableName, G4double theModelBFactor) ;
  void GetFieldValue(const G4ThreeVector &pos, G4ThreeVector &value);
protected:
  //  std::vector<G4float> _fieldMap ;
  G4float _fieldMap[MAP_BINS] ;
};

#endif // FieldX03_hh
