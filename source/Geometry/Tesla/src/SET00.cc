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
// $Id: SET00.cc,v 1.3 2005/04/07 16:17:02 musat Exp $
// $Name: mokka-07-00 $
//
//
// SET00.cc
//
// History:  
// - first implementation P. Mora de Freitas (apr 01)


#include "Control.hh"
#include "MySQLWrapper.hh"
#include "SET00.hh"
#include "CGAGeometryManager.hh"

#include "globals.hh"
#include "G4Tubs.hh"
#include "G4Box.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"

#include "CGADefs.h"

INSTANTIATE(SET00)

SET00::~SET00()
{
  //  if (!theSETSD) delete theSETSD;
}


G4bool SET00::construct(const G4String &aSubDetectorName,
			 G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding SET..." << G4endl;

  // Sensitive detector
  //theSETSD = new SETSD00("SET");

  Database* db = new Database(aSubDetectorName.data());

  // retrieve the envelop parameters
  db->exec("select * from set_envelop;");
  db->getTuple();

  G4double inner_radius    = db->fetchDouble("inner_radius");
  G4double outer_radius    = db->fetchDouble("outer_radius");
  G4int    N_layers        = db->fetchInt("N_layers");
  G4double fiber_thickness = db->fetchDouble("fiber_thickness");

  // retrieve the standard ladder parameters
  db->exec("select * from ladder;");
  db->getTuple();

  G4double l_thickness = db->fetchDouble("thickness");
  G4double l_width = db->fetchDouble("width");
  G4double l_length=db->fetchDouble("length");
  G4double l_border_gap = db->fetchDouble("border_gap");
  G4double l_vertical_gap = db->fetchDouble("vertical_gap");
  // G4double l_pitch = db->fetchDouble("pitch");
  // G4int    l_n_zseg = db->fetchInt("n_zseg");
  G4double l_z_overlap = db->fetchDouble("z_overlap");
  G4int    l_n_ladder_phi = db->fetchInt("n_ladder_phi");
  G4int    l_n_ladder_z = db->fetchInt("n_ladder_z");

  // retrieve the electronics parameters
  db->exec("select * from electronics;");
  db->getTuple();
  G4double e_thickness = db->fetchDouble("thickness");
  G4double e_width = db->fetchDouble("width");
  G4double e_length=db->fetchDouble("length");

  //***************************
  // SET fiber envelop
  //***************************
  // the Z envelop dimension is function of the number
  // of ladders, the z_overlap percent and so on.
  G4double env_z_length = 
    l_length * l_n_ladder_z
    - (l_n_ladder_z - 1) * l_z_overlap * l_length + 2 * l_border_gap;

  G4PVPlacement *Phys;

  G4Tubs *SetEnvelopSolid
    = new G4Tubs("SetEnvelopSolid", 
		 inner_radius, 
		 outer_radius,
		 env_z_length/2.,  // HalfZ as parameter !!!
		 0*deg, 
		 360.*deg);
  
  G4LogicalVolume *SetEnvelopLogical =
    new G4LogicalVolume(SetEnvelopSolid,
			CGAGeometryManager::GetMaterial("g10"),
			"SetEnvelopLogical", 
			0,
			0,
			0);
  
  G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(0.8,0.0,0.5));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  VisAtt->SetVisibility(false);
  SetEnvelopLogical->SetVisAttributes(VisAtt);
  Phys=
    new G4PVPlacement(0,
 		      G4ThreeVector(0., 0., 0.),
 		      SetEnvelopLogical,
 		      "SetEnvelopPhys",
 		      WorldLog,
 		      false,0);
  

  //***************************
  // Standard Air alveolus
  //***************************
  G4double HalfAlveolusX = l_width/2 + l_border_gap;

  // HalfAlveolusY: 1) the eletronics block takes more vertical 
  //                place than the ladder Si plate;
  //                2) we count the l_vertical_gap twice,
  //                to simulate also the Air in alveolus.
  G4double HalfAlveolusY = e_thickness + l_vertical_gap;

  //  G4double HalfAlveolusY = e_thickness + l_vertical_gap/2; 
  G4double HalfAlveolusZ = env_z_length/2 - l_border_gap;
  
  G4Box *AlveolusSolid = 
      new G4Box("Alveolus", 
		HalfAlveolusX,
		HalfAlveolusY,
		HalfAlveolusZ);

  G4LogicalVolume *AlveolusLogical =
    new G4LogicalVolume(AlveolusSolid,
			CGAGeometryManager::GetMaterial("air"),
			"AlveolusLogical", 
			0,
			0,
			0);
  
  VisAtt = new G4VisAttributes(G4Colour(1,1,1));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  AlveolusLogical->SetVisAttributes(VisAtt);

  // Placing the alveolus
  // (we deal with each shifted layer as an independent one.)


  G4double DeltaLayer = 
    (outer_radius - inner_radius - HalfAlveolusY) / (2 * N_layers);
  
  G4double RFirstLayer = 
    inner_radius + fiber_thickness + DeltaLayer / 2.;

  //G4double StepAng = atan(HalfAlveolusX/inner_radius) / 2.;
  G4double StepAng = pi / 180. * 3.214;
  G4double StartAng = 0.;

  // ??????????????????????????????????????
  l_n_ladder_phi = int(pi / StepAng);

  for (G4int i_layer=0; i_layer < 2*N_layers ; i_layer++)
    {
      G4double NextLayerR = 
	RFirstLayer + i_layer * DeltaLayer;

      for(G4int i_phy=0; i_phy < l_n_ladder_phi; i_phy++)
	{
	  G4RotationMatrix *rot=new G4RotationMatrix();
	  G4double Angle = StartAng + i_phy * 2 * StepAng 
	    - (i_layer % 2) * StepAng;
	  rot->rotateZ(Angle);
	  
	  Phys=
	    new G4PVPlacement(rot,
			      G4ThreeVector(NextLayerR * sin(Angle),
					    NextLayerR * cos(Angle), 
					    0.),
			      AlveolusLogical,
			      "AlveolusPhys",
			      SetEnvelopLogical,
			      false,0);
	}
    }

  //***************************
  // standard ladder
  //***************************
  G4Box *ladderSolid = 
      new G4Box("ladderSolid", 
		l_width/2,          // Hx
		l_thickness/2,      // Hy
		l_length/2);        // Hz

  G4LogicalVolume *ladderLogical =
    new G4LogicalVolume(ladderSolid,
			CGAGeometryManager::GetMaterial("silicon_2.33gccm"),
			"ladderLogical", 
			0,
			0,
			0);
  
  VisAtt = new G4VisAttributes(G4Colour(0,0,1));
  //VisAtt->SetForceWireframe(true);
  VisAtt->SetForceSolid(true);
  ladderLogical->SetVisAttributes(VisAtt);

  //***************************
  // standard electronics block
  //***************************
  G4Box *electronicsSolid = 
      new G4Box("electronics", 
		e_width/2,          // Hx
		e_thickness/2,      // Hy
		e_length/2);        // Hz

  G4LogicalVolume * electronicsLogical=
    new G4LogicalVolume(electronicsSolid,
			CGAGeometryManager::GetMaterial("silicon_2.33gccm"),
			"electronics", 
			0,
			0,
			0);
  
  VisAtt = new G4VisAttributes(G4Colour(1,0,0));
  //VisAtt->SetForceWireframe(true);
  VisAtt->SetForceSolid(true);
  electronicsLogical->SetVisAttributes(VisAtt);

  
  // placing the ladders and electronics  inside the standard
  // alveolus
  G4double Zpos  = -HalfAlveolusZ + l_length/2;
  G4double ZStep = l_length * (1.0 - l_z_overlap);
  G4double l_Ydisp = (l_vertical_gap + l_thickness)/2;
  G4double e_Ydisp = (l_vertical_gap + e_thickness)/2;

  for(G4int i_ladder=0; i_ladder<l_n_ladder_z; i_ladder++)
    {
      // ladder
      G4double YPos = l_Ydisp; 
      if(i_ladder%2) YPos = -YPos;

      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0.,YPos, Zpos),
			  ladderLogical,
			  "ladderPhys",
			  AlveolusLogical,
			  false,0);
      // electronics
      YPos = e_Ydisp;
      G4double e_ZPos = Zpos + (l_length+e_length)/2.;
      if(i_ladder%2) 
	{
	  YPos = -YPos;
	  e_ZPos = Zpos - (l_length+e_length)/2.;
	}
      
      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0.,
					YPos, 
					e_ZPos),
			  electronicsLogical,
			  "electronics",
			  AlveolusLogical,
			  false,0);
      Zpos+=ZStep;
    }

  // Closes Database connection
  delete db;
  G4cout << "SET done.\n" << G4endl;
  return true;
}
