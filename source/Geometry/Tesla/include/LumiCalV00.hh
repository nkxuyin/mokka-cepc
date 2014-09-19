#ifndef LUMICAL_H
#define LUMICAL_H 1
/* 
 * First implementation of LumiCal detector 
 * 
 */

/* Author Bogdan Pawlik, INP PAS Krakow */
/* Feb. 2005 */


/*
   - modified for crossing as LumiCalX               Feb. 2007 (bp)
   - modified to implement inter sector dead gap and
     proper description of electronic fanout         Oct. 2009 (bp)
   - modified for virtual cells                      Mar. 2010 (bp)
$Id: LumiCalV00.hh 32 2010-03-03 19:55:49Z bogdan $
*/
//Include derived from the model needed
#include "LumiCalSD.hh"

//includes for using Mokka frame
#include "Control.hh"
#include "CGADefs.h"
#include "MySQLWrapper.hh"
#include "CGAGeometryManager.hh"
#include "VSubDetectorDriver.hh"
#include "UserInit.hh"

//G4 includes
#include "G4Tubs.hh"
#include "G4Box.hh"
#include "G4EllipticalTube.hh"
#include "G4Polyhedra.hh"
#include "G4VSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"
#include "G4PVReplica.hh"
#include "G4PVDivision.hh"
#include "G4ThreeVector.hh"
#include "G4Transform3D.hh"
#include "G4RotationMatrix.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4Element.hh"
#include "G4ElementTable.hh"


class Database;

class G4LogicalVolume;
class G4VPhysicalVolume;
class G4Material;



class LumiCalV00 : public VSubDetectorDriver
{
  
public:
  LumiCalV00() : VSubDetectorDriver("LumiCalV00","LumiCal"), db(0) {}
  ~LumiCalV00(); 
  
  G4bool construct(const G4String &aSubDetectorDBName,
                   G4LogicalVolume *WorldLog);
  
  
  G4double GetPhiCellSize() { return phistrip_dphi;}
  G4double GetRhoCellSize() { return thetastrip_dr;}

  
  
  void Print();
  //method to fetch the database entries
  void FetchdbEntries(); 
  //method to declare a sensitive volume
  void SetSD();
  //Build elements of the detector
  void BuildElements();
  //method to contruct the actual detector
  G4bool Build_LumiCal();
  // mrthod to define materials
  void AddMaterial();
  
private: 
  //A pointer to the database
  Database* db;
 //Some Geometry variables
  G4int n_layers, ncell_theta, ncell_phi, n_tiles, sectors_per_tile; // n layers, number of cells in r and phi direction
  G4double cal_innerradius, cal_outerradius, cal_hz; // Outer dimensions of the detector
  G4double cal_extra_size, tile_gap, tile_dPhi;
  G4double thetastrip_dr, phistrip_dphi, cal_sphi, cal_ephi, cal_sensor_rmin; //cell size in r and phi  
  G4double layer_gap, layer_hz, plane_phi_offset; 
  G4double z_begin; // Defines the starting point of the calorimeter
  G4double bx_angle, phi_offset;
  
  
  //dimensions of the layer components along z
  G4double fanout_hthickness, tungsten_hthickness, silicon_hthickness, metalization_hthickness; 
  // support handles size
  G4double ear_height;

  //Logical Volumes used for the detector construction
  G4LogicalVolume *WholeLumiCalLogical, *WorldLogical,  *AbsLayerLogical;
  G4LogicalVolume *LayerLogical, *LogicFan1, *LogicFan2, *SupportSpaceLog, *FECoolLog;
  G4LogicalVolume *SupportLayerLog, *SupportEarsLog, *FEBoardLog, *FEMothLog, *ChipMothLog;
  G4LogicalVolume *SensorLog, *SiWaferLog, *PadMetalLog;
  
  //The pointer to the sensitive detector implementation
  LumiCalSD *theLumiCalSD;
  
  //Materials used for detector construction
  G4Material *air, *silicon, *fanele1, *fanele2, *Al2O3, *Alu;
  // some hardwired params
  G4int n_Bolts;

 
};
#endif
