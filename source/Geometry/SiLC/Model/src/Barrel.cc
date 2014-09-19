/*! \file Barrel.cc
    \brief An implementation of Silc::Barrel class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "Barrel.hh"

using namespace Silc;

Barrel::Barrel()
    : m_oModulesDistribution(new ModulesDistribution()), m_bIsAssembled(false)
{
}

void Barrel::Initialize()
{
    m_uNbLayers = GetNbLayers();
    TPlaneDistribution l_auInit2D;
    l_auInit2D.x=0;
    l_auInit2D.y=0;
    m_avuSuperModuleNbTiles.assign(m_uNbLayers, l_auInit2D);

    TCuboidSize l_auInit3D;
    l_auInit3D.x=0;
    l_auInit3D.y=0;
    l_auInit3D.z=0;
    m_avdSuperModuleSize.assign(m_uNbLayers, l_auInit3D);

    m_vdModuleDirection.assign(m_uNbLayers, 0);
    m_vdModuleFace.assign(m_uNbLayers, 0);

    SetNumberOfModulePrototypes(m_uNbLayers);

    m_vdSensitiveRadius.assign(m_uNbLayers, 0);
    m_oLayerDescriptors.clear();
}

TCuboidSize Barrel::GetEffectiveModuleSize(unsigned p_uLayerId)
{
    const TCuboidSize& moduleSize = GetModulePrototype(p_uLayerId).GetModuleSize();
    const TAngle moduleRotation = GetModuleDirection(p_uLayerId);
    const TLength diagonalLength = sqrt( sqr(moduleSize.x) + sqr(moduleSize.y) );
    const TAngle diagonalAngle = asin(moduleSize.y / diagonalLength);
    const TAngle firstDiagonalRotation = moduleRotation + diagonalAngle;
    const TAngle secondDiagonalRotation = moduleRotation - diagonalAngle;
    const TLength maxCos = max( abs( cos( firstDiagonalRotation ) ), abs( cos( secondDiagonalRotation ) ) );
    const TLength maxSin = max( abs( sin( firstDiagonalRotation ) ), abs( sin( secondDiagonalRotation ) ) );

    TCuboidSize effectiveSize;
    effectiveSize.x = diagonalLength * maxCos;
    effectiveSize.y = diagonalLength * maxSin;
    effectiveSize.z = moduleSize.z;
    return effectiveSize;
}

TCuboidSize Barrel::GetMaxSModuleSize()
{
    TCuboidSize l_adSuperModuleMaxSize(m_avdSuperModuleSize[0]);
    for(unsigned layer = 1; layer < GetNbLayers(); ++layer)
        for(unsigned n = 0; n < l_adSuperModuleMaxSize.dimension(); ++n)
            l_adSuperModuleMaxSize[n] = max(l_adSuperModuleMaxSize[n], m_avdSuperModuleSize[layer][n]);
    return l_adSuperModuleMaxSize;
}

TPlaneDistribution Barrel::GetEffectiveNbModuleSensor(unsigned p_uLayerId)
{
    assert_ex(GetNbLayers()==2, Exception);

    TPlaneDistribution l_auNbSensors = GetModulePrototype(p_uLayerId).GetSensorArray().GetNumberOfSensors();

    if(m_vdModuleDirection[p_uLayerId]==0)
    {
        unsigned l_uTemp = l_auNbSensors.x;
        l_auNbSensors.x = l_auNbSensors.y;
        l_auNbSensors.y = l_uTemp;
    }

    return l_auNbSensors;
}

const TCuboidSize& Barrel::GetSuperModuleSize(unsigned p_uLayerId) const
{
    return m_avdSuperModuleSize[p_uLayerId];
}

void Barrel::SetSuperModuleSize(unsigned p_uLayerId, const TCuboidSize& p_adSuperModuleSize)
{
    assert(p_uLayerId < GetNbLayers());
    m_avdSuperModuleSize[p_uLayerId] = p_adSuperModuleSize;
}

void Barrel::SetLength(TLength p_dLength)
{
    m_dLength = p_dLength;
}

TLength Barrel::GetLength() const
{
    return m_dLength;
}

TLength Barrel::GetInitialInnerRadius() const
{
    return m_dInitialInnerRadius;
}

void Barrel::SetInitialInnerRadius(TLength p_dInitialInnerRadius)
{
    assert(p_dInitialInnerRadius > 0);
    m_dInitialInnerRadius = p_dInitialInnerRadius;
}

void Barrel::SetInnerRadiusMin(TLength p_dMinRadius)
{
    m_dMinRadius = p_dMinRadius;
}

TLength Barrel::GetInnerRadiusMin() const
{
    return m_dMinRadius;
}

void Barrel::SetSensitiveRadius(unsigned p_uLayerId, TLength p_dRadius)
{
    assert(p_uLayerId < GetNbLayers());
    m_vdSensitiveRadius[p_uLayerId] = p_dRadius;
}

void Barrel::SetNbFace(unsigned p_uNbFace)
{
    assert(p_uNbFace >= 3);
    m_uNbFaces = p_uNbFace;
}

unsigned Barrel::GetNbFace() const
{
    return m_uNbFaces;
}

void Barrel::SetBarrelName(std::string p_sBarrelName)
{
    m_sBarrelName = p_sBarrelName;
}

std::string Barrel::GetBarrelName() const
{
    return m_sBarrelName;
}

unsigned Barrel::GetNbOfTilesAlongPhi(unsigned p_uLayerId) const
{
    return m_avuSuperModuleNbTiles[p_uLayerId].x;
}

unsigned Barrel::GetNbOfTilesAlongZ(unsigned p_uLayerId) const
{
    return m_avuSuperModuleNbTiles[p_uLayerId].y;
}

void Barrel::SetSuperModuleNbOfTiles(unsigned p_uLayerId, const TPlaneDistribution& p_auNbTiles)
{
    m_avuSuperModuleNbTiles[p_uLayerId].x = p_auNbTiles.x;
    m_avuSuperModuleNbTiles[p_uLayerId].y = p_auNbTiles.y;
}

const TPlaneDistribution& Barrel::GetSuperModuleNbTiles(unsigned p_uLayerId) const
{
    return m_avuSuperModuleNbTiles[p_uLayerId];
}

void Barrel::SetBarrelShape(TLength p_dRadius, TLength p_dLength)
{
    SetLength(p_dLength);
    SetInitialInnerRadius(p_dRadius);
    SetInnerRadiusMin(p_dRadius);
}

const string& Barrel::GetBarrelSupportMaterial() const
{
    return m_sSupportMaterial;
}

TLength Barrel::GetBarrelSupportThickness() const
{
    return m_dSupportThickness;
}

void Barrel::SetBarrelSupportParameters(string p_sBarrelSupportMaterial, TLength p_dBarrelSupportThickness)
{
    m_sSupportMaterial = p_sBarrelSupportMaterial;
    m_dSupportThickness = p_dBarrelSupportThickness;
}

bool Barrel::IsSupportEnable() const
{
    return m_bEnableSupport;
}

void Barrel::EnableSupport(bool p_bEnable)
{
    m_bEnableSupport = p_bEnable;
}

void Barrel::SetAdjustement(string p_sRadialAdjustement, string p_sLongitudinalAdjustement)
{
    m_sRadialAdjustement = p_sRadialAdjustement;
    m_sLongitudinalAdjustement = p_sLongitudinalAdjustement;
}

void Barrel::SetModuleOrientation(unsigned p_uLayerId, TAngle p_dAngle2Z, TAngle p_dFaceToBeam)
{
    m_vdModuleDirection[p_uLayerId] = p_dAngle2Z;
    m_vdModuleFace[p_uLayerId] = p_dFaceToBeam;
}

TAngle Barrel::GetModuleDirection(unsigned p_uLayerId) const
{
    return m_vdModuleDirection[p_uLayerId];
}

TAngle Barrel::GetModuleFace(unsigned p_uLayerId) const
{
    return m_vdModuleFace[p_uLayerId];
}

std::string Barrel::GetRadialAdjustment() const
{
    return m_sRadialAdjustement;
}

std::string Barrel::GetLongitudinalAdjustment() const
{
    return m_sLongitudinalAdjustement;
}

Barrel::ModulesDistribution& Barrel::GetModulesDistribution()
{
    return *m_oModulesDistribution;
}

const Barrel::ModulesDistribution& Barrel::GetModulesDistribution() const
{
    return *m_oModulesDistribution;
}

TAngle Barrel::GetInitialFaceAngle() const
{
    return 0;
}

TAngle Barrel::GetFaceRotationAngle(unsigned p_uFaceId) const
{
    assert(p_uFaceId < GetNbFace());
    return p_uFaceId * 2.0 * M_PI / GetNbFace();
}

// do form barrelid
Barrel::ModuleDescriptor::ModuleDescriptor(unsigned p_uLayerId, RotationIndicator /*p_bModuleRotation*/,
        const TSolidAngle& p_adModuleRotation, const TPosition& p_adModulePosition)
    :  m_uLayerID(p_uLayerId)
{
    for(unsigned n=0; n<SPATIAL_DIMENSION; n++)
    {
        m_adModulePosition[n] = p_adModulePosition[n];
        m_adModuleRotation[n] = p_adModuleRotation[n];
    }
}

