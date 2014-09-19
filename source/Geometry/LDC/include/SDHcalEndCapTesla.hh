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
// $Id: SDHcalEndCapTesla.hh,v 1.2 2008/10/22 14:38:15 angela Exp $
// $Name: mokka-07-00 $
//

#ifndef SDHcalEndCapTesla_h
#define SDHcalEndCapTesla_h 1

#include "G4ThreeVector.hh"
#include "SD.hh"

class SDHcalEndCapTesla : public SD
{
  
public:
 
    /**Constructor
	@param Idim I dimension (x-axis) of the integer cell
	@param Jdim J dimension (y-axis) of the cell (the same for integer and fractional cell)
	@param cellThickness cell thickness (z-axis); 
	@param Piece number of the sensitive detector piece
	@param SDname name of the hits collection of the sensitive detector
	@param applyBirksLaw flag to indicate if Birks law should be applied or not
	*/
    SDHcalEndCapTesla(G4double Idim,G4double Jdim, G4double cellThickness,
		      G4int Piece,G4String SDname,
		      G4bool applyBirksLaw=false);

    /**
	Destructor
	*/
    ~SDHcalEndCapTesla();
    
    /**
	Process hits in an event (i.e. calculate hit position in a tile, get its energy,
	apply or not the Birks law, and create new hits in the hits collection)
	*/
    G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);

    /**Calculate local cell center (in the coordinate system of the volume in which the hit is)
	@param pI cell index I (on the x-axis)
	@param pJ cell index J (on the y-axis)
	@param pK cell index K, equal to the layer number (on the z-axis)
	*/    
    G4ThreeVector GetLocalCellCenter(G4int pI,G4int pJ,G4int pK);

    
  private: 
    G4bool applyBirksLawFlag;  

};
#endif

