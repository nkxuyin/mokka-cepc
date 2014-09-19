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
// $Id: SDAHcalEndCapScalable.hh,2011.12.05 S.Lu $
//
//

#ifndef SDAHcalEndCapScalable_h
#define SDAHcalEndCapScalable_h 1

#include "G4ThreeVector.hh"
#include "SD.hh"
#define SCALABLE_MAX_ENDCAP_MODULES_NUMBER 100
#define AHCAL_ENDCAP_MAX_STAVES 2

class SDAHcalEndCapScalable : public SD
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
    SDAHcalEndCapScalable(G4double Idim,G4double Jdim, G4double cellThickness,
			  G4int Piece,G4String SDname,
		          G4bool applyBirksLaw=false);

    /**
	Destructor
	*/
    ~SDAHcalEndCapScalable();
    
    /**
	Process hits in an event (i.e. calculate hit position in a tile, get its energy,
	apply or not the Birks law, and create new hits in the hits collection)
	*/
    G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);

    /**Calculate local cell center (in the coordinate system of the volume in which the hit is)
	@param pI cell index I (on the x-axis)
	@param pJ cell index J (on the y-axis)
	@param pK cell index K, equal to the layer number (on the z-axis)
	@param pM endcapID
	*/    
    G4ThreeVector GetLocalCellCenter(G4int pI,G4int pJ,G4int pK, G4int pM);
	
    /**Set Module Y-Offset, Module here means endCapID
        @param endCapModuleNumber the end cap module number [0,99] 
        @param Yoff end cap module y offset 
        */
    void SetModuleYOffset(G4int endCapModuleNumber,G4double Yoff);

    /**Set maxScalableEndcapModuleNumber, Module here means endCapID
        @param maxEndCapModuleNumber the max end cap module number [0,99] 
        */
    void SetmaxScalableEndcapModuleNumber(G4int maxEndCapModuleNumber);
	
    
  private: 
    G4bool applyBirksLawFlag;  
    G4double * ModulesYOffsets[SCALABLE_MAX_ENDCAP_MODULES_NUMBER];
    G4int maxScalableEndcapModuleNumber;

};
#endif

