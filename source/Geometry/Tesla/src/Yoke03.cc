// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: Yoke03.cc,v 1.2 2007/10/09 13:01:34 mora Exp $
// $Name: mokka-07-00 $
//
// History:  
// - first implementation P. Mora de Freitas (May 2001)
// - selectable symmetry, self-scaling, removed pole tips -- Adrian Vogel, 2006-03-17
// - muon system plus
//   instrumented pole tip back for TESLA models   -- Predrag Krstonosic , 2006-08-30

#include "Yoke03.hh"
#include "G4PVPlacement.hh"
#include "CGADefs.h"
#include "MySQLWrapper.hh"
#include "Control.hh"
#include "MyPlacement.hh"
#include "G4UserLimits.hh"
#include "Hcal04.hh"
#include "CGAGeometryManager.hh"
#include "muonSD.hh"
#include "HECSD.hh"
#include "globals.hh"
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
#include "G4SubtractionSolid.hh"
#include <stdlib.h>
INSTANTIATE(Yoke03)


G4bool Yoke03::ContextualConstruct(const CGAGeometryEnvironment &env, G4LogicalVolume *worldLog)
{
  Database *db = new Database(env.GetDBName());
  db->exec("SELECT * FROM `yoke`;");
  db->getTuple();
  
  // Geometry parameters from the geometry environment and from the database

  symmetry = db->fetchInt("symmetry");


  const G4double rInnerBarrel         = env.GetParameterAsDouble("Yoke_barrel_inner_radius");
  const G4double rInnerEndcap         = env.GetParameterAsDouble("Yoke_endcap_inner_radius");
  const G4double drBarrel             = env.GetParameterAsDouble("Yoke_thickness"); 
  const G4double zStartEndcap         = env.GetParameterAsDouble("Yoke_Z_start_endcaps");
  const G4double zEndYoke             = env.GetParameterAsDouble("Yoke_Barrel_Half_Z");
  cout << " z end yoke " << zEndYoke << endl;
  // Simple calculations and some named constants

  const G4double fullAngle            = 360 * deg;
  const G4double tiltAngle            = 180 * deg / symmetry - 90 * deg; // the yoke should always rest 
                                                                         //on a flat side
  const G4double tiltAngle2            = 180 * deg / symmetry;
  const G4double rOuterBarrel         = rInnerBarrel + drBarrel; 
  const G4double dzEndcap             = zEndYoke - zStartEndcap;
  const G4double zEndcap              = zEndYoke - dzEndcap / 2;
 

  G4double Angle = 360 * deg / symmetry;
  double Angle2 = acos(-1.)/symmetry;

    db->exec("SELECT * FROM `muon`;");
    db->getTuple();
    iron_thickness   = db->fetchDouble("iron_thickness");
    layer_thickness  = db->fetchDouble("layer_thickness");
    number_of_layers  = db->fetchInt("number_of_layers");

	G4double cell_dim_x=db->fetchDouble("cell_size");
	G4double cell_dim_z=db->fetchDouble("cell_size"); 

    cout << " number of layers " << number_of_layers << endl;
  // check for consistensy ...
 
  G4double testbarrel= number_of_layers*(iron_thickness+layer_thickness); 
   if( testbarrel >= drBarrel  || testbarrel >= ( zEndYoke-zStartEndcap))
      Control::Abort("Yoke03: Overfull barrel or endcap! check layer number or dimensions !  sorry :( ",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

  // notice that the barrel is "short" and endcap has full radius !!! 
  const G4double zPosBarrelArray[2]   = { -zStartEndcap, +zStartEndcap };  
  const G4double rInnerBarrelArray[2] = { rInnerBarrel, rInnerBarrel };
  const G4double rOuterBarrelArray[2] = { rOuterBarrel, rOuterBarrel };
  
  const G4double zPosEndcapArray[2]   = { -dzEndcap / 2, +dzEndcap / 2 };
  const G4double rInnerEndcapArray[2] = { rInnerEndcap, rInnerEndcap };
  const G4double rOuterEndcapArray[2] = { rOuterBarrel, rOuterBarrel };
  
  // Materials

  G4Material *yokeMaterial = CGAGeometryManager::GetMaterial("iron");
  
  // Visualisation attributes

  G4VisAttributes *yokeVisAttributes = new G4VisAttributes(G4Colour(0.1, 0.8, 0.8)); // light cyan
  yokeVisAttributes->SetForceWireframe(true);
  yokeVisAttributes->SetDaughtersInvisible(true);

  // model with plugs or not   

  G4String is_plug=env.GetParameterAsString("Yoke_with_plug");
  cout << " is plug " << is_plug << endl;

  //    is_plug=true;
  // User Limits 

  const G4double maxStep  = 1.0*mm; // max allowed step size in this volume
  const G4double maxTrack =DBL_MAX; // max total track length
  const G4double maxTime  = DBL_MAX; // max time
  const G4double minEkine = 0;       // min kinetic energy  (only for charged particles)
  const G4double minRange = 0;       // min remaining range (only for charged particles)
  G4UserLimits *userLimits = new G4UserLimits(maxStep, maxTrack, maxTime, minEkine, minRange);

/*---------------------------------------------------------------------------------------*/
// calculation of the sensitive layer offset wtihin the rpc structure
  G4double chamber_thickness=0.0*mm;
  G4double shift_in_layer=0.0*mm; 
    db->exec("SELECT * FROM `rpc1`;");

    while (db->getTuple()) 
      {
	G4Material * Gas= CGAGeometryManager::GetMaterial("RPCGAS1");
        G4double dzPiece = db->fetchDouble("thickness") * mm;
	G4Material *pieceMaterial = CGAGeometryManager::GetMaterial(db->fetchString("materials"));
      if(pieceMaterial==Gas) 
	{
	  shift_in_layer+= dzPiece/2;
	  chamber_thickness=dzPiece;
	  break;
	}else{
	 shift_in_layer+= dzPiece;
        }
      }  
//    cout << " middle of gas " << shift_in_layer << endl;

/*--------------------------------------------------------------------------------*/
// Sensitive detectors 
// the cells are temoporary solution to the strips -LCIO problem  put "equivalent" cell size 
        
   muonSD*  muonBarrelSD =  new muonSD(cell_dim_x ,cell_dim_z,chamber_thickness,1,"MuonBarrel",1);
   RegisterSensitiveDetector(muonBarrelSD);

   muonSD*  muonECSD =  new muonSD(cell_dim_x ,cell_dim_z, chamber_thickness,2,"MuonEndCap",1);
   RegisterSensitiveDetector(muonECSD);
/*------------------------------------------------------------------------------*/
// Barrel part 
  
  G4Polyhedra *barrelSolid = new G4Polyhedra("YokeBarrelSolid", tiltAngle, fullAngle, symmetry,
                                      2, zPosBarrelArray, rInnerBarrelArray, rOuterBarrelArray);
  G4LogicalVolume *barrelLog = new G4LogicalVolume(barrelSolid, yokeMaterial, "YokeBarrelLog", 0, 0, 0);
  barrelLog->SetVisAttributes(yokeVisAttributes);
 
  G4double radius_low;G4double radius_mid;G4double radius_sensitive;

  for(int i=0;i<number_of_layers+1;i++)
      {

	if( i!=number_of_layers)
	  {
	   radius_low       = rInnerBarrel+ iron_thickness*(i+1) +i*layer_thickness; 
	   radius_mid       = radius_low+0.5*layer_thickness;  
	   radius_sensitive = radius_low+shift_in_layer;
	  }else{
	  // there is always the layer on the "top" of the yoke 
	  // that doesn't depend on the number of layers inside 
	   radius_low       = rOuterBarrel-layer_thickness; 
	   radius_mid       = radius_low+0.5*layer_thickness;  
	   radius_sensitive = radius_low+shift_in_layer;
	  }

	G4double dx=radius_low*tan(Angle2)-0.05*mm;  // safety margines of 0.1 mm
	G4double dy=zStartEndcap/2.0-0.05*mm;        // safety margines 
	G4Box* ChamberSolid1=new G4Box("layer", dx, layer_thickness/2.0 ,dy);
	G4Box* ChamberSolid2=new G4Box("layer1",dx, layer_thickness/2.0 ,dy);

        muonBarrelSD->AddLayer(i+1,dx,radius_sensitive,0.0); 

     for(int j=0;j<symmetry;j++)
       {

	 G4int id1=1+i+1000*(j+1)+100000*2;
	 G4int id2=1+i+1000*(j+1)+100000*3;
	 G4LogicalVolume *ChamberLogic1=BuildRPC1Box(ChamberSolid1,muonBarrelSD, id1, 
						     userLimits, db);
	 G4LogicalVolume *ChamberLogic2=BuildRPC1Box(ChamberSolid2,muonBarrelSD, id2,
						     userLimits, db);
	 G4double phirot=Angle*(j+1); 
	 G4double phirotp=phirot+90*deg;
	 muonBarrelSD->SetStaveRotationMatrix(j+1, phirot);

	 new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateZ(phirot), 
			   G4ThreeVector(radius_mid*cos(phirotp),radius_mid*sin(phirotp),
		          +zStartEndcap/2)),ChamberLogic1, "chamber", barrelLog, false, 0);
	 new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateZ(phirot),
		           G4ThreeVector(radius_mid*cos(phirotp),radius_mid*sin(phirotp),
		          -zStartEndcap/2)),ChamberLogic2, "chamber", barrelLog, false, 0);
	  }
     }

  new G4PVPlacement(0, G4ThreeVector(), barrelLog, "YokeBarrel", worldLog, false, 0);

 
