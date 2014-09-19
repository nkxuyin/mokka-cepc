#include "Control.hh"
#include "TBCellReplication.hh"

#include "G4VSolid.hh"
#include "G4Box.hh"
#include "G4VisAttributes.hh"

TBCellReplication::TBCellReplication()
{
  n_row = n_cell_z = 0; cell_hx = cell_hz = 0;
}


TBCellReplication::TBCellReplication(G4LogicalVolume *MomLog, G4LogicalVolume *CellLog)
{
  n_row = n_cell_z = 0; cell_hx = cell_hz = 0;
  
  SetCellLogical(CellLog);
  SetMomLogical(MomLog);
}


TBCellReplication::~TBCellReplication()
{}

void TBCellReplication::SetCellLogical(G4LogicalVolume *cLV) {
  assert(cLV != 0);

  const G4VSolid *cellVSolid = cLV->GetSolid();  
  assert(cellVSolid->GetEntityType()=="G4Box");
  const G4Box *cellSolid = static_cast<const G4Box*>(cellVSolid);
    
  cell_hx=cellSolid->GetXHalfLength();
  cell_hy=cellSolid->GetYHalfLength();
  cell_hz=cellSolid->GetZHalfLength();  

  CellLogical = cLV;
}

void TBCellReplication::SetMomLogical(G4LogicalVolume *mLV)
{

  assert(mLV != 0);

  const G4VSolid *momVSolid = mLV->GetSolid();

  assert(momVSolid->GetEntityType()=="G4Box");

  const G4Box *momSolid = static_cast<const G4Box*>(momVSolid);

  mom_hx = momSolid->GetXHalfLength();
  mom_hy = momSolid->GetYHalfLength();
  mom_hz = momSolid->GetZHalfLength();

  MomLogical = mLV;
}

void TBCellReplication::Fit()
{
  // first check dims
  assert((mom_hy == cell_hy) && 
	 (mom_hz >= cell_hz) && 
	 (mom_hx >= cell_hx));

  // fit cells to row
  G4double rem, n_whole;  
  rem = std::modf(mom_hz/cell_hz,&n_whole);

  // disallow fractional cells
  assert(rem==0);

  // set number of cells in a row
  n_cell_z = (G4int)n_whole;

  // create row solid & LV
  G4Box *RowSolid = new G4Box("RowSolid",
			      cell_hx,
			      cell_hy,
			      n_cell_z*cell_hz);

  RowLogical = new G4LogicalVolume(RowSolid,
				   CellLogical->GetMaterial(),
				   "RowLogical",
				   0,
				   0,
				   0);		

  RowLogical->SetVisAttributes(G4VisAttributes::Invisible);
	      
  // fit rows
  rem = std::modf(mom_hx/cell_hx,&n_whole);

  // no fractional rows
  assert(rem==0);

  n_row = (G4int)n_whole;    
}


void TBCellReplication::Print()
{
  G4cout << "\nTBCellReplication Info: " << G4endl
	 << "n_row: " << n_row << G4endl
	 << "n_cell_z: " << n_cell_z << G4endl
	 << "cell_hx: " << cell_hx << G4endl
	 << "cell_hy: " << cell_hy << G4endl
	 << "cell_hz: " << cell_hz << G4endl
	 << "mom_hx: " << mom_hx << G4endl
	 << "mom_hz: " << mom_hz << G4endl
	 << "mom_hy: " << mom_hy << G4endl;
    
  if (RowReplica && LayerReplica)
    G4cout << "_replicas placed_" << G4endl;
  else
    G4cout << "_replicas *not* placed_" << G4endl;

  G4cout << G4endl;
}

// public interface
void TBCellReplication::Replicate()
{
  Fit();    
  ReplicateCell();
  ReplicateRow();
  Print();
}


void TBCellReplication::ReplicateCell()
{
  RowReplica = new G4PVReplica("RowReplica",
			       CellLogical,
			       RowLogical,
			       kZAxis,
			       n_cell_z,
			       cell_hz*2,
			       0);
}


void TBCellReplication::ReplicateRow()
{
  LayerReplica = new G4PVReplica("LayerReplica",
				 RowLogical,
				 MomLogical,
				 kXAxis,
				 n_row,
				 cell_hx*2,
				 0);
}
