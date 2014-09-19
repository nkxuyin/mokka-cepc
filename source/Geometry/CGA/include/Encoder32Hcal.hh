#ifndef Encoder32Hcal_h
#define Encoder32Hcal_h 1
                                                                                
#include "CGADefs.h"
#include "VEncoder.hh"

class CalHit;

class Encoder32Hcal: public VEncoder {
public:
    Encoder32Hcal(void);
    
    cell_ids encode(G4int, G4int, G4int, G4int, G4int, G4int=0);
    
    CalHit* decode(cell_ids);
};

#endif // Encoder32Hcal_h
