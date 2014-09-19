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
// $Id: CGAGetSteps.cc,v 1.2 2006/04/12 12:55:48 musat Exp $
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
#include "CGADefs.h"

#include <vector>
#include <string>

void endString(char *str, int sLen);
void fillString(char *str, int sLen);

extern "C" {
	void cgagetsteps_(char * nomVol, char *nomMat, double * parcours,
		double *preSteps, double * nbX0, double *nInterLen, 
		int & nSteps, bool & OKFlag, int nomVolLen, int nomMatLen);
}


void CGAGetSteps(char ** nomVol, char **nomMat, double * parcours,
		double **preSteps, double * nbX0, double *nInterLen,
		int * nSteps, int *OKFlag, int nomVolLen, int nomMatLen) {

	int i=0;

	for(CGASteppingAction::iterator copie = CGASteppingAction::begin();
		copie != CGASteppingAction::end(); ++copie) {

		if(i >= *nSteps) {
			*OKFlag = -1;
			return;
		}

/*
		if((*copie)->nomVol==
			G4TransportationManager::GetTransportationManager()->
        		GetNavigatorForTracking()->GetWorldVolume()->
			GetLogicalVolume()->GetName())
			continue;
*/

		strncpy(nomVol[i], (*copie)->nomVol.data(),
			nomVolLen);

		strncpy(nomMat[i], (*copie)->nomMat.data(),
			nomMatLen);

		endString(nomVol[i], nomVolLen);

		endString(nomMat[i], nomMatLen);

		parcours[i] = (*copie)->parcours;

		preSteps[i][0]=(*copie)->preStepPoint.x();
		preSteps[i][1]=(*copie)->preStepPoint.y();
		preSteps[i][2]=(*copie)->preStepPoint.z();

		nbX0[i] = (*copie)->nbX0;

		nInterLen[i] = (*copie)->nInterLen;

		++i;
	}
	*nSteps=i;
} 

void CGAGetStepsForJava(std::vector<std::string> &volNames,
	std::vector<std::string> &matNames, std::vector<double> & distance,
	std::vector<double>& x,std::vector<double>& y,std::vector<double>& z,
	std::vector<double>& nbX0, std::vector<double> &nInterLen) {

	for(CGASteppingAction::iterator copie = CGASteppingAction::begin();
		copie != CGASteppingAction::end(); ++copie) {

		volNames.push_back((*copie)->nomVol);
		matNames.push_back((*copie)->nomMat);

		distance.push_back((*copie)->parcours);

		x.push_back((*copie)->preStepPoint.x());
		y.push_back((*copie)->preStepPoint.y());
		z.push_back((*copie)->preStepPoint.z());

		nbX0.push_back((*copie)->nbX0);
		nInterLen.push_back((*copie)->nInterLen);
	}
} 

void cgagetsteps_(char * nomVol, char *nomMat, double * parcours,
		double *preSteps, double * nbX0, double *nInterLen, 
		int & nSteps, bool & OKFlag, int nomVolLen, int nomMatLen) {

	int i=0;

	for(CGASteppingAction::iterator copie = CGASteppingAction::begin();
		copie != CGASteppingAction::end(); ++copie) {

		if(i >= nSteps) {
			OKFlag = false;
			return;
		}

/*
		if((*copie)->nomVol==
			G4TransportationManager::GetTransportationManager()->
        		GetNavigatorForTracking()->GetWorldVolume()->
			GetLogicalVolume()->GetName())
			continue;
*/

		strncpy(nomVol + i*nomVolLen, (*copie)->nomVol.data(),
			nomVolLen);
		fillString(nomVol + i*nomVolLen, nomVolLen);

		strncpy(nomMat + i*nomMatLen, (*copie)->nomMat.data(),
			nomMatLen);
		fillString(nomMat + i*nomMatLen, nomMatLen);

		parcours[i] = (*copie)->parcours;

		*(preSteps +          i)=(*copie)->preStepPoint.x();
		*(preSteps +   nSteps+i)=(*copie)->preStepPoint.y();
		*(preSteps + 2*nSteps+i)=(*copie)->preStepPoint.z();

		nbX0[i] = (*copie)->nbX0;

		nInterLen[i] = (*copie)->nInterLen;

		++i;
	}
	nSteps=i;
}

