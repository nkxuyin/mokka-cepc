#ifndef BeamCalSD00_hh
#define BeamCalSD00_hh 1
/* 
 * First implementation of BeamCal detector 
 * 
 */

/* Source from Fcal collaboration, implemented by A.Hartin */
/* Oct. 2008 */

#include "VSensitiveDetector.hh"

#include "G4SDManager.hh"
#include "G4TouchableHandle.hh"
#include "G4StepPoint.hh"
#include "G4VPhysicalVolume.hh"
#include "G4NavigationHistory.hh"
#include "G4TouchableHistory.hh"
#include "G4PVReplica.hh"
#include "Control.hh"

#include "G4Step.hh"
#include "G4ThreeVector.hh"

class BeamCal00;

#include "CalHit.hh"

typedef G4THitsCollection<CalHit> BCHitsCollection;

class BeamCalSD00: public VSensitiveDetector
{
public:
  BeamCalSD00(G4String name, 
	      G4double Rin, 
	      G4double Rout,
	      G4double cAngle,
	      G4double zS,
	      G4double sPhi,
	      G4double dPhi,
	      G4double nWafers,
	      G4double DAStart,
	      G4double dLayer,
	      G4int nLayers,
	      G4double dSensor,
	      G4double dAbsorber,
	      G4double segm,
	      G4double pairsMonitorZ,
	      G4double envVolLength,
	      G4double envVolCenterZ);

  ~BeamCalSD00(void);
  
  void Initialize(G4HCofThisEvent *eventHC);
  G4bool ProcessHits(G4Step *step, G4TouchableHistory *history);
  void EndOfEvent(G4HCofThisEvent *eventHC);
  void LoadEvent(FILE *eventFile);

  G4int nLayers;
  G4double dR, nRs,SegmdR;
  G4double Rinner, Router,xAngle,zStart, sPhi, dPhi, dLayer, dSensor, dAbsorber, Segm, nWafers, DAStart, pairsMonitorZ, envVolumeLength, envVolumeCenterZ;	

  G4double WaferPhiRange;
  G4double DAPhiRange;
   
  std::vector <G4double> SegmdPhi;
  std::vector <G4int> nPhis;

  inline G4double getPairsMonitorZ (G4double pairsMonZ)
  {
    pairsMonitorZ=pairsMonZ;
    return pairsMonitorZ;
  }

  inline G4int getNLayers (G4int nL)
  {
    nLayers=nL;
    return nLayers;
  }
  

  inline G4double getRin (G4double Rin)
  {
    Rinner=Rin;
    return Rinner;
  }

  inline G4double getRout (G4double Rout)
  {
    Router=Rout;
    return Router;
  }

  inline G4double getxAngle (G4double cAngle)
  {
    xAngle=cAngle/mrad;
    return xAngle;
  }

  inline G4double getZ (G4double zS)
  {
    zStart=zS;
    return zStart;
  }


  inline G4double getSPhi (G4double x)
  {
    sPhi=x;
    return sPhi;
  }
 
  inline G4double getDPhi (G4double x)
  {
    dPhi=x;
    return dPhi;
  }

  inline G4double getWafer (G4double x)
  {
    nWafers=x;
    return nWafers;
  }

  inline G4double getDA (G4double x)
  {
    DAStart=x;
    return x;
  }

  inline G4double getDl (G4double x)
  {
    dLayer=x;
    return dLayer;
  }

  inline G4double getDS (G4double x)
  {
    dSensor=x;
    return dSensor;
  }

  inline G4double getDAb (G4double x)
  {
    dAbsorber=x;
    return dAbsorber;
  }

  inline G4double getSegm (G4double x)
  {
    Segm=x;
    return Segm;
  }

  BCHitsCollection *CalCollection;
  G4int HCID;

};

inline G4double split_segm(G4double totLength, G4double initSegm)
{
  G4int n;
  n = G4int(totLength/initSegm);
  return totLength/(G4double(n+1));
}

inline G4int split_n(G4double totLength, G4double initSegm)
{
  return G4int(ceil(totLength/initSegm));
}


#endif
