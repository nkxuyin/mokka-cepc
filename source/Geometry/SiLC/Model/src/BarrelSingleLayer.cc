/*! \file BarrelSingleLayer.cc
    \brief An implementation of Silc::BarrelSingleLayer class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "BarrelSingleLayer.hh"

using namespace Silc;

const Barrel::BarrelType BarrelSingleLayer::BARREL_TYPE = "SingleLayer";

const Barrel::BarrelType& BarrelSingleLayer::GetBarrelType() const
{
    return BARREL_TYPE;
}

P<Barrel> BarrelSingleLayer::MakeInstance()
{
    return P<Barrel>(new BarrelSingleLayer());
}

void BarrelSingleLayer::Assemble()
{
    AdjustParameters();
    cerr << *this;
    FillSuperModule();
    Barrel::Assemble();
}

void BarrelSingleLayer::AdjustParameters()
{
//    if(GetShape()=="cylinder")
        AdjustParamCylinderLayer();
//    else
//        AdjustParamPolygonLayer();
}


void BarrelSingleLayer::AdjustParamPolygonLayer()
{
    typedef double (*round_function)(double);
    round_function l_fRadialRound = (!GetRadialAdjustment().compare("shrink")) ? &floor : &ceil;
    round_function l_fLongitudinalRound = (!GetRadialAdjustment().compare("shrink")) ? &floor : &ceil;

    TLength l_dMinRadius;
    TLength l_dMaxRadius;

    TPlaneDistribution l_auNbTiles;
    TPlaneDistribution l_auSuperModuleNbTiles;
    TCuboidSize l_adSuperModuleSize;
    TCuboidSize l_adEffectiveModuleSize;

    unsigned l_uNbFaces = GetNbFace();
    assert_ex(l_uNbFaces, Exception);

    TAngle l_dAngleOrign2Module = 2 * M_PI / l_uNbFaces;
    assert_ex(GetInnerRadiusMin() > 0, Exception);

    l_adEffectiveModuleSize = GetEffectiveModuleSize(0);

    // Size Of the Super Module
    l_adSuperModuleSize.x = 2*GetInnerRadiusMin()*tan(l_dAngleOrign2Module/2.);

    // ALong Phi Angle
    l_auSuperModuleNbTiles.x = (unsigned)l_fRadialRound(l_adSuperModuleSize.x/l_adEffectiveModuleSize.x);

    assert_ex(l_auSuperModuleNbTiles.x, Exception);

    // Re-adjust the size of the Super Module
    l_adSuperModuleSize.x = l_auSuperModuleNbTiles.x*l_adEffectiveModuleSize.x;

    if(l_adSuperModuleSize.x==0.)
        throw Exception("BarrelSingleLayer:: Assemble : Super Module X Size is Null. Please contact the dev team to report the bug ...");


    l_dMinRadius = l_adSuperModuleSize.x/2./tan(l_dAngleOrign2Module/2.);
    l_dMaxRadius = sqrt(l_adSuperModuleSize.x/2.*l_adSuperModuleSize.x/2. + l_dMinRadius*l_dMinRadius);

    // Reajust the size of the Max:Min Radius
    // Along the z axis
    assert_ex(l_adEffectiveModuleSize.x > 0, Exception);

    l_auSuperModuleNbTiles.y = (unsigned)l_fLongitudinalRound( GetLength()/l_adEffectiveModuleSize.y );
    l_adSuperModuleSize.y = l_auSuperModuleNbTiles.y*l_adEffectiveModuleSize.y;

    if(l_adSuperModuleSize.y==0.)
        throw Exception("BarrelSingleLayer:: Assemble : Super Module Y Size is Null. Please contact the dev team to report the bug ...");

    l_auNbTiles.x = l_auSuperModuleNbTiles.x*l_uNbFaces;
    l_auNbTiles.y = l_auSuperModuleNbTiles.y;

    //FInal Step
    SetInnerRadiusMin(l_dMinRadius);

    SetSuperModuleNbOfTiles(0, l_auSuperModuleNbTiles);
    SetSuperModuleSize(0, l_adSuperModuleSize);

    SetLength(l_auSuperModuleNbTiles.y * l_adEffectiveModuleSize.y);
}


void BarrelSingleLayer::AdjustParamCylinderLayer()
{
    typedef double (*round_function)(double);
    round_function l_fRadialRound = (!GetRadialAdjustment().compare("shrink")) ? &floor : &ceil;
    round_function l_fLongitudinalRound = (!GetRadialAdjustment().compare("shrink")) ? &floor : &ceil;

    TCuboidSize l_adEffectiveModuleSize = GetEffectiveModuleSize(0);

    TPlaneDistribution l_auNbTiles;
    TPlaneDistribution l_auSuperModuleNbTiles;
    TCuboidSize l_adSuperModuleSize;

    TAngle l_dAngleOrign2Module = 2.*atan(l_adEffectiveModuleSize.y/(2.*GetInnerRadiusMin()));

    l_auSuperModuleNbTiles.x = 1;
    l_auNbTiles.x = (unsigned)l_fRadialRound(2.*M_PI/l_dAngleOrign2Module);

    assert_ex(l_auNbTiles.x, Exception);

    SetNbFace(l_auNbTiles.x);

    l_adSuperModuleSize.x = l_adEffectiveModuleSize.x;
    l_dAngleOrign2Module = 2.*M_PI/l_auNbTiles.x;

    TLength l_dRadiusMin = l_adEffectiveModuleSize.x/(2.*tan(l_dAngleOrign2Module/2.));

    SetInnerRadiusMin(l_dRadiusMin);

    l_auSuperModuleNbTiles.y = (unsigned)l_fLongitudinalRound( GetLength()/l_adEffectiveModuleSize.y);

    assert_ex(l_auSuperModuleNbTiles.y, Exception);

    l_auNbTiles.y = l_auSuperModuleNbTiles.y * GetNbFace();
    l_adSuperModuleSize.y = l_auSuperModuleNbTiles.y * l_adEffectiveModuleSize.y;

    SetSuperModuleNbOfTiles(0, l_auSuperModuleNbTiles);
    SetLength(l_auSuperModuleNbTiles.y*l_adEffectiveModuleSize.y);
    SetSuperModuleSize(0, l_adSuperModuleSize);
}


void BarrelSingleLayer::FillLayer()
{
    TCuboidSize l_adEffectiveModuleSize = GetEffectiveModuleSize(0);
    TCuboidSize l_adSuperModuleSize = GetSuperModuleSize(0);
    TPlaneDistribution l_auNumberOfModule = GetSuperModuleNbTiles(0);
    TSolidAngle l_adModuleRotation;// = GetModuleRotation(0);
    l_adModuleRotation.psi = M_PI/2.-GetModuleDirection(0);
    l_adModuleRotation.theta = M_PI-GetModuleFace(0);
//    l_adModuleRotation.

    BuildState l_oState;
    l_oState.LayerId = 0;
    l_oState.IdX = 0;
    l_oState.IdY = 0;

    l_oState.PosX = -l_adSuperModuleSize.x/2.-l_adEffectiveModuleSize.x/2.;

    TLength l_dModuleZPosition =  0;
    TAngle l_dModuleFace = GetModuleDirection(0);
    TLength l_dModuleHeight = GetModulePrototype(0).GetSupport().GetThickness()
                              + GetModulePrototype(0).GetModuleSize().z
                              + GetModulePrototype(0).GetChipArray().GetChipSize().z;
    TLength l_dBarrelSupportThickness = GetBarrelSupportThickness();

    if(l_dModuleFace==180)
        l_dModuleZPosition = l_dModuleHeight;
    else
        l_dModuleZPosition = l_dBarrelSupportThickness;

    TLength l_dSensitiveRadius = l_dModuleZPosition;
    if(l_dModuleFace==180) l_dSensitiveRadius+=0.;
    else if(l_dModuleFace==0) l_dSensitiveRadius+=(l_dBarrelSupportThickness+GetModulePrototype(0).GetSupport().GetThickness());

    for(unsigned IdX=0; IdX<l_auNumberOfModule.x; IdX++)
    {
        l_oState.PosX += l_adEffectiveModuleSize.x;
        l_oState.PosY = -l_adSuperModuleSize.y/2. + l_adEffectiveModuleSize.y/2.;

        for(unsigned IdY=0; IdY<l_auNumberOfModule.y; IdY++)
        {
            TPosition l_adModulePosition;
            l_adModulePosition.x = l_oState.PosX;
            l_adModulePosition.y = l_oState.PosY;
            l_adModulePosition.z = l_dModuleZPosition;

            ModuleDescriptor l_oDescriptor(0, 0, l_adModuleRotation, l_adModulePosition);
            GetModulesDistribution().Add(l_oDescriptor);
            l_oState.PosY += l_adEffectiveModuleSize.y;
        }
    }
}

void BarrelSingleLayer::FillSuperModule()
{
    FillLayer();
}

unsigned BarrelSingleLayer::GetNbLayers() const
{
    return 1;
}
