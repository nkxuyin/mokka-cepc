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
// $Id: Ecal03.cc,v 1.7 2007/08/13 08:12:42 kristian Exp $
// $Name: mokka-07-00 $
//
// 
//
// Ecal03.cc
//
// History:  
// - first implementation Daisuke Yamashita (dec 99)
// - several changes (see README file), P MoraDeFreitas.

#include "MySQLWrapper.hh"
#include "Control.hh"
#include "MyPlacement.hh"
#include "G4PVReplica.hh"
#include "G4UserLimits.hh"
#include "Ecal03.hh"
#include "CGAGeometryManager.hh"
#include "G4VVisManager.hh"
#include "SD03.hh"
#include "ECSD03.hh"

#include "globals.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Torus.hh"
#include "G4Trap.hh"
#include "G4Trd.hh"
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"

#include <algorithm>

#include "CGADefs.h"

#ifdef MOKKA_GEAR
#include "gear/CalorimeterParameters.h"
#include "gearimpl/CalorimeterParametersImpl.h"
#include "gearimpl/LayerLayoutImpl.h"
#include "MokkaGear.h"
#endif


INSTANTIATE(Ecal03)

G4bool Ecal03::construct(const G4String &aSubDetectorName,
			 G4LogicalVolume *WorldLog)
{
  EnvLogEcalModuleBarrel=0;
  EnvLogEcalModuleEndCap=0;
  db=0;
  theBarrelCellSD=0;
  theBarrelGRSD=0;
  theEndCapCellSD=0;
  theEndCapGRSD=0;
  G4cout << "\nBuilding Ecal, Database name is " 
	 << aSubDetectorName << G4endl;
  
  db = new Database(aSubDetectorName.data());
  
  if(Control::DUMPG3) MyPlacement::Init("ECAL",aSubDetectorName);

  db->exec("select module_dim_x AS dim_x from endcap_standard_module;");
  db->getTuple();
  G4double ecDimX = db->fetchDouble("dim_x");
                                                                                
  db->exec("select bottom_dim_x AS xdh1 from barrel_standard_module;");
  db->getTuple();
                                                                                
  G4double barDimX = db->fetchDouble("xdh1");

  //--------- BarrelEcal Sensitive detector -----

  db->exec("select cell_dim_x,cell_dim_z,si_thickness,guard_ring_size,inter_wafer_gap,nmax_cell_x,nmax_cell_z,n_guard_ring_zones from barrel_standard_module,ecal;");
  db->getTuple();

  cell_dim_x = db->fetchDouble("cell_dim_x");
  cell_dim_z = db->fetchDouble("cell_dim_z");

  G4bool barID1Flag = true, ecID1Flag = true;
  if((barDimX/cell_dim_x) < 511)
        barID1Flag = false;
  if((ecDimX/cell_dim_x) < 511)
        ecID1Flag = false;

  si_thickness = db->fetchDouble("si_thickness");
  guard_ring_size = db->fetchDouble("guard_ring_size");
  inter_wafer_gap = db->fetchDouble("inter_wafer_gap");
  nmax_cell_x = db->fetchInt("nmax_cell_x");
  nmax_cell_z = db->fetchInt("nmax_cell_z");
  n_guard_ring_zones = db->fetchInt("n_guard_ring_zones");

  // The cell boundaries does not really exist as G4 volumes. So,
  // to avoid long steps over running  several cells, the 
  // theMaxStepAllowed inside the sensitive material is the
  // pad smaller x or z dimension.
  theMaxStepAllowed= std::min(cell_dim_x,cell_dim_z);

  // Sensitive detectors for the Ecal barrel
  // The first one is for the Silicon Cells
  theBarrelCellSD = 
    new SD03(cell_dim_x,
	   cell_dim_z,
	   si_thickness,
	   guard_ring_size,
	   inter_wafer_gap,
	   nmax_cell_x,
	   nmax_cell_z,
	   n_guard_ring_zones,
	   ECALBARREL,
	   "EcalBarrel",barID1Flag);
  RegisterSensitiveDetector(theBarrelCellSD);

  // the second one is for the Guard-Rings
  theBarrelGRSD = 
    new SD03(cell_dim_x,
	   cell_dim_z,
	   si_thickness,
	   guard_ring_size,
	   inter_wafer_gap,
	   nmax_cell_x,
	   nmax_cell_z,
	   n_guard_ring_zones,
	   ECALBARREL,
	   "EcalBarrelGuardRing",true);
  RegisterSensitiveDetector(theBarrelGRSD);

  // Sensitive detectors for the +z Ecal endcap
  // the first one is for the Silicon Cells
  theEndCapCellSD = 
    new ECSD03(cell_dim_x,
	     cell_dim_z,
	     si_thickness,
	     guard_ring_size,
	     inter_wafer_gap,
	     nmax_cell_x,
	     nmax_cell_z,
	     n_guard_ring_zones,
	     ECALENDCAPMINUS,
	     "EcalEndcap",ecID1Flag);
  RegisterSensitiveDetector(theEndCapCellSD);

  // the second one is for the Guard-Rings
  theEndCapGRSD = 
    new ECSD03(cell_dim_x,
	     cell_dim_z,
	     si_thickness,
	     guard_ring_size,
	     inter_wafer_gap,
	     nmax_cell_x,
	     nmax_cell_z,
	     n_guard_ring_zones,
	     ECALENDCAPMINUS,
	     "EcalEndcapGuardRing",true);
  RegisterSensitiveDetector(theEndCapGRSD);

  MyPlacement::InsertComment("Building Ecal");  
  //----------------------------------------------------
  // Barrel Standard Module in the air
  //----------------------------------------------------
  MyPlacement::InsertComment("Building Ecal barrel");  
  BarrelStandardModule(WorldLog);
  
  //----------------------------------------------------
  // EndCaps in the air
  //----------------------------------------------------
  
  MyPlacement::InsertComment("Building Ecal endcaps");  
  EndcapStandardModule(WorldLog);
  
  //----------------------------------------------------
  // Al Plate for the endcaps in the air 
  //----------------------------------------------------  
  
  //#####################################################
  // NO MORE AL PLATES FOR THE ECAL ENDCAPS WITH ecal02
  // ####################################################
  //MyPlacement::InsertComment("Building Ecal endcaps support plates");  
  //EndcapAlPlate(WorldLog);


#ifdef MOKKA_GEAR
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++
  // +  MOKKA GEAR                                      +
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++

  // get the information that are not yet included
  db->exec("SELECT barrel_phi_offset FROM barrel;");
  db->getTuple();
  helpBarrel.phi0 = db->fetchDouble("barrel_phi_offset") ;

  //helpBarrel.zMax = (helpBarrel.mostZ + helpBarrel.leastZ) / 2 ;
  helpBarrel.zMax = std::max( helpBarrel.mostZ , -helpBarrel.leastZ ) ;
  
  // ECAL Barrel
  gear::CalorimeterParametersImpl* barrelParam = 
    new gear::CalorimeterParametersImpl( helpBarrel.innerRadius, helpBarrel.zMax, 8, helpBarrel.phi0 );

  for (int i=0; i < helpBarrel.count; i++) {
    G4double calcThick = helpBarrel.layerPos[i+1] - helpBarrel.layerPos[i] ;

    // on last layer, gap has to be taken into account
    if( i == ( helpBarrel.count -1 ) ) {
      G4double layerGap = helpBarrel.layerPos[i] - helpBarrel.layerPos[i-1] - calcThick ;
      
      // check if layerGap makes sense
      if ( layerGap < calcThick ) {
	
	// add gap to Thickness
	calcThick = calcThick + layerGap ;
      }
    }
    
    barrelParam->layerLayout().positionLayer
      (0, calcThick, cell_dim_z, cell_dim_x, helpBarrel.radiThickness[i]);
  }
  
  // Ecal Endcap
  gear::CalorimeterParametersImpl* endcapParam = 
    new gear::CalorimeterParametersImpl( helpEndcap.innerRadius, helpEndcap.outerRadius, helpEndcap.zMax,2, helpBarrel.phi0 );

  for (int i=0; i < helpEndcap.count; i++) {

    G4double calcThick = helpEndcap.layerPos[i+1] - helpEndcap.layerPos[i] ;
    
    // on last layer, gap has to be taken into account
    if( i == ( helpEndcap.count - 1 ) ) {
      G4double layerGap = helpEndcap.layerPos[i] - helpEndcap.layerPos[i-1] - calcThick ;
      
      // check if layerGap makes sense
      if ( layerGap < calcThick ) {
	
	// add gap to Thickness
	calcThick = calcThick + layerGap ;
      }
    }

    endcapParam->layerLayout().positionLayer
      (0, calcThick, cell_dim_z, cell_dim_x, helpEndcap.radiThickness[i]);
  }
  
  
  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setEcalBarrelParameters( barrelParam ) ;
  gearMgr->setEcalEndcapParameters( endcapParam ) ;

#endif


  // Closes Database connection
  delete db;
  db = 0;
  G4cout << "Ecal done.\n" << G4endl;
  // returns OKAY
  return true;  
}

