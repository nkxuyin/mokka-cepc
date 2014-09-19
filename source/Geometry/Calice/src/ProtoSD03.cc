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
// $Id: ProtoSD03.cc,v 1.13 2007/07/10 16:27:43 mora Exp $
// $Name: mokka-07-00 $
//
#include "Control.hh"
#include "ProtoSD03.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4TransportationManager.hh"
#include "G4ParticleDefinition.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "globals.hh"
#include "G4GeometryTolerance.hh"
#include "G4UImanager.hh"
#include <assert.h>
#include <math.h>

#include "G4AffineTransform.hh"

#include "Encoder64.hh"
#include "Encoder32.hh"

ProtoSD03:: ProtoSD03(G4double dimX,G4double dimZ, G4int n_cell_x, 
		G4int n_cell_z, G4int n_waffers_x, G4int n_waffers_z, 
		G4double upper_waffer_shift, G4double garde_size,
		std::vector<G4double>& cell_y_pos_in_alveolus, 
		std::vector<std::pair<G4double, G4double> >& alveolus_y_spacing, 
		G4double exit_fiber_thickness,
		std::vector<std::pair<G4double,G4double> > & a_shifts_vector,
		G4double a_struct_shift[3],G4double StructHalfX[3],
		G4ThreeVector *anEcalPosition, 
		G4RotationMatrix * anEcalRotation,
		G4String ProtoSD03name,
		G4int start_layer_number,
		G4double inter_wafer_gap,
		G4double halfAlveolusX,
		G4double halfEnvEcalX,
		G4double halfEcalX,
		G4double & halfEcalY,
		G4double inter_tower_fiber_thickness,
		G4double inter_structures_gap,
		G4int n_guard_ring_zones,
		G4double lateralWaferGap, G4double endcap_x, G4bool useID1)
: VSensitiveDetector(ProtoSD03name), CalCollection(0),
    HCID(-1),theDimX(dimX), theDimZ(dimZ),
    the_n_cell_x(n_cell_x), the_n_cell_z(n_cell_z),
    the_n_waffers_x(n_waffers_x), the_n_waffers_z(n_waffers_z),
    the_upper_waffer_shift(upper_waffer_shift), the_garde_size(garde_size),
    the_cell_y_pos_in_alveolus(cell_y_pos_in_alveolus),
    the_alveolus_y_spacing(alveolus_y_spacing),
    the_exit_fiber_thickness(exit_fiber_thickness),
    the_shifts_vector(a_shifts_vector),
    theEcalPosition(anEcalPosition),
    theEcalRotation(anEcalRotation),
    the_start_layer_number(start_layer_number),
    the_inter_wafer_gap(inter_wafer_gap),
    theHalfAlveolusX(halfAlveolusX),
    theHalfEnvEcalX(halfEnvEcalX),
    theHalfEcalX(halfEcalX), 
    theHalfEcalY(halfEcalY), 
    the_inter_tower_fiber_thickness(inter_tower_fiber_thickness),
    the_inter_structures_gap(inter_structures_gap),
    the_n_guard_ring_zones(n_guard_ring_zones),
    theLateralWaferGap(lateralWaferGap),
    the_endcap_x(endcap_x)
{
  assert (theDimX > 0);
  assert (theDimZ > 0);

  G4String CollName=ProtoSD03name+"Collection";
  collectionName.insert(CollName);

  the_struct_shifts[0] = a_struct_shift[0];
  the_struct_shifts[1] = a_struct_shift[1];
  the_struct_shifts[2] = a_struct_shift[2];

  theStructHalfX[0] = StructHalfX[0];
  theStructHalfX[1] = StructHalfX[1];
  theStructHalfX[2] = StructHalfX[2];

  if(useID1)
	theEncoder = new Encoder64();
  else
	theEncoder = new Encoder32();
}

ProtoSD03::~ProtoSD03()
{
}

