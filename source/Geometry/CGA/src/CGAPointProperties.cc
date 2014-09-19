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
// $Id: CGAPointProperties.cc,v 1.5 2006/05/03 15:38:08 musat Exp $
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
#include "G4Material.hh"
#include "G4TransportationManager.hh"
#include "G4Navigator.hh"
#include "G4Region.hh"
#include "G4AffineTransform.hh"
#include <string.h>
#include <string>
#include <vector>
#include "CGADefs.h"
#include "CGAGeometryManager.hh"
#include <math.h>

void endString(char *str, int sLen);
void fillString(char *str, int sLen);

extern "C" {
	void cgagetmaterialname_(double pos[], char *nomMat, int nomMatLen);
	void cgagetdensity_(double pos[], double &density);
	void cgagettemperature_(double pos[], double &temperature);
	void cgagetpressure_(double pos[], double &pressure);
	void cgagetradlen_(double pos[], double &radlen);
	void cgagetintlen_(double pos[], double &intlen);
	void cgagetlistoflvs_(double pos[], char *volName, int &nsteps, 
					int volNameLen);
	void cgagetlistofpvs_(double pos[], char *volName, int &nsteps, 
					int volNameLen);
	void cgagetregionname_(double pos[], char *nomReg, int nomRegLen);
	void cgagetlocalposition_(double global[3], double *local);
	void cgaistracker_(double * position, bool &isTracker);
	void cgaiscalorimeter_(double * position, bool &isCalorimeter);
}

std::string CGAGetMaterialName(double pos[]) {

	G4ThreeVector point = G4ThreeVector(pos[0]*mm, pos[1]*mm, pos[2]*mm);

	std::string matName;

	G4VPhysicalVolume * physVol =
                G4TransportationManager::GetTransportationManager()->
                GetNavigatorForTracking()->
                LocateGlobalPointAndSetup(point, 0, false);

	if(physVol) {
		matName = physVol->GetLogicalVolume()->GetMaterial()->
			GetName();
	} else
		matName = "Out of World";

	return matName;
}

double CGAGetDensity(double pos[]) {

	G4ThreeVector point = G4ThreeVector(pos[0]*mm, pos[1]*mm, pos[2]*mm);

	std::string matName;

	G4VPhysicalVolume * physVol =
                G4TransportationManager::GetTransportationManager()->
                GetNavigatorForTracking()->
                LocateGlobalPointAndSetup(point, 0, false);

	if(physVol) {
		return physVol->GetLogicalVolume()->GetMaterial()->
			GetDensity() *cm3/g;
	} else
		return 0;
}

double CGAGetTemperature(double pos[]) {

	G4ThreeVector point = G4ThreeVector(pos[0]*mm, pos[1]*mm, pos[2]*mm);

	std::string matName;

	G4VPhysicalVolume * physVol =
                G4TransportationManager::GetTransportationManager()->
                GetNavigatorForTracking()->
                LocateGlobalPointAndSetup(point, 0, false);

	if(physVol) {
		return physVol->GetLogicalVolume()->GetMaterial()->
			GetTemperature();
	} else
		return 0;
}

double CGAGetPressure(double pos[]) {

	G4ThreeVector point = G4ThreeVector(pos[0]*mm, pos[1]*mm, pos[2]*mm);

	std::string matName;

	G4VPhysicalVolume * physVol =
                G4TransportationManager::GetTransportationManager()->
                GetNavigatorForTracking()->
                LocateGlobalPointAndSetup(point, 0, false);

	if(physVol) {
		return physVol->GetLogicalVolume()->GetMaterial()->
			GetPressure() / bar;
	} else
		return 0;
}

double CGAGetRadLen(double pos[]) {

	G4ThreeVector point = G4ThreeVector(pos[0]*mm, pos[1]*mm, pos[2]*mm);

	std::string matName;

	G4VPhysicalVolume * physVol =
                G4TransportationManager::GetTransportationManager()->
                GetNavigatorForTracking()->
                LocateGlobalPointAndSetup(point, 0, false);

	if(physVol) {
		return physVol->GetLogicalVolume()->GetMaterial()->
			GetRadlen();
	} else
		return 0;
}

