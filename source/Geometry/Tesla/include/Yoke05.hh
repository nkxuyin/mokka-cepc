// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: Yoke05.hh,v 1.1 2008/10/05 18:38:17 frank Exp $
// $Name:  $

#ifndef Yoke05_hh
#define Yoke05_hh 1

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


class Yoke05: public VSubDetectorDriver
{
public:
  Yoke05(void): VSubDetectorDriver("yoke05", "yoke") {}
  ~Yoke05(void) {}
  
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