Barrel::ModulesDistribution::Iterator Barrel::ModulesDistribution::Begin() const
{
    return m_oDescriptors.begin();
}

Barrel::ModulesDistribution::Iterator Barrel::ModulesDistribution::End() const
{
    return m_oDescriptors.end();
}

void Barrel::ModulesDistribution::Add(const ModuleDescriptor& p_oDescriptor)
{
    m_oDescriptors.push_back(p_oDescriptor);
    m_oLayerIndex[p_oDescriptor.m_uLayerID].push_back(p_oDescriptor);
}

const Barrel::ModulesDistribution::DescriptorCollection&
Barrel::ModulesDistribution::SelectLayer(unsigned p_uLayerId) const
{
    LayerIndex::const_iterator iter = m_oLayerIndex.find(p_uLayerId);
    assert(iter != m_oLayerIndex.end());
    return iter->second;
}

Barrel::LayerDescriptor::LayerDescriptor(TLength p_dFirstSurfaceRadius, TLength p_dSensitiveSurfaceRadius,
        TLength p_dThickness, TLength p_dSensitiveSurfaceThickness)
    : FirstSurfaceRadius(p_dFirstSurfaceRadius), SensitiveSurfaceRadius(p_dSensitiveSurfaceRadius),
      Thickness(p_dThickness), SensitiveSurfaceThickness(p_dSensitiveSurfaceThickness)
{ }