double CGAGetIntLen(double pos[]) {

	G4ThreeVector point = G4ThreeVector(pos[0]*mm, pos[1]*mm, pos[2]*mm);

	std::string matName;

	G4VPhysicalVolume * physVol =
                G4TransportationManager::GetTransportationManager()->
                GetNavigatorForTracking()->
                LocateGlobalPointAndSetup(point, 0, false);

	if(physVol) {
		return physVol->GetLogicalVolume()->GetMaterial()->
			GetNuclearInterLength();
	} else
		return 0;
}

void cgagetdensity_(double pos[], double &density) {
	density = CGAGetDensity(pos);
}

void cgagettemperature_(double pos[], double &temperature) {
	temperature = CGAGetTemperature(pos);
}

void cgagetpressure_(double pos[], double &pressure) {
	pressure = CGAGetPressure(pos);	
}

void cgagetradlen_(double pos[], double &radlen) {
	radlen = CGAGetRadLen(pos);
}

void cgagetintlen_(double pos[], double &intlen) {
	intlen = CGAGetIntLen(pos);
}

void cgagetmaterialname_(double pos[], char *nomMat, int nomMatLen) {

	std::string matName = CGAGetMaterialName(pos);

	strncpy(nomMat, matName.c_str(), nomMatLen);
	
	fillString(nomMat, nomMatLen);
}

std::vector<std::string> CGAGetListOfLogicalVolumes(double position[3]) {

  G4ThreeVector point(position[0]*mm, position[1]*mm, position[2]*mm);
  G4ThreeVector direction(0, 0, 1);

  G4TouchableHistory * theTouchable = new G4TouchableHistory();
  G4TransportationManager::GetTransportationManager()->
        GetNavigatorForTracking()->
        LocateGlobalPointAndUpdateTouchable(point, direction,
                theTouchable);
                                                                                
  G4int depth = theTouchable->GetHistory()->GetDepth();

  std::vector<std::string> listOfLVs;
  for(G4int i=depth; i >= 0; i--) {
	listOfLVs.push_back(theTouchable->GetHistory()->GetVolume(i)->
		GetLogicalVolume()->GetName());
  }

  return listOfLVs;
}

std::vector<std::string> CGAGetListOfPhysicalVolumes(double position[3]) {

  G4ThreeVector point(position[0]*mm, position[1]*mm, position[2]*mm);
  G4ThreeVector direction(0, 0, 1);

  G4TouchableHistory * theTouchable = new G4TouchableHistory();
  G4TransportationManager::GetTransportationManager()->
        GetNavigatorForTracking()->
        LocateGlobalPointAndUpdateTouchable(point, direction,
                theTouchable);
                                                                                
  G4int depth = theTouchable->GetHistory()->GetDepth();

  std::vector<std::string> listOfPVs;
  for(G4int i=depth; i >= 0; i--) {
	listOfPVs.push_back(theTouchable->GetHistory()->GetVolume(i)->
		GetName());
  }

  return listOfPVs;
}

void cgagetlistoflvs_(double pos[], char *volName, int &nsteps, int volNameLen) {

  std::vector<std::string> listOfLVs = CGAGetListOfLogicalVolumes(pos);

  for(unsigned int i=0; i < listOfLVs.size(); i++) {

	strncpy(volName + i*volNameLen, listOfLVs[i].data(), volNameLen);
	fillString(volName + i*volNameLen, volNameLen);
                                                                                
  }
 
  nsteps=(int)(listOfLVs.size());
}

