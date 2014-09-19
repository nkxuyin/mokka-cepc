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
//
// $Id: Hodoscope00.cc,v 1.4 2005/04/08 14:37:15 musat Exp $
// $Name: mokka-07-00 $
//
// Hodoscope00.cc
//
// History:  
// - first implementation P. Mora de Freitas (apr 01)


#include "MySQLWrapper.hh"
#include "Hodoscope00.hh"
#include "HodoscopeSD00.hh"
#include "CGAGeometryManager.hh"

#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4Box.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"

#include "CGADefs.h"

INSTANTIATE(Hodoscope00)

Hodoscope00::~Hodoscope00()
{
//   if (!theHodoscopeSD) delete theHodoscopeSD;
}


G4bool Hodoscope00::construct(const G4String &aSubDetectorName,
			 G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding Hodoscope..." << G4endl;

  // Sensitive detector
  theHodoscopeSD = new HodoscopeSD00("Hodoscope");
  RegisterSensitiveDetector(theHodoscopeSD);

  Database* db = new Database(aSubDetectorName.data());
  
  db->exec("select * from parameters;");
  db->getTuple();

  G4double fiber_tickness = db->fetchDouble("fiber_tickness");
  G4double cladding_percent = db->fetchDouble("cladding_percent");
  G4int n_fibers = db->fetchInt("n_fibers");
  G4double hodos_depth = db->fetchDouble("hodos_depth");

  G4double cladding_tickness = fiber_tickness * cladding_percent/100;
  G4double half_core_tickness = 
    (fiber_tickness - 2*cladding_tickness)/2.;

  // Each layer = A big PMMA envelope (here also in Polystyrene)
  // whith the Polystyrene fiber cores placed inside it

  // A single Layer envelope:
  G4double half_EnvelopeLayerSideX = 
    (fiber_tickness*n_fibers)/2.0;

  G4Box *HodoscopeLayerSolid
    = new G4Box ("LayerEnvSol",
		 half_EnvelopeLayerSideX, 
                 fiber_tickness/2,
		 half_EnvelopeLayerSideX);

  G4LogicalVolume *HodoscopeLayerLogical
    = new G4LogicalVolume(HodoscopeLayerSolid,
			  CGAGeometryManager::GetMaterial("polystyrene"),
			  "LayerEnvLog", 
			  0,
			  0,
			  0);
 
  G4VisAttributes * VisAtt = 
    new G4VisAttributes(G4Colour(0.,0.5,0.5));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  HodoscopeLayerLogical->SetVisAttributes(VisAtt);
  
  // A single Polystyrene fiber core
  G4Box *FiberCoreSolid
    = new G4Box ("FiberCoreSol",
		 half_core_tickness, 
                 half_core_tickness,
		 half_EnvelopeLayerSideX);

  G4LogicalVolume *FiberCoreLogical
    = new G4LogicalVolume(FiberCoreSolid,
			  CGAGeometryManager::GetMaterial("polystyrene"),
			  "FiberCoreLog", 
			  0,
			  0,
			  0);
 
  FiberCoreLogical->SetSensitiveDetector(theHodoscopeSD);
  VisAtt = new G4VisAttributes(G4Colour(1.,0.5,0.5));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  FiberCoreLogical->SetVisAttributes(VisAtt);

  // Now we place the fiber cores inside the Layer envelope
  // numbered from 1 to n_fibers

  G4PVPlacement *Phys;
  G4int i;
  for (i = 0; i < n_fibers; i++)
    {
      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(- half_EnvelopeLayerSideX
					+ cladding_tickness 
					+ half_core_tickness
					+ i*fiber_tickness, 
					0., 
					0.),
			  FiberCoreLogical,
			  "FiberCore",
			  HodoscopeLayerLogical,
			  false,i+1);
    }

  // Now we place the layers

  G4double base = hodos_depth/2.;
  G4double deltaX = fiber_tickness/4.;
  G4double deltaY = fiber_tickness/2.;
  G4double deltaSenseLayerY = fiber_tickness;

  G4RotationMatrix *rot=new G4RotationMatrix();
  rot->rotateY(pi*0.5);

  G4RotationMatrix *rotS=0;

  G4int s_X = -1;
  for (i = 0; i < 8; i++)
    { 
      s_X = - s_X;
      G4int s_base = (i<4)? -1:1;
      G4int s_deltaY = - s_X;
      G4int s_deltaSY = ((i/2)%2==0)? -1:1;
      rotS = (s_deltaSY==1)? rot : 0;
      G4int is_X = (rotS == 0)? 1 : 0;
      G4int is_Z = (is_X == 0)? 1 : 0;

      Phys=
	new G4PVPlacement(rotS,
			  G4ThreeVector(is_X * s_X * deltaX,
					s_base * base
					+ s_deltaY * deltaY
					+ s_deltaSY * deltaSenseLayerY,
					is_Z * s_X * deltaX),
			  HodoscopeLayerLogical,
			  "FiberLayer",
			  WorldLog,
			  false,i+1);
    }
  
  // Closes Database connection
  delete db;
  G4cout << "Hodoscope done.\n" << G4endl;
  return true;
}
