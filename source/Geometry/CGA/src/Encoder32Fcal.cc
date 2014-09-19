#include "Control.hh"
#include "Encoder32Fcal.hh"
#include "CalHit.hh"

#include <assert.h>

#define SHIFT_I_32Fcal 0 // I = 10 bits
#define SHIFT_J_32Fcal 10 // J = 10 bits
#define SHIFT_K_32Fcal 20 // K = 10 bits
#define SHIFT_S_32Fcal 30 // S = 2 bits

#define MASK_I_32Fcal (unsigned int) 0x000003FF
#define MASK_J_32Fcal (unsigned int) 0x000FFC00
#define MASK_K_32Fcal (unsigned int) 0x3FF00000
#define MASK_S_32Fcal (unsigned int) 0xC0000000
                                                                                
Encoder32Fcal::Encoder32Fcal(void):VEncoder(false) {

	idString="I:10,J:10,K:10,S-1:2";
}

cell_ids Encoder32Fcal::encode(
		G4int pS, G4int pI , G4int pJ, G4int pK, G4int, G4int) {

  assert(pS<=2 && pS>0);
  assert(pI<=1023 && pI>=0);
  assert(pJ<=1023 && pJ>=0);
  assert(pK<=1023 && pK>0);

  cell_ids index;

  index.id0  = (
          ( ((pS-1) << SHIFT_S_32Fcal) & MASK_S_32Fcal) |
          ( (pI << SHIFT_I_32Fcal) & MASK_I_32Fcal) |
          ( (pJ << SHIFT_J_32Fcal) & MASK_J_32Fcal) |
          ( (pK << SHIFT_K_32Fcal) & MASK_K_32Fcal)
               );

  index.id1 = -1;

  return index;
}

CalHit* Encoder32Fcal::decode(cell_ids index) {

	G4int S, I, J, K;

	G4int cellID = index.id0;

        I= ((((unsigned int)cellID)&MASK_I_32Fcal) >> SHIFT_I_32Fcal);
        J= ((((unsigned int)cellID)&MASK_J_32Fcal) >> SHIFT_J_32Fcal);
        K= ((((unsigned int)cellID)&MASK_K_32Fcal) >> SHIFT_K_32Fcal);
        S= ((((unsigned int)cellID)&MASK_S_32Fcal) >> SHIFT_S_32Fcal) +1;

	return new CalHit(0, S, 0, I, J, K, 0, 0, 0, 0, 0, 0, 0, 0, index);
}

