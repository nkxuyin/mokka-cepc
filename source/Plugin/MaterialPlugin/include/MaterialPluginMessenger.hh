// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: MaterialPluginMessenger.hh,v 1.2 2006/06/18 00:12:26 adrian Exp $
// $Name: mokka-07-00 $

#ifndef MaterialPluginMessenger_hh
#define MaterialPluginMessenger_hh 1

#include "G4UImessenger.hh"

class MaterialPlugin;
class G4UIcmdWithoutParameter;
class G4UIcmdWithAString;

class MaterialPluginMessenger: public G4UImessenger
{
public:
  MaterialPluginMessenger(MaterialPlugin *handler);
  ~MaterialPluginMessenger(void);
  
  void SetNewValue(G4UIcommand *command, G4String newValue);

private:
  G4UIcmdWithAString      *_printMaterialInfoCmd;
  G4UIcmdWithoutParameter *_printMaterialLengthsCmd;
    
  MaterialPlugin *_materialPlugin;
};

#endif
