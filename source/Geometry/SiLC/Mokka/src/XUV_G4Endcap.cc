/*! \file XUV_G4Endcap.cc
    \brief An implementation of Silc::XUV_G4Endcap class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "XUV_G4Endcap.hh"

using namespace std;
using namespace Silc;

// Constants.
static const TLength ROHACEL_THICKNESS = 3.9 * mm;
static const TLength GRAPHITE_THICKNESS = 0.8 * mm;   /// all 5.5
static const TLength REINFORCEMENT_THICKNESS = 10.0 * mm; /// - 2 maximum

static const TLength SUBTRACTION_OVERSTEP = 0.1 * mm;
static const TLength SUPPORT_THICKNESS = /*REINFORCEMENT_THICKNESS +*/ ROHACEL_THICKNESS + GRAPHITE_THICKNESS * 2;

static const char* ROHACEL_MATERIAL_NAME = "g10";
static const char* GRAPHITE_MATERIAL_NAME = "graphite";
static const char* SUPPORT_DISK_NAME_PREFIX = "PlaneDisc_";
static const char* LEFT_SUBTRACTION_SHAPE_NAME_PREFIX = "PlaneDiscLeftMask_";
static const char* BOTTOM_SUBTRACTION_SHAPE_NAME_PREFIX = "PlaneDiscBottomMask_";
static const char* RIGHT_SUPPORT_SHAPE_NAME_PREFIX = "PlaneDiscRight_";
static const char* SUPPORT_SHAPE_NAME_PREFIX = "SupportPlaneDisc_";
static const char* SUPPORT_VOLUME_NAME_PREFIX = "SupportVolume_";
static const char* REINFORCEMENT_DISC_NAME_PREFIX = "ReinforcerDisc_";
static const char* REINFORCEMENT_SMALL_EDGE_NAME_PREFIX = "ReinforcerSmallEdge_";
static const char* REINFORCEMENT_WITH_SMALL_EDGE_NAME_PREFIX = "ReinforcementWithSmallEdge_";
static const char* REINFORCEMENT_LONG_EDGE_NAME_PREFIX = "ReinforcerLongEdge_";
static const char* REINFORCEMENT_SHAPE_NAME_PREFIX = "ReinforcementShape_";
static const char* REINFORCEMENT_VOLUME_NAME_PREFIX = "ReinforcementVolume_";

static const G4Colour DEFAULT_VISUALISATION_COLOUR(0.0, 0.0, 0.0);
static const G4Colour GRAPHITE_VISUALISATION_COLOUR(0.0, 0.0, 0.9);

P<G4Endcap> XUV_G4Endcap::MakeInstance()
{
    return P<G4Endcap> ( new XUV_G4Endcap() );
}

void XUV_G4Endcap::Assemble()
{
    XUV_Endcap::Assemble();
    G4Endcap::Assemble();
}

void XUV_G4Endcap::MakeSuperModuleAssembly()
{
    G4Endcap::MakeSuperModuleAssembly();

    G4ThreeVector l_oActiveSurfaceShift;
    GetSuperModuleAssembly()->AddPlacedAssembly ( GetActiveSurfaceAssembly(0), l_oActiveSurfaceShift, nullptr );
    // ??
    G4ThreeVector l_oAssemblyShift(0.0, 0.0, -(GetModulePrototype(0).GetModuleSize().z + SUPPORT_THICKNESS));
    GetSuperModuleAssembly()->AddPlacedAssembly ( GetSupportAssembly(), l_oAssemblyShift, nullptr );
}

void XUV_G4Endcap::MakeSupportAssembly()
{
    G4Endcap::MakeSupportAssembly();
    MakeReinforcementLayer();
    MakeCarbonLayers();
    MakeRohacelLayer();
}

void XUV_G4Endcap::MakeRohacelLayer()
{
    G4LogicalVolume* l_pRohacelLayerVolume = BuildFlatLayerVolume(ROHACEL_MATERIAL_NAME, ROHACEL_THICKNESS);
    G4ThreeVector l_oRohacelLayerShift(0, 0, REINFORCEMENT_THICKNESS + GRAPHITE_THICKNESS + ROHACEL_THICKNESS / 2);
    GetSupportAssembly()->AddPlacedVolume(l_pRohacelLayerVolume, l_oRohacelLayerShift, nullptr);
}

void XUV_G4Endcap::MakeCarbonLayers()
{
    G4LogicalVolume* l_pCarbonLayerVolume = BuildFlatLayerVolume(GRAPHITE_MATERIAL_NAME, GRAPHITE_THICKNESS);
    G4ThreeVector l_oCarbonTopShift(0, 0, REINFORCEMENT_THICKNESS + ROHACEL_THICKNESS + GRAPHITE_THICKNESS * 1.5);
    GetSupportAssembly()->AddPlacedVolume(l_pCarbonLayerVolume, l_oCarbonTopShift, nullptr);
    G4ThreeVector l_oCarbonBottomShift(0, 0, REINFORCEMENT_THICKNESS + GRAPHITE_THICKNESS / 2);
    GetSupportAssembly()->AddPlacedVolume(l_pCarbonLayerVolume, l_oCarbonBottomShift, nullptr);
}

