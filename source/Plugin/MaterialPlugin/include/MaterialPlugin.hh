// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: MaterialPlugin.hh,v 1.2 2006/06/18 00:12:26 adrian Exp $
// $Name: mokka-07-00 $

#ifndef MaterialPlugin_hh
#define MaterialPlugin_hh 1

#include "Plugin.hh"

class MaterialPluginMessenger;

class MaterialPlugin: public Plugin
{
public:
  MaterialPlugin(const std::string &name): Plugin(name) {}
  ~MaterialPlugin(void) {}
  
  void Init(void);
  void Exit(void);

  void PrintMaterialInfo(const G4String &name) const;
  void PrintMaterialLengths(void) const;

private:
  MaterialPluginMessenger *_messenger;
};

#endif
