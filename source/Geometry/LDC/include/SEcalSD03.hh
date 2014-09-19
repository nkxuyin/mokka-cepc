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
// $Id: SEcalSD03.hh,v 1.3 2009/06/04 11:40:42 musat Exp $
// $Name: mokka-07-00 $
//

#ifndef SEcalSD03_h
#define SEcalSD03_h 1

#include "SEcalSD02.hh"
#include "G4ThreeVector.hh"

#define ECAL_SI_LAYERS                          '0'
#define ECAL_SC_LAYER_1_2_ALONG_X               '1'
#define ECAL_SC_LAYER_1_2_ALONG_Z               '2'
#define ECAL_SC_LAYER_1_ALONG_X_LAYER_2_ALONG_Z '3'
#define ECAL_SC_LAYER_1_ALONG_Z_LAYER_2_ALONG_X '4'

class SEcalSD03 : public SEcalSD02
{
  
public:
  SEcalSD03(G4double Idim, G4double Jdim, G4double Thickness,
	    G4int n_cells_i, G4int n_cells_j,
	    G4double GardRingSize, G4int the_n_strip_containers_along_z,
	    G4String the_Ecal_Sc_Si_mix,
	    G4double HWallSize, G4double TowerWallSize,
	    G4int Piece,G4String SDname,
	    G4bool id1Flag = false,
	    G4String BarrelSlabMode = "0110",
	    G4String ECSlabMode = "0110");
  virtual ~SEcalSD03();
  
  G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
  G4ThreeVector GetScintillatorCellCenter(G4int pP,G4int pS,G4int pM,
		  		G4int pI,G4int pJ,G4int pK,char direction);
  void Initialize(G4HCofThisEvent *);
  void EndOfEvent(G4HCofThisEvent*HCE);

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
  G4int HCID3;

//  HitsCollection *FirstLayerCalCollection;
  HitsCollection *NormalParallelToZCalCollection;
  HitsCollection *NormalParallelToXCalCollection;

};

#endif

