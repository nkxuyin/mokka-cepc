// History:
//- used TRKSD00 and changed algorithm how to hit is calculated (fix for delta el.) Z. Drasal Jan 2009
//- added new logging, Z. Drasal Oct 2009
//- fixed hit merging - used to create artificially long hits, Z. Drasal May 2010
//- according to ideas of M. Ritter - added option not to merge hits in PXD & SVD, Z. Drasal Jul 2010
//- adapted to FTD: added the _iSide datamember in the hit identification, changed the 
//                  way to extract the identification info from the Sensitive Volumes usign
//                  the sensor id instead the layer id.                J. Duarte Campderros Oct. 2011

//#define DEBUGTRKSD 1

#include "TRKSD_FTD01.hh"
#include "EncoderSi.hh"

// Basic C
#include <assert.h>

// Geant4 globals
#include "globals.hh"

// Basic Mokka classes
#include "Control.hh"

// Geant4 and Mokka classes
#include "G4AffineTransform.hh"
#include "G4ios.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4SDManager.hh"
#include "G4TransportationManager.hh"
#include "G4TouchableHistory.hh"
#include "G4VPhysicalVolume.hh"
#include "UserTrackInformation.hh"
#include "TrackSummary.hh"

//
// Constructor
//
TRKSD_FTD01::TRKSD_FTD01(G4String SDname, G4double threshold, G4bool mergeSensHits) : VSensitiveDetector(SDname) ,
                 _thresholdFTD(threshold)   , _mergeSensHits(mergeSensHits), _previousLayer(0)    , _previousLadder(0)  ,
                 _previousSensor(0)         , _previousTrackID(-1)         , _previousLeft(false),
                 _preStepPos(0.,0.,0.)      , _posStepPos(0.,0.,0.)        , _preStepMomentum (0.,0.,0.),
                 _posStepMomentum (0.,0.,0.), _stepLength(0.)              , _depEnergy(0.),
                 _globTime(0.), _nHits(0)   , _aCollection(0)              , _collectionID(-1)
{
	// Define the name of a hit collection
	collectionName.insert(G4String(SDname+"Collection"));
}

//
// Method invoked at the beginning of each event
//
void TRKSD_FTD01::Initialize(G4HCofThisEvent * HCTE)
{
	// Create a new hit collection
	_aCollection = new TRKFTDHitsCollection(SensitiveDetectorName,collectionName[0]);
	
	// Assign a unique ID to the hits collection
	if (_collectionID<0) 
	{
		_collectionID = G4SDManager::GetSDMpointer()->GetCollectionID(_aCollection);
	}
	
	// Attach collections to HitsCollectionsofThisEvent
	HCTE -> AddHitsCollection(_collectionID, _aCollection);
	
	// Initialize
	_previousLayer   =  0;
	_previousLadder  = -1;
	_previousSensor  = -1;
	_previousTrackID = -1;
	_previousLeft    = false;
}

