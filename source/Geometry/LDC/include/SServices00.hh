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
// $Id: SServices00.hh,v 1.0 2009/06/04 11:40:42 musat Exp $
// $Name: mokka-07-00 $
//
#ifndef SServices00_h
#define SServices00_h 1

class G4LogicalVolume;
class Database;

#include "VSubDetectorDriver.hh"

class SServices00 : public VSubDetectorDriver
{
 public:
  SServices00() : VSubDetectorDriver("SServices00","services")
  {}
  
  ~SServices00();

  G4bool 
  ContextualConstruct(const CGAGeometryEnvironment&,
		      G4LogicalVolume*);

  G4bool 
  PostConstructAction(CGAGeometryEnvironment& );

private:

   G4bool
   BuildTPCEndplateServices(const CGAGeometryEnvironment &,
			G4LogicalVolume *);

   G4bool
   BuildEcalBarrelServices(const CGAGeometryEnvironment &,
			G4LogicalVolume *);

   G4bool
   BuildEcalBarrel_EndCapServices(const CGAGeometryEnvironment &,
			G4LogicalVolume *);

   G4bool
   BuildHcalBarrel_EndCapServices(const CGAGeometryEnvironment &,
			G4LogicalVolume *);

   G4bool
   FillEcalBarrelServicesContainer(const CGAGeometryEnvironment &,
			G4LogicalVolume *);

   G4bool
   FillHcalServicesModuleWithInnerServices(const CGAGeometryEnvironment &,
			G4LogicalVolume *,G4LogicalVolume *);

   G4bool
   PlaceHcalInnerServicesLayer(G4LogicalVolume *, G4Material*, 
			G4VisAttributes* , G4double, G4ThreeVector &);

   G4bool
   FillHcalServicesModuleWithHcalElectronicsInterface(
			const CGAGeometryEnvironment &,
			G4LogicalVolume *,G4LogicalVolume *);

   G4bool
   FillHcalElectronicsInterfaceLayer(const CGAGeometryEnvironment &,
			G4LogicalVolume *,G4LogicalVolume *,
			G4double, G4double);

   G4bool
   PlaceHcalElectronicsInterfaceComponent(G4LogicalVolume *, G4LogicalVolume *,
			G4Material*, G4VisAttributes* , 
			G4double, G4double, G4double);

   G4VSolid * CutLayer(G4VSolid *, G4double);

   void
   BuildSitCables(G4LogicalVolume *);

   G4String FTD_db_name;
   G4double TPC_inner_radius;
   G4double Sit_cables_cylinder_thickness;
   G4double TUBE_IPOuterBulge_end_z;
   G4double TUBE_IPOuterBulge_end_radius;
   G4double Sit_cables_disk_thickness;
   G4double SIT1_Radius, SIT2_Radius;
   G4double FTD2_cone_thickness, FTD3_cone_thickness;

   G4double TPC_Ecal_Hcal_barrel_halfZ;
   G4double Ecal_cables_gap;
   G4double TPC_outer_radius;
   G4double InnerServicesWidth;
   G4double RailHeight;
   G4double Ecal_outer_radius;
   G4double module_thickness;
   G4double bottom_dim_x;
   G4double top_dim_x;
   G4double Hcal_total_dim_y;
   G4double Hcal_y_dim2_for_x;
   G4double Hcal_bottom_dim_x;
   G4double Hcal_midle_dim_x;
   G4double Hcal_top_dim_x;

   G4VisAttributes * VisAttAir;
   G4VisAttributes * VisAttPE;
   G4VisAttributes * VisAttCu;
   G4VisAttributes * VisAttStainlessSteel;

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
  
  helpParameters helpBarrel;
  helpParameters helpEndcap;
  helpParameters helpPlug;
#endif
  
};

#endif

