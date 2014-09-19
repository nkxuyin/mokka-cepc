//
//*******************************************************
//*                                                     *
//*                      Mokka                          *
//*   - the detailed geant4 simulation for Tesla -      *
//*                                                     *
//* For more information about Mokka, please, go to the *
//*                                                     *
//*  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
//*                                                     *
//* Mokka home page.                                    *
//*                                                     *
//*******************************************************
//
//

#ifndef TRKSD_FTD01_HH
#define TRKSD_FTD01_HH 1

//#define DEBUGTRKSD1
//#define DEBUGTRKSD2

// Mokka & Geant4 classes
#include "G4Step.hh"
#include "TRKFTDHit.hh"
#include "VSensitiveDetector.hh"

class G4Step;

//! Implementation of Mokka FTD sensitive class.
//! Sensitive detector reduces a number of hits to 1 per detector and per
//! each particle. Threshold parameter (given in Geant4 internal units)
//! defines if the detector is capable to register the particle. If yes
//! then the hit information is saved to LCIO (position is defined in mm,
//! energy (momentum) in GeV).
//!
//! @author Z. Drasal, Charles University Prague (based on TRKSD00 sens. detector)
//! @author J. Duarte Campderros, IFCA (adapted to FTD), Oct-2011
//!

class TRKSD_FTD01 : public VSensitiveDetector
{
	public:
		//!Constructor
		TRKSD_FTD01(G4String SDname, G4double theThreshold, G4bool mergeSensHits=true);
		
		//!Destructor
		~TRKSD_FTD01() {}
		
		//!Method invoked at the beginning of each event
		void Initialize(G4HCofThisEvent * HCTE );
		
		//!Method invoked for each step in a sensitive detector
		G4bool ProcessHits(G4Step * aStep,G4TouchableHistory * stepTouchHist);
		
		//!Method invoked at the end of each event
		void EndOfEvent(G4HCofThisEvent * HCTE );
		
		//!Print functions
		void DrawAll() {}
		void PrintAll() {}
		
		//!Reload hits from Ascii file
		void LoadEvent(FILE * iFile);
		
	private:
		//!Define new collection hit that is calculated as an average of hits created by the same particle
		void CreateNewColHit(G4Step * step, G4ThreeVector preStepPos, G4ThreeVector posStepPos,
				G4ThreeVector preStepMomentum, G4ThreeVector posStepMomentum,
				G4double depEnergy, G4double globTime, G4int iLayer, G4int iSide, G4int iLadder,
				G4int iSensor, G4int trackID, G4int partPDG);
		
		//!Update collection hit, i.e. add information from another Geant4 hit
		void UpdateColHit(G4ThreeVector posStepPos, G4ThreeVector posStepMomentum, G4double depEnergy,
				G4double globTime);
		
		//!Save collection hit as LCIO hit
		void SaveColHit();
		
		//!Clear collection hit
		void ClearColHit();
		
		// Threshold for hits (if deposited energy is less then the threshold, Geant4 hit thrown away)
		G4double _thresholdFTD;
		
		// Merge hits into one hit/layer/particle
		G4bool _mergeSensHits;
		
		// Previous layer, ladder, sensor (zero for passive materials, non-zero for active materials)
		G4int _previousLayer;
		G4int _previousLadder;
		G4int _previousSensor;
		
		// Previous track ID
		G4int _previousTrackID;
		
		// Previous left medium
		G4bool _previousLeft;
		
		// Collection hit - an average of n hits that were created by the same particle in 1 layer
		G4ThreeVector _preStepPos;
		G4ThreeVector _posStepPos;
		G4ThreeVector _preStepMomentum;
		G4ThreeVector _posStepMomentum;
		G4double      _stepLength;
		G4double      _depEnergy;
		G4double      _globTime;
		G4int         _nHits;
		
		// Define which layer, which ladder and which sensor + track ID and particle ID
		G4int         _iLayer;
		G4int         _iSide;
		G4int         _iLadder;
		G4int         _iSensor;
		G4int         _trackID;
		G4int         _partPDG;
		
		// Final hit collection
		TRKFTDHitsCollection * _aCollection;
		G4int _collectionID;
		
}; // Class

#endif // TRKSD_FTD01_HH
