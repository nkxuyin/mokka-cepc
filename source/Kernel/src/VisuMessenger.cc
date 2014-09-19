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
// $Id: VisuMessenger.cc,v 1.8 2006/03/22 12:29:58 musat Exp $
// $Name: mokka-07-00 $
//
// 
#include "VisuMessenger.hh"
#include "Visu.hh"
#include "globals.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4Tokenizer.hh"
#include "CGAGeometryManager.hh"
#include "G4VisAttributes.hh"
#include "G4UImanager.hh"

#ifdef G4LIB_USE_GDML
#include "G4GDMLParser.hh"
#include "G4PVPlacement.hh"
#endif

#include <vector>

VisuMessenger::VisuMessenger(Visu* aVisu)
  : theVisu(aVisu)
{
  theImmediateMode = false;
  theRefreshCmd = new G4UIcmdWithoutParameter("/Mokka/Visu/Refresh",this);
  theRefreshCmd->SetGuidance("refresh the view.");

  G4UIdirectory* aVisuDirectory  = new G4UIdirectory("/Mokka/Visu/Detector/");
  aVisuDirectory->SetGuidance("Geometry visualization commands.");

  theModeCmd = new G4UIcommand("/Mokka/Visu/Detector/Mode",this);
  theModeCmd->SetGuidance("Set the rendering mode for a given sub detector and deep");
  G4UIparameter *aSubdetectorParameter = new G4UIparameter("subdetector", 's', true);
  aSubdetectorParameter->SetDefaultValue("all");
  aSubdetectorParameter->SetGuidance("a sub detector name (ecal, vxd, hcal, etc.)");
  G4UIparameter *aModeParameter = new G4UIparameter("mode", 's', true);
  aModeParameter->SetDefaultValue("wire");
  aModeParameter->SetParameterCandidates("wire solid");
  G4UIparameter *aDeepParameter = new G4UIparameter("deep", 'i', true);
  aDeepParameter->SetDefaultValue(0);
  G4UIparameter *aLVParameter = new G4UIparameter("LogicalVolume", 's', true);
  aLVParameter->SetDefaultValue("all");

  aDeepParameter->SetGuidance("deep in the geometry three (0 = world volume)");
  theModeCmd->SetParameter(aSubdetectorParameter);
  theModeCmd->SetParameter(aModeParameter);
  theModeCmd->SetParameter(aLVParameter);
  theModeCmd->SetParameter(aDeepParameter);
  
  G4UIparameter *aRParameter = new G4UIparameter("R", 'd', true);
  aRParameter->SetDefaultValue(1.0);
  aRParameter->SetGuidance("red component");
  G4UIparameter *aGParameter = new G4UIparameter("G", 'd', true);
  aGParameter->SetDefaultValue(1.0);
  aGParameter->SetGuidance("green component");
  G4UIparameter *aBParameter = new G4UIparameter("B", 'd', true);
  aBParameter->SetDefaultValue(1.0);
  aBParameter->SetGuidance("blue component");
  G4UIparameter *aOpParameter = new G4UIparameter("opacity", 'd', true);
  aOpParameter->SetDefaultValue(1.0);
  aOpParameter->SetGuidance("opacity (1 = opaque)");

  theColorCmd = new G4UIcommand("/Mokka/Visu/Detector/Colour",this);
  theColorCmd->SetGuidance("Set the rendering color for a given sub detector deep");
  theColorCmd->SetParameter(aSubdetectorParameter);
  theColorCmd->SetParameter(aRParameter);
  theColorCmd->SetParameter(aGParameter);
  theColorCmd->SetParameter(aBParameter);
  theColorCmd->SetParameter(aOpParameter);
  theColorCmd->SetParameter(aLVParameter);
  theColorCmd->SetParameter(aDeepParameter);


  theDaughtersCmd = new G4UIcommand("/Mokka/Visu/Detector/Daughters",this);
  theDaughtersCmd->SetGuidance("Set the daughter's visibility for a given sub detector and deep");
  G4UIparameter *aOnOffParameter = new G4UIparameter("visible",'s',true);
  aOnOffParameter->SetParameterCandidates("true false");

  aOnOffParameter->SetDefaultValue("true");
  theDaughtersCmd->SetParameter(aSubdetectorParameter);
  theDaughtersCmd->SetParameter(aOnOffParameter);
  theDaughtersCmd->SetParameter(aLVParameter);
  theDaughtersCmd->SetParameter(aDeepParameter);

  thevisibilityCmd = new G4UIcommand("/Mokka/Visu/Detector/Visibility",this);
  thevisibilityCmd->SetGuidance("Set the visibility for a given sub detector");

  thevisibilityCmd->SetParameter(aSubdetectorParameter);
  thevisibilityCmd->SetParameter(aOnOffParameter);
  thevisibilityCmd->SetParameter(aLVParameter);

  theListThreeCmd = new G4UIcommand("/Mokka/Visu/Detector/ListGeometryTree",this);
  theListThreeCmd->SetGuidance("Prints the sub detector names, visibility and sub detector trees");
  theListThreeCmd->SetParameter(aSubdetectorParameter);

  theImmediateCmd  = new G4UIcommand("/Mokka/Visu/Detector/ImmediateMode",this);
  theImmediateCmd->SetGuidance("Automatical refresh of the viewer after each command");
  theImmediateCmd->SetParameter(aOnOffParameter);

  theDefaultCmd = new G4UIcommand("/Mokka/Visu/Detector/Reset",this);
  theDefaultCmd->SetGuidance("Reset the vis attributes to the model default");
  theDefaultCmd->SetParameter(aSubdetectorParameter);
  theDefaultCmd->SetParameter(aLVParameter);
  theDefaultCmd->SetParameter(aDeepParameter);

#ifdef G4LIB_USE_GDML
  theGDMLCmd = new G4UIcommand("/Mokka/Visu/Detector/DumpGDML",this);
  theGDMLCmd->
    SetGuidance("Dumps the GDML for all detector or for a given sub detector Logical Volume");
  //  G4UIparameter *aFileNameParameter = 
  //  new G4UIparameter("GDML File name", 's', false);
  theGDMLCmd->SetParameter(aSubdetectorParameter);
  theGDMLCmd->SetParameter(aLVParameter);
  //  theGDMLCmd->SetParameter(aFileNameParameter);
#endif

  theVRMLCmd = new G4UIcommand("/Mokka/Visu/Detector/DumpVRML",this);
  theVRMLCmd->
    SetGuidance("Dumps the VRML for all detector or for a given sub detector.");
  theVRMLCmd->SetParameter(aSubdetectorParameter);

}