Ecal03::~Ecal03() 
{
  //  if(theBarrelSD!=0) delete theBarrelSD;
  //  if(theEndCapSD!=0) delete theEndCapSD;
  theBarrelWafersMap.clear();
  theEndcapWafersMap.clear();
}  

G4LogicalVolume * Ecal03::BuildWafer(char theDetector,
		G4int n_cell_x, G4int n_cell_z) {
	
  char key[10];
  sprintf(key, "%dby%d", n_cell_x, n_cell_z);
  G4String theKey(key);
  G4LogicalVolume * theWafer;

  std::map<G4String, G4LogicalVolume *>::iterator it;
  if(theDetector == 'b') {//it's the barrel
  	if((it = theBarrelWafersMap.find(theKey)) != theBarrelWafersMap.end())
	  return (*it).second;

  	G4Box * BarrelBox;
  	BarrelBox = new G4Box("SiWafer", 
		cell_dim_x*n_cell_x/2 + guard_ring_size,
		cell_dim_z*n_cell_z/2 + guard_ring_size,
		si_thickness/2);
    
  	theWafer = new G4LogicalVolume(BarrelBox,
			  CGAGeometryManager::GetMaterial("silicon_2.33gccm"),
			  "BarrelSiWaferLog", 
			  0, 0, 0);  
  	theWafer->SetVisAttributes(VisAttSi);
  	theWafer->SetSensitiveDetector(theBarrelGRSD);
  	FillWafer(theWafer, n_cell_x, n_cell_z, theBarrelCellSD);

	std::map<G4String, G4LogicalVolume *>::value_type
		anElement(theKey, theWafer);
	std::pair<std::map<G4String, G4LogicalVolume *>::iterator, bool> ret;
	ret = theBarrelWafersMap.insert(anElement);
	if(!ret.second) {
		G4cout << "Insertion of wafer " << theKey << 
			" in the barrel wafers map failed." << G4endl;
		Control::Abort("BuildWafer failed!",MOKKA_OTHER_ERRORS);
	}
  }
  else { //it's the endcap ('e')
  	if((it = theEndcapWafersMap.find(theKey)) != theEndcapWafersMap.end())
	  return (*it).second;

  	G4Box * EndcapBox;
  	EndcapBox = new G4Box("SiWafer", 
		cell_dim_x*n_cell_x/2 + guard_ring_size,
		cell_dim_z*n_cell_z/2 + guard_ring_size,
		si_thickness/2);
    
  	theWafer = new G4LogicalVolume(EndcapBox,
			  CGAGeometryManager::GetMaterial("silicon_2.33gccm"),
			  "EndcapSiWaferLog", 
			  0, 0, 0);  
  	theWafer->SetVisAttributes(VisAttSi);
	theWafer->SetSensitiveDetector(theEndCapGRSD);
	FillWafer(theWafer, n_cell_x, n_cell_z, theEndCapCellSD);

	std::map<G4String, G4LogicalVolume *>::value_type
		anElement(theKey, theWafer);
	std::pair<std::map<G4String, G4LogicalVolume *>::iterator, bool> ret;
	ret = theEndcapWafersMap.insert(anElement);
	if(!ret.second) {
		G4cout << "Insertion of wafer " << theKey << 
			" in the endcap wafers map failed." << G4endl;
		Control::Abort("BuildWafer failed!",MOKKA_OTHER_ERRORS);
	}
  }
  return theWafer;
}

void Ecal03::FillWafer(G4LogicalVolume * theWafer, 
		G4int n_cell_x, G4int n_cell_z,
		VSensitiveDetector*theSD) {

    G4Box * BoxSolid = new G4Box("SensitiveWafer",
		    cell_dim_x*n_cell_x/2,
		    cell_dim_z*n_cell_z/2,
		    si_thickness/2);
    G4LogicalVolume * SensWaferLogical =
	    new G4LogicalVolume(BoxSolid,
				CGAGeometryManager::GetMaterial("silicon_2.33gccm"),
				"SensitiveWaferLog",
				0, 0, 0);
    SensWaferLogical->SetVisAttributes(VisAttSi);
    new MyPlacement(0,
		G4ThreeVector(0., 0., 0.),
		SensWaferLogical,
		"SensWafferPhys",
		theWafer,
		false,0);

    G4Box * siStrip = new G4Box("SiStripSolid",
		    		cell_dim_x/2,
				cell_dim_z*n_cell_z/2,
				si_thickness/2);
    G4LogicalVolume *siStripLogical=
	        new G4LogicalVolume(siStrip,
				CGAGeometryManager::GetMaterial("silicon_2.33gccm"),
				"SiStripLogical",
				0, 0, 0);
    siStripLogical->SetVisAttributes(VisAttSi);
    new G4PVReplica("SiStrips",
		siStripLogical,
		SensWaferLogical,
		kXAxis,
		n_cell_x,
		cell_dim_x,
		0);

    G4Box *siCell  = new G4Box("SiCellSolid",
		    		cell_dim_x/2,
		    		cell_dim_z/2,
		    		si_thickness/2);
    G4LogicalVolume *siCellLogical=
	    	new G4LogicalVolume(siCell,
			CGAGeometryManager::GetMaterial("silicon_2.33gccm"),
			"SiCellLogical",
			0, 0, 0);
    siCellLogical->SetVisAttributes(VisAttSi);
    siCellLogical->SetSensitiveDetector(theSD);

    new G4PVReplica("SiCells",
		siCellLogical,
		siStripLogical,
		kYAxis,
		n_cell_z,
		cell_dim_z,
		0);
}

