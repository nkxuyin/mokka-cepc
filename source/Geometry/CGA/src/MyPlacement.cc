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
// $Id: MyPlacement.cc,v 1.5 2008/10/21 15:38:42 engels Exp $
// $Name: mokka-07-00 $
//


#include "Control.hh"
#include <assert.h>
#include <algorithm>
#include "MyPlacement.hh"
#include "SD.hh"
#include "G4DisplacedSolid.hh"


// **************************************
// ATTENTION!!!!! Definitions "hard wired" 
// **************************************

#define ECAL_ENDCAPS_NSHAPES 3
#define ECAL_ENDCAPS_ENVELOP_LEVEL 2
#define ECAL_ENDCAPS_LAYER_LEVEL 3
#define BARREL_ENVELOP_LEVEL 2

// **************************************

LV MyPlacement::LVs [MAX_LVS];
G4int MyPlacement::n_LVs = 0;

G4int MyPlacement::n_MATs=0;

G4int MyPlacement::SolidNumber = -1;
FILE * MyPlacement::FVOLS = 0;

void MyPlacement::Open()
{
  G4cout << "\nBRAHMS backward mode is on, writing Fortran code." << G4endl;
}

void MyPlacement::InsertComment(const char* txt)
{
  if(Control::DUMPG3) fprintf(FVOLS,"C\nC %s\nC\n",txt);
}

void MyPlacement::Init(G4String Detector, G4String database) 
{

  if(FVOLS != 0) fprintf(FVOLS,"\tEND\n");
  else FVOLS = fopen("g4g3.f","w");
  
  fprintf(FVOLS,"C*********************************************************\n");
  fprintf(FVOLS,"C\nC This code was generate automatically by Mokka, the \n");
  fprintf(FVOLS,"C Geant4 simulation software for Tesla.\n");
  fprintf(FVOLS,"C\nC  BE CAREFUL : you should setup the parameters in the g4g3.inc\n");
  fprintf(FVOLS,"C   file before binding it with your Geant3 application!\n");
  fprintf(FVOLS,"C\nC Current database release is %s\nC\n",database.data());
  fprintf(FVOLS,"C\nC   (Developped by LPNHE -Ecole Polytechnique, France.)\n");
  fprintf(FVOLS,"C*********************************************************\n");
  fprintf(FVOLS,"\tSUBROUTINE G4G3%s\n",Detector.data());
  fprintf(FVOLS,"\tIMPLICIT NONE\n");
  fprintf(FVOLS,"C Parameters include file\n");
  fprintf(FVOLS,"\tINCLUDE g4g3.inc\n");
  fprintf(FVOLS,"C scratch\n");
  fprintf(FVOLS,"\tINTEGER IROT,IVOLU\n\tREAL PAR(11)\n");

}

void MyPlacement::Close(){
  fprintf(FVOLS,"\tEND\n");
  WriteCellMapping();
  fclose(FVOLS);

  for (G4int i=0; i< n_LVs; i++)
    if(LVs[i].boolShape != 0) {
      delete LVs[i].boolShape;
      LVs[i].boolShape = 0;
      LVs[i].theSD = 0;
    }
}

MyPlacement::MyPlacement(G4RotationMatrix *pRot, 
			 const G4ThreeVector &tlate,
			 G4LogicalVolume *pCurrentLogical,
			 const G4String& pName,
			 G4LogicalVolume *pMotherLogical,
			 G4bool pMany,
			 G4int pCopyNo,
			 G4bool pSurfCheck) :
  G4PVPlacement(pRot,tlate,pCurrentLogical,pName,pMotherLogical,pMany,pCopyNo,pSurfCheck)
{
  if(Control::DUMPG3) {
    
    G4int theMotherLV = DescribeLogical(pMotherLogical);

    PlaceLogical(pName,
		 pRot,
		 tlate,
		 pCopyNo,
		 theMotherLV,
		 DescribeLogical(pCurrentLogical));
  }
}

void MyPlacement::PlaceLogical(G4String pName,
			       G4RotationMatrix *pRot,
			       const G4ThreeVector &tlate,
			       G4int pCopyNo,
			       G4int theMotherLV,
			       G4int theLV){
  
  G4int NR;
  if(pCopyNo != 0) NR = pCopyNo;
  else  NR = ++LVs[theLV].nCopy;
  
  fprintf(FVOLS,"C Placement %s\n",pName.data());

  DescribeRotation(pRot);

  G4ThreeVector Place = tlate;
  G4String Mode = "'ONLY'";
  if (LVs[theLV].boolShape!=0) Mode = "'MANY'";

  if (LVs[theLV].boolShape!=0 && LVs[theMotherLV].boolShape==0) {
    G4ThreeVector 
      BoxDelta(LVs[theLV].boolShape->envelope->GetXHalfLength() - 
	       LVs[theLV].boolShape->composants[0].XH-
	       LVs[theLV].boolShape->deltaCenter.x(),
	       LVs[theLV].boolShape->envelope->GetYHalfLength() - 
	       LVs[theLV].boolShape->composants[0].YH-
	       LVs[theLV].boolShape->deltaCenter.y(),
	       LVs[theLV].boolShape->envelope->GetZHalfLength() - 
	       LVs[theLV].boolShape->composants[0].ZH-
	       LVs[theLV].boolShape->deltaCenter.z());
    
    
    BoxDelta = pRot->inverse()*BoxDelta;

    Place[0]=Place(0)+ BoxDelta(0);
    Place[1]=Place(1)+ BoxDelta(1);
    Place[2]=Place(2)+ BoxDelta(2);
  }

  if (LVs[theMotherLV].boolShape!=0 && LVs[theLV].boolShape==0) {

    G4ThreeVector 
      BoxDelta(LVs[theMotherLV].boolShape->envelope->GetXHalfLength() - 
	       LVs[theMotherLV].boolShape->composants[0].XH-
	       LVs[theMotherLV].boolShape->deltaCenter.x(),
	       LVs[theMotherLV].boolShape->envelope->GetYHalfLength() - 
	       LVs[theMotherLV].boolShape->composants[0].YH-
	       LVs[theMotherLV].boolShape->deltaCenter.y(),
	       LVs[theMotherLV].boolShape->envelope->GetZHalfLength() - 
	       LVs[theMotherLV].boolShape->composants[0].ZH-
	       LVs[theMotherLV].boolShape->deltaCenter.z());
    
    Place[0]=Place(0)- BoxDelta(0);
    Place[1]=Place(1)- BoxDelta(1);
    Place[2]=Place(2)- BoxDelta(2);
  }
  
  fprintf(FVOLS,
	  "\tCALL GSPOS (%s,%d,%s,%f,%f,%f,\n     * IROT,%s)\n",
	  LVs[theLV].G3GSName.data(),
	  NR,
	  LVs[theMotherLV].G3GSName.data(),
	  Place.x()/cm,
	  Place.y()/cm,
	  Place.z()/cm,
	  Mode.data());

}

