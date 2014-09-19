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
// $Id: ECSD03.hh,v 1.4 2006/03/01 14:13:30 musat Exp $
// $Name: mokka-07-00 $
//

#ifndef ECSD03_h
#define ECSD03_h 1

#include "G4ThreeVector.hh"
#include "SD03.hh"

class ECSD03 : public SD03
{
  
public:
  ECSD03(G4double Idim,G4double Jdim, G4double Thickness,
     G4double guard_ring_size, G4double inter_wafer_gap,
     G4int nmax_cell_x, G4int nmax_cell_z, G4int n_guard_ring_zones,
     G4int Piece,G4String ECSDname, G4bool useID1=false);
  ~ECSD03();
  
  G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
  G4ThreeVector GetCellCenter(G4int pP,G4int pS,G4int pM,
				G4int pI,G4int pJ,G4int pK);
  G4bool GetCellIndices(const G4VTouchable * theTouchable,
#ifdef MOKKA_DEBUG
		G4ThreeVector& thePosition, 
#endif
		G4int& theSDPiece, G4int& theStave, 
		G4int& theModule, G4int& I, G4int& J, G4int& theLayer); 

cell_ids GetCellIndex(double X, double Y, double Z,
		int & flag, double xDir, double yDir, double zDir);

void GetNearestCell(const G4VTouchable * theTouchable,
		G4ThreeVector thePosition,
		G4int& P, G4int& S, G4int& M, G4int& I, G4int& J, G4int& K,
		G4int & zone);
};

#endif

