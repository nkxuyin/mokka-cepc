#ifdef LCIO_MODE

#include "G4Types.hh"

#include "G4ios.hh"
#include "HepLCIOInterface.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include "G4HEPEvtParticle.hh"
#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "Control.hh"

#include <algorithm>
#include <assert.h>

#include "HepLCIOInterfaceNew.hh"

//#include "UTIL/LCTOOLS.h"

using namespace lcio ;


//fg: helper function to modify the generator status (add 100) for particle that are not to be simulated
inline void modifyGeneratorStatus(MCParticleImpl* mcp){

  int genStat = mcp->getGeneratorStatus() ;

  if( genStat < 100 ) 
    mcp->setGeneratorStatus( genStat + 100 );
}

HepLCIOInterface::HepLCIOInterface(G4String evfile, 
				   HEPFILEFORMAT FileFormat) 
  : theHepFileFormat(FileFormat), theStdHepRdr(NULL),
    theAscHepRdr(NULL)
{
  switch(theHepFileFormat) 
    {
    case stdhep :
      // print version
      G4cout << lStdHep::getText() << G4endl;
      
      // open input stdhep binary file
      G4cout << "HepLCIOInterface in binary input mode\n";
      theStdHepRdr = new lcio::LCStdHepRdr( evfile.data() );
      break;
    case HEPEvt :
    case hepevt :
      G4cout << "HepLCIOInterface in ASCII input mode\n";
      theAscHepRdr = new LCAscHepRdr( evfile.data(), theHepFileFormat );
      break;
    default :
      G4Exception ("Internal error in HepLCIOInterface!");
    }
  
  G4ThreeVector zero;
  particle_position = zero;
  particle_time = 0.0;
}

HepLCIOInterface::~HepLCIOInterface()
{
  if(theStdHepRdr) delete theStdHepRdr ;
  if(theAscHepRdr) delete theAscHepRdr;
}

bool HepLCIOInterface::isLeptonCascadeParticle(lcio::MCParticle* mcp) {

  bool tk=false;

  G4int IDHEP = mcp->getPDG();    
  if ( abs(IDHEP) == 11 ||
       abs(IDHEP) == 13 ||
       abs(IDHEP) == 15 ) {

    if (mcp->getDaughters().size() == 0 ) {
      tk = true ; }
    else {
      for ( unsigned i=0 ; i < mcp->getDaughters().size() ; i++ ) {
        MCParticle* mcp2=mcp->getDaughters()[i];
        if ( mcp2->getPDG ()  == IDHEP ||
             abs(mcp2->getPDG () ) == 94 ) {
          tk=true ;
        }
      }
    }
  }
  return tk;
}
bool HepLCIOInterface::isNeutrino(G4int IDHEP){
  

  if ( abs(IDHEP) == 12 ||
     abs(IDHEP) == 14 ||
     abs(IDHEP) == 16 ) {
    return true; }
  else {
    return false;
  }
}
bool HepLCIOInterface::isTopQuark(G4int IDHEP){ 
  if ( abs(IDHEP) == 6 ) {
    return true; }
  else {
    return false;
  }
}


//fg: helper function to change the generator status of all parents recursively
// with FixStdHepLeptonFSR
void modifyParentsGeneratorStatus(MCParticleImpl* mcp){
  
  MCParticleVec pv = mcp->getParents() ;
  
  for(unsigned int i=0; i<pv.size(); ++i ){
    
    MCParticleImpl* pImpl = dynamic_cast<MCParticleImpl*>( pv[i] ) ;
    
    modifyGeneratorStatus( pImpl ) ;
    
    modifyParentsGeneratorStatus( pImpl ) ;
  }
}


