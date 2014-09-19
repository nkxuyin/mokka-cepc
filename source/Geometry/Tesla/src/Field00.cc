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
//* Mokka home page.                                    *
//*                                                     *
//*******************************************************
//
// $Id: Field00.cc,v 1.5 2008/02/15 13:44:38 musat Exp $
// $Name: mokka-07-00 $
//
// 
//
// Field00.cc
//
// History:  
// - first implementation P. Mora de Freitas (apr 01)

#include "globals.hh"
#include "G4MagneticField.hh"
#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"

#include "Control.hh"
#include "Field00.hh"
#include "MySQLWrapper.hh"

#include "CGADefs.h"

#ifdef MOKKA_GEAR
#include "gearimpl/GearParametersImpl.h"
#include "MokkaGear.h"
#include "gearimpl/Vector3D.h"
#include "gearimpl/ConstantBField.h"
 
#endif


INSTANTIATE(Field00)

G4bool Field00::construct(const G4String &aSubDetectorName,
			 G4LogicalVolume*)
{
  G4cout << "\nBuilding Field..." << G4endl;
  char buffer[80];
  sprintf(buffer,"Field00: global magnetic field factor is %f",
	  Control::BFactor);
  Control::Log(buffer);
  Database* db = new Database(aSubDetectorName.data());
  
  db->exec("select * from field_map;");
  FieldMapSize = db->nrTuples();
  if(FieldMapSize == 0) 
	Control::Abort("Mokka abort: No field map for driver Field00!",
			MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
  FieldMap = new FieldRegion*  [FieldMapSize];
  G4int iElem=0;
  while(db->getTuple())
    {
      FieldRegion * aFieldRegion = new FieldRegion;
      // 
      // zmin and zmax assumed symetric
      aFieldRegion->zmin=db->fetchDouble("zmin");
      aFieldRegion->zmax=db->fetchDouble("zmax");
      //
      // rmin and rmax stored as squared!
      aFieldRegion->rmin=db->fetchDouble("rmin");
      aFieldRegion->rmin=aFieldRegion->rmin*aFieldRegion->rmin;
      aFieldRegion->rmax=db->fetchDouble("rmax");
      aFieldRegion->rmax = aFieldRegion->rmax*aFieldRegion->rmax;
      //
      // MagField assumed expressed in tesla unit.
      aFieldRegion->MagField[0]=db->fetchDouble("mag_field_x")*tesla*Control::BFactor;
      aFieldRegion->MagField[1]=db->fetchDouble("mag_field_y")*tesla*Control::BFactor;
      aFieldRegion->MagField[2]=db->fetchDouble("mag_field_z")*tesla*Control::BFactor;
      //FieldMap.push_back (aFieldRegion);
      FieldMap[iElem] = aFieldRegion;
      iElem++;
    }

  if(FieldMapSize != iElem) 
	Control::Abort("Mokka abort: error while reading DB field map for driver Field00!",
			MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

//  if(FieldMap.size()>0)
  if(FieldMapSize>0)
    {
      G4FieldManager* fieldMgr
	= G4TransportationManager::GetTransportationManager()
	->GetFieldManager();
      fieldMgr->SetDetectorField(this);
      fieldMgr->CreateChordFinder(this);
    }
  else
    Control::Abort("driver Field00: field region didn't find in the given database!",
			MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

#ifdef MOKKA_GEAR

  MokkaGear* gearMgr = MokkaGear::getMgr() ; 
  double pos[3]={0,0,0};
  double b_field[3];
  GetFieldValue(pos,b_field);
  gear::Vector3D b_vect(b_field[0]/ tesla,b_field[1]/tesla,b_field[2]/tesla);
  gear::ConstantBField* magfield = new gear::ConstantBField(b_vect);

  gearMgr->setBField(magfield);
#endif
  
  delete db;
  G4cout << "Field done.\n" << G4endl;
  return true;
}

void Field00::GetFieldValue(const double Point[3],
			    double* Bfield) const
{
  
  // defaut B=0
  Bfield[0] = 0.;
  Bfield[1] = 0.;
  Bfield[2] = 0.;
  
  // rmin and rmax are stored as squared!
  G4double R2 = Point[0]*Point[0]+Point[1]*Point[1];

  // zmin and zmax assumed symetric
  G4double Z = abs(Point[2]);
  
  for(int i_region=0;i_region<FieldMapSize;i_region++)
    if(FieldMap[i_region]->zmin <= Z &&
       FieldMap[i_region]->zmax >  Z  &&
       FieldMap[i_region]->rmin <= R2 &&
       FieldMap[i_region]->rmax >  R2)
      {
	Bfield[0] = (FieldMap[i_region]->MagField)[0];
	Bfield[1] = (FieldMap[i_region]->MagField)[1];
	Bfield[2] = (FieldMap[i_region]->MagField)[2];
	//
	// if Z<0, Bx and By are inversed 
	if(Point[2]<0)
	  {
	    Bfield[0] = -Bfield[0];
	    Bfield[1] = -Bfield[1];
	  }
	break;
      }
}
