#ifndef EncoderTBscecal01_h
#define EncoderTBscecal01_h 1
                                                                                
#include "CGADefs.h"
#include "VEncoder.hh"

class CalHit;

class EncoderTBscecal01: public VEncoder {
public:
        EncoderTBscecal01(void);

	cell_ids encode(G4int, G4int, G4int, G4int, G4int, G4int=0);

	CalHit* decode(cell_ids);
};

#endif // EncoderTBscecal01_h
