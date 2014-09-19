/*! \file BarrelArray.cc
    \brief An implementation of Silc::BarrelArray class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "BarrelArray.hh"
#include "BarrelSingleLayer.hh"
#include "BarrelDoubleLayer.hh"

using namespace Silc;
using namespace std;

BarrelArray::BarrelTypeMap BarrelArray::InitializeBarrelTypeMap()
{
    BarrelTypeMap result;
    result[BarrelSingleLayer::BARREL_TYPE] = &BarrelSingleLayer::MakeInstance;
    result[BarrelDoubleLayer::BARREL_TYPE] = &BarrelDoubleLayer::MakeInstance;
    return result;
}

P<Barrel> BarrelArray::MakeBarrel(const Barrel::BarrelType& p_oBarrelType)
{
    static const BarrelTypeMap l_TypeMap = InitializeBarrelTypeMap();
    BarrelTypeMap::const_iterator iter = l_TypeMap.find(p_oBarrelType);
    assert_ex(iter != l_TypeMap.end(), Exception);
    return iter->second();
}

void BarrelArray::Assemble()
{
    for(unsigned n=0; n<GetNumberOfBarrels(); n++)
        (*this)[n].Assemble();
}

Barrel& BarrelArray::AddNewBarrel(const Barrel::BarrelType& p_sBarrelType)
{
    P<Barrel> p_pBarrel = MakeBarrel(p_sBarrelType);
    m_apBarrels.push_back(p_pBarrel);
    const unsigned l_uBarrelId = m_apBarrels.size() - 1;
    for(unsigned n = 0; n < p_pBarrel->GetNbLayers(); ++n)
        m_vLayers.push_back(new LayerDescriptor(l_uBarrelId, n));
    return *p_pBarrel;
}

const Barrel& BarrelArray::operator[] (unsigned p_uBarrelId) const
{
    assert(p_uBarrelId < GetNumberOfBarrels());
    return *m_apBarrels[p_uBarrelId];
}

Barrel& BarrelArray::operator[] (unsigned p_uBarrelId)
{
    assert(p_uBarrelId < GetNumberOfBarrels());
    return *m_apBarrels[p_uBarrelId];
}

unsigned BarrelArray::GetNumberOfBarrels() const
{
    return m_apBarrels.size();
}

unsigned BarrelArray::GetTotalNumberOfLayers() const
{
    unsigned l_uTotal = 0;
    for(unsigned n = 0; n < GetNumberOfBarrels(); ++n)
        l_uTotal += (*this)[n].GetNbLayers();
    return l_uTotal;
}

unsigned BarrelArray::GetNbLayerInTheCurrentBarrel(unsigned p_uBarrelId) const
{
    return (*this)[p_uBarrelId].GetNbLayers();
}

TLength BarrelArray::GetInnerRadius(unsigned p_uBarrelId) const
{
    return (*this)[p_uBarrelId].GetInnerRadiusMin();
}

TLength BarrelArray::GetZLength(unsigned p_uBarrelId) const
{
    return (*this)[p_uBarrelId].GetLength();
}

const Sensor& BarrelArray::GetSensorPrototype(unsigned p_uLayerId) const
{
    assert(p_uLayerId < m_vLayers.size());
    const LayerDescriptor& l_oDescriptor =  *m_vLayers[p_uLayerId];
    const Barrel& l_oBarrel = (*this)[l_oDescriptor.BarrelId];
    return l_oBarrel.GetModulePrototype(l_oDescriptor.LayerId).GetSensorArray().GetSensorPrototype();
}

TVector BarrelArray::TransformVectorToLocal(const TVector& p_vGlobalVector, GlobalNodeId p_uSensorId) const
{
    const unsigned l_uLayerId = p_uSensorId.GetLayerId();
    assert(l_uLayerId < m_vLayers.size());
    const LayerDescriptor& l_oDescriptor =  *m_vLayers[l_uLayerId];
    const Barrel& l_oBarrel = (*this)[l_oDescriptor.BarrelId];
    return l_oBarrel.TransformVectorToLocal(p_vGlobalVector, p_uSensorId, l_oDescriptor.LayerId);
}

TVector BarrelArray::TransformVectorToGlobal(const TVector& p_vLocalVector, GlobalNodeId p_uSensorId) const
{
    const unsigned l_uLayerId = p_uSensorId.GetLayerId();
    assert(l_uLayerId < m_vLayers.size());
    const LayerDescriptor& l_oDescriptor =  *m_vLayers[l_uLayerId];
    const Barrel& l_oBarrel = (*this)[l_oDescriptor.BarrelId];
    return l_oBarrel.TransformVectorToGlobal(p_vLocalVector, p_uSensorId, l_oDescriptor.LayerId);
}

GlobalNodeId BarrelArray::FindGlobalNodeId(const TVector& p_vGlobalVector) const
{
    assert(GetNumberOfBarrels() >= 1);
    const TLength rho = sqrt( sqr(p_vGlobalVector.x) + sqr(p_vGlobalVector.y) );
    unsigned currentBarrelId = 0;
    unsigned layerIdShift = 0;
    for(; currentBarrelId < GetNumberOfBarrels() - 1; ++currentBarrelId)
    {
        const unsigned nextBarrelId = currentBarrelId + 1;
        const TLength currentInnerRadius = (*this)[currentBarrelId].GetLayerRadius(0);
        const TLength nextInnerRadius = (*this)[nextBarrelId].GetLayerRadius(0);
        if(rho >= currentInnerRadius && rho < nextInnerRadius)
            break;
        layerIdShift += (*this)[currentBarrelId].GetNbLayers();
    }
    return (*this)[currentBarrelId].FindGlobalNodeId(p_vGlobalVector, layerIdShift);
}

const set<string>& BarrelArray::GetSensitiveDetectorNames() const
{
    return m_SensitiveDetectorNames;
}

void BarrelArray::SetSensitiveDetectorNames(const string& p_sPrefix)
{
    m_SensitiveDetectorNames.clear();
    for(unsigned barrel=0; barrel<GetNumberOfBarrels(); barrel++)
    {
        Barrel& l_oBarrel = (*this)[barrel];
        stringstream l_sNewPrefix;
        l_sNewPrefix << p_sPrefix << "_barrel_" << barrel;
        l_oBarrel.SetSensitiveDetectorNames(l_sNewPrefix.str());
        const set<string>& names = l_oBarrel.GetSensitiveDetectorNames();
        m_SensitiveDetectorNames.insert(names.begin(), names.end());
    }
}

const BarrelArray::LayerDescriptor& BarrelArray::GetLayerDescriptor(unsigned p_uLayerId) const
{
    assert(p_uLayerId < m_vLayers.size());
    return *m_vLayers[p_uLayerId];
}

BarrelArray::LayerDescriptor::LayerDescriptor(unsigned p_uBarrelId, unsigned p_uLayerId)
    : BarrelId(p_uBarrelId), LayerId(p_uLayerId)
{
}

ostream& Silc::operator<<(ostream& o, const BarrelArray& a)
{
    o << "Number of barrels = " << a.GetNumberOfBarrels() << endl;
    o << "Total number of layers = " << a.GetTotalNumberOfLayers() << endl;

    for(unsigned n=0; n<a.GetNumberOfBarrels(); n++)
    {
        o << "Barrel #" << n << "." << std::endl;
        o << a[n] << std::endl;
    }

    return o;
}