/*----------------------------------------------------------------------------------------*/
// EndCap

G4Polyhedra *endcapSolid = new G4Polyhedra("YokeEndcapSolid", tiltAngle, fullAngle, symmetry, 2,
                                           zPosEndcapArray, rInnerEndcapArray, rOuterEndcapArray);
G4Polyhedra *endcapSolid1 = new G4Polyhedra("YokeEndcapSolid", tiltAngle, fullAngle, symmetry, 2,
                                           zPosEndcapArray, rInnerEndcapArray, rOuterEndcapArray);

G4LogicalVolume *endcapLog = new G4LogicalVolume(endcapSolid, yokeMaterial, "YokeEndcapLog", 0, 0, 0);
                 endcapLog->SetVisAttributes(yokeVisAttributes);

G4LogicalVolume *endcapLog1 = new G4LogicalVolume(endcapSolid1, yokeMaterial, "YokeEndcapLog", 0, 0, 0);
                 endcapLog1->SetVisAttributes(yokeVisAttributes);


      //   const G4double zPosEndcapArray[2]   = { -layer_thickness/2.0, layer_thickness/2.0 };
// 	const G4double rInnerEndcapArray[2] = { rInnerEndcap, rInnerEndcap };
// 	const G4double rOuterEndcapArray[2] = { rOuterBarrel, rOuterBarrel };

	G4double shift_middle; G4double shift_sensitive;
   for(int i=0;i<number_of_layers+1;i++)
     {
       const G4double zPosEndcapArray[2]   = { -layer_thickness/2.0, layer_thickness/2.0 };
	const G4double rInnerEndcapArray[2] = { rInnerEndcap, rInnerEndcap };
	const G4double rOuterEndcapArray[2] = { rOuterBarrel, rOuterBarrel };

     if( i!=number_of_layers)
      {
      shift_middle = -dzEndcap/2 + iron_thickness*(i+1) + (i+0.5)*layer_thickness; 
      shift_sensitive = zEndcap-dzEndcap/2 + iron_thickness*(i+1) + i*layer_thickness +shift_in_layer; 
      }else{
	  // there is also endlayer in the endcap 
       shift_middle=(dzEndcap-layer_thickness)/2; 
       shift_sensitive=zEndcap+shift_middle+shift_in_layer;
      }
       G4Polyhedra* ChamberSolid1=new G4Polyhedra("layer", tiltAngle, fullAngle, symmetry,
						  2, zPosEndcapArray, rInnerEndcapArray, 
						  rOuterEndcapArray);
       G4Polyhedra* ChamberSolid2=new G4Polyhedra("layer1", tiltAngle, fullAngle, symmetry,
						  2, zPosEndcapArray, rInnerEndcapArray, 
						  rOuterEndcapArray); 
       G4int id1=i+1+1000*(0+1)+100000*1;
       G4int id2=i+1+1000*(0+1)+100000*4;

       G4LogicalVolume *ChamberLogic1=BuildRPC1ECShape(ChamberSolid1,muonECSD, id1, 
						       userLimits, db,env);
       G4LogicalVolume *ChamberLogic2=BuildRPC1ECShape(ChamberSolid2,muonECSD, id2,
						       userLimits, db,env);

       muonECSD->AddLayer(i+1,0.0,0.0,shift_sensitive); 

       new G4PVPlacement(G4Transform3D(G4RotationMatrix(),G4ThreeVector(0,0,shift_middle)), 
			 ChamberLogic1, "chamber",  endcapLog, false, 0);

       new G4PVPlacement(G4Transform3D(G4RotationMatrix(),G4ThreeVector(0,0,shift_middle)), 
			 ChamberLogic2, "chamber",  endcapLog1, false,0);

	  }

