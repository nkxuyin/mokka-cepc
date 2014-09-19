#include "Control.hh"
#include "EncoderTB.hh"
#include "CalHit.hh"

#include <assert.h>

#define K_Protoshift 0  // K = 8 bits
#define J_Protoshift 8  // J = 8 bits
#define I_Protoshift 16 // I = 8 bits
                                                                                
#define K_ProtoMask (unsigned int) 0x000000FF 
#define J_ProtoMask (unsigned int) 0x0000FF00
#define I_ProtoMask (unsigned int) 0x00FF0000

EncoderTB::EncoderTB(void):VEncoder(false) {
	
	idString="K:8,J:8,I:8";
}

cell_ids EncoderTB::encode(G4int, G4int, G4int I , G4int J, G4int K, 
			   G4int) {

  assert((I<256) && (I>=0));
  assert((J<256) && (J>=0));
  assert((K<256) && (K>=0));

  cell_ids index;

  index.id0  = (( (I << I_Protoshift) & I_ProtoMask ) |
                ( (J << J_Protoshift) & J_ProtoMask ) |
                ( (K << K_Protoshift) & K_ProtoMask ) 
               );

  index.id1 = -2;

  return index;
}

CalHit* EncoderTB::decode(cell_ids index) {

  G4int I, J, K;

  G4int cellID = index.id0;

  I= ( (  (unsigned int) cellID & I_ProtoMask ) >> I_Protoshift);
  J= ( (  (unsigned int) cellID & J_ProtoMask ) >> J_Protoshift);
  K= ( (  (unsigned int) cellID & K_ProtoMask ) >> K_Protoshift);


  return new CalHit(0, 0, 0, I, J, K, 0, 0, 0, 0, 0, 0, 0, 0, index);
}

