// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: Plugin.cc,v 1.2 2006/04/25 13:12:32 adrian Exp $
// $Name: mokka-07-00 $

#include "Plugin.hh"
#include "PluginManager.hh"

Plugin::Plugin(const std::string &name): _name(name)
{
  // register module in the map of available (but not necessarily active) plugins
  PluginManager::getInstance()->registerPlugin(this);
}
