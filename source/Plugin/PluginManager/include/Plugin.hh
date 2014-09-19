// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: Plugin.hh,v 1.1 2005/10/20 01:21:53 adrian Exp $
// $Name: mokka-07-00 $

#ifndef Plugin_hh
#define Plugin_hh 1

#include "G4Run.hh"
#include "G4Event.hh"
#include "G4Track.hh"
#include "G4Step.hh"

#include <string>

#define INITPLUGIN(plugin, name) plugin the##plugin(name);

class Plugin 
{
public:
  Plugin(const std::string &name);
  virtual ~Plugin(void) {}

  virtual void Init(void) {}
  virtual void Exit(void) {}

  virtual void BeginOfRunAction(const G4Run *) {}
  virtual void EndOfRunAction(const G4Run *) {}

  virtual void BeginOfEventAction(const G4Event *) {}
  virtual void EndOfEventAction(const G4Event *) {}

  virtual void PreUserTrackingAction(const G4Track *) {}
  virtual void PostUserTrackingAction(const G4Track *) {}

  virtual void UserSteppingAction(const G4Step *) {}

  const std::string &getName(void) const { return _name; }

private:
  // prevent users from calling the constructor without a name
  Plugin(void) {}

  std::string _name;
};  

#endif
