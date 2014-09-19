// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: PluginManager.hh,v 1.2 2006/04/25 13:12:32 adrian Exp $
// $Name: mokka-07-00 $
//
// The PluginManager holds a map of available plugins and a vector
// of registered plugins that are called for all relevant actions.

#ifndef PluginManager_hh
#define PluginManager_hh 1

#include "Plugin.hh"
#include "globals.hh"

#include <map>
#include <vector>

class PluginManager 
{
public:
  // access to the PluginManager singleton
  static PluginManager *getInstance(void);

  // add a plugin to the map of available plugins
  void registerPlugin(Plugin *plugin);
  
  // add a plugin to the vector of active plugins
  void activatePlugin(const std::string &name);
  
  virtual ~PluginManager(void) {}
  
  void Init(void) const;
  void Exit(void) const;
  
  void BeginOfRunAction(const G4Run *run) const;
  void EndOfRunAction(const G4Run *run) const;

  void BeginOfEventAction(const G4Event *evt) const;
  void EndOfEventAction(const G4Event *evt) const;
  
  void PreUserTrackingAction(const G4Track *trk) const;
  void PostUserTrackingAction(const G4Track *trk) const;

  void UserSteppingAction(const G4Step *step) const;

private:
  // the PluginManager is a singleton
  PluginManager(void) {}
  static PluginManager *_me;

  std::map<std::string, Plugin *> _registeredPlugins;
  std::vector<Plugin *> _activePlugins;
};  

#endif