std::ostream& Silc::operator<<(std::ostream& o, const Barrel& p)
{
    using std_ext::format;

    o << format("============================================\n");
    //o << format(" Support                %s                           \n", p.GetSupportStyle().data());
    //o << format(" SupportThickness       %f                           \n", p.GetSupportThickness());
    //o << format(" Angle                  phi=%f   theta=%f   psi=%f   \n", p.GetModuleAngle().psi, p.GetModuleAngle().theta, p.GetModuleAngle().phi);
    //o << format(" Shift                  x=%fmm   y=%fmm     z=%fmm   \n", p.GetModuleShifting().x, p.GetModuleShifting().y, p.GetModuleShifting().z);
    o << format(" NbFaces                %d                           \n", p.GetNbFace());
    o << format(" MinRadius              %f mm                         \n", p.GetInnerRadiusMin());
    o << format(" dLength                %fmm                         \n", p.GetLength());
    o << format(" NbLayers                %d                           \n", p.GetNbLayers());
    //o << format(" NbOfTileAlongPhiPerSModule   %d                     \n", p.GetNbOfTilesAlongPhiPerSModule());
    //o << format(" NbOfTileAlongZPerSModule     %d                     \n", p.GetNbOfTilesAlongZPerSModule());
    //o << format(" NbOfTileAlongPhi       %d                           \n", p.GetNbOfTilesAlongPhi(0));
    //o << format(" NbOfTileAlongZ         %d                           \n", p.GetNbOfTilesAlongZ(0));

    for(unsigned layer=0; layer<p.GetNbLayers(); layer ++)
    {
        o << format("========================== LAYER ==================\n");
        o << format(" NbOfTileAlongPhiPerSModule   %d                     \n", p.GetSuperModuleNbTiles(layer).x);
        o << format(" NbOfTileAlongZPerSModule   %d                     \n", p.GetSuperModuleNbTiles(layer).y);
        o << format(" SModuleSize   %f                     \n", p.GetSuperModuleSize(layer).x);
        o << format(" SModuleSize   %f                     \n", p.GetSuperModuleSize(layer).y);

        o << p.GetModulePrototype(layer);
    }
    o << format("============================================\n");
    return o;
}

TLength Barrel::GetLayerRadius(unsigned l_uLayerId) const
{
    return GetLayerDescriptor(l_uLayerId).FirstSurfaceRadius;
}

TLength Barrel::GetLayerThickness(unsigned l_uLayerId) const
{
    return GetLayerDescriptor(l_uLayerId).Thickness;
}

TLength Barrel::GetLayerWidth(unsigned l_uLayerId) const
{
    assert(l_uLayerId < GetNbLayers());
    return GetSuperModuleSize(l_uLayerId).x;
}

TLength Barrel::GetLayerLength(unsigned l_uLayerId) const
{
    assert(l_uLayerId < GetNbLayers());
    return GetSuperModuleSize(l_uLayerId).y;
}

