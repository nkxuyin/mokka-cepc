#ifdef LCIO_MODE

#include "G4Types.hh"

#include "G4ios.hh"
#include "HepLCIOInterfaceNew.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include "G4HEPEvtParticle.hh"
#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "Control.hh"

#include <algorithm>
#include <assert.h>

//#include "UTIL/LCTOOLS.h"

using namespace lcio ;

HepLCIOInterfaceNew::LCIO2Geant4Map HepLCIOInterfaceNew::Map;
std::set<lcio::MCParticle*>         HepLCIOInterfaceNew::visitedParticles;


HepLCIOInterfaceNew::HepLCIOInterfaceNew(G4String evfile, HEPFILEFORMAT FileFormat)
  : theHepFileFormat(FileFormat), theStdHepRdr(NULL),
    theAscHepRdr(NULL)
{
  switch(theHepFileFormat) 
    {
    case stdhep :
      // print version
      G4cout << lStdHep::getText() << G4endl;
	  
      // open input stdhep binary file
      G4cout << "HepLCIOInterfaceNew in binary input mode\n";
      theStdHepRdr = new lcio::LCStdHepRdr( evfile.data() );
      break;
    case HEPEvt :
    case hepevt :
      G4cout << "HepLCIOInterfaceNew in ASCII input mode\n";
      theAscHepRdr = new LCAscHepRdr( evfile.data(), theHepFileFormat );
      break;
    default :
      G4Exception ("Internal error in HepLCIOInterfaceNew!");
    }
  
  G4ThreeVector zero;
  particle_position = zero;
  particle_time = 0.0;
}

HepLCIOInterfaceNew::~HepLCIOInterfaceNew(){
  if(theStdHepRdr) delete theStdHepRdr ;
  if(theAscHepRdr) delete theAscHepRdr;
}


std::set<G4PrimaryParticle*> HepLCIOInterfaceNew::getRelevantParticles(MCParticle* p){
  //log each particle which has been called, to avoid double counting and increase efficiency
  visitedParticles.insert(p);
  HepLCIOInterfaceNew::LCIO2Geant4Map::iterator mcpIT ;
  std::set<G4PrimaryParticle*> relevantParticlesSet; //holds all relevant decay particles of p
  
  //logic starts here:
  //CASE1: if p is a stable particle: search for it in LCIO2Geant4Map
  //if it is already there: get G4PrimaryParticle version of p from LCIO2Geant4Map
  //else: create G4PrimaryParticle version of p and write it to LCIO2Geant4Map
  //in the end: insert this single G4PrimaryParticle verion of p to the 
  //relevant particle list and return this "list".
  if (p->getGeneratorStatus() == 1) {
    G4PrimaryParticle* g4p;
    mcpIT = Map.find( p ) ;
    if( mcpIT != Map.end() ){
      g4p =  mcpIT->second;
    }
    else{
      G4int IDHEP =    p->getPDG();
      G4double PHEP1 = p->getMomentum()[0];
      G4double PHEP2 = p->getMomentum()[1];
      G4double PHEP3 = p->getMomentum()[2];
      G4double PHEP5 = p->getMass();
      // create G4PrimaryParticle object
      g4p = new G4PrimaryParticle( IDHEP, PHEP1*GeV, PHEP2*GeV, PHEP3*GeV );	
      g4p->SetMass( PHEP5*GeV );
      Map[ p ] = g4p;  
      //std::cout << "*" << std::flush;
    }
    //std::cout << g4p->GetPDGcode() << std::flush;
    relevantParticlesSet.insert(g4p);
    return relevantParticlesSet;
  }

  //CASE2: if p is not stable: get first decay daughter and calculate the proper time of p
  //if proper time is not zero: particle is "relevant", since it carries vertex information
  //if p is already in LCIO2Geant4Map: take it
  //otherwise: create G4PrimaryParticle version of p and write it to LCIO2Geant4Map
  //now collect all relevant particles of all daughters and setup "relevant mother-
  //daughter-relations" between relevant decay particles and G4PrimaryParticle version of p
  //in the end: insert only the G4PrimaryParticle version of p to the
  //relevant particle list and return this "list". The added particle has now the correct pre-assigned
  //decay products and (hopefully) the right lifetime.
  else if(  p->getDaughters().size() > 0  ) {  //fg: require that there is at least one daughter - otherwise forget the particle
    // calculate proper time
    MCParticle* dp = p->getDaughters()[0];
    
    double proper_time = fabs((dp->getTime()-p->getTime()) * p->getMass()) / p->getEnergy();
    //  fix by S.Morozov for real != 0
    double aPrecision = dp->getTime() * p->getMass() / p->getEnergy();
    double bPrecision =  p->getTime() * p->getMass() / p->getEnergy();

    double  proper_time_Precision =  pow(10,-DBL_DIG)*fmax(fabs(aPrecision),fabs(bPrecision));

    bool isProperTimeZero = ( proper_time <= proper_time_Precision ) ;

    // -- remove original --- if (proper_time != 0) {
    if ( isProperTimeZero == false ) {

      G4PrimaryParticle* g4p;
      mcpIT = Map.find( p ) ;
      if( mcpIT != Map.end() ){
	g4p =  mcpIT->second;
      }
      else{
	G4int IDHEP =    p->getPDG();    
	G4double PHEP1 = p->getMomentum()[0];
	G4double PHEP2 = p->getMomentum()[1];
	G4double PHEP3 = p->getMomentum()[2];	
	G4double PHEP5 = p->getMass();
	// create G4PrimaryParticle object
	g4p = new G4PrimaryParticle( IDHEP, PHEP1*GeV, PHEP2*GeV, PHEP3*GeV );	
	g4p->SetMass( PHEP5*GeV );
	g4p->SetProperTime( proper_time*ns );
	Map[ p ] = g4p;
	std::set<G4PrimaryParticle*> vec3;
	for (size_t i=0; i<p->getDaughters().size(); i++){
	  if (visitedParticles.count(p->getDaughters()[i])==0){
	    //std::cout << "<" << std::flush;
	    std::set<G4PrimaryParticle*> vec2 = getRelevantParticles(p->getDaughters()[i]);
	    //std::cout << ">" << std::flush;
			
	    std::set<G4PrimaryParticle*>::iterator setit;
	    for (setit=vec2.begin() ; setit != vec2.end(); setit++ ){
	      vec3.insert(*setit);
	    }
	  }
	  //else {
	  //  std::cout << "<v>" << std::flush;
	  //}
	}

	std::set<G4PrimaryParticle*>::iterator setit;
	for (setit=vec3.begin() ; setit != vec3.end(); setit++ ){
	  g4p->SetDaughter(*setit);
	}
	//std::cout << "*" << std::flush;
      }
      //std::cout << g4p->GetPDGcode() << std::flush;
      relevantParticlesSet.insert(g4p);
      return relevantParticlesSet;
    }
	

    //CASE3: if p is not stable AND has zero lifetime: forget about p and retrieve all relevant
    //decay particles of all daughters of p. In this case this step of recursion is just there for
    //collecting all relevant decay products of daughters (and return them).
    else{
      for (size_t i=0; i<p->getDaughters().size(); i++){
	if (visitedParticles.count(p->getDaughters()[i])==0){
	  //std::cout << "<" << std::flush;
	  std::set<G4PrimaryParticle*> vec2 = getRelevantParticles(p->getDaughters()[i]);
	  //std::cout << ">" << std::flush;
	  std::set<G4PrimaryParticle*>::iterator setit;
	  for (setit=vec2.begin() ; setit != vec2.end(); setit++ ){
	    relevantParticlesSet.insert(*setit);
	  }
	}
	else{
	  //std::cout << "<v>" << std::flush;
	}
      }
      return relevantParticlesSet;
    }
  }
  //fg: add a default return
  return relevantParticlesSet;
}


