/*! \file XUV_Endcap.cc
    \brief An implementation of Silc::XUV_Endcap class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "XUV_Endcap.hh"

using namespace Silc;

// Constants.
static const Endcap::ModuleDescriptor::RotationIndicator MODULE_ROTATION =
    Endcap::ModuleDescriptor::OrtogonalRotation;
static const unsigned NUMBER_OF_JOINED_LAYERS = 1;
const Endcap::EndcapType XUV_Endcap::ENDCAP_TYPE = "XUV";

P<Endcap> XUV_Endcap::MakeInstance()
{
    return P<Endcap> ( new XUV_Endcap() );
}

const Endcap::EndcapType& XUV_Endcap::GetType() const
{
    return ENDCAP_TYPE;
}

TCuboidSize XUV_Endcap::GetRotatedModuleSize(unsigned p_uZoneId)
{
    typedef TLength TCuboidSize::* coordinate_pointer;
    static const coordinate_pointer l_pInitialPointers[] = { &TCuboidSize::x, &TCuboidSize::y, &TCuboidSize::z };
    static const coordinate_pointer l_pRotatedPointers[] = { &TCuboidSize::y, &TCuboidSize::x, &TCuboidSize::z };
    const coordinate_pointer* l_pCurrentPointers = MODULE_ROTATION == ModuleDescriptor::ZeroRotation ?
            l_pInitialPointers : l_pRotatedPointers;

    const TCuboidSize& l_oModuleSize = GetModulePrototype(p_uZoneId).GetModuleSize();
    TCuboidSize l_oRotatedModuleSize;
    for(unsigned n = 0; n < l_oModuleSize.dimension(); ++n)
        l_oRotatedModuleSize.*l_pInitialPointers[n] = l_oModuleSize.*l_pCurrentPointers[n];

    return l_oRotatedModuleSize;
}

unsigned XUV_Endcap::CalculateNumberOfLadderSteps()
{
    const TLength l_dActiveZoneLength = GetActiveSuperModuleShift()
                                        + sqrt( sqr( GetActiveOuterRadius() ) - sqr( GetActiveInnerRadius() ) );
    return floor( l_dActiveZoneLength / GetRotatedModuleSize().x );
}

void XUV_Endcap::FillZone(BuildState& p_oState, compare_function p_fCompare, round_function p_fRound)
{
    const TLength l_dZoneRadius = GetZoneRadius(p_oState.ZoneId);
    const TCuboidSize l_oModuleSize = GetRotatedModuleSize(p_oState.ZoneId);
    const TLength l_dLeftZoneY = sqrt( max( sqr(l_dZoneRadius) - sqr(p_oState.LeftStepX), 0.0 ) );
    const TLength l_dRightZoneY = sqrt( max( sqr(l_dZoneRadius) - sqr(p_oState.RightStepX), 0.0 ) );
    const TLength l_dZoneY = p_fCompare(l_dLeftZoneY, l_dRightZoneY);
    const double l_dAvaibleModules = max( (l_dZoneY - p_oState.AvailableY) / l_oModuleSize.y, 0.0 );
    const unsigned l_uNumberOfModules = p_fRound(l_dAvaibleModules);

    for(unsigned n = 0; n < l_uNumberOfModules; ++n)
    {
        TPlanePosition l_adModulePosition;
        l_adModulePosition.x = p_oState.LeftStepX + l_oModuleSize.x / 2;
        l_adModulePosition.y = p_oState.AvailableY + (n + 0.5) * l_oModuleSize.y;
        const ModuleDescriptor l_oDescriptor(0, p_oState.ZoneId, MODULE_ROTATION, l_adModulePosition);
        GetModulesDistribution().Add(l_oDescriptor);
    }

    p_oState.AvailableY += l_uNumberOfModules * l_oModuleSize.y;
}

void XUV_Endcap::FillSupermodule(unsigned p_uNumberOfLadderSteps)
{
    const TLength l_dModuleSizeX = GetRotatedModuleSize().x;

    BuildState l_oState;

    for(l_oState.StepId = 0; l_oState.StepId < p_uNumberOfLadderSteps; ++l_oState.StepId)
    {
        l_oState.LeftStepX = - GetActiveSuperModuleShift() + l_dModuleSizeX * l_oState.StepId;
        l_oState.RightStepX = l_oState.LeftStepX + l_dModuleSizeX;
        l_oState.AvailableY = GetActiveInnerRadius();

        for(l_oState.ZoneId = 0; l_oState.ZoneId < GetNumberOfZones() - 1; ++l_oState.ZoneId)
            FillZone(l_oState, &max<TLength>, &ceil);
        FillZone(l_oState, &min<TLength>, &floor);
    }
}

void XUV_Endcap::Assemble()
{
    assert_ex(GetNumberOfZones(), std_ext::invalid_operation_exception);
    Endcap::Assemble();
    const unsigned l_uNumberofLadderSteps = CalculateNumberOfLadderSteps();
    FillSupermodule(l_uNumberofLadderSteps);
}

unsigned XUV_Endcap::GetNumberOfJoinedLayers() const
{
    return NUMBER_OF_JOINED_LAYERS;
}
