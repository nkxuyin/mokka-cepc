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
// $Id: Mask04.hh,v 1.2 2006/02/14 15:28:25 mora Exp $
// $Name: mokka-07-00 $
//
#ifndef Mask04_h
#define Mask04_h 1

class G4LogicalVolume;
class Database;

#include "VSubDetectorDriver.hh"

class Mask04 : public VSubDetectorDriver
{
public:
  Mask04() : VSubDetectorDriver("mask04","mask"), 
	   db(0)
  {}

  ~Mask04() {};
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);
  
private:

  Database* db;
  G4double mask1_z1, mask1_z2, mask1_rmin, mask1_rout;
  G4double mask2_z1, mask2_z2, mask2_rmin, mask2_rout;
  G4double mask3_z1, mask3_z2, mask3_rmin, mask3_rout;
  G4double mask4_z1, mask4_z2, mask4_rmin, mask4_rout;

};

#endif


