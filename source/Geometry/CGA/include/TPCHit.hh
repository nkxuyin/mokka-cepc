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
// $Id: TPCHit.hh,v 1.3 2003/07/25 15:11:04 mora Exp $
// $Name: mokka-07-00 $
//

#ifndef TPCHit_h
#define TPCHit_h 1

#include "VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "G4Circle.hh"

class TPCHit : public VHit
{
public:
  TPCHit() :  
#ifdef LCIO_MODE
    VHit( LCIO::SIMTRACKERHIT ),
#endif
    P(0),K(0),X(0),Y(0),Z(0),
    Px(0),Py(0),Pz(0),PID(0),PDG(0) {}
  
  TPCHit(G4int pP,G4int pK,G4double pX,G4double pY,G4double pZ,
	 G4double pPx,G4double pPy,G4double pPz, G4int pPID,G4int pPDG)
    : 
#ifdef LCIO_MODE
    VHit( LCIO::SIMTRACKERHIT ),
#endif
    P(pP),K(pK),X(pX),Y(pY),Z(pZ),
    Px(pPx),Py(pPy),Pz(pPz),PID(pPID),PDG(pPDG)
  {}
  
  ~TPCHit(){}
  
  TPCHit(const TPCHit &right);
  const TPCHit& operator=(const TPCHit &right);
  int operator==(const TPCHit &right) const;
  
  inline void *operator new(size_t);
  inline void operator delete(void *aTPCHit);
  
  void Draw();
  void Print();
  void Save(FILE *oFile);
#ifdef LCIO_MODE
  void Save(LCCollectionVec* )
  {
    Control::Abort("VHit::Save(LCCollectionVec*) NOT IMPLEMENTED!",
	MOKKA_ERROR_INCOMPLETE_DERIVED_CLASS);
  }
#endif

  G4bool Load(FILE *iFile);
  
private:
  
  G4int P,K; // Hit type and layer number
  G4double X,Y,Z;    // cell coordinates in space
  G4double Px,Py,Pz;    // cell coordinates in space
  G4int PID;         // primary id
  G4int PDG;         // particle PDG
  
public:

  inline G4int GetP() const {return P;} 
  inline G4int GetK() const {return K;} 
  
  inline G4double GetX() const {return X;} 
  inline G4double GetY() const {return Y;} 
  inline G4double GetZ() const {return Z;} 
  
  inline G4double GetPx() const {return Px;} 
  inline G4double GetPy() const {return Py;} 
  inline G4double GetPz() const {return Pz;} 
  
  inline G4int GetPID() const {return PID;} 
  inline G4int GetPDG() const {return PDG;} 
};

typedef G4THitsCollection<TPCHit> TPCHitsCollection;

extern G4Allocator<TPCHit> TPCHitAllocator;

inline void* TPCHit::operator new(size_t)
{
  void *aTPCHit;
  aTPCHit = (void *) TPCHitAllocator.MallocSingle();
  return aTPCHit;
}

inline void TPCHit::operator delete(void *aTPCHit)
{
  TPCHitAllocator.FreeSingle((TPCHit*) aTPCHit);
}

#endif


