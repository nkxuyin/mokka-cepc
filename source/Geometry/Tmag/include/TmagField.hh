// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// 
// $Name: mokka-07-00 $

#ifndef TmagField_hh
#define TmagField_hh 1
#include "globals.hh"
#include "G4MagneticField.hh"
#include "VSubDetectorDriver.hh"

#include <vector>

class CGAGeometryEnvironment;
class G4LogicalVolume;
class TmagFieldMap;

class TmagField: public VSubDetectorDriver, public G4MagneticField
{
public:
  TmagField(): VSubDetectorDriver("TmagField") {};
  ~TmagField() {};

 G4bool ContextualConstruct(const CGAGeometryEnvironment &geometryEnv, G4LogicalVolume *worldLog);
  void GetFieldValue(const double point[4], double *bField) const;

private:
 
  double maparr[60][95][4];

};


#endif // TmagField_hh
