/*! \file G4Module.hh
    \brief A definition of Silc::G4Module class.

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

#include "G4Globals.hh"
#include "Module.hh"

namespace Silc
{
    /// Provides a silicon module construction functionality for the GEANT4 platform.
    class G4Module : public Module
    {
    public: // Basic methods.
        /// The Silc::G4Module constructor.
        G4Module(const SensorArray& p_oSensorArray, const ChipArray& p_oChipArray,
                 const Support& p_oModuleSupport) throw();

    public: // IAssemblable implementations.
        /// Creates and fills a GEANT4 assembly volume of the module.
        virtual void Assemble();

    public: // Data access methods.
        /// Returns a GEANT4 assembly volume of the silicon module.
        G4AssemblyVolume* GetAssemblyVolume() const;

        /// Returns a sensitive detector prototype which the silicon module uses.
        VSensitiveDetector* GetSensitiveDetector() const;

        /// ??? Returns a sensitive detector prototype which the silicon module uses.
        G4LogicalVolume* GetSensitiveVolume() const;

    private: // Internal methods.
        /// Creates a sensitive volume description.
        void MakeSensitiveVolume();

        /// Creates a silicon module logical volume.
        G4LogicalVolume* CreateSiliconModuleVolume();

        /// Creates a chip logical volume.
        G4LogicalVolume* CreateChipVolume();

        /// Creates a chip module assembly.
        G4AssemblyVolume* CreateChipAssembly(G4LogicalVolume* p_pChipVolume);

        /// Creates a support module logical volume.
        G4LogicalVolume* CreateSupportVolume();

        /// Creates a module assembly.
        void MakeModuleAssembly(G4LogicalVolume* p_pSiliconModuleVolume, G4AssemblyVolume* p_pChipAssembly,
                                G4LogicalVolume* p_pSupportVolume);

    private: // Data members.
        /// A GEANT4 assembly of this module.
        G4AssemblyVolume* m_pModuleAssembly;

        /// A sensitive detector description.
        VSensitiveDetector* m_pSensitiveDetector;
        G4LogicalVolume* m_pSensitiveVolume;
    };
}
