/*! \file G4Endcap.hh
    \brief A definition of Silc::G4Endcap class.

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

#include "G4SiliconSubDetector.hh"
#include "Endcap.hh"

namespace Silc
{
    /// Provides an endcap construction functionality for the GEANT4 platform.
    class G4Endcap : virtual public Endcap, virtual public G4SiliconSubDetector
    {
    public: // IAssemblable implementations.
        /// Creates and fills a GEANT4 assembly volume of the endcap.
        virtual void Assemble();

    public: // Data access methods.
        /// Returns a sensitive detector prototype used for a zone with the specified ID.
        VSensitiveDetector* GetSensitiveDetector(unsigned p_uZoneId) const;

        /// Set the prefix nam of the sensitive volumes.
        void SetSensitiveVolumePrefix(string p_sPrefix);

        /// Returns a GEANT4 assembly volume of the endcap.
        G4AssemblyVolume* GetAssemblyVolume() const;

        /// Returns a GEANT4 assembly volume of a super-module prototype.
        G4AssemblyVolume* GetSuperModuleAssembly() const;

        /// ???
        G4AssemblyVolume* GetSupportAssembly() const;

        /// ???
        G4AssemblyVolume* GetActiveSurfaceAssembly(unsigned p_uLayerId) const;

    protected: // Expandable methods.
        /// Creates a GEANT4 assembly of the endcap's support.
        virtual void MakeSupportAssembly();

        /// Creates a GEANT4 assembly of a super-module.
        virtual void MakeSuperModuleAssembly();

    private: // Internal methods.

        /// ???
        void MakeActiveSurfaces();

        /// ???
        void MakeEndcapAssembly();

        /// Creates a GEANT4 assembly of a layer.
//        void MakeLayerAssembly();

    private: // Data members.
        /// A GEANT4 assembly of the endcap.
        G4AssemblyVolume* m_pEndcapAssembly;

        /// A GEANT4 assembly of a layer.
//        G4AssemblyVolume* m_pLayerAssembly;

        /// A GEANT4 assembly of a super-module
        G4AssemblyVolume* m_pSuperModuleAssembly;

        /// ???
        G4AssemblyVolume* m_pSupportAssembly;

        /// ???
        vector<G4AssemblyVolume*> m_vActiveSurfaces;
    };
}