void Ecal03::FillBarrelPlane(G4LogicalVolume * thePlane, 
		G4double dimx, G4double dimz){

  G4int nFullWafersZ = (G4int)(dimz/
	  (cell_dim_z*nmax_cell_z+2*guard_ring_size+inter_wafer_gap));
  		                  
  G4double dimzLastWafer = dimz-nFullWafersZ*(cell_dim_z*nmax_cell_z+
		  2*guard_ring_size+inter_wafer_gap);

  G4int nCellsZInLastWafer = (G4int)((dimzLastWafer-2*guard_ring_size)/
		  cell_dim_z);

  G4int nFullWafersX = (G4int)(dimx/
	  (cell_dim_x*nmax_cell_x+2*guard_ring_size+inter_wafer_gap));
  
  G4double dimxForLastWafer = dimx-nFullWafersX*(cell_dim_x*nmax_cell_x+
		  2*guard_ring_size+inter_wafer_gap);

  G4int nCellsXInLastWafer = (G4int)((dimxForLastWafer-2*guard_ring_size)/
		  cell_dim_x);

  G4LogicalVolume * FullWafer = BuildWafer('b', nmax_cell_x, nmax_cell_z);

  G4RotationMatrix *rot=new G4RotationMatrix();
  rot->rotateX(pi); // on couche le module.

  for(G4int iX=0; iX<nFullWafersX; iX++) {

	G4double posX = -dimx/2 + (cell_dim_x*nmax_cell_x/2+guard_ring_size) +
	    iX*(cell_dim_x*nmax_cell_x+2*guard_ring_size+inter_wafer_gap);
		
	for(G4int iZ=0; iZ<nFullWafersZ; iZ++) {

		G4double posZ = dimz/2 - (cell_dim_z*nmax_cell_z/2 +
			guard_ring_size)-iZ*(cell_dim_z*nmax_cell_z+
			2*guard_ring_size+inter_wafer_gap);

    		new MyPlacement(rot,
		    G4ThreeVector(posX, posZ, 0),
		    FullWafer,
		    "FullWafer",
		    thePlane,
		    false,iX*100000+iZ*1000+nmax_cell_x*100+nmax_cell_z*10);

	}

  }

  if(nCellsZInLastWafer>0) {

    G4LogicalVolume * ZSmallWafer = BuildWafer('b', nmax_cell_x, 
		  				nCellsZInLastWafer);

    G4double posZ = dimz/2 - (cell_dim_z*nCellsZInLastWafer/2 +
			guard_ring_size)-nFullWafersZ*(cell_dim_z*nmax_cell_z+
			2*guard_ring_size+inter_wafer_gap);

    for(G4int iX=0; iX<nFullWafersX; iX++) {

	G4double posX = -dimx/2 + (cell_dim_x*nmax_cell_x/2+guard_ring_size) +
	    iX*(cell_dim_x*nmax_cell_x+2*guard_ring_size+inter_wafer_gap);
		
    	new MyPlacement(rot,
		    G4ThreeVector(posX, posZ, 0),
		    ZSmallWafer,
		    "ZSmallWafer",
		    thePlane,
		    false,iX*100000+nFullWafersZ*1000+nmax_cell_x*100+
		    nCellsZInLastWafer*10);
    }
  }


  if(nCellsXInLastWafer>0) {

  	G4LogicalVolume * XSmallWafer = BuildWafer('b', nCellsXInLastWafer, 
		  				nmax_cell_z);
  
 	G4double posX = -dimx/2 + (cell_dim_x*nCellsXInLastWafer/2+
			 guard_ring_size) + nFullWafersX*(cell_dim_x*nmax_cell_x
			 +2*guard_ring_size+inter_wafer_gap);

  	for(G4int iZ=0; iZ<nFullWafersZ; iZ++) {

		G4double posZ = dimz/2 - (cell_dim_z*nmax_cell_z/2 +
			guard_ring_size)-iZ*(cell_dim_z*nmax_cell_z+
			2*guard_ring_size+inter_wafer_gap);

    		new MyPlacement(rot,
		    G4ThreeVector(posX, posZ, 0),
		    XSmallWafer,
		    "XSmallWafer",
		    thePlane,
		    false,nFullWafersX*100000+iZ*1000+nCellsXInLastWafer*100+
		    nmax_cell_z*10);

  	}

	if(nCellsZInLastWafer>0){

  		G4LogicalVolume * CornerWafer = BuildWafer('b', 
						nCellsXInLastWafer,
		  				nCellsZInLastWafer);

  		G4double posZ = dimz/2 - (cell_dim_z*nCellsZInLastWafer/2 +
			guard_ring_size)-nFullWafersZ*(cell_dim_z*nmax_cell_z+
			2*guard_ring_size+inter_wafer_gap);

  		new MyPlacement(rot,
		    G4ThreeVector(posX, posZ, 0),
		    CornerWafer,
		    "CornerWafer",
		    thePlane,
		    false,nFullWafersX*100000+nFullWafersZ*1000+
		    nCellsXInLastWafer*100+nCellsZInLastWafer*10);
	}
  }
}

