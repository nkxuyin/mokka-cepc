/*! \file G4Globals.hh
    \brief Consolidates all includes from GEANT4 which needed for the classes in Silc namespace.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#pragma once

#include <globals.hh>
#include <G4LogicalVolume.hh>
#include <G4AssemblyVolume.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4VisAttributes.hh>
#include <G4VSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4UnionSolid.hh>
#include <G4Trap.hh>
#include <G4EmCalculator.hh>
#include <G4ParticleTable.hh>

#include <CGAGeometryManager.hh>
//#include <TRKSD00.hh>
#include <SiLCSD.hh>
#include <VSubDetectorDriver.hh>

#include "Silc_Globals.hh"

namespace Silc
{
    namespace G4Extensions
    {
        /// ???
        G4Box* CreateG4Box(const string& p_sBoxName, const TCuboidSize& p_oBoxSize);

        /// ???
        void ApplyVisualisationColour(G4LogicalVolume* p_pLogicalVolume, const G4Colour& p_oColour);

        /// ???
        G4ThreeVector G4Vector(const TPosition& p_vPosition);

        /// ???
        TEnergyLinearDensity CalculateEnergyLoss(const G4Material * p_poMaterial);
    }
}