G4double MyPlacement::ToDegrees(G4double angle)
{
  return angle*180/pi;
}

void MyPlacement::DescribeRotation(G4RotationMatrix *pRot)
{
  if (pRot == 0)
    fprintf(FVOLS,"\tIROT=0\n");
  else {
    n_MATs++;
    G4RotationMatrix  aRotM;   // Initialised to identity
    aRotM= pRot->inverse();
    fprintf(FVOLS,"\tCALL GSROTM(IROTBASE+%d,%f,%f,\n     *%f,%f,%f,%f)\n",
	    n_MATs,
	    ToDegrees(aRotM.thetaX()),
	    ToDegrees(aRotM.phiX()),
	    ToDegrees(aRotM.thetaY()),
	    ToDegrees(aRotM.phiY()),
	    ToDegrees(aRotM.thetaZ()),
	    ToDegrees(aRotM.phiZ()));
    fprintf(FVOLS,"\tIROT=IROTBASE+%d\n",n_MATs);
  }
}

G4int MyPlacement::TubParameters(G4VSolid* theSolid,MyBoolShape* boolShape)
{
  G4int npar =1;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,((G4Tubs*)theSolid)->GetInnerRadius()/cm);
  npar++;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,((G4Tubs*)theSolid)->GetOuterRadius()/cm);
  npar++;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,((G4Tubs*)theSolid)->GetZHalfLength()/cm);
  if(boolShape!=0) {
    G4cout << "TubP cannot be placed as boolean!!!! STOP.\n";
    exit(1);
//     boolShape->composants[boolShape->nComp].XH=
//       ((G4Box*)theSolid)->GetXHalfLength();
//     boolShape->composants[boolShape->nComp].YH=
//       ((G4Box*)theSolid)->GetYHalfLength();
//     boolShape->composants[boolShape->nComp].ZH=
//       ((G4Box*)theSolid)->GetZHalfLength();
  }
  return npar;
  
}

G4int MyPlacement::BoxParameters(G4VSolid* theSolid,MyBoolShape* boolShape)
{
  G4int npar =1;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,((G4Box*)theSolid)->GetXHalfLength()/cm);
  npar++;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,((G4Box*)theSolid)->GetYHalfLength()/cm);
  npar++;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,((G4Box*)theSolid)->GetZHalfLength()/cm);
  if(boolShape!=0) {
    boolShape->composants[boolShape->nComp].XH=
      ((G4Box*)theSolid)->GetXHalfLength();
    boolShape->composants[boolShape->nComp].YH=
      ((G4Box*)theSolid)->GetYHalfLength();
    boolShape->composants[boolShape->nComp].ZH=
      ((G4Box*)theSolid)->GetZHalfLength();
  }
  return npar;
}

G4int MyPlacement::TrdParameters(G4VSolid* theSolid,MyBoolShape* boolShape)
{
  G4int npar =1;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,((G4Trd*)theSolid)->GetXHalfLength1()/cm);
  npar++;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,((G4Trd*)theSolid)->GetXHalfLength2()/cm);
  npar++;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,((G4Trd*)theSolid)->GetYHalfLength1()/cm);
  npar++;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,((G4Trd*)theSolid)->GetYHalfLength2()/cm);
  npar++;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,((G4Trd*)theSolid)->GetZHalfLength()/cm);
  if(boolShape!=0) {
    boolShape->composants[boolShape->nComp].XH=
      std::max(((G4Trd*)theSolid)->GetXHalfLength1(),((G4Trd*)theSolid)->GetXHalfLength2());
    boolShape->composants[boolShape->nComp].YH=
      std::max(((G4Trd*)theSolid)->GetYHalfLength1(),((G4Trd*)theSolid)->GetYHalfLength2());
    boolShape->composants[boolShape->nComp].ZH=
      ((G4Trd*)theSolid)->GetZHalfLength();
  }
  return npar;
}

