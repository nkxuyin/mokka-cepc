//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for Tesla -      *
//*                                                     *
//* For more information about Mokka, please, go to the *
//*                                                     *
//*  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
//*                                                     *
//* Mokka home page.                                    *
//*                                                     *
//*******************************************************
//
// $Id: HECSD.hh,v 1.2 2003/12/12 13:44:12 musat Exp $
// $Name: mokka-07-00 $
//

#ifndef HECSD_h
#define HECSD_h 1

#include "G4ThreeVector.hh"
#include "SD.hh"

class HECSD : public SD
{
  
public:
  HECSD(G4double Idim,G4double Jdim, G4double Thickness,
     G4int Piece,G4String HECSDname);
  ~HECSD();
  
  G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
  G4ThreeVector GetCellCenter(G4int pP,G4int pS,G4int pM,
				G4int pI,G4int pJ,G4int pK);


};

#endif

