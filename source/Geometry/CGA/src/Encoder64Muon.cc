#include "Control.hh"
#include "Encoder64Muon.hh"
#include "CalHit.hh"

#include <assert.h>

#define SHIFT_S_64 0 //Stave = 4 bits
#define SHIFT_M_64 4 //Module = 3 bits
#define SHIFT_K_64 7 //Layer = 6 bits
#define SHIFT_I_64 13 //Cell X index = 16 bits
#define SHIFT_Z_64 29 //Guard-ring zone = 3 bits
#define SHIFT_J_64 0 //Cell Z index = 16 bits
#define SHIFT_P_64 16 //Provision = 15 bits
#define SHIFT_2_64 31 //Sign = 1 bit
                                                                                
#define MASK_S_64 (unsigned int) 0x0000000F
#define MASK_M_64 (unsigned int) 0x00000070
#define MASK_K_64 (unsigned int) 0x00001F80
#define MASK_I_64 (unsigned int) 0x1FFFE000
#define MASK_Z_64 (unsigned int) 0xE0000000
#define MASK_J_64 (unsigned int) 0x0000FFFF
#define MASK_P_64 (unsigned int) 0x7FFF0000
#define MASK_2_64 (unsigned int) 0x80000000

Encoder64Muon::Encoder64Muon(void):VEncoder(true) {
	idString="S-1:4,M:3,K-1:6,I:16,GRZone:3,J:32:16";
}

cell_ids Encoder64Muon::encode(G4int S, G4int M, G4int I , G4int J, G4int K, 
			   G4int GRZ) {

  G4int L2, Prov;
                                                                                
  assert(S<=16 && S>0);
  assert(M<8 && M>=0);
  assert(I<65535 && I>=0);
  assert(J<65535 && J>=0);
  assert(K<=64 && K>0);
  assert(GRZ<8 && GRZ>=0);
                                                                                
  L2 = 0;
  Prov = 0;
                                                                                
  cell_ids index;
                                                                                
  index.id0 = (unsigned int)
    (
          ( ((S-1) << SHIFT_S_64) & MASK_S_64) |
          ( (M << SHIFT_M_64) & MASK_M_64) |
          ( ((K-1) << SHIFT_K_64) & MASK_K_64) |
          ( (I << SHIFT_I_64) & MASK_I_64)|
          ( (GRZ << SHIFT_Z_64) & MASK_Z_64)
     );
                                                                                
  index.id1 =
     (
          ( (J << SHIFT_J_64) & MASK_J_64) |
          ( (Prov << SHIFT_P_64) & MASK_P_64) |
          ( (L2 << SHIFT_2_64) & MASK_2_64)
     );
                                                                                
  return index;
}

CalHit* Encoder64Muon::decode(cell_ids cellID) {

  G4int S, M, I, J, K, GRZone;
//  G4int L1, L2, Prov;
                                                                                
  S= (((unsigned int)(cellID.id0) &MASK_S_64) >> SHIFT_S_64) +1;
  M= (((unsigned int)(cellID.id0) &MASK_M_64) >> SHIFT_M_64);
  K= (((unsigned int)(cellID.id0) &MASK_K_64) >> SHIFT_K_64) +1;
  I= (((unsigned int)(cellID.id0) &MASK_I_64) >> SHIFT_I_64);
  GRZone= (((unsigned int)(cellID.id0) &MASK_Z_64) >> SHIFT_Z_64);

  J= (((unsigned int)(cellID.id1) &MASK_J_64) >> SHIFT_J_64);
//  Prov= ((cellID.id1 &MASK_P_64) >> SHIFT_P_64);
//  L2= ((cellID.id1 &MASK_2_64) >> SHIFT_2_64);


  return new CalHit(0, S, M, I, J, K, GRZone, 0, 0, 0, 0, 0, 0, 0, cellID);
}

