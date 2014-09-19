/*! \file XY_G4Endcap.cc
    \brief An implementation of Silc::XY_G4Endcap class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "XY_G4Endcap.hh"

using namespace std;
using namespace Silc;

// Constants.
static const TLength ROHACEL_THICKNESS = 3.0 * mm;
static const TLength GRAPHITE_THICKNESS = 1.5 * mm;
static const TLength REINFORCEMENT_THICKNESS = 2 * 10.0 * mm;

static const TLength SUBTRACTION_OVERSTEP = 0.1 * mm;
//static const TLength SUPPORT_THICKNESS = REINFORCEMENT_THICKNESS + ROHACEL_THICKNESS + GRAPHITE_THICKNESS * 2;
static const TLength SUPPORT_THICKNESS = ROHACEL_THICKNESS + GRAPHITE_THICKNESS * 2;

static const TLength LAYER_SPACE = 25.0 * mm;       ///< A distance between to consecutive layers.

static const unsigned NUMBER_OF_TRAPEZOID_VERTICES = 8;

static const char* ROHACEL_MATERIAL_NAME = "g10";
static const char* GRAPHITE_MATERIAL_NAME = "graphite";
static const char* SUPPORT_TRAPEZOID_NAME_PREFIX = "SupportTrapezoid_";
static const char* SUPPORT_CUBOID_NAME_PREFIX = "SupportCuboid_";
//static const char* LEFT_SUBTRACTION_SHAPE_NAME_PREFIX = "PlaneDiscLeftMask_";
//static const char* BOTTOM_SUBTRACTION_SHAPE_NAME_PREFIX = "PlaneDiscBottomMask_";
//static const char* RIGHT_SUPPORT_SHAPE_NAME_PREFIX = "PlaneDiscRight_";
static const char* SUPPORT_SHAPE_NAME_PREFIX = "SupportPlaneDisc_";
static const char* SUPPORT_VOLUME_NAME_PREFIX = "SupportVolume_";
//static const char* REINFORCEMENT_DISC_NAME_PREFIX = "ReinforcerDisc_";
//static const char* REINFORCEMENT_SMALL_EDGE_NAME_PREFIX = "ReinforcerSmallEdge_";
//static const char* REINFORCEMENT_WITH_SMALL_EDGE_NAME_PREFIX = "ReinforcementWithSmallEdge_";
//static const char* REINFORCEMENT_LONG_EDGE_NAME_PREFIX = "ReinforcerLongEdge_";
//static const char* REINFORCEMENT_SHAPE_NAME_PREFIX = "ReinforcementShape_";
static const char* REINFORCEMENT_VOLUME_NAME_PREFIX = "ReinforcementVolume_";
static const string BOX_NAME_PREFIX = "Box_";
static const string UNION_NAME_PREFIX = "Union_";

static const G4Colour DEFAULT_VISUALISATION_COLOUR(0.0, 0.0, 0.0);
static const G4Colour GRAPHITE_VISUALISATION_COLOUR(0.0, 0.0, 0.9);

P<G4Endcap> XY_G4Endcap::MakeInstance()
{
    return P<G4Endcap> ( new XY_G4Endcap() );
}

XY_G4Endcap::~XY_G4Endcap()
{
}

void XY_G4Endcap::Assemble()
{
    XY_Endcap::Assemble();
    G4Endcap::Assemble();
}

void XY_G4Endcap::MakeSuperModuleAssembly()
{
    G4Endcap::MakeSuperModuleAssembly();

    G4ThreeVector l_oAssemblyShift(0.0, 0.0, 0.0/*GetModulePrototype(0).GetModuleSize().z*/);
    GetSuperModuleAssembly()->AddPlacedAssembly ( GetSupportAssembly(), l_oAssemblyShift, nullptr );

    for(unsigned l_uLayerId = 0; l_uLayerId < GetNumberOfJoinedLayers(); ++l_uLayerId)
    {
        G4ThreeVector l_oActiveSurfaceShift(0, 0, (-1./2.+l_uLayerId) * (SUPPORT_THICKNESS));
        GetSuperModuleAssembly()->AddPlacedAssembly ( GetActiveSurfaceAssembly(l_uLayerId), l_oActiveSurfaceShift, nullptr );
    }
}

void XY_G4Endcap::MakeSupportAssembly()
{
    G4Endcap::MakeSupportAssembly();
    SetupParameters();
    MakeReinforcementLayer();
    MakeCarbonLayers();
    MakeRohacelLayer();
}

