#ifndef TBhcal01_h
#define TBhcal01_h 1

#include "Control.hh"
#include "TBSD_VCell.hh"
#include "VSubDetectorDriver.hh"

class G4LogicalVolume;
class Database;
class G4Material;
class G4VPhysicalVolume;

class TBhcal01 : public VSubDetectorDriver
{
public:
  TBhcal01() : VSubDetectorDriver("TBhcal01","TBhcal"),
               db(0)
  {
  }

  ~TBhcal01();

  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *WorldLog);

  void Print();

private:
  void BuildElements();  
  void SetSD(); // called only after cell LV creation
  void BuildLayer(G4LogicalVolume*,G4int);
  //void BuildSensitive(G4VPhysicalVolume* SensPhys, G4bool is_complex);
  G4bool BuildHcal();

  void ReplicateCells(G4LogicalVolume *mLV, G4LogicalVolume *cLV);

  void FetchAll();

private:
  G4int n_layers, n_complex_layers; // n layers, n complex layers
  G4double poly_hthickness, steel_hthickness, layer_hthickness; // along Y
  G4double cal_hx, cal_hy, cal_hz; // cal parameters
  G4double y_place;

  G4double ocell_dim_hx, ocell_dim_hz, n_ocell_x, n_ocell_z; // outer sens   
  G4double icell_dim_hx, icell_dim_hz, n_icell_x, n_icell_z; // inner sens
  G4double bcell_dim_hx, bcell_dim_hz; // border cell
  G4double outer_side_hx, outer_side_hz; // side quadrant
  G4double outer_tb_hz, outer_tb_hx; // top/bottom quadrant
  G4double ocal_hx, ocal_hz;
    
  G4LogicalVolume *WholeLayerLogical, 
    *WholeSensLayerLogical, 
    *InnerSensLayerLogical, 
    *AbsLayerLogical, 
    *WholeSensQuad,
    *SideSensLayerLogical, 
    *TopBottomSensLayerLogical,
    *DetectorLogical, 
    *WorldLogical,
    *CellInnerLogical,
    *CellOuterLogical,
    *CellBorderLogical,
    *BorderLogical;

  G4Material *steel, *poly;

  Database* db;
  TBSD_VCell *hcalSD;
};

#endif
