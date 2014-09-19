// *********************************************************
// *        --- Mokka --- http://mokka.in2p3.fr ---        *
// *    -- A Detailed Geant4 Simulation for the ILC --     *
// *********************************************************
//
// $Id: EUTelescope.hh,v 1.1 2008/10/24 11:45:20 tatsiana Exp $
// $Name: mokka-07-00 $

#ifndef EUTelescope_hh
#define EUTelescope_hh 1

#include "VSubDetectorDriver.hh"

class EUTelescope: public VSubDetectorDriver
{
public:
    EUTelescope(void): VSubDetectorDriver("eutelescope", "telescope") {}
    ~EUTelescope(void) {}
    
    G4bool ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog);
    
private:
    
#ifdef MOKKA_GEAR
    struct helpLayer {
	G4int    ID ;
	G4double positionX ;
	G4double positionY ;
	G4double positionZ ;
	G4double sizeX ;
	G4double sizeY ;
	G4double thickness ;
	G4double radLength ;
    };
    
    struct helpSensitive {
	G4int    ID ;
	G4double positionX ;
	G4double positionY ;
	G4double positionZ ;
	G4double sizeX ;
	G4double sizeY ;
	G4double thickness ;
	G4int    npixelX ;
	G4int    npixelY ;
	G4double pitchX ;
	G4double pitchY ;
	G4double resolution ;
	G4double rotation1 ;
	G4double rotation2 ;
	G4double rotation3 ;
	G4double rotation4 ;
	G4double radLength ;
    };
#endif
    
};
#endif // EUTelescope_hh
