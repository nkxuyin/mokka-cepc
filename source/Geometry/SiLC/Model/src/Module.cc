/*! \file Module.cc
    \brief An implementation of Silc::Module class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

/*
TODO LIST:
 - Implement the consistency checks. For example, size depends of number of pitch times number of strips.
 - Add additional parameters:
   - The support thickness.
   - The whole module thickness = electronic's thickness + sensor's thickness + support thickness.
   - The support style description.
   - A pitch position.
   - A module stereo angle.
   - A number of strips in one module in which direction.
*/

#include "Module.hh"

using namespace Silc;

Module::Module(const SensorArray& p_oSensorArray, const ChipArray& p_oChipArray,
               const Support& p_oSupport) throw()
{
    m_pSensorArray = new SensorArray(p_oSensorArray);
    m_pChipArray = new ChipArray(p_oChipArray);
    m_pSupport = new Support(p_oSupport);

    const TCuboidSize& l_vSensorSize = p_oSensorArray.GetSensorPrototype().GetSensorSize();
    const TPlaneDistribution& l_vNumberOfSensors = p_oSensorArray.GetNumberOfSensors();
    const TPlaneDimension& l_vGapBetweenSensors = p_oSensorArray.GetGapBetweenSensors();
    const TLength l_dSupportThickness = p_oSupport.GetThickness();
    const TCuboidSize& l_vChipSize = p_oChipArray.GetChipSize();

    m_vModuleSize.x = l_vSensorSize.x * l_vNumberOfSensors.x + (l_vNumberOfSensors.x - 1) * l_vGapBetweenSensors.x;
    m_vModuleSize.y = l_vSensorSize.y * l_vNumberOfSensors.y + (l_vNumberOfSensors.y - 1) * l_vGapBetweenSensors.y;
    m_vModuleSize.z = l_vSensorSize.z + l_dSupportThickness + l_vChipSize.z;
}

Module::Module(const Module& p_oModule) throw()
    : m_pSensorArray(new SensorArray(*p_oModule.m_pSensorArray)),
      m_pChipArray(new ChipArray(*p_oModule.m_pChipArray)),
      m_pSupport(new Support(*p_oModule.m_pSupport)),
      m_vModuleSize(p_oModule.m_vModuleSize)
{ }

void Module::Assemble()
{
}

const Module::SensorArray& Module::GetSensorArray() const throw()
{
    return *m_pSensorArray;
}

Module::SensorArray& Module::GetSensorArray() throw()
{
    return *m_pSensorArray;
}

const Module::ChipArray& Module::GetChipArray() const throw()
{
    return *m_pChipArray;
}

Module::ChipArray& Module::GetChipArray() throw()
{
    return *m_pChipArray;
}

const Module::Support& Module::GetSupport() const throw()
{
    return *m_pSupport;
}

Module::Support& Module::GetSupport() throw()
{
    return *m_pSupport;
}

TCuboidSize Module::GetModuleSize() const throw()
{
    return m_vModuleSize;
}

const string& Module::GetSensitiveDetectorName() const
{
    return m_sSensitiveDetectorName;
}

void Module::SetSensitiveDetectorName(const string& p_sSensitiveDetectorName)
{
    m_sSensitiveDetectorName = p_sSensitiveDetectorName;
}

Module::SensorArray::SensorArray(P<Sensor> p_pSensorPrototype, const TPlaneDistribution& p_vNumberOfSensors,
                                 const TPlaneDimension& p_vGapBetweenSensors) throw(std_ext::out_of_range_exception)
    : m_pSensorPrototype(p_pSensorPrototype), m_vNumberOfSensors(p_vNumberOfSensors),
      m_vGapBetweenSensors(p_vGapBetweenSensors)
{
    assert(p_pSensorPrototype != nullptr);
    assert(p_vNumberOfSensors.x != 0);
    assert(p_vNumberOfSensors.y != 0);
    assert(p_vGapBetweenSensors.x >= 0);
    assert(p_vGapBetweenSensors.y >= 0);
}

const Sensor& Module::SensorArray::GetSensorPrototype() const throw()
{
    return *m_pSensorPrototype;
}

