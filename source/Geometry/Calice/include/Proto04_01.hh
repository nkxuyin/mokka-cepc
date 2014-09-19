//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for Tesla -      *
//*                                                     *
//* For more information about Mokka, please, go to the *
//*                                                     *
//*  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
//*                                                     *
//* Mokka home page.                                    *
//*                                                     *
//*******************************************************
//
// $Id: Proto04_01.hh,v 1.3 2008/12/03 16:13:30 musat Exp $
// $Name: mokka-07-00 $
//
#ifndef Proto04_01_h
#define Proto04_01_h 1

class G4VisAttributes;
class G4LogicalVolume;
class Database;
class G4Material;
class ProtoSD03;
class TRKSD00;
class G4Region;

#include <vector>
#include <utility>
#include "Control.hh"
#include "VSubDetectorDriver.hh"

class WLAYERS {
public:
  WLAYERS(G4int nLayers, G4double a_thickness)
    : n_layers(nLayers), nominal_w_thickness(a_thickness),
      real_w_thickness(0), // for a future detailed release
      AsLeftDeadWLogical(0), AsCenterDeadWLogical(0), 
      AsCenterSlabWLogical(0), AsRightDeadWLogical(0),
      AsRightSlabWLogical(0), AsLeftAlveolusLogical(0),
      AsCenterAlveolusLogical(0),AsRightAlveolusLogical(0),
      center_slab_shift(0), center_waffer_shift(0),
      right_slab_shift(0), right_waffer_shift(0),
      cell_y_pos_in_alveolus(0),
      maxSlabShift(0),
      AsAlveolusTotalHalfY(0) {}
  
G4int n_layers;
G4double nominal_w_thickness;
G4double real_w_thickness;
G4LogicalVolume *AsLeftDeadWLogical;
G4LogicalVolume *AsCenterDeadWLogical;
G4LogicalVolume *AsCenterSlabWLogical;
G4LogicalVolume *AsRightDeadWLogical;
G4LogicalVolume *AsRightSlabWLogical;
G4LogicalVolume *AsLeftAlveolusLogical;
G4LogicalVolume *AsCenterAlveolusLogical;
G4LogicalVolume *AsRightAlveolusLogical;
G4double center_slab_shift, center_waffer_shift;
G4double right_slab_shift, right_waffer_shift;
G4double cell_y_pos_in_alveolus;
G4double maxSlabShift;
G4double AsAlveolusTotalHalfY;
};

class Proto04_01 : public VSubDetectorDriver
{
public:
  Proto04_01() : VSubDetectorDriver("proto04_01","proto"),
		db(0),theCellProtoSD(0),theGRProtoSD(0),
		theMaxStepAllowed(DBL_MAX), Dumped(false),
		theTRKSD(0)
  { }
  
  ~Proto04_01();

  G4bool construct(const G4String &aSubDetectorDBName,
			    G4LogicalVolume *WorldLog);

  virtual G4bool ContextualConstruct(
		  const CGAGeometryEnvironment &aGeometryEnvironment,
		  G4LogicalVolume *theWorld);

  void BeginOfEventAction(const G4Event* anEvent);
  void EndOfEventAction(const G4Event* evt);

private:
  
  void BuildElements();
  void BuildDeadPlate(WLAYERS*);
  void BuildAlveola(WLAYERS*);
  void FillAlveola(WLAYERS*, G4LogicalVolume *, G4LogicalVolume *);
  void PlaceWafers(G4LogicalVolume *, G4LogicalVolume *,
	            G4LogicalVolume *, G4LogicalVolume *,
		    G4double, G4double, G4double, G4int, G4int);
  void BuildStructures(void);
  void DefineMaterial(void);
  void CalculateSlabShifts(void);

G4String theSubDetectorName;

G4Material * CarbonFiber;

G4int start_layer_number, n_towers;
G4double struct_shift[3];
G4double StructHalfX[3];

G4double inter_tower_fiber_thickness, lateral_fiber_thickness;
G4double front_rear_fiber_thickness, inter_deadw_fiber_thickness;
G4double deadw_fiber_thickness, exit_fiber_thickness; 
G4double inter_structures_gap, al_cf_z_gap, al_cf_y_gap;

G4int total_W_plates;
G4double upper_waffer_shift, wafer_x_shift, endcap_x;
G4double waffer_fiber_lateral_thickness;

G4double waffer_fiber_vertical_thickness, g10_thickness, g10_x_out; 
G4double al_thickness, al_g10_gap;
G4double lateralWaferGap;

G4double inter_waffer_gap;
G4int n_waffers_x, n_waffers_z;

G4double HalfWafferY;
G4double HalfWafferZ, HalfWafferX;
G4double HalfAlveolusX, HalfAlveolusZ;

G4double cell_dim_x, cell_dim_z, garde_size;
G4int n_cell_x, n_cell_z, n_guard_ring_zones;
Database* db;

ProtoSD03* theCellProtoSD;
ProtoSD03* theGRProtoSD;

G4LogicalVolume *SiWafferLogical, *SiO2WafferLogical;
G4LogicalVolume *siCellLogical;

G4VisAttributes * VisAttAir, 
		*VisAttAl, 
		*VisAttCF;
G4VisAttributes * VisAttG10, 
		*VisAttSi;

G4double theMaxStepAllowed;

G4double HalfEcalX, HalfEcalY, HalfEcalZ;
G4double HalfWSlabX,HalfWSlabZ;
G4double HalfDeadWX,HalfDeadWZ;

G4double config_angle;
G4ThreeVector TranslationVector;

G4ThreeVector EcalPosition;
G4RotationMatrix * EcalRotation;

G4bool Dumped;
G4bool c_angle_from_steering;

std::vector<WLAYERS*> PlateGroups;
std::vector<WLAYERS*> Plates;
std::vector<std::pair<G4double,G4double> > slab_shifts_vector;
std::vector<G4double> cell_y_pos_in_alveolus;
std::vector<std::pair<G4double, G4double> > alveolus_y_spacing;

std::vector<std::pair<G4LogicalVolume *, G4double> > theStructuresVector;
G4LogicalVolume *DetectorLogical,*WorldLog;
G4Region * G10Region, *WRegion;

G4double trkHalfY;
G4bool useTracker;
G4LogicalVolume *TrackerLogical;
TRKSD00 * theTRKSD;

G4String theG10MaterialName;
};

#endif

