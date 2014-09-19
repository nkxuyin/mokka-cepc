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
// $Id: TRKHit.cc,v 1.11 2009/04/24 15:09:11 musat Exp $
// $Name: mokka-07-00 $
//
// TRKHit generalizes the old TPCHit for all tracking
// devices (Paulo, Sept 2002)

#include "Control.hh"
#include "TRKHit.hh"
#include "G4VVisManager.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "G4Polyline.hh"
#include <assert.h>

G4Allocator<TRKHit> TRKHitAllocator;

TRKHit::TRKHit(const TRKHit &right)
#ifdef LCIO_MODE
  : VHit( LCIO::SIMTRACKERHIT )
#else
    : VHit()
#endif
{
  Layer = right.Layer;
  X = right.X;
  Y = right.Y;
  Z = right.Z;
  PID = right.PID;
  PDG = right.PDG;
  Energy = right.Energy;
  Time = right.Time ;
}

const TRKHit& TRKHit::operator=(const TRKHit &right)
{
#ifdef LCIO_MODE
  ((VHit*)this)->operator=(right);
#endif
  Layer = right.Layer;
  X = right.X;
  Y = right.Y;
  Z = right.Z;
  PID = right.PID;
  PDG = right.PDG;
  Energy = right.Energy;
  Time = right.Time ;
  return *this;
}

int TRKHit::operator==(const TRKHit &right) const
{
  return ((Layer == right.Layer) &&
	  (X==right.X) &&
	  (Y==right.Y) &&
	  (Z==right.Z) &&
	  (PID==right.PID) &&
	  (PDG==right.PDG));
}    

void TRKHit::Draw() 
{  
  G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
  if(pVVisManager) {
    G4Colour colour(((PID%2)+1.)/2.,((PID%3)+1.)/3.,1.);
    G4VisAttributes attribs(colour);
    G4Circle circle;
    circle.SetPosition(G4Point3D(X,Y,Z));
    circle.SetScreenDiameter (2.0); 
    circle.SetFillStyle (G4Circle::filled);
    circle.SetVisAttributes(attribs); 
    pVVisManager->Draw(circle);
  }
}
void TRKHit::Print()
{
}
void 
TRKHit::Save(FILE *oFile)
{
  if(oFile)
    {
      fprintf(oFile,"%d %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %d %d %15e\n",
	      GetLayer(),
	      GetX(),
	      GetY(),
	      GetZ(),
	      GetPx(),
	      GetPy(),
	      GetPz(),		      
	      GetPID(),
	      GetPDG(),
	      (GetEnergy())/GeV
	      // GetTime() // time not standard in ascii output 
	      );      
    }
}

G4bool
TRKHit::Load(FILE *iFile) 
{
  G4bool ReadStatus;
  ReadStatus = 
    (fscanf(iFile,"%d %lf %lf %lf %lf %lf %lf %d %d %lf",
	    &Layer,&X,&Y,&Z,&Px,&Py,&Pz,&PID,&PDG,&Energy) == 10);
  Energy = Energy * GeV;
  return ReadStatus;
}

#ifdef LCIO_MODE
void 
TRKHit::Save(LCCollectionVec* aLCCollectionVec)
{
  SimTrackerHitImpl* hit = new SimTrackerHitImpl ;
  hit->setEDep(Energy  / GeV);   
  double pos[3];
  pos[0]=X;
  pos[1]=Y;
  pos[2]=Z;

#if LCIO_VERSION_GE( 1 , 60 )
  hit->setCellID0(Layer);
#else
  hit->setCellID(Layer);
#endif
  hit->setPosition( pos ) ;
  hit->setMCParticle(NULL);
  hit->setTime( Time / ns ) ;

#if LCIO_VERSION_GE( 1, 6 )
  hit->setMomentum(Px/GeV, Py/GeV, Pz/GeV);
#endif

#if LCIO_VERSION_GE( 1 , 7 )
  hit->setPathLength( StepLength ) ;
#endif 

//   hit->setMCParticle( dynamic_cast<const MCParticle*>
// 		      (Control::lcMCVec->getElementAt( PID-1 ) ) ) ;
  hit->setMCParticle( dynamic_cast<MCParticle*>
		      (Control::MCParticleMap[PID]));
//		      (Control::lcMCVec->getElementAt( PID-1 ) ) ) ;

  aLCCollectionVec->push_back( hit );
  
}
#endif