//
// Method invoked for every step in sensitive detector
//
G4bool TRKSD_FTD01::ProcessHits(G4Step * aStep,G4TouchableHistory *)
{
	// Get pre and pos step - positions, momenta
	G4StepPoint * preStep = aStep->GetPreStepPoint();
	G4StepPoint * posStep = aStep->GetPostStepPoint();
	
	G4ThreeVector preStepPos = preStep->GetPosition();
	G4ThreeVector posStepPos = posStep->GetPosition();
	
	G4ThreeVector preStepMomentum = preStep->GetMomentum();
	G4ThreeVector posStepMomentum = posStep->GetMomentum();
	
	// Get deposited energy and hit time
	G4double depEnergy  = aStep->GetTotalEnergyDeposit();
	G4double globTime   = aStep->GetTrack()->GetGlobalTime();
	
	// Get trackID and particle PDG
	G4int trackID = aStep->GetTrack()->GetTrackID();
	G4int partPDG = aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
	
	if (fabs(aStep->GetTrack()->GetDefinition()->GetPDGCharge()) < 0.01) return true;
  
	// Find out current sensor and sensor in following step (if zero, posStep is in passive material),
	// sensors supposed to be numbered from 1!!!
	
	// Initialize the codificators
	EncoderSi theEncoderPre;
	G4int codedPreStep = theEncoderPre.encode( aStep->GetPreStepPoint()->GetTouchable() );
	G4int iSensorPreStep =  preStep->GetPhysicalVolume()->GetCopyNo();

	EncoderSi theEncoderPost;
	G4int codedPosStep = theEncoderPost.encode( aStep->GetPostStepPoint()->GetTouchable() );
	
	// Find out layer, side and ladder number
	G4int iLayer  = theEncoderPre.getlayer();
	G4int iSide   = theEncoderPre.getside();
	G4int iLadder = theEncoderPre.getmodule();
	
	// Report if this bug occurs
	if(posStep->GetPhysicalVolume() == 0) 
	{
		G4cout << "WARNING: TRKSD_FTD01::ProcessHits: PosStep - Physical volume pointer is null, skipping ..." 
			<< G4endl;
		return true;
	}
	
#ifdef DEBUGTRKSD
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	G4cout << "DEBUG:  Geant4 hit: "    << G4endl
    << "Layer: "           << iLayer                    << " "
    << "Side:  "           << iSide                     << " "
    << "Ladder: "          << iLadder                   << " "
    << "Sensor: "          << iSensorPreStep            << " "
    << "PreStepX: "        << preStepPos.getX()/mm      << " "
    << "PreStepY: "        << preStepPos.getY()/mm      << " "
    << "PreStepZ: "        << preStepPos.getZ()/mm      << " "
    << "PosStepX: "        << posStepPos.getX()/mm      << " "
    << "PosStepY: "        << posStepPos.getY()/mm      << " "
    << "PosStepZ: "        << posStepPos.getZ()/mm      << " "
		//                        << "PreMomentum: "     << preStepMomentum/GeV       << " "
		//                        << "PosMomentum: "     << posStepMomentum/GeV       << " "
		//                        << "DepEnergy: "       << depEnergy/keV             << " "
		//                        << "StepLength: "      << aStep->GetStepLength()/mm << " "
		//                        << "GlobTime: "        << globTime/ns               << " "
    << "TrackID: "         << trackID                   << " "
    << "PartID: "          << partPDG                   << G4endl;
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif
	
	//
	// First hit in active medium
	//
	if(_previousTrackID == -1) 
	{
		// Start with new collection hit
		CreateNewColHit(aStep, preStepPos, posStepPos, preStepMomentum, posStepMomentum,
				depEnergy, globTime, iLayer, iSide, iLadder, iSensorPreStep, trackID, partPDG);
		// Set layer number, ladder number, sensor number and trackID for future decisions
		_previousLayer   = iLayer;
		_previousLadder  = iLadder;
		_previousSensor  = iSensorPreStep;
		_previousTrackID = trackID;
	}
	//
	// Another hit in the same active medium - layer, ladder, sensor
	//
	else if( (_mergeSensHits) && (_previousLayer == iLayer) && 
			(_previousLadder == iLadder) && (_previousSensor == iSensorPreStep)) 
	{
		//
		// Created by the same particle
		if((_previousTrackID == trackID) && (!_previousLeft)) 
		{
			// Update collection hit and update number of hits that contributed
			UpdateColHit(posStepPos, posStepMomentum, depEnergy, globTime);
			
			// Particle leaving medium? - set for future decisions
			if(codedPreStep != codedPosStep)
			{
				_previousLeft = true;
			}
		}
		//
		// Not created by the same particle
		else 
		{
			// Previous hit was created by a particle that stopped in a material or left active volume,
			// so first save and then clear
			if(_nHits != 0) 
			{
				SaveColHit();
				ClearColHit();
			}
			
			// Start with new collection hit
			CreateNewColHit(aStep, preStepPos, posStepPos, preStepMomentum, posStepMomentum,
					depEnergy, globTime, iLayer, iSide, iLadder, iSensorPreStep, trackID, partPDG);
			
			// Set trackID for future decisions
			_previousTrackID = trackID;
			
			// Particle leaving medium? - set for future decisions
			if(codedPreStep != codedPosStep)
			{
				_previousLeft = true;
			}
			
		}
	}
	//
	// Another hit in different active medium - layer or merging of hits forbidden
	//
	else 
	{
		// Previous hit was created by a particle that stopped in a material or left active volume,
		// so first save and then clear
		if(_nHits != 0) 
		{
			SaveColHit();
			ClearColHit();
		}
		
		// Start with new collection hit
		CreateNewColHit(aStep, preStepPos, posStepPos, preStepMomentum, posStepMomentum,
				depEnergy, globTime, iLayer, iSide, iLadder, iSensorPreStep, trackID, partPDG);
		
		// Set layer number and trackID for future decisions
		_previousLayer   = iLayer;
		_previousLadder  = iLadder;
		_previousSensor  = iSensorPreStep;
		_previousTrackID = trackID;
		
		// Particle leaving medium? - set for future decisions
		if(codedPreStep!=codedPosStep)
		{
			_previousLeft = true;
		}
		
	} // Collection hit creation

	return true;
} // Method

