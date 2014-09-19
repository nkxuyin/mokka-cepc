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
// $Id: SEcalSD03.cc,v 1.7 2009/06/04 11:40:42 musat Exp $
// $Name: mokka-07-00 $
//
#include "Control.hh"
#include "SEcalSD03.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4ParticleDefinition.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "globals.hh"
#include <assert.h>

#include "CGAGeometryManager.hh"
#include "G4VProcess.hh"

#include "G4ThreeVector.hh"

#include "Encoder64.hh"
#include "Encoder32.hh"

SEcalSD03::
SEcalSD03(G4double Idim, G4double Jdim, G4double Thickness,
	  G4int the_n_cells_i, G4int the_n_cells_j,
	  G4double theGardRingSize, G4int the_n_strip_containers_along_z,
	  G4String the_Ecal_Sc_Si_mix,
	  G4double theHWallSize, G4double theTowerWallSize,
	  G4int Piece, G4String SDname, G4bool id1Flag,
	  G4String theBarrelSlabMode, G4String theECSlabMod) 
  : SEcalSD02(Idim, Jdim, Thickness,
          the_n_cells_i, the_n_cells_j,
          theGardRingSize, theHWallSize,
          theTowerWallSize,
          Piece, SDname, id1Flag,
          theBarrelSlabMode, theECSlabMod),stripSizeinX(Idim),
	  stripSizeParallelToZ(Jdim),Ecal_Sc_thickness(Thickness),
	  Ecal_Sc_N_strips_across_module(the_n_cells_i),
	  Ecal_Sc_number_of_virtual_cells(the_n_cells_j),
	  Ecal_Sc_reflector_thickness(theGardRingSize),
	  n_strip_containers_along_z(the_n_strip_containers_along_z),
	  Ecal_Sc_Si_mix(the_Ecal_Sc_Si_mix),
	  HCID3(-1)
{

      if(Ecal_Sc_N_strips_across_module == 0)
        Control::Abort("SEcalSD03: Parameter \"Ecal_Sc_N_strips_across_module\" must be different from zero",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);

      if(Ecal_Sc_number_of_virtual_cells == 0)
        Control::Abort("SEcalSD03: Parameter \"Ecal_Sc_number_of_virtual_cells\" must be different from zero",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);


      //Dimension of a strip virtual cell
      virtualCellDim = stripSizeParallelToZ / 
			Ecal_Sc_number_of_virtual_cells;

      FirstLayerCollectionFlag = false;
      ParallelToZFlag = false;
      ParallelToXFlag = false;

      collectionName.clear();
      G4String FirstLayerCollectionType;

      unsigned int i_char = 0;
      switch(Ecal_Sc_Si_mix[i_char]) {
                case ECAL_SI_LAYERS:
                        FirstLayerCollectionFlag = false;
                        break;
                case ECAL_SC_LAYER_1_2_ALONG_X:
			ParallelToXFlag = true;
			FirstLayerCollectionFlag = true;
			FirstLayerCollectionType = "LongitudinalStrips";
			break;
                case ECAL_SC_LAYER_1_ALONG_X_LAYER_2_ALONG_Z:
			ParallelToZFlag = true;
                        FirstLayerCollectionFlag = true;
                        FirstLayerCollectionType = "LongitudinalStrips";
                        break;
                case ECAL_SC_LAYER_1_2_ALONG_Z:
                        ParallelToZFlag = true;
                        FirstLayerCollectionFlag = true;
                        FirstLayerCollectionType = "TransverseStrips";
                        break;
                case ECAL_SC_LAYER_1_ALONG_Z_LAYER_2_ALONG_X:
			ParallelToXFlag = true;
                        FirstLayerCollectionFlag = true;
                        FirstLayerCollectionType = "TransverseStrips";
                        break;
                default:
                        Control::Abort("SEcalSD03: The Ecal_Sc_Si_mix parameter should contain only 0's, 1's, 2's, 3's or 4's",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
        };

	if(FirstLayerCollectionFlag) {
		  G4String CollName1=SDname+FirstLayerCollectionType+
					"PreShowerCollection";
		  collectionName.insert(CollName1);
        };

	for(i_char = 1; i_char < Ecal_Sc_Si_mix.size(); i_char++)
          switch(Ecal_Sc_Si_mix[i_char]) {
                case ECAL_SI_LAYERS:
			break;
                case ECAL_SC_LAYER_1_2_ALONG_X:
			ParallelToXFlag = true;
			break;
                case ECAL_SC_LAYER_1_2_ALONG_Z:
			ParallelToZFlag = true;
			break;
                case ECAL_SC_LAYER_1_ALONG_X_LAYER_2_ALONG_Z:
			ParallelToXFlag = true;
			ParallelToZFlag = true;
			break;
                case ECAL_SC_LAYER_1_ALONG_Z_LAYER_2_ALONG_X:
			ParallelToXFlag = true;
			ParallelToZFlag = true;
                        break;
                default:
                        Control::Abort("SEcal04: The Ecal_Sc_Si_mix parameter should contain only 0's, 1's, 2's, 3's or 4's",MOKKA_ERROR_BAD_GLOBAL_PARAMETERS);
          };

	if(ParallelToZFlag) {
		  G4String CollName2=SDname+"TransverseStrips";
		  collectionName.insert(CollName2);
        };

	if(ParallelToXFlag) {
		  G4String CollName3=SDname+"LongitudinalStrips";
		  collectionName.insert(CollName3);
        };
}

SEcalSD03::~SEcalSD03()
{
}

void SEcalSD03::Initialize(G4HCofThisEvent *)
{
  G4int offset = 0;

  if(FirstLayerCollectionFlag)
  	FirstLayerCalCollection = new HitsCollection
    		(SensitiveDetectorName,collectionName[offset++]);

  if(ParallelToZFlag)
  	NormalParallelToZCalCollection = new HitsCollection
    		(SensitiveDetectorName,collectionName[offset++]);

  if(ParallelToXFlag)
  	NormalParallelToXCalCollection = new HitsCollection
    		(SensitiveDetectorName,collectionName[offset]);
}

void SEcalSD03::EndOfEvent(G4HCofThisEvent*HCE)
{
  G4int offset = 0;

  if(FirstLayerCollectionFlag) {
  	if(HCID1 <0) HCID1 = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[offset++]);
  	HCE->AddHitsCollection( HCID1, FirstLayerCalCollection );
  }

  if(ParallelToZFlag) {
  	if(HCID2 < 0) HCID2 = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[offset++]);
  	HCE->AddHitsCollection( HCID2, NormalParallelToZCalCollection );
  }

  if(ParallelToXFlag) {
  	if(HCID3 < 0) HCID3 = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[offset]);
  	HCE->AddHitsCollection( HCID3, NormalParallelToXCalCollection );
  }
}

