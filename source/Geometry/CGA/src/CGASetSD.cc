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
// $Id: CGASetSD.cc,v 1.3 2006/03/01 14:13:30 musat Exp $
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

#include "VSubDetectorDriver.hh"
#include "VSensitiveDetector.hh"
#include "Control.hh"
#include "G4String.hh"
#include "G4Types.hh"
#include "G4ThreeVector.hh"
#include "CGADefs.h"

#include <iostream>
#include <map>
#include <vector>
#include <string>


VSensitiveDetector * currentSD = NULL;

extern "C" {
	void cgasetsd_(int &flag);
}

void CGASetSD(int flag) {

	union uni_flag {
		int i;
		unsigned char ch[4];
	}index;

	index.i = flag;

	if((unsigned int)(index.ch[1]) >= Control::DETECTOR_DRIVERS.size()) {
		std::cout << "CGASetSD - wrong driver index: " <<
			(int)(index.ch[1]) << std::endl;
		return;
	}

	currentSD = Control::DETECTOR_DRIVERS[(int)(index.ch[1])]->
		getSD((int)(index.ch[0]));
}

void cgasetsd_(int &flag) {
	
	CGASetSD(flag);
}
