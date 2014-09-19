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
// $Id: Tmag06.hh,v 1.2 2007/02/27 11:47:41 predrag Exp $
// $Name: mokka-07-00 $
//
//
#ifndef Tmag06_h
#define Tmag06_h 1

class G4LogicalVolume;
class Database;

#include "VSubDetectorDriver.hh"

class Tmag06 : public VSubDetectorDriver
{
public:
  Tmag06() : VSubDetectorDriver("Tmag06"){}

  ~Tmag06() {};
  

 G4bool ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog);
private:
};

#endif