new G4PVPlacement(0, G4ThreeVector(0, 0, +zEndcap), endcapLog, "YokeEndcap", worldLog, false, 0);
new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(180 * deg), 
		  G4ThreeVector(0, 0, -zEndcap)), endcapLog1, "YokeEndcap", worldLog, false, 0);

/*-----------------------------------------------------------------------------------*/
//  The Plugs for TESLA TDR models series  <=> LDC00_xx 
                      
  if(is_plug=="true")
     {
	
  muonSD* muonPlugSD =  new muonSD(cell_dim_x ,cell_dim_z, chamber_thickness,2,"MuonPlug",1);
  RegisterSensitiveDetector(muonPlugSD);

 const G4double HCAL_z=atof(((*Control::globalModelParameters)["calorimeter_region_zmax"]).c_str())*mm;
 
  db->exec("SELECT * FROM `muon`;");
  db->getTuple();

 const G4double HCAL_plug_gap= db->fetchDouble("Hcal_plug_gap")*mm;
 const G4double plug_thickness = zStartEndcap-HCAL_z-HCAL_plug_gap;

  HCAL_R_max=atof(((*Control::globalModelParameters)["Hcal_R_max"]).c_str())*mm;
  HCAL_R_max= HCAL_R_max*cos(tiltAngle2)-100*mm;

//   cout << "HCal_R_max "<< HCAL_R_max << endl;

 if (plug_thickness<0.0*mm || plug_thickness < 5*layer_thickness) 
  Control::Abort("Yoke03: Plug thickness negative or thinner then 5 layers of rpc ! \n Check the geometry! Sorry for abort :( ",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

 
  const G4double zPosPlugArray[2]   = { -plug_thickness/2.0, +plug_thickness/2.0 };
  const G4double rInnerPlugArray[2] = { rInnerEndcap, rInnerEndcap };
  const G4double rOuterPlugArray[2] = { HCAL_R_max, HCAL_R_max };
  
 
 G4Polyhedra *plugSolid = new G4Polyhedra("YokePlugSolid", tiltAngle, fullAngle, symmetry, 2,
					  zPosPlugArray, rInnerPlugArray, rOuterPlugArray);

 G4LogicalVolume *plugLog = new G4LogicalVolume(plugSolid, yokeMaterial, "YokePlugLog", 0, 0, 0);
 G4Polyhedra *plugSolid1 = new G4Polyhedra("YokePlugSolid1", tiltAngle, fullAngle, symmetry, 2,
					  zPosPlugArray, rInnerPlugArray, rOuterPlugArray);

 G4LogicalVolume *plugLog1 = new G4LogicalVolume(plugSolid1, yokeMaterial, "YokePlugLog1", 0, 0, 0);
 
 G4VisAttributes *plugVisAttributes = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0)); // red!
 plugVisAttributes->SetForceWireframe(true);
 plugVisAttributes->SetDaughtersInvisible(true);

 plugLog->SetVisAttributes(plugVisAttributes);
 plugLog1->SetVisAttributes(plugVisAttributes);

 // ******* plug detector  ( 5 layers ) see TESLA-tdr IV p.122

 G4double plug_step=(plug_thickness-5*layer_thickness)/6;
 

 const G4double shift_plug=zStartEndcap-plug_thickness/2.0;

   for(unsigned int i=0;i<5;i++)
      {
	const G4double zPosPlugArray[2]   = { -layer_thickness/2.0, layer_thickness/2.0};
	const G4double rInnerPlugArray[2] = { rInnerEndcap, rInnerEndcap };
	const G4double rOuterPlugArray[2] = { HCAL_R_max, HCAL_R_max };


	G4double shift =-plug_thickness / 2.0+plug_step*(i+1) +i*layer_thickness; 
	G4double shift_sensitive=shift_plug-plug_thickness / 2.0+plug_step*(i+1)
	                         +i*layer_thickness+shift_in_layer;
 
	G4Polyhedra* ChamberSolid1=new G4Polyhedra("layer", tiltAngle, fullAngle, symmetry,
						   2, zPosPlugArray, rInnerPlugArray, 
						   rOuterPlugArray);
 	G4Polyhedra* ChamberSolid2=new G4Polyhedra("layer1", tiltAngle, fullAngle, symmetry,
 						   2, zPosPlugArray, rInnerPlugArray, 
 						   rOuterPlugArray);
      
	muonPlugSD->AddLayer(i+1, 0.0,0.0,shift_sensitive);

	G4int id1=i+1+1000*(0+1)+100000*6;
	G4int id2=i+1+1000*(0+1)+100000*7;


       G4LogicalVolume *ChamberLogic1=BuildRPC1PlugShape(ChamberSolid1,muonPlugSD, id1, 
							 userLimits, db,env);
       G4LogicalVolume *ChamberLogic2=BuildRPC1PlugShape(ChamberSolid2,muonPlugSD, id2,
							 userLimits, db,env);

       new G4PVPlacement(G4Transform3D(G4RotationMatrix(),G4ThreeVector(0,0,shift)), 
			 ChamberLogic1, "chamber",  plugLog, false, 0);

       new G4PVPlacement(G4Transform3D(G4RotationMatrix(),G4ThreeVector(0,0,shift)), 
			 ChamberLogic2, "chamber",  plugLog1, false,0);
     }

 new G4PVPlacement(0, G4ThreeVector(0, 0, +shift_plug), plugLog, "YokePlug", worldLog, false, 0);
 new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateY(180 * deg), G4ThreeVector(0, 0, -shift_plug)),
		    plugLog1, "YokePlug", worldLog, false, 0);

