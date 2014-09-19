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
// $Id: Coil00.cc,v 1.6 2008/10/05 18:28:27 frank Exp $
// $Name: mokka-07-00 $
//
// 
//----------------------------------------------------
// Coil00.cc
//
// History:  
// - first implementation P. Mora de Freitas (may 01)
// - F.Gaede:  write out parameters to GEAR  (Oct 08)

#include "globals.hh"
#include "G4Tubs.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

#include "Coil00.hh"
#include "MySQLWrapper.hh"
#include "Control.hh"
#include "CGAGeometryManager.hh"

#include "CGADefs.h"

#ifdef MOKKA_GEAR
#include "gearimpl/GearParametersImpl.h" 
#include "MokkaGear.h"
#endif

INSTANTIATE(Coil00)

G4bool Coil00::construct(const G4String &aSubDetectorDBName,
			 G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding Coil..." << G4endl;
  Database *db = new Database(aSubDetectorDBName.data());
  
  db->exec("select * from coil;");
  db->getTuple();
  // Coil as an Al tube
  G4Tubs *CoilEnvelopeSolid
    = new G4Tubs("CoilEnvelope", 
		 db->fetchDouble("inner_radius"), 
		 db->fetchDouble("outer_radius"),
		 db->fetchDouble("half_z"),
		 0*deg, 
		 360.*deg);
  
  G4LogicalVolume *CoilLogical=
    new G4LogicalVolume(CoilEnvelopeSolid,
			CGAGeometryManager::GetMaterial("aluminium"),
			"CoilLogical", 
			0,
			0,
			0);
  
  G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(0.,0.,0.7));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  CoilLogical->SetVisAttributes(VisAtt);

  new G4PVPlacement(0,
		    G4ThreeVector(0., 0., 0),
		    CoilLogical,
		    "CoilEnvelope",
		    WorldLog,
		    false,0);
  
#ifdef MOKKA_GEAR
  //----------------------------------------------------
  // MokkaGear
  //----------------------------------------------------

  MokkaGear* gearMgr = MokkaGear::getMgr() ;
  
  gear::GearParametersImpl* gp = new gear::GearParametersImpl ;

  gp->setDoubleVal( "Coil_cryostat_inner_radius", db->fetchDouble("inner_radius") ) ;
  gp->setDoubleVal( "Coil_cryostat_outer_radius", db->fetchDouble("outer_radius") ) ;
  gp->setDoubleVal( "Coil_cryostat_half_z", db->fetchDouble("half_z") ) ;
  gp->setStringVal( "Coil_material", "aluminium"  ) ;


  gearMgr->setGearParameters("CoilParameters", gp ) ;
#endif


  G4cout << "Coil done.\n" << G4endl;
  delete db;
  return true;
}

