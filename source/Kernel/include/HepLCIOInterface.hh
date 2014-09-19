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
// $Id: HepLCIOInterface.hh,v 1.3 2008/10/31 14:29:32 frank Exp $
// $Name: mokka-07-00 $
//

#ifndef HepLCIOInterface_h
#define HepLCIOInterface_h 1

#ifdef LCIO_MODE  // ---- only works with LCIO --------

#include <fstream>
#include <vector>
#include "globals.hh"
#include "G4VPrimaryGenerator.hh"
#include "G4HEPEvtParticle.hh"

#include "lcio.h"
#include "EVENT/MCParticle.h"
#include "UTIL/LCStdHepRdr.h"
#include "LCAscHepRdr.h"
#include <map>

class G4PrimaryVertex;
class G4Event;

typedef enum HEPFILEFORMATS
  {
    stdhep = 0,
    HEPEvt,
    hepevt
  }  HEPFILEFORMAT;

/** Implementation of G4VPrimaryGenerator that reads in an StdHep file
 *  and creates an LCIO MCParticle collection and uses that to
 *  create the G4PrimaryParticles needed by geant4. 
 *  This results in a complete 'HepEvt' recors in the MCparticle collection.
 * 
 *  @author F.Gaede, DESY
 */


class HepLCIOInterface:public G4VPrimaryGenerator
{
  public:

  //typedef std::map< lcio::MCParticle*  , G4PrimaryParticle* > LCIO2Geant4Map ;

  HepLCIOInterface(G4String evfile,  HEPFILEFORMAT FileFormat = stdhep);
  // Constructors, "evfile" is the file name (with directory path).
  
public:
  virtual ~HepLCIOInterface();
  
  void GeneratePrimaryVertex(G4Event* evt);
  

  //  static LCIO2Geant4Map Map ;
  
protected:
  /** Helper method that returns true if the particle has no parent with generator status equal to 2 */
  virtual bool isDecayProduct( lcio::MCParticle* mcp ) ;

  // Helper method to assign charge for known particles in
  // physics list, or -1000 for unknown ones. (PMdeF)
  
  virtual void setCharge (G4int);
  
private:
  HEPFILEFORMAT      theHepFileFormat;
  lcio::LCStdHepRdr* theStdHepRdr;
  lcio::LCAscHepRdr* theAscHepRdr;
  bool isLeptonCascadeParticle(lcio::MCParticle* mcp);
  bool isNeutrino(G4int IDHEP);
  bool isTopQuark(G4int IDHEP);

};

#endif
#endif 
