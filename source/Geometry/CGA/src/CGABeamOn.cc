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
// $Id: CGABeamOn.cc,v 1.1 2003/07/18 09:08:50 musat Exp $
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

#include <string.h>

#include "G4UImanager.hh"
#include "CGASteppingAction.hh"
#include "G4String.hh"
#include "G4Types.hh"
#include "G4TransportationManager.hh"
#include "CGADefs.h"

void endString(char *str, int sLen);
void fillString(char *str, int sLen);

extern "C" {
	void cgabeamon_(double initial[], double final[], double direction[],  
		char * particule, float & energie, 
		int &nbPart, int partLen);
}

void CGABeamOn(double *initial, double *final, double *direction,  
		char * particule, float energie, int nbPart) {

	cgabeamon_(initial, final, direction, particule, energie, 
		nbPart, strlen(particule));
}

void cgabeamon_(double initial[], double final[], double direction[],  
		char * Particule, float & energie, 
		int &nbPart, int partLen) {

	char particule[1024];

	G4UImanager * UImanager = G4UImanager::GetUIpointer();
	char startPosition[1024], directionStr[1024], ener[1024], nbP[1024];

	CGASteppingAction::resetCGAVector();
	CGASteppingAction::setEndPoints(initial[0], initial[1], initial[2],
			final[0], final[1], final[2]);

	sprintf(startPosition, "%f %f %f cm", 
		initial[0], initial[1], initial[2]);
	sprintf(directionStr, "%f %f %f", 
		direction[0], direction[1], direction[2]);
	sprintf(ener, "%f GeV", energie);
	sprintf(nbP, "%d", nbPart);
	strncpy(particule, Particule, partLen);
	endString(particule, partLen+1);


	UImanager->ApplyCommand(G4String("/generator/generator particleGun"));
	UImanager->ApplyCommand(G4String("/gun/position ") + startPosition);
	UImanager->ApplyCommand(G4String("/gun/direction ") + directionStr);
	UImanager->ApplyCommand(G4String("/gun/energy ") + ener);
	UImanager->ApplyCommand(G4String("/gun/particle ") + particule);
	UImanager->ApplyCommand(G4String("/run/beamOn ") + nbP);
}

