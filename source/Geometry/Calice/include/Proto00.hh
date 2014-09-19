//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for Tesla -      *
//*                                                     *
//* For more information about Mokka, please, go to the *
//*                                                     *
//*  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
//*                                                     *
//* Mokka home page.                                    *
//*                                                     *
//*******************************************************
//
// $Id: Proto00.hh,v 1.1 2003/07/18 09:04:59 musat Exp $
// $Name: mokka-07-00 $
//
#ifndef Proto00_h
#define Proto00_h 1

class G4LogicalVolume;
class Database;
class G4Material;
class ProtoSD;

#include <vector>
#include "Control.hh"
#include "VSubDetectorDriver.hh"

class PLATEGROUP {
public:
  PLATEGROUP(int an_plates,G4double aW_thickness)
    : n_plates(an_plates),W_thickness(aW_thickness),
      AsDeadWLogical(0),AsAlveolusLogical(0),
      AsDeadTotalHalfY(0),AsAlveolusTotalHalfY(0){}
  
  G4int n_plates;
  G4double W_thickness;
  G4double x_offset_start,x_offset_step;
  G4LogicalVolume *AsDeadWLogical;
  G4LogicalVolume *AsAlveolusLogical;
  G4double AsDeadTotalHalfY,AsAlveolusTotalHalfY;
};

class Proto00 : public VSubDetectorDriver
{
public:
  Proto00() : VSubDetectorDriver("proto00","proto"),
	      db(0),theProtoSD(0),Mix(0)
  {}
  
  ~Proto00();

  G4bool construct(const G4String &aSubDetectorDBName,
			    G4LogicalVolume *WorldLog);

private:
  
  void BuildElements();
  void BuildAlveolus(PLATEGROUP*);
  void BuildDeadPlate(PLATEGROUP*);
  
  G4int total_W_plates,n_towers,n_dead_w_plates;
  G4double fiber_thickness,inter_tower_fiber_thickness;
  G4double HalfAlveolusX,HalfAlveolusZ,HalfWSlabX,HalfWSlabZ;
  G4double HalfSuppX,HalfWafferX,HalfWafferY,HalfWafferZ;
  G4double HalfCuY,HalfMixY;

  Database* db;

  ProtoSD* theProtoSD;
  G4Material* Mix;

  G4double HalfDeadWX,HalfDeadWZ,
    tan_rate;

  G4LogicalVolume *WafferLogical,*CuLogical,*KaptonLogical;
  G4LogicalVolume *DetectorLogical,*WorldLog;

 std::vector<PLATEGROUP*> PlateGroups;
 std::vector<PLATEGROUP*> Plates;

};

#endif


