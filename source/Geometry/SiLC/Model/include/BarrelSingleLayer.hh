/*! \file BarrelSingleLayer.hh
    \brief A definition of Silc::BarrelSingleLayer class.

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

#include "Barrel.hh"

namespace Silc
{
    class BarrelSingleLayer : virtual public Barrel
    {
    private:

        struct BuildState
        {
            unsigned LayerId;
            unsigned IdX;
            unsigned IdY;
            unsigned ldZ;
            TLength PosX;
            TLength PosY;
            TLength PosZ;
        };

        void AdjustParameters();
        void AdjustParamCylinderLayer();
        void AdjustParamPolygonLayer();

        void FillSuperModule();
        void FillLayer();

    public:
        static const BarrelType BARREL_TYPE;
        static P<Barrel> MakeInstance();

    public: // Silc::IAssemblable implementations.
        /// @copydoc Silc::Barrel::Assemble()
        virtual void Assemble();

    public: // Silc::Barrel implementations.
        /// @copydoc Silc::Barrel::GetBarrelType()
        virtual const BarrelType& GetBarrelType() const;

        virtual unsigned GetNbLayers() const;
    };
}
