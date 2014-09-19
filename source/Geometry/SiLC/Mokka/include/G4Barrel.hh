/*! \file G4Barrel.hh
    \brief A definition of Silc::G4Barrel class.

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
#include "G4SiliconSubDetector.hh"

namespace Silc
{
    class G4Barrel : virtual public Barrel, virtual public G4SiliconSubDetector
    {
    public:
        virtual void Assemble();

        VSensitiveDetector* GetSensitiveDetector(unsigned p_uLayerId) const;
        G4LogicalVolume* GetSensitiveVolume(unsigned p_uLayerId) const;

    public:
        G4AssemblyVolume* GetAssemblyVolume() const;
        G4AssemblyVolume* GetSupportSuperModuleAssembly() const;

    protected:
        virtual void MakeSupportAssembly() = 0;

    private:
        void PlaceActiveVolume();
        void PlaceDetectionElement();
        void MakeSuperModuleAssembly();
        void MakeSuperModuleSupportAssembly();
        void MakeDetectionElementAssembly();
        void MakeBarrelAssembly();

    private:
        G4AssemblyVolume* m_pBarrelAssembly;
        G4AssemblyVolume* m_pActiveSurfaceAssembly;
        G4AssemblyVolume* m_pSupportSuperModuleAssembly;
        G4AssemblyVolume* m_pSuperModuleAssembly;
        G4AssemblyVolume* m_pDetectionElementAssembly;

    };
}