TLength Barrel::GetMeanSupportRadiationLength(unsigned l_uLayerId) const
{
    assert(l_uLayerId < GetNbLayers());
    return GetModulePrototype(l_uLayerId).GetSupport().GetRadiationLength();
}

TLength Barrel::GetSensitiveVolumeRadius(unsigned l_uLayerId) const
{
    return GetLayerDescriptor(l_uLayerId).SensitiveSurfaceRadius;
}

TLength Barrel::GetSensitiveVolumeThickness(unsigned l_uLayerId) const
{
    return GetLayerDescriptor(l_uLayerId).SensitiveSurfaceThickness;
}

TLength Barrel::GetSensitiveVolumeWidth(unsigned l_uLayerId) const
{
    assert(l_uLayerId < GetNbLayers());
    return GetLayerWidth(l_uLayerId);
}

TLength Barrel::GetSensitiveVolumeRadiationLength(unsigned l_uLayerId) const
{
    assert(l_uLayerId < GetNbLayers());
    return GetModulePrototype(l_uLayerId).GetSensorArray().GetSensorPrototype().GetRadiationLength();
}

TVector Barrel::TransformVectorToLocal(const TVector& p_vGlobalVector, GlobalNodeId p_uSensorId,
                                       unsigned p_uLayerId) const
{
    const ModulesDistribution::DescriptorCollection& l_vModuleDescriptors =
        GetModulesDistribution().SelectLayer(p_uLayerId);
    const unsigned l_uSuperModuleId = p_uSensorId.GetSuperModuleId();
    const unsigned l_uModuleId = p_uSensorId.GetModuleId();
    assert(l_uModuleId < l_vModuleDescriptors.size());
    const ModuleDescriptor& l_oModuleDescriptor = l_vModuleDescriptors[l_uModuleId];
    const LayerDescriptor& l_oLayerDescriptor = GetLayerDescriptor(p_uLayerId);
    const TAngle l_dModuleRotation = l_oModuleDescriptor.m_adModuleRotation.phi;
    const TAngle l_dSuperModuleRotation = GetFaceRotationAngle(l_uSuperModuleId);
    const TPosition& l_vModulePosition = l_oModuleDescriptor.m_adModulePosition;
    const TPosition l_vShift(l_vModulePosition.x, l_vModulePosition.y, l_oLayerDescriptor.SensitiveSurfaceRadius);

    TVector rotated = p_vGlobalVector;
    rotated.rotate_z(-l_dSuperModuleRotation);
    TVector switched(p_vGlobalVector.is_bound(), -rotated.x, rotated.z, rotated.y);
    if(switched.is_bound())
        switched -= l_vShift;
    switched.rotate_z(-l_dModuleRotation);
    return switched;
}

GlobalNodeId Barrel::FindGlobalNodeId(const TVector& p_vGlobalVector, unsigned p_uLayerIdShift) const
{
    const TAngle l_dFaceRotationAngle = 2 * M_PI / GetNbFace();
    TAngle l_dPhi = atan2(p_vGlobalVector.y, p_vGlobalVector.x) + (l_dFaceRotationAngle - M_PI)/ 2.0;
    if(l_dPhi < 0)
        l_dPhi += 2.0 * M_PI;

    const unsigned l_uSuperModuleId = (unsigned) floor(l_dPhi / l_dFaceRotationAngle);
    const TAngle l_dRotationAngle = l_dFaceRotationAngle * l_uSuperModuleId;
    TVector l_vRotatedVector = p_vGlobalVector;
    l_vRotatedVector.rotate_z(-l_dRotationAngle);
    const TVector l_vLocalVector(-l_vRotatedVector.x, l_vRotatedVector.z, l_vRotatedVector.y);

    unsigned l_uLayerId = GetNbLayers();
    for(unsigned n = 0; n < GetNbLayers(); ++n)
    {
        if(l_vLocalVector.z >= GetLayerRadius(n) && l_vLocalVector.z - GetLayerRadius(n) <= GetLayerThickness(n))
        {
            l_uLayerId = n;
            break;
        }
    }
    assert(l_uLayerId != GetNbLayers());

    const ModulesDistribution::DescriptorCollection& l_vDescriptors = GetModulesDistribution().SelectLayer(l_uLayerId);
    unsigned l_uModuleId = l_vDescriptors.size();
    const Module& l_oModulePrototype = GetModulePrototype(l_uLayerId);
    const TCuboidSize& l_vModuleSize = l_oModulePrototype.GetModuleSize();
    for(unsigned n = 0; n < l_vDescriptors.size(); ++n)
    {
        const ModuleDescriptor& l_oDescriptor = l_vDescriptors[n];
        const TAngle l_dModuleRotation = l_oDescriptor.m_adModuleRotation.phi;
        const TPosition& l_vModulePosition = l_oDescriptor.m_adModulePosition;
        TVector l_vRotatedVector = l_vLocalVector;
        l_vRotatedVector -= l_vModulePosition;
        l_vRotatedVector.rotate_z(-l_dModuleRotation);

        if( l_vRotatedVector.x >= -l_vModuleSize.x / 2.0
                && l_vRotatedVector.x <=  l_vModuleSize.x / 2.0
                && l_vRotatedVector.y >= -l_vModuleSize.y / 2.0
                && l_vRotatedVector.y <=  l_vModuleSize.y / 2.0 )
        {
            l_uModuleId = n;
            break;
        }
    }
    assert(l_uModuleId != l_vDescriptors.size());
    return GlobalNodeId(0, 0, l_uLayerId + p_uLayerIdShift, l_uSuperModuleId, l_uModuleId);
}


