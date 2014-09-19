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
// $Id: SiDBar00.cc,v 1.7 2006/02/07 16:50:15 musat Exp $
//
//
// SiDBar00.cc
//


#include "Control.hh"
#include "G4PVPlacement.hh"
#include "SiDBar00.hh"
#include "TRKSD00.hh"

#include "CGAGeometryManager.hh"

#include "globals.hh"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

#include "MySQLWrapper.hh"
#include <assert.h>

#include "CGADefs.h"

INSTANTIATE(SiDBar00)

SiDBar00::~SiDBar00()
{
//   if (!theSiDBarSD) delete theSiDBarSD;
}


G4bool SiDBar00::construct(const G4String &aSubDetectorName,
			 G4LogicalVolume *WorldLog)
{
//   G4double start_phi = 0.0*deg;
//   G4double stop_phi = 360.0*deg;

  G4VisAttributes * VisAtt = 
    new G4VisAttributes(G4Colour(1,0,0));
  VisAtt->SetForceWireframe(true);
  ///VisAtt->SetForceSolid(true);

  G4cout << "\nBuilding SiDBar..." << G4endl;
  db = new Database(aSubDetectorName.data());



  //=== retrieve the envelop parameters
  db->exec("select * from sit_envelop;");
  db->getTuple();


  G4double inner_radious   = db->fetchDouble("inner_radius");
  G4double outer_radius    = db->fetchDouble("outer_radius");
  G4int    N_layers        = db->fetchInt("N_layers");
//   G4double fiber_thickness = db->fetchDouble("fiber_thickness");


  //=== retrieve the standard ladder parameters
  db->exec("select * from ladder;");
  db->getTuple();


//   G4int    layer_id       = db->fetchInt("id");
  G4double l_thickness    = db->fetchDouble("thickness");
//   G4double l_width        = db->fetchDouble("width");
  G4double l_length       = db->fetchDouble("length");
//   G4double l_border_gap   = db->fetchDouble("border_gap");
  G4double l_vertical_gap = db->fetchDouble("vertical_gap");
  G4double l_pitch        = db->fetchDouble("pitch");
//   G4int    l_n_zseg       = db->fetchInt("n_zseg");
//   G4double l_z_overlap    = db->fetchDouble("z_overlap");
  G4int    l_n_ladder_phi = db->fetchInt("n_ladder_phi");
  G4int    l_n_ladder_z   = db->fetchInt("n_ladder_z");



  // retrieve the electronics parameters
  db->exec("select * from electronics;");
  db->getTuple();

  G4double e_thickness = db->fetchDouble("thickness");
  G4double e_width     = db->fetchDouble("width");
  G4double e_length    = db->fetchDouble("length");


  
  // The SiDBar Sensitive detector
  //

  theSiDBarSD = 
    new TRKSD00("SiDBar00",
		l_thickness * mm 
		* 340 * keV
		* 0.2);
  RegisterSensitiveDetector(theSiDBarSD);




  G4LogicalVolume *SitEnvelopLogical = NULL;
  
  G4double HalfAlveolusX;
  G4double HalfAlveolusZ = 0.;
  G4LogicalVolume *AlveolusLogical = NULL;
  G4double l_width_L;

  
   for (G4int i_layer=0; i_layer < N_layers ; i_layer++) 
    {

      ///G4double HalfAlveolusY = e_thickness + l_vertical_gap;
      G4double HalfAlveolusY = l_vertical_gap;
      G4double StartAng =0.;
      
      G4double RFirstLayer =
	inner_radious + i_layer*l_pitch + HalfAlveolusY;


      for (G4int i_slayer=0; i_slayer < 2 ; i_slayer++)
	{

	  G4double NextLayerR = 
	    RFirstLayer + i_slayer*2*HalfAlveolusY;
	  

	  G4double StepAng; 
	  l_n_ladder_phi = 32; 
	  StepAng = pi/l_n_ladder_phi;
	  l_width_L = StepAng*NextLayerR;
// 	  G4cout << " === l_width_L= " << l_width_L << endl; 


	  if (!i_slayer){
	    
	    ///HalfAlveolusX = l_width/2 + l_border_gap;
	    HalfAlveolusX = l_width_L/2;

	    ///  G4double HalfAlveolusZ = env_z_length/2 - l_border_gap;
	    // ???

	    HalfAlveolusZ = l_length*l_n_ladder_z/2.;   


	    //***************************
	    // SiDBar00  envelop
	    //***************************
	    // 
	    
	    
	    G4Tubs *SitEnvelopSolid
	      = new G4Tubs("SitEnvelopSolid", 
			   inner_radious-5*l_thickness, 
			   outer_radius+40*l_thickness,
			   HalfAlveolusZ, // HalfZ as parameter !!!
			   0*deg, 
			   360.*deg);
	    
	    
	    SitEnvelopLogical =
	      new G4LogicalVolume(SitEnvelopSolid,
				  CGAGeometryManager::GetMaterial("g10"),
				  "SitEnvelopLogical", 
				  0,
				  0,
				  0);
	    
	   
	    VisAtt = new G4VisAttributes(G4Colour(0.0,0.8,0.0));
	    VisAtt->SetForceWireframe(true);
	    // VisAtt->SetForceSolid(true);
	    VisAtt->SetVisibility(true);
	    SitEnvelopLogical->SetVisAttributes(VisAtt);
	    
	    
	    // Phys=
// 	    new G4PVPlacement(0,
// 			      G4ThreeVector(0., 0., 0.),
// 			      SitEnvelopLogical,
// 			      "SitEnvelopPhys",
// 			      WorldLog,
// 			      false,0);
	    // === SiDBar00  envelop ===^

	    SitEnvelopLogical=    WorldLog;

	    G4Box *AlveolusSolid = 
	      new G4Box("Alveolus", 
			HalfAlveolusX,
			HalfAlveolusY,
			HalfAlveolusZ);

	    AlveolusLogical =
	      new G4LogicalVolume(AlveolusSolid,
				  CGAGeometryManager::GetMaterial("air"),
				  "AlveolusLogical", 
				  0,
				  0,
				  0);
      
	    VisAtt = new G4VisAttributes(G4Colour(1,1,1));
	    VisAtt->SetForceWireframe(true);
	    //VisAtt->SetForceSolid(true);
	    VisAtt->SetDaughtersInvisible(true);
	    AlveolusLogical->SetVisAttributes(VisAtt);
      
	  } // if !i_slayer
	  
	  
	  for(G4int i_phy=0; i_phy < l_n_ladder_phi; i_phy++)
	    {
	      
	      G4RotationMatrix *rot=new G4RotationMatrix();
	      G4double Angle = StartAng + i_phy * 2* (StepAng) 
		+ (i_slayer) * StepAng;
	      
// 	      G4cout << " === Angle= " << Angle  << endl;
	      rot->rotateZ(Angle);
	      
	      //// Phys=
	      new G4PVPlacement(rot,
				G4ThreeVector(NextLayerR * sin(Angle),
					      NextLayerR * cos(Angle), 
					      0.),
				AlveolusLogical,
				"AlveolusPhys",
				SitEnvelopLogical,
				false,
				0);
	    } // for i_phy 
	} // for i_slayer



      



      G4Box *ladderSolid = 
	new G4Box("ladderSolid", 
		  l_width_L/2,          // Hx
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

      ladderLogical->SetSensitiveDetector(theSiDBarSD);


      // standard electronics block
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
      VisAtt->SetForceWireframe(true);
      /////VisAtt->SetForceSolid(true);
      electronicsLogical->SetVisAttributes(VisAtt);
      

      
      // placing the ladders and electronics inside the standard alveolus
      
      G4double Zpos  = -HalfAlveolusZ + l_length/2;

      G4double ZStep = l_length;

      G4double l_Ydisp = (l_vertical_gap + l_thickness)/2;
      G4double e_Ydisp = (l_vertical_gap + e_thickness)/2;


      
      for(G4int i_ladder=0; i_ladder<l_n_ladder_z; i_ladder++)
	{
	  // ladder
	  G4double YPos = l_Ydisp; 
	  if(i_ladder%2) YPos = -YPos;
	  
	  ////Phys=
	  new G4PVPlacement(0,
			    G4ThreeVector(0.,YPos, Zpos),
			    ladderLogical,
			    "ladderPhys",
			    AlveolusLogical,
			    false,
			    i_ladder+1);

	  // electronics
	  YPos = e_Ydisp;
	  G4double e_ZPos = Zpos + (l_length+e_length)/2.;

	  if(i_ladder%2) 
	    {
	      YPos = -YPos;
	      e_ZPos = Zpos - (l_length+e_length)/2.;
	    }
	  
	  ///Phys=
	  new G4PVPlacement(0,
			    G4ThreeVector(0.,
					  YPos, 
					  e_ZPos),
			    electronicsLogical,
			    "electronics",
			    AlveolusLogical,
			    false,
			    0);
	  Zpos+=ZStep;
	} // for (i_ladder)
    

    } // for (N_layers)
  
  // Closes Database connection
  delete db;
  db = 0;
  G4cout << "SiDBar done.\n" << G4endl;
  return true;
}

