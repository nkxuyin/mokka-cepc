#ifndef GEAR_CGAGEOMETRYINITIALIZER_H
#define GEAR_CGAGEOMETRYINITIALIZER_H 1

#include <string>
#include <vector>

#include "gear/GEAR.h"
#include "gear/GearMgr.h"

#ifdef LCIO_MODE
#include "lcio.h"
#include "IO/LCReader.h"
#include "EVENT/LCRunHeader.h"

using namespace EVENT ;
using namespace lcio ;
#endif

namespace gear {

/** CGA Geometry Initializer class
 * @author G. Musat, Ecole Polytechnique
 * @version $Id: CGAGeometryInitializer.h,v 1.1 2008/10/13 14:47:43 frank Exp $
 */
class CGAGeometryInitializer {

public: 
    /// Destructor.
    virtual ~CGAGeometryInitializer() { /* nop */; }

#ifdef LCIO_MODE
    static CGAGeometryInitializer * GetCGAGeometryInitializer(
					std::string fileName)
					throw ( IOException );
#endif
    static CGAGeometryInitializer * GetCGAGeometryInitializer(
	std::string steeringFile, std::string model,
        std::string setup, std::string host, std::string user,
        std::string password);

   GearMgr * getCGAGearMgr() throw ( gear::Exception );

private:

    CGAGeometryInitializer(std::string steeringFile, std::string model,
	std::string setup, std::string host, std::string user,
	std::string password);

    static CGAGeometryInitializer * theInitializer;

}; // class
} // namespace gear
#endif /* ifndef GEAR_CGAGEOMETRYINITIALIZER_H */
