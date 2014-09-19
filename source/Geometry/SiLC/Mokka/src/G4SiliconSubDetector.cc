/*! \file G4SiliconSubDetector.cc
    \brief An implementation of Silc::G4SiliconSubDetector class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "G4SiliconSubDetector.hh"

using namespace Silc;


P<Module> G4SiliconSubDetector::MakeModulePrototype(const Module::SensorArray& p_oSensorArray,
        const Module::ChipArray& p_oChipArray,
        const Module::Support& p_oModuleSupport)
{
    P<G4Module> l_pModule = P<G4Module>(new G4Module(p_oSensorArray, p_oChipArray, p_oModuleSupport));
    m_mModuleOrigins[l_pModule] = l_pModule;
    m_mModuleConstOrigins[l_pModule] = l_pModule;
    return l_pModule;
}


G4Module& G4SiliconSubDetector::GetG4ModulePrototype(unsigned p_uZoneId)
{
    Module* l_pModule = &SiliconSubDetector::GetModulePrototype(p_uZoneId);
    return *m_mModuleOrigins[l_pModule];
}

const G4Module& G4SiliconSubDetector::GetG4ModulePrototype(unsigned p_uZoneId) const
{
    const Module* l_pModule = &SiliconSubDetector::GetModulePrototype(p_uZoneId);
    return *m_mModuleConstOrigins.find(l_pModule)->second;
}
