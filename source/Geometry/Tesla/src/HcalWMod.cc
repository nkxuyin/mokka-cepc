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
// $Id: HcalWMod.cc,v 1.6 2007/07/10 16:27:43 mora Exp $
// $Name: mokka-07-00 $
//

// HcalWMod.cc
//
// History:  
// - several changes (see README file), P.MoraDeFreitas

#include "MySQLWrapper.hh"
#include "Control.hh"
#include "MyPlacement.hh"
#include "HcalWMod.hh"
#include "CGAGeometryManager.hh"
#include "SD.hh"
#include "HECSD.hh"

#include "globals.hh"
#include "G4GeometryTolerance.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Polyhedra.hh"
#include "G4Trd.hh"
#include "G4Trap.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4UnionSolid.hh"

#include "CGADefs.h"

INSTANTIATE(HcalWMod)

G4bool 
HcalWMod::construct(const G4String &aSubDetectorName,
		  G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding Hcal..." << G4endl;
  db = new Database(aSubDetectorName);
  
  if(Control::DUMPG3) MyPlacement::Init("HCAL",aSubDetectorName);

  //--------- BarrelHcal Sensitive detector -----
  db->exec("select cell_dim_x, cell_dim_z, chamber_tickness from barrel_module,hcal;");
  db->getTuple();

  //  Hcal  barrel regular modules
  theBarrilRegSD = 
    new SD(db->fetchDouble("cell_dim_x"),
	   db->fetchDouble("cell_dim_z"),
	   db->fetchDouble("chamber_tickness"),
	   HCALBARREL,
	   "HcalBarrelReg");
  RegisterSensitiveDetector(theBarrilRegSD);
  
  // Hcal  barrel end modules
  theBarrilEndSD =
    new SD(db->fetchDouble("cell_dim_x"),
	   db->fetchDouble("cell_dim_z"),
	   db->fetchDouble("chamber_tickness"),
	   HCALBARREL,
	   "HcalBarrelEnd");

  RegisterSensitiveDetector(theBarrilEndSD);

  // Hcal  endcap modules
  theENDCAPEndSD =
    new HECSD(db->fetchDouble("cell_dim_x"),
	      db->fetchDouble("cell_dim_z"),
	      db->fetchDouble("chamber_tickness"),
	      HCALENDCAPMINUS,
	      "HcalEndCaps");

  RegisterSensitiveDetector(theENDCAPEndSD);

  //----------------------------------------------------
  // Barrel
  //----------------------------------------------------
  Barrel(WorldLog);
  
  //----------------------------------------------------
  // EndCap Modules
  //----------------------------------------------------
  
  Endcaps(WorldLog);
  
  // Closes Database connection
  delete db;

  return true;
}
HcalWMod::~HcalWMod() 
{
  //  if(theBarrilRegSD!=0) delete theBarrilRegSD;
  //  if(theBarrilEndSD!=0) delete theBarrilEndSD;
  //  if(theENDCAPEndSD!=0) delete theENDCAPEndSD;
}  


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              Barrel
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HcalWMod::Barrel(G4LogicalVolume* MotherLog)
{
  BarrelRegularModules(MotherLog);
  BarrelEndModules(MotherLog);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              Barrel Regular Modules               ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HcalWMod::BarrelRegularModules(G4LogicalVolume* MotherLog)
{
  // Regular modules

  db->exec("select bottom_dim_x/2 AS BHX,midle_dim_x/2. AS MHX, top_dim_x/2 AS THX, y_dim1_for_x/2. AS YX1H,y_dim2_for_x/2. AS YX2H,module_dim_z/2. AS DHZ from barrel_module,barrel_regular_module;");
  db->getTuple();
  G4double BottomDimY = db->fetchDouble("YX1H");
  G4double chambers_y_off_correction = db->fetchDouble("YX2H");
  
  // Attention: on bâtit le module dans la verticale
  // à cause du G4Trd et on le tourne avant de le positioner
  G4Trd * Bottom = new G4Trd("Bottom_Barrel_Module",
			   db->fetchDouble("BHX"), 
			   db->fetchDouble("MHX"),
			   db->fetchDouble("DHZ"),
			   db->fetchDouble("DHZ"),
			   db->fetchDouble("YX1H"));

  G4Trd * Top = new G4Trd("Top_Barrel_Module",
			   db->fetchDouble("MHX"), 
			   db->fetchDouble("THX"),
			   db->fetchDouble("DHZ"),
			   db->fetchDouble("DHZ"),
			   db->fetchDouble("YX2H"));

  G4UnionSolid* ModuleSolid = 
    new G4UnionSolid("ModuleSolid",
		     Bottom,
		     Top,
		     0,
		     G4ThreeVector(0,
				   0,
				   db->fetchDouble("YX1H")+
				   db->fetchDouble("YX2H")));
  
  EnvLogHcalModuleBarrel  = new G4LogicalVolume(ModuleSolid,
 				CGAGeometryManager::GetMaterial("tungsten_11gccm"),
 						"barrelHcalModule", 
 						0, 0, 0);
  G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(.8,.8,.2));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  EnvLogHcalModuleBarrel->SetVisAttributes(VisAtt);
  

  //----------------------------------------------------
  // Chambers in the Hcal Barrel 
  //----------------------------------------------------
  BarrelRegularChambers(EnvLogHcalModuleBarrel,chambers_y_off_correction);

//   // BarrelStandardModule placements
  db->exec("select stave_id,module_id,module_type,stave_phi_offset,inner_radius,module_z_offset from barrel,barrel_stave, barrel_module, barrel_modules where module_type = 1;"); //  un module: AND stave_id=1 AND module_id = 2
  db->getTuple();
  G4double Y;
  Y = db->fetchDouble("inner_radius")+BottomDimY;
  do {
    G4double phirot = db->fetchDouble("stave_phi_offset")*pi/180;
    G4RotationMatrix *rot=new G4RotationMatrix();
    rot->rotateX(pi*0.5); // on couche le module.
    rot->rotateY(phirot);
    new MyPlacement(rot,
		    G4ThreeVector(-Y*sin(phirot),
				  Y*cos(phirot),
				  db->fetchDouble("module_z_offset")),
		    EnvLogHcalModuleBarrel,
		    "BarrelHcalModule",
		    MotherLog,
		    false,
		    HCALBARREL*100+db->fetchInt("stave_id")*10+
		    db->fetchInt("module_id"));
    theBarrilRegSD->SetStaveRotationMatrix(db->fetchInt("stave_id"),phirot);
    theBarrilRegSD->
      SetModuleZOffset(db->fetchInt("module_id"),
		       db->fetchDouble("module_z_offset"));
  } while(db->getTuple()!=NULL);
}