void XUV_G4Endcap::MakeReinforcementLayer()
{
    G4LogicalVolume* l_pReinforcementVolume = BuildBendedLayerVolume(GRAPHITE_MATERIAL_NAME, REINFORCEMENT_THICKNESS);
    G4ThreeVector l_oReinforcementShift(0, 0, REINFORCEMENT_THICKNESS / 2);
    GetSupportAssembly()->AddPlacedVolume(l_pReinforcementVolume, l_oReinforcementShift, nullptr);
}

G4LogicalVolume* XUV_G4Endcap::BuildFlatLayerVolume(const string& p_sMaterial, TLength p_dThickness)
{
    G4VSolid* l_pSupportDiscShape = CreateSupportDiscShape(p_sMaterial, p_dThickness);
    G4VSolid* l_pSupportRightShape = ApplyLeftSubtraction(l_pSupportDiscShape, p_sMaterial, p_dThickness);
    G4VSolid* l_pSupportShape = ApplyBottomSubtraction(l_pSupportRightShape, p_sMaterial, p_dThickness);
    return CreateLogicalVolume(l_pSupportShape, p_sMaterial, SUPPORT_VOLUME_NAME_PREFIX);
}

G4LogicalVolume* XUV_G4Endcap::BuildBendedLayerVolume(string p_sMaterial, TLength p_dThickness)
{
    G4VSolid* l_pReinforcementDisc = CreateReinforcementDisc(p_sMaterial, p_dThickness);
    G4VSolid* l_pDiscWithSmallEdge = JoinReinforcementShortEdge(l_pReinforcementDisc, p_sMaterial, p_dThickness);
    G4VSolid* l_pReinforcementSolid = JoinReinforcementLongEdge(l_pDiscWithSmallEdge, p_sMaterial, p_dThickness);

    return CreateLogicalVolume(l_pReinforcementSolid, p_sMaterial, REINFORCEMENT_VOLUME_NAME_PREFIX);
}

G4VSolid* XUV_G4Endcap::CreateSupportDiscShape(const string& p_sMaterial, TLength p_dThickness)
{
    const TLength l_dOuterRadius = GetOuterRadius();
    return new G4Tubs(SUPPORT_DISK_NAME_PREFIX + p_sMaterial, 0, l_dOuterRadius, p_dThickness / 2, 0, M_PI);
}

G4VSolid* XUV_G4Endcap::CreateReinforcementDisc(const string& p_sMaterial, TLength p_dThickness)
{
    const string l_sDiscName = REINFORCEMENT_DISC_NAME_PREFIX + p_sMaterial;
    const TLength l_dOuterDiscRadius = GetOuterRadius();
    const TLength l_dInnerDiscRadius = l_dOuterDiscRadius - GetCircularSupportWidth();
    const TAngle l_dInitialAngle = asin(GetEffectiveInnerRadius() / l_dOuterDiscRadius);
    const TAngle l_dFinalAngle = M_PI / 2.0;

    return new G4Tubs(l_sDiscName, l_dInnerDiscRadius, l_dOuterDiscRadius, p_dThickness / 2.0,
                      l_dInitialAngle, l_dFinalAngle);
}

G4VSolid* XUV_G4Endcap::ApplyLeftSubtraction(G4VSolid* p_pShape, const string& p_sMaterial, TLength p_dThickness)
{
    const TLength l_dOuterRadius = GetOuterRadius();
    const TLength l_dInnerRadius = GetEffectiveInnerRadius();

    const TCuboidSize l_oSupportLeftHoleSize(l_dOuterRadius - l_dInnerRadius,
            l_dOuterRadius,
            p_dThickness + SUBTRACTION_OVERSTEP);

    G4VSolid* l_pSupportLeftHoleShape = G4Extensions::CreateG4Box(LEFT_SUBTRACTION_SHAPE_NAME_PREFIX + p_sMaterial,
                                        l_oSupportLeftHoleSize);

    const G4ThreeVector l_oSupportLeftHoleShift(-l_dInnerRadius - (l_dOuterRadius - l_dInnerRadius) / 2.0,
            l_dOuterRadius / 2.0,
            0.0);

    const G4Transform3D l_oSupportLeftHoleTransform(G4RotationMatrix(), l_oSupportLeftHoleShift);

    return new G4SubtractionSolid(RIGHT_SUPPORT_SHAPE_NAME_PREFIX + p_sMaterial, p_pShape, l_pSupportLeftHoleShape,
                                  l_oSupportLeftHoleTransform);
}

