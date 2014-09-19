// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: LogPlugin.hh,v 1.1 2006/02/23 15:15:33 adrian Exp $
// $Name: mokka-07-00 $

#ifndef LogPlugin_hh
#define LogPlugin_hh 1

#include "Plugin.hh"
#include <string>
#include <fstream>

class LogPlugin: public Plugin
{
public:
  LogPlugin(const std::string &name): Plugin(name) {}
  ~LogPlugin(void) {}

  void Init(void);
  void Exit(void);

  void BeginOfRunAction(const G4Run *run);
  void EndOfRunAction(const G4Run *run);
  void EndOfEventAction(const G4Event *event);

private:
  std::string GetCurrentTime(void) const;
  
  std::ofstream *fLogFile;
  G4int fLogStep;
  G4int fEventCounter;
  G4int fEventTotal;
};

#endif
