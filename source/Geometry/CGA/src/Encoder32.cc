#include "Control.hh"
#include "Encoder32.hh"
#include "CalHit.hh"

#include <assert.h>

#define SHIFT_M_32 0 // M = 3 bits
#define SHIFT_S_32 3 // S = 3 bits
#define SHIFT_I_32 6 // I = 9 bits
#define SHIFT_J_32 15 // J = 9 bits
#define SHIFT_K_32 24 // K = 6 bits
#define SHIFT_2_32 30 
#define SHIFT_1_32 31

#define MASK_M_32 (unsigned int) 0x00000007
#define MASK_S_32 (unsigned int) 0x00000038
#define MASK_I_32 (unsigned int) 0x00007FC0
#define MASK_J_32 (unsigned int) 0x00FF8000
#define MASK_K_32 (unsigned int) 0x3F000000
#define MASK_2_32 (unsigned int) 0x40000000
#define MASK_1_32 (unsigned int) 0x80000000
                                                                                
Encoder32::Encoder32(void):VEncoder(false) {

	idString="M:3,S-1:3,I:9,J:9,K-1:6";
}

cell_ids Encoder32::encode(G4int pS, G4int pM, G4int pI , G4int pJ, G4int pK, 
			   G4int) {

  assert(pS<=8 && pS>0);
  assert(pM<8 && pM>=0);
  assert(pI<512 && pI>=0);
  assert(pJ<512 && pJ>=0);
  assert(pK<=64 && pK>0);

  G4int L1 = 0 , L2 = 0;
  
  cell_ids index;

  index.id0  = (
          ( (L1 << SHIFT_1_32) & MASK_1_32) |
          ( (L2 << SHIFT_2_32) & MASK_2_32) |
          ( ((pS-1) << SHIFT_S_32) & MASK_S_32) |
          ( ((pM) << SHIFT_M_32) & MASK_M_32) |
          ( (pI << SHIFT_I_32) & MASK_I_32) |
          ( (pJ << SHIFT_J_32) & MASK_J_32) |
          ( ((pK-1) << SHIFT_K_32) & MASK_K_32)
               );

  index.id1 = -1;

  return index;
}

CalHit* Encoder32::decode(cell_ids index) {

	G4int S, M, I, J, K;

	G4int cellID = index.id0;

        K= ((((unsigned int)cellID)&MASK_K_32) >> SHIFT_K_32) +1;
        J= ((((unsigned int)cellID)&MASK_J_32) >> SHIFT_J_32);
        I= ((((unsigned int)cellID)&MASK_I_32) >> SHIFT_I_32);
        M= ((((unsigned int)cellID)&MASK_M_32) >> SHIFT_M_32);
        S= ((((unsigned int)cellID)&MASK_S_32) >> SHIFT_S_32) +1;

	return new CalHit(0, S, M, I, J, K, 0, 0, 0, 0, 0, 0, 0, 0, index);
}

