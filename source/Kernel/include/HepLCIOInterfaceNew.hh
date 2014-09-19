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
// $Id: HepLCIOInterfaceNew.hh,v 1.3 2008/10/31 14:29:32 frank Exp $
// $Name: mokka-07-00 $
//

#ifndef HepLCIOInterfaceNew_h
#define HepLCIOInterfaceNew_h 1

#ifdef LCIO_MODE  // ---- only works with LCIO --------

#include <fstream>
#include <vector>
#include <set>
#include "globals.hh"
#include "G4VPrimaryGenerator.hh"
#include "G4HEPEvtParticle.hh"
#include "Randomize.hh"

#include "lcio.h"
#include "EVENT/MCParticle.h"
#include "UTIL/LCStdHepRdr.h"
#include "LCAscHepRdr.h"
#include <map>

class G4PrimaryVertex;
class G4Event;

/** Implementation of G4VPrimaryGenerator that reads in an StdHep file
 *  and creates an LCIO MCParticle collection and uses that to
 *  create the G4PrimaryParticles needed by geant4. 
 *  This results in a complete G4PrimaryParticle tree.
 * 
 *  @author B.Vormwald, DESY
 */


class HepLCIOInterfaceNew: public G4VPrimaryGenerator
{
  public:
  typedef enum HEPFILEFORMATS
    {
      stdhep = 0,
      HEPEvt,
      hepevt
    }  HEPFILEFORMAT;
  
  
  typedef std::map< lcio::MCParticle*  , G4PrimaryParticle* > LCIO2Geant4Map ;
  
  // Constructors, "evfile" is the file name (with directory path).
  HepLCIOInterfaceNew(G4String evfile,  HEPFILEFORMAT FileFormat = stdhep);
  
public:
  virtual ~HepLCIOInterfaceNew();
  void GeneratePrimaryVertex(G4Event* evt);
  static LCIO2Geant4Map Map ;

protected:
  // Helper method to assign charge for known particles in
  // physics list, or -1000 for unknown ones. (PMdeF)
  virtual void setCharge (G4int);

private:
  HEPFILEFORMAT      theHepFileFormat;
  lcio::LCStdHepRdr* theStdHepRdr;
  lcio::LCAscHepRdr* theAscHepRdr;
  static std::set<lcio::MCParticle*> visitedParticles; //variable to avoid double counting

  std::set<G4PrimaryParticle*> getRelevantParticles(lcio::MCParticle* p);
};

#endif
#endif
