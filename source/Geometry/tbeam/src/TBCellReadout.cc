#include "TBCellReadout.hh"

TBCellReadout::TBCellReadout()
{
  // 1x1 cm = default
  cell_x = 1;
  cell_z = 1;

  // defaults probably cause crash!
  cal_hx = 0;
  cal_hz = 0;

  globalRef = G4ThreeVector(0,0,0);
}

TBCellReadout::TBCellReadout(G4double cx, G4double cz, G4double calhx, G4double calhz)
{
  cell_x = cx;
  cell_z = cz;

  cal_hx = calhx;
  cal_hz = calhz;

  globalRef = G4ThreeVector(0,0,0);
}

TBCellReadout::~TBCellReadout()
{}

// general cell # utility
G4int TBCellReadout::GetCellNo(G4double hit_dim, 
			       G4double cell_dim)
			       //,
			       //G4double cal_hdim)
{
  // readjust coordinates
  // hit_dim += cal_hdim;
  
  // get cell center dbl
  //G4double cell_dbl = hit_dim/cell_dim;
  //G4double cell_n;

  // get cell center int
  //std::modf(cell_dbl,&cell_n);

  G4double cell_n = floor(hit_dim/cell_dim);

  // G4double cell_cntr = .5*(cell_dim) + (cell_n * cell_dim);

  //G4cerr << "hit_dim (adj): " << hit_dim << G4endl;
  //G4cerr << "cell_dim:" << cell_dim << G4endl;
  //G4cerr << "cell_n: " << cell_n << G4endl;
  // G4cerr << "cell_cntr: " << cell_cntr << G4endl;
  
  // return (G4int)cell_cntr;
  return (G4int)cell_n;
}

// get cell # X from hit coord.
G4int TBCellReadout::GetCellNoX(G4double hit_x)
{
  return GetCellNo(hit_x, cell_x);
		   // , cal_hx);
}

// get cell # Y from hit coord.
G4int TBCellReadout::GetCellNoZ(G4double hit_z)
{
  return GetCellNo(hit_z, cell_z);
		   //, cal_hz);
}

// get X and Y cell nos. by reference
void TBCellReadout::GetCellNos(G4ThreeVector &hitPos, G4int &cnx, G4int &cnz)
{
  cnx = GetCellNoX(hitPos.x());
  cnz = GetCellNoZ(hitPos.z());
}

// get cell center coord. in (adjusted) local coords.
G4double TBCellReadout::GetLocalCenter(G4int cell_n, G4double cell_dim)
{  
  G4double cell_cntr = .5*(cell_dim) + (((G4int)cell_n) * cell_dim);
  // G4cerr << "cell_cntr (local): " << cell_cntr << G4endl;
  return cell_cntr;
}

// get cell center coord. X in local coords.
G4double TBCellReadout::GetLocalCenterX(G4int cnx)
{
  return GetLocalCenter(cnx, cell_x);
  //-cal_hx;
}

// get cell center coord. Y in local coords.
G4double TBCellReadout::GetLocalCenterZ(G4int cnz)
{
  return GetLocalCenter(cnz, cell_z);
  //-cal_hz;
}

// get cell center coords. X & Y
//void TBCellReadout::GetLocalCenters(G4int cnx, G4int cnz, G4double ccentx, G4double ccentz)
//{
  
//}
  
// local to global transform
//void TBCellReadout::GetGlobalTransform(G4ThreeVector &posLocal)
//{}
