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
// $Id: CGAGetVolumeData.cc,v 1.2 2006/04/12 12:55:48 musat Exp $
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
#include "G4ThreeVector.hh"
#include "CGADefs.h"

#include <vector>
#include <string>

void endString(char *str, int sLen);
void fillString(char *str, int sLen);

extern "C" {
	void cgagetvolumedata_(char * nomVolume, 
		double * parcours, double *preSteps, double * nbX0, 
		double *nInterLen, int & nSteps, 
		bool & OKFlag, int nomVolLen);
}

bool checkVolume(G4String nomVol,std::vector<G4String> logicals) {

        for (G4int ii = 0; ii < (G4int)logicals.size(); ii++) {
		if(nomVol == logicals[ii])
             		return true;
	}
	return false;
}

void CGAGetVolumeData(char * nomVol, double * parcours, 
		double **preSteps, double * nbX0, double *nInterLen,
		int * nSteps, int * OKFlag) {

	int noLayer=0, noModule=0;

	for(G4int i=0; i<*nSteps; i++) {
		parcours[i] = 0;
		nbX0[i] = 0;
		nInterLen[i] = 0;
	}
	for(CGASteppingAction::iterator copie = CGASteppingAction::begin();
		copie != CGASteppingAction::end(); ++copie) {

		if(noModule > *nSteps) {
			*OKFlag = -1;
			return;
		}

		if(checkVolume(G4String(nomVol), (*copie)->logicals)) {
			noLayer++; 
		}
		else {
			noLayer=0;
			continue;
		}

		if(noLayer==1) {
			preSteps[noModule][0]=
				(*copie)->preStepPoint.x();
			preSteps[noModule][1]=
				(*copie)->preStepPoint.y();
			preSteps[noModule][2]=
				(*copie)->preStepPoint.z();
			noModule++;
		}
		parcours[noModule-1] += (*copie)->parcours;
		nbX0[noModule-1] += (*copie)->nbX0;
		nInterLen[noModule-1] += (*copie)->nInterLen;
	}
	*nSteps=noModule;
}

void CGAGetVolumeDataForJava(string nomVol,std::vector<double *> &distance, 
		std::vector<double> &x,std::vector<double> &y,std::vector<double> &z,
		std::vector<double *> &nbX0, std::vector<double *> &nInterLen) {

	int noLayer=0;

	double * parcours;
	double * X0, *InterLen;

	for(CGASteppingAction::iterator copie = CGASteppingAction::begin();
		copie != CGASteppingAction::end(); ++copie) {

		if(checkVolume(G4String(nomVol.c_str()), (*copie)->logicals)) {
			noLayer++; 
		}
		else {
			noLayer=0;
			continue;
		}

		if(noLayer==1) {
			x.push_back((*copie)->preStepPoint.x());
			y.push_back((*copie)->preStepPoint.y());
			z.push_back((*copie)->preStepPoint.z());
			parcours = new double;
			X0 = new double;
			InterLen = new double;
			distance.push_back(parcours);
			nbX0.push_back(X0);		
			nInterLen.push_back(InterLen);		

			*parcours = 0;
			*X0 = 0;
			*InterLen = 0;
		}
		*parcours += (*copie)->parcours;
		*X0 += (*copie)->nbX0;
		*InterLen += (*copie)->nInterLen;
	}
}

void cgagetvolumedata_(char * nomVolume, 
		double * parcours, double *preSteps, double * nbX0, 
		double *nInterLen, int & nSteps, 
		bool & OKFlag, int nomVolLen) {

	int noLayer=0, noModule=0;
	char nomVol[2048];

	strncpy(nomVol, nomVolume, nomVolLen);
	endString(nomVol, nomVolLen+1);

	for(G4int i=0; i<nSteps; i++) {
		parcours[i] = 0;
		nbX0[i] = 0;
		nInterLen[i] = 0;
	}
	for(CGASteppingAction::iterator copie = CGASteppingAction::begin();
		copie != CGASteppingAction::end(); ++copie) {

		if(noModule > nSteps) {
			OKFlag = false;
			return;
		}

		if(checkVolume(G4String(nomVol), (*copie)->logicals)) {
			noLayer++; 
		}
		else {
			noLayer=0;
			continue;
		}

		if(noLayer==1) {
			*(preSteps+         noModule)=
				(*copie)->preStepPoint.x();
			*(preSteps+  nSteps+noModule)=
				(*copie)->preStepPoint.y();
			*(preSteps+2*nSteps+noModule)=
				(*copie)->preStepPoint.z();
			noModule++;
		}
		parcours[noModule-1] += (*copie)->parcours;
		nbX0[noModule-1] += (*copie)->nbX0;
		nInterLen[noModule-1] += (*copie)->nInterLen;
	}
	nSteps=noModule;
}

