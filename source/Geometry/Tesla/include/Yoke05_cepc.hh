// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: yoke05_cepc.hh,v 1.1 2008/10/05 18:38:17 frank Exp $
// $Name:  $

#ifndef yoke05_cepc_hh
#define yoke05_cepc_hh 1

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


class yoke05_cepc: public VSubDetectorDriver
{
public:
  yoke05_cepc(void): VSubDetectorDriver("yoke05_cepc", "yoke") {}
  ~yoke05_cepc(void) {}
  
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
  // G4VSolid* BuildECShape(G4Polyhedra* ChamberSolid,G4double L,G4double dz);
 
  G4double iron_thickness  ;
  G4double layer_thickness ; 
  G4int    number_of_layers;  
  G4int    symmetry  ;

  G4double HCAL_R_max;
};

#endif
