/*! \file G4Globals.cc
    \brief An implementation of GEANT4 adaptations for the Silc project.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "G4Globals.hh"

G4Box* Silc::G4Extensions::CreateG4Box(const string& p_sBoxName, const TCuboidSize& p_oBoxSize)
{
    return new G4Box(p_sBoxName, p_oBoxSize.x / 2, p_oBoxSize.y / 2, p_oBoxSize.z / 2);
}

void Silc::G4Extensions::ApplyVisualisationColour(G4LogicalVolume* p_pLogicalVolume, const G4Colour& p_oColour)
{
    G4VisAttributes* l_pAttributes = new G4VisAttributes(p_oColour);
    l_pAttributes->SetForceSolid(true);
    p_pLogicalVolume->SetVisAttributes(l_pAttributes);
}

G4ThreeVector Silc::G4Extensions::G4Vector(const TPosition& p_vPosition)
{
    return G4ThreeVector(p_vPosition.x, p_vPosition.y, p_vPosition.z);
}

Silc::TEnergyLinearDensity Silc::G4Extensions::CalculateEnergyLoss(const G4Material * p_poMaterial)
{
    G4double l_dDEdx;
    G4EmCalculator l_poDEdxFinder;

    G4ParticleTable* m_poParticleTable = G4ParticleTable::GetParticleTable();

    //  Looping over bins in the dEdx table to obtain the MIP
    //  =>  from energy 0.0001MeV to 1000MeV in steps of 10
    G4double l_dBinSize = 10.;
    G4double l_dDEdxMin = 99999.;

    for (G4double ebin=0.0001; ebin<=1000.; ebin+=l_dBinSize)
    {
        l_dDEdx = l_poDEdxFinder.ComputeTotalDEDX(ebin, m_poParticleTable->FindParticle("mu-"), p_poMaterial);
        if(l_dDEdx<l_dDEdxMin) l_dDEdxMin = l_dDEdx;
    }

    return l_dDEdx/1000.;
}