void Ecal03::FillEndcapPlane(G4LogicalVolume * thePlane, G4double L, 
		                  G4double dimx, G4double dimy){

  G4double waferDimY=cell_dim_z*nmax_cell_z+2*guard_ring_size;
  G4double waferDimX=cell_dim_x*nmax_cell_x+2*guard_ring_size;

  G4int nFullWafersX = (G4int)(dimx/(waferDimX+inter_wafer_gap));
  
  G4double dimxForLastWafer = dimx-nFullWafersX*(waferDimX+inter_wafer_gap);

  G4int nCellsXInLastWafer = (G4int)((dimxForLastWafer-2*guard_ring_size)/
		  		cell_dim_x);

  G4int nFullWafersY = (G4int)((dimy-L)/(waferDimY+inter_wafer_gap));
  		                  
  G4LogicalVolume * FullWafer = BuildWafer('e', nmax_cell_x, nmax_cell_z);

  for(G4int iY=0; iY<nFullWafersY; iY++) {

	G4double posY = -(dimy-L)/2+(waferDimY/2)
			+iY*(waferDimY+inter_wafer_gap);

  	for(G4int iX=0; iX<nFullWafersX; iX++) {

		G4double posX = -dimx/2+(waferDimX/2)
				+iX*(waferDimX+inter_wafer_gap);
		
    		new MyPlacement(0,
		    G4ThreeVector(posX, posY, 0),
		    FullWafer,
		    "FullWafer",
		    thePlane,
		    false,iX*100000+iY*1000+nmax_cell_x*100+nmax_cell_z*10);

	}

  }

  if(nCellsXInLastWafer>0) {

  	G4LogicalVolume * XSmallWafer = BuildWafer('e', nCellsXInLastWafer, 
		  				nmax_cell_z); 

	G4double xSmallWaferDimX = nCellsXInLastWafer*cell_dim_x+
					2*guard_ring_size;

    	G4double posX = -dimx/2+(xSmallWaferDimX/2)
			+nFullWafersX*(waferDimX+inter_wafer_gap);

  	for(G4int iY=0; iY<nFullWafersY; iY++) {

		G4double posY = -(dimy-L)/2+(waferDimY/2)
				+iY*(waferDimY+inter_wafer_gap);

		new MyPlacement(0,
		    G4ThreeVector(posX, posY, 0),
		    XSmallWafer,
		    "XSmallWafer",
		    thePlane,
		    false,nFullWafersX*100000+iY*1000+nCellsXInLastWafer*100+
		    nmax_cell_z*10);
	}
  }
//here is the first line of the cut
  G4double posY = -(dimy-L)/2+(waferDimY/2)
			+nFullWafersY*(waferDimY+inter_wafer_gap);

  for(G4int iX=0; iX<nFullWafersX-1; iX++) {

	G4double posX = -dimx/2+(waferDimX/2)
				+iX*(waferDimX+inter_wafer_gap);
		
    	new MyPlacement(0,
		    G4ThreeVector(posX, posY, 0),
		    FullWafer,
		    "FullWafer",
		    thePlane,
		    false,iX*100000+nFullWafersY*1000+nmax_cell_x*100
		    +nmax_cell_z*10);

  }

  G4double dimyForLastWafer = dimy-L-nFullWafersY*(waferDimY+inter_wafer_gap)+
	  	dimxForLastWafer+inter_wafer_gap; //tan(pi/4)=1;

  G4int nCellsYInLastWafer = (G4int)((dimyForLastWafer-2*guard_ring_size)/
		  cell_dim_z);

  if(nCellsYInLastWafer>0) {
	  
  	G4LogicalVolume * YSmallWafer = BuildWafer('e', nmax_cell_x, 
		  			nCellsYInLastWafer); 

    	G4double posX = -dimx/2+(waferDimX/2)
			+(nFullWafersX-1)*(waferDimX+inter_wafer_gap);

	G4double ySmallWaferDimY = nCellsYInLastWafer*cell_dim_z+
					2*guard_ring_size;

	G4double posY = -(dimy-L)/2+(ySmallWaferDimY/2)
				+nFullWafersY*(waferDimY+inter_wafer_gap);

	new MyPlacement(0,
		    G4ThreeVector(posX, posY, 0),
		    YSmallWafer,
		    "YSmallWafer",
		    thePlane,
		    false,(nFullWafersX-1)*100000+nFullWafersY*1000
		    +nmax_cell_x*100+nCellsYInLastWafer*10);

  	if(nCellsXInLastWafer>0) {

  		G4double dimyForLastWafer = dimy-L-nFullWafersY*
			(waferDimY+inter_wafer_gap);

  		G4int nCellsYInLastWafer = (G4int)((dimyForLastWafer-
					2*guard_ring_size)/cell_dim_z);

		G4double ySmallWaferDimY = nCellsYInLastWafer*cell_dim_z+
					2*guard_ring_size;

		G4double posY = -(dimy-L)/2+(ySmallWaferDimY/2)
				+nFullWafersY*(waferDimY+inter_wafer_gap);

	if(nCellsYInLastWafer>0){
  		G4LogicalVolume *CornerWafer=BuildWafer('e',nCellsXInLastWafer, 
		  				nCellsYInLastWafer); 

		G4double xSmallWaferDimX = nCellsXInLastWafer*cell_dim_x+
					2*guard_ring_size;

    		G4double posX = -dimx/2+(xSmallWaferDimX/2)
			+nFullWafersX*(waferDimX+inter_wafer_gap);

		new MyPlacement(0,
		    G4ThreeVector(posX, posY, 0),
		    CornerWafer,
		    "CornerWafer",
		    thePlane,
		    false,nFullWafersX*100000+nFullWafersY*1000
		    +nCellsXInLastWafer*100+nCellsYInLastWafer*10);

	}
	}
  }

//from here on is the cut
  G4double partialDimY = (nFullWafersY+1)*(waferDimY+inter_wafer_gap)
	  		+waferDimY;
  G4int k;
  for(k=2; (partialDimY=(nFullWafersY+k-1)*(waferDimY+inter_wafer_gap)
	   +waferDimY) <= dimy; k++) {

	G4double partialDimX = dimx-(partialDimY-dimy+L);
					//tan(pi/4)=1
  	G4int nFullWafersX = (G4int)(partialDimX/(waferDimX+inter_wafer_gap));
	
	G4double restXForLastWafer = partialDimX-nFullWafersX*
	                                         (waferDimX+inter_wafer_gap);

  	G4int nCellsXInLastWafer=
		(waferDimX>=waferDimY)?nmax_cell_z:nmax_cell_x;

  	G4double dimxForLastWafer = nCellsXInLastWafer*cell_dim_x+
					2*guard_ring_size;

  	G4double dimyForLastWafer = waferDimY-(dimxForLastWafer
					       -restXForLastWafer); 
						//tan(pi/4)=1;

  	G4int nCellsYInLastWafer = (G4int)((dimyForLastWafer-2*guard_ring_size)/
		  cell_dim_z);

	G4double ySmallWaferDimY = nCellsYInLastWafer*cell_dim_z+
					2*guard_ring_size;

  	G4double posY = -(dimy-L)/2+(waferDimY/2)
			+(nFullWafersY+k-1)*(waferDimY+inter_wafer_gap);

  	for(G4int iX=0; iX<nFullWafersX; iX++) {

		G4double posX = -dimx/2+(waferDimX/2)
				+iX*(waferDimX+inter_wafer_gap);
		
    		new MyPlacement(0,
		    G4ThreeVector(posX, posY, 0),
		    FullWafer,
		    "FullWafer",
		    thePlane,
		    false,iX*100000+(nFullWafersY+k-1)*1000+nmax_cell_x*100
		    +nmax_cell_z*10);

  	}

	if(nCellsYInLastWafer>0){

		G4double posY = -(dimy-L)/2+(ySmallWaferDimY/2)
				+(nFullWafersY+k-1)*(waferDimY+inter_wafer_gap);

  		G4LogicalVolume *CornerWafer=BuildWafer('e',nCellsXInLastWafer, 
		  				nCellsYInLastWafer); 

    		G4double posX = -dimx/2+(dimxForLastWafer/2)
			+nFullWafersX*(waferDimX+inter_wafer_gap);

		new MyPlacement(0,
		    G4ThreeVector(posX, posY, 0),
		    CornerWafer,
		    "CornerWafer",
		    thePlane,
		    false,nFullWafersX*100000+(nFullWafersY+k-1)*1000
		    +nCellsXInLastWafer*100+nCellsYInLastWafer*10);

	}
  }

// here comes the last line, at the top
  partialDimY = (nFullWafersY+k-1)*(waferDimY+inter_wafer_gap);

  nCellsYInLastWafer=(G4int)((dimy-partialDimY-2*guard_ring_size)/cell_dim_z);

  if(nCellsYInLastWafer>0) {
	  
	G4double partialDimX = dimx-(partialDimY-dimy+L);
					//tan(pi/4)=1
  	G4int nFullWafersX = (G4int)(partialDimX/(waferDimX+inter_wafer_gap));

	G4double restXforLastWafer = partialDimX - nFullWafersX*(waferDimX+inter_wafer_gap);

	G4double ySmallWaferDimY = nCellsYInLastWafer*cell_dim_z+
					2*guard_ring_size;

  	G4double posY = -(dimy-L)/2+(ySmallWaferDimY/2)+partialDimY;

  	G4LogicalVolume * YSmallWafer = BuildWafer('e', nmax_cell_x, 
		  			nCellsYInLastWafer); 

  	for(G4int iX=0; iX<nFullWafersX; iX++) {

		G4double posX = -dimx/2+(waferDimX/2)
				+iX*(waferDimX+inter_wafer_gap);
		
    		new MyPlacement(0,
		    G4ThreeVector(posX, posY, 0),
		    YSmallWafer,
		    "YSmallWafer",
		    thePlane,
		    false,iX*100000+(nFullWafersY+k-1)*1000+nmax_cell_x*100
		    +nCellsYInLastWafer*10);

  	}

  	G4int nCellsXInLastWafer=
		(waferDimX>=waferDimY)?(G4int)(nmax_cell_z/2):(G4int)(nmax_cell_x/2);

  	G4double dimxForLastWafer = nCellsXInLastWafer*cell_dim_x+
					2*guard_ring_size;

  	G4double dimyForLastWafer = restXforLastWafer-dimxForLastWafer-inter_wafer_gap;
 						//tan(pi/4)=1;

  	G4int newNCellsYInLastWafer=(G4int)((dimyForLastWafer-
				2*guard_ring_size)/cell_dim_z);

	if(newNCellsYInLastWafer>0){

		ySmallWaferDimY = newNCellsYInLastWafer*cell_dim_z+
					2*guard_ring_size;

		G4double posY = -(dimy-L)/2+(ySmallWaferDimY/2)+partialDimY;

  		G4LogicalVolume *CornerWafer=BuildWafer('e',nCellsXInLastWafer, 
		  				newNCellsYInLastWafer); 

    		G4double posX = -dimx/2+(dimxForLastWafer/2)
			+nFullWafersX*(waferDimX+inter_wafer_gap);
		
		new MyPlacement(0,
		    G4ThreeVector(posX, posY, 0),
		    CornerWafer,
		    "CornerWafer",
		    thePlane,
		    false,nFullWafersX*100000+(nFullWafersY+k-1)*1000
		    +nCellsXInLastWafer*100+newNCellsYInLastWafer*10);
	}
  }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              BarrelStandardModule                 ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Ecal03::BarrelStandardModule(G4LogicalVolume* MotherLog)
{

  db->exec("select bottom_dim_x/2 AS xdh1,top_dim_x/2 AS xdh2, module_dim_y/2. AS ydh,module_dim_z/2. AS zdh from barrel_standard_module;");
  db->getTuple();

  // Attention: on bâtit le module dans la verticale
  // à cause du G4Trd et on le tourne avant de le positioner

  G4Trd * MyTrd = new G4Trd("Barrel_Module",
			    db->fetchDouble("xdh1"), 
			    db->fetchDouble("xdh2"),
			    db->fetchDouble("zdh"),
			    db->fetchDouble("zdh"),
			    db->fetchDouble("ydh"));

  EnvLogEcalModuleBarrel  = new G4LogicalVolume(MyTrd,
			CGAGeometryManager::GetMaterial("g10"),
						"EnvLog", 
						0, 0, 0);
  //G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(.5,.7,.1))
  G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(1.,1.,0));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  VisAtt->SetDaughtersInvisible(true);

  EnvLogEcalModuleBarrel->SetVisAttributes(VisAtt);
  
  //----------------------------------------------------
  // W_Plate in the Barrel Envelop Ecal 
  //----------------------------------------------------
  MyPlacement::InsertComment("Ecal Barrel W Plates");
  BarrelWPlate(EnvLogEcalModuleBarrel);

  //----------------------------------------------------
  // EnvelopeAlveolus in the Barrel Envelop Ecal 
  //----------------------------------------------------

  MyPlacement::InsertComment("Ecal Barrel Alveolus");
  BarrelAlveolusModule(EnvLogEcalModuleBarrel);

  // BarrelStandardModule placements
  db->exec("select stave_id,module_id,stave_phi_offset,module_x_offset,module_y_offset,module_z_offset from barrel_stave, barrel_standard_module, barrel_modules;");
  db->getTuple();
  G4double X,Y;
  X = db->fetchDouble("module_x_offset");
  Y = db->fetchDouble("module_y_offset");

#ifdef MOKKA_GEAR
  // set first radius
  helpBarrel.innerRadius = X*X + Y*Y ;

  // set last layer position
  helpBarrel.layerPos.push_back( MyTrd->GetZHalfLength() ) ;
#endif
  
  do {
    G4double phirot = db->fetchDouble("stave_phi_offset")*pi/180;
    G4RotationMatrix *rot=new G4RotationMatrix();
    rot->rotateX(pi*0.5); // on couche le module.
    rot->rotateY(phirot);
    new MyPlacement(rot,
		    G4ThreeVector(X*cos(phirot)-Y*sin(phirot),
				  X*sin(phirot)+Y*cos(phirot),
				  db->fetchDouble("module_z_offset")),
		    EnvLogEcalModuleBarrel,
		    "BarrelEcalModule",
		    MotherLog,
		    false,
		    ECALBARREL*100+db->fetchInt("stave_id")*10+
		    db->fetchInt("module_id"));
    theBarrelCellSD->SetStaveRotationMatrix(db->fetchInt("stave_id"),phirot);
    theBarrelCellSD->
      SetModuleZOffset(db->fetchInt("module_id"),
		       db->fetchDouble("module_z_offset"));
    theBarrelGRSD->SetStaveRotationMatrix(db->fetchInt("stave_id"),phirot);
    theBarrelGRSD->
      SetModuleZOffset(db->fetchInt("module_id"),
		       db->fetchDouble("module_z_offset"));

#ifdef MOKKA_GEAR
    // find out most and least extensions in z
    // take offset and add/subtract dimension of trapezoid
    // attention z<->y
    G4double Z = db->fetchDouble( "module_z_offset" ) ;
    helpBarrel.leastZ = std::min( helpBarrel.leastZ, Z - MyTrd->GetYHalfLength1() );
    helpBarrel.mostZ  = std::max( helpBarrel.mostZ , Z + MyTrd->GetYHalfLength1() );
     
    // get innerRadius as minimun of all occurend inner radius
    // helf heigth of module
    G4double moduleHeigth = MyTrd->GetZHalfLength() ;
    G4double radius = std::sqrt( X*X + Y*Y );
    helpBarrel.innerRadius = std::min( helpBarrel.innerRadius, radius - moduleHeigth );
#endif

  } while(db->getTuple()!=NULL);
}

