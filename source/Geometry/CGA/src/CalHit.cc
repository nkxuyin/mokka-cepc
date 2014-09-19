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
// $Id: CalHit.cc,v 1.14 2009/04/27 15:13:41 musat Exp $
// $Name: mokka-07-00 $
//
// 

#include "Control.hh"
#include "CalHit.hh"
#include "G4VVisManager.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "G4Polyline.hh"
#include <assert.h>
#include <map>

G4Allocator<CalHit> CalHitAllocator;

CalHit::CalHit(const CalHit &right)
#ifdef LCIO_MODE
    : VHit(LCIO::SIMCALORIMETERHIT)
#else
    : VHit()
#endif
{
  P = right.P;
  S = right.S;
  M = right.M;
  I = right.I;
  J = right.J;
  K = right.K;
  GRZone = right.GRZone;
  X = right.X;
  Y = right.Y;
  Z = right.Z;
  Energy = right.Energy;
  code.id0 = right.code.id0;
  code.id1 = right.code.id1;
  PrimaryContributions = right.PrimaryContributions;
}

CalHit::~CalHit(){
  for(PrimaryContribution_type::iterator p = PrimaryContributions.begin();
      p != PrimaryContributions.end(); p++){
    delete p->second ;
      //     PrimaryContribution_type::value_type x = *p;
      //     PrimaryContribution* PIDcontrib = x.second;
      }
}

const CalHit& CalHit::operator=(const CalHit &right)
{
#ifdef LCIO_MODE
  ((VHit*)this)->operator=(right);
#endif
  P = right.P;
  S = right.S;
  M = right.M;
  I = right.I;
  J = right.J;
  K = right.K;
  GRZone = right.GRZone;
  X = right.X;
  Y = right.Y;
  Z = right.Z;
  Energy = right.Energy;
  code.id0 = right.code.id0;
  code.id1 = right.code.id1;
  PrimaryContributions = right.PrimaryContributions;
  return *this;
}

void CalHit::Draw() 
{  
  G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
  if(pVVisManager) {
    PrimaryContribution_type::iterator p = PrimaryContributions.begin();
    PrimaryContribution_type::value_type x = *p;
    G4int PID = x.first;
    G4Colour colour(((PID%2)+1.)/2.,((PID%3)+1.)/3.,1.);
    //G4Colour colour(1.,1.,1.);
    G4VisAttributes attribs(colour);
    G4Circle circle;
    circle.SetPosition(G4Point3D(X,Y,Z));
    circle.SetScreenDiameter (2.0); 
    circle.SetFillStyle (G4Circle::filled);
    circle.SetVisAttributes(attribs); 
    pVVisManager->Draw(circle);
  }
}
void CalHit::Print()
{
}
void CalHit::Save(FILE *oFile)
{
  if(oFile)
    {
      fprintf(oFile,"%d %d %d %3d %3d %2d %7.2f %7.2f %7.2f %15e %d %d %d %d\n",
	      GetP(),
	      GetS(),
	      GetM(),
	      GetI(),
	      GetJ(),
	      GetK(),
	      GetX(),
	      GetY(),
	      GetZ(),
	      (GetEnergy())/GeV,
	      code.id0,
	      code.id1,
	      getAsciiFlag(),
	      (int)(PrimaryContributions.size()));

      for(PrimaryContribution_type::iterator p = PrimaryContributions.begin();
	  p != PrimaryContributions.end();
	  p++)
	{
	  PrimaryContribution_type::value_type x = *p;
	  PrimaryContribution* PIDcontrib = x.second;
	  

	  //fg: PDGContribution_type is now a multimap that holds energy and time
	  // for every secondary - so we first have to iterate over that and accumulate 
	  // the energy 
	  std::map<G4int,G4double> pdgMap ;


//fg 	  fprintf(oFile,"%d %15e %d\n",
// 		  x.first, x.second->E, PIDcontrib->PDGContributions.size());

	  for(PDGContribution_type::iterator q = PIDcontrib->PDGContributions.begin();
	      q != PIDcontrib->PDGContributions.end();
	      q++)
	    {
	      PDGContribution_type::value_type y = *q;
	      
	      // map::insert returns std::pair< iterator , bool > !        
	      if( ! pdgMap.insert( std::make_pair(   y.first , y.second.energy  )  ).second  ) {

		pdgMap[ y.first ] += y.second.energy ;

	      } 

//fg 	      fprintf(oFile,"%d %15e\n",
// 		      y.first, y.second );
	    }

	  //fg -- now we write to the ASCII file from the map --
 	  fprintf(oFile,"%d %15e %d\n",
		  x.first, (x.second->E)/GeV, (int)(pdgMap.size()));
	  
	  for( std::map<G4int,G4double>::iterator i = pdgMap.begin() ; i != pdgMap.end() ; i++ ) {
	  
	    fprintf(oFile,"%d %15e\n",
		    i->first, (i->second)/GeV );
	  }

	}
      
      //	      GetPID(),
      //	      GetPDG(),
    }
}

