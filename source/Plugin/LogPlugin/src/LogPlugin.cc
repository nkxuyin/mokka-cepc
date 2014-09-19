// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: LogPlugin.cc,v 1.2 2006/04/03 14:48:18 adrian Exp $
// $Name: mokka-07-00 $

#include "LogPlugin.hh"

#include "UserInit.hh"
#include "G4Run.hh"
#include <iomanip>
#include <ctime>

INITPLUGIN(LogPlugin, "LogPlugin")

void LogPlugin::Init(void)
{
  fLogFile = 0;
  
  // filename for the log file
  const G4String logName = UserInit::getInstance()->getString("logName");
  if (logName == "") return;
  
  // step width for the progress report (none if zero)
  fLogStep = abs(UserInit::getInstance()->getInt("logStep"));

  // write mode: append (false) or overwrite (true)
  const G4bool logClear = UserInit::getInstance()->getBool("logClear");
  const std::ios_base::openmode logMode = (logClear) ? (std::ios::out) : (std::ios::app);

  fLogFile = new std::ofstream(logName, logMode);
  *fLogFile << "#### Starting Mokka #### " << GetCurrentTime() << std::endl;
}

void LogPlugin::Exit(void)
{
  if (!fLogFile) return;
  *fLogFile << "#### Exiting  Mokka #### " << GetCurrentTime() << std::endl;
  fLogFile->close();
}

void LogPlugin::BeginOfRunAction(const G4Run *run)
{
  if (!fLogFile) return;
  fEventCounter = 0;
  fEventTotal = run->GetNumberOfEventToBeProcessed();
  *fLogFile << "Run " << std::setw(8) << run->GetRunID() << " started     " << GetCurrentTime() << std::endl;
}

void LogPlugin::EndOfRunAction(const G4Run *run)
{
  if (!fLogFile) return;
  *fLogFile << "Run " << std::setw(8) << run->GetRunID() << " finished    " << GetCurrentTime() << std::endl;
}

void LogPlugin::EndOfEventAction(const G4Event *)
{
  if (!fLogFile) return;
  fEventCounter++;
  // give a progress report each (fLogStep) events
  if (fLogStep && ((fEventCounter % fLogStep == 0) || (fEventCounter == fEventTotal)))
    *fLogFile << std::setw(12) << fEventCounter << " done"
      " (" << std::setw(3) << (100 * fEventCounter) / fEventTotal << "%) " << GetCurrentTime() << std::endl;
}

std::string LogPlugin::GetCurrentTime(void) const
{
  const std::time_t now(std::time(0)); // the current time ...
  const std::string nowStr(std::ctime(&now)); // ... as ASCII text
  return nowStr.substr(0, nowStr.length() - 1); // strip the trailing newline
}
