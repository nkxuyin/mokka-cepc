/*! \file G4BarrelSingleLayer.cc
    \brief An implementation of Silc::G4BarrelSingleLayer class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "G4BarrelSingleLayer.hh"
#include "BarrelSingleLayer.hh"

using namespace std;
using namespace Silc;

static const char* SUPER_MODULE_SUPPORT_SHAPE = "_SuperModuleSupportShape_";
static const char* SUPER_MODULE_SUPPORT_VOLUME = "_SuperModuleSupportVolume_";

P<G4Barrel> G4BarrelSingleLayer::MakeInstance()
{
    return P<G4Barrel> ( new G4BarrelSingleLayer() );
}

void G4BarrelSingleLayer::Assemble()
{
    BarrelSingleLayer::Assemble();
    G4Barrel::Assemble();
}

void G4BarrelSingleLayer::MakeSupportAssembly()
{
    MakeSuperModuleSupport();
}

void G4BarrelSingleLayer::MakeSuperModuleSupport()
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
    G4ThreeVector l_oSupperModuleSupportShift(0, 0, GetBarrelSupportThickness()/2.);
    GetSupportSuperModuleAssembly()->AddPlacedVolume(l_pSuperModuleSupportVolume, l_oSupperModuleSupportShift, nullptr);
}
