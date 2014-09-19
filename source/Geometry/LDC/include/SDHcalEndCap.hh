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
// $Id: SDHcalEndCap.hh,v 1.2 2008/04/10 16:42:04 angela Exp $
// $Name: mokka-07-00 $
//

#ifndef SDHcalEndCap_h
#define SDHcalEndCap_h 1

#include "G4ThreeVector.hh"
#include "SD.hh"

class SDHcalEndCap : public SD
{
  
public:
    SDHcalEndCap(G4double Idim,G4double Jdim, G4double Thickness,
		 G4int Piece,G4String SDHcalEndCapname,
         G4double StaveGapWidth, G4double CenterBoxWidth,
		 G4bool applyBirksLaw=false);
    ~SDHcalEndCap();
    
    G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
    G4ThreeVector GetCellCenter(G4int pP,G4int pS,G4int pM,
				G4int pI,G4int pJ,G4int pK);
    
  private: 
    const G4double EndcapStaveGap; //width of the gap between staves 
                                   //(distance of stave from axis is half the width)
    const G4double CenterBox;      //half length of a side of the center box
                                   //hits inside the box square will not be processed
    G4bool applyBirksLawFlag;
};

#endif