void XY_G4Endcap::SetupParameters()
{
    m_dOctagonInscribedCircleRadius = GetOuterRadius();
    m_dTrapeziumShift = GetSuperModuleShift();
    m_dOctagonSide = CalculateOctagonSideLength( m_dOctagonInscribedCircleRadius );
    m_dTrapeziumHeight = m_dOctagonInscribedCircleRadius - m_dOctagonSide / 2;
    m_dTrapeziumLowerSide = m_dOctagonInscribedCircleRadius + m_dTrapeziumShift;
    m_dTrapeziumUpperSide = m_dOctagonSide / 2 + m_dTrapeziumShift;
    m_dRectangleShift = GetSuperModuleShift();
    m_dRectangleVerticalSide = m_dOctagonSide / 2 - GetEffectiveInnerRadius();
    m_dRectangleHorizontalSide = m_dOctagonInscribedCircleRadius + m_dRectangleShift;
}

void XY_G4Endcap::MakeRohacelLayer()
{
    G4AssemblyVolume* l_pRohacelLayerVolume = BuildFlatLayerVolume(ROHACEL_MATERIAL_NAME, ROHACEL_THICKNESS);
    G4ThreeVector l_oRohacelLayerShift(0, 0, 0);
    GetSupportAssembly()->AddPlacedAssembly(l_pRohacelLayerVolume, l_oRohacelLayerShift, nullptr);
}

void XY_G4Endcap::MakeCarbonLayers()
{
    G4AssemblyVolume* l_pCarbonLayerVolume = BuildFlatLayerVolume(GRAPHITE_MATERIAL_NAME, GRAPHITE_THICKNESS);
    G4ThreeVector l_oCarbonTopShift(0, 0, ROHACEL_THICKNESS/2. + GRAPHITE_THICKNESS/2.);
    GetSupportAssembly()->AddPlacedAssembly(l_pCarbonLayerVolume, l_oCarbonTopShift, nullptr);
    G4ThreeVector l_oCarbonBottomShift(0, 0, -ROHACEL_THICKNESS/2. - GRAPHITE_THICKNESS/2.);
    GetSupportAssembly()->AddPlacedAssembly(l_pCarbonLayerVolume, l_oCarbonBottomShift, nullptr);
}

void XY_G4Endcap::MakeReinforcementLayer()
{
    G4AssemblyVolume* l_pReinforcementVolume = BuildBendedLayerVolume(GRAPHITE_MATERIAL_NAME, REINFORCEMENT_THICKNESS);
    G4ThreeVector l_oReinforcementShift(0, 0, 0);
    GetSupportAssembly()->AddPlacedAssembly(l_pReinforcementVolume, l_oReinforcementShift, nullptr);
}

G4AssemblyVolume* XY_G4Endcap::BuildFlatLayerVolume(const string& p_sMaterial, TLength p_dThickness)
{
    G4VSolid* l_pLayerTrapeziumShape = CreateTrapezoidShape(p_sMaterial, p_dThickness);
    G4VSolid* l_pLayerShape = JoinSupportCuboidShape(l_pLayerTrapeziumShape, p_sMaterial, p_dThickness);
    G4LogicalVolume* l_pLayerVolume = CreateLogicalVolume(l_pLayerShape, p_sMaterial, SUPPORT_VOLUME_NAME_PREFIX);

    G4ThreeVector l_vLayerShift( (m_dTrapeziumLowerSide + m_dTrapeziumUpperSide) / 4 - m_dTrapeziumShift,
                                 (m_dTrapeziumHeight + m_dOctagonSide) / 2,
                                 0.);
    return new G4AssemblyVolume(l_pLayerVolume, l_vLayerShift, nullptr);
}