void HepLCIOInterface::GeneratePrimaryVertex(G4Event* evt)
{

  //
  // Read in the event
  //

  HepLCIOInterfaceNew::Map.clear() ;

  if( theHepFileFormat == stdhep )
    Control::lcMCVec =  theStdHepRdr->readEvent() ;
  else
    Control::lcMCVec =  theAscHepRdr->readEvent() ;
    
//   std::cout << " HepLCIOInterface::GeneratePrimaryVertex for event " << evt->GetEventID() 
// 	    << std::endl ;


  if( Control::lcMCVec == 0  ) 
  {
    std::stringstream eventNumber;
    eventNumber << "Event " <<  evt->GetEventID() + Control::SYNCEVT;
    Control::RunAborted = true;
    G4Exception("HepLCIOInterface::GeneratePrimaryVertex",
	eventNumber.str().c_str(),
        RunMustBeAborted, 
	"*** Error when reading hep file; probably ran out of events");

    return;
  }

//  LCTOOLS::printMCParticles( Control::lcMCVec ) ;


  std::vector<MCParticle*> col( Control::lcMCVec->size() )  ;

  G4int NHEP = col.size() ;

  G4int ISTHEP;   // status code
  G4int IDHEP;    // PDG code
  G4int JDAHEP1;  // first daughter
  G4int JDAHEP2;  // last daughter
  G4double PHEP1; // px in GeV
  G4double PHEP2; // py in GeV
  G4double PHEP3; // pz in GeV
  G4double PHEP5; // mass in GeV

  for( G4int IHEP=0; IHEP<NHEP; IHEP++ )
  {

    lcio::MCParticleImpl* mcp = 
      dynamic_cast<MCParticleImpl*> ( Control::lcMCVec->getElementAt( IHEP ) ) ;


    col[ IHEP ] = mcp ;

    ISTHEP = mcp->getGeneratorStatus();

    IDHEP = mcp->getPDG();    

    if( Control::FixStdHepLeptonFSR ){

      //       if ( ISTHEP == 1 ) {
      // 	if ( abs(IDHEP) == 15 ) {
      //           // Set undecayed taus to dcayed
      // 	  mcp->setGeneratorStatus(2);
      // 	}
      //       } else { 

      if ( ISTHEP == 2 ) {
	
	if (  isLeptonCascadeParticle(mcp) || 
	      isNeutrino(IDHEP) ||  
	      isTopQuark(IDHEP) ) {
	  
	  modifyGeneratorStatus(mcp) ;
	  
	  modifyParentsGeneratorStatus( mcp ) ;
	}
      }
      //      }
    } // endif Control::FixStdHepLeptonFSR
    
  }
  for( G4int IHEP=0; IHEP<NHEP; IHEP++ ) {

    setCharge (IHEP);

    lcio::MCParticle* mcp = 
      dynamic_cast<MCParticle*> ( Control::lcMCVec->getElementAt( IHEP ) ) ;

    col[ IHEP ] = mcp ;
    
    ISTHEP =  mcp->getGeneratorStatus();

    // use only stable particles and preassigned decays
    if( ! ( ISTHEP == 1 ||  ISTHEP == 2 ) )  
      continue ; 
      
    IDHEP = mcp->getPDG();    

    JDAHEP1 = 0 ; // mcp->daughter1(IHEP)%10000;
    JDAHEP2 = 0 ; // mcp->daughter2(IHEP)%10000;

    PHEP1 = mcp->getMomentum()[0];
    PHEP2 = mcp->getMomentum()[1];
    PHEP3 = mcp->getMomentum()[2];

    PHEP5 = mcp->getMass();

    // create G4PrimaryParticle object
    G4PrimaryParticle* particle 
      = new G4PrimaryParticle( IDHEP, PHEP1*GeV, PHEP2*GeV, PHEP3*GeV );

    particle->SetMass( PHEP5*GeV );


    HepLCIOInterfaceNew::Map[ mcp ] = particle ; 


  }

  // check if there is at least one particle
//   if( HPlist.size() == 0 ) return; 
  if( NHEP == 0 ) return ;



  // make connection between daughter particles decayed from 
  // the same mother

  std::set<G4PrimaryParticle*> daughtersUsed ;

  for( size_t i=0; i< col.size(); i++ ){

    MCParticle* mcp = col[i] ;

    ISTHEP =  mcp->getGeneratorStatus();

    // use only stable particles and preassigned decays
    if( ! ( ISTHEP == 1 ||  ISTHEP == 2 ) )  
      continue ; 

//     for( unsigned int j=0 ; j < mcp->getDaughters().size() ; j++ ) {
//       G4PrimaryParticle* mother = HepLCIOInterfaceNew::Map[ mcp ] ;
//       G4PrimaryParticle* daughter = HepLCIOInterfaceNew::Map[ mcp->getDaughters()[j]  ] ;
      
    HepLCIOInterfaceNew::LCIO2Geant4Map::iterator mcpIT ;
    mcpIT = HepLCIOInterfaceNew::Map.find( mcp ) ;
    
    //    G4PrimaryParticle* mother = 0 ;
    //     if ( mcpIT == Map.end() ) {
    //       std::cout << " HepLCIOInterface: null pointer " << mcp << " - pdg: " << mcp->getPDG() 
    // 		<< " genstat: " << mcp->getGeneratorStatus() 
    // 		<< " E = " << mcp->getEnergy() 
    // 		<< std::endl ;
    //     } else {
    //       mother = mcpIT->second ;
    //     }
    
    assert( mcpIT != HepLCIOInterfaceNew::Map.end() )  ;

    G4PrimaryParticle* mother = mcpIT->second ;



    for( unsigned int j=0 ; j < mcp->getDaughters().size() ; j++ ) {
      
      
      MCParticle* dau = mcp->getDaughters()[j] ;

      mcpIT = HepLCIOInterfaceNew::Map.find( dau ) ;

      //      G4PrimaryParticle* daughter = 0 ;
      //       if ( mcpIT == HepLCIOInterfaceNew::Map.end() ) {
      // 	std::cout << " null pointer " << mcp << " - pdg: " << mcp->getPDG() 
      // 		  << " genstat: " << mcp->getGeneratorStatus() 
      // 	  //		  << " realGenStat: " << mcp->ext<RealGeneratorStatus>() 
      // 		  << " E = " << mcp->getEnergy() 
      // 		  << std::endl ;
      // 	std::cout << " daughter null pointer " << dau << " - pdg: " << dau->getPDG() 
      // 		  << " genstat: " << dau->getGeneratorStatus() 
      // 	  //		  << " realGenStat: " << dau->ext<RealGeneratorStatus>() 
      // 		  << " E = " << dau->getEnergy() 
      // 		  << std::endl ;
      //       } else {
      // 	daughter = mcpIT->second ;
      //       }

      assert( mcpIT != HepLCIOInterfaceNew::Map.end() ) ; 
      G4PrimaryParticle* daughter = mcpIT->second ;
    
      if( daughtersUsed.find( daughter ) == daughtersUsed.end() ) 
	{  // force daughters to have unique mothers
	
	  mother->SetDaughter( daughter );
	  
	  daughtersUsed.insert( daughter );
	  //
	  // Computation of proper_time (following Ron Cassell's code in SLIC)
	  //
	  if (j == 0 ) // Need just the first daughter to calculate 
	               // the mother proper_time
	    {
	      G4ThreeVector parMom = mother->GetMomentum();
	      G4double E = sqrt(pow(mother->GetMass(), 2) 
				+ pow( parMom.x(), 2 ) + pow( parMom.y(), 2)
				+ pow( parMom.z(), 2 ) );

	      MCParticle* mcpdau = mcp->getDaughters()[j];
	      G4double proper_time = 
		( ( mcpdau->getTime() - mcp->getTime() ) * mother->GetMass() ) / E;
	      mother->SetProperTime( proper_time );
// 	      G4cout << "\n\nproper_time calculation for particle " << j
// 		     << " : mother's PDG = " 
// 		     << mother-> GetPDGcode()
// 		     << "\n  mother tima = " << mcp->getTime()
// 		     << ", daughter time = " << mcpdau->getTime()
// 		     << ", proper_time = "
// 		     << proper_time
// 		     << G4endl;
	    }
	}
      
      //       else {  std::cout << " ----- daughter already used in relationship : " 
      // 			<<  mother << " - " << daughter 
      // 			<< std::endl ;
      //       }
      
    }
  }
  
  
  // create G4PrimaryVertex object
  G4PrimaryVertex* vertex = new G4PrimaryVertex(particle_position,particle_time);
  
  
//   double E_tot = 0.0 ;

  // put initial particles to the vertex
  for( size_t i=0; i< col.size(); i++ ){
    
    MCParticle* mcp = col[i] ;
    
    // this should avoid double counting of energy in geant4
    // at least this seems to work for pythia 

    if( ! isDecayProduct( mcp ) && ( mcp->getGeneratorStatus() == 2 || mcp->getGeneratorStatus()  == 1  ) ) {
      
//       E_tot += mcp->getEnergy() ;
//       std::cout << " HepLCIOInterface::GeneratePrimaryVertex: adding particle with pdg: " 
// 		<< mcp->getPDG() << " and energy: " << mcp->getEnergy()  
// 		<< " status : ["  << mcp->getGeneratorStatus() <<"]" << std::endl ;
      
      G4PrimaryParticle* initialParticle = HepLCIOInterfaceNew::Map[ mcp ] ;
      
      if( initialParticle ==0 ) {
	std::cout << " HepLCIOInterface::GeneratePrimaryVertex: adding null pointer primary !" 
		  << std::endl ;
      }
      
      vertex->SetPrimary( initialParticle );
      
    }
  }
//   std::cout << " >>>>>> HepLCIOInterface::GeneratePrimaryVertex: E_tot: " << E_tot << std::endl ;
  
   // Put the vertex to G4Event object
  evt->AddPrimaryVertex( vertex );
  


//  LCTOOLS::printMCParticles( Control::lcMCVec ) ;


}

 bool HepLCIOInterface::isDecayProduct( MCParticle* mcp){
  
  bool isDecayProduct = false ;
  const MCParticleVec& parents = mcp->getParents() ;

  for( size_t i=0; i< parents.size(); i++ ){
    
    // if a parent has status 2 this particle is a decay product
    if( parents[i]->getGeneratorStatus() == 2 ){
      
      isDecayProduct = true ;
      break ;
    }
  }

  return isDecayProduct ;
} 


void HepLCIOInterface::setCharge (G4int IHEP)
{
  //
  // Initialise the particle charge looking at the actual 
  // Particle Table, or set it to -1000 to flag missing 
  // information, if PDG unknown in the actual physics list..
  //

 G4ParticleTable* theParticleTable =
   G4ParticleTable::GetParticleTable();

  lcio::MCParticleImpl* mcp_impl =
    dynamic_cast<MCParticleImpl*> ( Control::lcMCVec->getElementAt( IHEP ) ) ;

  G4ParticleDefinition* theParticle =
    theParticleTable->FindParticle(mcp_impl->getPDG());
  
  if( theParticle )
    mcp_impl->setCharge(theParticle->GetPDGCharge());
  else
    mcp_impl->setCharge(-1000);
}

#endif
