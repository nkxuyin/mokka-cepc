// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: RandomPlugin.cc,v 1.1 2006/10/24 10:00:29 adrian Exp $
// $Name: mokka-07-00 $

#include "RandomPlugin.hh"
#include "RandomPluginMessenger.hh"

#include "Randomize.hh"
#include <time.h>
#include <unistd.h>

INITPLUGIN(RandomPlugin, "RandomPlugin")

void RandomPlugin::Init(void)
{
  _messenger = new RandomPluginMessenger(this);
}

void RandomPlugin::Exit(void)
{
  delete _messenger;
}

long RandomPlugin::GetRandomSeed(void) const
{
  return G4RandGauss::getTheSeed();
}

void RandomPlugin::SetRandomSeed(long seed) const
{
  if (seed)
    G4RandGauss::setTheSeed(seed);
  else {
    seed = time(0) * getpid();
    seed = ((seed & 0xFFFF0000) >> 16) | ((seed & 0x0000FFFF) << 16);
    G4RandGauss::setTheSeed(std::abs(seed));
  }
}