void ProtoSD03::Initialize(G4HCofThisEvent *)
{
  //if(CalCollection!=0) delete CalCollection;
  CalCollection = new HitsCollection
    (SensitiveDetectorName,collectionName[0]); 
}

cell_ids ProtoSD03::GetCellIndex(double X, double Y, double Z, 
		int & flag, double xDir, double yDir,
		double zDir) {// cell = 1; ring = 0; else = -1
	
  G4ThreeVector point(X*mm, Y*mm, Z*mm);
  G4ThreeVector direction(xDir, yDir, zDir);
  
  G4TouchableHistory * theTouchable = new G4TouchableHistory();
  G4TransportationManager::GetTransportationManager()->
	GetNavigatorForTracking()->
	LocateGlobalPointAndUpdateTouchable(point, direction,
	       	theTouchable);

  G4int depth = theTouchable->GetHistory()->GetDepth();

  G4int I, J, WI, WJ, K, zone=0;
  if(theTouchable->GetHistory()->GetVolume(depth)->IsReplicated()) {
   GetCellIndices(theTouchable, I, J, WI, WJ, K);
   flag = 1;
  }
  else {
  	G4int copyNo = theTouchable->GetHistory()->GetVolume(depth)
	  ->GetCopyNo();
	if(abs(copyNo) > 10000) {
   		GetNearestCell(theTouchable, point, I, J, WI, WJ, K, zone);
   		flag = 0;
	}
	else {
		I = 0; J = 0; WI = 0; WJ = 0; K = 0; flag = -1;
	}
  }

  return theEncoder->encode(WI,WJ,I,J,K,zone);
}

void ProtoSD03::GetNearestCell(const G4VTouchable * theTouchable,
  G4ThreeVector thePosition,
  G4int & I, G4int & J, G4int & WI, G4int & WJ, G4int & K, G4int & zone) {

  G4int depth = theTouchable->GetHistory()->GetDepth();

  G4VPhysicalVolume * physVol = theTouchable->GetHistory()->GetVolume(depth);

  G4int WIWJ = physVol->GetCopyNo() % 10000;

  WI = WIWJ/10;
  WJ = WIWJ%10;
  G4int slabNo = theTouchable->GetHistory()->GetVolume(3)
	  ->GetCopyNo() % 100;
  K = slabNo * 2 + the_start_layer_number - 1;

  if(WIWJ < 0.) {
	WI = abs(WI);
	WJ = abs(WJ);
  	K = (slabNo - 1) * 2 + the_start_layer_number;
  }

  G4AffineTransform theAffineTransform= theTouchable->GetHistory()
	  ->GetTransform(depth);

  G4ThreeVector theLocalPosition = 
		  theAffineTransform.TransformPoint(thePosition);

  G4double mI = theLocalPosition(0)/theDimX;
  G4int mIint = static_cast<G4int>(mI);
  if(mI > 0) {
    if(the_n_cell_x == 2 * mIint)
	I = the_n_cell_x;
    else
	I = mIint + the_n_cell_x/2 + 1;
  }
  else {
    if(the_n_cell_x== - 2 * mIint)
	I = 1;
    else
	I = mIint + the_n_cell_x/2;
  }

  G4double mJ = theLocalPosition(2)/theDimZ;
  G4int mJint = static_cast<G4int>(mJ);
  if(mJ > 0) {
    if(the_n_cell_z == 2 * mJint)
	J = 1;
    else
	J = -mJint + the_n_cell_z/2;
  }
  else {
    if(the_n_cell_z== - 2 * mJint)
	J = the_n_cell_z;
    else
	J = -mJint + the_n_cell_z/2 + 1;
  }
  
  if(the_n_guard_ring_zones == 0)
	  return;

  G4double delta_x = fabs(theLocalPosition(0)) - theDimX * the_n_cell_x / 2;
  G4double delta_z = fabs(theLocalPosition(2)) - theDimZ * the_n_cell_z / 2;
  
  G4double zone_width = the_garde_size / the_n_guard_ring_zones;

  G4int zone_x = static_cast<G4int>(delta_x / zone_width); 
  G4int zone_z = static_cast<G4int>(delta_z / zone_width); 

  if((delta_x < 0) && (delta_z < 0))
          zone = -2;

  if((I != 1) && (I != the_n_cell_x))
	  zone = zone_z;

  else if((J != 1) && (J != the_n_cell_z))
	  zone = zone_x;
  
  else if((delta_x <= 0) && (delta_z >= 0))
	  zone = zone_z;
  
  else if((delta_z <= 0) && (delta_x >= 0))
	  zone = zone_x;

  else if((zone_x == 0) && (delta_z <= 0))
	  zone = 0;

  else if((zone_z == 0) && (delta_x <= 0))
	  zone = 0;

  else {
  	G4double radius = sqrt(delta_x * delta_x + delta_z * delta_z);

	zone = static_cast<G4int>(radius / zone_width);
  }

  if(zone >= the_n_guard_ring_zones)
	  zone = the_n_guard_ring_zones - 1;

/*
#ifdef MOKKA_DEBUG
  if (zone < 0) {
	  G4cout << " I: " << I << " J: " << J << " WIWJ: " << 
		 physVol->GetCopyNo() << " zone: " << zone << 
		 " zone width: " << zone_width << 
		 " delta_x: " << delta_x << " delta_z: " << 
		  delta_z << " local pos: " << theLocalPosition << G4endl;
          Control::Abort("ProtoSD03::GetNearestCell: Assertion failed (zone < 0)");
  }
#endif
*/
   //GM: 30.01.06 : new coding convention; zone = 0 if hit in cell;
   //                                      zone = 1, ..., theNGuardRingZones
   //                                              if hit in Guard Ring
   zone++;
}

