#include "Control.hh"
#include "Encoder32Hcal.hh"
#include "CalHit.hh"

#include <assert.h>

#define SHIFT_M_32 0  // M = 3 bits
#define SHIFT_S_32 3  // S = 4 bits
#define SHIFT_I_32 7  // I = 9 bits
#define SHIFT_J_32 16 // J = 9 bits
#define SHIFT_K_32 25 // K = 7 bits

#define MASK_M_32 (unsigned int) 0x00000007
#define MASK_S_32 (unsigned int) 0x00000078
#define MASK_I_32 (unsigned int) 0x0000FF80
#define MASK_J_32 (unsigned int) 0x00FF8000
#define MASK_K_32 (unsigned int) 0xFE000000
                                                                                

Encoder32Hcal::Encoder32Hcal(void):VEncoder(false) {
  idString="M:3,S-1:4,I:9,J:9,K-1:7";
}

cell_ids Encoder32Hcal::encode(G4int pS, G4int pM, G4int pI , G4int pJ, G4int pK, 
			       G4int) {

  assert(pS <= 16 && pS > 0);
  assert(pM < 8   && pM >= 0);
  assert(pI < 512 && pI >= 0);
  assert(pJ < 512 && pJ >= 0);
  assert(pK <=128 && pK > 0);
  
  cell_ids index;

  index.id0  = (
		( ((pM)   << SHIFT_M_32) & MASK_M_32) |
		( ((pS-1) << SHIFT_S_32) & MASK_S_32) |
		( (pI     << SHIFT_I_32) & MASK_I_32) |
		( (pJ     << SHIFT_J_32) & MASK_J_32) |
		( ((pK-1) << SHIFT_K_32) & MASK_K_32)
		);
  
  index.id1 = -1;

  return index;
}

CalHit* Encoder32Hcal::decode(cell_ids index) {

	G4int S, M, I, J, K;

	G4int cellID = index.id0;

        K = ((((unsigned int)cellID) & MASK_K_32) >> SHIFT_K_32) + 1;
        J = ((((unsigned int)cellID) & MASK_J_32) >> SHIFT_J_32);
        I = ((((unsigned int)cellID) & MASK_I_32) >> SHIFT_I_32);
        M = ((((unsigned int)cellID) & MASK_M_32) >> SHIFT_M_32);
        S = ((((unsigned int)cellID) & MASK_S_32) >> SHIFT_S_32) + 1;

	return new CalHit(0, S, M, I, J, K, 0, 0, 0, 0, 0, 0, 0, 0, index);
}