void Ecal03::BarrelWPlate(G4LogicalVolume* MotherLog)
{
  G4LogicalVolume * BoxLogical[200];
  
  // Shapes
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(.7,.7,.9));
  VisAtt->SetForceWireframe(true);
  VisAtt->SetVisibility(true);
  db->exec("select barrel_w_plate_shape.shape_id,w_dim_x/2. AS xdh,w_dim_y/2. AS ydh,w_dim_z/2. AS zdh FROM barrel_w_plate_shape,barrel_w_plate,barrel_layer,barrel_standard_module where  barrel_layer.layer_id = barrel_w_plate.layer_id and barrel_w_plate.shape_id = barrel_w_plate_shape.shape_id;");
  while(db->getTuple()!=NULL){
    G4Box *box = 
      new G4Box("WPlateSolid", 
		db->fetchDouble("xdh"),
		db->fetchDouble("zdh"),  // !!!
		db->fetchDouble("ydh")); //attention!
    BoxLogical[db->fetchInt("shape_id")]= 
      new G4LogicalVolume(box,
			  CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
			  "WPlateLogical", 
			  0, 0, 0);  
    BoxLogical[db->fetchInt("shape_id")]->SetVisAttributes(VisAtt);
  }
  // Placements
  //
  db->exec("select w_x_offset,w_y_offset,w_z_offset,barrel_w_plate.shape_id FROM barrel_layer,barrel_w_plate WHERE  barrel_layer.layer_id = barrel_w_plate.layer_id;");
  while(db->getTuple()!=NULL) {
    G4int shapeId = db->fetchInt("shape_id") ; // added by RL for convinience
    new MyPlacement(0,
		    G4ThreeVector(db->fetchDouble("w_x_offset"),
				  db->fetchDouble("w_z_offset"), //!!
				  db->fetchDouble("w_y_offset")),//!!
		    //!!attention!! y<->z
		    BoxLogical[shapeId],  // subst fetchInt by shapeID, RL
		    "BarrelW",
		    MotherLog,false,0);

#ifdef MOKKA_GEAR
    // get positions of Layer as offset + half size of box (upper corner)
    
    G4Box* box = dynamic_cast<G4Box *>( BoxLogical[shapeId]->GetSolid() ) ;
    
    // check if dynamic_cast was successfull
    if( box== 0 ) {
      Control::Abort( "BarrelWPlates are expected to be of type G4Box \nerror in 'Ecal02::BarrelWPlate' construction MokkaGear." ,MOKKA_OTHER_ERRORS) ;
    }

    // get lower Part of W-Plate as first layer pos
    helpBarrel.layerPos.push_back( db->fetchDouble("w_y_offset") - box->GetZHalfLength()  ) ;
    // get radiator thickness as W-Plate Thickness
    helpBarrel.radiThickness.push_back( box->GetZHalfLength() * 2 ) ;
    // count layers
    helpBarrel.count ++ ;
#endif

  } // close loop
}

