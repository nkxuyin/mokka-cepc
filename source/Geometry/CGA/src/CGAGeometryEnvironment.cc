//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//* For more information about Mokka, please, go to the *
//*                                                     *
//*  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
//*                                                     *
//* Mokka home page.                                    *
//*                                                     *
//*******************************************************
//
// $Id: CGAGeometryEnvironment.cc,v 1.5 2006/01/12 12:50:30 mora Exp $
// $Name: mokka-07-00 $
//
  
#include "Control.hh"
#include "CGAGeometryEnvironment.hh"

CGAGeometryEnvironment::
CGAGeometryEnvironment(G4String DBName, 
		       G4String DetectorConceptName,
		       CGASetupParameters *SetupParameters,
		       CGASetupParameters *GlobalParameters)
  : _DBName(DBName), _DetectorConcept(DetectorConceptName), _SetupParameters(SetupParameters),_GlobalParameters(GlobalParameters)
{
  //   G4cout << ">>>>>>>>>>>>>>>>> SetupParameters size = " << _SetupParameters->size() << G4endl;
  //   for (std::map < G4String, G4String >::iterator par = (*_SetupParameters).begin();
  //        par != (*_SetupParameters).end();
  //        par++)
  //     G4cout << (*par).first << " - " << (*par).second << G4endl;  
}

CGAGeometryEnvironment::
~CGAGeometryEnvironment()
{
  if( _SetupParameters )
    {
      _SetupParameters->clear();
      delete _SetupParameters;
    }
}

CGAGeometryEnvironment::
CGAGeometryEnvironment(const CGAGeometryEnvironment &right)
{
  _DBName = right._DBName;
  _DetectorConcept = right._DetectorConcept;
  _SetupParameters = new CGASetupParameters(*right._SetupParameters);
  _GlobalParameters = right._GlobalParameters;
  
}

const 
CGAGeometryEnvironment& CGAGeometryEnvironment::operator=(const CGAGeometryEnvironment &right)
{
  _DBName = right._DBName;
  _DetectorConcept = right._DetectorConcept;
  _SetupParameters = new CGASetupParameters(*right._SetupParameters);
  _GlobalParameters = right._GlobalParameters;
  return *this;
}

void 
CGAGeometryEnvironment::CreateGlobalParameter(G4String PName, G4String PValue)
{
  std::map < G4String, G4String >::value_type newGlobalParameter(PName, PValue);
  std::pair<std::map < G4String, G4String >::iterator, bool> ret;
  ret = _GlobalParameters->insert(newGlobalParameter);

 if(!ret.second){
    G4cout << "Insertion of " << PName << " = " << PValue
           << " in the _GlobalParameters map failed." << G4endl;
    Control::Abort("CGAGeometryEnvironment::CreateGlobalParameter failed!",
		MOKKA_ERROR_CANNOT_CREATE_GLOBAL_PARAMETER);
 }

}

G4String
CGAGeometryEnvironment::
GetParameterAsString (G4String PName) const
{
  std::map < G4String, G4String >::iterator par; 
  if( (par =  _SetupParameters->find(PName)) != _SetupParameters->end())
    return par->second;

  // If not found, abort Mokka (geometry driver is inconsistent!!!)
  G4String message;
  message = "geometry driver inconsistence: \nsetup parameter \"" 
    + PName 
    + "\" has to have at least a default value in the sharing table, \nin the models database!";
  Control::Abort(message,MOKKA_ERROR_CANNOT_FIND_GLOBAL_PARAMETER);
  return "";
}

G4int
CGAGeometryEnvironment::
GetParameterAsInt (G4String PName) const
{
  std::map < G4String, G4String >::iterator par; 
  if( (par =  _SetupParameters->find(PName)) == _SetupParameters->end())
    {
      // If not found, abort Mokka (geometry driver is inconsistent!!!)
      G4String message;
      message = "geometry driver inconsistence: \nsetup parameter \"" 
	+ PName 
	+ "\" has to have at least a default value in the sharing table, \nin the models database!";
      Control::Abort(message,MOKKA_ERROR_CANNOT_FIND_GLOBAL_PARAMETER);
    }
  G4int val;
  std::istringstream is(par->second.data());
  is >> val;
  return val;
}

G4double
CGAGeometryEnvironment::
GetParameterAsDouble (G4String PName) const
{
  std::map < G4String, G4String >::iterator par; 
  if( (par =  _SetupParameters->find(PName)) == _SetupParameters->end())
    {
      // If not found, abort Mokka (geometry driver is inconsistent!!!)
      G4String message;
      message = "geometry driver inconsistence: \nsetup parameter \"" 
	+ PName 
	+ "\" has to have at least a default value in the sharing table, \nin the models database!";
      Control::Abort(message,MOKKA_ERROR_CANNOT_FIND_GLOBAL_PARAMETER);
    }
  G4double val;
  std::istringstream is(par->second.data());
  is >> val;
  return val;
}
