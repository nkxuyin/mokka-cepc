/*! \file Sensors.cc
    \brief Implements all features defined in Sensors.hh.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "Sensors.hh"

using namespace Silc;
using namespace std_ext;

Sensor::Sensor(const TCuboidSize& p_vSensorSize, const TPlaneDistribution& p_vSensitiveNodeDistribution,
               const TPlaneDimension& p_vPitchSize, const TCuboidSize& p_vSensitiveNodeSize,
               const string& p_sMaterialName) throw(std_ext::out_of_range_exception)
    : MaterialObject(p_sMaterialName), m_vSensorSize(p_vSensorSize),
      m_vSensitiveNodeDistribution(p_vSensitiveNodeDistribution), m_vPitchSize(p_vPitchSize),
      m_vSensitiveNodeSize(p_vSensitiveNodeSize)
{
    assert(p_vSensorSize.x > 0);
    assert(p_vSensorSize.y > 0);
    assert(p_vSensorSize.z > 0);
    assert(p_vSensitiveNodeDistribution.x > 0);
    assert(p_vSensitiveNodeDistribution.y > 0);
    //assert(1/*p_vPitchSize.x > p_vSensitiveNodeSize.x*/);
    //assert(p_vPitchSize.y > p_vSensitiveNodeSize.y);
    assert(p_vSensitiveNodeSize.x > 0);
    assert(p_vSensitiveNodeSize.y > 0);
    assert(p_vSensitiveNodeSize.z >= 0);
}

const TCuboidSize& Sensor::GetSensorSize() const throw()
{
    return m_vSensorSize;
}

const TPlaneDistribution& Sensor::GetSensitiveNodeDistribution() const throw()
{
    return m_vSensitiveNodeDistribution;
}

const TPlaneDimension& Sensor::GetPitchSize() const throw()
{
    return m_vPitchSize;
}

const TCuboidSize& Sensor::GetSensitiveNodeSize() const throw()
{
    return m_vSensitiveNodeSize;
}

unsigned Sensor::GetNumberOfSensitiveNodes() const throw()
{
    return GetSensitiveNodeDistribution().x * GetSensitiveNodeDistribution().y;
}

TPlanePosition Sensor::GetSensitiveNodePosition(SensitiveNodeId p_vNodeId)
const throw(std_ext::out_of_range_exception)
{
    assert(p_vNodeId.x < GetSensitiveNodeDistribution().x && p_vNodeId.y < GetSensitiveNodeDistribution().y);
    TPlanePosition l_vNodePosition;
    for(unsigned n = 0; n < l_vNodePosition.dimension(); ++n)
    {
        const TLength l_dStepSize = GetPitchSize()[n] + GetSensitiveNodeSize()[n];
        l_vNodePosition[n] = l_dStepSize / 2.0 + l_dStepSize * p_vNodeId[n];
    }
    return l_vNodePosition;
}

Sensor::SensitiveNodeId Sensor::GetNearestSensitiveNodeId(TPlanePosition p_vLocalPosition) const throw()
{
    SensitiveNodeId l_vNodeId;
    for(unsigned n = 0; n < l_vNodeId.dimension(); ++n)
    {
        if(p_vLocalPosition[n] < 0)
            l_vNodeId[n] = 0;
        else if(p_vLocalPosition[n] >= GetSensorSize()[n])
            l_vNodeId[n] = GetSensitiveNodeDistribution()[n] - 1;
        else
        {
            const TLength l_dStepSize = GetPitchSize()[n] + GetSensitiveNodeSize()[n];
            l_vNodeId[n] = (unsigned) std::floor( p_vLocalPosition[n] / l_dStepSize);
            assert(l_vNodeId[n] < GetSensitiveNodeDistribution()[n]);
        }
    }
    return l_vNodeId;
}
