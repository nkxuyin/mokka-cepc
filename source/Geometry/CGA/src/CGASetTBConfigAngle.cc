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
// $Id: CGASetTBConfigAngle.cc,v 1.1 2004/11/04 16:23:22 musat Exp $
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

#include "Control.hh"
#include "CGADefs.h"

extern "C" {
	void cgasettbconfigangle_(float * angle);
}

void CGASetTBConfigAngle(float angle) {

	cgasettbconfigangle_(&angle);
}

void cgasettbconfigangle_(float * angle) {

	Control::ConfigAngle = (double)(*angle);	
}
