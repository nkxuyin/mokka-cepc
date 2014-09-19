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
// $Id: TBscecal01.hh,v 1.2 2008/12/03 16:13:30 musat Exp $
// $Name: mokka-07-00 $
//
#ifndef TBscecal01_h
#define TBscecal01_h 1

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
//120228.#include "TBSD_VCell02.hh"
#include "TBSDVCellscecal01.hh"

/*
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
*/

class TBscecal01 : public VSubDetectorDriver
{
public:
  TBscecal01() : VSubDetectorDriver("tbscecal01","tbscecal"),
		db(0),
		theMaxStepAllowed(DBL_MAX), Dumped(false),
		theTRKSD(0)
  { }
  
  ~TBscecal01();

  G4bool construct(const G4String &aSubDetectorDBName,
			    G4LogicalVolume *WorldLog);

  virtual G4bool ContextualConstruct(
		  const CGAGeometryEnvironment &aGeometryEnvironment,
		  G4LogicalVolume *theWorld);

  void BeginOfEventAction(const G4Event* anEvent);
  void EndOfEventAction(const G4Event* evt);

  const G4int* GetNCells() {return &ncell_xz[0];}
  const G4double* GetGridSize() {return &grid_size;}

private:
 
  void DefineMaterial(void);
  void BuildEcal(); 
  void BuildLayer(G4LogicalVolume *DetLog, G4int nlay); 
  void BuildElements(); 
  void SetSD();

// for the future?
  void BuildStructures(void);
  void CalculateSlabShifts(void);


G4String theSubDetectorName;

//G4double cell_dim_x, cell_dim_z, garde_size;

Database* db;
G4double lateral_x;
G4double lateral_y;
G4int n_layers, ncell_xz[2];

//from the steering file
G4double grid_size;
G4double config_angle;//configuration_angle, EcalRotationAngle 
G4ThreeVector TranslationVector; //EcalTranslateX, EcalTranslateY
G4double absorberDens; // absorberDensity
G4double cable_etcDens; //cable_etcDensity
G4double massFraction_W;  //massFraction_W
G4double massFraction_C;  //massFraction_C
G4double massFraction_Co; //massFraction_Co
G4double massFraction_Cr; //massFraction_Cr

G4double y_place;
G4double cal_hx, cal_hy, cal_hz, layer_hthickness;

G4double w_hthickness, reffront_hthickness, sc_hthickness, refrear_hthickness, mixgap_hthickness, air_hthickness;

G4Material *w, *reffront, *sc, *refrear, *mixgap, *g10, *air;
G4LogicalVolume *WholeLayerLogical, *WLogical, *FrontRefLogical, *ScLogical, *RearRefLogical, *MixgapLogical, *AirLogical;

G4LogicalVolume *WorldLog, *DetectorLogical;

//120228.1018TBSD_VCell02 *ecalSD;
TBSDVCellscecal01 *ecalSD;


//ProtoSD03* theCellProtoSD;
//ProtoSD03* theGRProtoSD;

//TODO G4VisAttributes * VisAttAir, 
//		*VisAttAl, 
//		*VisAttCF;
//G4VisAttributes * VisAttG10, 
//		*VisAttSi;

G4double theMaxStepAllowed;




G4ThreeVector EcalPosition;
G4RotationMatrix * EcalRotation;

G4bool Dumped;
G4bool c_angle_from_steering;

G4bool useTracker;
G4LogicalVolume *TrackerLogical;
TRKSD00 * theTRKSD;

};

#endif