void HcalWMod::BarrelRegularChambers(G4LogicalVolume* MotherLog,G4double chambers_y_off_correction)
{

  G4LogicalVolume * ChamberLog[200];
  G4Box * ChamberSolid;

  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(.2,.8,.2));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);

  db->exec("select layer_id,chamber_dim_x/2. AS xdh,chamber_tickness/2. AS ydh,chamber_dim_z/2. AS zdh from hcal,barrel_regular_layer,barrel_regular_module;");

  while(db->getTuple()!=NULL){
    ChamberSolid = 
       new G4Box("ChamberSolid", 
		 db->fetchDouble("xdh"),  //hx
		 db->fetchDouble("zdh"),  //hz attention!
		 db->fetchDouble("ydh")); //hy attention!
     
     ChamberLog [db->fetchInt("layer_id")] = 
       new G4LogicalVolume(ChamberSolid,
			   CGAGeometryManager::GetMaterial("polystyrene"),
			   "ChamberLogical", 
			   0, 0, 0);  
     ChamberLog[db->fetchInt("layer_id")]->SetVisAttributes(VisAtt);
     ChamberLog[db->fetchInt("layer_id")]->SetSensitiveDetector(theBarrilRegSD);
   }
   
   
   // Chamber Placements
  // module x and y offsets (needed for the SD)
  db->exec("select 0 AS module_x_offset, module_y_offset from barrel_module;");
  db->getTuple();
  
  G4double Xoff,Yoff;
  Xoff = db->fetchDouble("module_x_offset");
  Yoff = db->fetchDouble("module_y_offset");
   
  db->exec("select layer_id, 0. as chamber_x_offset,chamber_y_offset,0. as chamber_z_offset from barrel_regular_layer;");
   
  while(db->getTuple()!=NULL){    
    G4int layer_id = db->fetchInt("layer_id");
    new MyPlacement(0,
		    G4ThreeVector(db->fetchDouble("chamber_x_offset"),
				  db->fetchDouble("chamber_z_offset"),
				  db->fetchDouble("chamber_y_offset")+
				  chambers_y_off_correction),
		    //!!attention!! y<->z
		    ChamberLog [layer_id],
		    "ChamberBarrel",
		    MotherLog,false,layer_id);
    theBarrilRegSD->
      AddLayer(layer_id,
	       db->fetchDouble("chamber_x_offset") + Xoff - 
	       ((G4Box *)ChamberLog[layer_id]->GetSolid())->GetXHalfLength(),
	       db->fetchDouble("chamber_y_offset") + Yoff,
	       // - ((G4Box *)ChamberLog [layer_id]->GetSolid())->GetZHalfLength(),
	       db->fetchDouble("chamber_z_offset") - 
	       ((G4Box *)ChamberLog[layer_id]->GetSolid())->GetYHalfLength());    
  }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              Barrel End Modules                 ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HcalWMod::BarrelEndModules(G4LogicalVolume* MotherLog)
{
  // End modules
  // Get parameters
  db->exec("select y_dim1_for_z/2. AS YZ1H, y_dim2_for_z/2. AS YZ2H, y_dim3_for_z/2. AS YZ3H, top_end_dim_z/2.  AS TZ from barrel_end_module;");
  db->getTuple();
  G4double YZ1H = db->fetchDouble("YZ1H");
  G4double YZ2H = db->fetchDouble("YZ2H");
  G4double YZ3H = db->fetchDouble("YZ3H");
  G4double TZ = db->fetchDouble("TZ");
  G4double chambers_y_off_correction = YZ3H;

  db->exec("select bottom_dim_x/2 AS BHX,midle_dim_x/2. AS MHX, top_dim_x/2 AS THX, y_dim1_for_x/2. AS YX1H,y_dim2_for_x/2. AS YX2H,module_dim_z/2. AS DHZ from barrel_module,barrel_regular_module;");
  db->getTuple();
  G4double BHX = db->fetchDouble("BHX");
  G4double MHX = db->fetchDouble("MHX");
  G4double THX = db->fetchDouble("THX");
  G4double YX1H = db->fetchDouble("YX1H");
  G4double YX2H = db->fetchDouble("YX2H");
  G4double DHZ = db->fetchDouble("DHZ");

  // Attention: on bâtit le module dans la verticale
  // à cause des G4Trd et on le tourne avant de le positioner
  //
  // Base

  G4double MHXZ = BHX+(YZ1H+YZ2H)*(MHX-BHX)/YX1H;
  G4Trd * Base1 = new G4Trd("Base1_Barrel_End_Module",
			   BHX, // dx1 
			   MHXZ, // dx2
			   DHZ, // dy1
			   DHZ, // dy2
			   YZ1H+YZ2H); // dz
  
  G4Trd * Base2 = new G4Trd("Base2_Barrel_End_Module",
			    MHXZ,  
			    MHX,
			    TZ,
			    TZ,
			    YX1H-(YZ1H+YZ2H));

  G4UnionSolid* Base = 
    new G4UnionSolid("BasesEndSolid",
		     Base1,
		     Base2,
		     0,
		     G4ThreeVector(0,
				   TZ-DHZ,
				   YX1H));
  
  G4Trd * Top = new G4Trd("Top_Barrel_End_Module",
			  MHX, 
			  THX,
			  TZ,
			  TZ,
			  YX2H);
  
  G4UnionSolid* Ensemble1 = 
    new G4UnionSolid("EndSolid",
		     Base,
		     Top,
		     0,
		     G4ThreeVector(0,
				   TZ-DHZ,
				   2*YX1H-(YZ1H+YZ2H)+YX2H));

  G4double MHXZ1 = BHX+((YZ1H+YZ2H)-(TZ-DHZ))*(MHX-BHX)/YX1H;

  G4RotationMatrix *rot = new G4RotationMatrix();
  rot->rotateZ(pi/2.);


  G4double pDX = TZ-DHZ;
  G4double pDz = sqrt(4*YZ2H*YZ2H+pDX*pDX)/2.;  
  G4double  pTheta = pi/2-atan(2*pDz/pDX);

  G4Trap* detail = new G4Trap("Trap",
			      pDz, //pDz
			      -pTheta,    //pTheta
			      0.,       //pPhi
			      MHXZ1, //pDy1
	G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(), //pDx1  
	G4GeometryTolerance::GetInstance()->GetSurfaceTolerance(), //pDx2
			      0.,      //pAlp1
			      MHXZ, //pDy2,
			      pDX, //pDx3
			      pDX, //pDx4
			      0.);      //pAlp2
  
    
  G4UnionSolid* Ensemble2 = 
    new G4UnionSolid("EndSolid",
		     Ensemble1,
		     detail,
		     rot,
		     G4ThreeVector(0,
				   DHZ+pDX-pDz*tan(pTheta),
				   YZ1H+YZ2H-pDz));  //-pDX/2/tan(pTheta)
  
  EnvLogHcalModuleBarrel  = new G4LogicalVolume(Ensemble2,
 				CGAGeometryManager::GetMaterial("tungsten_11gccm"),
 						"barrelHcalModule", 
 						0, 0, 0);

  G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(.8,.8,.2));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  EnvLogHcalModuleBarrel->SetVisAttributes(VisAtt);


  //----------------------------------------------------
  // Chambers in the Hcal Barrel 
  //----------------------------------------------------
  BarrelEndChambers(EnvLogHcalModuleBarrel,chambers_y_off_correction);

