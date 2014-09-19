#include "Control.hh"
#include "Encoder64.hh"
#include "CalHit.hh"

#include <assert.h>

#define SHIFT_S_64 0 //Stave = 3 bits
#define SHIFT_M_64 3 //Module = 3 bits
#define SHIFT_K_64 6 //Layer = 6 bits
#define SHIFT_I_64 12 //Cell X index = 16 bits
#define SHIFT_Z_64 28 //Guard-ring zone = 3 bits
#define SHIFT_1_64 31 //Sign = 1 bit
#define SHIFT_J_64 0 //Cell Z index = 16 bits
#define SHIFT_P_64 16 //Provision = 15 bits
#define SHIFT_2_64 31 //Sign = 1 bit
                                                                                
#define MASK_S_64 (unsigned int) 0x00000007
#define MASK_M_64 (unsigned int) 0x00000038
#define MASK_K_64 (unsigned int) 0x00000FC0
#define MASK_I_64 (unsigned int) 0x0FFFF000
#define MASK_Z_64 (unsigned int) 0x70000000
#define MASK_1_64 (unsigned int) 0x80000000
#define MASK_J_64 (unsigned int) 0x0000FFFF
#define MASK_P_64 (unsigned int) 0x7FFF0000
#define MASK_2_64 (unsigned int) 0x80000000

Encoder64::Encoder64(void):VEncoder(true) {
	idString="S-1:3,M:3,K-1:6,I:16,GRZone:3,J:32:16";
}

cell_ids Encoder64::encode(G4int S, G4int M, G4int I , G4int J, G4int K, 
			   G4int GRZ) {

  G4int L1, L2, Prov;
                                                                                
  assert(S<=8 && S>0);
  assert(M<8 && M>=0);
  assert(I<65535 && I>=0);
  assert(J<65535 && J>=0);
  assert(K<=64 && K>0);
  assert(GRZ<8 && GRZ>=0);
                                                                                
  L1 = 0;
  L2 = 0;
  Prov = 0;
                                                                                
  cell_ids index;
                                                                                
  index.id0 =
    (
          ( ((S-1) << SHIFT_S_64) & MASK_S_64) |
          ( (M << SHIFT_M_64) & MASK_M_64) |
          ( ((K-1) << SHIFT_K_64) & MASK_K_64) |
          ( (I << SHIFT_I_64) & MASK_I_64)|
          ( (GRZ << SHIFT_Z_64) & MASK_Z_64) |
          ( (L1 << SHIFT_1_64) & MASK_1_64)
     );
                                                                                
  index.id1 =
     (
          ( (J << SHIFT_J_64) & MASK_J_64) |
          ( (Prov << SHIFT_P_64) & MASK_P_64) |
          ( (L2 << SHIFT_2_64) & MASK_2_64)
     );
                                                                                
  return index;
}

CalHit* Encoder64::decode(cell_ids cellID) {

  G4int S, M, I, J, K, GRZone;
//  G4int L1, L2, Prov;
                                                                                
  S= ((cellID.id0 &MASK_S_64) >> SHIFT_S_64) +1;
  M= ((cellID.id0 &MASK_M_64) >> SHIFT_M_64);
  K= ((cellID.id0 &MASK_K_64) >> SHIFT_K_64) +1;
  I= ((cellID.id0 &MASK_I_64) >> SHIFT_I_64);
  GRZone= ((cellID.id0 &MASK_Z_64) >> SHIFT_Z_64);
//  L1= ((cellID.id0 &MASK_1_64) >> SHIFT_1_64);

  J= ((cellID.id1 &MASK_J_64) >> SHIFT_J_64);
//  Prov= ((cellID.id1 &MASK_P_64) >> SHIFT_P_64);
//  L2= ((cellID.id1 &MASK_2_64) >> SHIFT_2_64);


  return new CalHit(0, S, M, I, J, K, GRZone, 0, 0, 0, 0, 0, 0, 0, cellID);
}

