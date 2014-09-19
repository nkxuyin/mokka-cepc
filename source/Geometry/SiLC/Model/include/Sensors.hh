/*! \file Sensors.hh
    \brief A definition of Silc::Sensor class and it descendants.

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

#include "MaterialObject.hh"

namespace Silc
{
    /// An abstract class which represents a silicon detector.
    class Sensor : public MaterialObject
    {
    public: // Type definitions.
        /// Represents a sensor's technology.
        /** The avaible technologies: pixel, single strip, stereo strip, strixel, false double single strip, etc.*/
        typedef string SensorTechnology;

        /// Represents a local sensitive node ID.
        typedef phys::vector<unsigned, PLANE_DIMENSION> SensitiveNodeId;

    public: // Basic methods.
        /// The Silc::Sensor constructor.
        Sensor(const TCuboidSize& p_vSensorSize, const TPlaneDistribution& p_vSensitiveNodeDistribution,
               const TPlaneDimension& p_vPitchSize, const TCuboidSize& p_vSensitiveNodeSize,
               const string& p_sMaterialName) throw(std_ext::out_of_range_exception);

        /// Returns a size of the sensor.
        const TCuboidSize& GetSensorSize() const throw();

        /// Returns sensitive node distribution.
        const TPlaneDistribution& GetSensitiveNodeDistribution() const throw();

        /// Returns pitch size.
        const TPlaneDimension& GetPitchSize() const throw();

        /// Returns sensitive node size.
        const TCuboidSize& GetSensitiveNodeSize() const throw();

        /// Returns a total number of sensitive nodes.
        unsigned GetNumberOfSensitiveNodes() const throw();

        /// Returns a position of the sensitive node with the ID equals to 'p_vNodeId'.
        TPlanePosition GetSensitiveNodePosition(SensitiveNodeId p_vNodeId)
        const throw(std_ext::out_of_range_exception);

        /// Returns an ID of the sensitive node nearest to the postion specified in 'p_vLocalPosition'.
        SensitiveNodeId GetNearestSensitiveNodeId(TPlanePosition p_vLocalPosition) const throw();

    private: // Data members.
        /// The sensor size.
        TCuboidSize m_vSensorSize;

        /// The sensitive node distribution.
        TPlaneDistribution m_vSensitiveNodeDistribution;

        /// The pitch size.
        TPlaneDimension m_vPitchSize;

        /// The sensitive node sise.
        TCuboidSize m_vSensitiveNodeSize;
    };
}
