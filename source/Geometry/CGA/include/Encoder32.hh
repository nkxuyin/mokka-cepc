#ifndef Encoder32_h
#define Encoder32_h 1
                                                                                
#include "CGADefs.h"
#include "VEncoder.hh"

class CalHit;

class Encoder32: public VEncoder {
public:
        Encoder32(void);

	cell_ids encode(G4int, G4int, G4int, G4int, G4int, G4int=0);

	CalHit* decode(cell_ids);
};

#endif // Encoder32_h
