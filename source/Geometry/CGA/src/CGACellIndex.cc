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
// $Id: CGACellIndex.cc,v 1.6 2006/03/01 14:13:30 musat Exp $
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

#include "VSensitiveDetector.hh"
#include "CalHit.hh"
#include "Control.hh"
#include "G4String.hh"
#include "G4Types.hh"
#include "G4ThreeVector.hh"
#include "CGADefs.h"

#include <iostream>
#include <vector>
#include <string>

extern VSensitiveDetector * currentSD;


extern "C" {
	void cgacellindex_(int &cellID0, int & cellID1,
	       double &x, double &y, double &z, int &grzone);
}

cell_info CGACellIndex(cell_ids cellID) {

	cell_info center, error;

	if(currentSD == NULL) {
		error.X = -99999.;
		error.Y = -99999.;
		error.Z = -99999.;
		error.GRZone = -99999;

		std::cout <<"CGACellIndex: please call CGASetSD first!" <<
			std::endl;

		return error;
	}

	G4ThreeVector result;

	CalHit * theHit = currentSD->getEncoder()->decode(cellID);

	result = currentSD->GetCellCenter(0,
				theHit->GetS(),
				theHit->GetM(),
				theHit->GetI(),
				theHit->GetJ(),
				theHit->GetK()           );


	center.X = result.x();
	center.Y = result.y();
	center.Z = result.z();
	center.GRZone = theHit->GetGRZone();

	return center;

}

void cgacellindex_(int &cellID0, int & cellID1,
	       double &x, double &y, double &z, int &grzone) {

	cell_ids index;
	index.id0 = cellID0;
	index.id1 = cellID1;

	cell_info center = 
		CGACellIndex(index);

	x = center.X;
	y = center.Y;
	z = center.Z;
	grzone = center.GRZone;
}
