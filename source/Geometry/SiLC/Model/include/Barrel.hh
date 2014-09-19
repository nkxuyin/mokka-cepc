/*! \file Barrel.hh
    \brief A definition of Silc::Barrel class.

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
#include "GlobalNodeId.hh"

namespace Silc
{
    /// An abstract class which represents a silicon sub-detector in the shape of the barrel.
    /** Local coordinate system definition:
         - (X,Y,Z)-basis is left handed.
         - (0,0,0) -interaction point
         - Z-axis coresponds to the beam direction
         - Y-axis point to the vertical direction
         - X-axis point to the horizontal direction
    */
    class Barrel : public virtual SiliconSubDetector
    {
    public:
        /// The type of the barrel.
        typedef string BarrelType;

        /// A descriptor to identify a module in a super-module.
        class ModuleDescriptor;

        /// A distribution of the modules into a super-module.
        class ModulesDistribution;

        class LayerDescriptor;
        typedef map< unsigned, P<LayerDescriptor> > LayerDescriptorCollection;

    public: // Basic methods.
        /// The Silc::Barrel constructor.
        Barrel();

    public: // Abstract methods.
        /// Returns a type of the barrel.
        virtual const BarrelType& GetBarrelType() const = 0;

    public:
        //// Barrel Name: SIT/SET etc ...
        void SetBarrelName(std::string p_sBarrelName);
        std::string GetBarrelName() const;
        /// Number of faces associated to the barrel
        unsigned GetNbFace() const;
        void SetNbFace(unsigned p_uNbFace);
        /// Get the length of the whole barrel
        TLength GetLength() const;
        void SetLength(TLength p_dLength);
        /// About the number of layer
        virtual unsigned GetNbLayers() const = 0;
        void Initialize();
        /// Parameter for the splitting along z axis

        TLength GetInitialInnerRadius() const;
        void SetInitialInnerRadius(TLength p_dInitialInnerRadius);
        /// Inner radius of the silicon barrel detector - Min ref the distance between the z axis and the supermodule
        TLength GetInnerRadiusMin() const;
        void SetInnerRadiusMin(TLength p_dInnerRadiusMin);

        void SetSensitiveRadius(unsigned p_uLayerId, TLength p_dRadius);
        void SetSupportRadius(TLength p_dRadius);

        void SetBarrelShape(TLength p_dRadius, TLength p_dLength);
        bool IsSupportEnable() const;

        void SetBarrelSupportParameters(string p_sBarrelSupportMaterial, TLength p_dBarrelSupportThickness);
        const string& GetBarrelSupportMaterial() const;
        TLength GetBarrelSupportThickness() const;
        void EnableSupport(bool p_bEnable);

        void SetAdjustement(string p_sRadialAdjustement, string p_sLongitudinalAdjustement);

        std::string GetRadialAdjustment() const;
        std::string GetLongitudinalAdjustment() const;

        TCuboidSize GetEffectiveModuleSize(unsigned p_uLayerId);
        TCuboidSize GetMaxSModuleSize();
        TPlaneDistribution GetEffectiveNbModuleSensor(unsigned p_uLayerId);

        unsigned GetNbOfTilesAlongPhi(unsigned p_uLayerId) const;
        unsigned GetNbOfTilesAlongZ(unsigned p_uLayerId) const;
        void SetSuperModuleNbOfTiles(unsigned p_uLayerId, const TPlaneDistribution& p_auNbTiles);
        const TPlaneDistribution& GetSuperModuleNbTiles(unsigned p_uLayerId) const;

        /// Set the dimension of the super module
        void SetSuperModuleSize(unsigned p_uLayerId, const TCuboidSize& p_adSuperModuleSize);
        /// Get the dimension of the super module
        const TCuboidSize& GetSuperModuleSize(unsigned p_uLayerId) const;

        /// Get the module direction
        TAngle GetModuleDirection(unsigned p_uLayerId) const;
        TAngle GetModuleFace(unsigned p_uLayerId) const;

        /// Set the module Rotation for after spatial positionning
        void SetModuleOrientation(unsigned p_dLayerId, TAngle p_dAngle2Z, TAngle p_dFaceToBeam);

        /// Returns a reference for an object which defines a modules distribution in the barrel.
        ModulesDistribution& GetModulesDistribution();

        /// Returns a constant reference for an object which defines a modules distribution in the barrel.
        const ModulesDistribution& GetModulesDistribution() const;

        TAngle GetInitialFaceAngle() const;
        TAngle GetFaceRotationAngle(unsigned p_uFaceId) const;
        TLength GetLayerRadius(unsigned l_uLayerId) const;
        TLength GetLayerThickness(unsigned l_uLayerId) const;
        TLength GetLayerWidth(unsigned l_uLayerId) const;
        TLength GetLayerLength(unsigned l_uLayerId) const;
        TLength GetMeanSupportRadiationLength(unsigned l_uLayerId) const;
        TLength GetSensitiveVolumeRadius(unsigned l_uLayerId) const;
        TLength GetSensitiveVolumeThickness(unsigned l_uLayerId) const;
        TLength GetSensitiveVolumeWidth(unsigned l_uLayerId) const;
        TLength GetSensitiveVolumeRadiationLength(unsigned l_uLayerId) const;

        TVector TransformVectorToLocal(const TVector& p_vGlobalVector, GlobalNodeId p_uSensorId,
                                       unsigned p_uLayerId) const;

        TVector TransformVectorToGlobal(const TVector& p_vLocalVector, GlobalNodeId p_uSensorId,
                                        unsigned p_uLayerId) const;

        GlobalNodeId FindGlobalNodeId(const TVector& p_vGlobalVector, unsigned p_uLayerIdShift) const;

        const LayerDescriptor& GetLayerDescriptor(unsigned p_uLayerId) const;
        void SetLayerDescriptor(unsigned p_uLayerId, TLength p_dFirstSurfaceShift, TLength p_dSensitiveSurfaceShift,
                                TLength p_dThickness, TLength p_dSensitiveSurfaceThickness);

        void SetSensitiveDetectorNames(const string& p_sPrefix);
        const set<string>& GetSensitiveDetectorNames() const;

    private:
        ///===============================================
        /// Barrel information
        ///===============================================
        /// General Information of the barrel
        ///< Id of the Barrel
        unsigned m_uBarrelId;
        ///< Number of layer in the barrel
        unsigned m_uNbLayers;
        ///< Name given for the barrel: ex Sit 1, Sit 2 etc ...
        std::string m_sBarrelName;
        ///< Enable/Disable the barrel support
        bool m_bEnableSupport;
        ///< Generic material for generic support
        std::string m_sSupportMaterial;
        /// Generic size for the thickness of the barrel support
        TLength m_dSupportThickness;
        /// Space between the two layers
        TLength m_dSpacer;

        ///< Number of face in both option (mean cylinder or polygon)
        ///< for cylinder it is calculted as function of the radius and module size;
        ///< for polygon, it is the number of face of the polygon.
        unsigned m_uNbFaces;

        TLength m_dInitialInnerRadius;
        ///< Minimum radius; narmal distance with Z axis
        TLength m_dMinRadius;

        ///< Radius of the sensitive layers of the barrel layer
        vector <TLength> m_vdSensitiveRadius;
        ///< Radius of the barrel support of the barrel layer
        vector <TLength> m_vdSupportRadius;

        ///< Length of the detector
        TLength m_dLength;
        ///< how to adjust the parameters => shrink or expand in the radial axis
        string m_sRadialAdjustement;
        ///< how to adjust the parameters => shrink or expand in the longitudinal
        string m_sLongitudinalAdjustement;

        TPlaneDimension m_adSuperModuleSize;

        ///===============================================
        /// Module information
        ///===============================================
        ///< Direction of the module along the z axis - ie rotation in the xOz plan
        vector <TAngle> m_vdModuleDirection;
        ///< Rotation of the module along the z axis - ie the active edge is face to the beam
        vector <TAngle> m_vdModuleFace;

        ///===============================================
        /// Related to the topology of the barrel
        ///===============================================
        /// Size of the super module
        vector<TCuboidSize> m_avdSuperModuleSize;
        /// (along Phi, along Z)
        vector<TPlaneDistribution> m_avuSuperModuleNbTiles;

        /// A pointer to an object which defines a modules distribution in the barrel.
        P<ModulesDistribution> m_oModulesDistribution;

        LayerDescriptorCollection m_oLayerDescriptors;

        set<string> m_SensitiveDetectorNames;

        bool m_bIsAssembled;
    };

    class Barrel:: ModuleDescriptor
    {
    public:
        /// A module coordinate rotation.
        typedef TAngle RotationIndicator;

        /// Indicates that module does not rotated.
        static const RotationIndicator m_dZeroRotation;

        /// Indicates that module does rotated at Pi/2.
        static const RotationIndicator m_dOrtogonalRotation;

        // A relative ratation of module in a super-module.
        TSolidAngle m_adModuleRotation;

        // A relative position of module in a super-module.
        TPosition m_adModulePosition;

        /// Index for the LayerID
        unsigned int m_uLayerID;

    public:
        /// The constructor
        ModuleDescriptor(unsigned p_uLayerID, RotationIndicator p_bModuleRotation, const TSolidAngle& p_adModuleAngle,
                         const TPosition& p_adModulePosition);
    };

    class Barrel::ModulesDistribution
    {
    public:
        /// Type to define the collection of the module
        typedef vector<ModuleDescriptor>  DescriptorCollection;
        /// An iterator for the distribution
        typedef std::vector<ModuleDescriptor>::const_iterator Iterator;

    private:
        /// Map the Collection of module according the layerID
        typedef map<unsigned, DescriptorCollection> LayerIndex;

    public:
        /// Adds a module described by 'p_oDescriptor' to the barrel.
        void Add(const ModuleDescriptor& p_oDescriptor);
        /// Removes all modules from the
        void Clear();
        /// Returns an iterator referring to the first element in the distribution container.
        Iterator Begin() const;
        /// Returns an iterator referring to the past-the-end element in the distribution container.
        Iterator End() const ;
        /// Select the module according the layer Id
        const DescriptorCollection& SelectLayer(unsigned p_uLayerId) const;

    private:
        /// A container for descriptors of all barrels modules.
        DescriptorCollection m_oDescriptors;
        /// Layer Index
        LayerIndex m_oLayerIndex;
    };

    class Barrel::LayerDescriptor
    {
    public:
        TLength FirstSurfaceRadius;
        TLength SensitiveSurfaceRadius;
        TLength Thickness;
        TLength SensitiveSurfaceThickness;

    public:
        LayerDescriptor(TLength p_dFirstSurfaceRadius, TLength p_dSensitiveSurfaceRadius, TLength p_dThickness,
                        TLength p_dSensitiveSurfaceThickness);
    };

    /// Provides a stream output for the SilcBarrelParam class.
    ostream& operator<<(ostream& p_oStream, const Barrel& p_oBarrel);
}