VisuMessenger::~VisuMessenger()
{
  delete theRefreshCmd;
  delete theModeCmd;
  delete theColorCmd;
  delete theDaughtersCmd;
  delete thevisibilityCmd;
  delete theListThreeCmd;
  delete theImmediateCmd;
  delete theDefaultCmd;
  delete theGDMLCmd;
  delete theVRMLCmd;
}

void VisuMessenger::SetNewValue(G4UIcommand* command, G4String aValue)
{
  if( command == theRefreshCmd) 
    {
      theVisu->Refresh();
      return;
    }

  if( command == theImmediateCmd)
    {
      G4Tokenizer nextToken(aValue);
      theImmediateMode = (G4String("true") == nextToken());
      if(theImmediateMode)
	G4cout << "ImmediateMode active\n";
      else
	G4cout << "ImmediateMode inactive\n";
      return;
    }

  G4String subdetector,aLVName;
  LV_level *aLV_level;
  GeometryThree *theGeometryThree =  
    CGAGeometryManager::GetCGAGeometryManager()->GetGeometryThree();
  

  if( command == theListThreeCmd)
    {
      G4Tokenizer nextToken(aValue);
      subdetector = nextToken();
      
      G4cout << "Geometry three \nsub_detector (visibility): level / daughter:\n";
      for (unsigned int i_vol = 0;
	   i_vol < theGeometryThree->size();
	   i_vol ++)
	{
	  aLV_level = theGeometryThree->operator[](i_vol);
	  if(aLV_level == 0)
	    {
	      G4cout << "aLV_level == 0 !!!" << G4endl;
	      exit(0);
	    }

	  if(aLV_level->SubDetectorName == subdetector 
	     || subdetector == "all")
	    {
	      G4cout << aLV_level->SubDetectorName;
	      if(aLV_level->LV == 0)
		{
		  G4cout << "aLV_level->LV == 0 !!!" << G4endl;
		  exit(0);
		}
	      if(aLV_level->LV->GetVisAttributes() == 0)
		{
		  G4cout << "aLV_level->LV->GetVisAttributes() == 0 !!!" << G4endl;
		  exit(0);
		}
	      
	      if(aLV_level->LV->GetVisAttributes()->IsVisible())
		G4cout << " (visible) ";
	      else
		G4cout << " (invisible) ";
	      G4cout << ": "
		     << aLV_level->level 
		     << " / "
		     << aLV_level->LV->GetName()
		     << G4endl;	 
	    }   
	}
      return;
    }
  
  G4VisAttributes *aVisAtt=NULL;
  G4bool found = false; 
  G4String deep;
  G4int vlevel = -99;

  if( command == theModeCmd) 
    {
      G4Tokenizer nextToken(aValue);
      subdetector = nextToken();
      G4String mode = nextToken();
      aLVName  = nextToken();
      deep = nextToken();
      
      const char* t = deep;
      std::istringstream is((char*)t);
      is >> vlevel;
      
      for (unsigned int i_vol = 0;
	   i_vol < theGeometryThree->size();
	   i_vol ++)
	{	  
	  aLV_level = theGeometryThree->operator[](i_vol);
	  if( (aLV_level->SubDetectorName == subdetector 
	       || subdetector == "all") 
	      &&
	      ( ( aLV_level->level == vlevel && aLVName == "all") ||
		aLV_level->LV->GetName() == aLVName))
	    {
	      found = true;
	      aVisAtt = 
		new G4VisAttributes(aLV_level->
				    LV->
				    GetVisAttributes()->
				    GetColour());
	      if ( mode == "wire")
		aVisAtt->SetForceWireframe(true);
	      else
		aVisAtt->SetForceSolid(true);

	      aVisAtt->
		SetDaughtersInvisible(aLV_level->
				      LV->
				      GetVisAttributes()->
				      IsDaughtersInvisible());
	      aLV_level->LV->SetVisAttributes(aVisAtt);
	    }
	}
    }

#ifdef G4LIB_USE_GDML
  else if( command == theGDMLCmd )
    {
      G4Tokenizer nextToken(aValue);
//       G4String GDMLFileName;
//       GDMLFileName = nextToken();
      subdetector = nextToken();
      aLVName  = nextToken();

      if( subdetector == "all")
	{
	  G4GDMLParser parser;
	  const G4VPhysicalVolume * theWorld =  
	    CGAGeometryManager::GetCGAGeometryManager()->GetWorldPhys();
	  parser.
	    Write("World.gdml",
		  theWorld);
	  found = true; 
	}
      else 
	{
	  
	  for (unsigned int i_vol = 0;
	       i_vol < theGeometryThree->size();
	       i_vol ++)
	    {	  
	      aLV_level = theGeometryThree->operator[](i_vol);
	      if( (aLV_level->SubDetectorName == subdetector)
		  &&
		  aLV_level->LV->GetName() == aLVName )
		{
		  G4PVPlacement 
		    *DummyWorldPhys=
		    new G4PVPlacement(0,G4ThreeVector(),
				      "DummyWorldPhysical",
				      aLV_level->LV,
				      0,false,0);

		  G4GDMLParser parser;
		  parser.Write(aLVName + G4String(".gdml"),
			       DummyWorldPhys);
		  delete DummyWorldPhys;
		  found = true;
		  break;
		}
	    }
	}
    }
#endif

  else if( command == theVRMLCmd)
    {
      G4Tokenizer nextToken(aValue);
      subdetector = nextToken();
      G4String command;
      found = false;
      G4cout << "Erasing old VRML directory, if any, and creating a new empty one." << G4endl;
      system ("rm -fr g4_00.wrl; rm -fr VRML; mkdir VRML;");
      G4UImanager *UImanager = G4UImanager::GetUIpointer();
//      UImanager->ApplyCommand("/Mokka/Visu/Detector/Daughters all true");
      G4String lastDumped = "void";
      G4String vrmlExt = ".wrl";
      for (unsigned int i_vol = 0;
	   i_vol < theGeometryThree->size();
	   i_vol ++)
	{
	  aLV_level = theGeometryThree->operator[](i_vol);
	  if ( lastDumped != 
	       aLV_level->SubDetectorName)
	    {
	      lastDumped = aLV_level->SubDetectorName;
	      if(subdetector != "all" && subdetector != lastDumped) continue;
	      found = true;
	      UImanager->ApplyCommand("/Mokka/Visu/Detector/Visibility all false");
	      command = "/Mokka/Visu/Detector/Visibility ";
	      command += lastDumped;
	      command += " true";
	      UImanager->ApplyCommand(command);
	      UImanager->ApplyCommand("/vis/open VRML2FILE");
	      UImanager->ApplyCommand("/vis/drawVolume");
	      UImanager->ApplyCommand("/vis/viewer/flush");

// Version avec DEFS + scale pour EDMS DESY
	      command = "awk '/SOLID/ {if(ok==0) print \"DEF ";
	      command += lastDumped;
	      command += " Group { Transform { scale 0.001 0.001 0.001 children [\";ok=1;}";
	      command += " {print} END {print \"  ] ]} } \" }'";
	      command += " g4_00.wrl > VRML/tmp.wrl";
	      system (command);

// Version avec DEFS
// 	      command = "awk '/SOLID/ {if(ok==0) print \"DEF ";
// 	      command += lastDumped;
// 	      command += " Group { children [\";ok=1;}";
// 	      command += " {print} END {print \"  ] } \" }'";
// 	      command += " g4_00.wrl > VRML/tmp.wrl";
// 	      system (command);


// Version EDMS DESY + DEFS
// 	      command = "awk '/SOLID/ {if(ok==0) print \"DEF ";
// 	      command += lastDumped;
// 	      command += " Transform { scale 0.001 0.001 0.001 children [\";ok=1;}";
// 	      command += " {print} END {print \"  ] } \" }'";
// 	      command += " g4_00.wrl > VRML/tmp.wrl";
// 	      system (command);
	      

// Version pour EDMS DESY
//	      system ("awk '/SOLID/ {if(on==1) print \"  ] }\"; print \"Transform { scale 0.001 0.001 0.001 children [\"; on = 1;} /#End/ {print \"  ] } \" } {print}' g4_00.wrl > VRML/tmp.wrl");

	      command = "mv  VRML/tmp.wrl VRML/";
	      command +=  lastDumped + vrmlExt;
	      system (command);
	      system ("rm g4_00.wrl");
	    }
	}
      UImanager->ApplyCommand("/Mokka/Visu/Detector/Reset all");
    }
  else if( command == theDaughtersCmd)
    {
      G4Tokenizer nextToken(aValue);
      subdetector = nextToken();
      G4String visibility  = nextToken();
      aLVName  = nextToken();
      deep = nextToken();

      const char* t = deep;
      std::istringstream is((char*)t);
      is >> vlevel;
      
      for (unsigned int i_vol = 0;
	   i_vol < theGeometryThree->size();
	   i_vol ++)
	{	  
	  aLV_level = theGeometryThree->operator[](i_vol);
	  if( (aLV_level->SubDetectorName == subdetector 
	       || subdetector == "all") 
	      &&
	      ( ( aLV_level->level == vlevel && aLVName == "all") ||
		aLV_level->LV->GetName() == aLVName))

	    {
	      found = true;
	      aVisAtt = 
		new G4VisAttributes(aLV_level->
				    LV->
				    GetVisAttributes()->
				    GetColour());
	      aVisAtt->SetDaughtersInvisible (visibility != "true");
	      
	      if(aLV_level->
		 LV->
		 GetVisAttributes()->
		 GetForcedDrawingStyle() ==  G4VisAttributes::wireframe)
		aVisAtt->SetForceWireframe(true);
	      else 
		aVisAtt->SetForceSolid(true);
	      
	      aLV_level->LV->SetVisAttributes(aVisAtt);
	    }
	}
    }
  else if( command == thevisibilityCmd )
    {
      G4Tokenizer nextToken(aValue);
      subdetector = nextToken();
      G4String visibility  = nextToken();
      aLVName  = nextToken();
      
      for (unsigned int i_vol = 0;
	   i_vol < theGeometryThree->size();
	   i_vol ++)
	{	  
	  aLV_level = theGeometryThree->operator[](i_vol);
	  if( (aLV_level->SubDetectorName == subdetector 
	       || subdetector == "all")
	      &&
	      (aLV_level->LV->GetName() == aLVName ||
	       aLVName == "all"))
	    {
	      found = true;
	      aVisAtt = 
		new G4VisAttributes(aLV_level->
				    LV->
				    GetVisAttributes()->
				    GetColour());
	      aVisAtt->SetDaughtersInvisible (aLV_level->
					      LV->
					      GetVisAttributes()->
					      IsDaughtersInvisible());
	      
	      if(aLV_level->
		 LV->
		 GetVisAttributes()->
		 GetForcedDrawingStyle() ==  G4VisAttributes::wireframe)
		aVisAtt->SetForceWireframe(true);
	      else 
		aVisAtt->SetForceSolid(true);
	      
	      aVisAtt->SetVisibility(visibility == "true");
	      
	      aLV_level->LV->SetVisAttributes(aVisAtt);
	    }
	  
	}
    }
  else if( command == theColorCmd )
    {
      G4Tokenizer nextToken(aValue);
      G4double R,G,B,A;

      const char* t = aValue;
      std::istringstream is((char*)t);
      is >> subdetector;
      is >> R;
      is >> G;
      is >> B;
      is >> A;
      is >> aLVName;
      is >> vlevel;

      for (unsigned int i_vol = 0;
	   i_vol < theGeometryThree->size();
	   i_vol ++)
	{	  
	  aLV_level = theGeometryThree->operator[](i_vol);
	  if( (aLV_level->SubDetectorName == subdetector 
	       || subdetector == "all")

	      &&
	      ( ( aLV_level->level == vlevel && aLVName == "all") ||
		aLV_level->LV->GetName() == aLVName))

	    {
	      found = true;
	      aVisAtt = 
		new G4VisAttributes(aLV_level->
				    LV->
				    GetVisAttributes()->
				    IsVisible());
	      aVisAtt->SetDaughtersInvisible (aLV_level->
					      LV->
					      GetVisAttributes()->
					      IsDaughtersInvisible());
	      
	      if(aLV_level->
		 LV->
		 GetVisAttributes()->
		 GetForcedDrawingStyle() ==  G4VisAttributes::wireframe)
		aVisAtt->SetForceWireframe(true);
	      else 
		aVisAtt->SetForceSolid(true);
	      
	      aVisAtt->SetColour(G4Colour(R,G,B,A));
	      aLV_level->LV->SetVisAttributes(aVisAtt);
	    }
	  
	}
    }
  else if(  command == theDefaultCmd )
    {
      G4Tokenizer nextToken(aValue);
      subdetector = nextToken();
      aLVName  = nextToken();
      deep = nextToken();
      const char* t = deep;
      std::istringstream is((char*)t);
      is >> vlevel;
      for (unsigned int i_vol = 0;
	   i_vol < theGeometryThree->size();
	   i_vol ++)
	{	  
	  aLV_level = theGeometryThree->operator[](i_vol);
	  if( (aLV_level->SubDetectorName == subdetector 
	       || subdetector == "all")
	      &&
	      ( ( aLV_level->level == vlevel && aLVName == "all") ||
		aLV_level->LV->GetName() == aLVName))
	    {
	      found = true;
	      aLV_level->LV->SetVisAttributes(aLV_level->DefaultVisAttributes);
	    }
	}
    }
  if (!found)
    {
      G4cout << "\nDetector name and/or level and/or Logical Volume name not found!\nThe detector names available are:\n";
      G4String lastDumped = "void";
      for (unsigned int i_vol = 0;
	   i_vol < theGeometryThree->size();
	   i_vol ++)
	{	  
	  aLV_level = theGeometryThree->operator[](i_vol);
	  if(lastDumped != aLV_level->SubDetectorName)
	    {
	      lastDumped = aLV_level->SubDetectorName;
	      G4cout << lastDumped << "\n";
	    }
	}
      G4cout << G4endl;
    }
  
  if(aVisAtt)
    {
      G4cout << subdetector;
      if(aVisAtt->IsVisible())
	G4cout << " (visible): ";
      else
	G4cout << " (invisible): ";
      
      if(aLVName != "all")
	G4cout << aLVName << ", ";
      
      if( vlevel != -99)
	G4cout << " deep " << vlevel;
      
      G4cout << " set to color " 
	     << aVisAtt->GetColour()
	     << ", mode ";
      if(aVisAtt->GetForcedDrawingStyle() == 0)
	G4cout << "wire";
      else
	G4cout << "solid";
      
      G4cout << " and daughters visibility to ";
      if (aVisAtt->IsDaughtersInvisible())
	G4cout << "invisible.";
      else
	G4cout << "visible.";
    }

  if(theImmediateMode && found)
    G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/refresh");
  else
    G4cout << "(these changes will take effect on the next view rendering if this deep is visible)";
  
  G4cout << "\n";
}



