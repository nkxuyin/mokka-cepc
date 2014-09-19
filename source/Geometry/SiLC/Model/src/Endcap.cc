/*! \file Endcap.cc
    \brief An implementation of Silc::Endcap class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "Endcap.hh"

using namespace Silc;

// Constants.
const Endcap::ModuleDescriptor::RotationIndicator Endcap::ModuleDescriptor::ZeroRotation = 0.0;
const Endcap::ModuleDescriptor::RotationIndicator Endcap::ModuleDescriptor::OrtogonalRotation = M_PI_2;

Endcap::Endcap()
    : m_pCarbonMaterial(nullptr), m_pMousseMaterial(nullptr), m_pModulesDistribution( new ModulesDistribution() )
{
}

TLength Endcap::GetInnerRadius() const
{
    return m_dInnerRadius;
}

TLength Endcap::GetOuterRadius() const
{
    return m_dOuterRadius;
}

void Endcap::SetLimits(TLength p_dInnerRadius, TLength p_dOuterRadius)
{
    if(p_dInnerRadius < 0)
        throw Exception("Endcap::SetLimits : The inner radius can't be less than zero.");
    if(p_dOuterRadius < 0)
        throw Exception("Endcap::SetLimits : The outer radius can't be less than zero.");
    if(p_dInnerRadius > p_dOuterRadius)
        throw Exception("Endcap::SetLimits : The inner radius can't be bigger than the outer");
    m_dOuterRadius = p_dOuterRadius;
    m_dInnerRadius = p_dInnerRadius;
}

TLength Endcap::GetEffectiveInnerRadius() const
{
    return m_dEffectiveInnerRadius;
}

void Endcap::SetEffectiveInnerRadius(TLength p_dEffectiveInnerRadius)
{
    if(p_dEffectiveInnerRadius < GetInnerRadius())
        throw Exception("Endcap::SetEffectiveInnerRadius : Effective radius cann't be less than inner radius.");
    m_dEffectiveInnerRadius = p_dEffectiveInnerRadius;
}

Endcap::ModulesDistribution& Endcap::GetModulesDistribution()
{
    return *m_pModulesDistribution;
}

const Endcap::ModulesDistribution& Endcap::GetModulesDistribution() const
{
    return *m_pModulesDistribution;
}

TLength Endcap::GetActiveInnerRadius() const
{
    return GetEffectiveInnerRadius() + GetPlaneSupportWidth();
}

TLength Endcap::GetActiveOuterRadius() const
{
    return GetOuterRadius() - GetCircularSupportWidth();
}

TLength Endcap::GetPlaneSupportWidth() const
{
    return m_dPlaneSupportWidth;
}

TLength Endcap::GetCircularSupportWidth() const
{
    return m_dCircularSupportWidth;
}

void Endcap::SetSupermoduleSupportInformation(TLength p_dPlaneSupportWidth, TLength p_dCircularSupportWidth)
{
    if(p_dPlaneSupportWidth <= 0)
        throw Exception( "Endcap::SetSupermoduleSupportInformation : The plane support width should be positive.");
    if(p_dCircularSupportWidth <= 0)
        throw Exception( "Endcap::SetSupermoduleSupportInformation : The circular support width sould be positive.");

    m_dPlaneSupportWidth = p_dPlaneSupportWidth;
    m_dCircularSupportWidth = p_dCircularSupportWidth;
}

TLength Endcap::GetCarbonThickness() const
{
    return m_dCarbonThickness;
}

TLength Endcap::GetMousseThickness() const
{
    return m_dMousseThickness;
}

TLength Endcap::GetReinforcementThickness() const
{
    return m_dReinforcementThickness;
}

void Endcap::SetEndcapSupportInformation(TLength p_dCarbonThickness, TLength p_dMousseThickness,
        TLength p_dReinforcementThickness)
{
    if(p_dCarbonThickness <= 0)
        throw Exception( "Endcap::SetEndcapSupportInformation : The carbon thickness should be positive.");
    if(p_dMousseThickness <= 0)
        throw Exception( "Endcap::SetEndcapSupportInformation : The mousse thickness should be positive.");
    if(p_dReinforcementThickness <= 0)
        throw Exception( "Endcap::SetEndcapSupportInformation : The reinforcement thickness should be positive.");

    m_dCarbonThickness = p_dCarbonThickness;
    m_dMousseThickness = p_dMousseThickness;
    m_dReinforcementThickness = p_dReinforcementThickness;
}

TLength Endcap::GetSuperModuleShift() const
{
    return GetEffectiveInnerRadius();
}

TLength Endcap::GetActiveSuperModuleShift() const
{
    return GetSuperModuleShift() - GetPlaneSupportWidth();
}

unsigned Endcap::GetNumberOfZones() const
{
    return m_vZoneRadius.size();
}

void Endcap::SetNumberOfZones(unsigned p_uNumberOfZones)
{
    SiliconSubDetector::SetNumberOfModulePrototypes(p_uNumberOfZones);
    m_vZoneRadius.clear();
    m_vZoneRadius.assign(p_uNumberOfZones, 0);
}

TLength Endcap::GetZoneRadius(unsigned p_uZoneId) const
{
    assert(p_uZoneId < GetNumberOfZones());
    if(p_uZoneId == GetNumberOfZones() - 1)
        return GetActiveOuterRadius();
    return m_vZoneRadius[p_uZoneId];
}

void Endcap::SetZoneRadius(unsigned p_uZoneId, TLength p_dZoneRadius)
{
    assert(p_uZoneId < GetNumberOfZones());
    m_vZoneRadius[p_uZoneId] = p_dZoneRadius;
}

TLength Endcap::GetMaxModuleThickness() const
{
    TLength l_dThickness = 0;
    for(unsigned l_uZoneId = 0; l_uZoneId < GetNumberOfZones(); ++l_uZoneId)
        l_dThickness = max(l_dThickness, GetModulePrototype(l_uZoneId).GetModuleSize().z);
    return l_dThickness;
}

TLength Endcap::GetThickness() const
{
    const TLength l_dSupportThickness = GetMousseThickness() + GetCarbonThickness();
    const TLength l_dPeggedThickness = max(GetMaxModuleThickness(), GetReinforcementThickness());
    return l_dSupportThickness + l_dPeggedThickness * GetNumberOfJoinedLayers();
}

void Endcap::SetSupportMaterials(const MaterialObject& p_oCarbonSupport, const MaterialObject& p_oMousseSupport)
{
    m_pCarbonMaterial = P<MaterialObject>(new MaterialObject(p_oCarbonSupport));
    m_pMousseMaterial = P<MaterialObject>(new MaterialObject(p_oMousseSupport));
}

const MaterialObject& Endcap::GetCarbonSupportMaterial() const
{
    assert_ex(m_pCarbonMaterial != nullptr, std_ext::invalid_operation_exception);
    return *m_pCarbonMaterial;
}

const MaterialObject& Endcap::GetMousseSupportMaterial() const
{
    assert_ex(m_pMousseMaterial != nullptr, std_ext::invalid_operation_exception);
    return *m_pMousseMaterial;
}

Endcap::ModuleDescriptor::ModuleDescriptor(unsigned p_uLayerId, unsigned p_uZoneId, RotationIndicator p_bModuleRotation,
        const TPlanePosition& p_adModulePosition)
    : LayerId(p_uLayerId), ZoneId(p_uZoneId), ModuleRotation(p_bModuleRotation), ModulePosition(p_adModulePosition)
{
}

void Endcap::ModulesDistribution::Add(const ModuleDescriptor& p_oDescriptor)
{
    m_oDescriptors.push_back(p_oDescriptor);
    m_oLayerIndex[p_oDescriptor.LayerId].push_back(p_oDescriptor);
}

void Endcap::ModulesDistribution::Clear()
{
    m_oDescriptors.clear();
}

Endcap::ModulesDistribution::Iterator Endcap::ModulesDistribution::Begin() const
{
    return m_oDescriptors.begin();
}

Endcap::ModulesDistribution::Iterator Endcap::ModulesDistribution::End() const
{
    return m_oDescriptors.end();
}

const Endcap::ModulesDistribution::DescriptorCollection& Endcap::ModulesDistribution::SelectLayer(unsigned p_uLayerId)
{
    return m_oLayerIndex[p_uLayerId];
}

Endcap::Filler::Filler(unsigned p_uNumberOfZones, TLength p_dInitialPosition)
    : m_dInitialPosition(p_dInitialPosition)
{
    assert(p_uNumberOfZones);
    m_vModuleLengths.assign(p_uNumberOfZones, 0);
    m_vNumberOfModules.assign(p_uNumberOfZones, 0);
    m_vZoneLeftLimits.assign(p_uNumberOfZones, p_dInitialPosition);
    m_vZoneRightLimits.assign(p_uNumberOfZones, p_dInitialPosition);
    m_vInitializedZones.assign(p_uNumberOfZones, false);
}

void Endcap::Filler::InitializeZone(unsigned p_uZoneId, TLength p_dModuleLength,
                                    TLength p_dZoneLeftLimit, TLength p_dZoneRightLimit)
{
    assert(p_uZoneId < GetNumberOfZones());
    assert(p_dModuleLength > 0.0);

    m_vModuleLengths[p_uZoneId] = p_dModuleLength;
    m_vZoneLeftLimits[p_uZoneId] = p_dZoneLeftLimit;
    m_vZoneRightLimits[p_uZoneId] = p_dZoneRightLimit;
    m_vInitializedZones[p_uZoneId] = true;
}

const vector<unsigned>& Endcap::Filler::CalculateLadderStepFilling()
{
    CheckIntegrity();
    ApplyDefaultLadderStepFilling();
    ApplyStepFillingOptimization();
    return m_vNumberOfModules;
}

void Endcap::Filler::CheckIntegrity() const
{
    for(unsigned n = 0; n < GetNumberOfZones(); ++n)
        assert_ex(m_vInitializedZones[n], std_ext::invalid_operation_exception);
}

unsigned Endcap::Filler::GetNumberOfZones() const
{
    return m_vModuleLengths.size();
}

TLength Endcap::Filler::GetZoneLimit(unsigned p_uZoneId) const
{
    typedef const TLength& (*compare_function)(const TLength&, const TLength&);
    compare_function l_fCompare;
    if(p_uZoneId < GetNumberOfZones() - 1)
        l_fCompare = &max<TLength>;
    else
        l_fCompare = &min<TLength>;
    return l_fCompare(m_vZoneLeftLimits[p_uZoneId], m_vZoneRightLimits[p_uZoneId]);
}

TLength Endcap::Filler::GetStepLength() const
{
    const TLength l_dLastZoneLimit = GetZoneLimit(GetNumberOfZones() - 1);
    return max(l_dLastZoneLimit - m_dInitialPosition, 0.0);
}

TLength Endcap::Filler::GetFillingLength() const
{
    TLength l_dFillingLength = 0;
    for(unsigned n = 0; n < GetNumberOfZones(); ++n)
        l_dFillingLength += m_vNumberOfModules[n] * m_vModuleLengths[n];
    return l_dFillingLength;
}

void Endcap::Filler::ApplyDefaultLadderStepFilling()
{
    typedef double (*round_function)(double);
    TLength l_dFirstAvaiblePosition = m_dInitialPosition;
    for(unsigned l_uZoneId = 0; l_uZoneId < GetNumberOfZones(); ++l_uZoneId)
    {
        const TLength l_dZoneLimit = GetZoneLimit(l_uZoneId);
        const TLength l_dModuleLength = m_vModuleLengths[l_uZoneId];
        const double l_dAvaibleModules = max( (l_dZoneLimit - l_dFirstAvaiblePosition) / l_dModuleLength, 0.0 );
        round_function l_fRound = l_uZoneId == GetNumberOfZones() - 1 ? &floor : &ceil;
        const unsigned l_uNumberOfModules =  l_fRound(l_dAvaibleModules);
        l_dFirstAvaiblePosition += l_uNumberOfModules * l_dModuleLength;
        m_vNumberOfModules[l_uZoneId] = l_uNumberOfModules;
    }
}

void Endcap::Filler::ApplyStepFillingOptimization()
{
    const TLength l_dStepLength = GetStepLength();
    TLength l_dFillingLength = GetFillingLength();

    for(unsigned n = 0; n < GetNumberOfZones(); ++n)
    {

        const unsigned l_uZoneId = GetNumberOfZones() - n - 1;
        const TLength l_dModuleLength = m_vModuleLengths[l_uZoneId];
        while(l_dStepLength - l_dFillingLength >= l_dModuleLength)
        {
            ++m_vNumberOfModules[l_uZoneId];
            l_dFillingLength += l_dModuleLength;
        }
    }
}

ostream& Silc::operator<<(ostream& o, const Endcap& e)
{
    using std_ext::format;

    o << format("============================================================\n");
    o << format(" Inner Radius           R=%fmm		                         \n", e.GetInnerRadius());
    o << format(" Outer Radius           R=%fmm		                         \n", e.GetOuterRadius());
    o << format(" Number of Layers       %d                                  \n", e.GetNumberOfJoinedLayers());
    o << format(" Number of Zones        %d                                  \n", e.GetNumberOfZones());
    for(unsigned zone = 0; zone < e.GetNumberOfZones()-1 ; zone++)
        o << format("   ---->  Zone Radius   %d           %f                     \n", zone, e.GetZoneRadius(zone));
    for(unsigned zone=0; zone < e.GetNumberOfZones(); zone++)
        o << e.GetModulePrototype(zone);
    o << format("============================================================\n");
    return o;
}
