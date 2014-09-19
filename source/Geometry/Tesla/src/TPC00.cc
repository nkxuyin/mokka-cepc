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
// $Id: TPC00.cc,v 1.6 2007/10/17 22:06:09 kristian Exp $
// $Name: mokka-07-00 $
//
//
// TPC00.cc
//
// History:  
// - first implementation P. Mora de Freitas (apr 01)
// - implemented new GEAR interface -- K. Harder, T. Pinto Jayawardena  2007-07-31



#include "Control.hh"
#include "MySQLWrapper.hh"
#include "TPC00.hh"
#include "TPCSD00.hh"
#include "CGAGeometryManager.hh"

#include "globals.hh"
#include "G4Tubs.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "CGADefs.h"
#ifdef MOKKA_GEAR
#include "gear/TPCParameters.h"
#include "gearimpl/TPCParametersImpl.h"
#include "gearimpl/FixedPadSizeDiskLayout.h"
#include "MokkaGear.h"
#include "G4EmCalculator.hh"
#include "G4ParticleTable.hh"

#endif



INSTANTIATE(TPC00)

TPC00::~TPC00()
{
//   if (!theTPCSD) delete theTPCSD;
}


G4bool TPC00::construct(const G4String &aSubDetectorName,
			 G4LogicalVolume *WorldLog)
{
  G4cout << "\nBuilding TPC..." << G4endl;

  // Sensitive detector
  // Threshold is 2 times the energy for Ar ionization,
  // let's say Ar_IonPotential  2 * 15.7596 ev.
 
  // Control::TPCCut enables the user to control the
  // TPC output file length.

  Ar_IonPotential=(2*15.7596)* eV;
 
  theTPCSD = new TPCSD00("TPC");
  RegisterSensitiveDetector(theTPCSD);

  Database* db = new Database(aSubDetectorName.data());
  
  db->exec("select * from tpc;");
  db->getTuple();

  inner_radius = db->fetchDouble("inner_radius");
  outer_radius = db->fetchDouble("outer_radius");
  z_half_length = db->fetchDouble("z_half_length");
  endplate_thickness = db->fetchDouble("endplate_thickness");
  inner_wall_thickness = db->fetchDouble("inner_wall_thickness");
  outer_wall_thickness = db->fetchDouble("outer_wall_thickness");
  inner_sensitive = inner_radius + inner_wall_thickness;
  outer_sensitive = outer_radius - outer_wall_thickness;
  G4PVPlacement *Phys;

  //***************************
  // TPC Al inner shield 
  //***************************
  G4Tubs *TPCInnerShieldSolid
    = new G4Tubs("TPCInnerShieldSolid", 
		 inner_radius, 
		 inner_radius+inner_wall_thickness,
		 z_half_length,
		 0.0*deg, 
		 360.0*deg);
    
  wallMat =CGAGeometryManager::GetMaterial("aluminium");

  
  G4LogicalVolume *TPCInnerShieldLogical =
    new G4LogicalVolume(TPCInnerShieldSolid,
			wallMat,
			"TPCInnerShieldLogical", 
			0,
			0,
			0);
  
  G4VisAttributes * VisAtt = new G4VisAttributes(G4Colour(0.,0.5,0.5));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  TPCInnerShieldLogical->SetVisAttributes(VisAtt);
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., 0.),
		      TPCInnerShieldLogical,
		      "TPCInnerShield",
		      WorldLog,
		      false,0);

  //***************************
  // TPC Al outer shield 
  //***************************
  G4Tubs *TPCOuterShieldSolid
    = new G4Tubs("TPCOuterShieldSolid", 
		 outer_radius-outer_wall_thickness, 
		 outer_radius,
		 z_half_length,
		 0.0*deg, 
		 360.0*deg );
  
  G4LogicalVolume *TPCOuterShieldLogical =
    new G4LogicalVolume(TPCOuterShieldSolid,
			CGAGeometryManager::GetMaterial("aluminium"),
			"TPCOuterShieldLogical", 
			0,
			0,
			0);
  
  TPCOuterShieldLogical->SetVisAttributes(VisAtt);
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., 0.),
		      TPCOuterShieldLogical,
		      "TPCOuterShield",
		      WorldLog,
		      false,0);
  

  // TPC gas chamber in number_of_layers layers
  number_of_layers = db->fetchInt("number_of_layers");
  G4double FirstLayer= inner_radius+inner_wall_thickness;
  LayerThickness = 
    (outer_radius-outer_wall_thickness - FirstLayer) / number_of_layers;
  
  G4double ZLayer = z_half_length;
  
  // TPC gas chamber "gas enveloppe" 
  // (to avoid Geant4 navigation failures with G4Tub!!)
  
  G4Tubs *TPCGasEnvelopeSolid;
  G4LogicalVolume *TPCGasEnvelopeLogical;

  VisAtt = new G4VisAttributes(G4Colour(1.,0.5,0.5));
  VisAtt->SetForceWireframe(true);
  TPCGasEnvelopeSolid 
    = new G4Tubs("TPCChamber", 
		 FirstLayer, 
		 outer_radius-outer_wall_thickness,
		 ZLayer,
		 0.0*deg, 
		 360.0*deg);
  
  TPCGasEnvelopeLogical=  
    new G4LogicalVolume(TPCGasEnvelopeSolid,
			CGAGeometryManager::GetMaterial("argon"),
			"TPCGasEnvelopeLogical", 
			0, 
			0, 
			0);

  TPCGasEnvelopeLogical->SetVisAttributes(VisAtt);

  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., 0.),
		      TPCGasEnvelopeLogical,
		      "TPCGasEnvelope",
		      WorldLog,
		      false,0);
  
  // Now, the gas layers to stop steps on the boudaries
  //
  G4Tubs *TPCChamberSolid;
  G4LogicalVolume *TPCChamberLogical;
  VisAtt = new G4VisAttributes(G4Colour(1.,1.,1.));
  VisAtt->SetForceWireframe(true);

  for (G4int n_layer=0;n_layer<number_of_layers;n_layer++)
    {
      TPCChamberSolid
	= new G4Tubs("TPCChamber", 
		     FirstLayer+(n_layer*LayerThickness), 
		     FirstLayer+((n_layer+1)*LayerThickness),
		     ZLayer,
		     0.0*deg, 
		     360.0*deg);

      TPCChamberLogical=  
	new G4LogicalVolume(TPCChamberSolid,
			    CGAGeometryManager::GetMaterial("argon"),
			    "TPCChamberLogical", 
			    0, 
			    0, 
			    0);
      
      TPCChamberLogical->SetSensitiveDetector(theTPCSD);
      TPCChamberLogical->SetVisAttributes(VisAtt);

      Phys=
	new G4PVPlacement(0,
			  G4ThreeVector(0., 0., 0.),
			  TPCChamberLogical,
			  "TPCChamber",
			  TPCGasEnvelopeLogical,
			  false,n_layer+1);
    }

  // TPC endplates
  // Material = mix. Lets calculate the mass for a box with
  // 1 mm**2 as base and 100 mm long
  G4double Base = 1.;

  G4double kapton_percent = db->fetchDouble("endplate_kapton_percent");
  G4double cu_percent = db->fetchDouble("endplate_cu_percent");
  G4double air_percent = db->fetchDouble("endplate_air_percent");

  if((abs(kapton_percent+cu_percent+air_percent - 100)) > 0.000001)
    {
      G4cout << "\nkapton_percent=" << kapton_percent
	     << "\ncu_percent=" << cu_percent
	     << "\nair_percent= " << air_percent 
	     << "\nkapton_percent+cu_percent+air_percent ="
	     << kapton_percent+cu_percent+air_percent << G4endl;
      Control::Abort("TPC endplates material percents doesn't 100 %!!!",MOKKA_ERROR_BAD_DATABASE_PARAMETERS);
    }

  // to avoid precision troubles:
  air_percent = air_percent + 
    abs(kapton_percent+cu_percent+air_percent - 100);

  G4double KaptonMass = CGAGeometryManager::GetMaterial("kapton")->GetDensity()*
    Base*kapton_percent;
  G4double CuMass = CGAGeometryManager::GetMaterial("copper")->GetDensity()*
    Base*cu_percent;
  G4double AirMass = CGAGeometryManager::GetMaterial("air")->GetDensity()*
    Base*air_percent;

  // masse totale pour calculer les fractions
  G4double TotalMass = 
    KaptonMass+CuMass+AirMass;

  // densite du melange = masse totale/ volume
  G4double MixDensite = TotalMass/(100*1);
  G4cout << "MixDensite = " 
	 << MixDensite / g * cm3 
	 << " g/cm3" << G4endl;

  // definition du melange
  G4Material* Mix = new G4Material("MixTpcEndplates",MixDensite,3);
  Mix->AddMaterial(CGAGeometryManager::GetMaterial("kapton"),KaptonMass/TotalMass);
  Mix->AddMaterial(CGAGeometryManager::GetMaterial("copper"),CuMass/TotalMass);
  Mix->AddMaterial(CGAGeometryManager::GetMaterial("air"),AirMass/TotalMass);
  G4cout << "MixTpcEndplates->GetRadlen()= " 
	 << Mix->GetRadlen() /mm   << " mm\n";

  G4cout << "Total radiation length in " 
	 << endplate_thickness
	 << " mm of TPC endplate = "
	 << endplate_thickness/Mix->GetRadlen() /mm
	 << "." << G4endl;

  G4Tubs *TPCEndSolid
    = new G4Tubs("TPCEndSolid", 
		 inner_radius,
		 outer_radius,
		 endplate_thickness/2.,
		 0.0*deg, 
		 360.0*deg);
  
  G4LogicalVolume *TPCEndLogical=
    new G4LogicalVolume(TPCEndSolid,
			Mix,
			"TPCEndLogical", 
			0,
			0,
			0);
 
  VisAtt = new G4VisAttributes(G4Colour(0.,0.5,0.5));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  TPCEndLogical->SetVisAttributes(VisAtt);
  
  // TPC endplates placement
  G4double ZEndPlates = 
    z_half_length +
    endplate_thickness/2.;
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., ZEndPlates),
		      TPCEndLogical,
		      "TPCEnd",
		      WorldLog,
		      false,0);
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0., -ZEndPlates),
		      TPCEndLogical,
		      "TPCEnd",
		      WorldLog,
		      false,0);

  // FCH = two sensitive twin "plates of air", just to register 
  // the particle step inside it.
  G4Tubs *FCHSolid
    = new G4Tubs("FCHSolid", 
		 db->fetchDouble("fch_inner_radius"), 
		 db->fetchDouble("fch_outer_radius"),
		 db->fetchDouble("fch_thickness")/2.,
		 0*deg, 
		 360.*deg);
  
  G4LogicalVolume *FCHLogical=
    new G4LogicalVolume(FCHSolid,
			CGAGeometryManager::GetMaterial("air"),
			"FCHLogical", 
			0,
			0,
			0);
  
  VisAtt = new G4VisAttributes(G4Colour(0.2,0.0,0.8));
  VisAtt->SetForceWireframe(true);
  //VisAtt->SetForceSolid(true);
  FCHLogical->SetVisAttributes(VisAtt);
  FCHLogical->SetSensitiveDetector(theTPCSD);
  
  // FCH placements (nexted to the Tpc endplates!!!)
  // (obs: copy number >= 1000 flags FCH)
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0.,
				    db->fetchDouble("fch_thickness")/2.+
				    z_half_length+
				    endplate_thickness),
		      FCHLogical,
		      "FCH",
		      WorldLog,
		      false,1000);
  Phys=
    new G4PVPlacement(0,
		      G4ThreeVector(0., 0.,
				    -(db->fetchDouble("fch_thickness")/2.+
				      z_half_length+
				      endplate_thickness)),
		      FCHLogical,
		      "FCH",
		      WorldLog,
		      false,1001);

  // Closes Database connection
  delete db;
  G4cout << "TPC done.\n" << G4endl;
  return true;
}
//new routine to ensure all data is obtained before they called to
//be written to the xml file

