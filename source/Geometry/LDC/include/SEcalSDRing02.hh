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
// $Id: SEcalSDRing02.hh,v 1.2 2008/10/20 13:40:52 mora Exp $
// $Name: mokka-07-00 $
//

#ifndef SEcalSDRing02_h
#define SEcalSDRing02_h 1

#include "G4ThreeVector.hh"
#include "SEcalSD02.hh"

class SEcalSDRing02 : public SEcalSD02
{
  
public:
  SEcalSDRing02(G4double Idim,G4double Jdim, G4double Thickness,
     G4int Piece,G4String ECSDRingname,G4bool id1Flag = false,
		G4bool preShower=true);
  ~SEcalSDRing02();
  
  G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
  G4ThreeVector GetCellCenter(G4int pP,G4int pS,G4int pM,
				G4int pI,G4int pJ,G4int pK);
private:
  G4bool WithPreShower;
};

#endif

