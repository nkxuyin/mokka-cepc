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
// $Id: lHEPEvtInterface.hh,v 1.1 2004/08/06 15:50:39 mora Exp $
// $Name: mokka-07-00 $
//

//========================================================
// This code is a G4HEPEvtInterface adaptation to read
// binary stdhep files using the lStdHep light-weight StdHep 
// class wrote by W.G.J. Langeveld.
//========================================================

#ifndef lHEPEvtInterface_h
#define lHEPEvtInterface_h 1

#include <fstream>
#include <vector>
#include "globals.hh"
#include "G4VPrimaryGenerator.hh"
#include "G4HEPEvtParticle.hh"

class lStdHep;
class G4PrimaryVertex;
class G4Event;

// class description:
//
//  This is a concrete class of G4VPrimaryGenerator.
//
// The position and time of the primary interaction must be set by 
// the corresponding set methods of G4VPrimaryGenerator base class, 
// otherwise zero will be set.

class lHEPEvtInterface:public G4VPrimaryGenerator
{
  public:
  lHEPEvtInterface(G4String evfile);
  // Constructors, "evfile" is the file name (with directory path).
  
public:
  virtual ~lHEPEvtInterface();
  
  void GeneratePrimaryVertex(G4Event* evt);
  
private:
  lStdHep* theStdHep;
  std::vector<G4HEPEvtParticle*> HPlist;
};

#endif

