// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: RandomPlugin.hh,v 1.1 2006/10/24 10:00:29 adrian Exp $
// $Name: mokka-07-00 $

#ifndef RandomPlugin_hh
#define RandomPlugin_hh 1

#include "Plugin.hh"

class RandomPluginMessenger;

class RandomPlugin: public Plugin
{
public:
  RandomPlugin(const std::string &name): Plugin(name) {}
  ~RandomPlugin(void) {}

public: // from Plugin
  void Init(void);
  void Exit(void);

public:
  long GetRandomSeed(void) const;
  void SetRandomSeed(long seed) const;

private:
  RandomPluginMessenger *_messenger;
};

#endif
