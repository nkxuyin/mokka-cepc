/*! \file EndcapArray.hh
    \brief A definition of Silc::EndcapArray class.

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

#include "Endcap.hh"

namespace Silc
{
    /// Represents an complete configuration of the endcap array.
    class EndcapArray : public IAssemblable
    {
    public: //Type definitions.
        struct EndcapDescriptor;

        /// A vector of endcap descriptors.
        typedef vector< P<EndcapDescriptor> > DescriptorVector;

        /// ???
        typedef map< P<Endcap>, vector<unsigned> > EndcapSet;

        /// A vector of angles.
        typedef vector<TAngle> AngleArray;

    private: // Type definitions.
        /// An endcap maker.
        typedef P<Endcap> (*EndcapMaker)();

        /// A correspondence provider between the endcap types and the endcap makers.
        typedef map<Endcap::EndcapType, EndcapMaker> EndcapTypeMap;

    public: // IAssemblable implementations.
        /// ??? Assembles all endcaps in the array.
        virtual void Assemble();

    public: // Virtual methods.
        /// Creates a new andcap with a type specified by 'p_sEndcapType'.
        virtual P<Endcap> MakeEndcap(const Endcap::EndcapType &p_sEndcapType);

    public: // Data access methods.
        ///  ??? Creates a new endcap with a type specified by 'p_sEndcapType', adds this endcap to array and returns it
        /// position into the array.
        unsigned AddNewEndcap(const Endcap::EndcapType p_sEndcapType, TCoordinate p_dZPosition, TAngle p_dRotation);

        /// ???
        unsigned AddDuplicateEndcap(unsigned p_uOriginalIndex, TCoordinate p_dZPosition, TAngle p_dRotation);

        /// Returns a reference to an endcap descriptor with ID specified by 'p_uEndcapId'.
        EndcapDescriptor& operator[] (unsigned p_uEndcapId);

        /// Returns a constant reference to an endcap descriptor with ID specified by 'p_uEndcapId'.
        const EndcapDescriptor& operator[] (unsigned p_uEndcapId) const;

        /// Returns a number of endcaps in the array.
        unsigned GetNumberOfEndcaps() const;

        /// Returns a total number of layers summed over all endcap in the array.
        unsigned GetTotalNumberOfLayers() const;

        /// ??? Returns a number of zones with a different required measurement accuracy. As a result, each zone has it's
        /// own module prototype.
        unsigned GetNumberOfZones() const;

        /// ??? Sets a number of zones with a different required measurement accuracy.
        void SetNumberOfZones(unsigned p_uNumberOfZones);

        /// ??? Returns an angle which defines an endcap zone specified by 'p_uZoneId'. This angle defines a cone with a
        /// vertex situated at the impact point and an axis coincident with the beam flow axis.
        TAngle GetZoneAngle(unsigned p_uZoneId) const;

        /// ??? Sets an angle which defines an endcap zone specified by 'p_uZoneId'.
        /// An angle value should be less than Pi/2.
        void SetZoneAngle(unsigned p_uZoneId, TAngle p_dZoneAngle);

        /// ???
        bool HasMirrorImage() const;

        /// ???
        void SetMirrorImageFlag(bool l_bHasMirrorImage);

        const EndcapSet& GetBaseEndcaps() const;
        EndcapSet& GetBaseEndcaps();

        /// Patch for GEAR v1
//        vector<TLength> GetInnerRadius();
//        vector<TLength> GetOuterRadius();
        //vector<TLength> GetEndcapZPositions(); // in G4 where it shouldn't be
//        TLength GetSensorThickness();
//        TLength GetSupportThickness();

    private: // Internal methods.
        /// Makes an endcap type correspondence map initalization.
        static EndcapTypeMap InitializeEndcapTypeMap();

        /// ???
        unsigned AddEndcap(P<Endcap> p_pEndcap, TCoordinate p_dZPosition, TAngle p_dRotation);

        /// Returns an analitical solution for X coordinate of vector (p_dSquareHalfSide, p_dSquareHalfSide) after
        /// rotation to an angle p_dAngle.
        static TLength GetSquareOverstep(TLength p_dSquareHalfSide, TAngle p_dAngle);

        /// Calculates a maximal possible overstep of inner structure among all endcap layers.
        TLength CalculateMaximalSquareOverstep(P<Endcap> p_pEndcap);


    private: // Data members.
        /// ??? A vector of endcaps in the array.
        DescriptorVector m_vDescriptors;

        /// ???
        EndcapSet m_vEndcaps;

        /// ??? A vector of angles which defines an endcap zones with a different measurement accuracy. Each angle defines
        /// a cone with a vertex situated at the impact point and an axis coincident with the beam flow axis. An angle
        /// value should be less than Pi/2.
        AngleArray m_vZoneAngle;

        /// ???
        bool m_bHasMirrorImage;
    };

    /// ???
    struct EndcapArray::EndcapDescriptor
    {
        /// ???
        P<Endcap> EndcapObject;

        /// An endcap position at the Z-axe : a distance between the collision point and the first endcap layer.
        /// ???
        TCoordinate ZPosition;

        /// ???
        TAngle RotationAngle;
    };

    /// Provides a stream output for EndcapArray class.
    ostream& operator<<(ostream& o, const EndcapArray& p_oEndcapArray);
}