//
// Method invoked at the end of each event
//
void TRKSD_FTD01::EndOfEvent(G4HCofThisEvent *)
{
//
// Save last collection hit, if some
//
   if (_nHits != 0) {
      SaveColHit();
      ClearColHit();
   }
}

//
// Method used to reload hits from Ascii file
//
void TRKSD_FTD01::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
	TRKFTDHit * newHit = new TRKFTDHit();

	while(newHit->Load(theSubDetectorEventHitsFileInput)) 
	{
		_aCollection->insert(newHit);
		newHit = new TRKFTDHit();
	}
	
	delete newHit;
}

//
// Method used to create a new collection hit that is calculated as an average of hits created by the same particle
//
void TRKSD_FTD01::CreateNewColHit(G4Step * step, G4ThreeVector preStepPos, G4ThreeVector posStepPos,
                              G4ThreeVector preStepMomentum, G4ThreeVector posStepMomentum,
                              G4double depEnergy, G4double globTime, G4int iLayer, G4int iSide, G4int iLadder,
                              G4int iSensor, G4int trackID, G4int partPDG)
{
   // All secondary particles must be saved! Thus, change the track information ...
   UserTrackInformation * theUserTrackInformation = (UserTrackInformation*) (step->GetTrack()->GetUserInformation());
   if(theUserTrackInformation) theUserTrackInformation->GetTheTrackSummary()->SetToBeSaved();

   //
   _preStepPos.setX(preStepPos.getX());
   _preStepPos.setY(preStepPos.getY());
   _preStepPos.setZ(preStepPos.getZ());

   _posStepPos.setX(posStepPos.getX());
   _posStepPos.setY(posStepPos.getY());
   _posStepPos.setZ(posStepPos.getZ());

   _preStepMomentum.setX(preStepMomentum.getX());
   _preStepMomentum.setY(preStepMomentum.getY());
   _preStepMomentum.setZ(preStepMomentum.getZ());

   _posStepMomentum.setX(posStepMomentum.getX());
   _posStepMomentum.setY(posStepMomentum.getY());
   _posStepMomentum.setZ(posStepMomentum.getZ());

   _depEnergy  = depEnergy;
   _globTime   = globTime;

   _iLayer  = iLayer;
   _iSide   = iSide;
   _iLadder = iLadder;
   _iSensor = iSensor;
   _trackID = trackID;
   _partPDG = partPDG;

   _previousLeft = false;

   _nHits      = 1;
}

