#ifndef TBCellReadout_h
#define TBCellReadout_h 1

#include "globals.hh"
#include "G4ThreeVector.hh"

class TBCellReadout {

public:

  TBCellReadout();
  TBCellReadout(G4double cx, G4double cz, G4double calhx, G4double calhz);
  virtual ~TBCellReadout();

public:

  // general cell # utility
  G4int GetCellNo(G4double hit_dim, 
		  G4double cell_dim);
    //,
    //	  G4double cal_hdim);

  // get cell # X from hit coord.
  G4int GetCellNoX(G4double hit_x);

  // get cell # Y from hit coord.
  G4int GetCellNoZ(G4double hit_z);

  // get X and Y cell nos.
  void GetCellNos(G4ThreeVector &hitPos, G4int &cnx, G4int &cnz);

  G4double GetLocalCenter(G4int cnx, G4double cdim);

  // get cell center coord. X
  G4double GetLocalCenterX(G4int cnx);

  // get cell center coord. Y
  G4double GetLocalCenterZ(G4int cnz);

  // get cell center coords. X & Y
  void GetLocalCenters(G4int cnx, G4int cnz, G4double ccentx, G4double ccentz);
  
  // local to global transform
  // void GetGlobalTransform(G4ThreeVector &posLocal);

private:

  G4double cell_x, cell_z;
  G4double cal_hx, cal_hz;

  G4ThreeVector globalRef;
};

#endif
