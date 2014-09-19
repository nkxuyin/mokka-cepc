/*! \file EndcapArray.cc
    \brief An implementation of Silc::EndcapArray class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "EndcapArray.hh"
#include "XUV_Endcap.hh"
#include "XY_Endcap.hh"

using namespace Silc;

EndcapArray::EndcapTypeMap EndcapArray::InitializeEndcapTypeMap()
{
    EndcapTypeMap result;
    result[XUV_Endcap::ENDCAP_TYPE] = &XUV_Endcap::MakeInstance;
    result[XY_Endcap::ENDCAP_TYPE] = &XY_Endcap::MakeInstance;
    return result;
}

P<Endcap> EndcapArray::MakeEndcap(const Endcap::EndcapType& p_oEndcapType)
{
    static const EndcapTypeMap l_TypeMap = InitializeEndcapTypeMap();
    EndcapTypeMap::const_iterator iter = l_TypeMap.find(p_oEndcapType);
    assert(iter != l_TypeMap.end());
    return iter->second();
}

const EndcapArray::EndcapSet& EndcapArray::GetBaseEndcaps() const
{
    return m_vEndcaps;
}

EndcapArray::EndcapSet& EndcapArray::GetBaseEndcaps()
{
    return m_vEndcaps;
}

void EndcapArray::Assemble()
{
    for(EndcapSet::iterator l_pIter = m_vEndcaps.begin(); l_pIter != m_vEndcaps.end(); ++l_pIter)
    {
        const TLength l_dMaxSquareOverstep = CalculateMaximalSquareOverstep(l_pIter->first);
        l_pIter->first->SetEffectiveInnerRadius(l_dMaxSquareOverstep);
        l_pIter->first->Assemble();
    }
}

unsigned EndcapArray::AddNewEndcap(Endcap::EndcapType p_sEndcapType, TCoordinate p_dZPosition, TAngle p_dRotation)
{
    P<Endcap> l_pEndcap = MakeEndcap(p_sEndcapType);
    return AddEndcap(l_pEndcap, p_dZPosition, p_dRotation);
}

unsigned EndcapArray::AddDuplicateEndcap(unsigned p_uOriginalIndex, TCoordinate p_dZPosition, TAngle p_dRotation)
{
    P<Endcap> l_pEndcap = m_vDescriptors[p_uOriginalIndex]->EndcapObject;
    return AddEndcap(l_pEndcap, p_dZPosition, p_dRotation);
}

unsigned EndcapArray::AddEndcap(P<Endcap> p_pEndcap, TCoordinate p_dZPosition, TAngle p_dRotation)
{
    P<EndcapDescriptor> p_pEndcapDescriptor(new EndcapDescriptor());
    p_pEndcapDescriptor->EndcapObject = p_pEndcap;
    p_pEndcapDescriptor->ZPosition = p_dZPosition;
    p_pEndcapDescriptor->RotationAngle = p_dRotation;
    m_vDescriptors.push_back(p_pEndcapDescriptor);
    unsigned l_uDescriptorId = m_vDescriptors.size() - 1;
    m_vEndcaps[p_pEndcap].push_back(l_uDescriptorId);
    return l_uDescriptorId;
}

const EndcapArray::EndcapDescriptor& EndcapArray::operator[] (unsigned p_uEndcapId) const
{
    assert(p_uEndcapId < GetNumberOfEndcaps());
    return *m_vDescriptors[p_uEndcapId];
}

EndcapArray::EndcapDescriptor& EndcapArray::operator[] (unsigned p_uEndcapId)
{
    assert(p_uEndcapId < GetNumberOfEndcaps());
    return *m_vDescriptors[p_uEndcapId];
}

unsigned EndcapArray::GetNumberOfEndcaps() const
{
    return m_vDescriptors.size();
}

unsigned EndcapArray::GetTotalNumberOfLayers() const
{
    unsigned l_uTotal = 0;
    for(unsigned n = 0; n < GetNumberOfEndcaps(); ++n)
        l_uTotal += (*this)[n].EndcapObject->GetNumberOfJoinedLayers();
    return l_uTotal;
}

unsigned EndcapArray::GetNumberOfZones() const
{
    return m_vZoneAngle.size();
}

void EndcapArray::SetNumberOfZones(unsigned p_uNumberOfZones)
{
    m_vZoneAngle.clear();
    m_vZoneAngle.assign(p_uNumberOfZones, 0);

    for(unsigned n = 0; n < GetNumberOfEndcaps(); ++n)
        (*this)[n].EndcapObject->SetNumberOfZones(p_uNumberOfZones);
}

TAngle EndcapArray::GetZoneAngle(unsigned p_uZoneId) const
{
    assert(p_uZoneId < GetNumberOfZones());
    return m_vZoneAngle[p_uZoneId];
}

void EndcapArray::SetZoneAngle(unsigned p_uZoneId, TAngle p_dZoneAngle)
{
    assert(p_uZoneId < GetNumberOfZones());
    assert(p_dZoneAngle < M_PI_2);

    m_vZoneAngle[p_uZoneId] = p_dZoneAngle;

    for(unsigned n = 0; n < GetNumberOfEndcaps(); ++n)
    {
        P<EndcapDescriptor> l_pDescriptor = m_vDescriptors[n];
        const TLength l_dDistance = abs(l_pDescriptor->ZPosition);
        const TAngle l_dZoneAngle = GetZoneAngle(p_uZoneId);
        const TLength l_dZoneRadius = l_dDistance * tan( l_dZoneAngle );
        l_pDescriptor->EndcapObject->SetZoneRadius(p_uZoneId, l_dZoneRadius);
    }
}

bool EndcapArray::HasMirrorImage() const
{
    return m_bHasMirrorImage;
}

void EndcapArray::SetMirrorImageFlag(bool l_bHasMirrorImage)
{
    m_bHasMirrorImage = l_bHasMirrorImage;
}

TLength EndcapArray::GetSquareOverstep(TLength p_dSquareHalfSide, TAngle p_dAngle)
{
    const TAngle l_dAngle = abs( asin( sin(p_dAngle) ) );
    return p_dSquareHalfSide * sqrt(2.0) * sin(M_PI_4 + l_dAngle);
}

TLength EndcapArray::CalculateMaximalSquareOverstep(P<Endcap> p_pEndcap)
{
    TLength l_dMaxSquareOverstep = p_pEndcap->GetInnerRadius();
    const vector<unsigned>& l_vIndexes = m_vEndcaps[p_pEndcap];

    for(unsigned n = 0; n < l_vIndexes.size(); ++n)
    {
        const TLength l_dInnerRadius = p_pEndcap->GetInnerRadius();
        const TAngle l_dRotationAngle = m_vDescriptors[n]->RotationAngle;
        const TLength l_dSquareOverstep = GetSquareOverstep( l_dInnerRadius, l_dRotationAngle );
        l_dMaxSquareOverstep = max(l_dMaxSquareOverstep, l_dSquareOverstep);
    }
    return l_dMaxSquareOverstep;
}

ostream& Silc::operator<<(ostream& o, const EndcapArray& a)
{
    o << "Number of endcaps = " << a.GetNumberOfEndcaps() << endl;
    o << "Total number of layers = " << a.GetTotalNumberOfLayers() << endl;
    for(unsigned n = 0; n < a.GetNumberOfEndcaps(); ++n)
    {
        o << "Endcap #" << n << "." << std::endl;
        o << *a[n].EndcapObject << std::endl;
    }
    return o;
}



///
/// Following, very fast implementation to get the parameters for GEAR - do for ILD01-pre00 - have to be re-implemented more cleanly
///

//vector<TLength> EndcapArray::GetInnerRadius()
//{
//    vector<TLength> l_vdInnerRadius;
//
//    for(unsigned endcap=0; endcap<GetNumberOfEndcaps(); endcap++)
//    {
//        for(unsigned layer=0; layer<(*this)[endcap].GetNumberOfJoinedLayers(); layer++)
//        {
//            l_vdInnerRadius.push_back((*this)[endcap].GetInnerRadius());
//        }
//    }
//    return l_vdInnerRadius;
//}

//vector<TLength> EndcapArray::GetOuterRadius()
//{
//    vector<TLength> l_vdOuterRadius;
//
//    for(unsigned endcap=0; endcap<GetNumberOfEndcaps(); endcap++)
//    {
//        for(unsigned layer=0; layer<(*this)[endcap].GetNumberOfJoinedLayers(); layer++)
//        {
//            l_vdOuterRadius.push_back((*this)[endcap].GetOuterRadius());
//        }
//    }
//    return l_vdOuterRadius;
//}

// in G4 where it shouldn't be
//vector<TLength> EndcapArray::GetEndcapZPositions()
//{
/*
vector<TLength> l_vdZPosition;

for(unsigned endcap=0; endcap<GetNumberOfEndcaps(); endcap++)
  {
    for(unsigned layer=0; layer<(*this)[endcap].GetNumberOfJoinedLayers(); layer++)
{
  // l_vdZPosition.push_back((*this)[endcap].ZPosition);
  l_vdZPosition.push_back(0);
}
  }
return l_vdZPosition;
*/
//}


//TLength EndcapArray::GetSensorThickness()
//{
//    TLength l_dSensorThickness;
//    l_dSensorThickness = (*this)[0].GetModulePrototype(0).GetSensorArray().GetSensorPrototype().GetSensorSize().z;
//    return l_dSensorThickness;
//
//    //  vector<TLength> l_vdSensorThickness;
//    //  for(unsigned endcap=0; endcap<GetNumberOfEndcaps(); endcap++)
//    //  {
//    //    for(unsigned layer=0; layer<(*this)[endcap].GetNumberOfJoinedLayers())
//    //	{
//    // l_vdSensorThickness.push_back((*this)[endcap]->ZPosition);
//    //	}
//    // }
//    // return l_vdSensorThickness;
//}


//TLength EndcapArray::GetSupportThickness()
//{
//    TLength l_dSupportThickness = 0;
//    l_dSupportThickness = 2*(*this)[0].GetCarbonThickness();
//    l_dSupportThickness += (*this)[0].GetMousseThickness();
//    (*this)[0].GetModulePrototype(0).GetSupport().GetThickness();
//
//    return l_dSupportThickness;
//}



