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
// $Id: TPCHit.cc,v 1.3 2003/10/10 15:11:47 mora Exp $
// $Name: mokka-07-00 $
//

#include "Control.hh"
#include "TPCHit.hh"
#include "G4VVisManager.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "G4Polyline.hh"
#include <assert.h>

G4Allocator<TPCHit> TPCHitAllocator;

TPCHit::TPCHit(const TPCHit &right)
#ifdef LCIO_MODE
  : VHit(LCIO::SIMTRACKERHIT)
#else
    : VHit()
#endif
{
  P = right.P;
  K = right.K;
  X = right.X;
  Y = right.Y;
  Z = right.Z;
  PID=right.PID;
  PDG=right.PDG;
}

const TPCHit& TPCHit::operator=(const TPCHit &right)
{
#ifdef LCIO_MODE
  ((VHit*)this)->operator=(right);
#endif
  P = right.P;
  K = right.K;
  X = right.X;
  Y = right.Y;
  Z = right.Z;
  PID=right.PID;
  PDG=right.PDG;
  return *this;
}

int TPCHit::operator==(const TPCHit &right) const
{
  return ((P==right.P) &&
	  (X==right.X) &&
	  (Y==right.Y) &&
	  (Z==right.Z) &&
	  (PID==right.PID) &&
	  (PDG==right.PDG));
}    

void TPCHit::Draw() 
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
void TPCHit::Print()
{
}
void 
TPCHit::Save(FILE *oFile)
{
  if(oFile)
    {
      G4int type;
      if(GetK()<1000) 
	type=1;
      else 
	type=2;
      fprintf(oFile,"%d %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %d %d\n",
	      type,
	      GetX(),
	      GetY(),
	      GetZ(),
	      GetPx(),
	      GetPy(),
	      GetPz(),		      
	      GetPID(),
	      GetPDG());      
    }
}

G4bool
TPCHit::Load(FILE *iFile) 
{
  K=0;
  return fscanf(iFile,"%d %lf %lf %lf %lf %lf %lf %d %d",
		&P,&X,&Y,&Z,&Px,&Py,&Pz,&PID,&PDG) == 9;
}

