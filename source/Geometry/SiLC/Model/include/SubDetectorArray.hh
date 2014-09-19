/*! \file EndcapArray.hh
    \brief A definition of Silc::SubDetectorArray class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#pragma once

#include "Silc_Globals.hh"
#include "Sensors.hh"
#include "GlobalNodeId.hh"

namespace Silc
{
    /// A abstract class which represents a generic array of the similar sub-detectors.
    class SubDetectorArray
    {
    public: // Abstract methods.
        virtual const Sensor& GetSensorPrototype(unsigned p_uLayerId) const = 0;
        virtual TVector TransformVectorToLocal(const TVector& p_vGlobalVector, GlobalNodeId p_uSensorId) const = 0;
        virtual TVector TransformVectorToGlobal(const TVector& p_vLocalVector, GlobalNodeId p_uSensorId) const = 0;
        virtual GlobalNodeId FindGlobalNodeId(const TVector& p_vGlobalVector) const = 0;
        virtual const set<string>& GetSensitiveDetectorNames() const = 0;
    };
}
