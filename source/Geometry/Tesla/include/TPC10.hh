// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: TPC10.hh,v 1.1 2009/05/13 08:22:57 steve Exp $
// $Name: mokka-07-00 $

#ifndef TPC10_hh
#define TPC10_hh 1

class G4LogicalVolume;
class Database;

#include "VSubDetectorDriver.hh"
#include "G4Material.hh"
class TPC10 : public VSubDetectorDriver
{
public:
  TPC10(void): VSubDetectorDriver("tpc10", "tpc") {}
  ~TPC10(void) {}

  G4bool ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog);

#ifdef MOKKA_GEAR
  void GearSetup();

private:

  G4double _gear_r_min;
  G4double _gear_r_max;
  G4double _gear_inner_wall_thickness;
  G4double _gear_outer_wall_thickness;

  G4double _gear_r_min_readout;
  G4double _gear_r_max_readout;
  G4int    _gear_n_rows_readout;
  G4double _gear_pad_height;
  G4double _gear_pad_width;

  G4double _gear_max_drift_length;
  G4double _gear_z_anode; ///< the z position of the drift anode (the edge of the readout terminating the drift volume)
  
  std::map<std::string, G4double> gear_inner_wall_material_thicknesses;
  G4double gear_inner_wall_material_total_density;

  std::map<std::string, G4double> gear_outer_wall_material_thicknesses;
  G4double gear_outer_wall_material_total_density;  

  G4Material* _gear_gas_material;







#endif


private:

};

#endif
