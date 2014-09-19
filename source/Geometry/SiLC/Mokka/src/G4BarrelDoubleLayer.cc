/*! \file G4BarrelDoubleLayer.cc
    \brief An implementation of Silc::G4BarrelDoubleLayer class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "G4BarrelDoubleLayer.hh"
#include "BarrelDoubleLayer.hh"

using namespace std;
using namespace Silc;

static const char* SUPER_MODULE_SUPPORT_SHAPE = "_SuperModuleSupportShape_";
static const char* SUPER_MODULE_SUPPORT_VOLUME = "_SuperModuleSupportVolume_";

P<G4Barrel> G4BarrelDoubleLayer::MakeInstance()
{
    return P<G4Barrel> ( new G4BarrelDoubleLayer() );
}

void G4BarrelDoubleLayer::Assemble()
{
    BarrelDoubleLayer::Assemble();
    G4Barrel::Assemble();
}

void G4BarrelDoubleLayer::MakeSupportAssembly()
{
    MakeSuperModuleSupport();
}

void G4BarrelDoubleLayer::MakeSuperModuleSupport()
{
    for(unsigned layer=0; layer<2; layer++)
    {
        G4LogicalVolume *l_pSupportElementVolume = MakeSuperModuleSupportElement();
        G4ThreeVector l_oSupperModuleSupportShift(0, 0, (layer+0.5)*GetBarrelSupportThickness());
        GetSupportSuperModuleAssembly()->AddPlacedVolume(l_pSupportElementVolume, l_oSupperModuleSupportShift, nullptr);
    }
}

G4LogicalVolume* G4BarrelDoubleLayer::MakeSuperModuleSupportElement()
{
    TCuboidSize l_oSuperModuleMaxSize = GetMaxSModuleSize();
    TCuboidSize l_oSuperModuleSize(l_oSuperModuleMaxSize.x, l_oSuperModuleMaxSize.y, l_oSuperModuleMaxSize.z); /// here get the generic thickness

    G4VSolid* l_pSuperModuleSupportShape = G4Extensions::CreateG4Box(SUPER_MODULE_SUPPORT_SHAPE, l_oSuperModuleSize);
    G4Material* l_pMaterial = CGAGeometryManager::GetMaterial(GetBarrelSupportMaterial());
    G4LogicalVolume* l_pSuperModuleSupportVolume = new G4LogicalVolume(l_pSuperModuleSupportShape,
            l_pMaterial,
            SUPER_MODULE_SUPPORT_VOLUME);
    static const G4Colour SUPPORT_VISUALISATION_COLOR(0.2, 0.5, 0.7);
    G4Extensions::ApplyVisualisationColour(l_pSuperModuleSupportVolume, SUPPORT_VISUALISATION_COLOR);

    return l_pSuperModuleSupportVolume;
}