G4int MyPlacement::TrapParameters(G4VSolid* theSolid,MyBoolShape* boolShape)
{
  G4int npar =1;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,((G4Trap*)theSolid)->GetZHalfLength()/cm); // DZ
  npar++;
  G4ThreeVector SymAxis = ((G4Trap*)theSolid)->GetSymAxis();
  
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,-ToDegrees(acos(SymAxis(2))));   // Theta in Geant3 = - Theta in Geant4 !!!
  npar++;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,0.0);                   // PHI allways ZERO !!!
  npar++;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,((G4Trap*)theSolid)->GetYHalfLength1()/cm); // H1
  npar++;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,((G4Trap*)theSolid)->GetXHalfLength1()/cm);  // BL1
  npar++;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,((G4Trap*)theSolid)->GetXHalfLength2()/cm);  // TL1
  npar++;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,
	  atan(((G4Trap*)theSolid)->GetTanAlpha1()));  // ALP1
  npar++;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,((G4Trap*)theSolid)->GetYHalfLength2()/cm);  // H2
  npar++;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,((G4Trap*)theSolid)->GetXHalfLength3()/cm);  // BL2
  npar++;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,((G4Trap*)theSolid)->GetXHalfLength4()/cm);  // TL2
  npar++;
  fprintf(FVOLS,"\tPAR(%d)=%f\n",
	  npar,
	  atan(((G4Trap*)theSolid)->GetTanAlpha2()));    // ALP2
  if(boolShape!=0) {
    boolShape->composants[boolShape->nComp].XH=
      std::max(std::max(((G4Trap*)theSolid)->GetXHalfLength1(),((G4Trap*)theSolid)->GetXHalfLength2()),
	  std::max(((G4Trap*)theSolid)->GetXHalfLength3(),((G4Trap*)theSolid)->GetXHalfLength4()));
    boolShape->composants[boolShape->nComp].YH=
      std::max(((G4Trap*)theSolid)->GetYHalfLength1(),((G4Trap*)theSolid)->GetYHalfLength2());
    boolShape->composants[boolShape->nComp].ZH=
      ((G4Trap*)theSolid)->GetZHalfLength();
  }
  return npar;
}

G4int MyPlacement::DescribeLogical(G4LogicalVolume* LV)
{
  G4int i;
  for (i=0;i<n_LVs;i++)
    if (LVs[i].LogVol == LV) break;
  if(i < n_LVs) return i;

  
  G4VSolid* theSolid = LV->GetSolid();

  // intercepts the World Volume
  if(theSolid->GetName()=="WorldBox") {
    fprintf(FVOLS,"C Geant4 World volume skipped.\n");
    LVs[n_LVs].G3GSName="WORLD";
    LVs[n_LVs].G3GSNumber=0;
    LVs[n_LVs].LogVol = LV;
    LVs[n_LVs].nCopy =0;
    LVs[n_LVs].boolShape=0;
    LVs[n_LVs].theSD=0;
    n_LVs++;
    assert (n_LVs<MAX_LVS);
    return n_LVs-1;
  }

  LVs[n_LVs].boolShape = 0;
  LVs[n_LVs].theSD = 0;

  G4int theSolidNumber= DescribeSolid(theSolid,
				      LV->GetMaterial()->GetName(),
				      (LV->GetSensitiveDetector()!=NULL));
  
  char buff[10];
  char prefix = 'D';
  if (LV->GetSensitiveDetector()!=NULL) prefix = 'S';
  sprintf(buff,"'%c%3.3d'",prefix,theSolidNumber);
  LVs[n_LVs].G3GSName=buff;
  LVs[n_LVs].G3GSNumber=theSolidNumber;
  LVs[n_LVs].LogVol = LV;
  LVs[n_LVs].nCopy = 0;
  LVs[n_LVs].theSD=(SD*)LV->GetSensitiveDetector();

  n_LVs++;
  assert (n_LVs<MAX_LVS);

  return n_LVs-1;
}

