#include "TRKFTDHit.hh"

// Basic C
#include <assert.h>

// Basic Mokka classes
#include "Control.hh"

// Geant4 classes
#include "G4VVisManager.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4Circle.hh"

// LCIO classes
#ifdef LCIO_MODE
#include <lcio.h>
#include <IMPL/LCFlagImpl.h>
#include <UTIL/CellIDEncoder.h>
#include <UTIL/ILDConf.h>
#endif

G4Allocator<TRKFTDHit> TRKFTDHitAllocator;

//
// Copy constructor
//
TRKFTDHit::TRKFTDHit(const TRKFTDHit &right)
#ifdef LCIO_MODE
: VHit( LCIO::SIMTRACKERHIT )
#else
: VHit()
#endif
{
	_iLayer    = right._iLayer;
	_iSide     = right._iSide;
	_iLadder   = right._iLadder;
	_iSensor   = right._iSensor;
	_PID       = right._PID;
	_PDG       = right._PDG;
	_Pos       = right._Pos;
	_Momentum  = right._Momentum;
	_Time      = right._Time;
	_DepEnergy = right._DepEnergy;
	_stepLength= right._stepLength;
}

//
// Operator =
//
const TRKFTDHit& TRKFTDHit::operator=(const TRKFTDHit &right)
{
#ifdef LCIO_MODE
	((VHit*)this)->operator=(right);
#endif
	_iLayer    = right._iLayer;
	_iSide     = right._iSide;
	_iLadder   = right._iLadder;
	_iSensor   = right._iSensor;
	_PID       = right._PID;
	_PDG       = right._PDG;
	_Pos       = right._Pos;
	_Momentum  = right._Momentum;
	_Time      = right._Time;
	_DepEnergy = right._DepEnergy;
	_stepLength= right._stepLength;
	return *this;
}

//
// Operator ==
//
int TRKFTDHit::operator==(const TRKFTDHit &right) const
{
	return ((_iLayer    == right._iLayer    ) && 
			(_iSide     == right._iSide     ) &&
			(_iLadder   == right._iLadder   ) &&
			(_iSensor   == right._iSensor   ) &&
			(_PID       == right._PID       ) &&
			(_PDG       == right._PDG       ) &&
			(_Pos       == right._Pos       ) &&
			(_Momentum  == right._Momentum  ) &&
			(_Time      == right._Time      ) &&
			(_DepEnergy == right._DepEnergy ) &&
			(_stepLength== right._stepLength));
}

//
// Draw a hit
//
void TRKFTDHit::Draw()
{
	G4VVisManager* vVisManager = G4VVisManager::GetConcreteInstance();
	if(vVisManager) 
	{
		G4Colour colour(((GetPID()%2)+1.)/2.,((GetPID()%3)+1.)/3.,1.);
		G4VisAttributes attribs(colour);
		G4Circle circle;
		
		circle.SetPosition(G4Point3D(_Pos.getX(),_Pos.getY(),_Pos.getZ()));
		circle.SetScreenDiameter (2.0);
		circle.SetFillStyle (G4Circle::filled);
		circle.SetVisAttributes(attribs);
		
		//      vVisManager->Draw(circle);
	}
}

//
// Print
//
void TRKFTDHit::Print() {}

//
// Save the hit into a text file
//
void TRKFTDHit::Save(FILE *oFile)
{
	if(oFile)
	{
		fprintf(oFile,"nL:%d hPos[mm]:%7.2f %7.2f %7.2f vecP[GeV]:%7.2f %7.2f %7.2f time[ns]:%7.2f PID:%d PDG:%d depE[keV]:%15e\n",
				_iLayer, _Pos.getX()/mm, _Pos.getY()/mm, _Pos.getZ()/mm,
				_Momentum.getX()/GeV, _Momentum.getY()/GeV, _Momentum.getZ()/GeV,
				_Time/ns, _PID, _PDG, _DepEnergy/keV);
	}
}

//
// Load the hit from a text file
//
G4bool TRKFTDHit::Load(FILE *iFile)
{
	double X,Y,Z, Px,Py,Pz, Time, EDep;
	int    iLayer, iSide, iLadder, iSensor, PID, PDG;
	G4bool readStatus = 0;
	
	readStatus = (fscanf(iFile,"%d %d %d %d %lf %lf %lf %lf %lf %lf %lf %d %d %lf",
				&iLayer, &iSide, &iLadder, &iSensor, &X,&Y,&Z, &Px,&Py,&Pz, &Time, &PID, &PDG, &EDep) == 13);
	
	return readStatus;
}

//
// Save the hit into a lcio file
//
#ifdef LCIO_MODE
void TRKFTDHit::Save(LCCollectionVec* aLCCollectionVec)
{
	// Set flag - barrel detectors-- NO
	LCFlagImpl aFlag(0);
	//aFlag.setBit( LCIO::THBIT_BARREL );
	//aLCCollectionVec->setFlag( aFlag.getFlag() );
	
	// Set flag - momentum and step length will be saved
#ifdef MOKKA_SETMOMENTUM
	aFlag.setBit( LCIO::THBIT_MOMENTUM );
	aLCCollectionVec->setFlag( aFlag.getFlag() );
#endif
	
	// Create cellID encoder
	//CellIDEncoder<SimTrackerHitImpl> cellIDEnc( "layer:7,ladder:5,sensor:4" ,aLCCollectionVec ) ;
	ILDCellIDEncoder<SimTrackerHitImpl> cellIDEnc( aLCCollectionVec );
	
	// Set hit position
	G4double hitPos[3];
	hitPos[0] = _Pos.getX()/mm;
	hitPos[1] = _Pos.getY()/mm;
	hitPos[2] = _Pos.getZ()/mm;
	
	// Create a new LCIO SimTracker hit
	SimTrackerHitImpl * hit = new SimTrackerHitImpl;
	hit->setEDep(GetEDep()/GeV);
	hit->setPosition(hitPos);
	hit->setTime(GetTime()/ns);
	hit->setMomentum(_Momentum.getX()/GeV,_Momentum.getY()/GeV,_Momentum.getZ()/GeV);
	hit->setPathLength(_stepLength/mm);
	
	cellIDEnc[ILDCellID0::subdet] = ILDDetID::FTD;  
	cellIDEnc[ILDCellID0::side]   = _iSide;
	cellIDEnc[ILDCellID0::layer]  = _iLayer;
	cellIDEnc[ILDCellID0::module] = _iLadder;
	cellIDEnc[ILDCellID0::sensor] = _iSensor;
	
	cellIDEnc.setCellID(hit);
	
	// Set particle that has interacted in the detector
#ifdef  MOKKA_SETMCPART
	hit->setMCParticle( dynamic_cast<MCParticle*>(Control::MCParticleMap[_PID]));
#else
	hit->setMCParticle(NULL);
#endif
	
	// Put the hit into LCIO collection
	aLCCollectionVec->push_back( hit );
}
#endif