void Ecal03::BarrelAlveolusModule(G4LogicalVolume* MotherLog)
{

  G4LogicalVolume * EnvLogAlve[200];
  G4LogicalVolume * EnvLogPCB[200];
  G4LogicalVolume * EnvLogSi[200];
  G4Box * BoxSolid;

  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(.2,.8,.2));
  VisAtt->SetForceWireframe(true);
  VisAtt->SetVisibility(true);
  db->exec("select layer_id,alveolus_dim_x/2. AS xdh,alveolus_dim_y/2. AS ydh,alveolus_dim_z/2. AS zdh from barrel_layer,barrel_standard_module;");

  while(db->getTuple()!=NULL){
    BoxSolid = 
      new G4Box("AlveolusSolid", 
		db->fetchDouble("xdh"),  //hx
		db->fetchDouble("zdh"),  //hz attention!
		db->fetchDouble("ydh")); //hy attention!
    
    EnvLogAlve [db->fetchInt("layer_id")] = 
      new G4LogicalVolume(BoxSolid,
			  CGAGeometryManager::GetMaterial("air"),
			  "AlveolusLogical", 
			  0, 0, 0);  
    EnvLogAlve[db->fetchInt("layer_id")]->SetVisAttributes(VisAtt);
  }
  
  // PCB Logicals
  G4VisAttributes *VisAttPCB = new G4VisAttributes(G4Colour(.8,.2,.8));
  VisAttPCB->SetForceWireframe(true);
  VisAtt->SetVisibility(true);

  db->exec("select layer_id,pads_dim_x/2. AS xdh,pcb_thickness/2. AS ydh,alveolus_dim_z/2. AS zdh from barrel_layer,ecal,barrel_standard_module;");

  while(db->getTuple()!=NULL){
    BoxSolid = 
      new G4Box("PCBSolid", 
		db->fetchDouble("xdh"),  //hx
		db->fetchDouble("zdh"),  //hz attention!
		db->fetchDouble("ydh")); //hy attention!

    EnvLogPCB[db->fetchInt("layer_id")] = 
      new G4LogicalVolume(BoxSolid,
			  CGAGeometryManager::GetMaterial("g10"),
			  "PCBLog", 
			  0, 0, 0);  
    EnvLogPCB[db->fetchInt("layer_id")]->SetVisAttributes(VisAttPCB);
  }

  // Si Logicals
  VisAttSi = new G4VisAttributes(G4Colour(.8,.8,.2));
  VisAttSi->SetForceWireframe(true);
  VisAtt->SetVisibility(true);

  db->exec("select layer_id,pads_dim_x/2. AS xdh,si_thickness/2. AS ydh,alveolus_dim_z/2. AS zdh from barrel_layer,ecal,barrel_standard_module;");

  /*
  G4UserLimits* pULimits=
    new G4UserLimits(theMaxStepAllowed);
  */

  while(db->getTuple()!=NULL){

    G4int layer_id = db->fetchInt("layer_id");
    G4double xdh = db->fetchDouble("xdh");
    G4double zdh = db->fetchDouble("zdh");

    BoxSolid = 
      new G4Box("SiSolid", 
		xdh,  //hx
		zdh,  //hz attention!
		db->fetchDouble("ydh")); //hy attention!
    
    EnvLogSi[layer_id]  = 
      new G4LogicalVolume(BoxSolid,
			  CGAGeometryManager::GetMaterial("silicon_2.33gccm"),
			  "SiLog", 
			  0, 0, 0);  
    EnvLogSi[layer_id]->SetVisAttributes(VisAttSi);

    FillBarrelPlane(EnvLogSi[layer_id], 2*xdh, 2*zdh);
  }
  
  
  // PCB Placements
  // lower layer
  db->exec("select layer_id,pads_x_offset AS x_offset,-(si_thickness+pcb_thickness)/2. AS y_offset,alveolus_z_offset from barrel_layer,ecal,barrel_alveolus;");
  
  MyPlacement::InsertComment("Ecal Barrel G10 PCB inside Alveolus");
  while(db->getTuple()!=NULL)
    new MyPlacement(0,
		    G4ThreeVector(db->fetchDouble("x_offset"),
				  db->fetchDouble("alveolus_z_offset"),//
				  db->fetchDouble("y_offset")),//
		    EnvLogPCB[db->fetchInt("layer_id")],
		    "UnderPCB",
		    EnvLogAlve[db->fetchInt("layer_id")],
		    false,0);

  // upper layer
  db->exec("select layer_id,pads_x_offset AS x_offset,(si_thickness+pcb_thickness)/2. AS y_offset,alveolus_z_offset from barrel_layer,ecal,barrel_alveolus;");
  
  while(db->getTuple()!=NULL)
    new MyPlacement(0,
		    G4ThreeVector(db->fetchDouble("x_offset"),
				  db->fetchDouble("alveolus_z_offset"),//
				  db->fetchDouble("y_offset")),//
		    EnvLogPCB[db->fetchInt("layer_id")],
		    "UpperPCB",
		    EnvLogAlve[db->fetchInt("layer_id")],
		    false,0);
  
  // Si Placements
  // standard module x and y offsets (needed for the SD)
  db->exec("select module_x_offset, module_y_offset from barrel_standard_module;");
  db->getTuple();
  
  G4double Xoff,Yoff;
  Xoff = db->fetchDouble("module_x_offset");
  Yoff = db->fetchDouble("module_y_offset");
  
  db->exec("select layer_id,pads_x_offset AS x_offset,0 AS y_offset,alveolus_z_offset,pads_dim_x/2. AS xdh,alveolus_dim_z/2. AS zdh from barrel_layer,barrel_alveolus,ecal,barrel_standard_module;");
  
  MyPlacement::InsertComment("Ecal Barrel Silicium sensitive detector inside Alveolus");
  while(db->getTuple()!=NULL){
    G4int layer_id = db->fetchInt("layer_id");
    new MyPlacement(0,
		    G4ThreeVector(db->fetchDouble("x_offset"),
				  db->fetchDouble("alveolus_z_offset"),//
				  db->fetchDouble("y_offset")),         //
		    EnvLogSi[layer_id],
		    "SiBarrel",
		    EnvLogAlve[layer_id],
		    false,layer_id);
    
    theBarrelCellSD->
      AddLayer(layer_id,
	       db->fetchDouble("x_offset") + Xoff - 
	       ((G4Box *)EnvLogSi[layer_id]->GetSolid())->GetXHalfLength(),
	       db->fetchDouble("y_offset") + Yoff,
	       // - ((G4Box *)EnvLogSi[layer_id]->GetSolid())->GetZHalfLength(),
	       db->fetchDouble("alveolus_z_offset") - 
	       ((G4Box *)EnvLogSi[layer_id]->GetSolid())->GetYHalfLength());

    theBarrelGRSD->
      AddLayer(layer_id,
	       db->fetchDouble("x_offset") + Xoff - 
	       ((G4Box *)EnvLogSi[layer_id]->GetSolid())->GetXHalfLength(),
	       db->fetchDouble("y_offset") + Yoff,
	       // - ((G4Box *)EnvLogSi[layer_id]->GetSolid())->GetZHalfLength(),
	       db->fetchDouble("alveolus_z_offset") - 
	       ((G4Box *)EnvLogSi[layer_id]->GetSolid())->GetYHalfLength());
  }

  // Alveolus Placements
  db->exec("select layer_id,alveolus_x_offset,alveolus_y_offset,alveolus_z_offset from barrel_layer,barrel_alveolus,barrel_standard_module;");

  while(db->getTuple()!=NULL){    
    G4int layer_id = db->fetchInt("layer_id");
    new MyPlacement(0,
		    G4ThreeVector(db->fetchDouble("alveolus_x_offset"),
				  db->fetchDouble("alveolus_z_offset"),
				  db->fetchDouble("alveolus_y_offset")),
		    //!!attention!! y<->z
		    EnvLogAlve [layer_id],
		    "AlveolusBarrel",
		    MotherLog,false,0);
    theBarrelCellSD->
      AddLayerOffset(layer_id,
		     db->fetchDouble("alveolus_x_offset"),
		     db->fetchDouble("alveolus_y_offset"),
		     db->fetchDouble("alveolus_z_offset"));    

    theBarrelGRSD->
      AddLayerOffset(layer_id,
		     db->fetchDouble("alveolus_x_offset"),
		     db->fetchDouble("alveolus_y_offset"),
		     db->fetchDouble("alveolus_z_offset"));   

    /*#ifdef MOKKA_GEAR
    // get positions of Layer as offset + half size of box (upper corner)
    
    G4Box* box = dynamic_cast<G4Box *>( EnvLogAlve[layer_id]->GetSolid() ) ;
    
    // check if dynamic_cast was successfull
    if( box== 0 ) {
      Control::Abort( "BarrelAlveolus are expected to be of type G4Box \nerror in 'Ecal02::BarrelWPlate' construction MokkaGear." ) ;
    }

    // helpBarrel.layerPos.push_back( db->fetchDouble("alveolus_y_offset") + box->GetZHalfLength() ) ;
    #endif*/


 
  }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~              EndcapStandardModule                 ~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Ecal03::EndcapStandardModule(G4LogicalVolume* MotherLog)
{
  // Cut Shape L
  db->exec("select cut_shape_h AS H from endcap_standard_module;");
  db->getTuple();
  G4double L = db->fetchDouble("H")/cos(pi/4);

  // Box to be cutted
  db->exec("select module_dim_x AS dim_x, module_dim_y AS dim_y, module_dim_z AS dim_z from endcap_standard_module;");
  db->getTuple();

  // builds the endcap shape
  G4VSolid* CapModule = 
    BuildECShape(L,
		 db->fetchDouble("dim_x"),
		 db->fetchDouble("dim_y"),
		 db->fetchDouble("dim_z"));
  
  EnvLogEcalModuleEndCap  = new G4LogicalVolume(CapModule,
			CGAGeometryManager::GetMaterial("g10"),
						"EndCapLog", 
						0, 0, 0);
  G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(.1,.6,.1));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  VisAtt->SetDaughtersInvisible(true);
  EnvLogEcalModuleEndCap->SetVisAttributes(VisAtt);
  
#ifdef MOKKA_GEAR
  // retrieve ending value for layersposition
  G4double lastLayerPos = db->fetchDouble("dim_z")/2  ;