// end of plug !
/*------------------------------------------------------------------------------------*/
     }
 
  delete db;
  return true;
}


G4LogicalVolume * 
Yoke03::BuildRPC1Box(G4Box* ChamberSolid, muonSD* theSD, G4int layer_id, G4UserLimits* pULimits, Database *db)
{

  G4Material *Gas= CGAGeometryManager::GetMaterial("RPCGAS1");
  G4Material *Air= CGAGeometryManager::GetMaterial("Air");

  if(ChamberSolid->GetEntityType()=="G4Box")
    {
     
      // fill the Chamber Envelope with air
      G4LogicalVolume *ChamberLog = new G4LogicalVolume(ChamberSolid,Air,"muonRPC1", 0, 0, 0);

      // Assembly of the layer
      G4int pieceCounter = 0;
      G4double zCursor =-ChamberSolid->GetYHalfLength() ;

      db->exec("SELECT * FROM `rpc1`;");

      while (db->getTuple()) 
      {

	const G4double dzPiece = db->fetchDouble("thickness") * mm;
	G4Material *pieceMaterial = CGAGeometryManager::GetMaterial(db->fetchString("materials"));

	G4Box *pieceSolid = new G4Box("RPCLayerPieceSolid", ChamberSolid->GetXHalfLength(), 
				      dzPiece / 2 ,ChamberSolid->GetZHalfLength());

	if(pieceMaterial==Gas) 
	  {
	    G4LogicalVolume *pieceLog = new G4LogicalVolume(pieceSolid, pieceMaterial, 
							"RPCLayerPieceLog", 0, theSD, pULimits);

	    new G4PVPlacement(0, G4ThreeVector(0, zCursor + dzPiece / 2,0), pieceLog,
			      "RPCLayerPiecegas", ChamberLog, false, layer_id);
	  pieceCounter++;
	  }else{
	G4LogicalVolume *pieceLog = new G4LogicalVolume(pieceSolid, pieceMaterial, 
							"RPCLayerPieceLog", 0, 0, 0);
	new G4PVPlacement(0, G4ThreeVector(0,  zCursor + dzPiece / 2,0), pieceLog,
			          "RPCLayerPiece", ChamberLog, false, pieceCounter);
	  pieceCounter++;
	  }


	zCursor += dzPiece;
        
	if (zCursor > +ChamberSolid->GetYHalfLength()/ 2) 
	  Control::Abort("Yoke03:Overfull RPC layer! sorry :( ",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
      }
      
      return ChamberLog;  
    }

  Control::Abort("Yoke03::BuildRPC1Box: invalid ChamberSolidEnvelope",MOKKA_OTHER_ERRORS);
  return NULL;
}


G4LogicalVolume * Yoke03::BuildRPC1ECShape(G4Polyhedra* ChamberSolid,muonSD* theSD,
					   G4int layer_id,G4UserLimits* pULimits, Database *db,
					   const CGAGeometryEnvironment &env)
  { 

  G4Material * Gas= CGAGeometryManager::GetMaterial("RPCGAS1");
  G4Material * Air= CGAGeometryManager::GetMaterial("Air");

  db->exec("SELECT * FROM `yoke`;");
  db->getTuple();
  const G4int symmetry                = db->fetchInt("symmetry");
  const G4double rOuterBarrel         = env.GetParameterAsDouble("Yoke_barrel_inner_radius")+ 
                                        env.GetParameterAsDouble("Yoke_thickness"); 
  const G4double rInnerEndcap         = env.GetParameterAsDouble("Yoke_endcap_inner_radius");
  const G4double fullAngle            = 360 * deg;
  const G4double tiltAngle            = 180 * deg / symmetry - 90 * deg; 
 
  // These arrays are needed for the G4Polyhedra constructor
   
  if(ChamberSolid->GetEntityType()=="G4Polyhedra")
    {
     
      // fill the Chamber Envelope with air
      G4LogicalVolume *ChamberLog = new G4LogicalVolume(ChamberSolid,Air,"muonRPC1", 0, 0, 0);

      // Assembly of the layer
      G4int pieceCounter = 0;
      G4double zCursor =-layer_thickness/2.0; 

      db->exec("SELECT * FROM `rpc1`;");

      while (db->getTuple()) 
      {

	const G4double dzPiece = db->fetchDouble("thickness") * mm;
	G4Material *pieceMaterial = CGAGeometryManager::GetMaterial(db->fetchString("materials"));

  const G4double zPosEndcapArray[2]   = { -dzPiece/ 2, +dzPiece / 2 };
  const G4double rInnerEndcapArray[2] = { rInnerEndcap, rInnerEndcap };
  const G4double rOuterEndcapArray[2] = { rOuterBarrel, rOuterBarrel };


  G4Polyhedra *pieceSolid = new G4Polyhedra("RPCLayerPieceSolid", tiltAngle, fullAngle, 
					    symmetry, 2, zPosEndcapArray, rInnerEndcapArray,
					    rOuterEndcapArray); 
  // G4VSolid* pieceSolid= BuildECShape(G4Polyhedra* ChamberSolid,G4double L,G4double dz)

  if(pieceMaterial==Gas) 
	  {
	    G4LogicalVolume *pieceLog = new G4LogicalVolume(pieceSolid, pieceMaterial, 
							"RPCLayerPieceLog", 0, theSD, pULimits);

	    // cout<< " set sensitive " << endl;
	    // pieceLog->SetSensitiveDetector(theSD);
	    new G4PVPlacement(0, G4ThreeVector(0,0, zCursor + dzPiece / 2), pieceLog,
			      "RPCLayerPiecegas", ChamberLog, false, layer_id);
	  pieceCounter++;
	  }else{
	G4LogicalVolume *pieceLog = new G4LogicalVolume(pieceSolid, pieceMaterial, 
							"RPCLayerPieceLog", 0, 0, 0);
	new G4PVPlacement(0, G4ThreeVector(0, 0, zCursor + dzPiece / 2), pieceLog,
			          "RPCLayerPiece", ChamberLog, false, pieceCounter);
	  pieceCounter++;
	  }


	zCursor += dzPiece;
        
	if (zCursor > layer_thickness/2.0) 
	  Control::Abort("Yoke03:Overfull RPC layer! sorry :( ",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
      }
      
      return ChamberLog;  
    }

  Control::Abort("Yoke03::BuildRPC1ECLayer: invalid ChamberSolidEnvelope",MOKKA_OTHER_ERRORS);
  return NULL;

}

G4LogicalVolume * Yoke03::BuildRPC1PlugShape(G4Polyhedra* ChamberSolid,muonSD* theSD,
					   G4int layer_id,G4UserLimits* pULimits, Database *db,
					   const CGAGeometryEnvironment &env)
  { 

  G4Material * Gas= CGAGeometryManager::GetMaterial("RPCGAS1");
  G4Material * Air= CGAGeometryManager::GetMaterial("Air");

  db->exec("SELECT * FROM `yoke`;");
  db->getTuple();

   const G4int symmetry                = db->fetchInt("symmetry");
  const G4double rInnerEndcap         = env.GetParameterAsDouble("Yoke_endcap_inner_radius");
  const G4double fullAngle            = 360 * deg;
  const G4double tiltAngle            = 180 * deg / symmetry - 90 * deg; 
 
  db->exec("SELECT * FROM `muon`;");
  db->getTuple();
 
 
 if(ChamberSolid->GetEntityType()=="G4Polyhedra")
  {
     
      // fill the Chamber Envelope with air
      G4LogicalVolume *ChamberLog = new G4LogicalVolume(ChamberSolid,Air,"muonRPC1", 0, 0, 0);

      G4int pieceCounter = 0;
      G4double zCursor =-layer_thickness/2.0; 

      db->exec("SELECT * FROM `rpc1`;");

  while (db->getTuple()) 
    {
	const G4double dzPiece = db->fetchDouble("thickness") * mm;
	G4Material *pieceMaterial = CGAGeometryManager::GetMaterial(db->fetchString("materials"));

	const G4double zPosEndcapArray[2]   = { -dzPiece/ 2, +dzPiece / 2 };
	const G4double rInnerEndcapArray[2] = { rInnerEndcap, rInnerEndcap };
	const G4double rOuterEndcapArray[2] = { HCAL_R_max, HCAL_R_max };

  G4Polyhedra *pieceSolid = new G4Polyhedra("RPCLayerPieceSolid", tiltAngle, fullAngle, 
					    symmetry, 2, zPosEndcapArray, rInnerEndcapArray,
					    rOuterEndcapArray); 
  
  if(pieceMaterial==Gas) 
     {
         G4LogicalVolume *pieceLog = new G4LogicalVolume(pieceSolid, pieceMaterial, 
						      "RPCLayerPieceLog", 0, theSD, pULimits);

	 new G4PVPlacement(0, G4ThreeVector(0,0, zCursor + dzPiece / 2), pieceLog,
			"RPCLayerPiecegas", ChamberLog, false, layer_id);
	 pieceCounter++;

     }else{

	 G4LogicalVolume *pieceLog = new G4LogicalVolume(pieceSolid, pieceMaterial, 
							   "RPCLayerPieceLog", 0, 0, 0);
	 new G4PVPlacement(0, G4ThreeVector(0, 0, zCursor + dzPiece / 2), pieceLog,
			          "RPCLayerPiece", ChamberLog, false, pieceCounter);
	 pieceCounter++;

     }

  zCursor += dzPiece;
        
  if (zCursor > layer_thickness/2.0) 
	  Control::Abort("Yoke03:Overfull RPC layer! sorry :( ",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
  
  }
      
      return ChamberLog;  
 }

  Control::Abort("Yoke03::BuildRPC1PlugLayer: invalid ChamberSolidEnvelope",MOKKA_OTHER_ERRORS);
  return NULL;
}
