/*! \file Endcap.hh
    \brief A definition of Silc::Endcap class.

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

#include "SiliconSubDetector.hh"

namespace Silc
{
    /// An abstract class which represents a silicon sub-detector in the shape of the endcap.
    class Endcap : public virtual SiliconSubDetector
    {
    public: //Type definitions.
        /// An endcap type.
        typedef string EndcapType;

        /// A descriptor for identify a module in the super-module.
        class ModuleDescriptor;

        /// A distribution of the modules into the super-module.
        class ModulesDistribution;

        /// ???
        class Filler;

    public: // Basic methods.
        /// A default constructor.
        Endcap();

    public: // Abstract methods.
        /// ???
        virtual unsigned GetNumberOfJoinedLayers() const = 0;

        /// Returns a type of the endcap.
        virtual const EndcapType& GetType() const = 0;

    public: // Data access methods.
        /// Returns a number of zones with a different required measurement accuracy. As a result, each zone has it's
        /// own module prototype.
        unsigned GetNumberOfZones() const;

        /// Sets a number of zones with a different required measurement accuracy.
        void SetNumberOfZones(unsigned p_uNumberOfZones);

        /// Returns an inner endcap radius. An upper bound for the inner structure volume.
        TLength GetInnerRadius() const;

        /// Returns an outer endcap radius. A lower bound for the outer structure volume.
        TLength GetOuterRadius() const;

        /// Sets the lower and the upper bounds of the endcap.
        void SetLimits(TLength p_dInnerRadius, TLength p_dOuterRadius);

        /// Returns an effective inner endcap radius: a lower bound for a super-module construction to preserve a layer
        /// rotation invariance.
        TLength GetEffectiveInnerRadius() const;

        /// Sets an effective inner endcap radius.
        void SetEffectiveInnerRadius(TLength p_dEffectiveInnerRadius);

        /// Returns a width of a plane part of an inactive support area of the super-module.
        TLength GetPlaneSupportWidth() const;

        /// Returns a width of a circular part of an inactive support area of the super-module.
        TLength GetCircularSupportWidth() const;

        /// ??? Sets a width of a circular part of an inactive support area of the super-module.
        /// Sets a width of a plane part of an inactive support area of the super-module.
        void SetSupermoduleSupportInformation(TLength p_dPlaneSupportWidth, TLength p_dCircularSupportWidth);

        /// ???
        TLength GetCarbonThickness() const;

        /// ???
        TLength GetMousseThickness() const;

        /// ???
        TLength GetReinforcementThickness() const;

        /// ???
        void SetEndcapSupportInformation(TLength p_dCarbonThickness, TLength p_dMousseThickness,
                                         TLength p_dReinforcementThickness);

        /// Returns a radius which defines an endcap zone specified by 'p_uZoneId'. It is a radius of a circle which is
        /// given by intersection of the zone cone and a plane of the first endcap surface.
        TLength GetZoneRadius(unsigned p_uZoneId) const;

        /// ???
        void SetZoneRadius(unsigned p_uZoneId, TLength p_dZoneRadius);

        /// Returns a reference for an object which defines a modules distribution in the endcap.
        ModulesDistribution& GetModulesDistribution();

        /// Returns a constant reference for an object which defines a modules distribution in the endcap.
        const ModulesDistribution& GetModulesDistribution() const;

        /// Returns an active inner endcap radius.
        TLength GetActiveInnerRadius() const;

        /// Returns an active outer endcap radius.
        TLength GetActiveOuterRadius() const;

        /// Returns an orthogonal displacement of the super-module short side relatively to the quadrant line.
        TLength GetSuperModuleShift() const;

        /// Returns an orthogonal displacement of an active part of the super-module relatively to the quadrant line.
        TLength GetActiveSuperModuleShift() const;

        /// ???
        TLength GetThickness() const;

        /// ???
        TLength GetMaxModuleThickness() const;

        /// ???
        void SetSupportMaterials(const MaterialObject& p_oCarbonSupport, const MaterialObject& p_oMousseSupport);

        /// ???
        const MaterialObject& GetCarbonSupportMaterial() const;

        /// ???
        const MaterialObject& GetMousseSupportMaterial() const;

    private: // Data members.
        /// An inner endcap radius. An upper bound for the inner structure volume.
        TLength m_dInnerRadius;

        /// An outer endcap radius. A lower bound for the outer structure volume.
        TLength m_dOuterRadius;

        /// An effective inner endcap radius.
        TLength m_dEffectiveInnerRadius;

        /// A width of a plane part of an inactive support area of the super-module.
        TLength m_dPlaneSupportWidth;

        /// A width of a circular part of an inactive support area of the super-module.
        TLength m_dCircularSupportWidth;

        /// ???
        TLength m_dCarbonThickness;

        /// ???
        TLength m_dMousseThickness;

        /// ???
        TLength m_dReinforcementThickness;

        /// ???
        P<MaterialObject> m_pCarbonMaterial;

        /// ???
        P<MaterialObject> m_pMousseMaterial;

        /// ???
        vector<TLength> m_vZoneRadius;

        /// A pointer to an object which defines a modules distribution in the endcap.
        P<ModulesDistribution> m_pModulesDistribution;
    };

    /// A descriptor for identify a module in the super-module.
    class Endcap::ModuleDescriptor
    {
    public: // Type definitions.
        /// A module coordinate rotation.
        typedef TAngle RotationIndicator;

    public: // Static members.
        /// Indicates that module does not rotated.
        static const RotationIndicator ZeroRotation;

        /// Indicates that module does rotated at Pi/2.
        static const RotationIndicator OrtogonalRotation;

    public: // Basic methods.
        /// A descriptor constructor.
        ModuleDescriptor(unsigned p_LayerId, unsigned p_uZoneId, RotationIndicator p_bModuleRotation,
                         const TPlanePosition& p_adModulePosition);

    public: // Data members.
        /// ???
        const unsigned LayerId;

        /// An identifier of the zone to which the module belongs.
        const unsigned ZoneId;

        /// An indicator of the module rotation.
        const RotationIndicator ModuleRotation;

        /// A relative position of the module into a super-module.
        const TPlanePosition ModulePosition;
    };

    /// A distribution of the modules into the super-module.
    class Endcap::ModulesDistribution
    {
    public: // Type definitions.
        /// ???
        typedef list<ModuleDescriptor>  DescriptorCollection;

        /// An distribution iterator.
        typedef DescriptorCollection::const_iterator Iterator;

    private: // ???
        /// ???
        typedef map<unsigned, DescriptorCollection> LayerIndex;

    public: // Basic methods.
        /// Adds a module described by 'p_oDescriptor' to the endcap.
        void Add(const ModuleDescriptor& p_oDescriptor);

        /// Removes all modules from the endcap.
        void Clear();

        /// Returns an iterator referring to the first element in the distribution container.
        Iterator Begin() const;

        /// Returns an iterator referring to the past-the-end element in the distribution container.
        Iterator End() const;

        /// ???
        const DescriptorCollection& SelectLayer(unsigned p_uLayerId);

    private: // Data members.
        /// A container for descriptors of all endcap modules.
        DescriptorCollection m_oDescriptors;

        /// ???
        LayerIndex m_oLayerIndex;
    };

    /// ???
    class Endcap::Filler
    {
    public: // Basic methods.
        /// ???
        Filler(unsigned p_uNumberOfZones, TLength p_dInitialPosition);

        /// ???
        void InitializeZone(unsigned p_uZoneId, TLength p_dModuleLength,
                            TLength p_dZoneLeftLimit, TLength p_dZoneRightLimit);

        /// ???
        const vector<unsigned>& CalculateLadderStepFilling();

    private: // Internal methods.
        /// ???
        void CheckIntegrity() const;

        /// ???
        unsigned GetNumberOfZones() const;

        /// ???
        TLength GetZoneLimit(unsigned p_uZoneId) const;

        /// ???
        TLength GetStepLength() const;

        /// ???
        TLength GetFillingLength() const;

        /// ???
        void ApplyDefaultLadderStepFilling();

        /// ???
        void ApplyStepFillingOptimization();

    private: // Data members.
        /// ???
        TLength m_dInitialPosition;

        /// ???
        vector<TLength> m_vModuleLengths;

        /// ???
        vector<TLength> m_vZoneLeftLimits;

        /// ???
        vector<TLength> m_vZoneRightLimits;

        /// ???
        vector<unsigned> m_vNumberOfModules;

        /// ???
        vector<bool> m_vInitializedZones;
    };

    /// Provides a stream output for the Endcap class.
    ostream& operator<< (ostream& p_oStream, const Endcap& p_oEndcap);
}
