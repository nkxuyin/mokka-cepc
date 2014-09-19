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
// $Id: Trajectory.hh,v 1.2 2003/07/25 13:10:31 mora Exp $
// $Name: mokka-07-00 $
//
//---------------------------------------------------------------
//
// Trajectory.hh is a special implementation for G4Trajectory.hh
// just to enable building a Trajectory object given it's
// private's attributs. Mokka needs it to reload trajetories,
// for visualisation.
//
//---------------------------------------------------------------

#ifndef Trajectory_h
#define Trajectory_h 1

#include "G4VTrajectory.hh"
#include "G4Allocator.hh"
#include <stdlib.h>                 // Include from 'system'
#include "G4ios.hh"               // Include from 'system'
#include <vector>
#include "globals.hh"               // Include from 'global'
#include "G4ParticleDefinition.hh"  // Include from 'particle+matter'
#include "G4TrajectoryPoint.hh"     // Include from 'tracking'
#include "G4Track.hh"
#include "G4Step.hh"

class G4Polyline;                   // Forward declaration.

typedef std::vector<G4VTrajectoryPoint*>  TrajectoryPointContainer;

class Trajectory : public G4VTrajectory
{
public:

// Constructor/Destructor
  Trajectory();
  Trajectory(G4int aTrackID, G4int aParentID, 
	     G4int aPDGEncoding, G4double aPDGCharge,
	     G4String aParticleName,
	     G4ThreeVector aInitialMomentum);

  Trajectory(const G4Track* aTrack);
  Trajectory(Trajectory &);
  virtual ~Trajectory();
  
  // Operators
  inline void* operator new(size_t);
  inline void  operator delete(void*);
  inline int operator == (const Trajectory& right) const
  {return (this==&right);} 
  
  // Get/Set functions 
  inline G4int GetTrackID() const
  { return fTrackID; }
  inline G4int GetParentID() const
  { return fParentID; }
  inline G4String GetParticleName() const
  { return ParticleName; }
  inline G4double GetCharge() const
  { return PDGCharge; }
  inline G4int GetPDGEncoding() const
  { return PDGEncoding; }
  
  TrajectoryPointContainer* GetTrajectoryPointContainer()
  {return positionRecord;}
 
  inline G4ThreeVector GetInitialMomentum() const
  { return initialMomentum; }
  
  // Other member functions
  virtual void ShowTrajectory(std::ostream& os=G4cout) const;
  virtual void DrawTrajectory(G4int i_mode=0) const;
  virtual void AppendStep(const G4Step* aStep);
  virtual int GetPointEntries() const { return positionRecord->size(); }
  virtual G4VTrajectoryPoint* GetPoint(G4int i) const 
  { return (*positionRecord)[i]; }
  
  
  virtual void MergeTrajectory(G4VTrajectory* secondTrajectory);
  
  G4ParticleDefinition* GetParticleDefinition();
  
private:
  
  TrajectoryPointContainer* positionRecord;
  G4int                     fTrackID;
  G4int                     fParentID;
  G4int                     PDGEncoding;
  G4double                  PDGCharge;
  G4String                  ParticleName;
  G4ThreeVector             initialMomentum;
  
};

extern G4Allocator<Trajectory> aMokkaTrajectoryAllocator;

inline void* Trajectory::operator new(size_t)
{
  void* aTrajectory;
  aTrajectory = (void*)aMokkaTrajectoryAllocator.MallocSingle();
  return aTrajectory;
}

inline void Trajectory::operator delete(void* aTrajectory)
{
  aMokkaTrajectoryAllocator.FreeSingle((Trajectory*)aTrajectory);
}

#endif










