/** Interface for the implementation of Gear in Mokka.
 *
 * takes account of all includes and handles the communication with the
 * steering file and the output file
 *
 * @author: R.Lippe, DESY
 * @version $Id
 */

#ifndef MokkaGear_h
#define MokkaGear_h

#include "gear/GearMgr.h" 
#include "gearimpl/GearMgrImpl.h"
#include "gearimpl/GearParametersImpl.h"
#include "gearxml/GearXML.h"
#include "gearxml/tinyxml.h"

// these are all needed GEAR-Files
// you might want to include in ccfiles that
// use MokkaGear

/* #include "gear/TPCParameters.h" */
/* #include "gear/CalorimeterParameters.h" */
/* #include "gearimpl/TPCParametersImpl.h" */
/* #include "gearimpl/FixedPadSizeDiskLayout.h" */
/* #include "gearimpl/CalorimeterParametersImpl.h" */
/* #include "gearimpl/LayerLayoutImpl.h" */

class MokkaGear : public gear::GearMgrImpl {

 public:

  /** function defining singleton of MokkaGear
   */
  static MokkaGear* getMgr();
  
  
  // function to set FileName for XML output from Steeringfile
  // returns false if Filename already exists

  //bool setFileName(void);

  /** method to set Filename for XML output by hand
   * returns false if Filename already exists
   */
  bool setFileName( const std::string name);

  /** method to printout in XML File
   */
  bool printXML();

  /** method for merging two xml-files
   * uses mergeXML from gear.
   */
  bool mergeXML( const std::string devotFile, const std::string dominantFile, const std::string targetFile ) ;
  
  /** gearParameterImplementation that can hold temp information
   * allowing to use setValue and getValue methods from gear
   */
  gear::GearParametersImpl tmpParam ;

 private:  

  // Constructor in private to guarantee singleton
  MokkaGear();

  // Copy Constructor in private to guarantee singleton
  MokkaGear( const MokkaGear& );

  // Destructor in private to guarentee destroying only within scope
  ~MokkaGear();

 
  std::string fileName;

  
};
#endif
