#ifndef LUMICAL_H
#define LUMICAL_H 1
/* 
 * First implementation of LumiCal detector 
 * 
 */

/* Author Bogdan Pawlik, INP PAS Krakow */
/* Feb. 2005 */

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
#include "G4VSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4PVReplica.hh"
#include "G4PVDivision.hh"



class Database;

class G4LogicalVolume;
class G4VPhysicalVolume;
class G4Material;



class LumiCal : public VSubDetectorDriver
{
  
public:
  LumiCal() : VSubDetectorDriver("LumiCal","LumiCal"), db(0) {}
  ~LumiCal(); 
  
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
  
private: 
  //A pointer to the database
  Database* db;
  // type of the lumical design
  G4String LumiCal_Type;
  G4String Strip_Type;
 //Some Geometry variables
  G4int n_layers, ncell_theta, ncell_phi; // n layers, number of cells in r and phi direction
  G4double cal_innerradius, cal_outerradius, cal_hz; // Outer dimensions of the detector
  G4double thetastrip_dr, phistrip_dphi, cal_sphi, cal_ephi; //cell size in r and phi  
  G4double layer_gap,layer_hz; 
  G4double z_begin; // Defines the starting point of the calorimeter
  
  
  //dimensions of the layer components along z
  G4double support_hthickness, tungsten_hthickness, silicon_hthickness; 
  

  //Logical Volumes used for the detector construction
  G4LogicalVolume *WholeLumiCalLogical, *WorldLogical,  *AbsLayerLogical;
  G4LogicalVolume *ThetaLayerLogical, *PhiLayerLogical, *SupportLayerLogical;
  G4LogicalVolume *ThetaStripLogical, *PhiStripLogical;
  G4LogicalVolume *SenseThetaLogical, *SensePhiLogical; //whole silicon layer
  
  //The pointer to the sensitive detector implementation
  LumiCalSD *theLumiCalSD;
  
  //Materials used for detector construction
  G4Material *poly, *air, *silicon;

 
};
#endif
