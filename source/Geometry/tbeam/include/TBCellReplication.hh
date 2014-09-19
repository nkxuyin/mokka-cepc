#ifndef TBCellReplication_h
#define TBCellReplication_h 1

#include "G4LogicalVolume.hh"
#include "G4PVReplica.hh"

class TBCellReplication
{
public:
  TBCellReplication();
  TBCellReplication(G4LogicalVolume *MomLog, G4LogicalVolume *CellLog);

  ~TBCellReplication();

public:  
  void Replicate();
  
  void SetCellLogical(G4LogicalVolume *cLV);
  inline G4LogicalVolume* GetCellLogical() const {return CellLogical;}

  void SetMomLogical(G4LogicalVolume *mLV);
  inline G4LogicalVolume* GetMomLogical() const {return MomLogical;}

private:
  void ReplicateCell();
  void ReplicateRow();  

  void Fit();
  void Check();

  void Print();

private:
  G4int n_row, n_cell_z;  
  G4double cell_hx, cell_hz, cell_hy;
  G4double mom_hx, mom_hz, mom_hy;

  G4LogicalVolume *MomLogical, *CellLogical, *RowLogical;
  
  G4PVReplica *RowReplica;
  G4PVReplica *LayerReplica;
};

#endif
