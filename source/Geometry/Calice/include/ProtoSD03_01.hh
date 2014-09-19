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
// $Id: ProtoSD03_01.hh,v 1.1 2008/03/12 13:23:04 musat Exp $
// $Name: mokka-07-00 $
//

#ifndef ProtoSD03_01_h
#define ProtoSD03_01_h 1

#include "CalHit.hh"
#include "VSensitiveDetector.hh"
#include "G4Step.hh"
#include <vector>
#include <utility>

class ProtoSD03_01 : public VSensitiveDetector
{
  
public:
  ProtoSD03_01(G4double dimX,G4double dimZ, G4int n_cell_x, G4int n_cell_z,
	  G4int n_waffers_x, G4int n_waffers_z, G4double upper_waffer_shift,
	  G4double garde_size, 
	  std::vector<G4double>& cell_y_pos_in_alveolus,
	  std::vector<std::pair<G4double, G4double> >& alveolus_y_spacing,
	  G4double exit_fiber_thickness,
	  std::vector<std::pair<G4double,G4double> >& a_shifts_vector,
	  G4double a_struct_shift[3], G4double StructHalfX[3],
	  G4ThreeVector *anEcalPosition, G4RotationMatrix * anEcalRotation, 
	  G4String ProtoSD03_01name, G4int start_layer_number,
	  G4double inter_wafer_gap, G4double halfAlveolusX,
	  G4double halfEnvEcalX,
	  G4double halfEcalX, G4double & halfEcalY, 
	  G4double inter_tower_fiber_thickness,
	  G4double inter_structures_gap, G4int n_guard_ring_zones,
	  G4double lateralWaferGap, G4double endcap_x, 
	  std::vector<G4double> centerYLayerShifts,
	  std::vector<G4double> rightYLayerShifts,
	  G4bool useID1=false);
  virtual ~ProtoSD03_01();
  
  void Initialize(G4HCofThisEvent*HCE);
  G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
  void EndOfEvent(G4HCofThisEvent*HCE);
  void clear();
  void DrawAll();
  void PrintAll();

  // Basic reload hits from Ascii Files
  void LoadEvent(FILE* theSubDetectorEventHitsFileInput);
  cell_ids GetCellIndex(double X, double Y, double Z,
	int & flag, double xDir=0, double yDir=0, double zDir=1);
  G4ThreeVector GetCellCenter(G4int,G4int WI,G4int WJ,
	G4int I,G4int J,G4int K);

  HitsCollection *CalCollection;
  G4int HCID;

private:
  void GetCellIndices(const G4VTouchable * theTouchable,
	G4int & I, G4int & J, G4int & WI, G4int & WJ, G4int & K);
  void GetNearestCell(const G4VTouchable * theTouchable,
	G4ThreeVector thePosition,
	G4int & I, G4int & J, G4int & WI, G4int & WJ, G4int & K,
	G4int & zone);

  G4double theDimX,theDimZ;
  G4int the_n_cell_x, the_n_cell_z, the_n_waffers_x, the_n_waffers_z; 
  G4double the_upper_waffer_shift;
  G4double the_garde_size; 
  std::vector<G4double> the_cell_y_pos_in_alveolus; 
  std::vector<std::pair<G4double, G4double> > the_alveolus_y_spacing; 
  G4double the_exit_fiber_thickness;
  std::vector<std::pair<G4double,G4double> > the_shifts_vector;
  G4double theStructHalfX[3];
  G4double the_struct_shifts[3];
  G4ThreeVector * theEcalPosition;
  G4RotationMatrix * theEcalRotation;
  G4int the_start_layer_number;
  G4double the_inter_wafer_gap, theHalfAlveolusX;
  G4double theHalfEnvEcalX, theHalfEcalX, theHalfEcalY;
  G4double the_inter_tower_fiber_thickness;
  G4double the_inter_structures_gap;
  G4int the_n_guard_ring_zones;
  G4double theLateralWaferGap;
  G4double the_endcap_x;
  std::vector<G4double> theCenterYLayerShifts;
  std::vector<G4double> theRightYLayerShifts;
};

#endif

