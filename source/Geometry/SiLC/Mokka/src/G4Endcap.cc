/*! \file G4Endcap.cc
    \brief An implementation of Silc::G4Endcap class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "G4Endcap.hh"

using namespace Silc;

// Constants.
static const unsigned NUMBER_OF_SUPER_MODULES_PER_LAYER = 4;
static const TAngle SUPER_MODULE_ROTATION_ANGLE = M_PI_2;

G4AssemblyVolume* G4Endcap::GetAssemblyVolume() const
{
    return m_pEndcapAssembly;
}

VSensitiveDetector* G4Endcap::GetSensitiveDetector(unsigned p_uZoneId) const
{
    assert(p_uZoneId < GetNumberOfZones());
    return GetG4ModulePrototype(p_uZoneId).GetSensitiveDetector();
}

void G4Endcap::SetSensitiveVolumePrefix(string p_sPrefix)
{
    for(unsigned zone=0; zone<GetNumberOfZones(); zone++)
    {
        stringstream l_sNewPrefix;
        l_sNewPrefix << p_sPrefix << "_zone_" << zone;
        GetModulePrototype(zone).SetSensitiveDetectorName(l_sNewPrefix.str());
    }
}

void G4Endcap::Assemble()
{
    Endcap::Assemble();
    MakeActiveSurfaces();
    MakeSupportAssembly();
    MakeSuperModuleAssembly();
    MakeEndcapAssembly();
}

G4AssemblyVolume* G4Endcap::GetSuperModuleAssembly() const
{
    return m_pSuperModuleAssembly;
}

G4AssemblyVolume* G4Endcap::GetSupportAssembly() const
{
    return m_pSupportAssembly;
}

G4AssemblyVolume* G4Endcap::GetActiveSurfaceAssembly(unsigned p_uLayerId) const
{
    if(p_uLayerId >= m_vActiveSurfaces.size())
        throw Exception("G4Endcap::GetActiveSurfaceAssembly : An assembly of the active surface was not found.");
    return m_vActiveSurfaces[p_uLayerId];
}

void G4Endcap::MakeActiveSurfaces()
{
    for(unsigned l_uLayerId = 0; l_uLayerId < GetNumberOfJoinedLayers(); ++l_uLayerId)
    {
        G4AssemblyVolume* l_pActiveSurfaceAssembly = new G4AssemblyVolume();

        const ModulesDistribution::DescriptorCollection& l_oModulesDistribution =
            GetModulesDistribution().SelectLayer(l_uLayerId);

        for( Endcap::ModulesDistribution::Iterator l_pModuleDescriptor = l_oModulesDistribution.begin();
                l_pModuleDescriptor != l_oModulesDistribution.end(); ++l_pModuleDescriptor)
        {
            G4AssemblyVolume* l_pCurrentModuleAssembly =
                GetG4ModulePrototype(l_pModuleDescriptor->ZoneId).GetAssemblyVolume();
            G4RotationMatrix l_oModuleRotation ( l_pModuleDescriptor->ModuleRotation,
                                                 abs((double)l_uLayerId-1)*M_PI, 0.0 );
            const TPlanePosition& l_vModulePosition = l_pModuleDescriptor->ModulePosition;
            //const TLength l_dModuleZShift = -GetModulePrototype(l_pModuleDescriptor->ZoneId).GetModuleSize().z;
            const TLength l_dModuleZShift = 0.;
            G4ThreeVector l_oModuleShift ( l_vModulePosition.x, l_vModulePosition.y, l_dModuleZShift);
            G4Transform3D l_oModuleTransform (l_oModuleRotation, l_oModuleShift);
            l_pActiveSurfaceAssembly->AddPlacedAssembly(l_pCurrentModuleAssembly, l_oModuleTransform);
        }
        m_vActiveSurfaces.push_back(l_pActiveSurfaceAssembly);
    }
}

void G4Endcap::MakeSuperModuleAssembly()
{
    m_pSuperModuleAssembly = new G4AssemblyVolume();
}

void G4Endcap::MakeSupportAssembly()
{
    m_pSupportAssembly = new G4AssemblyVolume();
}

void G4Endcap::MakeEndcapAssembly()
{
    m_pEndcapAssembly = new G4AssemblyVolume();

    for(unsigned n = 0; n < NUMBER_OF_SUPER_MODULES_PER_LAYER; n++)
    {
        G4RotationMatrix l_oSuperModuleRotation (n * SUPER_MODULE_ROTATION_ANGLE, 0, 0);
        G4ThreeVector l_oSuperModuleShift (0, 0, 0);
        G4Transform3D l_oQuadrantTranform(l_oSuperModuleRotation, l_oSuperModuleShift);
        m_pEndcapAssembly->AddPlacedAssembly(m_pSuperModuleAssembly, l_oQuadrantTranform);
    }
}
