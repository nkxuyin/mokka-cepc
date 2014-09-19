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
// $Id: VSubDetectorDriver.hh,v 1.10 2007/10/05 11:52:10 mora Exp $
// $Name: mokka-07-00 $
//
#ifndef VSubDetectorDriver_h
#define VSubDetectorDriver_h 1

#include "globals.hh"
#include "Control.hh"
#include "CGAGeometryEnvironment.hh"
#include <vector>

class G4LogicalVolume;
class G4Event;
class VSensitiveDetector;

class VSubDetectorDriver
{
public:
  VSubDetectorDriver(const G4String &aDriverName, 
		     G4String aBaseFileName="");

  virtual ~VSubDetectorDriver() {};


#ifdef MOKKA_GEAR
  void GearSetupIfConstructed();
#endif   
 

  virtual G4bool IsApplicable(const G4String &aDriverName) const;
  
  virtual G4bool construct(const G4String&,
			   G4LogicalVolume*)
  {
    Control::Abort ("Bad construct method called in this driver",
			MOKKA_ERROR_INCOMPLETE_DERIVED_CLASS);
    return false;
  }  

  virtual G4bool 
  ContextualConstruct(const CGAGeometryEnvironment &aGeometryEnvironment,
		      G4LogicalVolume *theWorld)
  {
    return construct (aGeometryEnvironment.GetDBName(),theWorld);
  }

  virtual G4bool 
  PostConstructAction(CGAGeometryEnvironment&) 
  {
    return true;
  }
    
  virtual G4bool basic_construct(const CGAGeometryEnvironment &aGeometryEnvironment,
				 G4LogicalVolume *theWorld
#ifdef LCIO_MODE
				 , G4String aSubDetectorName = ""
#endif
				 ); 
  virtual void BeginOfEventAction(const G4Event*) {};
  virtual void EndOfEventAction(const G4Event* evt);

  G4String GetName() const 
  {return theDriverName;}
  
  G4String GetBaseName() const 
  {return theBaseFileName;}

  G4bool GetSaveTRKHitMomentum(G4int i) const
  { return saveHitMomentum[i]; }

  G4int  GetNumberOfSD() const
  { return theSensitiveDetectors.size(); }

  void SetSaveTRKHitMomentum(G4int i)
  {saveHitMomentum[i]=true; }
  //  void SetNotSaveTRKHitMomentum()
//   {saveHitMomentum.push_back(false); }

  void LoadEvent(G4Event* evt);

  virtual void setID(G4int index) { id = index;}

  virtual G4int getID(void) { return id;}

  virtual VSensitiveDetector* getSD(G4int index) {

	  if((unsigned int)(index) >= theSensitiveDetectors.size()) {
		G4cout << "VSubDetectorDriver::getSD - " <<
			  "wrong SD index: " << index << G4endl;
	  	return NULL;
	  }

	  return theSensitiveDetectors[index];
  }

protected:

#ifdef MOKKA_GEAR
 virtual void GearSetup () { }
#endif

  G4bool IsConstructed;
  void RegisterSensitiveDetector(VSensitiveDetector* aSensitiveDetector);

  G4String AsciiSubDetectorEventFileName(const G4Event* evt) const;

  virtual void LoadEvent(FILE* theSubDetectorEventHitsFileInput);

private:

  G4String theDriverName;
  G4String theBaseFileName;
  G4int id;

#ifdef LCIO_MODE
  G4String SubDetectorName;
#endif
  
  std::vector <VSensitiveDetector*> theSensitiveDetectors;

  //  G4bool saveHitMomentum; // used only if it's a tracker device.
  std::vector <G4bool> saveHitMomentum;
};

#endif


