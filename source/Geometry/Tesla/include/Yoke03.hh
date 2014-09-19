// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: Yoke03.hh,v 1.1 2006/09/04 11:43:09 musat Exp $
// $Name: mokka-07-00 $

#ifndef Yoke03_hh
#define Yoke03_hh 1

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


class Yoke03: public VSubDetectorDriver
{
public:
  Yoke03(void): VSubDetectorDriver("yoke03", "yoke") {}
  ~Yoke03(void) {}
  
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