//   // Barrel End Module placements
  db->exec("select stave_id,module_id,module_type,stave_phi_offset,inner_radius,module_z_offset from barrel,barrel_stave, barrel_module, barrel_modules where module_type = 2;");  // un module:  AND module_id = 1 AND stave_id = 1
  db->getTuple();
  G4double Y;
  Y = db->fetchDouble("inner_radius")+YZ1H+YZ2H;
  G4int stave_inv [8] = {1,8,7,6,5,4,3,2};
  do {
    G4double phirot = db->fetchDouble("stave_phi_offset")*pi/180;
    G4RotationMatrix *rot=new G4RotationMatrix();
    G4double Z = db->fetchDouble("module_z_offset");
    G4double Xplace = -Y*sin(phirot);
    G4double Yplace = Y*cos(phirot);
    G4int stave_number = db->fetchInt("stave_id");
    rot->rotateX(pi*0.5); // on couche le module.

    if(Z>0) {
      rot->rotateZ(pi);
      Xplace = - Xplace;
      stave_number = stave_inv[stave_number-1];
    }
    
    rot->rotateY(phirot);
    new MyPlacement(rot,
		    G4ThreeVector(Xplace,
				  Yplace,
				  Z),
		    EnvLogHcalModuleBarrel,
		    "BarrelHcalModule",
		    MotherLog,
		    false,
		    HCALBARREL*100+stave_number*10+db->fetchInt("module_id"));
    theBarrilEndSD->SetStaveRotationMatrix(db->fetchInt("stave_id"),phirot);
    theBarrilEndSD->
      SetModuleZOffset(db->fetchInt("module_id"),
		       db->fetchDouble("module_z_offset"));
  } while(db->getTuple()!=NULL);
}

