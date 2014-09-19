/*! \file Module.hh
    \brief A definition of Silc::Module class.

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

#include "Sensors.hh"

namespace Silc
{
    /// Represents a module with silicon detectors.
    /** Local coordinate system definition:
         - (X,Y,Z)-basis is left handed.
         - all coordinates of the points into the volume of the module is positive or zero
         - Z-axis coresponds to the thickness of the module
    */
    class Module : public IAssemblable
    {
    public:
        class SensorArray;
        class ChipArray;
        class Support;

    public: // Basic methods.
        /// The Silc::Module constructor.
        Module(const SensorArray& p_oSensorArray, const ChipArray& p_oChipArray,
               const Support& p_oModuleSupport) throw();

        /// The Silc::Module copy constructor.
        Module(const Module& p_oModule) throw();

        /// Returns an array of sensors.
        const SensorArray& GetSensorArray() const throw();

        /// Returns an array of sensors.
        SensorArray& GetSensorArray() throw();

        /// Returns an array of chips.
        const ChipArray& GetChipArray() const throw();

        /// Returns an array of chips.
        ChipArray& GetChipArray() throw();

        /// Returns a module support.
        const Support& GetSupport() const throw();

        /// Returns a module support.
        Support& GetSupport() throw();

        /// Returns a size of the module.
        TCuboidSize GetModuleSize() const throw();

        /// ???
        const string& GetSensitiveDetectorName() const;

        /// ???
        void SetSensitiveDetectorName(const string& p_sSensitiveDetectorName);

    public: // IAssemblable implementations.
        /// @copydoc IAssemblable::Assemble()
        virtual void Assemble();

    private: // Data members.
        /// The array of sensors.
        P<SensorArray> m_pSensorArray;

        /// The array of chips.
        P<ChipArray> m_pChipArray;

        /// The module support.
        P<Support> m_pSupport;

        /// The size of the module.
        TCuboidSize m_vModuleSize;

        /// ???
        string m_sSensitiveDetectorName;
    };

    /// Represents an array of sensors.
    class Module::SensorArray
    {
    public:
        /// The Silc::Module::SensorArray constructor.
        SensorArray(P<Sensor> p_pSensorPrototype, const TPlaneDistribution& p_vNumberOfSensors,
                    const TPlaneDimension& p_vGapBetweenSensors) throw(std_ext::out_of_range_exception);

        /// Returns a sensor prototype.
        const Sensor& GetSensorPrototype() const throw();

        /// Returns a sensor prototype.
        Sensor& GetSensorPrototype() throw();


        /// Returns a number of sensors on the module for each dimension.
        const TPlaneDistribution& GetNumberOfSensors() const throw();

        /// Returns a gap between sensors.
        const TPlaneDimension& GetGapBetweenSensors() const throw();

    private:
        /// The sensor prototype.
        P<Sensor> m_pSensorPrototype;

        /// The number of sensors on the module for each direction.
        TPlaneDistribution m_vNumberOfSensors;

        /// The gap between sensors.
        TPlaneDimension m_vGapBetweenSensors;
    };

    /// Represents an array of readout chips.
    class Module::ChipArray
    {
    public:
        /// The Silc::Module::ChipArray constructor.
        ChipArray(const TCuboidSize& p_vChipSize, const TPlaneDistribution& p_vChipDistribution,
                  unsigned p_uNumberOfChannelsPerChip) throw(std_ext::out_of_range_exception);

        /// Returns a dimensions of the readout chip.
        const TCuboidSize& GetChipSize() const throw();

        /// Returns a plane distribution of the readout chips.
        const TPlaneDistribution& GetChipDistribution() const throw();

        /// Returns a number of channels per readout chip.
        unsigned GetNumberOfChannelsPerChip() const throw();

        /// Returns a number of readout chips.
        unsigned GetNumberOfChips() const throw();

        /// Returns a total number of channels in all readout chips in the array.
        unsigned GetTotalNumberOfChannels() const throw();

    private:
        /// The dimensions of the readout chip.
        TCuboidSize m_vChipSize;

        /// The plane distribution of the readout chips.
        TPlaneDistribution m_vChipDistribution;

        /// The number of readout channels per chip.
        unsigned m_uNumberOfChannelsPerChip;
    };

    /// Represents a module support.
    class Module::Support : public MaterialObject
    {
    public:
        /// The Silc::Module::Support constructor.
        Support(TLength p_dThickness, TLength p_dWidth, TLength p_dStandoffFromEdge, const string& p_sMaterialName,
                bool p_bEnabled = true) throw(std_ext::out_of_range_exception);

        /// Returns a thickness of the support.
        TLength GetThickness() const throw();

        /// Returns a widht of the support.
        TLength GetWidth() const throw();

        /// Returns a standoff from an edge for the support.
        TLength GetStandoffFromEdge() const throw();

        /// Indicates if the support is enabled.
        bool IsEnabled() const throw();

    private:
        /// The thickness of the support.
        TLength m_dThickness;

        /// The width of the support.
        TLength m_dWidth;

        /// The standoff from an edge for the support.
        TLength m_dStandoffFromEdge;

        /// The indicator of an enable state for the support.
        bool m_bEnabled;
    };

    /// Provides a stream output for the Module class.
    ostream& operator<<(ostream& p_oOutputStream, const Module& p_oModule) throw(std::ios_base::failure);
}
