/*! \file G4BarrelDoubleLayer.hh
    \brief A definition of Silc::G4BarrelDoubleLayer class.

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

#include "G4Barrel.hh"
#include "BarrelDoubleLayer.hh"

namespace Silc
{
    class G4BarrelDoubleLayer : public BarrelDoubleLayer, public G4Barrel
    {
    public:
        static P<G4Barrel> MakeInstance();

    public:
        virtual void Assemble();
        virtual void MakeSupportAssembly();

        void MakeSuperModuleSupport();
        G4LogicalVolume* MakeSuperModuleSupportElement();
    };
}