void HcalWMod::BarrelEndChambers(G4LogicalVolume* MotherLog, 
			     G4double chambers_y_off_correction)
{
  G4LogicalVolume * ChamberLog[200];
  G4Box * ChamberSolid;

  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(.2,.8,.2));
  VisAtt->SetForceWireframe(true);
  db->exec("select barrel_regular_layer.layer_id,chamber_dim_x/2. AS xdh,chamber_tickness/2. AS ydh,chamber_dim_z/2. AS zdh from hcal,barrel_regular_layer,barrel_end_layer where barrel_regular_layer.layer_id = barrel_end_layer.layer_id ;");

   while(db->getTuple()!=NULL){
     ChamberSolid = 
       new G4Box("ChamberSolid", 
		 db->fetchDouble("xdh"),  //hx
		 db->fetchDouble("zdh"),  //hz attention!
		 db->fetchDouble("ydh")); //hy attention!
     
     ChamberLog [db->fetchInt("layer_id")] = 
       new G4LogicalVolume(ChamberSolid,
			   CGAGeometryManager::GetMaterial("polystyrene"),
			   "ChamberLogical", 
			   0, 0, 0);  
     ChamberLog[db->fetchInt("layer_id")]->SetVisAttributes(VisAtt);
     ChamberLog[db->fetchInt("layer_id")]->SetSensitiveDetector(theBarrilEndSD);
   }
   
   
   // End Chamber Placements
  // module x and y offsets (needed for the SD)
   db->exec("select 0 AS module_x_offset, module_y_offset from barrel_module;");
   db->getTuple();
   G4double Xoff,Yoff;
   Xoff = db->fetchDouble("module_x_offset");
   Yoff = db->fetchDouble("module_y_offset");
   
   db->exec("select barrel_regular_layer.layer_id, 0. as chamber_x_offset,chamber_y_offset,chamber_z_offset as chamber_z_offset,chamber_dim_z/2 AS YHALF,chamber_dim_x/2. XHALF from barrel_regular_layer,barrel_end_layer where barrel_regular_layer.layer_id = barrel_end_layer.layer_id;");
   
   while(db->getTuple()!=NULL){
     G4int layer_id = db->fetchInt("layer_id");
     new MyPlacement(0,
		     G4ThreeVector(db->fetchDouble("chamber_x_offset"),
				   db->fetchDouble("chamber_z_offset"),
				   db->fetchDouble("chamber_y_offset")+
				   chambers_y_off_correction),
		     //!!attention!! y<->z
		     ChamberLog [db->fetchInt("layer_id")],
		     "ChamberBarrel",
		     MotherLog,false,layer_id);
    theBarrilEndSD->
      AddLayer(layer_id,
	       db->fetchDouble("chamber_x_offset") + Xoff - db->fetchDouble("XHALF"),
	       db->fetchDouble("chamber_y_offset") + Yoff,
	       // - ((G4Box *)ChamberLog [layer_id]->GetSolid())->GetZHalfLength(),
	 //  db->fetchDouble("chamber_z_offset") - db->fetchDouble("YHALF"));
	       - (db->fetchDouble("chamber_z_offset")+db->fetchDouble("YHALF")));
   }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              Endcaps                 ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HcalWMod::Endcaps(G4LogicalVolume* MotherLog)
{
  db->exec("select module_radius AS pRMax, module_dim_z/2. AS pDz, center_box_size/2. AS pRMin from endcap_standard_module;");
  db->getTuple();

//   G4Tubs* EndCapSolid=
//     new G4Tubs ("HcalEndCapSolid",
// 		db->fetchDouble("pRMin"),
// 		db->fetchDouble("pRMax"),
// 		db->fetchDouble("pDz"),
// 		0.,
// 		2*pi);

  G4double zPlane[2];
  zPlane[0]=-db->fetchDouble("pDz");
  zPlane[1]=-zPlane[0];

  G4double rInner[2],rOuter[2];
  rInner[0]=rInner[1]=db->fetchDouble("pRMin");
  rOuter[0]=rOuter[1]=db->fetchDouble("pRMax");

  G4Polyhedra *EndCapSolid=
    new G4Polyhedra("HcalEndCapSolid",
		    0.,
		    360.,
		    32,
		    2,
		    zPlane,
		    rInner,
		    rOuter);
		    

  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(.8,.8,.2));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);

  G4LogicalVolume* EndCapLogical =
    new G4LogicalVolume(EndCapSolid,
			CGAGeometryManager::GetMaterial("tungsten_11gccm"),
			"EndCapLogical",
			0, 0, 0);
  EndCapLogical->SetVisAttributes(VisAtt);

  // build and place the chambers in the Hcal Endcaps
  EndcapChambers(EndCapLogical);

  // Placements
   
  db->exec("select endcap_id,endcap_z_offset from endcap;");
   
  G4RotationMatrix *rotEffect=new G4RotationMatrix();
  rotEffect->rotateZ(pi/8.);
  G4int ModuleNumber = HCALENDCAPPLUS*100+16;
  G4double Z1=0;
  while(db->getTuple()!=NULL){    
    Z1=db->fetchDouble("endcap_z_offset");
    new MyPlacement(rotEffect,
		    G4ThreeVector(0.,
				  0.,
				  Z1),
		    EndCapLogical,
		    "EndCapPhys",
		    MotherLog,
		    false,
		    ModuleNumber);
    rotEffect=new G4RotationMatrix();
    rotEffect->rotateZ(pi/8.);
    rotEffect->rotateY(pi); // On inverse les endcaps 
    ModuleNumber -= (HCALENDCAPPLUS-HCALENDCAPMINUS)*100 + 6;
  }
  theENDCAPEndSD->
    SetModuleZOffset(0,
		     fabs(Z1));
  theENDCAPEndSD->
    SetModuleZOffset(6,
		     fabs(Z1));
}