void cgagetlistofpvs_(double pos[], char *volName, int &nsteps, int volNameLen) {

  std::vector<std::string> listOfPVs = CGAGetListOfPhysicalVolumes(pos);

  for(unsigned int i=0; i < listOfPVs.size(); i++) {

	strncpy(volName + i*volNameLen, listOfPVs[i].data(), volNameLen);
	fillString(volName + i*volNameLen, volNameLen);
                                                                                
  }

  nsteps=(int)(listOfPVs.size());
}

std::string CGAGetRegionName(double position[3]) {

  G4ThreeVector point(position[0]*mm, position[1]*mm, position[2]*mm);
  G4ThreeVector direction(0, 0, 1);

  G4TouchableHistory * theTouchable = new G4TouchableHistory();
  G4TransportationManager::GetTransportationManager()->
        GetNavigatorForTracking()->
        LocateGlobalPointAndUpdateTouchable(point, direction,
                theTouchable);
                                                                                
  G4int depth = theTouchable->GetHistory()->GetDepth();

	return theTouchable->GetHistory()->GetVolume(depth)->GetLogicalVolume()
		->GetRegion()->GetName();
}

void cgagetregionname_(double pos[], char *nomReg, int nomRegLen) {

	std::string regName = CGAGetRegionName(pos);

	strncpy(nomReg, regName.data(), nomRegLen);
	fillString(nomReg, nomRegLen);
}

std::vector<double> CGAGetLocalPosition(double position[3]) {

  G4ThreeVector point(position[0]*mm, position[1]*mm, position[2]*mm);
  G4ThreeVector direction(0, 0, 1);

  G4TouchableHistory * theTouchable = new G4TouchableHistory();
  G4TransportationManager::GetTransportationManager()->
        GetNavigatorForTracking()->
        LocateGlobalPointAndUpdateTouchable(point, direction,
                theTouchable);
                                                                                
  G4int depth = theTouchable->GetHistory()->GetDepth();

  G4AffineTransform theAffineTransform= theTouchable->GetHistory()
          ->GetTransform(depth);
                                                                                
  G4ThreeVector theLocalPosition =
                  theAffineTransform.TransformPoint(point);
                                                                                
  std::vector<double> returnValues;

  returnValues.push_back(theLocalPosition.getX());
  returnValues.push_back(theLocalPosition.getY());
  returnValues.push_back(theLocalPosition.getZ());

  return returnValues;
}

void cgagetlocalposition_(double global[3], double *local) {

	std::vector<double> localPos = CGAGetLocalPosition(global);

	local[0] = localPos[0];
	local[1] = localPos[1];
	local[2] = localPos[2];
}

bool CGAIsTracker(double position[3]) {

	CGAGeometryManager * theGeometryManager = 
		CGAGeometryManager::GetCGAGeometryManager();

	G4double trkRMax = theGeometryManager->GetTrackerRegionRmax();
	G4double trkZMax = theGeometryManager->GetTrackerRegionZmax();

	G4double r = sqrt(position[0]*position[0] + 
				position[1]*position[1]);
	G4double z = fabs(position[2]);

	return ((r <= trkRMax) && ( z <= trkZMax));
}

bool CGAIsCalorimeter(double position[3]) {

	CGAGeometryManager * theGeometryManager = 
		CGAGeometryManager::GetCGAGeometryManager();

	G4double trkRMax = theGeometryManager->GetTrackerRegionRmax();
	G4double trkZMax = theGeometryManager->GetTrackerRegionZmax();
	G4double caloRMax = theGeometryManager->GetCalorimeterRegionRmax();
	G4double caloZMax = theGeometryManager->GetCalorimeterRegionZmax();

	G4double r = sqrt(position[0]*position[0] + 
				position[1]*position[1]);
	G4double z = fabs(position[2]);

	return (((r <= caloRMax) && (r > trkRMax)) || 
		((z <= caloZMax) && ( z > trkZMax)));
}

void cgaistracker_(double * position, bool &isTracker) {
	
	isTracker = CGAIsTracker(position);
}

void cgaiscalorimeter_(double * position, bool &isCalorimeter) {
	
	isCalorimeter = CGAIsCalorimeter(position);
}
