/*! \file G4BarrelArray.cc
    \brief An implementation of Silc::G4BarrelArray class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "G4BarrelArray.hh"
#include "G4BarrelSingleLayer.hh"
#include "G4BarrelDoubleLayer.hh"

using namespace Silc;

G4BarrelArray::BarrelTypeMap G4BarrelArray::InitializeBarrelTypeMap()
{
    BarrelTypeMap result;
    result[G4BarrelSingleLayer::BARREL_TYPE] = &G4BarrelSingleLayer::MakeInstance;
    result[G4BarrelDoubleLayer::BARREL_TYPE] = &G4BarrelDoubleLayer::MakeInstance;
    return result;
}

P<Barrel> G4BarrelArray::MakeBarrel(const Barrel::BarrelType& p_oBarrelType)
{
    static const BarrelTypeMap l_TypeMap = InitializeBarrelTypeMap();
    BarrelTypeMap::const_iterator iter = l_TypeMap.find(p_oBarrelType);
    assert(iter != l_TypeMap.end());
    P<G4Barrel> l_pBarrel = iter->second();
    m_mBarrelOrigins[l_pBarrel] = l_pBarrel;
    m_mBarrelConstOrigins[l_pBarrel] = l_pBarrel;
    return l_pBarrel;
}

G4BarrelArray::G4BarrelArray(G4LogicalVolume& p_oG4World)
    : m_oG4World(p_oG4World)
{
}

G4Barrel& G4BarrelArray::GetG4Barrel(unsigned p_uBarrelId)
{
    Barrel& l_oBarrel = (*this)[p_uBarrelId];
    return *m_mBarrelOrigins[&l_oBarrel];
}

const G4Barrel& G4BarrelArray::GetG4Barrel(unsigned p_uBarrelId) const
{
    const Barrel& l_oBarrel = (*this)[p_uBarrelId];
    return *m_mBarrelConstOrigins.find(&l_oBarrel)->second;
}

void G4BarrelArray::Assemble()
{
    BarrelArray::Assemble();
    G4Transform3D l_g4Transform;
    for(unsigned barrel=0; barrel<GetNumberOfBarrels(); barrel++)
    {
        G4AssemblyVolume *l_pBarrelAssemblyVolume = GetG4Barrel(barrel).GetAssemblyVolume();
        l_pBarrelAssemblyVolume->MakeImprint( &m_oG4World, l_g4Transform, 0, false);
    }
}

vector<VSensitiveDetector*> G4BarrelArray::GetSensitiveDetectors() const
{
    vector<VSensitiveDetector*> l_vDetectors;

    for(unsigned barrel=0; barrel<GetNumberOfBarrels(); barrel++)
    {
        const G4Barrel& l_oBarrel = GetG4Barrel(barrel);
        for(unsigned layer=0; layer<l_oBarrel.GetNbLayers(); layer++)
            l_vDetectors.push_back(l_oBarrel.GetSensitiveDetector(layer));
    }

    return l_vDetectors;
}