G4AssemblyVolume* XY_G4Endcap::BuildBendedLayerVolume(const string& p_sMaterial, TLength p_dThickness)
{
    const TLength l_dSupportWidth = GetPlaneSupportWidth();
    const TAngle l_dDiagonalBoxRotationAngle = M_PI_4;
    const double l_dAngleCosine = 1 / sqrt(2.0);
    //const double l_dAngleAbsSine = l_dAngleCosine;

    const TLength l_dLowerBoxXLength        = m_dOctagonInscribedCircleRadius + GetEffectiveInnerRadius();//m_dTrapeziumLowerSide;
    const TLength l_dLowerBoxYLength        = l_dSupportWidth;

    const TLength l_dLeftBoxXLength         = l_dLowerBoxYLength;
    const TLength l_dLeftBoxYLength         = m_dOctagonInscribedCircleRadius - GetEffectiveInnerRadius();

    const TLength l_dUpperBoxXLength        = m_dOctagonInscribedCircleRadius*tan(M_PI/8.) + GetEffectiveInnerRadius();
    const TLength l_dUpperBoxYLength        = l_dLowerBoxYLength;

    const TLength l_dRightBoxXLength        = l_dLowerBoxYLength;
    const TLength l_dRightBoxYLength        =  m_dOctagonInscribedCircleRadius*tan(M_PI/8.) - GetEffectiveInnerRadius();

    const TLength l_dDiagonalBoxXLength     = 2*m_dOctagonInscribedCircleRadius*tan(M_PI/8.);
    const TLength l_dDiagonalBoxYLength     = l_dLowerBoxYLength;

    const TCoordinate l_dLeftBoxXShift     = -l_dLowerBoxXLength/2. + l_dLeftBoxXLength/2.;
    const TCoordinate l_dLeftBoxYShift     = l_dLeftBoxYLength/2. - l_dLowerBoxYLength/2.;

    const TCoordinate l_dUpperBoxXShift     = -l_dLowerBoxXLength/2. + l_dUpperBoxXLength/2.;
    const TCoordinate l_dUpperBoxYShift     = -l_dLowerBoxYLength/2. + l_dLeftBoxYLength - l_dUpperBoxYLength/2.;

    const TCoordinate l_dRightBoxXShift     = l_dLowerBoxXLength/2. - l_dRightBoxXLength/2.;
    const TCoordinate l_dRightBoxYShift     = l_dRightBoxYLength/2. - l_dLowerBoxYLength/2.;

    TCoordinate l_dDiagonalBoxXShift  = l_dLowerBoxXLength/2. - l_dDiagonalBoxXLength * l_dAngleCosine / 2.;
    l_dDiagonalBoxXShift -= l_dAngleCosine * l_dDiagonalBoxYLength /2.;
    TCoordinate l_dDiagonalBoxYShift  = l_dRightBoxYLength + l_dDiagonalBoxXLength * l_dAngleCosine /2. - l_dLowerBoxYLength/2.;
    l_dDiagonalBoxYShift -= l_dAngleCosine * l_dDiagonalBoxYLength /2.;

    G4VSolid* l_pLowerBoxShape     = JoinBox(nullptr, p_sMaterial, p_dThickness,
                                     l_dLowerBoxXLength, l_dLowerBoxYLength);
    G4VSolid* l_pPlusLeftBoxShape  = JoinBox(l_pLowerBoxShape, p_sMaterial, p_dThickness,
                                     l_dLeftBoxXLength, l_dLeftBoxYLength,
                                     l_dLeftBoxXShift, l_dLeftBoxYShift);
    G4VSolid* l_pPlusRightBoxShape = JoinBox(l_pPlusLeftBoxShape, p_sMaterial, p_dThickness,
                                     l_dRightBoxXLength, l_dRightBoxYLength,
                                     l_dRightBoxXShift, l_dRightBoxYShift);
    G4VSolid* l_pPlusUpperBoxShape = JoinBox(l_pPlusRightBoxShape, p_sMaterial, p_dThickness,
                                     l_dUpperBoxXLength, l_dUpperBoxYLength,
                                     l_dUpperBoxXShift, l_dUpperBoxYShift);
    G4VSolid* l_pLayerShape        = JoinBox(l_pPlusUpperBoxShape, p_sMaterial, p_dThickness,
                                     l_dDiagonalBoxXLength, l_dDiagonalBoxYLength,
                                     l_dDiagonalBoxXShift, l_dDiagonalBoxYShift, l_dDiagonalBoxRotationAngle);

    const TLength l_dLowerBoxXShift        = l_dLowerBoxXLength/2. - GetEffectiveInnerRadius();
    const TLength l_dLowerBoxYShift        = GetEffectiveInnerRadius() + l_dLowerBoxYLength / 2.;

    G4LogicalVolume* l_pLayerVolume = CreateLogicalVolume(l_pLayerShape, p_sMaterial, REINFORCEMENT_VOLUME_NAME_PREFIX);
    G4ThreeVector l_vLayerShift( l_dLowerBoxXShift, l_dLowerBoxYShift, 0.);
    return new G4AssemblyVolume(l_pLayerVolume, l_vLayerShift, nullptr);
}

