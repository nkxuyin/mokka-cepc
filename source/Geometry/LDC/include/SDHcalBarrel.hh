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

#ifndef SDHcalBarrel_h
#define SDHcalBarrel_h 1

#include "CalHit.hh"
#include "SD.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"

#define MAX_HALF_STAVES 16
#define MAX_DOUBLE_LAYERS 2*MAX_LAYERS
   
/** @class SDHcalBarrel SDHcalBarrel.hh "SDHcalBarrel.hh" 
    \brief Sensitive detector of the HCAL barrel
*/

class SDHcalBarrel : public SD
{
  public:

    /**Constructor
	@param IdimInt I dimension (x-axis) of the integer cell
	@param Jdim    I dimension (y-axis) of the cell (the same for integer and fractional cell)
	@param cellThickness cell thickness (z-axis); is the same for integer and fractional cells
	@param Piece number of the sensitive detector piece
	@param SDname name of the hits collection of the sensitive detector
	@param xOffset x-offset of the HCAL layers (the same for all layers)
	@param applyBirksLaw flag to indicate if Birks law should be applied or not
    */
  SDHcalBarrel(G4double IdimInt, G4double Jdim, G4double cellThickness,
	       G4int Piece, G4String SDname,
	       G4double xOffset,
	       G4bool applyBirksLaw=false);

  /**Destructor*/
    virtual ~SDHcalBarrel();

    /**Set stave rotation matrix*/
    void SetStaveRotationMatrix(G4int staveNumber, G4double phirot);

    /**Add HCAL layer, i.e. set the coordinates of the layers*/
    void AddLayer(G4int layerNumber, G4double X,G4double Y,G4double Z);

    /**Calculate local cell center (in the global coordinates system)
	@param dummy some integer, not used, but introduced due to compiler warnings 
	(without it, this GetCellCenter will hide the virtual GetCellCenter method defined in the 
	mother class Geometry/CGA/include/VSensitiveDetector.hh)
	@param stave_id stave id
	@param module_id module id
	@param I cell index I (on the x-axis)
	@param J cell index J (on the z-axis)
	@param K cell index K, equal to the layer number (on the y-axis)
	*/
    G4ThreeVector GetCellCenter(G4int dummy, G4int stave_id, G4int module_id, G4int I, G4int J, G4int K);

    /**Decode stave- and module-id's; the encoding is done in the HCAL superdriver, in the call of MyPlacement()
	in RegularBarrelChambers(), where moduleCopyNumber = HCALBARREL(=5)*100 + stave_id*10 + module_id
	@param moduleCopyNumber module copy number
	@param theSDPiece id of the sensitive detector piece 
	@param theStave stave id, with values between 1 and 16
	@param theModule module id, with values between 1 and 2
	*/
    void DecodeStaveModuleID(G4int moduleCopyNumber, 
			     G4int &theSDPiece, G4int &theStave, G4int &theModule);

    /**Redefine layer ID: in the geometry with 8 HCAL barrel staves, MAX_LAYERS=64; however,
	in the new geometry with 16 staves, the physical number of layers is 2*MAX_LAYERS; since
	all the codes assume MAX_LAYERS as being 64, the layer id is just reassigned,
	the exact position of the hit being given by the stave_id (i.e. stave_id = 1 => hit 
	is in older stave 1, left layers; stave_id=2 => hit is in old stave 1, right layers)
	*/
    void RedefineLayerID(G4int oldLayer, G4int countHcalLayers, G4int &newLayer);

    /**Process hits in an event (i.e. calculate hit position in a tile, get its energy,
	apply or not the Birks law, and create new hits in the hits collection)*/
    G4bool ProcessHits(G4Step *aStep, G4TouchableHistory *);

    /**Set the dimensions of the fractional cell: since the x-dimension of the 
	fractional tile is known only when the active x-length of the layer
	is calculated, this can be set only when looping over the layers*/
    void SetFractCellDimPerLayer(G4int layer_id, G4ThreeVector newFractCellDim);

    //======================================================
    G4ThreeVector DimIntCell;/**>Dimensions of integer cell*/
    G4ThreeVector *DimFractCellPerLayer[MAX_DOUBLE_LAYERS];/**<Store the 3 dimensions of a fractional cell, per layer*/
    G4int countHcalLayers;/**<Counter of the total number of HCAL barrel layers*/

    LayerRef *Layers[MAX_DOUBLE_LAYERS];/**<Class to save layer coordinates*/

    G4RotationMatrix *StavesRotationMatrices [MAX_HALF_STAVES];/**<Rotation matrices of the HCAL barrel modules*/
    G4RotationMatrix *InverseStavesRotationMatrices [MAX_HALF_STAVES];/**<Inverse of the HCAL barrel rotation matrices*/
    
    G4double StavesPhirots[MAX_HALF_STAVES];/**<Contains the phi rotation angles of the staves (for debugging)*/    
    //-------------------------------------------
    G4bool applyBirksLawFlag;/**<Flag to indicate if Birks law should be applied or not*/
    G4double theXoffset;/**<x-offset of the HCAL barrel layers*/
};

#endif

