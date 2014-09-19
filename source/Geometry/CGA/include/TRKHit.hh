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
// $Id: TRKHit.hh,v 1.5 2006/06/23 15:35:28 musat Exp $
// $Name: mokka-07-00 $
//
// It's a first approach for hit's for all the tracking
// detectors (VXD, SIT, FTD and TPC). In this first release
// it keeps:
// 
// o the layer number (the plan number for the FTD)
// o the mean step position when crossing the layer
// o the mean momentum when crossing the layer
// o the primary PID number
// o the PDG particle code (it can be the secondary one)
// o the total energy deposited when crossing the layer
//
// (Paulo Sept. 2002)
// 19/05/2004 F. Gaede - added time information
//
#ifndef TRKHit_h
#define TRKHit_h 1

#include "VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "G4Circle.hh"

class TRKHit : public VHit
{
public:
  TRKHit() : 
#ifdef LCIO_MODE
    VHit( LCIO::SIMTRACKERHIT ),
#endif
    Layer(0),X(0),Y(0),Z(0),
    Px(0),Py(0),Pz(0),PID(0),
    PDG(0),Time(0.),
    StepLength(0.)
  {SetEnergy(0.0);}
  
  TRKHit(G4int pLayer, G4double pX, G4double pY, G4double pZ,
	 G4double pPx, G4double pPy, G4double pPz, 
	 G4int pPID, G4int pPDG,G4double pEnergy, G4double pTime,
	 G4double pStepLength)
    :  
#ifdef LCIO_MODE
    VHit( LCIO::SIMTRACKERHIT ),
#endif
    Layer(pLayer),X(pX),Y(pY),Z(pZ),
    Px(pPx),Py(pPy),Pz(pPz),PID(pPID),
    PDG(pPDG),Time(pTime),
    StepLength(pStepLength)
  {SetEnergy(pEnergy);}
  
  ~TRKHit(){}
  
  TRKHit(const TRKHit &right);
  const TRKHit& operator=(const TRKHit &right);
  int operator==(const TRKHit &right) const;
  
  inline void *operator new(size_t);
  inline void operator delete(void *aTRKHit);
  
  void Draw();
  void Print();
  void Save(FILE *oFile);

#ifdef LCIO_MODE
  virtual void Save(LCCollectionVec* aLCCollectionVec);
#endif

  G4bool Load(FILE *iFile);
  
private:
  G4int Layer;       // Layer number
  G4double X,Y,Z;    // mean coordinates in space
  G4double Px,Py,Pz; // Momemtum when entring the layer
  G4int PID;         // primary id
  G4int PDG;         // particle PDG (can be one of secondaries)
  //VHit::Energy = Total energy when crossing the layer
  G4double Time ;    // time of the hit (in the lab, relative to the event)
  G4double StepLength ; // Total step length in this hit

public:

  inline G4int GetLayer() const {return Layer;} 
  
  inline G4double GetX() const {return X;} 
  inline G4double GetY() const {return Y;} 
  inline G4double GetZ() const {return Z;} 
  
  inline G4double GetPx() const {return Px;} 
  inline G4double GetPy() const {return Py;} 
  inline G4double GetPz() const {return Pz;} 
  
  inline G4int GetPID() const {return PID;} 
  inline G4int GetPDG() const {return PDG;} 

  inline G4double GetTime() const {return Time ;} 

};

typedef G4THitsCollection<TRKHit> TRKHitsCollection;

extern G4Allocator<TRKHit> TRKHitAllocator;

inline void* TRKHit::operator new(size_t)
{
  void *aTRKHit;
  aTRKHit = (void *) TRKHitAllocator.MallocSingle();
  return aTRKHit;
}

inline void TRKHit::operator delete(void *aTRKHit)
{
  TRKHitAllocator.FreeSingle((TRKHit*) aTRKHit);
}

#endif