G4VSolid* XY_G4Endcap::CreateTrapezoidShape(const string& p_sMaterial, TLength p_dThickness)
{
    const string l_sTrapezoidName = SUPPORT_TRAPEZOID_NAME_PREFIX + p_sMaterial;
    G4ThreeVector l_adTrapezoidVertices[NUMBER_OF_TRAPEZOID_VERTICES];
    const TLength l_dTrapezoidHalfThickness = p_dThickness / 2;
    const TLength l_dTrapeziumHalfHeight = m_dTrapeziumHeight / 2;
    const TCoordinate l_dTrapeziumLeftX = - (m_dTrapeziumLowerSide + m_dTrapeziumUpperSide) / 4;

    l_adTrapezoidVertices[0].setX( l_dTrapeziumLeftX );
    l_adTrapezoidVertices[0].setY( l_dTrapeziumHalfHeight );
    l_adTrapezoidVertices[1].setX( l_dTrapeziumLeftX + m_dTrapeziumUpperSide );
    l_adTrapezoidVertices[1].setY( l_dTrapeziumHalfHeight );
    l_adTrapezoidVertices[2].setX( l_dTrapeziumLeftX );
    l_adTrapezoidVertices[2].setY( -l_dTrapeziumHalfHeight );
    l_adTrapezoidVertices[3].setX( l_dTrapeziumLeftX + m_dTrapeziumLowerSide );
    l_adTrapezoidVertices[3].setY( -l_dTrapeziumHalfHeight );

    const unsigned l_uNumberOfTrapezoidVerticesPerSurface = NUMBER_OF_TRAPEZOID_VERTICES / 2;
    for(unsigned l_uCurrentVertex = 0; l_uCurrentVertex < l_uNumberOfTrapezoidVerticesPerSurface; ++l_uCurrentVertex)
        l_adTrapezoidVertices[l_uCurrentVertex].setZ(-l_dTrapezoidHalfThickness);
    for(unsigned l_uCurrentVertex = l_uNumberOfTrapezoidVerticesPerSurface;
            l_uCurrentVertex < NUMBER_OF_TRAPEZOID_VERTICES; ++l_uCurrentVertex)
    {
        const unsigned l_uUpperVertexId = l_uCurrentVertex - l_uNumberOfTrapezoidVerticesPerSurface;
        const G4ThreeVector& l_vUpperVertex = l_adTrapezoidVertices[l_uUpperVertexId];
        l_adTrapezoidVertices[l_uCurrentVertex].setX(  l_vUpperVertex.x() );
        l_adTrapezoidVertices[l_uCurrentVertex].setY(  l_vUpperVertex.y() );
        l_adTrapezoidVertices[l_uCurrentVertex].setZ( -l_vUpperVertex.z() );
    }

    return new G4Trap(l_sTrapezoidName, l_adTrapezoidVertices);
}

G4VSolid* XY_G4Endcap::JoinSupportCuboidShape(G4VSolid* p_pShape, const string& p_sMaterial, TLength p_dThickness)
{
    const string l_sCuboidName = SUPPORT_CUBOID_NAME_PREFIX + p_sMaterial;
    const string l_sUnionName = SUPPORT_SHAPE_NAME_PREFIX + p_sMaterial;
    const TCuboidSize l_vCuboidSize( m_dRectangleHorizontalSide,
                                     m_dRectangleVerticalSide,
                                     p_dThickness);
    G4VSolid* l_pCuboidShape = G4Extensions::CreateG4Box(l_sCuboidName, l_vCuboidSize);

    G4ThreeVector l_vCuboidShift((m_dTrapeziumLowerSide - m_dTrapeziumUpperSide) / 4,
                                 -(l_vCuboidSize.y + m_dTrapeziumHeight) / 2, 0);
    G4Transform3D l_oCuboidTransform(G4RotationMatrix(), l_vCuboidShift);
    return new G4UnionSolid(l_sUnionName, p_pShape, l_pCuboidShape, l_oCuboidTransform);
}

G4VSolid* XY_G4Endcap::JoinBox(G4VSolid* p_pShape, const string& p_sMaterial, TLength p_dThickness,
                               TLength p_dXLength, TLength p_dYLength,
                               TCoordinate p_dXShift, TCoordinate p_dYShift, TAngle p_dRotation)
{
    const string l_sBoxName = BOX_NAME_PREFIX + p_sMaterial;
    const TCuboidSize l_vBoxSize(p_dXLength, p_dYLength, p_dThickness);
    G4VSolid* l_pBoxShape = G4Extensions::CreateG4Box(l_sBoxName, l_vBoxSize);
    if(!p_pShape)
        return l_pBoxShape;

    const string l_sUnionName = BOX_NAME_PREFIX + UNION_NAME_PREFIX + p_sMaterial;
    G4ThreeVector l_vBoxShift(p_dXShift, p_dYShift, 0);
    G4RotationMatrix l_vBoxRotation(p_dRotation, 0, 0);
    G4Transform3D l_oBoxTransform(l_vBoxRotation, l_vBoxShift);
    return new G4UnionSolid(l_sUnionName, p_pShape, l_pBoxShape, l_oBoxTransform);
}

G4LogicalVolume* XY_G4Endcap::CreateLogicalVolume(G4VSolid* p_pShape, const string& p_sMaterialName,
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
