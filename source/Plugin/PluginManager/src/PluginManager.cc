// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: PluginManager.cc,v 1.3 2006/04/25 13:12:32 adrian Exp $
// $Name: mokka-07-00 $

#include "PluginManager.hh"

PluginManager *PluginManager::_me = 0;

PluginManager *PluginManager::getInstance(void)
{
  if (_me == 0)
    _me = new PluginManager();
  return _me;
}

void PluginManager::registerPlugin(Plugin *plugin)
{
  const std::string &name = plugin->getName();
  if (_registeredPlugins.find(name) == _registeredPlugins.end())
    _registeredPlugins[name] = plugin;
  else
    G4cout << "WARNING: A plugin with name \"" << name << "\" already exists." << G4endl;
}

void PluginManager::activatePlugin(const std::string &name)
{
  if (_registeredPlugins.find(name) == _registeredPlugins.end())
    G4cout << "WARNING: There is no plugin with name \"" << name << "\"." << G4endl;
  else
    _activePlugins.push_back(_registeredPlugins[name]);
}

void PluginManager::Init(void) const
{
  for (std::vector<Plugin *>::const_iterator plugin = _activePlugins.begin(); plugin != _activePlugins.end(); plugin++)
    (*plugin)->Init();
}

void PluginManager::Exit(void) const
{
  for (std::vector<Plugin *>::const_iterator plugin = _activePlugins.begin(); plugin != _activePlugins.end(); plugin++)
    (*plugin)->Exit();
}

void PluginManager::BeginOfRunAction(const G4Run *run) const
{
  for (std::vector<Plugin *>::const_iterator plugin = _activePlugins.begin(); plugin != _activePlugins.end(); plugin++)
    (*plugin)->BeginOfRunAction(run);
}

void PluginManager::EndOfRunAction(const G4Run *run) const
{
  for (std::vector<Plugin *>::const_iterator plugin = _activePlugins.begin(); plugin != _activePlugins.end(); plugin++)
    (*plugin)->EndOfRunAction(run);
}

void PluginManager::BeginOfEventAction(const G4Event *evt) const
{
  for (std::vector<Plugin *>::const_iterator plugin = _activePlugins.begin(); plugin != _activePlugins.end(); plugin++)
    (*plugin)->BeginOfEventAction(evt);
}

void PluginManager::EndOfEventAction(const G4Event *evt) const
{
  for (std::vector<Plugin *>::const_iterator plugin = _activePlugins.begin(); plugin != _activePlugins.end(); plugin++)
    (*plugin)->EndOfEventAction(evt);
}

void PluginManager::PreUserTrackingAction(const G4Track *trk) const
{
  for (std::vector<Plugin *>::const_iterator plugin = _activePlugins.begin(); plugin != _activePlugins.end(); plugin++)
    (*plugin)->PreUserTrackingAction(trk);
}

void PluginManager::PostUserTrackingAction(const G4Track *trk) const
{
  for (std::vector<Plugin *>::const_iterator plugin = _activePlugins.begin(); plugin != _activePlugins.end(); plugin++)
    (*plugin)->PostUserTrackingAction(trk);
}

void PluginManager::UserSteppingAction(const G4Step *step) const
{
  for (std::vector<Plugin *>::const_iterator plugin = _activePlugins.begin(); plugin != _activePlugins.end(); plugin++)
    (*plugin)->UserSteppingAction(step);
}
