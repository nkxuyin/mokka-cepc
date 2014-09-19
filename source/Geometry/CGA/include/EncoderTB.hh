#ifndef EncoderTB_h
#define EncoderTB_h 1
                                                                                
#include "CGADefs.h"
#include "VEncoder.hh"

class CalHit;

class EncoderTB: public VEncoder {
public:
        EncoderTB(void);

	cell_ids encode(G4int, G4int, G4int, G4int, G4int, G4int=0);

	CalHit* decode(cell_ids);
};

#endif // EncoderTB_h
