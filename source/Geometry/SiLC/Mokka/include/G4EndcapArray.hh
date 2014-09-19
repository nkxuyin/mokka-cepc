/*! \file G4EndcapArray.hh
    \brief A definition of Silc::G4EndcapArray class.

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

#include "G4Endcap.hh"
#include "EndcapArray.hh"

namespace Silc
{
    /// Provides an endcap array construction functionality for the GEANT4 platform.
    class G4EndcapArray : public EndcapArray
    {
    private: // Type definitions.
        /// ???
        typedef map< P<Endcap>, P<G4Endcap> > EndcapOriginMap;

        /// An endcap maker.
        typedef P<G4Endcap> (*EndcapMaker)();

        /// A correspondence provider between the endcap types and the endcap makers.
        typedef map<Endcap::EndcapType, EndcapMaker> EndcapTypeMap;

    public: // Basic methods.
        /// A constructor.
        G4EndcapArray(G4LogicalVolume& p_oG4World);

    public: // IAssemblable implementations.
        /// ??? Assembles the array and places it assembly to the GEANT4 world volume.
        virtual void Assemble();

    public:
        //// Declare sensitive volume
        vector<VSensitiveDetector*> GetSensitiveDetectors() const;
        /// Set the preifx name for the sensitive detector
        void SetDriverPrefix(const string& p_sPrefix);
        //================
        /// shouldn't be here - TODO list
        vector<TLength> GetEndcapZPositions();
        ///
        //EndcapDesciptor& GetEndcapDescriptor(unsigned p_uEndcapId);
        //============

    public: // EndcapArray implementations.
        /// Creates a new endcap with a type specified by 'p_sEndcapType'.
        virtual P<Endcap> MakeEndcap(const Endcap::EndcapType &p_sEndcapType);

    private: // Internal methods.

        /// Makes an endcap type correspondence map initalization.
        static EndcapTypeMap InitializeEndcapTypeMap();

        /// Creates a GEANT4 assembly of a detection element.
        void MakeDetectionElementAssembly();

        /// ??? Places a pre-build detection element at the endcap assembly.
        void PlaceDetectionElement(TAngle p_dRotation, TCoordinate p_dShift);

    private: // Data members.
        /// ???
        EndcapOriginMap m_mEndcapOrigins;

        /// ??? A GEANT4 assembly of a detection element.
        G4AssemblyVolume* m_pDetectionElementAssembly;

        /// A GEANT4 world logical volume.
        G4LogicalVolume& m_oG4World;
    };
}
