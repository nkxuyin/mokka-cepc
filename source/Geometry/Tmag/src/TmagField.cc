// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: TmagField.cc,v 1.1 2007/02/09 15:57:48 predrag Exp $
// $Name: mokka-07-00 $
//
// History:
// - first implementation P. Krstonosic  2006-08-01
//   field map calculated on the basis of adjustment to experimetal I->Bmax curve (~750A->4T) 

#include "TmagField.hh"
#include "MySQLWrapper.hh"
#include "CGAGeometryEnvironment.hh"
#include "Control.hh"
#include "CGADefs.h"
#include "globals.hh"
#include "G4MagneticField.hh"
#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"
#include "G4ThreeVector.hh"
#include <iostream>
#include <sstream>
#include <fstream>

INSTANTIATE(TmagField)

G4bool  TmagField::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog)
{
  worldLog = 0; // suppresses a compiler warning, worldLog is not needed
  
 
         Database *db = new Database(env.GetDBName());
         
	   db->exec("SELECT * FROM `FieldMap`;");
	
	   for(unsigned int i=0;i<95;i++)
	     for(unsigned int j=0;j<60;j++)
	       { db->getTuple();
		 maparr[j][i][0]=db->fetchDouble("radius") ;  // radius 
		 maparr[j][i][1]=db->fetchDouble("z");  // z 
		 maparr[j][i][2]=db->fetchDouble("Br") ;  // Br 
		 maparr[j][i][3]=db->fetchDouble("Bz") ;  // Bz

	       }
	     


    

  G4FieldManager *fieldMgr = G4TransportationManager::GetTransportationManager()->GetFieldManager();
  fieldMgr->SetDetectorField(this);
  fieldMgr->CreateChordFinder(this);
  delete db;
  return true;
}

void TmagField::GetFieldValue(const double point[4], double *bField) const
{
  bField[0] = 0.0*tesla;
  bField[1] = 0.0*tesla;
  bField[2] = 0.0*tesla;
  G4double radius=sqrt(point[1]*point[1]+point[0]*point[0]);

  if( fabs(point[2])<950.0*mm && radius < 595.0*mm ) 
    {
    //double radius=sqrt(point[1]*point[1]+point[0]*point[0]);
  G4double angle=atan2(point[1],point[0]);
  if (angle<0.) angle=angle+3.14159265*2.0;
      
  int binr=(int) fabs(radius+5.0*mm)/10;
  int binz=(int) fabs(point[2])/10;

  
    
  G4double pointz=fabs(point[2])*mm;

    
 //  G4double dz=fabs(maparr[binr][binz+1][1]-maparr[binr][binz][1])*mm;  
//   G4double dr=fabs(maparr[binr+1][binz][0]-maparr[binr][binz][0])*mm;

  G4double br=0.0*tesla;
  G4double bz=0.0*tesla;

  if(binr==0 || binz==0  )
    {
        if( binr==0  &&  binz!=0   )
	 {
	  br=0.0*tesla;
	  if( binz<94)
	  bz=( maparr[binr][binz][3]+ 
	       (maparr[binr][binz+1][3]-maparr[binr][binz][3])*(pointz-maparr[binr][binz+1][1])/10.0 )*tesla;
	  else
	    bz=maparr[binr][binz][3]*tesla;
	 }

	if(binz==0 && binr!=0)
	  {
	  bz=maparr[binr][binz][3]*tesla;
	  if( binr<59)
	  br=( maparr[binr][binz][2]+ 
	       (maparr[binr+1][binz][2]-maparr[binr][binz][2])*(radius-maparr[binr+1][binz][0])/10.0 )*tesla;
	  else
	    br=maparr[binr][binz][2]*tesla;
         }
	  if(binz==0 && binr==0)
	    {
	      bz=maparr[binr][binz][3]*tesla;
	      br=maparr[binr][binz][2]*tesla;
	    }
    
    }else{

      
 if( binz<94)
 bz=( maparr[binr][binz][3]+ 
      (maparr[binr][binz+1][3]-maparr[binr][binz][3])*(pointz-maparr[binr][binz+1][1])/10.0 )*tesla;
 else
   bz=maparr[binr][binz][3]*tesla;

 if( binr<59)
 br=( maparr[binr][binz][2]+ 
      (maparr[binr+1][binz][2]-maparr[binr][binz][2])*(radius-maparr[binr+1][binz][0])/10.0) *tesla;
 else
   br=maparr[binr][binz][2]*tesla;

    }

      if( point[2]<0.0)
	{
	  bField[0] = -cos(angle)*br*Control::BFactor;
	  bField[1] = -sin(angle)*br*Control::BFactor;
	}else{
          bField[0] = cos(angle)*br*Control::BFactor;
	  bField[1] = sin(angle)*br*Control::BFactor;
        }
          bField[2] = bz*Control::BFactor;

    }


}






