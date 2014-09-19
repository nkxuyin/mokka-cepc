/*! \file SiliconSubDetector.cc
    \brief An implementation of Silc::SiliconSubDetector class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "SiliconSubDetector.hh"

using namespace Silc;

SiliconSubDetector::SiliconSubDetector()
    : m_bIsAssembled(false)
{
}

unsigned SiliconSubDetector::GetNumberOfModulePrototypes() const
{
    return m_oModulePrototypes.size();
}

void SiliconSubDetector::SetNumberOfModulePrototypes(unsigned p_uNumberOfZones)
{
    assert(p_uNumberOfZones > 0);
    m_oModulePrototypes.clear();
    m_oModulePrototypes.assign(p_uNumberOfZones, nullptr);
}

P<Module> SiliconSubDetector::MakeModulePrototype(const Module::SensorArray& p_oSensorArray,
        const Module::ChipArray& p_oChipArray,
        const Module::Support& p_oModuleSupport)
{
    return P<Module>(new Module(p_oSensorArray, p_oChipArray, p_oModuleSupport));
}

void SiliconSubDetector::InitializeModulePrototype(unsigned p_uZoneId, const Module::SensorArray& p_oSensorArray,
        const Module::ChipArray& p_oChipArray, const Module::Support& p_oModuleSupport)
{
    assert(p_uZoneId < GetNumberOfModulePrototypes());
    m_oModulePrototypes[p_uZoneId] = MakeModulePrototype(p_oSensorArray, p_oChipArray, p_oModuleSupport);
}

Module& SiliconSubDetector::GetModulePrototype(unsigned p_uZoneId)
{
    assert(p_uZoneId < GetNumberOfModulePrototypes());
    assert(m_oModulePrototypes[p_uZoneId] != nullptr);
    return *m_oModulePrototypes[p_uZoneId];
}

const Module& SiliconSubDetector::GetModulePrototype(unsigned p_uZoneId) const
{
    assert(p_uZoneId < GetNumberOfModulePrototypes());
    assert(m_oModulePrototypes[p_uZoneId] != nullptr);
    return *m_oModulePrototypes[p_uZoneId];
}

void SiliconSubDetector::Assemble()
{
    if(m_bIsAssembled) return;
    for(unsigned n = 0; n < GetNumberOfModulePrototypes(); ++n)
        GetModulePrototype(n).Assemble();
    m_bIsAssembled = true;
}