Sensor& Module::SensorArray::GetSensorPrototype() throw()
{
    return *m_pSensorPrototype;
}


const TPlaneDistribution& Module::SensorArray::GetNumberOfSensors() const throw()
{
    return m_vNumberOfSensors;
}

const TPlaneDimension& Module::SensorArray::GetGapBetweenSensors() const throw()
{
    return m_vGapBetweenSensors;
}

Module::ChipArray::ChipArray(const TCuboidSize& p_vChipSize, const TPlaneDistribution& p_vChipDistribution,
                             unsigned p_uNumberOfChannelsPerChip) throw(std_ext::out_of_range_exception)
    : m_vChipSize(p_vChipSize), m_vChipDistribution(p_vChipDistribution),
      m_uNumberOfChannelsPerChip(p_uNumberOfChannelsPerChip)
{
    assert(p_vChipSize.x > 0);
    assert(p_vChipSize.y > 0);
    assert(p_vChipSize.z > 0);
    assert(p_vChipDistribution.x > 0);
    assert(p_vChipDistribution.y > 0);
}

const TCuboidSize& Module::ChipArray::GetChipSize() const throw()
{
    return m_vChipSize;
}

const TPlaneDistribution& Module::ChipArray::GetChipDistribution() const throw()
{
    return m_vChipDistribution;
}

unsigned Module::ChipArray::GetNumberOfChannelsPerChip() const throw()
{
    return m_uNumberOfChannelsPerChip;
}

unsigned Module::ChipArray::GetNumberOfChips() const throw()
{
    return GetChipDistribution().x * GetChipDistribution().y;
}

unsigned Module::ChipArray::GetTotalNumberOfChannels() const throw()
{
    return GetNumberOfChips() * GetNumberOfChannelsPerChip();
}

Module::Support::Support(TLength p_dThickness, TLength p_dWidth, TLength p_dStandoffFromEdge,
                         const string& p_sMaterialName, bool p_bEnabled) throw(std_ext::out_of_range_exception)
    : MaterialObject(p_sMaterialName), m_dThickness(p_dThickness), m_dWidth(p_dWidth),
      m_dStandoffFromEdge(p_dStandoffFromEdge), m_bEnabled(p_bEnabled)
{
    assert(p_dThickness > 0);
    assert(p_dWidth > 0);
    assert(p_dStandoffFromEdge > 0);
}

TLength Module::Support::GetThickness() const throw()
{
    return m_dThickness;
}

TLength Module::Support::GetWidth() const throw()
{
    return m_dWidth;
}

TLength Module::Support::GetStandoffFromEdge() const throw()
{
    return m_dStandoffFromEdge;
}

bool Module::Support::IsEnabled() const throw()
{
    return m_bEnabled;
}

ostream& Silc::operator <<(ostream& o, const Module& p) throw(std::ios_base::failure)
{
    using std_ext::format;

    o << format("------------------------------------------------------------\n");
    o << format(" Sensor dimensions      x=%fmm     y=%fmm     z=%fmm        \n",
                p.GetSensorArray().GetSensorPrototype().GetSensorSize().x,
                p.GetSensorArray().GetSensorPrototype().GetSensorSize().y,
                p.GetSensorArray().GetSensorPrototype().GetSensorSize().z);
    o << format(" Number of sensors      x=%d       y=%d                     \n",
                p.GetSensorArray().GetNumberOfSensors().x,
                p.GetSensorArray().GetNumberOfSensors().y);
    o << format(" Number of chips        %d                                  \n", p.GetChipArray().GetNumberOfChips());
    o << format(" Number of chip rows    %d                                  \n",
                p.GetChipArray().GetChipDistribution().y);
    o << format(" Channels per chip      %d                                  \n",
                p.GetChipArray().GetNumberOfChannelsPerChip());
    o << format(" Chip size              x=%fmm     y=%fmm     z=%fmm        \n", p.GetChipArray().GetChipSize().x,
                p.GetChipArray().GetChipSize().y,
                p.GetChipArray().GetChipSize().z);
    o << format("------------------------------------------------------------\n");

    return o;
}
