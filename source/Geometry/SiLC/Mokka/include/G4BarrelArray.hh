/*! \file G4BarrelArray.hh
    \brief A definition of Silc::G4BarrelArray class.

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
#include "BarrelArray.hh"
#include "SiLCSD.hh"

namespace Silc
{
    /// Provides an barrel array construction functionality for the GEANT4 platform.
    class G4BarrelArray : public BarrelArray
    {
    private: // Type definitions.
        /// ???
        typedef map< Barrel*, G4Barrel* > BarrelOriginMap;

        /// ???
        typedef map< const Barrel*, const G4Barrel* > BarrelConstOriginMap;

        /// An barrel maker.
        typedef P<G4Barrel> (*BarrelMaker)();

        /// A correspondence provider between the barrel types and the barrel makers.
        typedef map<Barrel::BarrelType, BarrelMaker> BarrelTypeMap;

    public: // Basic methods.
        /// A constructor.
        G4BarrelArray(G4LogicalVolume& p_oG4World);

        /// ???
        G4Barrel& GetG4Barrel(unsigned p_uBarrelId);

        /// ???
        const G4Barrel& GetG4Barrel(unsigned p_uBarrelId) const;


    public: // Silc::IAssemblable implementations.
        /// Assembles the array and places it assembly to the GEANT4 world volume.
        virtual void Assemble();

    public: // Silc::BarrelArray implementations.
        /// @copydoc Silc::BarrelArray::MakeBarrel()
        virtual P<Barrel> MakeBarrel(const Barrel::BarrelType &p_sBarrelType);

    public:
        //// Declare sensitive volume
        //template<typename OutputType> OutputType GetSensitiveDetectors() const;
        vector<VSensitiveDetector*> GetSensitiveDetectors() const;
//        vector<G4LogicalVolume*> GetSensitiveVolumes() const;

        // Register the sensitive volumes as sensitive detectors
//        void SetSensitiveDetector(SiLCSD *p_poSD);
        /// Pointer to set the sensitive detector
//        void SetModuleSensitiveDetector(SiLCSD *p_poSD);

    private: // Internal methods.
        /// Makes an barrel type correspondence map initalization.
        static BarrelTypeMap InitializeBarrelTypeMap();

    private: // Data members.
        /// ???
        BarrelOriginMap m_mBarrelOrigins;

        /// ???
        BarrelConstOriginMap m_mBarrelConstOrigins;

        /// A GEANT4 world logical volume.
        G4LogicalVolume& m_oG4World;
    };
}