void ProtoSD03::GetCellIndices(const G4VTouchable * theTouchable,
  G4int & I, G4int & J, G4int & WI, G4int & WJ, G4int & K) {

  G4int depth = theTouchable->GetHistory()->GetDepth();

  I = theTouchable->GetReplicaNumber() + 1;
	     
  J = theTouchable->GetReplicaNumber(1);

  J = the_n_cell_z - J;

  G4int WIWJ = theTouchable->GetHistory()->GetVolume(depth - 3)
	  ->GetCopyNo() % 10000;

  WI = WIWJ/10;
  WJ = WIWJ%10;
  G4int slabNo = theTouchable->GetHistory()->GetVolume(3)
	  ->GetCopyNo() % 100;
  K = slabNo * 2 + the_start_layer_number - 1;

  if(WIWJ < 0.) {
	WI = abs(WI);
	WJ = abs(WJ);
  	K = (slabNo - 1) * 2 + the_start_layer_number;
  }
}

G4ThreeVector ProtoSD03::GetCellCenter(G4int,G4int WI,G4int WJ,
	G4int I,G4int J,G4int K) {

   G4double X=0., Y=0., Z=0.;

   //first we look inside the Wafer
   X = -theDimX * (the_n_cell_x - 1) / 2 + (I - 1) * theDimX;
   Z =  theDimZ * (the_n_cell_z - 1) / 2 - (J - 1) * theDimZ;
   
   //then look at the waffer position in the active zone
   G4double wafer_shift, slab_shift;
   G4int nSlabs = the_shifts_vector.size() / 2;
   G4int nLayersPerStruct = static_cast<int>(the_shifts_vector.size()/3);
   G4int nSlabsPerStruct = static_cast<int>(nLayersPerStruct/2);

   if(WJ == 1)
     slab_shift = the_shifts_vector[static_cast<int>((K+1)/2)-1+nSlabs].first;
   else 
     slab_shift = the_shifts_vector[static_cast<int>((K+1)/2)-1].first;

   if(K%2 == 1)
    wafer_shift = the_upper_waffer_shift;
   else {
    if(WJ == 1)
     wafer_shift = the_upper_waffer_shift + 
	     the_shifts_vector[static_cast<int>((K+1)/2)-1+nSlabs].second;
    else 
     wafer_shift = the_upper_waffer_shift + 
	     the_shifts_vector[static_cast<int>((K+1)/2)-1].second;
   }

   G4double halfWaferX = the_n_cell_x * theDimX / 2 + the_garde_size;
   G4double halfWaferZ = the_n_cell_z * theDimZ / 2 + the_garde_size;
   G4double siLayerCenterX = - theHalfAlveolusX + 
	   (the_n_waffers_x * halfWaferX + 
	   (the_n_waffers_x - 1) * the_inter_wafer_gap / 2 + 
	   wafer_shift);
   G4double xDisp= -(2*halfWaferX+the_inter_wafer_gap)*(the_n_waffers_x-1)/2.
	   +(2*halfWaferX+the_inter_wafer_gap)*WI;

   X += (siLayerCenterX + xDisp);
   X += (-theStructHalfX[static_cast<int>((K-1)/nLayersPerStruct)] +
		theHalfAlveolusX + slab_shift);
   X += (-theHalfEnvEcalX 
		   + theStructHalfX[static_cast<int>((K-1)/nLayersPerStruct)] 
		   + fabs(the_struct_shifts[0]) + 
		 the_struct_shifts[static_cast<int>((K-1)/nLayersPerStruct)]);

   //X += (theHalfEnvEcalX-theHalfEcalX-fabs(the_struct_shifts[0]));

   Z += -(halfWaferZ + the_inter_wafer_gap/2); 
   Z += (2*halfWaferZ+the_inter_wafer_gap) * (the_n_waffers_z - WJ);
   if(WJ == 1)
   	Z += (2*theLateralWaferGap - the_inter_wafer_gap 
		       + the_inter_tower_fiber_thickness);

   // now look at the layer position inside the detector
   if(K%2 == 0)
   	Y = - the_alveolus_y_spacing
		[static_cast<int>((K-1)/nLayersPerStruct)].second / 2 +
	    the_cell_y_pos_in_alveolus
		[static_cast<int>((K-1)/nLayersPerStruct)];
   else 
   	Y = - the_alveolus_y_spacing
		[static_cast<int>((K-1)/nLayersPerStruct)].second  / 2-
       	      the_cell_y_pos_in_alveolus
	        [static_cast<int>((K-1)/nLayersPerStruct)];
	
   Y -= theHalfEcalY;
   for(int i1 = 0; i1 < static_cast<int>((K-1)/(nLayersPerStruct)) + 1; i1++) {
  	for(int i2 = i1*nSlabsPerStruct; 
		(i2 < (i1+1)*nSlabsPerStruct) && (i2 <= (K-1)/2); i2++) {
        	Y += the_alveolus_y_spacing[i1].first;
	}
	if(i1 != 0)
		Y += (the_exit_fiber_thickness + the_inter_structures_gap);
   }

   // then translate and rotate the position
   G4ThreeVector pos(X, Y, Z);

   pos = ((theEcalRotation->inverse()))*pos;
   pos += (*theEcalPosition);

   return pos;
}

