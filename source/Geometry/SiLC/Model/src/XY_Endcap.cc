/*! \file XY_Endcap.cc
    \brief An implementation of Silc::XY_Endcap class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "XY_Endcap.hh"

using namespace Silc;

// Constants.
static const Endcap::ModuleDescriptor::RotationIndicator MODULE_ROTATIONS[] =
{ Endcap::ModuleDescriptor::ZeroRotation, Endcap::ModuleDescriptor::OrtogonalRotation };
static const Endcap::ModuleDescriptor::RotationIndicator FILL_ROTATIONS[] =
{ Endcap::ModuleDescriptor::OrtogonalRotation, Endcap::ModuleDescriptor::ZeroRotation };

static const unsigned NUMBER_OF_JOINED_LAYERS = 2;
const Endcap::EndcapType XY_Endcap::ENDCAP_TYPE = "XY";

P<Endcap> XY_Endcap::MakeInstance()
{
    return P<Endcap> ( new XY_Endcap() );
}

const Endcap::EndcapType& XY_Endcap::GetType() const
{
    return ENDCAP_TYPE;
}

TLength XY_Endcap::CalculateOctagonSideLength(TLength p_dInscribedCircleRadius)
{
    return 2.0 * ( sqrt(2.0) - 1.0 ) * p_dInscribedCircleRadius;
}

unsigned XY_Endcap::CalculateNumberOfLadderSteps(unsigned p_uLayerId)
{
    TLength l_dActiveZoneLength;
    if(FILL_ROTATIONS[p_uLayerId] == Endcap::ModuleDescriptor::ZeroRotation)
        l_dActiveZoneLength = GetActiveOuterRadius() + GetActiveSuperModuleShift();
    else
        l_dActiveZoneLength = GetActiveOuterRadius() - GetActiveInnerRadius();
    return floor( l_dActiveZoneLength / GetRotatedModuleProjection(p_uLayerId).x );
}

TCoordinate XY_Endcap::CalculateLimitForCircularBorder(TLength p_dRadius, TCoordinate p_dShift)
{
    return sqrt( max( sqr(p_dRadius) - sqr(p_dShift), 0.0 ) );
}

TCoordinate XY_Endcap::CalculateLimitForOctogonalBorder(TLength p_dRadius, TCoordinate p_dShift)
{
    TLength l_dSideLength = CalculateOctagonSideLength(p_dRadius);
    if(p_dShift < l_dSideLength / 2.0)
        return p_dRadius;
    return p_dRadius + l_dSideLength / 2.0 - p_dShift;
}

XY_Endcap::limit_calculator XY_Endcap::GetLimitCalculator(unsigned p_uZoneId)
{
    if(p_uZoneId < GetNumberOfZones() - 1)
        return &CalculateLimitForCircularBorder;
    return &CalculateLimitForOctogonalBorder;
}

void XY_Endcap::CollectStepParameters(unsigned p_uLayerId, Filler& p_oFiller, TCoordinate p_dStepLeftShift, TCoordinate p_dStepRightShift)
{
    const unsigned l_uNumberOfZones = GetNumberOfZones();
    for(unsigned l_uZoneId = 0; l_uZoneId < l_uNumberOfZones; ++l_uZoneId)
    {
        const TLength l_dModuleLength = GetRotatedModuleProjection(p_uLayerId, l_uZoneId).y;
        const TLength l_dZoneRadius = GetZoneRadius(l_uZoneId);
        const limit_calculator l_fLimitCalculator = GetLimitCalculator(l_uZoneId);
        const TCoordinate l_dZoneLeftLimit = l_fLimitCalculator(l_dZoneRadius, p_dStepLeftShift);
        const TCoordinate l_dZoneRightLimit = l_fLimitCalculator(l_dZoneRadius, p_dStepRightShift);
        p_oFiller.InitializeZone(l_uZoneId, l_dModuleLength, l_dZoneLeftLimit, l_dZoneRightLimit);
    }
}

void XY_Endcap::PlaceStepModules(unsigned p_uLayerId, const vector<unsigned> &p_vStepFilling, TLength p_dInitialPosition, TLength p_dStepLeftShift)
{
    const unsigned l_uNumberOfZones = GetNumberOfZones();
    const TLength l_dModuleWidth = GetRotatedModuleProjection(p_uLayerId).x;
    TLength l_dFirstAvaiblePosition = p_dInitialPosition;
    for(unsigned l_uZoneId = 0; l_uZoneId < l_uNumberOfZones; ++l_uZoneId)
    {
        const unsigned l_uNumberOfModules = p_vStepFilling[l_uZoneId];
        const TLength l_dModuleLength = GetRotatedModuleProjection(p_uLayerId, l_uZoneId).y;
        for(unsigned l_uModuleId = 0; l_uModuleId < l_uNumberOfModules; ++l_uModuleId)
        {
            TPlanePosition l_vRotatedModulePosition;
            l_vRotatedModulePosition.x = p_dStepLeftShift + l_dModuleWidth / 2;
            l_vRotatedModulePosition.y = l_dFirstAvaiblePosition + (l_uModuleId + 0.5) * l_dModuleLength;
            TPlanePosition l_vModulePosition = ApplyRotation(l_vRotatedModulePosition, FILL_ROTATIONS[p_uLayerId] );
            const ModuleDescriptor l_oDescriptor(p_uLayerId, l_uZoneId, MODULE_ROTATIONS[p_uLayerId], l_vModulePosition);
            GetModulesDistribution().Add(l_oDescriptor);
        }
        l_dFirstAvaiblePosition += l_uNumberOfModules * l_dModuleLength;
    }
}

void XY_Endcap::FillSupermodule(unsigned p_uLayerId, unsigned p_uNumberOfLadderSteps)
{
    const unsigned l_uNumberOfZones = GetNumberOfZones();
    const TPlanePosition l_dInitialPosition = GetInitialPosition(p_uLayerId);
    const TLength l_dModuleWidth = GetRotatedModuleProjection(p_uLayerId).x;

    for(unsigned l_uStepId = 0; l_uStepId < p_uNumberOfLadderSteps; ++ l_uStepId)
    {
        const TLength l_dStepLeftShift = l_dInitialPosition.x + l_dModuleWidth * l_uStepId;
        const TLength l_dStepRightShift = l_dStepLeftShift + l_dModuleWidth;
        Filler l_oFiller(l_uNumberOfZones, l_dInitialPosition.y);
        CollectStepParameters(p_uLayerId, l_oFiller, l_dStepLeftShift, l_dStepRightShift);
        const vector<unsigned>& l_vStepFilling = l_oFiller.CalculateLadderStepFilling();
        PlaceStepModules(p_uLayerId, l_vStepFilling, l_dInitialPosition.y, l_dStepLeftShift);
    }
}

TPlanePosition XY_Endcap::GetInitialPosition(unsigned p_uLayerId)
{
    const TPlanePosition l_vPosition(-GetActiveSuperModuleShift(), GetActiveInnerRadius());
    return ApplyRotation(l_vPosition, FILL_ROTATIONS[p_uLayerId] );
}

TPlaneDimension XY_Endcap::GetRotatedModuleProjection(unsigned p_uLayerId, unsigned p_uZoneId)
{
    const TCuboidSize& l_vModuleSize = GetModulePrototype(p_uZoneId).GetModuleSize();
    const TPlanePosition l_vModuleProjection(l_vModuleSize.x, l_vModuleSize.y);
    const TPlanePosition l_vSelfRotatedProjection = ApplyRotation(l_vModuleProjection, MODULE_ROTATIONS[p_uLayerId] );
    const TPlanePosition l_vRotatedProjection = ApplyRotation(l_vSelfRotatedProjection, FILL_ROTATIONS[p_uLayerId] );
    return TPlaneDimension(l_vRotatedProjection.x, l_vRotatedProjection.y);
}

TPlanePosition XY_Endcap::ApplyRotation(const TPlanePosition& p_vPosition,
ModuleDescriptor::RotationIndicator p_dRotationIndicator)
{
    typedef TCoordinate TPlanePosition::* coordinate_pointer;
    static const coordinate_pointer l_pInitialPointers[] = { &TPlanePosition::x, &TPlanePosition::y };
    static const coordinate_pointer l_pRotatedPointers[] = { &TPlanePosition::y, &TPlanePosition::x };
    const coordinate_pointer* l_pCurrentPointers = p_dRotationIndicator == ModuleDescriptor::ZeroRotation ?
    l_pInitialPointers : l_pRotatedPointers;

    TPlanePosition l_vRotatedPosition;
    for(unsigned n = 0; n < p_vPosition.dimension(); ++n)
        l_vRotatedPosition.*l_pInitialPointers[n] = p_vPosition.*l_pCurrentPointers[n];

    return l_vRotatedPosition;
}

void XY_Endcap::Assemble()
{
    assert_ex(GetNumberOfZones(), std_ext::invalid_operation_exception);
    Endcap::Assemble();
    for(unsigned l_uLayerId = 0; l_uLayerId < GetNumberOfJoinedLayers(); ++l_uLayerId)
    {
        const unsigned l_uNumberOfLadderSteps = CalculateNumberOfLadderSteps(l_uLayerId);
        FillSupermodule(l_uLayerId, l_uNumberOfLadderSteps);
    }
}

unsigned XY_Endcap::GetNumberOfJoinedLayers() const
{
    return NUMBER_OF_JOINED_LAYERS;
}