#endif

  //----------------------------------------------------
  // W_Plate in the EndCap Envelop Ecal 
  //----------------------------------------------------
  
  MyPlacement::InsertComment("Ecal endcaps W plates");
  EndcapWPlate(EnvLogEcalModuleEndCap);  

  //----------------------------------------------------
  // EnvelopeAlveolus in the EndCapEnvelop Ecal 
  //----------------------------------------------------
  
  MyPlacement::InsertComment("Ecal endcaps Alveolus");
  EndcapAlveolusModule(EnvLogEcalModuleEndCap);

#ifdef MOKKA_GEAR
  // set ending value for layersposition
  helpEndcap.layerPos.push_back( lastLayerPos ) ;
#endif

  // EndCap module Placements
  G4int irot;
  G4double phirot;
  
  db->exec("select al_plates_center_dim_xy AS xcenter, modules_gap AS Gap, module_dim_y AS dim_y,module_dim_x AS dim_x, module_dim_z AS dim_z, module_x_offset,module_y_offset,endcap_z_offset from endcap_standard_module,endcap;");
  db->getTuple();

  G4double X=db->fetchDouble("module_x_offset");
  G4double Y=(db->fetchDouble("dim_y") - L)/2. + 
    db->fetchDouble("xcenter")/2.+ db->fetchDouble("Gap");

  G4double Z1=db->fetchDouble("endcap_z_offset");
  db->getTuple();
  G4double Z2=db->fetchDouble("endcap_z_offset");

#ifdef MOKKA_GEAR
  
  // getting the outer radius as maximal reached radius
  // so as outermost corner of L-shaped module

  // get extensions for module
  G4double moduleX = db->fetchDouble("dim_x") ;
  G4double moduleY = db->fetchDouble("dim_y") ;
  G4double moduleZ = db->fetchDouble("dim_z") ;
  
  // calculate outermost point1
  // 2nd coordinate calculated elaborate so one can comprehend construction more easiely
  G4double pointX = X + moduleX/2 ;
  G4double pointY = Y - moduleY/2 + (moduleY - L) ;
  
  // set outer radius possibility one
  G4double radius = std::sqrt( std::pow( pointX, 2 ) + std::pow( pointY, 2 ) ) ;
  helpEndcap.outerRadius = radius ;

  // calculate sencond possible point 
  pointX = X - moduleX/2 + (moduleX - L) ;
  pointY = Y + moduleY/2 ;

  // set outer radius as max
  radius = std::sqrt( std::pow( pointX, 2 ) + std::pow( pointY, 2 ) ) ;
  helpEndcap.outerRadius = std::max( helpEndcap.outerRadius , radius ) ;

  // getting inner radius as minimal distance
  helpEndcap.innerRadius = std::abs( X  - moduleX/2 ) ;

  // get least z-distance as maximum of Z1 or Z2
  helpEndcap.zMax = std::max( std::abs(Z1) , std::abs(Z1) ) - ( moduleZ / 2 ) ;

  // set phi0 to zero
  helpEndcap.phi0 = 0. ;

#endif


  for(irot = 0; irot<4; irot++){
    G4RotationMatrix *rot1 = new G4RotationMatrix();
    phirot = pi*0.5*irot;
    rot1->rotateZ(phirot);
    new MyPlacement(rot1,
		    G4ThreeVector(+X*cos(phirot)+Y*sin(phirot),
				  -X*sin(phirot)+Y*cos(phirot),
				  Z1),
		    EnvLogEcalModuleEndCap,
		    "EndCapModule+z",
		    MotherLog,false,ECALENDCAPPLUS*100
		    +(irot+1)*10+6);
    theEndCapCellSD->SetStaveRotationMatrix(irot+1,-phirot);
    theEndCapGRSD->SetStaveRotationMatrix(irot+1,-phirot);
  }
  theEndCapCellSD->
    SetModuleZOffset(6,
		     Z1);

  theEndCapGRSD->
    SetModuleZOffset(6,
		     Z1);

  X=-X;
  //Y=-Y;
  for(irot = 0; irot<4; irot++){
    G4RotationMatrix *rot1 = new G4RotationMatrix();
    phirot = pi*0.5*irot;
    rot1->rotateY(pi);
    rot1->rotateZ(-phirot);
    G4int theStave;
    if((theStave = 5-irot)==5) theStave = 1;
    new MyPlacement(rot1,
		    G4ThreeVector(+X*cos(phirot)+Y*sin(phirot),
				  -X*sin(phirot)+Y*cos(phirot),
				  Z2),
		    EnvLogEcalModuleEndCap,
		    "EndCapModule-z",
		    MotherLog,false,ECALENDCAPMINUS*100
		    +theStave*10+0);
  theEndCapCellSD->
    SetModuleZOffset(0,
		     Z1);
  theEndCapGRSD->
    SetModuleZOffset(0,
		     Z1);
  }
}

void Ecal03::EndcapWPlate(G4LogicalVolume* MotherLog)
{
  G4LogicalVolume * LogWPlate[200];
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(.7,.7,.9));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  
  // W Logicals
  
  db->exec("select cut_shape_h AS H from endcap_standard_module;");
  db->getTuple();
  G4double L = db->fetchDouble("H")/cos(pi/4);

  db->exec("select layer_id, w_dim_x AS dim_x,w_dim_y AS dim_y,w_dim_z AS dim_z from endcap_standard_module,endcap_layer;");


  G4ThreeVector lastLV;
  while(db->getTuple()!=NULL){

    // G3 optimisation: lastLV versus nextLV
    G4ThreeVector nextLV(db->fetchDouble("dim_x"),
			 db->fetchDouble("dim_y"),
			 db->fetchDouble("dim_z"));
    if(lastLV==nextLV) {
      LogWPlate [db->fetchInt("layer_id")] = LogWPlate [db->fetchInt("layer_id")-1]; 
      continue;
    }
    else lastLV=nextLV;
    // end of G3 optimisation

    G4VSolid* WPlate = 
      BuildECShape(L,
		   db->fetchDouble("dim_x"),
		   db->fetchDouble("dim_y"),
		   db->fetchDouble("dim_z"));
    
    
    LogWPlate [db->fetchInt("layer_id")] =
      new G4LogicalVolume(WPlate,
			  CGAGeometryManager::GetMaterial("tungsten_19.3gccm"),
			  "WEndCapLogical", 
			  0, 0, 0);  
    LogWPlate [db->fetchInt("layer_id")]->SetVisAttributes(VisAtt);
  }
  
  // W Placements
  db->exec("select layer_id, w_dim_z AS dim_z,0 as w_x_offset, 0 as w_y_offset, w_z_offset from endcap_layer, endcap_standard_module;"); //   added dim_z


  
  while(db->getTuple()!=NULL){
    new MyPlacement(0,
		    G4ThreeVector(db->fetchDouble("w_x_offset"),
				  db->fetchDouble("w_y_offset"),
				  db->fetchDouble("w_z_offset")),
		    LogWPlate [db->fetchInt("layer_id")],
		    "WEndCap",
		    MotherLog,false,0);

#ifdef MOKKA_GEAR
    // get positions of Layer as offset - half dim_z
    G4double halfZ = db->fetchDouble("dim_z")/2 ;
    helpEndcap.layerPos.push_back( db->fetchDouble("w_z_offset") - halfZ ) ;

    // get radiator thickness as W-Plate Thickness
    helpEndcap.radiThickness.push_back( halfZ * 2 ) ;

    // count layers
    helpEndcap.count ++ ;

#endif

  } // close loop
}

