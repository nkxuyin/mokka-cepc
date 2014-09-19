// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: BeamCalSD00.cc,v 1.7 2008/11/12 16:05:32 steve Exp $
// $Name: mokka-07-00 $
//
// History:
// - first implementation, code from Fcal collaboration, implemented by A. Hartin and S. Aplin, Oct 2008
#include "Control.hh"
#include "BeamCalSD00.hh"
//#include "BeamCal00.hh"

#include "G4Step.hh"
#include "G4SDManager.hh"
#include "UserTrackInformation.hh"
#include "Encoder32Fcal.hh"
#include "G4VTouchable.hh"
#include "CalHit.hh"
#include "TRKHit.hh"
#include "G4ThreeVector.hh"

#include "G4SteppingManager.hh"
#include "G4Navigator.hh"

#include "assert.h"

#include "CGADefs.h"
#include "G4AffineTransform.hh"
#include "G4Material.hh"
#include "G4VProcess.hh"
#include "G4ParticleDefinition.hh"
#include "G4StepPoint.hh"
#include "globals.hh"
#include "G4UImanager.hh"
#include "G4TransportationManager.hh"

#ifdef MOKKA_GEAR
#include "gear/CalorimeterParameters.h"
#include "gearimpl/CalorimeterParametersImpl.h"
#include "gearimpl/LayerLayoutImpl.h"
#include "MokkaGear.h"
#endif

BeamCalSD00::BeamCalSD00(G4String name, 
			 G4double Rinner,
			 G4double Router,
			 G4double xAngle,
			 G4double zStart,
			 G4double sPhi,
			 G4double dPhi,
			 G4double nWafers,
			 G4double DAStart,
			 G4double dLayer,
			 G4int nLayers,
			 G4double dSensor,
			 G4double dAbsorber,
			 G4double Segm,
			 G4double pairsMonitorZ,
			 G4double envVolLength,
			 G4double envVolCenterZ) : VSensitiveDetector(name), CalCollection(0), HCID(-1) {
  G4String CollName = name + "Collection";
  collectionName.insert(CollName);

  theEncoder = new Encoder32Fcal();

  
  Rinner = getRin(Rinner);
  Router = getRout(Router);
  xAngle = getxAngle(xAngle);
  zStart = getZ(zStart);
  sPhi = getSPhi(sPhi);
  dPhi = getDPhi(dPhi);
  nWafers = getWafer (nWafers);
  DAStart = getDA (DAStart);
  dLayer = getDl(dLayer);
  dSensor = getDS(dSensor);
  dAbsorber = getDAb(dAbsorber);
  Segm = getSegm(Segm);
  nLayers = getNLayers(nLayers);
  pairsMonitorZ = getPairsMonitorZ(pairsMonitorZ);
  envVolumeLength = envVolLength;
  envVolumeCenterZ = envVolCenterZ;

//  G4cout << "SJA:TAG: BeamCalSD00 Constructor:" 
//	 << " Rinner = " << Rinner 	
//	 << " Router = " << Router 	
//	 << " xAngle = " << xAngle 	
//	 << " zStart = " << zStart 	
//	 << " sPhi = " << sPhi 	
//	 << " dPhi = " << dPhi	
//	 << " nWafers = " << nWafers	
//	 << " DAStart = " << DAStart	
//	 << " dLayer = " << dLayer 	
//	 << " dSensor = " << dSensor	
//	 << " dAbsorber = " << dAbsorber	
//	 << " Segm = " << Segm 	  
//	 << G4endl;

  //SJA: Setup the segmentation in R
  //SJA: We first divide by the estimated segmentation get the number of rows, and then divide the distance by that 
  nRs = G4int( (Router-Rinner) / Segm ) + 1;
  SegmdR = (Router-Rinner) / nRs ;

//  G4cout << "SJA:TAG: Number of Rows: " << nRs 
//	 << " Segmentation: " << SegmdR 
//	 << G4endl;  

  //SJA: Spanning angle divided by the number of wafers
  WaferPhiRange = dPhi/nWafers;
  //SJA: Angle range of DA area 
  DAPhiRange    = 360*deg - dPhi;

  //SJA: Setup the Phi segmentation
  for(int i=0; i<nRs; ++i){
    //SJA: angular range divided by (radial segmentation / radius ring)
    //SJA: this gives a nearly constant phi segmentation 
    G4double initSegm = SegmdR / (Rinner + SegmdR*i + SegmdR/2); 
    nPhis.push_back( G4int( WaferPhiRange/initSegm ) +1 ); 
    SegmdPhi.push_back( WaferPhiRange/nPhis.back() ) ;
    
//    G4cout << "SJA:TAG: Number of Pads: " <<  nPhis.back() 
//	   << " Segmentation Radians: " << SegmdPhi.back() 
//	   << " Segmentation mm: " << SegmdPhi.back()*(Rinner + SegmdR*i + SegmdR/2)  
//	   << G4endl;  

  }

#ifdef MOKKA_GEAR
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++
  // +  MOKKA GEAR                                      +
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++
	
  const G4double zMin = envVolumeLength/2.0 - nLayers * dLayer ;

  gear::CalorimeterParametersImpl* bcalParam = new gear::CalorimeterParametersImpl(Rinner, Router, zMin, 2, 0.);
	
  for (int i=0; i < nLayers; i++)
    {
      //SJA:FIXME: Phi segmentation still to be set. Nominally set to 1.0 here for now.
      bcalParam->layerLayout().positionLayer(0.0, dLayer, SegmdR, 1.0, dAbsorber);
    }
	
  bcalParam->setDoubleVal("beam_crossing_angle", xAngle ) ;
  bcalParam->setDoubleVal("cylinder_staring_phi", sPhi ) ;
  bcalParam->setDoubleVal("cylinder_spanning_phi", dPhi ) ;
  bcalParam->setDoubleVals("phi_segmentation", SegmdPhi);
  G4double DArinner =  Rinner + (DAStart-1)*SegmdR;
  bcalParam->setDoubleVal("dead_area_outer_r", DArinner ) ;
  bcalParam->setDoubleVal("pairsMonitorZ", pairsMonitorZ) ;
  gear::GearMgr* gearMgr = MokkaGear::getMgr() ;
  gearMgr->setBeamCalParameters ( bcalParam );

#endif


}

