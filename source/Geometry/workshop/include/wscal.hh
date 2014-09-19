#ifndef WSCAL_H
#define WSCAL_H 1
/* Disclaimer:
 * The detector created in this class for usage within the LC
 * Simulation Mini workshop.
 * It has no realization as a real detector. It's kept simple and yet
 * complicated enough to be used in a demonstration of the software
 * tools available for LC simulation studies. The user may use and/or
 * extend it freely for  his/her own purposes.
 */

/* Author Roman Poeschl, DESY Hamburg */
/* Dec. 2004 */

//Include derived from the model needed
#include "wscalSD.hh"

//includes for using Mokka frame
#include "Control.hh"
#include "CGADefs.h"
#include "MySQLWrapper.hh"
#include "CGAGeometryManager.hh"
#include "VSubDetectorDriver.hh"
#include "UserInit.hh"

//G4 includes
#include "G4Box.hh"
#include "G4VSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4PVReplica.hh"



class Database;

class G4LogicalVolume;
class G4VPhysicalVolume;
class G4Material;



class wscal : public VSubDetectorDriver
{
  
public:
  //Constructor for workshop caloe give name of driver and a basis filename
  //needed for ascii output of the simulation
  wscal() : VSubDetectorDriver("wscal","wscal"), db(0) {}
  
  ~wscal(); 
  
  G4bool construct(const G4String &aSubDetectorDBName,
                   G4LogicalVolume *WorldLog);
  
  
  G4double GetCellSize() { return cell_size; }
  
  
  void Print();
  //method to fetch the database entries
  void FetchdbEntries(); 
  //method to declare a sensitive volume
  void SetSD();
  //Build elements of the detector
  void BuildElements();
  //method to contruct the actual detector
  G4bool Build_wscal();
  //method to define material needed for the calo construction
  void AddMaterial();
  
private: 
  //A pointer to the database
  Database* db;
 //Some Geometry variables
  G4int n_layers, ncell_xy[2]; // n layers, number of cells in xy-direction
  G4double cell_size;        // Parameter determing the cellwise subdivision of a layer
  G4double cal_hx, cal_hy, cal_hz; // Outer dimensions of the detector
  G4double z_begin; // Defines the starting point of the calorimeter
  
  
  //dimensions of the layer components along z
  G4double poly_hthickness, steel_hthickness, layer_hthickness; 
  

  //Logical Volumes used for the detector construction
  G4LogicalVolume *WorldLogical, *AbsLayerLogical, *SenseLayerLogical, *WholeLayerLogical;
  G4LogicalVolume *SenseCellLogical, *RowLogical;
  
  //The pointer to the sensitive detector implementation
  wscalSD *wscal_SD;
  
  //Materials used for detector construction
  G4Material *steel, *poly, *air, *S235;

 
};
#endif
