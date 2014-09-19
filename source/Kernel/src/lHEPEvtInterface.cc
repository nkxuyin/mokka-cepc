
#include "G4Types.hh"

#include "G4ios.hh"
#include "lHEPEvtInterface.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include "G4HEPEvtParticle.hh"
#include "G4Event.hh"
#include "lStdHep.hh"

#include "Control.hh"

lHEPEvtInterface::lHEPEvtInterface(G4String evfile) 
  : theStdHep(NULL)
{
  // print version
  G4cout << lStdHep::getText() << G4endl;

  // open input stdhep binary file
  theStdHep = new lStdHep( evfile.data() );
  if(theStdHep->getError())
    Control::Abort("lHEPEvtInterface:: cannot open file.",
		MOKKA_ERROR_STDHEP_FILE_NOT_FOUND);

  G4cout << evfile << " binary event file has " 
	 << theStdHep->numEvents()
	 << " events." <<  G4endl;

  G4ThreeVector zero;
  particle_position = zero;
  particle_time = 0.0;
}

lHEPEvtInterface::~lHEPEvtInterface()
{
  if(theStdHep) delete theStdHep;
}

void lHEPEvtInterface::GeneratePrimaryVertex(G4Event* evt)
{


//   std::cout << " lHEPEvtInterface::GeneratePrimaryVertex for event: " << evt->GetEventID() << std::endl ;


  if(!theStdHep) Control::Abort("lHEPEvtInterface: file not opened",
		MOKKA_ERROR_STDHEP_FILE_NOT_FOUND);
  G4int NHEP;  // number of entries
//
// Read in the event
//

  std::stringstream eventNumber;
  eventNumber << "Event " <<  evt->GetEventID() + Control::SYNCEVT;

  if(theStdHep->more())
    {
      if(G4int errorcode = theStdHep->readEvent())
	{
      	  Control::RunAborted = true;
	  if (errorcode == LSH_ENDOFFILE)
      		G4Exception("lHEPEvtInterface::GeneratePrimaryVertex", 
			eventNumber.str().c_str(),
			RunMustBeAborted, 
			"*** The end of stdhep event file was reached");

	  else {
	    G4cout << "lHEPEvtInterface: Error " 
		   << errorcode 
		   << "when reading stdhep file" << G4endl;

   	    G4Exception("lHEPEvtInterface::GeneratePrimaryVertex", 
                        eventNumber.str().c_str(),
                        RunMustBeAborted, 
			"*** Error when reading stdhep file");
	  }

      	  return;
	}
    }
  else {
        Control::RunAborted = true;
   	G4Exception("lHEPEvtInterface::GeneratePrimaryVertex", 
			eventNumber.str().c_str(),
			RunMustBeAborted, 
			"*** The end of stdhep event file was reached");
      	return;
  }

  NHEP = theStdHep->nTracks();

  for( G4int IHEP=0; IHEP<NHEP; IHEP++ )
  {
    G4int ISTHEP;   // status code
    G4int IDHEP;    // PDG code
    G4int JDAHEP1;  // first daughter
    G4int JDAHEP2;  // last daughter
    G4double PHEP1; // px in GeV
    G4double PHEP2; // py in GeV
    G4double PHEP3; // pz in GeV
    G4double PHEP5; // mass in GeV
    
    ISTHEP = theStdHep->status(IHEP);
    IDHEP = theStdHep->pid(IHEP);    
    JDAHEP1 = theStdHep->daughter1(IHEP)%10000;
    JDAHEP2 = theStdHep->daughter2(IHEP)%10000;
    PHEP1 = theStdHep->Px(IHEP);
    PHEP2 = theStdHep->Py(IHEP);
    PHEP3 = theStdHep->Pz(IHEP);
    PHEP5 = theStdHep->M(IHEP);

    // create G4PrimaryParticle object
    G4PrimaryParticle* particle 
      = new G4PrimaryParticle( IDHEP, PHEP1*GeV, PHEP2*GeV, PHEP3*GeV );
    particle->SetMass( PHEP5*GeV );

    // create G4HEPEvtParticle object
    G4HEPEvtParticle* hepParticle
      = new G4HEPEvtParticle( particle, ISTHEP, JDAHEP1, JDAHEP2 );

    // Store
    HPlist.push_back( hepParticle );
  }

  // check if there is at least one particle
  if( HPlist.size() == 0 ) return; 

  // make connection between daughter particles decayed from 
  // the same mother
  for( size_t i=0; i<HPlist.size(); i++ )
  {
    if( HPlist[i]->GetJDAHEP1() > 0 ) //  it has daughters
    {
      G4int jda1 = HPlist[i]->GetJDAHEP1()-1; // FORTRAN index starts from 1
      G4int jda2 = HPlist[i]->GetJDAHEP2()-1; // but C++ starts from 0.
      G4PrimaryParticle* mother = HPlist[i]->GetTheParticle();
      for( G4int j=jda1; j<=jda2; j++ )
      {
        G4PrimaryParticle* daughter = HPlist[j]->GetTheParticle();
        if(HPlist[j]->GetISTHEP()>0)
        {
          mother->SetDaughter( daughter );
          HPlist[j]->Done();        // <<<<<< this  sets ISTHEP to negative values !
        }
      }
    }
  }

  // create G4PrimaryVertex object
  G4PrimaryVertex* vertex = new G4PrimaryVertex(particle_position,particle_time);

  // put initial particles to the vertex
  for( size_t ii=0; ii<HPlist.size(); ii++ )
  {
    if( HPlist[ii]->GetISTHEP() > 0 ) // ISTHEP of daughters had been 
                                       // set to negative
    {
      G4PrimaryParticle* initialParticle = HPlist[ii]->GetTheParticle();
      vertex->SetPrimary( initialParticle );
    }
  }

  // clear G4HEPEvtParticles
  //HPlist.clearAndDestroy();
  for(size_t iii=0;iii<HPlist.size();iii++)
  { delete HPlist[iii]; }
  HPlist.clear();

  // Put the vertex to G4Event object
  evt->AddPrimaryVertex( vertex );
}

