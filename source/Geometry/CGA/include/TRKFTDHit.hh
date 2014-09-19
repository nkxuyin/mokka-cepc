#ifndef TRKFTDHIT_HH
#define TRKFTDHIT_HH 1

#define MOKKA_SETMOMENTUM
#define MOKKA_SETMCPART

// Geant4 classes
#include "VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

//! Implementation of Mokka FTD microstrips hit 
//!
//! @author Z. Drasal, Charles University Prague (based on TRKHit)
//! @author J. Duarte Campderros, adaptation to FTD (Sept 30 2011
//! @version 01.00
//!

class TRKFTDHit : public VHit
{
	public:
		//!Constructor
		TRKFTDHit() :
#ifdef LCIO_MODE
			// Set LC collection type
			VHit( LCIO::SIMTRACKERHIT ),
#endif
			_iLayer(0), _iSide(0), _iLadder(0), _iSensor(0), _PID(0), _PDG(0), _Time(0.), _DepEnergy(0.),
			_stepLength(0.) 
			{
				
				_Pos.setX(0.);
				_Pos.setY(0.); 
				_Pos.setZ(0.);
				
				_Momentum.setX(0.); 
				_Momentum.setY(0.); 
				_Momentum.setZ(0.);
			}

		//!Constructor with initial values
		TRKFTDHit(G4int iLayer, G4int iSide, G4int iLadder, G4int iSensor, G4int PID, G4int PDG,
				G4double Time, G4double DepEnergy, G4double stepLength, G4ThreeVector Pos,
				G4ThreeVector Momentum ) :
#ifdef LCIO_MODE
			// Set LC collection type
			VHit( LCIO::SIMTRACKERHIT ),
#endif
			_iLayer(iLayer), _iSide(iSide), _iLadder(iLadder), _iSensor(iSensor), _PID(PID), _PDG(PDG), 
			_Time(Time),_DepEnergy(DepEnergy), _stepLength(stepLength) 
			{
				_Pos.setX(Pos.getX()); 
				_Pos.setY(Pos.getY()); 
				_Pos.setZ(Pos.getZ());

				_Momentum.setX(Momentum.getX()); 
				_Momentum.setY(Momentum.getY()); 
				_Momentum.setZ(Momentum.getZ());
			}
		
		//!Copy constructor
		TRKFTDHit(const TRKFTDHit & right);
		
		//!Destructor
		~TRKFTDHit(){}
		
		//!Operator =
		const TRKFTDHit& operator=(const TRKFTDHit &right);
		//!Operator ==
		int operator==(const TRKFTDHit &right) const;
		//!Operator new
		inline void *operator new(size_t);
		//!Operator delete
		inline void operator delete(void *aTRKFTDHit);
		
		//!Draw hit
		void Draw();
		//!Print hit information
		void Print();
		
		G4bool Load(FILE *iFile);
		//!Save hit into ASCII file
		void Save(FILE *oFile);
		
#ifdef LCIO_MODE
		//!Save hit into LCIO file
		virtual void Save(LCCollectionVec* aLCCollectionVec);
#endif
		
	private:
		G4int _iLayer;            //!< Layer number
		G4int _iSide;            //!< Side: -1 // +1
		G4int _iLadder;           //!< Ladder number
		G4int _iSensor;           //!< Sensor number
		G4int _PID;               //!< Track ID
		G4int _PDG;               //!< Particle PDG
		G4ThreeVector _Pos;       //!< Hit mean position
		G4ThreeVector _Momentum;  //!< Hit mean momentum
		G4double      _Time;      //!< Hit mean time
		G4double      _DepEnergy; //!< Hit deposited energy
		G4double      _stepLength;//!< Step length
		
	public:
		//!Get number of a layer where was registered hit
		inline G4int GetLayerNumber() const {return _iLayer;}
		//!Get side of a layer where was registered hit
		inline G4int GetSideNumber() const {return _iSide;}
		//!Get number of a ladder where was registered hit
		inline G4int GetLadderNumber() const {return _iLadder;}
		//!Get number of a sensor that registered hit
		inline G4int GetSensorNumber() const {return _iSensor;}
		
		//!Get X position of Geant4 hit
		inline G4double GetX() const {return _Pos.getX();}
		//!Get Y position of Geant4 hit
		inline G4double GetY() const {return _Pos.getY();}
		//!Get Z position of Geant4 hit
		inline G4double GetZ() const {return _Pos.getZ();}
		
		//!Get X component of particle momentum
		inline G4double GetPx() const {return _Momentum.getX();}
		//!Get Y component of particle momentum
		inline G4double GetPy() const {return _Momentum.getY();}
		//!Get Z component of particle momentum
		inline G4double GetPz() const {return _Momentum.getZ();}
		
		//!Get time when a hit occured
		inline G4double GetTime() const {return _Time;}
		
		//!Get deposited energy in each step
		inline G4double GetEDep() const {return _DepEnergy;}
		
		//!Get step length
		inline G4double GetStepLength() const {return _stepLength;}
		
		//!Get primary particle ID
		inline G4int GetPID() const {return _PID;}
		//!Get current particle ID
		inline G4int GetPDG() const {return _PDG;}
};

typedef G4THitsCollection<TRKFTDHit> TRKFTDHitsCollection;

extern G4Allocator<TRKFTDHit> TRKFTDHitAllocator;

// Operator new
inline void* TRKFTDHit::operator new(size_t)
{
	void *aTRKFTDHit;
	aTRKFTDHit = (void *) TRKFTDHitAllocator.MallocSingle();
	return aTRKFTDHit;
}

// Operator delete
inline void TRKFTDHit::operator delete(void *aTRKFTDHit)
{
	TRKFTDHitAllocator.FreeSingle((TRKFTDHit*) aTRKFTDHit);
}

#endif // TRKFTDHIT_HH
