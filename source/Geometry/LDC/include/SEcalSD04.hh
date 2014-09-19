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
// $Id: SEcalSD04.hh,v 1.3 2009/06/04 11:40:42 musat Exp $
// $Name: mokka-07-00 $
/*
  March 2013: add posibility of having different cell sizes per layer, in case of
  using scintillator as active material (Angela Lucaci)

  Updated version 04 : Daniel Jeans
  allows layer-by-layer configuration si-scint
  scalable endcap geometry
*/

#ifndef SEcalSD04_h
#define SEcalSD04_h 1

#include "SEcalSD02.hh"
#include "G4ThreeVector.hh"

#if G4_VERSION_GE( 920 )
#include "G4EmSaturation.hh"
#else
#include "../../Kernel/G4EmSaturation.hh"
#endif

#define ECAL_SI_LAYERS                          '0'
#define ECAL_SC_LAYER_1_2_ALONG_X               '1'
#define ECAL_SC_LAYER_1_2_ALONG_Z               '2'
#define ECAL_SC_LAYER_1_ALONG_X_LAYER_2_ALONG_Z '3'
#define ECAL_SC_LAYER_1_ALONG_Z_LAYER_2_ALONG_X '4'
#define ECAL_MIX_LAYER_1_SI_LAYER_2_ALONG_X     '5'
#define ECAL_MIX_LAYER_1_SI_LAYER_2_ALONG_Z     '6'
#define ECAL_MIX_LAYER_1_ALONG_X_LAYER_2_SI     '7'
#define ECAL_MIX_LAYER_1_ALONG_Z_LAYER_2_SI     '8'


class SEcalSD04 : public SEcalSD02
{
public:
  SEcalSD04(G4double Idim, 
	    G4double Jdim, 
	    G4double Thickness,
	    G4int n_cells_i, 
	    G4int n_cells_j,
	    G4double GuardRingSize, 
	    G4int the_n_strip_containers_along_z,
	    G4String the_Ecal_Sc_Si_mix,
	    G4double HWallSize, 
	    G4double TowerWallSize,
	    G4int Piece,
	    G4String SDname,
	    G4bool id1Flag = false,
	    G4String BarrelSlabMode = "0110",
	    G4String ECSlabMode = "0110");
  virtual ~SEcalSD04();
  
  G4bool ProcessHits(G4Step*aStep, G4TouchableHistory *ROhist);
  G4ThreeVector GetScintillatorCellCenter(G4int pP, G4int pS, G4int pM,
					  G4int pI, G4int pJ, G4int pK, char direction);
  void Initialize(G4HCofThisEvent *);
  void EndOfEvent(G4HCofThisEvent *HCE);

  /*Angela Lucaci: add Birks law for scintillators
   and add possibility of having different cell sizes in case of scintillator as active material*/
  G4double GetBirksAttenuatedEnergy(const G4Step *aStep);

  typedef std::vector<double> DoubleVector;
  typedef std::vector<int> IntVector;

  /**
   *    @brief  Populate lists of cell parameters for each layer
   * 
   *    @param  Ecal_Sc_cellDim1_vector
   *    @param  Ecal_Sc_cellDim2_vector
   *    @param  Ecal_Sc_N_strips_across_module_vector
   *    @param  Ecal_Sc_N_strip_containers_along_z_vector
   */
  void FillScintillatorCellSizesVectors(const DoubleVector &Ecal_Sc_cellDim1_vector, const DoubleVector &Ecal_Sc_cellDim2_vector,
        const IntVector &Ecal_Sc_N_strips_across_module_vector, const IntVector &Ecal_Sc_N_strip_containers_along_z_vector);

  DoubleVector  theEcal_Sc_cellDim1_vector;
  DoubleVector  theEcal_Sc_cellDim2_vector;
  IntVector     theEcal_Sc_N_strips_across_module_vector;
  IntVector     theEcal_Sc_N_strip_containers_along_z_vector;

  G4double stripSizeinX;
  G4double stripSizeParallelToZ;
  G4double Ecal_Sc_thickness;
  G4int    Ecal_Sc_N_strips_across_module;
  G4int    Ecal_Sc_number_of_virtual_cells;
  G4double Ecal_Sc_reflector_thickness;
  G4double virtualCellDim;
  G4int    n_strip_containers_along_z;

  G4bool FirstLayerCollectionFlag;
  G4bool ParallelToZFlag;
  G4bool ParallelToXFlag;
  G4String Ecal_Sc_Si_mix;
  G4EmSaturation *emSaturation;
  G4String theSDname;
  G4int HCID3;

  HitsCollection *NormalParallelToZCalCollection;
  HitsCollection *NormalParallelToXCalCollection;

};

#endif

