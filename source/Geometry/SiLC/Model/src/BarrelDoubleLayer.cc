/*! \file BarrelDoubleLayer.cc
    \brief An implementation of Silc::BarrelDoubleLayer class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "BarrelDoubleLayer.hh"

using namespace Silc;

const Barrel::BarrelType BarrelDoubleLayer::BARREL_TYPE = "DoubleLayer";

const Barrel::BarrelType& BarrelDoubleLayer::GetBarrelType() const
{
    return BARREL_TYPE;
}

P<Barrel> BarrelDoubleLayer::MakeInstance()
{
    return P<Barrel>(new BarrelDoubleLayer());
}

void BarrelDoubleLayer::Assemble()
{
    AdjustParameters();
    FillSuperModule();
    Barrel::Assemble();
}

TLength BarrelDoubleLayer::CalculateModuleZPosition(unsigned p_uLayerId)
{
    const TAngle l_dModuleFace = GetModuleDirection(p_uLayerId);
    const Module& l_oModule = GetModulePrototype(p_uLayerId);
    const TLength l_dModuleThickness = l_oModule.GetModuleSize().z;
    const TLength l_dChipThickness = l_oModule.GetChipArray().GetChipSize().z;
    const TLength l_dModuleSupportThickness = l_oModule.GetSupport().GetThickness();
    const TLength l_dSensorThickness = l_oModule.GetSensorArray().GetSensorPrototype().GetSensorSize().z;

    const TLength l_dBarrelSupportThickness = GetBarrelSupportThickness();

    const TLength l_dFirstSurfaceZPosition = p_uLayerId ? (l_dBarrelSupportThickness + l_dModuleThickness) : 0;

    const TLength l_dRelativeSensitiveShift = fcmp(l_dModuleFace, 0.0) ? l_dModuleSupportThickness : l_dChipThickness;
    const TLength l_dSensitiveShift = l_dFirstSurfaceZPosition + l_dRelativeSensitiveShift;

    SetLayerDescriptor(p_uLayerId, l_dFirstSurfaceZPosition, l_dSensitiveShift, l_dModuleThickness, l_dSensorThickness);
    const TLength l_dAdditionalShift = fcmp(l_dModuleFace, 0.0) ? 0 : l_dModuleThickness;
    const TLength l_dModuleZPosition = l_dFirstSurfaceZPosition + l_dAdditionalShift;
    return l_dModuleZPosition;
}

void BarrelDoubleLayer::FillLayer(unsigned p_uLayerId)
{
    TCuboidSize l_adEffectiveModuleSize = GetEffectiveModuleSize(p_uLayerId);
    TCuboidSize l_adSuperModuleSize = GetSuperModuleSize(p_uLayerId);
    TPlaneDistribution l_auNumberOfModule = GetSuperModuleNbTiles(p_uLayerId);
    TSolidAngle l_adModuleRotation;
    l_adModuleRotation.phi = GetModuleDirection(p_uLayerId);
    l_adModuleRotation.theta = GetModuleFace(p_uLayerId);
    l_adModuleRotation.psi = 0;

    BuildState l_oState;
    l_oState.LayerId = 0;
    l_oState.IdX = 0;
    l_oState.IdY = 0;

    l_oState.PosX = -l_adSuperModuleSize.x/2.-l_adEffectiveModuleSize.x/2.;
    TLength l_dModuleZPosition = CalculateModuleZPosition(p_uLayerId);

    for(unsigned IdX=0; IdX<l_auNumberOfModule.x; IdX++)
    {
        l_oState.PosX += l_adEffectiveModuleSize.x;
        l_oState.PosY = -l_adSuperModuleSize.y/2.+l_adEffectiveModuleSize.y/2.;

        for(unsigned IdY=0; IdY<l_auNumberOfModule.y; IdY++)
        {
            TPosition l_adModulePosition;
            l_adModulePosition.x = l_oState.PosX;
            l_adModulePosition.y = l_oState.PosY;
            l_adModulePosition.z = l_dModuleZPosition;

            ModuleDescriptor l_oDescriptor(p_uLayerId, 0, l_adModuleRotation, l_adModulePosition);
            GetModulesDistribution().Add(l_oDescriptor);
            l_oState.PosY += l_adEffectiveModuleSize.y;
        }
    }
}

void BarrelDoubleLayer::FillSuperModule()
{
    for(unsigned layer=0; layer< GetNbLayers(); layer++)
        FillLayer(layer);
}

unsigned BarrelDoubleLayer::GetNbLayers() const
{
    return 2;
}

// TEMPORARY CHANGE
void BarrelDoubleLayer::AdjustParameters()
{
//    const TCuboidSize& effectiveModuleSize = GetEffectiveModuleSize(0)
//    const bool useInversedOrder = !fcmp(GetModuleDirection(0), GetModuleDirection(1))
//                                && fcmp(GetModuleDirection(0), 0.0)
//                                && fcmp(GetModuleDirection(1), M_PI_2);
//    const unsigned initialLayerId = useInversedOrder ? 0 : 1;
//    const unsigned secondaryLayerId = (initialLayerId + 1) % GetNbLayers();

    AdjustParametersForInitialLayer(1);
    AdjustParametersForSecondaryLayer(0, 1);
}

void BarrelDoubleLayer::AdjustParametersForInitialLayer(unsigned p_uLayerId)
{
    TCuboidSize l_adEffectiveModuleSize = GetEffectiveModuleSize(p_uLayerId);

    TPlaneDistribution l_auSuperModuleNbTiles;
    TCuboidSize l_adSuperModuleSize;

    typedef double (*round_function)(double);
    round_function l_fRadialRound = (!GetRadialAdjustment().compare("shrink")) ? &floor : &ceil;
    round_function l_fLongitudinalRound = (!GetLongitudinalAdjustment().compare("shrink")) ? &floor : &ceil;

    l_auSuperModuleNbTiles.x = 1;
    const TAngle l_dInitialFaceAngle = 2. * atan(l_adEffectiveModuleSize.x / (2. * GetInitialInnerRadius()));
    const unsigned l_uNumberOfFace = (unsigned)l_fRadialRound(2.*M_PI/l_dInitialFaceAngle);
    SetNbFace(l_uNumberOfFace);

    l_adSuperModuleSize.x = l_auSuperModuleNbTiles.x * l_adEffectiveModuleSize.x;

    const TAngle l_dFaceAngle = 2.*M_PI/l_uNumberOfFace;
    const TLength l_dRadiusMin = l_adEffectiveModuleSize.x/(2.*tan(l_dFaceAngle/2.));

    SetInnerRadiusMin(l_dRadiusMin);

    l_auSuperModuleNbTiles.y = (unsigned)l_fLongitudinalRound( GetLength()/l_adEffectiveModuleSize.y);

    assert(l_auSuperModuleNbTiles.y);

    l_adSuperModuleSize.y = l_auSuperModuleNbTiles.y * l_adEffectiveModuleSize.y;

    SetSuperModuleNbOfTiles(p_uLayerId, l_auSuperModuleNbTiles);
    SetSuperModuleSize(p_uLayerId, l_adSuperModuleSize);
}

void BarrelDoubleLayer::AdjustParametersForSecondaryLayer(unsigned p_uLayerId, unsigned p_uInitialLayerId)
{
    assert(p_uLayerId != p_uInitialLayerId);

    const TCuboidSize l_adEffectiveModuleSize = GetEffectiveModuleSize(p_uLayerId);

    TPlaneDistribution l_auSuperModuleNbTiles;
    TCuboidSize l_adSuperModuleSize;

    l_auSuperModuleNbTiles.x = (unsigned)ceil(GetSuperModuleSize(p_uInitialLayerId).x/l_adEffectiveModuleSize.x-0.5);
    assert(l_auSuperModuleNbTiles.x);
    l_adSuperModuleSize.x = l_auSuperModuleNbTiles.x * l_adEffectiveModuleSize.x;

    const TAngle l_dFaceAngle = 2.*M_PI/GetNbFace();
    const TLength l_dMinRadius = l_adEffectiveModuleSize.x / (2. * tan(l_dFaceAngle / 2.));
    if(l_dMinRadius>GetInnerRadiusMin()) SetInnerRadiusMin(l_dMinRadius);

    typedef double (*round_function)(double);
    round_function l_fLongitudinalRound = (!GetLongitudinalAdjustment().compare("shrink")) ? &floor : &ceil;

    l_auSuperModuleNbTiles.y = (unsigned)l_fLongitudinalRound( GetLength()/l_adEffectiveModuleSize.y);
    assert(l_auSuperModuleNbTiles.y);
    l_adSuperModuleSize.y = l_auSuperModuleNbTiles.y * l_adEffectiveModuleSize.y;

    SetSuperModuleNbOfTiles(p_uLayerId, l_auSuperModuleNbTiles);
    SetSuperModuleSize(p_uLayerId, l_adSuperModuleSize);
}
