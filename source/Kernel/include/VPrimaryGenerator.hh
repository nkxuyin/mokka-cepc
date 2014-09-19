// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: VPrimaryGenerator.hh,v 1.2 2007/06/22 14:42:50 musat Exp $
// $Name: mokka-07-00 $

#ifndef VPrimaryGenerator_hh
#define VPrimaryGenerator_hh 1

#include "globals.hh"
class G4Event;

class VPrimaryGenerator
{
public:
  VPrimaryGenerator(G4String name): fGeneratorName(name)
    {;}

  virtual ~VPrimaryGenerator(void){};

  virtual void GeneratePrimaryVertex(G4Event *){
	G4cout << "VPrimaryGenerator::GeneratePrimaryVertex: Not implemented!" 
	       << G4endl;
  }

  virtual G4String GetGeneratorName(void) { return fGeneratorName; }
  virtual void SetGeneratorName(G4String name) { 
	fGeneratorName = name; 
  }

  virtual void PrintGeneratorInfo(void) {
	G4cout << "VPrimaryGenerator::PrintGeneratorInfo - Not implemented!" <<
	G4endl;
  }

  virtual bool AppliesLorentzTransform() { return false ; }

protected:
  G4String             fGeneratorName;
};

#endif