void HepLCIOInterfaceNew::GeneratePrimaryVertex(G4Event* evt){

  //**********************************************************
  // Read in the event
  //**********************************************************
  Map.clear() ;
  visitedParticles.clear() ;


  if(theHepFileFormat == stdhep) Control::lcMCVec =  theStdHepRdr->readEvent() ;
  else                           Control::lcMCVec =  theAscHepRdr->readEvent() ;

  if( Control::lcMCVec == 0  ) {
    std::stringstream eventNumber;
    eventNumber << "Event " <<  evt->GetEventID() + Control::SYNCEVT;
    Control::RunAborted = true;
    G4Exception("HepLCIOInterfaceNew::GeneratePrimaryVertex",
		eventNumber.str().c_str(),
                RunMustBeAborted, 
		"*** Error when reading hep file; probably ran out of events");
    return;
  }

  std::vector<MCParticle*> col( Control::lcMCVec->size() )  ;
  G4int NHEP = col.size() ;

  // parameters of the Lorentz transformation matrix
  const G4double zspreadparameter = Control::PrimaryVertexSpreadZ;
  const G4double alpha = Control::LorentzTransformationAngle;
  const G4double gamma = sqrt(1 + sqr(tan(alpha)));
  const G4double betagamma = tan(alpha);
  //std::cout << " alpha=" << alpha  <<"rad, gamma=" << gamma << ", betagamma=" << betagamma  << std::endl;

  double zspread = ( zspreadparameter == 0.0 ?  0.0 : G4RandGauss::shoot(0,zspreadparameter/mm ) ) ;
  //std::cout << " PrimaryVertexSpreadZ=" << zspreadparameter/mm  <<"mm, zspread=" << zspread << "mm" << std::endl;

  particle_position = G4ThreeVector(0,0,zspread);

  //**********************************************************
  // build collection of MCParticles
  //**********************************************************
   //if (alpha != 0)
   //  std::cout << " **************************  GeneratePrimaryVertex: applying Lorentz boost  !!!!!!!!!!! **************************** " << std::endl ;

  for( G4int IHEP=0; IHEP<NHEP; IHEP++ ) {
    setCharge (IHEP);
    lcio::MCParticleImpl* mcp =  dynamic_cast<MCParticleImpl*> ( Control::lcMCVec->getElementAt( IHEP ) ) ;

    //Boost to lab frame with crossing angle alpha
    if (alpha != 0) {

      double c_light = 299.792;
      const double* p = mcp->getMomentum() ;
      const G4double m  = mcp->getMass() ;
      //double e = mcp->getEnergy();
      // after the transformation (boost in x-direction)
      double pPrime[3] ;
      pPrime[0] = betagamma * sqrt(sqr(p[0])+sqr(p[1])+sqr(p[2])+sqr(m)) + gamma * p[0] ;
      pPrime[1] = p[1] ;
      pPrime[2] = p[2] ;

      const double* v = mcp->getVertex() ;
      const double t = mcp->getTime() ;
      double vPrime[3];
      double tPrime;
      tPrime = gamma * t + betagamma * v[0] / c_light;
      vPrime[0] = gamma * v[0] + betagamma * c_light * t;
      vPrime[1] = v[1];
      vPrime[2] = v[2];

      // py and pz remain the same, E changes implicitly with px
      mcp->setMomentum( pPrime );
      mcp->setVertex( vPrime );
      mcp->setTime( tPrime );
      //std::cout << IHEP  << " alpha=" << alpha  <<" gamma=" << gamma << " betagamma=" << betagamma  << std::endl;
      //	  std::cout << IHEP  << " unboosted momentum: ("  << e                << "," << p[0]      << "," << p[1]      << ","  <<  p[2]      << ")" << std::endl;
      //	  std::cout << IHEP  << "   boosted momentum: ("  << mcp->getEnergy() << "," <<  betagamma * sqrt(sqr(p[0])+sqr(p[1])+sqr(p[2])+sqr(m)) + gamma * p[0] << ","  << pPrime[1] << "," <<  pPrime[2] << ")" << std::endl;
      //	  std::cout << IHEP  << "                     ("  << mcp->getEnergy()-e << ","  << ( betagamma * sqrt(sqr(p[0])+sqr(p[1])+sqr(p[2])+sqr(m)) + gamma * p[0])-p[0] << ","  << pPrime[1]-p[1] << ","  <<  pPrime[2]-p[2] << ")" << std::endl;
      //std::cout << IHEP  << "   unboosted vertex: (" << t      << "," << v[0]      << "," << v[1]      << "," <<  v[2]      << ")" << std::endl;
      //	  std::cout << IHEP  << "     boosted vertex: (" << tPrime << "," << vPrime[0] << "," << vPrime[1] << "," <<  vPrime[2] << ")" << std::endl;
      //	  std::cout << IHEP  << "     boosted vertex: (" << tPrime-t << "," << vPrime[0]-v[0] << "," << vPrime[1]-v[1] << "," <<  vPrime[2]-v[2] << ")" << std::endl;
    }

    // apply z spread
    if (zspreadparameter != 0){
      const double* v = mcp->getVertex() ;
      double vPrime[3];
      vPrime[0] = v[0];
      vPrime[1] = v[1];
      vPrime[2] = v[2] + zspread;
      mcp->setVertex(vPrime);
      //std::cout << vPrime[2] << std::endl;
    }

    col[ IHEP ] = dynamic_cast<MCParticle*>( mcp );
  }


  //**********************************************************
  // check if there is at least one particle
  //**********************************************************
  if( NHEP == 0 ) return ;

  
  //**********************************************************
  // create G4PrimaryVertex object
  //**********************************************************
  G4PrimaryVertex* vertex = new G4PrimaryVertex(particle_position,particle_time);
  // put initial particles to the vertex
  for( size_t i=0; i< col.size(); i++ ){
    MCParticle* mcp = col[i] ;
    if( mcp->getParents().size()==0 ){
      //std::cout << "construct tree of relevant G4PrimaryParticles for initial MCParticle "<< i << " " <<std::flush;
      std::set<G4PrimaryParticle*> g4set = getRelevantParticles(mcp);
      //std::cout << " ..finished" << std::endl;
      std::set<G4PrimaryParticle*>::iterator setit;
      for (setit=g4set.begin() ; setit != g4set.end(); setit++ ){
	vertex->SetPrimary(*setit);
	//std::cout << "G4PrimaryParticle ("<< (*setit)->GetPDGcode() << ") added to G4PrimaryVertex." << std::endl;
      }
    }
  }

  // Put the vertex to G4Event object
  evt->AddPrimaryVertex( vertex );
}


void HepLCIOInterfaceNew::setCharge (G4int IHEP)
{
  //
  // Initialise the particle charge looking at the actual 
  // Particle Table, or set it to -1000 to flag missing 
  // information, if PDG unknown in the actual physics list..
  //

  G4ParticleTable* theParticleTable = G4ParticleTable::GetParticleTable();
  lcio::MCParticleImpl* mcp_impl = dynamic_cast<MCParticleImpl*> ( Control::lcMCVec->getElementAt( IHEP ) ) ;
  G4ParticleDefinition* theParticle = theParticleTable->FindParticle(mcp_impl->getPDG());
  
  if( theParticle ) mcp_impl->setCharge(theParticle->GetPDGCharge());
  else              mcp_impl->setCharge(-1000);
}

#endif
