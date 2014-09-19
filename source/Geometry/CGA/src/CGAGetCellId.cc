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
// $Id: CGAGetCellId.cc,v 1.4 2006/05/12 13:03:53 musat Exp $
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
#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4TransportationManager.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "CGADefs.h"

#include <iostream>
#include <vector>
#include <string>



extern "C" {
	void cgagetcellid_(double& x, double& y, double& z, int & cellID0,
			int & cellID1, int& flag,
			double& xDir, double& yDir, double& zDir);
}

cell_ids CGAGetCellId(double x, double y, double z, int &flag,
		double xDir, double yDir, double zDir) {

	cell_ids result, error;
	error.id0 = -99999;
	error.id1 = -99999;

	G4ThreeVector point = G4ThreeVector(x*mm, y*mm, z*mm);

	VSensitiveDetector * theSD = 0;

        G4VPhysicalVolume * physVol =
                G4TransportationManager::GetTransportationManager()->
                GetNavigatorForTracking()->
                LocateGlobalPointAndSetup(point, 0, false);

        if(physVol)
                theSD = dynamic_cast<VSensitiveDetector*>(
			physVol->GetLogicalVolume()->
				GetSensitiveDetector()	 );
	if(!theSD) {

		return error;
	}

	if((xDir == 0) && (yDir == 0) && (zDir == 0)) {
		std::cout << "CGAGetCellId: setting default direction to "
			<< " (0, 0, 1)" << std::endl;
		zDir = 1;
	}

       	result = theSD->GetCellIndex(x, y, z, flag, xDir, yDir, zDir);

	return result;

}

void cgagetcellid_(double& x, double& y, double& z, int & cellID0, 
			int & cellID1, int& flag,
			double& xDir, double& yDir, double& zDir) {


       cell_ids result = CGAGetCellId(x, y, z, flag, xDir, yDir, zDir);
       cellID0 = result.id0;
       cellID1 = result.id1;

}
