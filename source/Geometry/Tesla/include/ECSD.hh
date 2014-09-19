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
// $Id: ECSD.hh,v 1.3 2006/03/02 16:23:40 musat Exp $
// $Name: mokka-07-00 $
//

#ifndef ECSD_h
#define ECSD_h 1

#include "G4ThreeVector.hh"
#include "SD.hh"

class ECSD : public SD
{
  
public:
  ECSD(G4double Idim,G4double Jdim, G4double Thickness,
     G4int Piece,G4String ECSDname,G4bool id1Flag = false);
  ~ECSD();
  
  G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
  G4ThreeVector GetCellCenter(G4int pP,G4int pS,G4int pM,
				G4int pI,G4int pJ,G4int pK);

};

#endif