G4VSolid* XUV_G4Endcap::ApplyBottomSubtraction(G4VSolid* p_pShape, const string& p_sMaterial, TLength p_dThickness)
{
    const TLength l_dOuterRadius = GetOuterRadius();
    const TLength l_dInnerRadius = GetEffectiveInnerRadius();

    const TCuboidSize l_oSupportBottomHoleSize(l_dOuterRadius + l_dInnerRadius,
            l_dInnerRadius,
            p_dThickness + SUBTRACTION_OVERSTEP);

    G4VSolid* l_pSupportBottomHoleShape = G4Extensions::CreateG4Box(BOTTOM_SUBTRACTION_SHAPE_NAME_PREFIX + p_sMaterial,
                                          l_oSupportBottomHoleSize);


    const G4ThreeVector l_oSupportBottomHoleShift( (l_dOuterRadius + l_dInnerRadius) / 2.0 - l_dInnerRadius,
            l_dInnerRadius / 2.0,
            0.0 );

    const G4Transform3D l_oSupportBottomHoleTransform(G4RotationMatrix(), l_oSupportBottomHoleShift);

    return new G4SubtractionSolid(SUPPORT_SHAPE_NAME_PREFIX + p_sMaterial, p_pShape, l_pSupportBottomHoleShape,
                                  l_oSupportBottomHoleTransform);
}

G4VSolid* XUV_G4Endcap::JoinReinforcementShortEdge(G4VSolid* p_pShape, const string& p_sMaterial, TLength p_dThickness)
{
    const TLength l_dInnerRadius = GetEffectiveInnerRadius();
    const TLength l_dOuterRadius = GetOuterRadius();
    const TLength l_dSupportWidth = GetPlaneSupportWidth();
    const TLength l_dShortEdgeLength = sqrt(l_dOuterRadius*l_dOuterRadius - l_dInnerRadius*l_dInnerRadius) - l_dInnerRadius;

    const string l_sShortEdgeName = REINFORCEMENT_SMALL_EDGE_NAME_PREFIX + p_sMaterial;
    const TCuboidSize l_oShortEdgeSize(l_dSupportWidth, l_dShortEdgeLength, p_dThickness);
    G4VSolid* l_pShortEdgeSolid = G4Extensions::CreateG4Box(l_sShortEdgeName, l_oShortEdgeSize);

    const string l_sReinforcementWithShortEdgeName = REINFORCEMENT_WITH_SMALL_EDGE_NAME_PREFIX + p_sMaterial;
    const G4ThreeVector l_oShortEdgeShift(-l_dInnerRadius + l_dSupportWidth / 2.0,
                                          l_dInnerRadius + l_dShortEdgeLength / 2.0,
                                          0.0);
    G4RotationMatrix* l_pShortEdgeRotation = new G4RotationMatrix();
    return new G4UnionSolid(l_sReinforcementWithShortEdgeName, p_pShape, l_pShortEdgeSolid,
                            l_pShortEdgeRotation, l_oShortEdgeShift);
}

G4VSolid* XUV_G4Endcap::JoinReinforcementLongEdge(G4VSolid* p_pShape, const string& p_sMaterial, TLength p_dThickness)
{
    const TLength l_dInnerRadius = GetEffectiveInnerRadius();
    const TLength l_dOuterRadius = GetOuterRadius();
    const TLength l_dSupportWidth = GetPlaneSupportWidth();
    const TLength l_dLongEdgeLength = sqrt(sqr(l_dOuterRadius)-sqr(l_dInnerRadius)) + l_dInnerRadius;

    const string l_sLongEdgeName = REINFORCEMENT_LONG_EDGE_NAME_PREFIX + p_sMaterial;
    const TCuboidSize l_oLongEdgeSize(l_dLongEdgeLength, l_dSupportWidth, p_dThickness);
    G4VSolid* l_pLongEdgeSolid = G4Extensions::CreateG4Box(l_sLongEdgeName, l_oLongEdgeSize);

    const string l_sReinforcementSolidName = REINFORCEMENT_SHAPE_NAME_PREFIX + p_sMaterial;
    const G4ThreeVector l_oLongEdgeShift(-l_dInnerRadius + l_dLongEdgeLength / 2.0,
                                         l_dInnerRadius + l_dSupportWidth / 2.0,
                                         0.0);
    G4RotationMatrix* l_pLongEdgeRotation = new G4RotationMatrix();

    return new G4UnionSolid(l_sReinforcementSolidName, p_pShape, l_pLongEdgeSolid,
                            l_pLongEdgeRotation, l_oLongEdgeShift);
}

G4LogicalVolume* XUV_G4Endcap::CreateLogicalVolume(G4VSolid* p_pShape, const string& p_sMaterialName,
        const string& p_sPrefix)
{
    G4Material* l_pMaterial = CGAGeometryManager::GetMaterial(p_sMaterialName.c_str());
    G4LogicalVolume* l_pVolume = new G4LogicalVolume(p_pShape, l_pMaterial, p_sPrefix + p_sMaterialName);

    const G4Colour& l_oColour = p_sMaterialName == GRAPHITE_MATERIAL_NAME ? GRAPHITE_VISUALISATION_COLOUR
                                : DEFAULT_VISUALISATION_COLOUR;
    G4VisAttributes* l_pVisualisationAttributes = new G4VisAttributes(l_oColour);
    l_pVisualisationAttributes->SetForceWireframe(true);
    l_pVolume->SetVisAttributes(l_pVisualisationAttributes);

    return l_pVolume;
}