G4ThreeVector SEcalSD03::GetScintillatorCellCenter(G4int ,G4int pS,G4int pM,
				       G4int pI,G4int pJ,G4int pK,
				       char direction) 
{
//  assert (pP >=ECALENDCAPMINUS);
//  assert (pP <= ECALENDCAPPLUS);
  assert (pS>0);
  assert (pM>=0);
  assert (pI>=0);
  assert (pJ>=0);
  assert (pK>0);
  assert(pS<=MAX_STAVES);
  assert(pM<MAX_MODULES);
  assert (pK<=MAX_LAYERS);

  G4ThreeVector localCellCenter;
  if((pM != 6) && (pM != 0))
    {
      // Barrel
      G4double X0 = Layers[pK-1]->X0 + StandardXOffset;

      if(direction == 'z')
      {
        // I = X direction
      	localCellCenter[0]= X0 + pI * stripSizeinX + stripSizeinX / 2;
      
        // upwards direction 
        localCellCenter[1]=Layers[pK-1]->Y0;

        // J = Z direction (virtual cells)
      
        G4int N_strips_before_z = static_cast<G4int>
	 	 (pJ / Ecal_Sc_number_of_virtual_cells);
      
        G4int N_cells_before_z =
	         N_strips_before_z * Ecal_Sc_number_of_virtual_cells;
      
        G4int N_towers_before_z = static_cast<G4int>
		 (N_strips_before_z / Ecal_Sc_N_strips_across_module);
      
        localCellCenter[2]= Layers[pK-1]->Z0 
	  + N_strips_before_z * stripSizeParallelToZ
	  + N_towers_before_z * 2 * ( TowerWallSize + HWallSize )
	  + Ecal_Sc_reflector_thickness
	  + (pJ - N_cells_before_z) * virtualCellDim
	  + virtualCellDim / 2.;
      }//end if(direction == 'z')
      else {  //  direction == 'x'

        // I = X direction (virtual cells)

        // We neglect the effect of twice the Ecal_Sc_reflector_thickness 
	// (2 X 0.057 mm) that is missing if there is a longer last strip.
	G4int N_strips_before_x = static_cast<G4int>
                 (pI / Ecal_Sc_number_of_virtual_cells);
      	localCellCenter[0]= X0 + 
			N_strips_before_x * stripSizeParallelToZ +
			Ecal_Sc_reflector_thickness +
			(pI % Ecal_Sc_number_of_virtual_cells)*virtualCellDim +
			virtualCellDim / 2.;

        // upwards direction 
        localCellCenter[1]=Layers[pK-1]->Y0;

        // J = Z direction 
        G4int N_towers_before_z = static_cast<G4int>
		 (pJ / n_strip_containers_along_z);
      
        localCellCenter[2] = 
	  Layers[pK-1]->Z0 
	  + pJ * stripSizeinX
	  + N_towers_before_z * 2 * ( TowerWallSize + HWallSize )
	  + stripSizeinX / 2.;
      } // end if direction == 'x'
    } // end if Barrel
  else
    {
      // EndCaps
      G4double X0 = Layers[pK-1]->X0;

      if(direction == 'z') {
	// I direction (virtual cells)
        G4int N_strips_before_x = static_cast<G4int>
                 (pI / Ecal_Sc_number_of_virtual_cells);

        G4int N_cells_before_x =
                 N_strips_before_x * Ecal_Sc_number_of_virtual_cells;

        G4int N_towers_before_x = static_cast<G4int>
                 (N_strips_before_x / Ecal_Sc_N_strips_across_module);

      // builds the cell center coodinates for I,J
      localCellCenter[0]=
        X0
        // + StandardXOffset
          + N_strips_before_x * stripSizeParallelToZ
          + N_towers_before_x * 2 * ( TowerWallSize + HWallSize )
          + Ecal_Sc_reflector_thickness
          + (pI - N_cells_before_x) * virtualCellDim
          + virtualCellDim / 2.;


      localCellCenter[1]=Layers[pK-1]->Y0;

      localCellCenter[2]= 
		Layers[pK-1]->Z0 
		+ pJ * stripSizeinX + stripSizeinX / 2;

      } // end if direction == 'z'
      else {  //  direction == 'x'

      
         G4int N_towers_before_x = static_cast<G4int>
	                 (pI / n_strip_containers_along_z);

         // builds the cell center coodinates for I,J
         localCellCenter[0]= 
             X0
             // + StandardXOffset
             + pI * stripSizeinX
             + N_towers_before_x * 2 * ( TowerWallSize + HWallSize )
             + stripSizeinX / 2.;

      
          localCellCenter[1]=Layers[pK-1]->Y0;
 
          // We neglect the effect of twice the Ecal_Sc_reflector_thickness
          // (2 X 0.057 mm) that is missing if there is a longer last strip.

          G4int N_strips_before_z = static_cast<G4int>
                 (pJ / Ecal_Sc_number_of_virtual_cells);

          localCellCenter[2]=
                       Layers[pK-1]->Z0 +
                       N_strips_before_z * stripSizeParallelToZ +
                       Ecal_Sc_reflector_thickness +
                       (pJ % Ecal_Sc_number_of_virtual_cells)*virtualCellDim +
                       virtualCellDim / 2.;
       } //  end if direction == 'x'

      // X grows against +X in end caps
      localCellCenter.setX(-localCellCenter[0]);
      // Y <-> Z in endcaps
      G4double temp = localCellCenter[1];
      localCellCenter.setY(localCellCenter[2]);
      localCellCenter.setZ(temp);
    } // end if EndCaps
  
  // find out the actual cell center coodinates
  assert (InverseStavesRotationMatrices[pS-1]!=0);
  G4ThreeVector theCellCenter = 
    *InverseStavesRotationMatrices[pS-1] * localCellCenter;
  
  //if(pP == ECALENDCAPMINUS)
  if(pM == 0)
    {
      theCellCenter[2] += *ModulesZOffsets[6];
      G4RotationMatrix rot1;
      rot1.rotateY(pi);
      theCellCenter = rot1 * theCellCenter;
    }
  else
      theCellCenter[2] += *ModulesZOffsets[pM];
  

  return theCellCenter;
}