#ifdef MOKKA_GEAR

void TPC00::GearSetup()
{

  // write data to MokkaGear that will print out as xml
  // set parameters for PadRowLayout

   
  //Approximate Ion Potential = Ar_IonPotential
  //Divide by 1000 to obtain in GeV
  G4double IonPotential = Ar_IonPotential/1000;

  G4EmCalculator findDEdx;
  G4ParticleTable* theParticleTable=G4ParticleTable::GetParticleTable();
 
  //Radiation Length from the Material database of aluminium
  TPCWallProperties_RadLen = wallMat->GetRadlen();

  //Looping over bins in the DEDX table to obtain the mip DEDX 

  //From energy 0.0001MeV to 1000MeV in steps of 10
  G4double step_size=10,step,mindEdx=99999;
  G4double CurrentdEdx;
  
  
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx = 
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  wallMat);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }

  TPCWallProperties_dEdx=(mindEdx)/1000;

  G4Material *gasMat =CGAGeometryManager::GetMaterial("argon");
  
  //Obtaining the Radiation Length for Argon from the Materials database
  TPCGasProperties_RadLen = gasMat-> GetRadlen();

  
   
  //looping over bins in the DEDX table to obtain the mip DEDX 
 
  mindEdx=99999;
  for (step=0.0001;step<=1000;step+=step_size)
    {
      CurrentdEdx=
	findDEdx.ComputeTotalDEDX(step,
				  theParticleTable->FindParticle("mu-"),
				  gasMat);
      if(CurrentdEdx<mindEdx)
	{
	  mindEdx=CurrentdEdx;
	}
    }

  TPCGasProperties_dEdx=(mindEdx)/1000;

 
  G4double rMin             = inner_sensitive ;
  G4double rMax             = outer_sensitive ;
  G4double padHeight        = floor(LayerThickness*1000.)/1000. ;
  G4double padWidth         = 2. ; // gear does not allow zero for the pad width
  G4int nRows               = number_of_layers ;
  G4double padGap           = 0 ;

  // set parameters for tpc
  G4double maxDriftLength   = z_half_length ;
  G4double driftVelocity    = 0 ;
  G4double readoutFrequency = 0 ;
  
  // set parameters to PadRowLayout
  gear::PadRowLayout2D* padLayout = 
    new gear::FixedPadSizeDiskLayout( rMin, rMax, padHeight, padWidth, nRows, padGap );
  
  // Set TPCParameters
  gear::TPCParametersImpl* tpcParameters = new gear::TPCParametersImpl;
  tpcParameters -> setPadLayout ( padLayout );
  tpcParameters -> setMaxDriftLength( maxDriftLength );
  tpcParameters -> setDriftVelocity( driftVelocity );
  tpcParameters -> setReadoutFrequency( readoutFrequency );

  // set non-standard parameters in map
  tpcParameters -> setDoubleVal( "tpcOuterRadius" , outer_radius ) ;
  tpcParameters -> setDoubleVal( "tpcInnerRadius", inner_radius ) ;
  tpcParameters -> setDoubleVal( "tpcInnerWallThickness", inner_wall_thickness ) ;
  tpcParameters -> setDoubleVal( "tpcOuterWallThickness", outer_wall_thickness ) ;
  tpcParameters -> setDoubleVal( "TPCWallProperties_RadLen",TPCWallProperties_RadLen ) ;
  tpcParameters -> setDoubleVal( "TPCWallProperties_dEdx",TPCWallProperties_dEdx) ;
  tpcParameters -> setDoubleVal( "TPCGasProperties_RadLen",TPCGasProperties_RadLen);
  tpcParameters -> setDoubleVal( "TPCGasProperties_dEdx", TPCGasProperties_dEdx);
  tpcParameters -> setDoubleVal( "tpcIonPotential",IonPotential);


  // Write tpcParameters to GearMgr
  // Parameters for TPC
  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setTPCParameters( tpcParameters ) ;
}

#endif

