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
//*    Mokka home page.                                 *
//*                                                     *
//*******************************************************
//
// $Id: CGASteppingAction.cc,v 1.5 2008/10/15 09:16:14 musat Exp $
// $Name: mokka-07-00 $
//
// History
// first implementation for the 
// Mokka Common Geometry Access (CGA) by 
// Gabriel Musat (musat@poly.in2p3.fr), July 2002
//
// see CGA documentation at 
// http://polype.in2p3.fr/geant4/tesla/www/mokka/
//        software/doc/CGADoc/CGAIndex.html
//-------------------------------------------------------


#include "CGASteppingAction.hh"
#include "SteppingAction.hh"
#include "SteppingActionMessenger.hh"

#include "G4SteppingManager.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "G4Trajectory.hh"

#include "G4TransportationManager.hh"
#include "G4VPhysicalVolume.hh"
#include "G4AffineTransform.hh"

#include "Control.hh"

#include <string.h>

G4ThreeVector CGASteppingAction::finalPoint=G4ThreeVector(0, 0, 0);
G4ThreeVector CGASteppingAction::startPoint=G4ThreeVector(0, 0, 0);
G4String CGASteppingAction::finalLVName="";
std::vector<CGAStep*> CGASteppingAction::volData;
G4double CGASteppingAction::maxDist = 0.;

CGASteppingAction::iterator CGASteppingAction::begin(void) { 
	return volData.begin();
}

CGASteppingAction::iterator CGASteppingAction::end(void) { 
	return volData.end();
}

void CGASteppingAction::resetCGAVector(void) {
	
	for(unsigned int i=0; i<volData.size(); i++)
		delete volData[i];

	volData.clear();
}

CGASteppingAction::CGASteppingAction(void)
{
}

void CGASteppingAction::UserSteppingAction(const G4Step* aStep)
{

  G4String nomVol, nomMat;
  CGAStep * elem;
  G4double parcours, nbX0, nbIntLen;
  //G4AffineTransform curTransform;
  G4ThreeVector localFinal;
  //G4int depth;

//  SteppingAction::UserSteppingAction(aStep);

  nomVol=aStep->GetPreStepPoint()->GetPhysicalVolume()->GetLogicalVolume()
		->GetName();
  nomMat=aStep->GetPreStepPoint()->GetPhysicalVolume()->GetLogicalVolume()
		->GetMaterial()->GetName();

  parcours = aStep->GetStepLength();

  const G4NavigationHistory *history =
	aStep->GetPreStepPoint()->GetTouchable()->GetHistory();

  G4ThreeVector diff = 
	(G4ThreeVector)(aStep->GetPostStepPoint()->GetPosition()) -
		startPoint;
  G4double currentDist = diff.mag();

  if((currentDist >= maxDist)) {
		fpSteppingManager->GetTrack()->SetTrackStatus(fStopAndKill);
		parcours = ((G4ThreeVector)(finalPoint - 
			      (G4ThreeVector)(aStep->GetPreStepPoint()->
				GetPosition()))).mag();
  }
/*
  if((nomVol == finalLVName)) {
  	for(depth=0; depth <= history->GetDepth(); depth++)
		if(nomVol == history->GetVolume(depth)->
			GetLogicalVolume()->GetName())
			break;
  
	curTransform = history->GetTransform(depth);	
	localFinal = curTransform.TransformPoint(finalPoint);
	EInside res = history->GetVolume(depth)->GetLogicalVolume()->
		GetSolid()->Inside(localFinal);
	if(res != kOutside) {
		G4ThreeVector direction = localFinal - 
			(G4ThreeVector)(curTransform.TransformPoint(
			aStep->GetPreStepPoint()->GetPosition()));

		G4double d1 = direction.mag();
		direction /= d1;
		G4double d2 = history->GetVolume(depth)->GetLogicalVolume()->
			GetSolid()->DistanceToOut(localFinal, direction);
		parcours *= (d1/(d1+d2));

		fpSteppingManager->GetTrack()->SetTrackStatus(fStopAndKill);
	}
  }
*/
  nbX0 = parcours / (aStep->GetPreStepPoint()->GetMaterial()->GetRadlen());
  nbIntLen = parcours / (aStep->GetPreStepPoint()->GetMaterial()
		->GetNuclearInterLength());
  
  elem = new CGAStep;
  elem->nomVol=nomVol;
  elem->nomMat=nomMat;
  elem->parcours=parcours;
  elem->parcours=parcours;
  elem->nbX0=nbX0;
  elem->nInterLen=nbIntLen;
  elem->preStepPoint=aStep->GetPreStepPoint()->GetPosition();

  for (G4int ii = 0; ii <= history->GetDepth(); ii++)
  	elem->logicals.push_back(history->GetVolume(ii)
		->GetLogicalVolume()->GetName());

  volData.push_back(elem);
}

void CGASteppingAction::setEndPoints(G4double xi, G4double yi, G4double zi,
	G4double xf, G4double yf, G4double zf) {
	startPoint=G4ThreeVector(xi*cm, yi*cm, zi*cm);
	finalPoint=G4ThreeVector(xf*cm, yf*cm, zf*cm);
	G4ThreeVector diff = finalPoint - startPoint;
        maxDist = diff.mag();
/*
  	G4VPhysicalVolume * physVol = 
		G4TransportationManager::GetTransportationManager()->
		GetNavigatorForTracking()->
		LocateGlobalPointAndSetup(finalPoint, 0, false);
	if(physVol) {
  		finalLVName=physVol->GetLogicalVolume()->GetName();
	}
	else {
		G4cout << "Le point final est exterieur au World !" <<
		G4endl << "Arret du tracking aux frontieres du World." 
			<< G4endl;
		finalLVName="Out of World";
	}
*/
}
