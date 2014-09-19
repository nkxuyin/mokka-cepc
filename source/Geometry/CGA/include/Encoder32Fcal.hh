#ifndef Encoder32Fcal_h
#define Encoder32Fcal_h 1
                                                                                
#include "CGADefs.h"
#include "VEncoder.hh"

class CalHit;

class Encoder32Fcal: public VEncoder {
public:
        Encoder32Fcal(void);

	cell_ids encode(G4int, G4int, G4int, G4int, G4int, G4int=0);

	CalHit* decode(cell_ids);
};

#endif // Encoder32Fcal_h