//
// Method used to update collection hit, i.e. add information from another Geant4 hit
//
void TRKSD_FTD01::UpdateColHit(G4ThreeVector posStepPos, G4ThreeVector posStepMomentum, G4double depEnergy,
                           G4double globTime)
{

#ifdef DEBUGTRKSD
        //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    G4cout << " DEBUG:                      " << G4endl
    << "                     " << G4endl
    << ">> UpdateColHit  :   " << G4endl
    << "Layer: "               << _iLayer                << " "
    << "Side : "               << _iSide                 << " "
    << "Ladder: "              << _iLadder               << " "
    << "Sensor: "              << _iSensor               << " "
    << "PosStepX: "        << posStepPos.getX()/mm      << " "
    << "PosStepY: "        << posStepPos.getY()/mm      << " "
    << "PosStepZ: "        << posStepPos.getZ()/mm      << " "
    << "DepEnergy: "           << _depEnergy/keV         << " "
    << "StepLength: "          << _stepLength/mm         << " "
        //                           << "GlobTime: "            << _globTime/ns           << " "
    << "TrackID: "             << _trackID               << " "
    << "PartID: "              << _partPDG               << G4endl
    << "                     " << G4endl
    << "                     " << G4endl;
        //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif

    
    _posStepPos.setX(posStepPos.getX());
   _posStepPos.setY(posStepPos.getY());
   _posStepPos.setZ(posStepPos.getZ());

   _posStepMomentum.setX(posStepMomentum.getX());
   _posStepMomentum.setY(posStepMomentum.getY());
   _posStepMomentum.setZ(posStepMomentum.getZ());

   _depEnergy  += depEnergy;
   _globTime   += globTime;

   _nHits++;

}

//
// Method used to save collection hit as LCIO hit
//
void TRKSD_FTD01::SaveColHit()
{
	G4ThreeVector meanPosition = 0.5*(_posStepPos + _preStepPos);
	G4ThreeVector meanMomentum = (0.5*(_preStepMomentum + _posStepMomentum)).mag() * (_posStepPos -_preStepPos).unit();
	
	_stepLength = (_posStepPos - _preStepPos).mag();
	_globTime   = _globTime/_nHits;
	
	// Insert new collection hit into collection (iLayer was artificially increased to make
	// work the algorithm in sensitive detector, so decrease it again by one now
    // don't do it for sensor as these have to be from 1-4 in gear at the moment
  --_iLayer;
  --_iLadder;
    
	// Deposited energy must be above certain threshold, otherwise hit thrown away
	if( (_depEnergy>=_thresholdFTD) || ((!_mergeSensHits) && (_depEnergy>0)) || Control::TrackingPhysicsListELossOn == false )
  {
		_aCollection->insert(new TRKFTDHit(_iLayer, _iSide,_iLadder, _iSensor, _trackID, _partPDG, _globTime, 
					_depEnergy,_stepLength, meanPosition, meanMomentum) );

#ifdef DEBUGTRKSD
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		G4cout << " DEBUG:                      " << G4endl
                             << "                     " << G4endl
                             << ">> Collection hit:   " << G4endl
                             << "Layer: "               << _iLayer                << " "
                             << "Side : "               << _iSide                 << " "
                             << "Ladder: "              << _iLadder               << " "
                             << "Sensor: "              << _iSensor               << " "
                             << "MeanPointX: "          << meanPosition.getX()/mm << " "
                             << "MeanPointY: "          << meanPosition.getY()/mm << " "
                             << "MeanPointZ: "          << meanPosition.getZ()/mm << " "
                             << "MeanMomentum: "        << meanMomentum.mag()/GeV << " "
                             << "DepEnergy: "           << _depEnergy/keV         << " "
//                           << "StepLength: "          << _stepLength/mm         << " "
//                           << "GlobTime: "            << _globTime/ns           << " "
                             << "TrackID: "             << _trackID               << " "
                             << "PartID: "              << _partPDG               << G4endl
                             << "                     " << G4endl
                             << "                     " << G4endl;
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif
	}
#ifdef DEBUGTRKSD
	else 
	{
		G4cout << " DEBUG: TRKSD_FTD01::SaveColHit: Particle energy below threshold!" << G4endl;
	}
#endif
}

void TRKSD_FTD01::ClearColHit() 
{
	_preStepPos.setX(0.);
	_preStepPos.setY(0.);
	_preStepPos.setZ(0.);
	
	_posStepPos.setX(0.);
	_posStepPos.setY(0.);
	_posStepPos.setZ(0.);
	
	_preStepMomentum.setX(0.);
	_preStepMomentum.setY(0.);
	_preStepMomentum.setZ(0.);
	
	_posStepMomentum.setX(0.);
	_posStepMomentum.setY(0.);
	_posStepMomentum.setZ(0.);
	
	_stepLength = 0.;
	_depEnergy  = 0.;
	_globTime   = 0.;
	
	_nHits = 0;
}
