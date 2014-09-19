// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: Trajectory.cc,v 1.3 2006/09/13 17:11:03 adrian Exp $
// $Name: mokka-07-00 $

#include "Trajectory.hh"
#include "G4TrajectoryPoint.hh"
#include "G4ParticleTable.hh"
#include "G4ThreeVector.hh"
#include "G4Polyline.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "G4VVisManager.hh"

G4Allocator<Trajectory> aMokkaTrajectoryAllocator;

///////////////////////////////////////////
Trajectory::Trajectory()
  :  positionRecord(0), fTrackID(0), fParentID(0),
     PDGEncoding( 0 ), PDGCharge(0.0), ParticleName(""),
     initialMomentum( G4ThreeVector())  {}
///////////////////////////////////////////

///////////////////////////////////////////
Trajectory::Trajectory(G4int aTrackID, G4int aParentID, 
		       G4int aPDGEncoding, G4double aPDGCharge,
		       G4String aParticleName,
		       G4ThreeVector aInitialMomentum)
  : fTrackID(aTrackID), fParentID(aParentID),
    PDGEncoding(aPDGEncoding), PDGCharge(aPDGCharge), 
    ParticleName(aParticleName),
    initialMomentum(aInitialMomentum)
{
  positionRecord = new TrajectoryPointContainer();
  
}
///////////////////////////////////////////

///////////////////////////////////////////
Trajectory::Trajectory(const G4Track* aTrack)
///////////////////////////////////////////
{
   G4ParticleDefinition * fpParticleDefinition = aTrack->GetDefinition();
   ParticleName = fpParticleDefinition->GetParticleName();
   PDGCharge = fpParticleDefinition->GetPDGCharge();
   PDGEncoding = fpParticleDefinition->GetPDGEncoding();
   fTrackID = aTrack->GetTrackID();
   fParentID = aTrack->GetParentID();
   //   positionRecord = new G4RWTPtrOrderedVector<G4VTrajectoryPoint>;
   positionRecord = new TrajectoryPointContainer();
   positionRecord->push_back(new G4TrajectoryPoint(aTrack->GetPosition()));
}

//////////////////////////////////////////
Trajectory::Trajectory(Trajectory & right) : 
  G4VTrajectory(right)
//////////////////////////////////////////
{
  ParticleName = right.ParticleName;
  PDGCharge = right.PDGCharge;
  PDGEncoding = right.PDGEncoding;
  fTrackID = right.fTrackID;
  fParentID = right.fParentID;
  //  positionRecord = new G4RWTPtrOrderedVector<G4VTrajectoryPoint>;
  // std::vector<G4VTrajectoryPoint*> *positionRecord;
  positionRecord = new TrajectoryPointContainer();

  for(size_t i=0;i<right.positionRecord->size();i++)
  {
    G4TrajectoryPoint* rightPoint = (G4TrajectoryPoint*)((*(right.positionRecord))[i]);
    positionRecord->push_back(new G4TrajectoryPoint(*rightPoint));
  }
}


/////////////////////////////
Trajectory::~Trajectory()
/////////////////////////////
{
  //  positionRecord->clearAndDestroy();
  size_t i;
  for(i=0;i<positionRecord->size();i++){
    delete  (*positionRecord)[i];
  }
  positionRecord->clear();

  delete positionRecord;
}

///////////////////////////////////
void Trajectory::ShowTrajectory(std::ostream&) const
///////////////////////////////////
{
   G4cout << G4endl << "TrackID =" << fTrackID 
        << ":ParentID=" << fParentID << G4endl;
   G4cout << "Particle name : " << ParticleName 
        << "  Charge : " << PDGCharge << G4endl;
   G4cout << "  Current trajectory has " << positionRecord->size() 
        << " points." << G4endl;

   for( size_t i=0 ; i < positionRecord->size() ; i++){
       G4TrajectoryPoint* aTrajectoryPoint = (G4TrajectoryPoint*)((*positionRecord)[i]);
       G4cout << "Point[" << i << "]" 
            << " Position= " << aTrajectoryPoint->GetPosition() << G4endl;
   }
}

///////////////////////////////////////////////
void Trajectory::DrawTrajectory(G4int i_mode) const
///////////////////////////////////////////////
{
   // static objects which are allocated only once per program run
   static G4VisAttributes *visAttributesRed   = new G4VisAttributes(G4Colour::Red());
   static G4VisAttributes *visAttributesGreen = new G4VisAttributes(G4Colour::Green());
   static G4VisAttributes *visAttributesBlue  = new G4VisAttributes(G4Colour::Blue());

   G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
   G4ThreeVector pos;

   if(i_mode>=0)
   {
     G4Polyline pPolyline;
     for (size_t i = 0; i < positionRecord->size() ; i++) {
       G4TrajectoryPoint* aTrajectoryPoint = (G4TrajectoryPoint*)((*positionRecord)[i]);
       pos = aTrajectoryPoint->GetPosition();
       pPolyline.push_back( pos );
     }

     G4VisAttributes *visAttributes;
     if      (PDGCharge < 0) visAttributes = visAttributesRed;
     else if (PDGCharge > 0) visAttributes = visAttributesBlue;
     else                    visAttributes = visAttributesGreen;

     pPolyline.SetVisAttributes(visAttributes);
     if(pVVisManager) pVVisManager->Draw(pPolyline);
   }

   if(i_mode!=0)
   {
     for(size_t j=0; j<positionRecord->size(); j++) {
       G4TrajectoryPoint* aTrajectoryPoint = (G4TrajectoryPoint*)((*positionRecord)[j]);
       pos = aTrajectoryPoint->GetPosition();
       G4Circle circle( pos );
       circle.SetScreenSize(0.001*i_mode);
       circle.SetFillStyle(G4Circle::filled);
       G4Colour colSpot(0.,0.,0.);
       G4VisAttributes attSpot(colSpot);
       circle.SetVisAttributes(attSpot);
       if(pVVisManager) pVVisManager->Draw(circle);
     }
   }

}

////////////////////////////////////////////
void Trajectory::AppendStep(const G4Step* aStep)
////////////////////////////////////////////
{
   positionRecord->push_back( new G4TrajectoryPoint(aStep->GetPostStepPoint()->
                                 GetPosition() ));
}
  
/////////////////////////////////////////////
G4ParticleDefinition* Trajectory::GetParticleDefinition()
/////////////////////////////////////////////
{
   return (G4ParticleTable::GetParticleTable()->FindParticle(ParticleName));
}

/////////////////////////////////////////////
void Trajectory::MergeTrajectory(G4VTrajectory* secondTrajectory)
/////////////////////////////////////////////
{
  if(!secondTrajectory) return;

  Trajectory* seco = (Trajectory*)secondTrajectory;
  G4int ent = seco->GetPointEntries();
  for(G4int i=1;i<ent;i++) // initial point of the second trajectory should not be merged
  { 
    positionRecord->push_back((*(seco->positionRecord))[i]);
    //    positionRecord->push_back(seco->positionRecord->removeAt(1));
  }
  delete (*seco->positionRecord)[0];
  seco->positionRecord->clear();
}