TVector Barrel::TransformVectorToGlobal(const TVector& p_vLocalVector, GlobalNodeId p_uSensorId,
                                        unsigned p_uLayerId) const
{
    const ModulesDistribution::DescriptorCollection& l_vModuleDescriptors =
        GetModulesDistribution().SelectLayer(p_uLayerId);
    const unsigned l_uSuperModuleId = p_uSensorId.GetSuperModuleId();
    const unsigned l_uModuleId = p_uSensorId.GetModuleId();

    assert(l_uModuleId < l_vModuleDescriptors.size());
    const ModuleDescriptor& l_oModuleDescriptor = l_vModuleDescriptors[l_uModuleId];
    const LayerDescriptor& l_oLayerDescriptor = GetLayerDescriptor(p_uLayerId);
    const TAngle l_dModuleRotation = l_oModuleDescriptor.m_adModuleRotation.phi;
    const TAngle l_dSuperModuleRotation = GetFaceRotationAngle(l_uSuperModuleId);
    const TPosition& l_vModulePosition = l_oModuleDescriptor.m_adModulePosition;
    const TPosition l_vShift(l_vModulePosition.x, l_vModulePosition.y, l_oLayerDescriptor.SensitiveSurfaceRadius);

    TVector rotated = p_vLocalVector;
    rotated.rotate_z(l_dModuleRotation);
    if(rotated.is_bound())
        rotated += l_vShift;
    TVector switched(p_vLocalVector.is_bound(), -rotated.x, rotated.z, rotated.y);
    switched.rotate_z(l_dSuperModuleRotation);
    return switched;
}

const Barrel::LayerDescriptor& Barrel::GetLayerDescriptor(unsigned p_uLayerId) const
{
    LayerDescriptorCollection::const_iterator iter = m_oLayerDescriptors.find(p_uLayerId);
    assert(iter != m_oLayerDescriptors.end());
    return *iter->second;
}

void Barrel::SetLayerDescriptor(unsigned p_uLayerId, TLength p_dFirstSurfaceShift, TLength p_dSensitiveSurfaceShift,
                                TLength p_dThickness, TLength p_dSensitiveSurfaceThickness)
{
    assert(p_uLayerId <= GetNbLayers());
    const TLength l_dSuperModuleShift = GetInnerRadiusMin();
    P<LayerDescriptor> l_pLayerDescriptor = new LayerDescriptor(p_dFirstSurfaceShift + l_dSuperModuleShift,
            p_dSensitiveSurfaceShift + l_dSuperModuleShift,
            p_dThickness, p_dSensitiveSurfaceThickness);
    m_oLayerDescriptors[p_uLayerId] = l_pLayerDescriptor;
}

void Barrel::SetSensitiveDetectorNames(const string& p_sPrefix)
{
    m_SensitiveDetectorNames.clear();
    for(unsigned layer=0; layer<GetNbLayers(); layer++)
    {
        stringstream l_sNewPrefix;
        l_sNewPrefix << p_sPrefix << "_layer_" << layer;
        GetModulePrototype(layer).SetSensitiveDetectorName(l_sNewPrefix.str());
        m_SensitiveDetectorNames.insert(GetModulePrototype(layer).GetSensitiveDetectorName());
    }
}

const set<string>& Barrel::GetSensitiveDetectorNames() const
{
    return m_SensitiveDetectorNames;
}

