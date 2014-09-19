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
// $Id: SDHcalECSD01.hh,v 1.1 2009/02/05 16:32:15 musat Exp $
// $Name: mokka-07-00 $
//

#ifndef SDHcalECSD01
#define SDHcalECSD01_h 1

#include "G4ThreeVector.hh"
#include "SD.hh"

class SDHcalECSD01 : public SD
{
  
public:
  SDHcalECSD01(G4double Idim,G4double Jdim, G4double Thickness,
     G4int Piece,G4String SDHcalECSD01name, G4double padSpacing=0);
  ~SDHcalECSD01();
  
  G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
  G4ThreeVector GetCellCenter(G4int pP,G4int pS,G4int pM,
				G4int pI,G4int pJ,G4int pK);

protected:
  G4double PadSpacing;
};

#endif

