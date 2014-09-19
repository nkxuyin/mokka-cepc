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
// $Id: CGAFieldCalculations.cc,v 1.4 2006/05/10 13:04:54 musat Exp $
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
#include "G4String.hh"
#include "G4Types.hh"
#include "CGADefs.h"
#include "G4TransportationManager.hh"
#include "G4FieldManager.hh"
#include "G4MagneticField.hh"
#include "G4ElectricField.hh"

#include <vector>
#include <string>

void endString(char *str, int sLen);
void fillString(char *str, int sLen);

extern "C" {
	void cgagetbdl_(double start[], double end[], double &IBdl);
	void cgagetedl_(double start[], double end[], double &IBdl);
	void cgagetb_(double position[], double bfield[]);
	void cgagete_(double position[], double efield[]);
}

G4ThreeVector CGAGetFieldVector(const G4Field *field, const G4ThreeVector &position)
{
  G4double point[4] = { position.getX(), position.getY(), position.getZ(), 0 };
  G4double bField[3] = { 0, 0, 0 };

  G4double unit;

  if(dynamic_cast<const G4MagneticField*>(field))
		unit = tesla;

  else if(dynamic_cast<const G4ElectricField*>(field))
		unit = volt/meter;

  else {
		G4cout << "Unknown field" << G4endl;
		return G4ThreeVector(0.0, 0.0, 0.0);
	}

  field->GetFieldValue(point, bField);
  return G4ThreeVector(bField[0]/unit, bField[1]/unit, bField[2]/unit);
}

double CGAGetFieldIntegral(const G4Field *detectorField, 
		double start[3], double end[3]) {

	G4ThreeVector p1(start[0]*mm, start[1]*mm, start[2]*mm);
	G4ThreeVector p2(end[0]*mm, end[1]*mm, end[2]*mm);
	
	G4ThreeVector direction = p2 - p1;
	G4double distance = direction.mag();
	G4ThreeVector u = direction/distance;

	G4double step = (distance > 1*meter) ? 1*mm : distance/1000.;

	G4int nSteps = (G4int)(distance/step);

	G4double Ix = 0, Iy = 0, Iz = 0;
	//integration loop
	for(G4int i=0; i< nSteps-1; i++) {
		G4ThreeVector ri = p1 + u*i*step;
		G4ThreeVector rj = p1 + u*(i+1)*step;
		
		G4ThreeVector Bi = CGAGetFieldVector(detectorField, ri);
		G4ThreeVector Bj = CGAGetFieldVector(detectorField, rj);

		Ix += (Bi.getX() + Bj.getX())*step*u.getX()/2.;
		Iy += (Bi.getY() + Bj.getY())*step*u.getY()/2.;
		Iz += (Bi.getZ() + Bj.getZ())*step*u.getZ()/2.;

	}

	G4ThreeVector ri = p1 + u*(nSteps-1)*step;
	G4ThreeVector rj = p2;

	G4ThreeVector Bi = CGAGetFieldVector(detectorField, ri);
	G4ThreeVector Bj = CGAGetFieldVector(detectorField, rj);

	Ix += (Bi.getX() + Bj.getX())*(rj-ri).getX()/2.;
	Iy += (Bi.getY() + Bj.getY())*(rj-ri).getY()/2.;
	Iz += (Bi.getZ() + Bj.getZ())*(rj-ri).getZ()/2.;
	
	return Ix+Iy+Iz;
}

std::vector<double> CGAGetB(double position[3]) {

        const G4MagneticField *detectorField =
                dynamic_cast<const G4MagneticField*>(
                G4TransportationManager::GetTransportationManager()->
                        GetFieldManager()->GetDetectorField());
                                                                                
        if (!detectorField) {
                G4cout << "No magnetic field defined." << G4endl;
		std::vector<double> error; 
		error.push_back(0.0);
		error.push_back(0.0);
		error.push_back(0.0);
                return error;
        }

	G4ThreeVector pos(position[0]*mm, position[1]*mm, position[2]*mm);
	
	G4ThreeVector B = CGAGetFieldVector(detectorField, pos);

	std::vector<double> BField;
	BField.push_back(B.getX()); 
	BField.push_back(B.getY()); 
	BField.push_back(B.getZ());

	return BField;
}

std::vector<double> CGAGetE(double position[3]) {

        const G4ElectricField *detectorField =
                dynamic_cast<const G4ElectricField*>(
                G4TransportationManager::GetTransportationManager()->
                        GetFieldManager()->GetDetectorField());
                                                                                
        if (!detectorField) {
                G4cout << "No electric field defined." << G4endl;
		std::vector<double> error; 
		error.push_back(0.0);
		error.push_back(0.0);
		error.push_back(0.0);
                return error;
        }

	G4ThreeVector pos(position[0]*mm, position[1]*mm, position[2]*mm);
	
	G4ThreeVector E = CGAGetFieldVector(detectorField, pos);

	std::vector<double> EField;
	EField.push_back(E.getX()); 
	EField.push_back(E.getY()); 
	EField.push_back(E.getZ());

	return EField;
}

double CGAGetBdl(double start[3], double end[3]) {

	const G4MagneticField *detectorField = 
		dynamic_cast<const G4MagneticField*>(
		G4TransportationManager::GetTransportationManager()->
			GetFieldManager()->GetDetectorField());

	if (!detectorField) {
		G4cout << "No magnetic field defined." << G4endl;
		return 0;
	}

	return CGAGetFieldIntegral(detectorField, start, end);
}

double CGAGetEdl(double start[3], double end[3]) {

	const G4ElectricField *detectorField = 
		dynamic_cast<const G4ElectricField*>(
		G4TransportationManager::GetTransportationManager()->
			GetFieldManager()->GetDetectorField());

	if (!detectorField) {
		G4cout << "No electric field defined." << G4endl;
		return 0;
	}

	return CGAGetFieldIntegral(detectorField, start, end);
}

void cgagetbdl_(double start[], double end[], double &IBdl) {

	IBdl = CGAGetBdl(start, end);
}

void cgagetedl_(double start[], double end[], double &IEdl) {

	IEdl = CGAGetEdl(start, end);
}

void cgagetb_(double position[], double bfield[]) {
	
	std::vector<double> B = CGAGetB(position);
	bfield[0] = B[0];
	bfield[1] = B[1];
	bfield[2] = B[2];
}

void cgagete_(double position[], double efield[]) {
	
	std::vector<double> E = CGAGetE(position);
	efield[0] = E[0];
	efield[1] = E[1];
	efield[2] = E[2];
}