BeamCalSD00::~BeamCalSD00(void)
{

}

void BeamCalSD00::Initialize(G4HCofThisEvent *)
{
  CalCollection = new BCHitsCollection(SensitiveDetectorName, collectionName[0]);


}


G4bool BeamCalSD00::ProcessHits(G4Step *aStep, G4TouchableHistory *)
{

  G4int LrNum=0; 
  G4int PhiNum=0;
  G4int RingNum=0;
  //  G4int direction=0;
  //  G4String LrName;
  //  char charbuf[256];
	
	
  //  G4double dPhis[100], DAdPhis[100], SegmdPhi[100],SegmdPhiDA[100] , SegmnPhisDA[100];
  //  G4int nPhis[100], SegmnPhis[100], DAnPhis[100];

  G4double edep = aStep->GetTotalEnergyDeposit();

  //process only if energy>0 except geantinos
  //  G4cout << "SJA:TAG: edep:" << edep  << G4endl; 
  if(edep<=0. && aStep->GetTrack()->GetDefinition()->GetParticleType()!="geantino") return true;

  G4String volumeName = aStep->GetTrack()->GetVolume()->GetName();


  //	LrName.assign(volumeName);
  //	memset(charbuf, '\0',sizeof(charbuf));
  //
  //
  //	if(LrName.find("Forw") != G4String::npos)
  //	{
  //		direction = 1;
  //	}
  //	else if (LrName.find("Back") != G4String::npos)
  //	{
  //		direction = -1;
  //	}
  //
  //	if(LrName.find("DA") != G4String::npos)	
  //	{
  //		LrName.copy(charbuf, LrName.length()-17, 17);
  //	}
  //	else
  //	{		
  //		LrName.copy(charbuf, LrName.length()-15, 15);
  //	}
  //	LrNum=atoi(charbuf);//-->Layer number (K)


  LrNum = aStep->GetPreStepPoint()->GetPhysicalVolume()->GetCopyNo();

  //  G4cout << "SJA:TAG: LrNum:" << LrNum  << G4endl; 

  G4ThreeVector preHitPos = G4ThreeVector(aStep->GetPreStepPoint()->GetPosition());
  G4ThreeVector postHitPos = G4ThreeVector(aStep->GetPostStepPoint()->GetPosition());

  G4ThreeVector hitPos = (preHitPos + postHitPos) / 2.0 ;
	
//    G4cout << "SJA:TAG: Hit X:" << hitPos(0)  
//	 << " Hit Y:" << hitPos(1)
//	 << " Hit Z:" << hitPos(2)
//	 << " Hit phi:" << hitPos.phi()
//	 << " Hit r:" << hitPos.perp()
//	 << G4endl; 

  G4int side = 0;

  //
  if ( LrNum > 0 )
    {
      side=1;		
      hitPos.rotateY(-xAngle/2.*mrad);
    }
  if ( LrNum < 0 )
    {
      side=2;
      hitPos.rotateY(xAngle/2.*mrad);
    }


//  G4cout << "SJA:TAG: After rotation: Hit X:" << hitPos(0)  
//	 << " Hit Y:" << hitPos(1)
//	 << " Hit Z:" << hitPos(2)
//	 << " Hit phi:" << hitPos.phi()
//	 << " Hit r:" << hitPos.perp()
//	 << G4endl; 


  //SJA: this is ok as hitPos is rotated
  G4double rHit = hitPos.perp();
  G4double phiHit = hitPos.phi();

  if(phiHit<0.0) phiHit = phiHit + 360*deg;

  //SJA: this will give ring 0 for the first ring
  RingNum = (G4int)floor((rHit - Rinner)/SegmdR);//-->Ring number(I)
  //  G4cout << "SJA:TAG: RingNum:" << RingNum << " (rHit - Rinner)/SegmdR:" << (rHit - Rinner)/SegmdR << G4endl;

  //SJA: we want to start the indexing in phi from the starting phi of the non cut-away area
  G4double phiHit_rel = phiHit - sPhi;
  
  //  G4cout << "SJA:TAG: relative phi:" << phiHit_rel << " sPhi:" << sPhi << G4endl;

  if( phiHit_rel < 0.0 ) phiHit_rel = (360.0*deg) + phiHit_rel;

  //  G4cout << "SJA:TAG: relative phi:" << phiHit_rel << G4endl;

  //  if(phiHit<(sPhi-360.*deg)) phiHit_rel+=360.*deg;

  //  dPhis[RingNum] = split_segm(WaferPhiRange, dR/(Rinner + dR/2+dR*RingNum));
  //  SegmdPhi[RingNum]=dPhis[RingNum];

  //  nPhis[RingNum] = split_n(WaferPhiRange, dR/(Rinner + dR/2+dR*RingNum));
  //  SegmnPhis[RingNum]=nPhis[RingNum];


//  DAdPhis[RingNum]=0;
//  DAnPhis[RingNum]=0;
//  SegmdPhiDA[RingNum]=0.;
	
//  if(RingNum>=DAStart)
//    {
//      DAdPhis[RingNum] = split_segm(DAPhiRange, dR/(Rinner + dR/2 + dR*RingNum));
//      DAnPhis[RingNum] = split_n(DAPhiRange, dR/(Rinner + dR/2 + dR*RingNum));
//      SegmdPhiDA[RingNum]=DAdPhis[RingNum];
//      SegmnPhisDA[RingNum]=DAnPhis[RingNum];
      //    }

  PhiNum = (G4int)floor(phiHit_rel/(SegmdPhi[RingNum]));//-->Phi number(J)
  G4double phi = sPhi + PhiNum*(SegmdPhi[RingNum]) + (SegmdPhi[RingNum]/2);

  //  G4cout << "SJA:TAG: PhiNum:" << PhiNum << G4endl;

//  //SJA:FIXME: This will give values of phi > 360
//  if(phiHit_rel<dPhi) {
//    PhiNum = (G4int)floor(phiHit_rel/(SegmdPhi[RingNum]));//-->Phi number(J)
//    phi = sPhi + PhiNum*(SegmdPhi[RingNum]) + (SegmdPhi[RingNum]/2);
//  } else {
//    G4cout << "SJA:TAG: hit in DA skipping for now" << G4endl ; return true;
//  }

  //  else if(phiHit_rel>=dPhi && RingNum>=DAStart) 
  //    {
  //     PhiNum = (G4int)nWafers*SegmnPhis[RingNum] + (G4int)floor((phiHit_rel-dPhi)/(SegmdPhiDA[RingNum]));
  //     phi = sPhi + dPhi + (PhiNum - (G4int)nWafers*SegmnPhis[RingNum])*(SegmdPhiDA[RingNum]) + (SegmdPhiDA[RingNum]/2);
  //   }


  G4double r = Rinner + SegmdR/2. + RingNum*SegmdR;
  //  G4cout << "SJA:TAG: Max R:" <<  Rinner + SegmdR*nRs << " nRs:" << nRs << " SegmdR:" << SegmdR << " Rinner:" << Rinner << " Router:" << Router << G4endl;


  //SJA: this gives the middle of the sensitive layer from of the sensitive region
  //G4double z = dAbsorber + dSensor/2.0 + (abs(LrNum) -1) * dLayer  ;

  //SJA: this looks problematic as the z is simply given as the middle of the sensitive layer from the sensitive region
  //SJA: the layer start at the front of the calorimter volume, not the center, hence: posSens = -length/2. + dAbsorber + dSensor/2. +i*dLayer;

  //  z = z - (nLayers/2) * dLayer;

  G4double z = envVolumeLength/2.0 - nLayers * dLayer + dAbsorber + dSensor/2.0 + (abs(LrNum) -1) * dLayer;   

  //SJA:FIXME: the prelayer is in front of the beamCal so needs special treatment
  //SJA:FIXME: a better way must be found to do this
  if(abs(LrNum)==nLayers+1) {
    //    G4cout << "we are in the pairs monitor now what ;) " << G4endl;
    z = -envVolumeLength/2.0 + dSensor/2.0 ;
  }

  if( LrNum<0 ) z = -z ;


//  G4cout << "SJA:TAG: localCellPos X:" << r*cos(phi)  
//	 << " localCellPos Y:" << r*sin(phi)
//	 << " localCellPos Z:" << z
//	 << " localCellPos phi:" << phi
//	 << " localCellPos r:" << r
//	 << " localCellPos r+SegmdR:" << r+SegmdR
//	 << G4endl; 


  G4ThreeVector *localCellPos = new G4ThreeVector(r*cos(phi),r*sin(phi),z);

  G4TouchableHandle theTouchable = aStep->GetPreStepPoint()->GetTouchableHandle();
  G4int depth = theTouchable->GetHistoryDepth();

  G4ThreeVector &transl = (G4ThreeVector&)theTouchable->GetTranslation(depth-1);
  G4RotationMatrix *rotat = (G4RotationMatrix*)theTouchable->GetRotation(depth-1);
  G4AffineTransform aftrans(rotat, transl);

  G4ThreeVector GlobalCellPos = aftrans.Inverse().TransformPoint(*localCellPos);

//  G4cout << "SJA:TAG: GlobalCellPos X:" << GlobalCellPos(0)
//	 << " GlobalCellPos Y:" << GlobalCellPos(1)
//	 << " GlobalCellPos Z:" << GlobalCellPos(2)
//	 << " GlobalCellPos phi:" << GlobalCellPos.phi()
//	 << " GlobalCellPos r:" << GlobalCellPos.perp()
//	 << G4endl; 


  // SJA: Let's try and find where this hit really is

//  G4StepPoint* startPoint = aStep->GetPreStepPoint();
//  G4StepPoint* finalPoint = aStep->GetPostStepPoint();
//  G4TouchableHandle startTouch = startPoint->GetTouchableHandle();
//  G4TouchableHandle finalTouch = finalPoint->GetTouchableHandle();
//	
//  G4VPhysicalVolume* currentVolume = startTouch->GetVolume();
//  G4String currentVolumeName = currentVolume->GetName();
//	
//  //  G4int currentCopyNumber = startTouch->GetCopyNumber();
//  //  G4LogicalVolume* currentLogicalVolume = currentVolume->GetLogicalVolume();
//	
//  G4VPhysicalVolume* nextVolume = finalTouch->GetVolume();
//  G4String nextVolumeName = nextVolume->GetName();
//	
//  const G4VProcess* limitingProcess = finalPoint->GetProcessDefinedStep();
//	
//  G4String limitingProcessName = limitingProcess->GetProcessName();
//
//  G4Navigator* myNavigator = new G4Navigator();
//  myNavigator->SetWorldVolume(G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->GetWorldVolume());	
//
//  G4TouchableHistoryHandle  myTouchable = myNavigator->CreateTouchableHistoryHandle();
//  G4VPhysicalVolume* myVolume = myNavigator->LocateGlobalPointAndSetup(GlobalCellPos); 
//	
//  G4String myVolumeName = myVolume->GetName();
//	
//  if( myVolumeName != currentVolumeName ) {
//    G4cout << "SJA:TAG:BeamCalSD00: Current Volume Name: " << currentVolumeName << G4endl;
//    G4cout << "SJA:TAG:BeamCalSD00: Next Volume Name: " << nextVolumeName << G4endl;
//    G4cout << "SJA:TAG:BeamCalSD00: Limiting Process: " << limitingProcessName << G4endl;
//    G4cout << "SJA:TAG:BeamCalSD00: GlobalCellPos Volume Name: " << myVolumeName << G4endl;
//    G4cout << "SJA:TAG:BeamCalSD00: GlobalCellPos X: " << GlobalCellPos(0) << G4endl;
//    G4cout << "SJA:TAG:BeamCalSD00: GlobalCellPos Y: " << GlobalCellPos(1) << G4endl;
//    G4cout << "SJA:TAG:BeamCalSD00: GlobalCellPos Z: " << GlobalCellPos(2) << G4endl;
//    G4cout << "SJA:TAG:BeamCalSD00: localCellPos X: " << localCellPos->x() << G4endl;
//    G4cout << "SJA:TAG:BeamCalSD00: localCellPos Y: " << localCellPos->y() << G4endl;
//    G4cout << "SJA:TAG:BeamCalSD00: localCellPos Z: " << localCellPos->z() << G4endl;
//    exit(99);
//  }
//
//  delete myNavigator;
//
  // SJA:END: Let's try and find where this hit really is

  //SJA:FIXME: Not allowed negative layer numbers that is what side is for
  LrNum = abs(LrNum);

  int cellId = 0;
  cellId |= (LrNum << 0);
  cellId |= (PhiNum  << 8);
  cellId |= (RingNum   << 16); 

  G4double time = aStep->GetTrack()->GetGlobalTime();
  G4int PDG = aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
  G4int PID = Control::GetControl()->GetPIDForCalHit(aStep);
  G4int n_hit = CalCollection->entries();

//  G4cout << "SJA:TAG: For the Encoder " 
//	 << " side:" << side  
//	 << " RingNum:" << RingNum 
//	 << " PhiNum:" << PhiNum 
//	 << " LrNum:" << LrNum 
//	 << G4endl; 

  cell_ids theCode = theEncoder->encode(side,RingNum,PhiNum,LrNum,0,0);	

  G4bool found=false;
  for(int i_hit=0; i_hit<n_hit;i_hit++)
    {
      if((*CalCollection)[i_hit]->testCell(theCode))
	{
	  (*CalCollection)[i_hit]->AddEdep(PID,PDG,edep,time);
	  found=true;
	  break;
	}
    }	


  if(!found) 
    CalCollection-> insert(new CalHit(0,//Piece
				      side,//Stave
				      0,//Module
				      RingNum,//I
				      PhiNum, //J
				      LrNum,  //K
				      0,//GRZone
				      GlobalCellPos.x(),
				      GlobalCellPos.y(),
				      GlobalCellPos.z(),
				      edep,
				      PID,
				      PDG,
				      time,
				      theCode));
	


  return true;
}

void BeamCalSD00::EndOfEvent(G4HCofThisEvent *HCE)
{
  if (HCID < 0)
    {
      HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    }
  HCE->AddHitsCollection(HCID, CalCollection);

}

void BeamCalSD00::LoadEvent(FILE *eventFile)
{
  CalHit *newHit = new CalHit();
  while(newHit->Load(eventFile))
    {
      CalCollection->insert (newHit);
      newHit = new CalHit();
    }
  delete newHit;
}