void HcalWMod::EndcapChambers(G4LogicalVolume* MotherLog)
{
  // Chambers in the HcalWMod::Endcaps
  // standard endcap chamber solid:
  db->exec("select chamber_radius AS pRMax, chamber_tickness/2. AS pDz, center_box_size/2. AS pRMin from endcap_standard_module,hcal;");
  db->getTuple();
  
//   G4Tubs* EndCapChamberSolid=
//     new G4Tubs ("EndCapChamberSolid",
// 		db->fetchDouble("pRMin"),
// 		db->fetchDouble("pRMax"),
// 		db->fetchDouble("pDz"),
// 		0.,
// 		2*pi);

  
  G4double zPlane[2];
  zPlane[0]=-db->fetchDouble("pDz");
  zPlane[1]=-zPlane[0];

  G4double rInner[2],rOuter[2];
  rInner[0]=rInner[1]=db->fetchDouble("pRMin");
  rOuter[0]=rOuter[1]=db->fetchDouble("pRMax");

  G4Polyhedra *EndCapChamberSolid=
    new G4Polyhedra("EndCapChamberSolid",
		    0.,
		    360.,
		    32,
		    2,
		    zPlane,
		    rInner,
		    rOuter);
		    
  
  // standard endcap chamber logical
  G4LogicalVolume* EndCapChamberLogical =
    new G4LogicalVolume(EndCapChamberSolid,
			CGAGeometryManager::GetMaterial("polystyrene"),
			"EndCapChamberLogical",
			0, 0, 0);
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(0,1.,1.));
  EndCapChamberLogical->SetVisAttributes(VisAtt);
  
  // LE SENSITIVE ICI
  EndCapChamberLogical->SetSensitiveDetector(theENDCAPEndSD); 

  // standard endcap chamber placements
  db->exec("select layer_id,chamber_z_offset AS Zoff from endcap_layer;");
  
  G4int layer_id;
  while(db->getTuple()!=NULL){
    layer_id=db->fetchInt("layer_id");
    new MyPlacement(0,
		    G4ThreeVector(0.,
				  0.,
				  db->fetchDouble("Zoff")),
		    EndCapChamberLogical,
		    "EndCapChamberPhys",
		    MotherLog,false,layer_id);
    theENDCAPEndSD->
      AddLayer(layer_id,
	       0,
	       0,
	       db->fetchDouble("Zoff"));    
  }  
}
