// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: CGAGeometryManager.hh,v 1.15 2009/04/23 15:42:41 mora Exp $
// $Name: mokka-07-00 $
//
// History
// - first implementation for the Mokka Common Geometry Access (CGA)
//   by Gabriel Musat (musat@poly.in2p3.fr), July 2002
// - updated to make use of G4NistManager, Adrian Vogel, 2006-07-11
//
// see CGA documentation at 
// http://mokka.in2p3.fr/software/doc/CGADoc/CGAIndex.html

#ifndef CGAGeometryManager_h
#define CGAGeometryManager_h 1

#include <vector>
#include <map>
#include <utility>
#include "globals.hh"
#include "G4VUserDetectorConstruction.hh"
#include "MySQLWrapper.hh"
#include "G4Region.hh"
#include "G4LogicalVolume.hh"

class G4Material;
class G4Element;
class G4VisAttributes;
class VSubDetectorDriver;
class VSuperSubDetectorDriver;
class CGAGeometryEnvironment;
class SubDetector;

// auxiliary structure to keep Geometry three
struct LV_level
{
  LV_level(G4String aSubDetectorName,
	   G4LogicalVolume* aLV, 
	   G4int alevel)
    : SubDetectorName(aSubDetectorName),LV(aLV),
      level(alevel)
  {
    DefaultVisAttributes = LV->GetVisAttributes();
  }

  virtual ~LV_level() {}
 public :
  G4String SubDetectorName;
  G4LogicalVolume* LV;
  const G4VisAttributes* DefaultVisAttributes;
  G4int level;
};

typedef std::vector<LV_level*> GeometryThree;

class CGAGeometryManager : public G4VUserDetectorConstruction
{
public:
  
  // The CGAGeometryManager has to be a singleton class.
  // To get access to the CGAGeometryManager the user 
  // must call the CGAGeometryManager::GetCGAGeometryManager()
  // static method.
  static CGAGeometryManager* GetCGAGeometryManager();

  void RegisterGeometryDriver(VSubDetectorDriver* aSubDetectorDriver);

  void RegisterGeometryDriver(VSuperSubDetectorDriver* aSuperSubDetectorDriver);

  void RegisterGeometryRegion(G4Region *aRegion, G4double aCut);

#ifdef MOKKA_GEAR
  void GearSetup ();
#endif
  
  virtual ~CGAGeometryManager();

  G4double GetTrackerRegionRmax  () const
  { return tracker_region_rmax;}

  G4double GetTrackerRegionZmax  () const
  { return tracker_region_zmax;}
  
  G4double GetCalorimeterRegionRmax  () const
  { return calorimeter_region_rmax;}

  G4double GetCalorimeterRegionZmax  () const
  { return calorimeter_region_zmax;}
  
  G4int GetNumberOfRegions();
  G4Region* GetGeometryRegion(G4int index);
  G4double GetRegionCutValue(G4int index);

  const G4VPhysicalVolume * GetWorldPhys() const
  { return WorldPhys;}

  GeometryThree * GetGeometryThree() const
  { return theGeometryThree; }
  
  G4VPhysicalVolume* Construct();
  
  // Access to element and material definitions
  static G4Material *GetMaterial(const G4String &name);
  static G4Element *GetElement(const G4String &name, G4bool warning = true);

  // for Control abort usage    
  void
  TmpDBCleanup(G4bool releaseDB = true);

private:
  // The CGAGeometryManager has to be a singleton class.
  // To get access to the CGAGeometryManager the user 
  // must call the CGAGeometryManager::GetCGAGeometryManager()
  // static method.
  CGAGeometryManager() 
    : tracker_region_rmax(0.), tracker_region_zmax(0.), 
      calorimeter_region_rmax(0.), calorimeter_region_zmax(0.), 
      model_tracker_region_rmax(0.), model_tracker_region_zmax(0.), 
      model_calorimeter_region_rmax(0.), model_calorimeter_region_zmax(0.), 
      dbtmp(NULL), tmpDBName(""),WorldLog(NULL),
      last_world_daughter(0)
  {
    theGeometryThree = new GeometryThree();
  }

  static CGAGeometryManager* theCGAGeometryManager;

  G4VPhysicalVolume*   BuildWorldVolume();

  void BuildRecipe();

#ifdef LCIO_MODE
  LCRunHeaderImpl* runHdr; 
  void   InitializeLCIOHeader();
#endif

  void
  BuildSubDriverEnvironment(G4String,CGAGeometryEnvironment&);
  
  void
  BuildGeometrySubDetectorThree(G4String aSubDetectorName);

  G4bool 
  LoadandExecMySQLScript(G4String);
 
  G4VPhysicalVolume *WorldPhys;

  // Registered Sub Detector Drivers
  std::vector <VSubDetectorDriver*> theRegisteredDrivers;

  // Registered Super Sub Detector Drivers
  std::vector <VSuperSubDetectorDriver*> theRegisteredSuperSubDrivers;

  // Geometry Regions
  std::vector<G4Region *> theRegisteredRegions;
  std::vector<G4double> theRegionCuts;
  
  // Tracker and calorimeter regions for hit assignements policy
  // and energy leak studies
  G4double tracker_region_rmax;
  G4double tracker_region_zmax;
  G4double calorimeter_region_rmax;
  G4double calorimeter_region_zmax;

  G4double model_tracker_region_rmax;
  G4double model_tracker_region_zmax;
  G4double model_calorimeter_region_rmax;
  G4double model_calorimeter_region_zmax;

  Database* db;
  Database* db_aux;
  Database* dbtmp;
  G4String tmpDBName;

  std::vector<SubDetector*> Ingredients;

  G4String detector_concept;

  G4LogicalVolume *WorldLog;
  GeometryThree *theGeometryThree;
  G4int last_world_daughter;
};  

#endif