G4int MyPlacement::DescribeSolid(G4VSolid* theSolid, 
				 G4String MatName,
				 G4bool IsSensitive=false,
				 MyBoolShape* boolShape)
{

  fprintf(FVOLS,"C Shape %s\n",theSolid->GetName().data());

  // Booleans?
  G4VSolid* comp=0;
  MyBoolShape* theboolShape=boolShape;

  for (G4int nbool = 0; nbool < 2;nbool++) {
    comp = theSolid->GetConstituentSolid(nbool);
    if(comp != 0) {
      fprintf(FVOLS,"C %s shape, part of the %s boolean solid\n",
	      comp->GetName().data(),
	      theSolid->GetName().data());
      if(theboolShape ==0) {
	theboolShape = new MyBoolShape();
	//	fprintf(FVOLS,"2000\tcontinue\n");
      }
      DescribeSolid(comp,MatName,IsSensitive,theboolShape);
    }
    else break;
  }
  if(comp !=0 && boolShape == 0) { 
    fprintf(FVOLS,"C %s shape = %d shapes: ",
	    theSolid->GetName().data(),
	    theboolShape->nComp);
    for (G4int i=0;i<theboolShape->nComp;i++)
      fprintf(FVOLS,"%s  ",theboolShape->composants[i].shapeName.data());
    fprintf(FVOLS,"\n");
    
    // Boolean creation
    G4VSolid* envelope = theboolShape->BuildEnvelopeShape();
    DescribeSolid(envelope,"Air",IsSensitive);  
    theboolShape->FillEnvelope(SolidNumber,IsSensitive);
    LVs[n_LVs].boolShape=theboolShape;
    return SolidNumber;
  }
  if(comp !=0) return SolidNumber;
  
  // Not Booleans
  G4String SolidType = theSolid->GetEntityType();
  G4int npar=0;
  G4String G3Shape;
  G4bool type_ok = false;
  
  if(SolidType == "G4Box") {
    type_ok = true;
    npar=BoxParameters(theSolid,boolShape);
    G3Shape = "BOX ";
  }
  if(SolidType == "G4Trd") {
    type_ok = true;
    npar=TrdParameters(theSolid,boolShape);
    G3Shape = "TRD2";
  }
  if(SolidType == "G4DisplacedSolid") {
    type_ok = true;
    boolShape->composants[boolShape->nComp].theDisplacedSolid=(G4DisplacedSolid*)theSolid;
    return DescribeSolid(((G4DisplacedSolid*)theSolid)->GetConstituentMovedSolid(),
			 MatName,
			 IsSensitive,
			 boolShape);
  }
  if(SolidType == "G4Trap") {
    type_ok = true;
    npar=TrapParameters(theSolid,boolShape);
    G3Shape = "TRAP";
  }
  if(SolidType == "G4Tubs") {
    type_ok = true;
    npar=TubParameters(theSolid,boolShape);
    G3Shape = "TUBE";
  }
  
  if(SolidType == "G4DisplacedSolid") {
    type_ok = true;
    boolShape->composants[boolShape->nComp].theDisplacedSolid=(G4DisplacedSolid*)theSolid;
    return DescribeSolid(((G4DisplacedSolid*)theSolid)->GetConstituentMovedSolid(),
			 MatName,
			 IsSensitive,
			 boolShape);
  }

  
  if(!type_ok) {
    G4cout << "SolidType " << SolidType 
	   << " not found!!!\n";
    Close();
    exit(1);
  }
  SolidNumber++;
  
  char prefix = 'D';
  if (IsSensitive) prefix = 'S';
  fprintf(FVOLS,"\tCALL GSVOLU('%c%3.3d','%s',%s,PAR,%d,IVOLU)\n",
	  prefix,
	  SolidNumber,
	  G3Shape.data(),
	  MatName.data(),
	  npar);
  fprintf(FVOLS,"\tIF(IVOLU.lt.0) STOP 'ERROR WITH GSVOLU'\n");
  if(boolShape!=0) {
    char buff[10];
    prefix = 'D';
    if (IsSensitive) prefix = 'S';
    sprintf(buff,"'%c%3.3d'",prefix,SolidNumber);
    boolShape->composants[boolShape->nComp].shapeName=buff;
    boolShape->composants[boolShape->nComp].shapeNumber=SolidNumber;
    boolShape->nComp++;
    assert (boolShape->nComp<MAX_BOOL_COMP);
  }
  return SolidNumber;
}

G4VSolid* MyBoolShape::BuildEnvelopeShape()
{
  G4double hx,hy,hz;
  hx=hy=hz=0;

  for (G4int i=0;i<nComp;i++) {
    G4ThreeVector
      compSize(composants[i].XH,
	       composants[i].YH,
	       composants[i].ZH);
    
    if(composants[i].theDisplacedSolid!=0){
//  In Geant4 2.0, reversed implementation of GetFrameRotation and GetObjectRotation!
//      compSize = composants[i].theDisplacedSolid->GetObjectRotation().inverse()*compSize;
      compSize = composants[i].theDisplacedSolid->GetObjectRotation()*compSize;
      compSize[0]=fabs(compSize[0]);
      compSize[1]=fabs(compSize[1]);
      compSize[2]=fabs(compSize[2]);

      G4ThreeVector Translation = 
	composants[i].theDisplacedSolid->GetObjectTranslation();
      if(composants[0].XH<fabs(Translation.x())+compSize(0)) 
	compSize[0] = std::max((compSize(0)+composants[0].XH+fabs(Translation.x()))/2.,
			  compSize(0));
      if(composants[0].YH<fabs(Translation.y())+compSize(1)) 
	compSize[1] = std::max((compSize(1)+composants[0].YH+fabs(Translation.y()))/2.,
			  compSize(1));
      if(composants[0].ZH<fabs(Translation.z())+compSize(2)) 
	compSize[2] = std::max((compSize(2)+composants[0].ZH+fabs(Translation.z()))/2.,
			  compSize(2));
      
      
      if (compSize.x()>hx) {
	hx = compSize.x();	
	if(compSize(0)-Translation.x()>composants[0].XH)
	  deltaCenter[0]=compSize(0)-composants[0].XH;
      }
      if (compSize.y()>hy) {
	hy = compSize.y();
	if(compSize(1)-Translation.y()>composants[0].YH) 
	  deltaCenter[1]=compSize(1)-composants[0].YH;
      }
      if (compSize.z()>hz) {
	hz = compSize.z();
	if(compSize(2)-Translation.z()>composants[0].ZH)
	  deltaCenter[2]=compSize(2)-composants[0].ZH;
      }
    } else { // est le i=0!
      hx = compSize.x();
      hy = compSize.y();
      hz = compSize.z();
    }
  }

  G4cout << "boolEnvelope, hz = " << hz << G4endl;
  return envelope= new G4Box("boolEnvelope",
			     hx,
			     hy,
			     hz);
  
}
void MyBoolShape::FillEnvelope(G4int EnvelopeSolidNumber,G4bool IsSensitive)
{
  char prefix = 'D';
  if (IsSensitive) prefix = 'S';

  fprintf(MyPlacement::FVOLS,"C Envelopes are not visible!!!\n");
  fprintf(MyPlacement::FVOLS,"\tCALL GSATT('%c%3.3d','SEEN',0)\n",prefix,EnvelopeSolidNumber);
  fprintf(MyPlacement::FVOLS,"C Filling the %c%3.3d boolean shape\n",prefix,EnvelopeSolidNumber);
  G4String Mode = "'ONLY'";

  for (G4int i=0;i<nComp;i++) {

    G4ThreeVector 
      Translation(composants[0].XH-envelope->GetXHalfLength()+deltaCenter.x(),
		  composants[0].YH-envelope->GetYHalfLength()+deltaCenter.y(),
		  composants[0].ZH-envelope->GetZHalfLength()+deltaCenter.z());
    
    if(composants[i].theDisplacedSolid!=0){
      G4RotationMatrix Rotation = composants[i].theDisplacedSolid->GetFrameRotation();
      MyPlacement::DescribeRotation(&Rotation);
      Translation += composants[i].theDisplacedSolid->GetObjectTranslation();
    }
    else MyPlacement::DescribeRotation(0);
    //if(i!=0) Mode = "'MANY'";
    Mode = "'MANY'";
    fprintf(MyPlacement::FVOLS,
	    "\tCALL GSPOS (%s,%d,'%c%3.3d',%f,%f,%f,\n     * IROT,%s)\n",
	    composants[i].shapeName.data(),
	    1,
	    prefix,
	    EnvelopeSolidNumber,
	    Translation.x()/cm,
	    Translation.y()/cm,
	    Translation.z()/cm,
	    Mode.data()); 
  }
}