G4bool ProtoSD03::ProcessHits(G4Step *aStep,G4TouchableHistory *) {

  if(aStep->GetStepLength() <= 
	G4GeometryTolerance::GetInstance()->GetSurfaceTolerance())
    return true;

  // process only if energy>0. except geantinos
  G4double edep;
  if ((edep = aStep->GetTotalEnergyDeposit())<=0. &&
      aStep->GetTrack()->GetDefinition()->GetParticleType()!="geantino") 
    return true;

  G4double time = aStep->GetTrack()->GetGlobalTime() ;

  const G4VTouchable * theTouchable = 
	  aStep->GetPreStepPoint()->GetTouchable();

  G4int depth = theTouchable->GetHistory()->GetDepth();
  
  G4ThreeVector origin;

  G4AffineTransform theAffineTransformation, theInverseAffineTransformation;

  theAffineTransformation = theTouchable->GetHistory()
	  ->GetTransform(depth);
  theInverseAffineTransformation = theAffineTransformation.Inverse();

  G4ThreeVector theCellCenter;

  G4int I, J, WI, WJ, K, zone = 0;

  if(theTouchable->GetHistory()->GetVolume(depth)->IsReplicated()) {
   GetCellIndices(theTouchable, I, J, WI, WJ, K);
   K = K - the_start_layer_number + 1;
   theCellCenter = 
	  theInverseAffineTransformation.TransformPoint(origin);
  }
  else {
   GetNearestCell(theTouchable, aStep->GetPreStepPoint()->GetPosition(), 
		   I, J, WI, WJ, K, zone);

   if(zone < 1)
           return false;

   K = K - the_start_layer_number + 1;
   theCellCenter = GetCellCenter(0, WI, WJ, I, J, K);
  }


#ifdef MOKKA_DEBUG
  G4ThreeVector thePosition =
	(aStep->GetPreStepPoint()->GetPosition()+
	aStep->GetPostStepPoint()->GetPosition())*0.5;
  G4ThreeVector distCenter = thePosition - theCellCenter;
  if (distCenter.mag() > (theDimX + theDimZ) / 2.) {
	G4cout << "======= ASSERT WILL CRASH :\n"
	<< ", WI = " << WI << ", WJ = " << WJ
	<< "\nI = " << I << ", J= " << J << ", K = " << K
	<< "\nthePosition = " << thePosition
	<< ", theCellCenter = " << theCellCenter
	<< ", distCenter.mag() = " << distCenter.mag() << G4endl;
        Control::Abort("ProtoSD03::ProcessHits: Assertion failed (distCenter.mag() > (theDimX + theDimZ) / 2.)",MOKKA_OTHER_ERRORS);
  }
#endif


  G4int PDG=
    aStep->GetTrack()->GetDefinition()->GetPDGEncoding();

  G4int n_hit = CalCollection->entries();

  cell_ids theCode = 
    theEncoder->encode(WI,WJ,I,J,K,zone);

  G4bool found=false;
  for(G4int i_hit = 0; i_hit< n_hit; i_hit++)
    if((*CalCollection)[i_hit]->testCell(theCode)) {
       (*CalCollection)[i_hit]->AddEdep(Control::primaryId,PDG,edep,time);
      found = true;
      break;
    }
  
  if(!found) {
	if(zone == 0) // the hit is inside a cell
		CalCollection->
			insert(new CalHit(PROTOMODULE, 
				  WI, 
				  WJ, 
				  I, 
				  J, 
				  K,
				  zone,
				  theCellCenter (0),
				  theCellCenter (1),
				  theCellCenter (2),
				  edep, Control::primaryId, PDG,time,theCode));
	else          // the hit is in the guard-ring
		CalCollection->
			insert(new CalHit(GRBARREL,
				  WI, 
				  WJ, 
				  I, 
				  J, 
				  K,
				  zone,
				  theCellCenter (0),
				  theCellCenter (1),
				  theCellCenter (2),
				  edep, Control::primaryId, PDG,time,theCode));
  }

  return true;
}

void ProtoSD03::EndOfEvent(G4HCofThisEvent*HCE)
{
  if(HCID<0)
    { HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]); }
  HCE->AddHitsCollection( HCID, CalCollection );
}

void ProtoSD03::clear()
{
} 

void ProtoSD03::DrawAll()
{
} 

void ProtoSD03::PrintAll()
{
} 

void ProtoSD03::LoadEvent(FILE* theSubDetectorEventHitsFileInput)
{
  CalHit* newHit = new CalHit();
  while (newHit->Load(theSubDetectorEventHitsFileInput))
    {
      CalCollection->insert(newHit);
      newHit = new CalHit();
    }
  delete newHit;
}



