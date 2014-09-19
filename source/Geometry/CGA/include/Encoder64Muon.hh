#ifndef Encoder64Muon_h
#define Encoder64Muon_h 1
                                                                                
#include "CGADefs.h"
#include "VEncoder.hh"

class CalHit;

class Encoder64Muon: public VEncoder {
public:
        Encoder64Muon(void);

	cell_ids encode(G4int, G4int, G4int, G4int, G4int, G4int=0);

	CalHit* decode(cell_ids);
};

#endif // Encoder64Muon_h
