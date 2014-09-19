#ifndef TBt3b00_h
#define TBt3b00_h 1

#include "Control.hh"
#include "VSubDetectorDriver.hh"
#include "TBSD_VCell03.hh"
#include "G4Transform3D.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"

class Database;

class TBt3b00 : public VSubDetectorDriver
{
public:
  TBt3b00() : VSubDetectorDriver("TBt3b00","TBt3b00"),
	       db(0)
  {}

  ~TBt3b00();

  G4bool ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
			     G4LogicalVolume *theWorld);

private:
  
  G4bool T3BConstruct(G4LogicalVolume *theWorld, 
		      G4double xdim, 
		      G4double ydim, 
		      G4double xdim_scint, 
		      G4double ydim_scint, 
		      G4double xdim_pcb, 
		      G4double ydim_pcb,
		      G4double T3B_X_Pos_ScintAndPCB,
		      G4double T3B_Y_Pos_ScintAndPCB, 
		      G4double T3B_X_Pos,
		      G4double T3B_Y_Pos,
		      G4double T3B_Z_Pos);
 
  void BuildElements(G4double xdim, 
		     G4double ydim,
		     G4double xdim_scint, 
		     G4double ydim_scint,
		     G4double xdim_pcb,
		     G4double ydim_pcb);
  
  G4bool BuildT3B(G4double  T3B_X_Pos, 
		  G4double  T3B_Y_Pos, 
		  G4double  T3B_Z_Pos, 
		  G4double T3B_X_Pos_ScintAndPCB, 
		  G4double T3B_Y_Pos_ScintAndPCB);
  void FetchAll();

private:

  /*useful quantities*/
  G4double translateX, translateY;
  G4double PCB_density, PCB_silicon_fractiomass, PCB_elO_fractionmass, PCB_graphite_fractionmass, PCB_elH_fractionmass, PCB_elBr_fractionmass;
  G4double T3B_X_dim, T3B_Y_dim, T3B_X_Pos, T3B_Y_Pos, T3B_Z_Pos, Aluminium_thickness, Poly_thickness, PCB_thickness, T3B_Scint_X_dim, T3B_Scint_Y_dim, Air_thickness;
  G4double T3B_PCB_X_dim, T3B_PCB_Y_dim, T3B_X_Pos_ScintAndPCB, T3B_Y_Pos_ScintAndPCB;
 
  /* translation of Scintillator*/
  G4ThreeVector translateAl, translateAir, translatePoly, translatePCB;

  /* logical volumes*/
  G4LogicalVolume *WorldLogVol, *AlLogical, *AirLogical, *PolyLogical, *PCBLogical;

  Database* db;
};


#endif
