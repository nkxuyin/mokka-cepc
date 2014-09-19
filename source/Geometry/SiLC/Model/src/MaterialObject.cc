/*! \file MaterialObject.cc
    \brief An implementation of Silc::MaterialObject class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "MaterialObject.hh"

using namespace Silc;

MaterialObject::MaterialObject(const string& p_sMaterialName) throw()
    : m_sMaterialName(p_sMaterialName), m_bHasMaterialProperties(false)
{ }

const string& MaterialObject::GetMaterialName() const throw()
{
    return m_sMaterialName;
}

void MaterialObject::SetMaterialProperties(/*TEnergyLinearDensity p_dEnergyLoss,*/ TLength p_dRadiationLength)
throw(std_ext::out_of_range_exception)
{
//    assert(p_dEnergyLoss >= 0);
    assert(p_dRadiationLength >= 0);
//    m_dEnergyLoss = p_dEnergyLoss;
    m_dRadiationLength = p_dRadiationLength;
    m_bHasMaterialProperties = true;
}

bool MaterialObject::HasMaterialProperties() const throw()
{
    return m_bHasMaterialProperties;
}

//TEnergyLinearDensity MaterialObject::GetEnergyLoss() const throw(std_ext::invalid_operation_exception)
//{
//    assert_ex(m_bHasMaterialProperties, std_ext::invalid_operation_exception);
//    return m_dEnergyLoss;
//}

TLength MaterialObject::GetRadiationLength() const throw(std_ext::invalid_operation_exception)
{
    assert_ex(m_bHasMaterialProperties, std_ext::invalid_operation_exception);
    return m_dRadiationLength;
}
