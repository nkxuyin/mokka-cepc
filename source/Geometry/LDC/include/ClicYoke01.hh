// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: ClicYoke01.hh,v 1.1 2010/08/17 $
// $Name: mokka-07-05$

#ifndef ClicYoke01_hh
#define ClicYoke01_hh 1

#include "VSubDetectorDriver.hh"

class G4LogicalVolume;
class Database;
class G4VSolid;
class muonSD;
class HECSD;
class G4UserLimits;
class G4Polyhedra;
class G4Material;
class G4Box;


class ClicYoke01: public VSubDetectorDriver
{
public:
  ClicYoke01(void): VSubDetectorDriver("ClicYoke01", "yoke") {}
  ~ClicYoke01(void) {}
  
   G4bool ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog);

 
 private:


  G4LogicalVolume * BuildRPC1Box(G4Box* ChamberSolid,
				 muonSD* theSD, 
				 G4int layer_id, 
				 G4UserLimits* pULimits, 
				 Database *db);


  G4LogicalVolume * BuildRPC1ECShape(G4Polyhedra* ChamberSolid,
				     muonSD* theSD,
				     G4int layer_id,
				     G4UserLimits* pULimits,
				     Database *db,const CGAGeometryEnvironment &env);

  G4LogicalVolume * BuildRPC1PlugShape(G4Polyhedra* ChamberSolid,
				     muonSD* theSD,
				     G4int layer_id,
				     G4UserLimits* pULimits,
				     Database *db,const CGAGeometryEnvironment &env);
 
  G4double iron_thickness;
  G4double layer_thickness; 
  G4int    number_of_layers;  
  G4int    symmetry;

  G4double HCAL_R_max;
};

#endif
