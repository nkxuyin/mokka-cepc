// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
//
// "TrackingOnlyPlugin" invoke to stop particles if they leave the tracking region.
//
// Steve Aplin DESY
//

#ifndef TrackingOnlyPlugin_hh
#define TrackingOnlyPlugin_hh 1

#include "Plugin.hh"

class TrackingOnlyPlugin: public Plugin
{
public:
  TrackingOnlyPlugin(const std::string &name): Plugin(name) {}
  virtual ~TrackingOnlyPlugin() {}

  virtual void Init();
  virtual void Exit();

  virtual void BeginOfRunAction(const G4Run *run);
  virtual void EndOfRunAction(const G4Run *run);

  virtual void BeginOfEventAction(const G4Event *evt);
  virtual void EndOfEventAction(const G4Event *evt);

  virtual void PreUserTrackingAction(const G4Track *trk);
  virtual void PostUserTrackingAction(const G4Track *trk);

  virtual void UserSteppingAction(const G4Step *step);

protected:
  TrackingOnlyPlugin();

private:
  double tracking_radius_max;
  double tracking_z_max;

};  

#endif // TrackingOnlyPlugin_hh
