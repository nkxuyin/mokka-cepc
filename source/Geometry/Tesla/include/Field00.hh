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
// $Id: Field00.hh,v 1.2 2008/02/15 13:44:38 musat Exp $
// $Name: mokka-07-00 $
//
#ifndef Field00_h
#define Field00_h 1

#include "globals.hh"
#include "G4MagneticField.hh"
#include "VSubDetectorDriver.hh"

#include <vector>

class Field00 : public VSubDetectorDriver, 
		public G4MagneticField
{
public:
  Field00() : VSubDetectorDriver("field00"), FieldMap(0), FieldMapSize(0)
  {}

  ~Field00() {};
  
  G4bool construct(const G4String &aSubDetectorDBName,
		   G4LogicalVolume *theWorld);

  void GetFieldValue( const  double Point[3],
		      double *Bfield ) const;
  
private:
  typedef struct
  {
    G4double zmin;
    G4double zmax;
    G4double rmin;
    G4double rmax;
    G4double MagField [3];
  } FieldRegion;

 //std::vector<FieldRegion> FieldMap;
   FieldRegion **FieldMap;
   G4int FieldMapSize;
};

#endif


