//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for ILC   -      *
//*                                                     *
//* For more information about Mokka, visit the         *
//*                                                     *
//*  Mokka.in2p3.fr  Mokka home page.                   *
//*                                                     *
//*******************************************************
//
// $Id: SLHcal01.hh,v 1.3 2008/10/23 15:57:38 mora Exp $
// $Name: mokka-07-00 $
//
#ifndef SLHcal01_h
#define SLHcal01_h 1

class G4LogicalVolume;
class G4VSolid;
class G4Box;
class G4Tubs;
class SEcalSDRing02;		     

#include "VSubDetectorDriver.hh"

class SLHcal01 : public VSubDetectorDriver
{
 public:
  SLHcal01() : VSubDetectorDriver("SLHcal01","lhcal"), 
	       theMaxStepAllowed(DBL_MAX)
    //, theLHcalSD(0)
  {}
  
  ~SLHcal01();

  G4bool 
  ContextualConstruct(const CGAGeometryEnvironment&,
		      G4LogicalVolume*);
  
  G4bool 
  PostConstructAction(CGAGeometryEnvironment& );
  
  // To reload hits for visualisation
  void LoadEvent(FILE* theSubDetectorEventHitsFileInput);
  
private:
  
  // Main and calculated parameters
  G4double  Ecal_fiber_thickness,Ecal_Si_thickness,
    Hcal_endcap_center_box_size, LHcal_cell_size,
    LHcal_inner_radius, LHcal_Electronics_space;

  G4double Hcal_endcap_zmin, LHcal_zmin_displacement;
  
  G4double module_thickness,Hcal_inner_radius,
    LHcal_total_Slab_thickness, TUBE_crossing_angle, 
    LHcal_lateral_face_thickness,sensitive_Si_size;

  G4double LhcalBoxSize, LhcalAvailableBoxSize;

  G4double Ecal_Slab_shielding, Ecal_Slab_copper_thickness,
    Ecal_Slab_PCB_thickness, Ecal_Slab_glue_gap,
    Ecal_Slab_ground_thickness, Ecal_Alveolus_Air_Gap;
  
  G4int LHcal_nlayers;

  G4double LHcal_radiator_thickness;

  // Setup
  G4bool Setup(const CGAGeometryEnvironment &);
  // Build
  G4bool Build(G4LogicalVolume*);
  G4double BuildLhcalAlveolus (G4int,
				G4double,
				G4LogicalVolume*,
				G4bool Zminus=false);


  G4Material * RadiatorMaterial;
  SEcalSDRing02* theLHcalSD;

  G4double theMaxStepAllowed;
  G4Tubs *CenterECTub;
  G4TranslateX3D* FollowLcal, *FollowLcalZminus;
  
  G4Box *LhcalSiBox;
  G4LogicalVolume *LhcalLogZplus, *LhcalLogZminus, 
    *LhcalSiLogZplus, *LhcalSiLogZminus;
  
  G4LogicalVolume *LhcalRadiator;
  
  G4double module_z_offset;

#ifdef MOKKA_GEAR
  // MokkaGear
  
  struct helpParameters{
    G4double innerRadius;
    G4double outerRadius;
    G4double zMax;
    G4double phi0;
    std::vector<double> layerPos;
    std::vector<double> radiThickness;
    G4int count;
    G4double leastZ;
    G4double mostZ;
  };
  
  helpParameters helpLHcal;
#endif
  
};

#endif