void MyPlacement::WriteCellMapping(){

  fprintf(FVOLS,"C*********************************************************\n");
  fprintf(FVOLS,"C\nC This code was generate automatically by Mokka, the \n");
  fprintf(FVOLS,"C Geant4 simulation software for Tesla.\n");
  fprintf(FVOLS,"C\nC   (Developped by LPNHE -Ecole Polytechnique, France.)\n");
  fprintf(FVOLS,"C*********************************************************\n");
  fprintf(FVOLS,"\tlogical function CellMap()\n");
  fprintf(FVOLS,"C\nC If the particle is inside a calorimeter sensitive layer\n");
  fprintf(FVOLS,"C this logical function fills the G4G3 common block and returns\n");
  fprintf(FVOLS,"C TRUE. It returns FALSE otherwise.\nC\n");
  fprintf(FVOLS,"\tinteger Piece,Stave,Module,ICell,JCell,KCell\n");
  fprintf(FVOLS,"\tREAL*8 Cell(3)\n");
  fprintf(FVOLS,"\tCOMMON/G4G3/Piece,Stave,Module,ICell,JCell,KCell,Cell\n");
  
  fprintf(FVOLS,"\tCOMMON/GCVOLU/NLEVEL,NAMES(15),NUMBER(15),\n");
  fprintf(FVOLS,"     +     LVOLUM(15),LINDEX(15),INFROM,NLEVMX,NLDEV(15),LINMX(15),\n");
  fprintf(FVOLS,"     +     GTRAN(3,15),GRMAT(10,15),GONLY(15),GLX(3)\n");
  
  fprintf(FVOLS,"\tPARAMETER (MAXMEC=30)\n");
  fprintf(FVOLS,"\tCOMMON/GCTRAK/VECT(7),GETOT,GEKIN,VOUT(7),NMEC,LMEC(MAXMEC)\n");
  fprintf(FVOLS,"     + ,NAMEC(MAXMEC),NSTEP ,MAXNST,DESTEP,DESTEL,SAFETY,SLENG\n");
  fprintf(FVOLS,"     + ,STEP  ,SNEXT ,SFIELD,TOFG  ,GEKRAT,UPWGHT,IGNEXT,INWVOL\n");
  fprintf(FVOLS,"     + ,ISTOP ,IGAUTO,IEKBIN, ILOSL, IMULL,INGOTO,NLDOWN,NLEVIN\n");
  fprintf(FVOLS,"     + ,NLVSAV,ISTORY\n");
  
  fprintf(FVOLS,"\tREAL*8 Xloc,Yloc,Zloc,CXloc,CYloc,CZloc\n");
  fprintf(FVOLS,"\tREAL*8 COSPHI,SINPHI\n");
  fprintf(FVOLS,"\tREAL*8 Inv\n");
  fprintf(FVOLS,"\tcharacter*4 NAMES\n");

  // find out the number of SDs
  SD* SDs [MAX_LVS];
  G4int SD_n_layers [MAX_LVS];
  G4int SD_n_staves [MAX_LVS];
  G4int SD_low_shape [MAX_LVS];
  G4int SD_up_shape [MAX_LVS];

  G4int n_SDs = 0;
  G4int i;  
  for ( i=0; i< n_LVs; i++)
    if(LVs[i].theSD != 0) 
      {
	G4int j;
	for (j=0;j< n_SDs && SDs [j]!=LVs[i].theSD; j++);
	if(j==n_SDs) {
	  SDs [n_SDs] = LVs[i].theSD;
	  n_SDs++;
	}
      }
  
  // define and fills each data structure for each SD
  for (i=0; i< n_SDs; i++)
    {
      G4int n_staves;
      for (n_staves=0; 
	   SDs [i]->StavesPhirots[n_staves]!=0 && n_staves<MAX_STAVES; 
	   n_staves++);
      SD_n_staves [i] = n_staves;
      G4int n_modules=MAX_MODULES;
      G4int n_layers;
      for (n_layers=0; 
	   SDs [i]->Layers[n_layers]!=0 && n_layers<MAX_LAYERS; 
	   n_layers++);

      SD_n_layers [i]= n_layers;

      G4cout << "SD " << i << " has " << n_staves << " staves, " 
	     << n_modules << " modules, " << n_layers << " layers." << G4endl;
      fprintf(FVOLS,"\n\tREAL*8 z0ff_%d(%d),X0_%d(%d),Y0_%d(%d),Z0_%d(%d)",
	      i,n_modules,i,n_layers,i,n_layers,i,n_layers);

      fprintf(FVOLS,"\n\tREAL*8 cos_%d(%d),sin_%d(%d)",
	      i,SD_n_staves [i],i,SD_n_staves [i]);
      
      char sep = ',';
      G4int k = 0;
      G4int j;

      // cosinus et sinus constants
      sep = ',';
      k = 0;
      fprintf(FVOLS,"\n\tdata cos_%d/",i);
      for (j=0;j<SD_n_staves [i] ; j++)
	{
	  k++;
	  if(k==SD_n_staves [i]) sep='/';
	  if((j%4)==0) fprintf(FVOLS,"\n     *");
	  fprintf(FVOLS,"%+13e%c",
		  cos(*(SDs[i]->StavesPhirots[j])),sep);
	}

      sep = ',';
      k = 0;
      fprintf(FVOLS,"\n\tdata sin_%d/",i);
      for (j=0;j<SD_n_staves [i] ; j++)
	{
	  k++;
	  if(k==SD_n_staves [i]) sep='/';
	  if((j%4)==0) fprintf(FVOLS,"\n     *");
	  fprintf(FVOLS,"%+13e%c",
		  sin(*(SDs[i]->StavesPhirots[j])),sep);
	}
      
      // ZOffsets
      sep = ',';
      k = 0;
      fprintf(FVOLS,"\n\tdata z0ff_%d/",i);
      for (j=0;j<n_modules ; j++)
	{
	  k++;
	  if(k==n_modules) sep='/';
	  if((j%4)==0) fprintf(FVOLS,"\n     *");
	  if(SDs [i]->ModulesZOffsets[j]!=0)
	    fprintf(FVOLS,"%+13e%c",
		    *(SDs [i]->ModulesZOffsets[j])/cm,sep);
	  else
	    fprintf(FVOLS,"-0.000000e+00%c",sep);
	}


      // recherche des plages sensibles : endcaps
      if(SDs [i]->SDPiece != ECALENDCAPMINUS) {
	k = 0;
	for (j=0;j< n_LVs; j++)
	  if(LVs[j].theSD == SDs [i])
	    {
	      k++;
	      if(k==1) SD_low_shape[i]=LVs[j].G3GSNumber;
	      if(k==n_layers) SD_up_shape[i]=LVs[j].G3GSNumber;
	    }
      }
      // recherche des plages sensibles : barrel
      else {
	for (j=0;j< n_LVs; j++)
	  if(LVs[j].theSD == SDs [i])
	    {
	      for (k=0;k < LVs[j].boolShape->nComp; k++){
		if(k==0) 
		  SD_low_shape[i]= LVs[j].boolShape->composants[k].shapeNumber;
		if(k==LVs[j].boolShape->nComp-1) 
		  SD_up_shape[i]= LVs[j].boolShape->composants[k].shapeNumber;
	      }
	    }
      }
      
      fprintf(FVOLS,"\n\tdata X0_%d/",i);
      sep = ',';
      k = 0;
      for (j=0;j< SD_n_layers [i]; j++)
	{
	  k++;
	  if(k==SD_n_layers [i]) sep='/';
	  if((j%4)==0) fprintf(FVOLS,"\n     *");
	  fprintf(FVOLS,"%+13e%c",
		  (SDs [i]->Layers[j])->X0/cm,sep);
	}

      fprintf(FVOLS,"\n\tdata Y0_%d/",i);
      sep = ',';
      k = 0;
      for (j=0;j< SD_n_layers [i]; j++)
	{
	  k++;
	  if(k==SD_n_layers [i]) sep='/';
	  if((j%4)==0) fprintf(FVOLS,"\n     *");
	  fprintf(FVOLS,"%+13e%c",
		  (SDs [i]->Layers[j])->Y0/cm,sep);
	}
      fprintf(FVOLS,"\n\tdata Z0_%d/",i);
      sep = ',';
      k = 0;
      for (j=0;j< SD_n_layers [i]; j++)
	{
	  k++;
	  if(k==SD_n_layers [i]) sep='/';
	  if((j%4)==0) fprintf(FVOLS,"\n     *");
	  fprintf(FVOLS,"%+13e%c",
		  (SDs [i]->Layers[j])->Z0/cm,sep);
	}
    }
  
  fprintf(FVOLS,"\n\tinteger VolNumber\n");

  fprintf(FVOLS,"\n\tCellMap = .false.\n");
  fprintf(FVOLS,"\tread(names(nlevel),'(x,I)',err=999) VolNumber\n");
  
  // Looking for the Shape number
  for (i=0; i< n_SDs; i++)
    {
      if(SDs [i]->SDPiece == ECALENDCAPMINUS ||
	 SDs [i]->SDPiece == ECALENDCAPPLUS)
	fprintf(FVOLS,"C\nC ECAL, ENDCAPS\nC\n");
      if(SDs [i]->SDPiece == ECALBARREL)
	fprintf(FVOLS,"C\nC ECAL, BARREL\nC\n");
      if(SDs [i]->SDPiece == HCALBARREL)
	fprintf(FVOLS,"C\nC HCAL, BARREL\nC\n");
      
      if(SDs [i]->SDPiece != ECALENDCAPMINUS)
	{
	  fprintf(FVOLS,"\tif(VolNumber.ge.%d.and.VolNumber.le.%d) then\n",
		  SD_low_shape[i],SD_up_shape[i]);
//   	  fprintf(FVOLS,"\tif(names(nlevel).ge.'S%d'.and.names(nlevel).le.'S%d') then\n",
// 		  SD_low_shape[i],SD_up_shape[i]);
	  fprintf(FVOLS,"\t   CellMap = .true.\n");
 	  fprintf(FVOLS,"\t   KCell = NUMBER(nlevel)\n");
	  fprintf(FVOLS,"\t   Piece = NUMBER(%d)/100\n",
		  BARREL_ENVELOP_LEVEL);
	  fprintf(FVOLS,"\t   Stave = (NUMBER(%d)-Piece*100)/10\n",
		  BARREL_ENVELOP_LEVEL);
	  fprintf(FVOLS,"\t   Module = mod(NUMBER(%d)-Piece*100,10)\n",
		  BARREL_ENVELOP_LEVEL);

//	  fprintf(FVOLS,"\t   angle=(Stave-1)*acos(-1.)/%d.\n",SD_n_staves [i]/2);
	  fprintf(FVOLS,"\t   COSPHI=cos_%d(Stave)\n",i);
	  fprintf(FVOLS,"\t   SINPHI=sin_%d(Stave)\n",i);
// 	  fprintf(FVOLS,"\t   Xloc= VECT(1)*cos(angle)+VECT(2)*sin(angle)\n");
// 	  fprintf(FVOLS,"\t   Yloc= -VECT(1)*sin(angle)+VECT(2)*cos(angle)\n");
// 	  fprintf(FVOLS,"\t   Zloc= VECT(3)-z0ff_%d(Module)\n",i);
	  fprintf(FVOLS,"\t   Xloc= VECT(1)*COSPHI+VECT(2)*SINPHI\n");
	  fprintf(FVOLS,"\t   Yloc= -VECT(1)*SINPHI+VECT(2)*COSPHI\n");
	  fprintf(FVOLS,"\t   Zloc= VECT(3)-z0ff_%d(Module)\n",i);

	  
	  fprintf(FVOLS,"\t   ICell= (Xloc - X0_%d(KCell))/%f\n",
		  i,SDs [i]->CellDim(0)/cm);
	  fprintf(FVOLS,"\t   JCell= (Zloc - Z0_%d(KCell))/%20e\n",
		  i,SDs [i]->CellDim(2)/cm);
	  
	  // CXloc,CYloc,CZloc     
	  fprintf(FVOLS,"\t   CXloc=X0_%d(KCell)+ICell*%f+%f\n",
		  i,SDs [i]->CellDim(0)/cm,SDs [i]->CellDim(0)/cm/2.);
	  fprintf(FVOLS,"\t   CYloc=Y0_%d(KCell)\n",i);
	  fprintf(FVOLS,"\t   CZloc= Z0_%d(KCell) + JCell * %f + %f\n",
		  i,SDs [i]->CellDim(2)/cm,SDs [i]->CellDim(2)/cm/2.);
	  
// 	  fprintf(FVOLS,"\t   Cell(1)= CXloc*cos(angle)-CYloc*sin(angle)\n",i,i);
// 	  fprintf(FVOLS,"\t   Cell(2)= CXloc*sin(angle)+CYloc*cos(angle)\n",i,i);
// 	  fprintf(FVOLS,"\t   Cell(3)= CZloc + z0ff_%d(Module)\n",i);
	  fprintf(FVOLS,"\t   Cell(1)= CXloc*COSPHI-CYloc*SINPHI\n");
	  fprintf(FVOLS,"\t   Cell(2)= CXloc*SINPHI+CYloc*COSPHI\n");
	  fprintf(FVOLS,"\t   Cell(3)= CZloc + z0ff_%d(Module)\n",i);
	  
	  fprintf(FVOLS,"\t   if(sqrt((VECT(1)-Cell(1))**2+(VECT(2)-Cell(2))**2+\n");
	  fprintf(FVOLS,"     *\t\t(VECT(3)-Cell(3))**2).gt.%f)\n",
		  SDs [i]->CellDim(0)/cm+SDs [i]->CellDim(1)/cm);
	  fprintf(FVOLS,"     *\t\tstop 'ERROR IN CELLMAP!!!!!!'\n");
	  
// 	  fprintf(FVOLS,"\tprint *,'NUMBER(j) = ',NUMBER(j)\n");
// 	  fprintf(FVOLS,"\tprint *,'Piece = ',Piece,', Stave = ',Stave,', Module =',Module\n");
// 	  fprintf(FVOLS,"\tprint *,'VECT = (',VECT(1),',',VECT(2),',',VECT(3),')'\n");
// 	  fprintf(FVOLS,"\tprint *,'Xloc = ',Xloc,', Yloc= ',Yloc,', = Zloc',Zloc\n");
// 	  fprintf(FVOLS,"\tprint *,'ICell = ',ICell,', JCell = ',JCell,', KCell = ',KCell\n");
// 	  fprintf(FVOLS,"\tprint *,'CXloc = ',CXloc,', CYloc= ',CYloc,', = CZloc',CZloc\n");
// 	  fprintf(FVOLS,"\tprint *,'Cell = (',Cell(1),',',Cell(2),',',Cell(3),')'\n");
	  
	  fprintf(FVOLS,"\t   return\n");
	  fprintf(FVOLS,"\tendif\n");
	} 
      else  // Ecal Endcaps
	{
	  fprintf(FVOLS,"\tif(VolNumber.ge.%d.and.VolNumber.le.%d) then\n",
		  SD_low_shape[i],SD_up_shape[i]);
	  fprintf(FVOLS,"\t   CellMap = .true.\n");
	  fprintf(FVOLS,"\t   Inv=1.0\n");
	  fprintf(FVOLS,"\t   if(VECT(3).lt.0.) Inv=-1.0\n");
	  fprintf(FVOLS,"\t   KCell = NUMBER(%d)\n",ECAL_ENDCAPS_LAYER_LEVEL);
	  fprintf(FVOLS,"\t   Piece = NUMBER(%d)/100\n",
		  ECAL_ENDCAPS_ENVELOP_LEVEL);
	  fprintf(FVOLS,"\t   Stave = (NUMBER(%d)-Piece*100)/10\n",
		  ECAL_ENDCAPS_ENVELOP_LEVEL);
	  fprintf(FVOLS,"\t   Module = mod(NUMBER(%d)-Piece*100,10)\n",
		  ECAL_ENDCAPS_ENVELOP_LEVEL);
	  
//	  fprintf(FVOLS,"\t   angle=-(Stave-1)*acos(-1.)/%d.\n",SD_n_staves [i]/2);
	  fprintf(FVOLS,"\t   COSPHI=cos_%d(Stave)\n",i);
	  fprintf(FVOLS,"\t   SINPHI=sin_%d(Stave)\n",i);
	  
	  fprintf(FVOLS,"\t   Xloc= Inv*VECT(1)*COSPHI+VECT(2)*SINPHI\n");
	  fprintf(FVOLS,"\t   Yloc= -INV*VECT(1)*SINPHI+VECT(2)*COSPHI\n");
	  fprintf(FVOLS,"\t   Zloc= VECT(3)-z0ff_%d(Module)\n",i);
	  
	  fprintf(FVOLS,"\t   ICell= (Xloc - X0_%d(KCell))/%f\n",
		  i,SDs [i]->CellDim(0)/cm);
	  fprintf(FVOLS,"\t   JCell= (Yloc - Y0_%d(KCell))/%20e\n",
		  i,SDs [i]->CellDim(2)/cm);
	  
	  // CXloc,CYloc,CZloc     
	  fprintf(FVOLS,"\t   CXloc=X0_%d(KCell)+ICell*%f+%f\n",
		  i,SDs [i]->CellDim(0)/cm,SDs [i]->CellDim(0)/cm/2.);
	  fprintf(FVOLS,"\t   CZloc=Z0_%d(KCell)+ %f\n",i,SDs [i]->CellDim(1)/cm/2.);
	  fprintf(FVOLS,"\t   CYloc= Y0_%d(KCell) + JCell * %f + %f\n",
		  i,SDs [i]->CellDim(2)/cm,SDs [i]->CellDim(2)/cm/2.);
	  
	  fprintf(FVOLS,"\t   Cell(1)= Inv*(CXloc*COSPHI-CYloc*SINPHI)\n");
	  fprintf(FVOLS,"\t   Cell(2)= CXloc*SINPHI+CYloc*COSPHI\n");
	  fprintf(FVOLS,"\t   Cell(3)= Inv*(CZloc + z0ff_%d(Module))\n",i);
	  
	  fprintf(FVOLS,"\t   if(sqrt((VECT(1)-Cell(1))**2+(VECT(2)-Cell(2))**2+\n");
	  fprintf(FVOLS,"     *\t\t(VECT(3)-Cell(3))**2).gt.%f)\n",
		  SDs [i]->CellDim(0)/cm+SDs [i]->CellDim(1)/cm);
	  fprintf(FVOLS,"     *\t\tstop 'ERROR IN CELLMAP!!!!!!'\n");
	  
// 	  fprintf(FVOLS,"\tprint *,'NUMBER(j) = ',NUMBER(j)\n");
// 	  fprintf(FVOLS,"\tprint *,'Piece = ',Piece,', Stave = ',Stave,', Module =',Module\n");
// 	  fprintf(FVOLS,"\tprint *,'VECT = (',VECT(1),',',VECT(2),',',VECT(3),')'\n");
// 	  fprintf(FVOLS,"\tprint *,'Xloc = ',Xloc,', Yloc= ',Yloc,', = Zloc',Zloc\n");
// 	  fprintf(FVOLS,"\tprint *,'ICell = ',ICell,', JCell = ',JCell,', KCell = ',KCell\n");
// 	  fprintf(FVOLS,"\tprint *,'CXloc = ',CXloc,', CYloc= ',CYloc,', = CZloc',CZloc\n");
// 	  fprintf(FVOLS,"\tprint *,'Cell = (',Cell(1),',',Cell(2),',',Cell(3),')'\n");
	  
	  fprintf(FVOLS,"\t   return\n");
	  fprintf(FVOLS,"\tendif\n");
	}
	    
    }
  fprintf(FVOLS,"999   return\n\tend\n");
}

