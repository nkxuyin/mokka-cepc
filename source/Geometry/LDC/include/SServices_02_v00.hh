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
// $Id: SServices_02_v00.hh,v 1.0 2009/06/04 11:40:42 musat Exp $
// $Name: mokka-07-00 $
//
#ifndef SServices_02_v00_h
#define SServices_02_v00_h 1

class G4LogicalVolume;
class Database;

#include "VSubDetectorDriver.hh"

class SServices_02_v00 : public VSubDetectorDriver
{
 public:
  SServices_02_v00() : VSubDetectorDriver("SServices_02_v00","services")
  {}
  
  ~SServices_02_v00();

  G4bool 
  ContextualConstruct(const CGAGeometryEnvironment&,
		      G4LogicalVolume*);

  G4bool 
  PostConstructAction(CGAGeometryEnvironment& );

private:

   void
   BuildTPCEndplateServices(Database *, G4LogicalVolume *);

   void
   BuildEcalBarrelServices(Database *, G4LogicalVolume *);

   void
   FillEcalBarrelServicesContainer(Database *, G4LogicalVolume *);

   void
   BuildTPCAndEcalCables(Database *, G4LogicalVolume *);

   void
   BuildCables(G4double, G4double, G4double,
                G4double, G4double, G4double,
                G4LogicalVolume *, G4int iZ, G4int iTPC=0);

   void
   BuildEcalCoolingServices(Database *, G4LogicalVolume *);

   void
   BuildEcalBarrelCooling_Staves_1_2(Database *, G4LogicalVolume *, G4int iZ);

   void
   BuildEcalBarrelCooling_Staves_3_4_5_6(Database *, G4LogicalVolume *, G4int iZ);

   void
   BuildEcalBarrelCooling_Staves_7_8(Database *, G4LogicalVolume *, G4int iZ);

   void
   BuildEcalEndcapCooling(Database *, G4LogicalVolume *);

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
   G4double RailHeight;
   G4double Hcal_outer_radius;
   G4double Ecal_outer_radius;
   G4double Hcal_inner_radius;
   G4double Ecal_inner_radius;
   G4double top_dim_x;

   G4double CoolingEcalStaves_MaxThickness;
   G4double deltaYDueToCoolingStave2_ZMinus;
   G4double deltaYDueToCoolingStave2_ZPlus;

   G4VisAttributes * VisAttAir;
   G4VisAttributes * VisAttPE;
   G4VisAttributes * VisAttCu;

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

