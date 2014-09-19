/*! \file G4Barrel.cc
    \brief An implementation of Silc::G4Barrel class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "G4Barrel.hh"

using namespace Silc;

G4AssemblyVolume* G4Barrel::GetAssemblyVolume() const
{
    return m_pBarrelAssembly;
}

G4LogicalVolume* G4Barrel::GetSensitiveVolume(unsigned p_uLayerId) const
{
    return GetG4ModulePrototype(p_uLayerId).GetSensitiveVolume();
}

VSensitiveDetector* G4Barrel::GetSensitiveDetector(unsigned p_uLayerId) const
{
    return GetG4ModulePrototype(p_uLayerId).GetSensitiveDetector();
}

void G4Barrel::Assemble()
{
    Barrel::Assemble();
    PlaceActiveVolume();
    MakeSuperModuleSupportAssembly();
    MakeSuperModuleAssembly();
    MakeSupportAssembly();
    MakeBarrelAssembly();
}

void G4Barrel::PlaceActiveVolume()
{
    m_pActiveSurfaceAssembly = new G4AssemblyVolume();

    for(unsigned layer=0; layer<GetNbLayers(); layer++)
    {
        const ModulesDistribution::DescriptorCollection& l_pModulesDistribution =
            GetModulesDistribution().SelectLayer(layer);

        for( Barrel::ModulesDistribution::Iterator l_pModuleDescriptor = l_pModulesDistribution.begin();
                l_pModuleDescriptor != l_pModulesDistribution.end(); ++l_pModuleDescriptor)
        {
            G4AssemblyVolume* l_pCurrentModuleAssembly = GetG4ModulePrototype(layer).GetAssemblyVolume();
            G4RotationMatrix l_oModuleRotation(l_pModuleDescriptor->m_adModuleRotation.psi,
                                               l_pModuleDescriptor->m_adModuleRotation.theta,
                                               l_pModuleDescriptor->m_adModuleRotation.phi);
            const TPosition& l_adModulePosition = l_pModuleDescriptor->m_adModulePosition;
            G4ThreeVector l_oModuleShift = G4Extensions::G4Vector(l_adModulePosition);
            G4Transform3D l_oModuleTransform (l_oModuleRotation, l_oModuleShift);
            m_pActiveSurfaceAssembly->AddPlacedAssembly(l_pCurrentModuleAssembly, l_oModuleTransform);
        }
    }
}

void G4Barrel::MakeSuperModuleSupportAssembly()
{
    m_pSupportSuperModuleAssembly = new G4AssemblyVolume();
}

void G4Barrel::MakeSuperModuleAssembly()
{
    m_pSuperModuleAssembly = new G4AssemblyVolume();

    G4Transform3D l_oTransform;
    m_pSuperModuleAssembly->AddPlacedAssembly(m_pActiveSurfaceAssembly, l_oTransform);
    m_pSuperModuleAssembly->AddPlacedAssembly(m_pSupportSuperModuleAssembly, l_oTransform);
}

void G4Barrel::MakeBarrelAssembly()
{
    m_pBarrelAssembly = new G4AssemblyVolume();

    for(unsigned sModuleID=0; sModuleID<GetNbFace(); sModuleID++)
    {
        const double l_dAngle = GetFaceRotationAngle(sModuleID);
        TCoordinate l_dXPosition = GetInnerRadiusMin() * sin(l_dAngle);
        TCoordinate l_dYPosition = GetInnerRadiusMin() * cos(l_dAngle);

        G4RotationMatrix l_oSuperModuleRotation (0., M_PI_2, l_dAngle);
        G4ThreeVector l_oSuperModuleShift (l_dXPosition, l_dYPosition, 0);
        G4Transform3D l_oSModuleTranform(l_oSuperModuleRotation, l_oSuperModuleShift);
        m_pBarrelAssembly->AddPlacedAssembly(m_pSuperModuleAssembly, l_oSModuleTranform);
    }
}

G4AssemblyVolume* G4Barrel::GetSupportSuperModuleAssembly() const
{
    return m_pSupportSuperModuleAssembly;
}
