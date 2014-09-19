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
// $Id: CGAWhereAMI.cc,v 1.1 2003/07/18 09:08:56 musat Exp $
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

#include "G4UImanager.hh"
#include "CGASteppingAction.hh"
#include "G4String.hh"
#include "G4Types.hh"
#include "G4TransportationManager.hh"
#include <string.h>
#include "CGADefs.h"

void endString(char *str, int sLen);
void fillString(char *str, int sLen);

extern "C" {
	void cgawhereami_(double pos[], char *nomVol, int nomVolLen);
}

void CGAWhereAmI(double * pos, char *nomVol, int NOMVOLLEN) {

	cgawhereami_(pos, nomVol, NOMVOLLEN);

	endString(nomVol, NOMVOLLEN);
}

void cgawhereami_(double pos[], char *nomVol, int nomVolLen) {

	G4ThreeVector point = G4ThreeVector(pos[0]*cm, pos[1]*cm, pos[2]*cm);

	G4VPhysicalVolume * physVol =
                G4TransportationManager::GetTransportationManager()->
                GetNavigatorForTracking()->
                LocateGlobalPointAndSetup(point, 0, false);

	if(physVol) {
		strncpy(nomVol, physVol->GetLogicalVolume()->
			GetName().data(), nomVolLen);
	} else
		strncpy(nomVol, "Out of World", nomVolLen);

	fillString(nomVol, nomVolLen);
}

string CGAWhereAmIForJava(double pos[]) {

	G4ThreeVector point = G4ThreeVector(pos[0]*cm, pos[1]*cm, pos[2]*cm);

	G4VPhysicalVolume * physVol =
                G4TransportationManager::GetTransportationManager()->
                GetNavigatorForTracking()->
                LocateGlobalPointAndSetup(point, 0, false);

	if(physVol) {
		return physVol->GetLogicalVolume()->GetName();
	} else
		return "Out of World";
}