G4bool SEcalSD03::ProcessHits(G4Step *aStep,G4TouchableHistory *)
{
  // process only if energy>0. except geantinos
  G4double edep;
  if ((edep = aStep->GetTotalEnergyDeposit())<=0. &&
      aStep->GetTrack()->GetDefinition()->GetParticleType()!="geantino") 
    return true;
  G4double time = aStep->GetTrack()->GetGlobalTime() ;  

  // hit will deposited in the midle of the step
  G4ThreeVector thePosition = 
    (aStep->GetPreStepPoint()->GetPosition()+
     aStep->GetPostStepPoint()->GetPosition())*0.5;
  
  // Find out the stave and module id looking for the
  // module copy number and decoding it
  const G4VTouchable *history =aStep->GetPreStepPoint()->GetTouchable();
  G4int copyNumber;

  // Stave (if end cap), Tower and layer
  copyNumber = history->GetVolume(4)->GetCopyNo();
  G4int theStave = 0;
  G4int n_tower, theLayer;
  theStave = copyNumber / 100000;
  n_tower = (copyNumber - theStave*100000) /1000;
  theLayer = copyNumber % 1000;
  
  // if theStave > 0 => endcap!
  // SDPiece, Stave and module
  copyNumber = history->GetVolume(5)->GetCopyNo();
  G4int theSDPiece = 0;
  G4int theModule = 6; // Default endcap module
  if(theStave == 0)
    {
      theSDPiece = copyNumber/100;
      theStave = (copyNumber - theSDPiece*100)/10;
      theModule = (copyNumber - theSDPiece*100)%10;
    }
  else
    {
      theSDPiece =  copyNumber;
    }

  assert (StavesPhirots[theStave-1]!=0);
  assert (StavesRotationMatrices[theStave-1]!=0);


  // find out local position in the standard module reference
  G4ThreeVector localPosition;

  if(theSDPiece == ECALBARREL)
    {
      // barrel
      localPosition = 
	*StavesRotationMatrices[theStave-1] * thePosition;
      localPosition [0] -= StandardXOffset;
    }
  else
    {
      // endcaps
      // For the endcaps, the end cap reference is the Z>0 one.
      // So if Z<0, rotate the position to the positive one.
      G4ThreeVector tmpPosition = thePosition;
      if(tmpPosition(2)<0.)
	{
	  G4RotationMatrix rot1;
	  rot1.rotateY(pi);
	  tmpPosition  = rot1 * tmpPosition;
	}
      // find out local position in the standard module reference
      localPosition = 
	*StavesRotationMatrices[theStave-1] * tmpPosition;
      
    }


  // calculates I,J
  G4int I=0,J=0;

  assert (theLayer>0);

  assert (Layers[theLayer-1]!=0);
  assert (ModulesZOffsets[theModule]!=0);

  char direction = '0';
  G4int n_container_x = 0, n_strip_Z = 0;
  G4int n_container_z = 0, n_strip_X = 0;

  if(theSDPiece==ECALBARREL)
    {
      // BARREL
	direction = 'x';
	copyNumber = history->GetVolume(2)->GetCopyNo();
	if(copyNumber > 10000) {
		direction = 'z';
		n_container_x = copyNumber % 10000;
		n_strip_Z = history->GetVolume(1)->GetCopyNo() + 1;
		if((theLayer %2 ) == 0)
			n_strip_Z = Ecal_Sc_N_strips_across_module 
				- n_strip_Z + 1;
	}
        else {
		n_container_z = copyNumber + 1;
		if((theLayer %2 ) == 0)
			n_container_z = n_strip_containers_along_z
				- n_container_z + 1;

		n_strip_X = history->GetVolume(1)->GetCopyNo();
	}

      if(direction == 'z')
      {
        // I = X direction: start counting from zero
	I = n_container_x - 1;
		
        // displacement from the Z0 offset
        G4double z_size_before_tower =
               (n_tower -1) * (2*(TowerWallSize + HWallSize) +
			Ecal_Sc_N_strips_across_module *
			stripSizeParallelToZ);

        // The Z0 offset starts at alveolus boundary
        G4double z_starting_strip =
               z_size_before_tower +
               Ecal_Sc_reflector_thickness;

        z_starting_strip +=
	  (n_strip_Z - 1) * stripSizeParallelToZ;
	 
        // z in local strip 
        G4double z_in_strip = 
		localPosition(2) - 
		*ModulesZOffsets[theModule] -
		Layers[theLayer-1]->Z0 -
		z_starting_strip;
      
        // J = Z direction: start counting from zero
        J = static_cast<G4int>(z_in_strip/virtualCellDim) + 
	   (n_tower -1) * Ecal_Sc_N_strips_across_module * 
			Ecal_Sc_number_of_virtual_cells +
	   (n_strip_Z - 1) * Ecal_Sc_number_of_virtual_cells;
      } //end if(direction == 'z')
      else {  //  direction == 'x'
        // J = Z direction: start counting from zero
        J = n_container_z - 1 +
		(n_tower - 1) * n_strip_containers_along_z;

	G4double x_starting_strip = (n_strip_X - 1) * 
		stripSizeParallelToZ + Ecal_Sc_reflector_thickness;

	G4double x_in_strip = localPosition(0) -
				Layers[theLayer-1]->X0 -
				x_starting_strip;

        // I = X direction: start counting from zero
        I = static_cast<G4int>( x_in_strip / virtualCellDim ) +
        	(n_strip_X - 1) * Ecal_Sc_number_of_virtual_cells;
      }
    }
  else
    {
      // END CAPS

	direction = 'x';
	copyNumber = history->GetVolume(2)->GetCopyNo();
	if(copyNumber > 10000) {
		direction = 'z';
		n_container_x = copyNumber % 10000;
		n_strip_Z = history->GetVolume(1)->GetCopyNo() + 1;
      		if ( ECSlabMode == "0110" && theLayer % 2 != 0 )
			n_strip_Z = Ecal_Sc_N_strips_across_module 
				- n_strip_Z + 1;
	}
        else {
		n_container_z = copyNumber + 1;
      		if ( ECSlabMode == "0110" && theLayer % 2 != 0 )
			n_container_z = n_strip_containers_along_z
				- n_container_z + 1;

		n_strip_X = history->GetVolume(1)->GetCopyNo();
	}

//====================================================
//G4cout << "\ntheSDPiece = " << theSDPiece
//   	     << ", theStave = " << theStave 
//   	     << ", theModule = " << theModule
//		<< " ,theLayer = " << theLayer
//		<< " ,n_tower = " << n_tower 
//		<< " ,n_container_x = " << n_container_x 
//		<< " ,n_strip_Z = " << n_strip_Z 
//		<< " ,direction = " << direction 
//   	        << "\nthePosition = " << thePosition
//              << "TrackID = " << aStep->GetTrack()->GetTrackID()  		
//		<< G4endl;
//=================================================

      // After turning the slabs in the end caps,it become really
      // confuse. What we call below "z" is in fact "x", and what
      // we call below "z" is in fact "y". We keep it as it's because
      // we reuse the old code already tested, which is already
      // an adaptation from the barrel code. So, to complicate a bit
      // more things, Y<->Z in end caps. But it works fine...
 
      if(theSDPiece==ECALENDCAPMINUS)
		theModule = 0;

      if(direction == 'z') {
        // J = longer dimension of the slab: start counting from zero
        J = n_container_x - 1;

        // displacement from the Z0 offset
        G4double z_size_before_tower =
               (n_tower -1) * (2*(TowerWallSize + HWallSize) +
                        Ecal_Sc_N_strips_across_module *
                        stripSizeParallelToZ);

       // The Z0 offset starts at alveolus boundary
       G4double z_starting_strip =
               z_size_before_tower +
               Ecal_Sc_reflector_thickness;

       z_starting_strip +=
          (n_strip_Z - 1) * stripSizeParallelToZ;

       // x in local strip x offset
       G4double z_in_strip =
          - localPosition(0) -
          Layers[theLayer-1]->X0 -
          z_starting_strip;

        // I = shorter dimension of the slab: start counting from zero
        I = static_cast<G4int>(z_in_strip/virtualCellDim) +
           (n_tower -1) * Ecal_Sc_N_strips_across_module *
                        Ecal_Sc_number_of_virtual_cells +
           (n_strip_Z - 1) * Ecal_Sc_number_of_virtual_cells;

      } //end if(direction == 'z')
      else {  //  direction == 'x'
        // I = shorter dimension of the slab: start counting from zero
        I = n_container_z - 1 +
                (n_tower - 1) * n_strip_containers_along_z;

        G4double x_starting_strip = (n_strip_X - 1) *
                stripSizeParallelToZ + Ecal_Sc_reflector_thickness;

        G4double x_in_strip = localPosition(1) -
			        Layers[theLayer-1]->Z0 -
                                x_starting_strip;

        // J = longer dimension of the slab: start counting from zero
        J = static_cast<G4int>( x_in_strip / virtualCellDim ) +
                (n_strip_X - 1) * Ecal_Sc_number_of_virtual_cells;
      }// end if direction == 'x'
    } // end if END CAPS

  assert (theSDPiece==ECALENDCAPMINUS || 
	  theSDPiece==ECALENDCAPPLUS ||
	  theSDPiece==ECALBARREL);
  
  assert (I>=0);
  assert (J>=0);
  
  // find out the actual cell center coodinates
  G4ThreeVector theCellCenter = 
    GetScintillatorCellCenter(theSDPiece, theStave, theModule, I, J, theLayer,direction);
  
#ifdef MOKKA_DEBUG
  G4ThreeVector theCellDim(stripSizeinX,Ecal_Sc_thickness,virtualCellDim);

  // test if the cell center is not so far way the current
  // step point.
  G4ThreeVector distCenter = thePosition - theCellCenter;
  if (distCenter.mag() > theCellDim.mag()*sqrt(2.))
    {
      G4cout << "======= ASSERT WILL CRASH :\n"
	     << "\ntheSDPiece = " << theSDPiece
	     << ", theStave = " << theStave 
	     << ", theModule = " << theModule
	     << "\nI = " << I << ", J= " << J << ", K = " << theLayer
	     << "\nthePosition = " << thePosition
	     << ", localPosition = " << localPosition
	     << ", \ntheCellCenter = " << theCellCenter
	     << ", distCenter = " << distCenter 
	     << ", distCenter.mag() = "
	     << distCenter.mag() 
	     << ", theCellDim.mag()/2 = "
	     << theCellDim.mag()/2 << G4endl;

      Control::Abort("SEcalSD03::ProcessHits: Assertion failed (distCenter.mag() > theCellDim.mag()/2)",MOKKA_OTHER_ERRORS);
    }
#endif

  // creates a new cell or add< the energy to the cell if it already exists
  
  G4int PID = 
    Control::GetControl()->GetPIDForCalHit(aStep);
  assert(PID!=-1);

  G4int PDG=
    aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
  
  G4bool found=false;
  HitsCollection *CalCollection = 0;
  if (theLayer > 1)
    {
      if(direction == 'z')
      	CalCollection = NormalParallelToZCalCollection ;
      else if(direction == 'x')
      	CalCollection = NormalParallelToXCalCollection  ;
      theLayer --;
    }
  else
    {
      CalCollection = FirstLayerCalCollection;
      theSDPiece = -theSDPiece;
    }

  G4int n_hit = CalCollection->entries();

  cell_ids theCode = 
    theEncoder->encode(theStave,theModule,
		   I,J,theLayer,0);
  
  for(G4int i_hit = 0; i_hit< n_hit; i_hit++)
    if((*CalCollection)[i_hit]->
       testCell(theCode)) {
      (*CalCollection)[i_hit]->AddEdep(PID,PDG,edep,time);
      found = true;
      break;
    }
  
  if(!found) CalCollection->
	       insert(new CalHit (theSDPiece,
				  theStave,
				  theModule,
				  I,
				  J,
				  theLayer,
				  0,
				  theCellCenter (0),
				  theCellCenter (1),
				  theCellCenter (2),
				  edep,
				  PID,
				  PDG,
				  time,
				  theCode));
  return true;
}

