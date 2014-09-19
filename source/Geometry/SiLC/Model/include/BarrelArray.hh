/*! \file BarrelArray.hh
    \brief A definition of Silc::BarrelArray class.

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
#include "SubDetectorArray.hh"

namespace Silc
{
    class BarrelArray : public SubDetectorArray, public IAssemblable
    {

    public:        
  
        /// ???
        class LayerDescriptor;

    
    private: // Type definitions.
        /// A vector of barrel.
        typedef vector< P<Barrel> > BarrelVector;

        /// ???
        typedef vector< P<LayerDescriptor> >  LayerVector;

        /// An barrel maker.
        typedef P<Barrel> (*BarrelMaker)();

        /// A correspondence provider between the barrel types and the barrel makers.
        typedef map<Barrel::BarrelType, BarrelMaker> BarrelTypeMap;

    private: // Static methods.
        /// Makes an barrel type correspondence map initalization.
        static BarrelTypeMap InitializeBarrelTypeMap();

    public: // Silc::IAssemblable implementations.
        /// @copydoc Silc::IAssemblable
        virtual void Assemble();

    public: // Silc::SubDetectorArray implementations.
        /// @copydoc Silc::SubDetectorArray::GetSensorPrototype()
        virtual const Sensor& GetSensorPrototype(unsigned p_uLayerId) const;

        /// @copydoc Silc::SubDetectorArray::TransformVectorToLocal()
        virtual TVector TransformVectorToLocal(const TVector& p_vGlobalVector, GlobalNodeId p_uSensorId) const;

        /// @copydoc Silc::SubDetectorArray::TransformVectorToGlobal()
        virtual TVector TransformVectorToGlobal(const TVector& p_vLocalVector, GlobalNodeId p_uSensorId) const;

        /// @copydoc Silc::SubDetectorArray::FindGlobalNodeId()
        virtual GlobalNodeId FindGlobalNodeId(const TVector& p_vGlobalVector) const;

        /// @copydoc Silc::SubDetectorArray::GetSensitiveDetectorNames()
        virtual const set<string>& GetSensitiveDetectorNames() const;

    protected: // Virtual methods.
        /// Creates a new andcap with a type specified by 'p_sBarrelType'.
        virtual P<Barrel> MakeBarrel(const Barrel::BarrelType &p_sBarrelType);

    public:
        Barrel& AddNewBarrel(const Barrel::BarrelType& p_sBarrelType);
        Barrel& operator[] (unsigned p_uBarrelId);
        const Barrel& operator[] (unsigned p_uBarrelId) const;
        unsigned GetNumberOfBarrels() const;        ///< A number of installed endcaps.

        unsigned GetTotalNumberOfLayers() const;
        unsigned GetNbLayerInTheCurrentBarrel(unsigned p_uBarrelId) const;
        TLength GetInnerRadius(unsigned p_uBarrelId) const;
        TLength GetZLength(unsigned p_uBarrelId) const;

        void SetSensitiveDetectorNames(const string& p_sPrefix);

        const LayerDescriptor& GetLayerDescriptor(unsigned p_uLayerId) const;

    private: // Data members.
        /// A vector of barrels in the array.
        BarrelVector m_apBarrels;

        /// ???
        LayerVector m_vLayers;

        set<string> m_SensitiveDetectorNames;
    };

    class BarrelArray::LayerDescriptor
    {
    public:
        unsigned BarrelId;
        unsigned LayerId;

    public:
        LayerDescriptor(unsigned p_uBarrelId, unsigned p_uLayerId);
    };


    /// Provides a stream output for BarrelArray class.
    ostream& operator<<(ostream& o, const BarrelArray& p_oBarrelArray);
}