void Ecal03::EndcapAlveolusModule(G4LogicalVolume* MotherLog)
{
  G4LogicalVolume * AlvLog;

  // Cut Shape Solid
  db->exec("select cut_shape_h AS H from endcap_standard_module;");
  db->getTuple();
  G4double L = db->fetchDouble("H")/cos(pi/4);

  db->exec("select alveolus_dim_x AS dim_x,alveolus_dim_y AS dim_y,alveolus_dim_z AS dim_z from endcap_standard_module;");
  db->getTuple();

  G4VSolid* EndCapAlveolus = 
    BuildECShape(L,
		 db->fetchDouble("dim_x"),
		 db->fetchDouble("dim_y"),
		 db->fetchDouble("dim_z"));
  
  G4VisAttributes *VisAtt = new G4VisAttributes(G4Colour(.2,.8,.2));
  VisAtt->SetForceWireframe(true);
  AlvLog = 
    new G4LogicalVolume(EndCapAlveolus,
			CGAGeometryManager::GetMaterial("air"),
			"EndCapAlveolus", 
			0, 0, 0);  
  AlvLog->SetVisAttributes(VisAtt);

  // Fill the standard Alveolus
  EndcapAlveolusPads(AlvLog);

  // EndCap Alveolus Placements
  // standard module x and y offsets (needed for the SD)
  db->exec("select module_x_offset,module_y_offset,alveolus_dim_x/2. AS dimX,alveolus_dim_y/2. AS dimY, module_dim_x/2. AS MDimX, modules_gap AS Gap,al_plates_center_dim_xy AS xcenter from endcap_standard_module;");
  db->getTuple();
  
  G4double Xoff,Yoff;
  Xoff = db->fetchDouble("module_x_offset")-
	 db->fetchDouble("dimX");
  Yoff = db->fetchDouble("xcenter")/2.+ db->fetchDouble("Gap")+
	 db->fetchDouble("MDimX")-db->fetchDouble("dimX");
  /*
  Yoff = db->fetchDouble("module_y_offset")-
	 db->fetchDouble("dimY");
  Yoff = db->fetchDouble("MDimX")-db->fetchDouble("module_x_offset")+
	 db->fetchDouble("Gap")+db->fetchDouble("MDimX")-
	 db->fetchDouble("dimX");
  */
	  
  db->exec("select layer_id,0 as alveolus_x_offset, 0 as alveolus_y_offset, alveolus_z_offset from endcap_layer, endcap_standard_module;");

  while(db->getTuple()!=NULL) {
    G4int layer_id = db->fetchInt("layer_id");
    new MyPlacement(0,
		    G4ThreeVector(db->fetchDouble("alveolus_x_offset"),//x
				  db->fetchDouble("alveolus_y_offset"),//y
				  db->fetchDouble("alveolus_z_offset")),//z
		    AlvLog,
		    "EndCapAlveolus",
		    MotherLog,false,layer_id);
    theEndCapCellSD->
      AddLayer(layer_id,
	       Xoff,
	       Yoff,
	       db->fetchDouble("alveolus_z_offset"));    
    theEndCapGRSD->
      AddLayer(layer_id,
	       Xoff,
	       Yoff,
	       db->fetchDouble("alveolus_z_offset"));    
  }
}


void Ecal03::EndcapAlveolusPads(G4LogicalVolume* MotherLog)
{
  db->exec("select cut_shape_h AS H from endcap_standard_module;");
  db->getTuple();
  G4double L = db->fetchDouble("H")/cos(pi/4);

//--------------------------------------
//                    PCB
//--------------------------------------
  
  // 
  db->exec("select pads_dim_x AS dim_x,pads_dim_y AS dim_y, pcb_thickness AS dim_z from endcap_standard_module,ecal;");
  db->getTuple();

  G4VSolid* SubPads= 
    BuildECShape(L,
		 db->fetchDouble("dim_x"),
		 db->fetchDouble("dim_y"),
		 db->fetchDouble("dim_z"));
  
  G4VisAttributes *VisAttPCB = new G4VisAttributes(G4Colour(.8,.2,.8));
  VisAttPCB->SetForceWireframe(true);
  G4LogicalVolume * PCBLog = 
    new G4LogicalVolume(SubPads,
			CGAGeometryManager::GetMaterial("g10"),
			"PCBLog", 
			0, 0, 0);  
  PCBLog->SetVisAttributes(VisAttPCB);
  
  // PCB Placements - couche dessous
  db->exec("select pads_x_offset, pads_y_offset,-(si_thickness+pcb_thickness)/2. as pads_z_offset from endcap_standard_module, ecal;");
  db->getTuple();

  MyPlacement::InsertComment("Ecal endcap G10 PCB");  
  new MyPlacement(0,
		  G4ThreeVector(db->fetchDouble("pads_x_offset"),//x
				db->fetchDouble("pads_y_offset"),//y
				db->fetchDouble("pads_z_offset")),//z
		  PCBLog,
		  "PCBunder",
		  MotherLog,false,0);
  
  // PCB Placements - couche dessus  
  db->exec("select pads_x_offset, pads_y_offset,(si_thickness+pcb_thickness)/2. as pads_z_offset from endcap_standard_module, ecal;");
  db->getTuple();
  new MyPlacement(0,
		  G4ThreeVector(db->fetchDouble("pads_x_offset"),//x
				db->fetchDouble("pads_y_offset"),//y
				db->fetchDouble("pads_z_offset")),//z
		  PCBLog,
		  "PCBup",
		  MotherLog,false,0);
  
  //--------------------------------------
  //                  Si
  //--------------------------------------
  
  // Solid
  db->exec("select pads_dim_x AS dim_x,pads_dim_y AS dim_y, si_thickness AS dim_z from endcap_standard_module,ecal;");
  db->getTuple();

  G4VSolid*  SubSi = 
    BuildECShape(L,
		 db->fetchDouble("dim_x"),
		 db->fetchDouble("dim_y"),
		 db->fetchDouble("dim_z"));

  /*
  G4UserLimits* pULimits=
    new G4UserLimits(theMaxStepAllowed);
  */
  G4VisAttributes *VisAttPads = new G4VisAttributes(G4Colour(.8,.8,.2));
  VisAttPads->SetForceWireframe(true);
  G4LogicalVolume *PadLog = 
    new G4LogicalVolume(SubSi,
			CGAGeometryManager::GetMaterial("silicon_2.33gccm"),
			"EndCapPad", 
			0, 0, 0);  
  PadLog->SetVisAttributes(VisAttPads);

  FillEndcapPlane(PadLog,L,
	db->fetchDouble("dim_x"),
	db->fetchDouble("dim_y"));

  //PadLog->SetSensitiveDetector(theEndCapSD);
  
  // Si Placement
  db->exec("select pads_x_offset, pads_y_offset,0 as pads_z_offset from endcap_standard_module, ecal;");
  db->getTuple();
  MyPlacement::InsertComment("Ecal endcap Silicium sensitive detector layer");
  new MyPlacement(0,
		  G4ThreeVector(db->fetchDouble("pads_x_offset"),//x
				db->fetchDouble("pads_y_offset"),//y
				db->fetchDouble("pads_z_offset")),//z
		  PadLog,
		  "SiEndCap",
		  MotherLog,false,ENDCAP_SD_PLATE_FLAG);
}

G4VSolid* Ecal03::BuildECShape(G4double L,
			     G4double dim_x,
			     G4double dim_y,
			     G4double dim_z)
{
  G4Box * box1 = new G4Box("Box1",
			   dim_x/2,
			   (dim_y-L)/2.,
			   dim_z/2.);
  G4Box * box2 = new G4Box("Box2",
			   (dim_x-L)/2,
			   dim_y/2.,
			   dim_z/2.);

  G4Trd * Trd = new G4Trd("Trd",
			  0,
			  L*cos(pi/4.),
			  dim_z/2.,
			  dim_z/2.,
			  L*sin(pi/4.)/2.);
  
  G4UnionSolid* union1=
    new G4UnionSolid("ECShapeBoxes",
		     box1,
		     box2,
		     0,
		     G4ThreeVector(-L/2.,
				   L/2.,
				   0.));
  G4RotationMatrix *rot = new G4RotationMatrix();
  rot->rotateX(pi*0.5);
  rot->rotateY(-pi*0.25);
  G4double ht = sin(pi/4.)*L*sin(pi/4.)/2.;
  G4double ot = cos(pi/4.)*L*sin(pi/4.)/2. - L + dim_x/2.;

  G4UnionSolid* union2=
    new G4UnionSolid("ECShape",
		     union1,
		     Trd,
		     rot,
		     G4ThreeVector(ot,
				   (dim_y-L)/2.+ht,
				   0.));
  return union2;
}

void 
Ecal03::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
  // In this complex subdetector the best is to read
  // all hits in the one of the sensitive detectors,
  // just for visualisation, so just for your eyes...
  theBarrelCellSD->LoadEvent(theSubDetectorEventHitsFileInput);
}