G4bool
CalHit::Load(FILE *iFile) 
{
  G4int ReadStatus;
  G4int nPIDS;
  ReadStatus = 
    fscanf(iFile,"%d%d%d%d%d%d%lf%lf%lf%le%d%d%d%d",
	   &P,&S,&M,&I,&J,&K,&X,&Y,&Z,&Energy,&(code.id0),&(code.id1),&theAsciiFlag,&nPIDS);

  ReadStatus = (ReadStatus == 14);
  if(feof(iFile) || !ReadStatus) return false;
  
  if(ReadStatus) {
    Energy = Energy * GeV;
    for (G4int iPID = 0; iPID < nPIDS; iPID++)
      {
	G4int PID,nPDGS;
	G4double EPID;
	ReadStatus = 
	  fscanf(iFile,"%d%le%d",
		 &PID, &EPID, &nPDGS) == 3;
	if(ReadStatus) {
	  for (G4int iPDG = 0; iPDG < nPDGS; iPDG++)
	    {
	      G4int PDG;
	      G4double EPDG;
	      ReadStatus = 
		fscanf(iFile,"%d%le",
		       &PDG, &EPDG) == 2;
	      if(ReadStatus)  AddEdep(PID,PDG,EPDG,0);
	    }
	}
      }
  }
  if(!ReadStatus) 
    G4cout << "Error reading hits file!" << G4endl;

  return ReadStatus;
}

#ifdef LCIO_MODE
void 
CalHit::Save(LCCollectionVec* aLCCollectionVec)
{
  SimCalorimeterHitImpl* hit = new SimCalorimeterHitImpl;

  hit->setCellID0(code.id0);
  hit->setCellID1(code.id1);

  float pos[3];
  pos[0]=X;
  pos[1]=Y;
  pos[2]=Z;
  hit->setPosition( pos );
  
//   // Warning: we repeat here the E, to be foreseen!!!!!!
//   const MCParticle* theMCPRIM =
//     dynamic_cast<const MCParticle*>(Control::lcMCVec->getElementAt( PID-1 ));

  for(PrimaryContribution_type::iterator p = PrimaryContributions.begin();
      p != PrimaryContributions.end();
      p++)
    {
      PrimaryContribution_type::value_type x = *p;
      G4int PID = x.first;
      MCParticle* theMCPRIM = 
	dynamic_cast<MCParticle*> (Control::MCParticleMap[PID]);
//	dynamic_cast<MCParticle*>(Control::lcMCVec->getElementAt( PID-1 )); 

      PrimaryContribution* PIDcontrib = x.second;
      
      if( Control::LCIODetailedShowerMode ) {
	// add contributions for every secondary
	for(PDGContribution_type::iterator q = PIDcontrib->PDGContributions.begin();
	    q != PIDcontrib->PDGContributions.end();
	    q++)
	  {
	    PDGContribution_type::value_type y = *q;
	    G4int PDG = y.first;
	    G4double PDG_E = y.second.energy ;
	    G4double PDG_T = y.second.time ;
	    float *sp = y.second.stepPosition;
	    hit->addMCParticleContribution( theMCPRIM , 
					    PDG_E/GeV,
					    PDG_T/ns,
					    PDG
#if LCIO_VERSION_GE( 1, 60 )
					    , sp
#endif
);
	  }

      } else {
	// just one contributio for every primary (== particle that entered the calorimeter)
	hit->addMCParticleContribution( theMCPRIM , 
					PIDcontrib->E/GeV , 
					PIDcontrib->time/ns );

      }
    }
  
  //  hit->addMCParticleContribution( theMCPRIM ,  E / GeV, 0. , PDG ) ;
  aLCCollectionVec->push_back( hit );

}
#endif

void 
CalHit::AddEdep(G4int pPID, G4int pPDG,
		G4double de, G4double pt,float*sp)
{
//   G4cout << "CalHit::AddEdep" << G4endl;
//   G4cout << "K, J, I, M, S = " << K << ", " <<  J 
// 	 << ", " <<  I << ", " <<  M << ", " <<  S  << G4endl;
//   G4cout << "pPID / pPDG / de = " << pPID 
// 	 << " / " << pPDG << " / " << de << G4endl;

  Energy += de; // total in the cell

  PrimaryContribution*& thePrimaryContribution = PrimaryContributions[pPID];
  if(!thePrimaryContribution)
    thePrimaryContribution = new PrimaryContribution(pPDG,de,pt,sp);
  else
    thePrimaryContribution->AddEdep(pPDG,de,pt,sp);
}
