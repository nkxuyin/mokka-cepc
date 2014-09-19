#include "Control.hh"
#include "EncoderTBscecal01.hh"
#include "CalHit.hh"

#include <assert.h>

#define K_Protoshift 0  // K = 8 bits
#define J_Protoshift 8  // J = 8 bits
#define I_Protoshift 16 // I = 8 bits
                                                                                
#define K_ProtoMask (unsigned int) 0x000000FF 
#define J_ProtoMask (unsigned int) 0x0000FF00
#define I_ProtoMask (unsigned int) 0x00FF0000

// Refer to EncorderTB at .../Geometry/CGA/src(include)/EncoderTB.cc(hh) 

EncoderTBscecal01::EncoderTBscecal01(void):VEncoder(false) {
	
	idString="K:8,J:8,I:8";
}

cell_ids EncoderTBscecal01::encode(G4int, G4int, G4int, G4int J, G4int K, 
			   G4int) {

  //assert((I<256) && (I>=0));
  assert((J<256) && (J>=0));
  assert((K<256) && (K>=0));

  cell_ids index;

  index.id0  = (( (J << J_Protoshift) & J_ProtoMask ) |
                ( (K << K_Protoshift) & K_ProtoMask ) 
               );

//120228. Strip no also from the first digit. --> using K_Protoshift, K_ProtoMask
  index.id1  = (
                ( (J << K_Protoshift) & K_ProtoMask ) 
               );

  return index;
}

CalHit* EncoderTBscecal01::decode(cell_ids index) {

  G4int J, K;

  G4int cellID0 = index.id0;
  G4int cellID1 = index.id1;

  K= ( (  (unsigned int) cellID0 & K_ProtoMask ) >> K_Protoshift);
  J= ( (  (unsigned int) cellID1 & K_ProtoMask ) >> K_Protoshift);


  return new CalHit(0, 0, 0, J, K, 0, 0, 0, 0, 0, 0, 0, 0, 0, index);
}

