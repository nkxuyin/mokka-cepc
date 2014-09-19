#include <string>
#include <vector>

#include "gear/GEAR.h"
#include "gear/GearMgr.h"
#include "CGAGeometryInitializer.h"
#include "CGAGearPointProperties.h"
#include "CGAGearDistanceProperties.h"
#include "CGADefs.h"

#ifdef LCIO_MODE
#include "lcio.h"
#include "IO/LCReader.h"
#include "EVENT/LCRunHeader.h"

using namespace EVENT ;
using namespace lcio ;
#endif

extern gear::GearMgr* cgaGearMgr;

namespace gear {

/** CGA Geometry Initializer class
 * @author G. Musat, Ecole Polytechnique
 * @version $Id: CGAGeometryInitializer.cc,v 1.2 2008/10/14 12:31:25 frank Exp $
 */
    CGAGeometryInitializer * CGAGeometryInitializer::theInitializer = NULL;

#ifdef LCIO_MODE
    CGAGeometryInitializer * CGAGeometryInitializer::GetCGAGeometryInitializer(
                                        std::string fileName) 
					throw ( IOException ){
        if(theInitializer != NULL)
        	return theInitializer;

	LCReader* lcReader = LCFactory::getInstance()->createLCReader();
	lcReader->open( fileName );
	LCRunHeader *runHdr = 0;
	std::string steeringFileContent;
                // read first run header
 	if( ( runHdr = lcReader->readNextRunHeader() ) != 0 ){

                steeringFileContent = runHdr->parameters().
                        getStringVal("MOKKA_SteeringFile");

		theInitializer = new CGAGeometryInitializer(
                        steeringFileContent.c_str(), "", "", "", "", "");
      	}

	return theInitializer;
    }
#endif

    CGAGeometryInitializer * CGAGeometryInitializer::GetCGAGeometryInitializer(
	std::string steeringFile, std::string model,
        std::string setup, std::string host, std::string user,
        std::string password) {

	if(theInitializer == NULL) {
		theInitializer = new CGAGeometryInitializer(steeringFile,
			model, setup, host, user, password);
	}
	return theInitializer;
    }

    CGAGeometryInitializer::CGAGeometryInitializer(std::string steeringFile, 
	std::string model, std::string setup, std::string host, 
	std::string user, std::string password) {

	CGAInit(steeringFile.c_str(), model.c_str(), setup.c_str(), 
		host.c_str(), user.c_str(), password.c_str());

	if(cgaGearMgr != 0) {
	    cgaGearMgr->setPointProperties(new CGAGearPointProperties);
	    cgaGearMgr->setDistanceProperties(new CGAGearDistanceProperties);
	}
	
    }

    GearMgr * CGAGeometryInitializer::getCGAGearMgr()  throw ( gear::Exception ) {
	
	if(cgaGearMgr != 0) {
	   return cgaGearMgr;
	}
	else {
                std::cout << "CGAGeometryInitializer::getCGAGearMgr Error: " <<
		std::endl << "CGAGearMgr not initialized. CGAInit not " << 
		"successfully ended!" << std::endl;

		throw gear::Exception("CGAGearMgr not initialized!");
        }
    }
} // namespace gear
