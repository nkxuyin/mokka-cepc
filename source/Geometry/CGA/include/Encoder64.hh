#ifndef Encoder64_h
#define Encoder64_h 1
                                                                                
#include "CGADefs.h"
#include "VEncoder.hh"

class CalHit;

class Encoder64: public VEncoder {
public:
        Encoder64(void);

	cell_ids encode(G4int, G4int, G4int, G4int, G4int, G4int=0);

	CalHit* decode(cell_ids);
};

#endif // Encoder64_h
