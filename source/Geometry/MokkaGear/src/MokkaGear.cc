// Class managing exchange from Mokka to Gear

#include "gearxml/MergeXML.h" 
#include "../include/MokkaGear.h"
//#include "UserInit.hh"
//#include "Control.hh"

// function defining singelton of MokkaGear

MokkaGear* MokkaGear::getMgr()
{
      
      // Static instance for MokkaGear

      static MokkaGear instance;

      return &(instance);
}


// Constructor

MokkaGear::MokkaGear()
{
  // check if singleton

  static int count;

  count += 1;

  if (count>1){
    std::cout << "MokkaGear::MokkaGear another instance of MokkaGear has been called."
	      << "Geometry XML-File will probably not be correct."
	      << std::endl ; 
  }

  // set Filename from steeringfile - UserInit

  // MokkaGear::setFileName() ;
  
}


// Destructor

MokkaGear::~MokkaGear(){
}


// Write out to XML file

bool MokkaGear::printXML()
{

  // check whether file already exists
  gear::TiXmlDocument* doc = new gear::TiXmlDocument ;
  bool doesExist = (doc -> LoadFile( fileName )) ;

  // if file exists tell so
  if ( doesExist ) {
    std::cout << "\n MokkaGear -Warning- The XML file for geometry already exists and will be overwritten."
	      << std::endl ;
  }
  
  // Write GearMgr to XML-File
    
  gear::GearXML::createXMLFile( this, fileName);

  // Check whether file can be read in again

  doesExist = (doc -> LoadFile(fileName));

  return doesExist;

}


// function to set xml-filename for output

bool MokkaGear::setFileName( const std::string name)
{

  fileName=name;

  // if file already exists return false

  gear::TiXmlDocument* doc = new gear::TiXmlDocument;

  bool isNew = !(doc -> LoadFile( fileName ));

  return isNew;
}


// function to merge xml file with another

bool MokkaGear::mergeXML( const std::string devotFile, const std::string dominantFile, const std::string targetFile ) {
  
  // debug information
  // std::cout << "This is mergeXML" <<std::endl ;

  // if attributes have not been given abort
  if ( ( devotFile == "" ) || ( dominantFile == "" ) || ( targetFile == "" ) ) {
    std::cout << "MokkaGear::mergeXML No files for merging mentioned. \n"
	      << "MokkaGear::mergeXML Nothing merged. " << std::endl ;

    return false ;
  }
  
  // otherwise merge
  gear::MergeXML merge ;
  
  
  // check dominant file
  if ( merge.setFile1( dominantFile ) ){
    //std::cout << "dominant: ok" << std::endl ;
  }
  else {
    std::cout << "MokkaGear::mergeXML error reading file " << dominantFile <<std::endl ;
    return false ;
  }

  // check devot file
  if ( merge.setFile2( devotFile ) ){
    //std::cout << "devot   : ok" << std::endl ;
  }
  else {
    std::cout << "MokkaGear::mergeXML error reading file " << devotFile <<std::endl ;
    return false ;
  }

  // check merging
  if ( merge.mergeFiles( targetFile ) ){
    //std::cout << "target  : ok" << std::endl ;
    return true ;
  }
  else {
    std::cout << "MokkaGear::mergeXML error merging into file " << targetFile <<std::endl ;
    return false ;
  }

} 


// function to set the xml-filename for output from steering

// bool MokkaGear::setFileName(void)
// {

//     // get instance from UserInit

//   UserInit* uInit = UserInit::getInstance();

//   // get string for xml-File

//   std::string name = uInit -> getString("gearXmlOut");

//   // set filename and check if exists

//   return MokkaGear::setFileName(name);

// }

// bool MokkaGear::setFileName(void)
// {

//   // get singleton instance from control
//   Control* ctrl = Control::GetControl() ;

//   // get filename
//   std::string name = ctrl->GEARFILENAME ;

//   // set file name
//   return MokkaGear::setFileName( name ) ;
// }
  
  
