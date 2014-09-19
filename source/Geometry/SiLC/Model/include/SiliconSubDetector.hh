/*! \file SiliconSubDetector.hh
    \brief A definition of Silc::SiliconSubDetector class.

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

#include "Module.hh"

namespace Silc
{
    /// A class which represents a silicon sub-detector.
    class SiliconSubDetector : public IAssemblable
    {
    private: // Type definitions.
        /// A vector of module prototypes.
        typedef vector< P<Module> > ModulePrototypeArray;

    public: // Basic methods.
        /// The Silc::SiliconSubDetector constructor.
        SiliconSubDetector();

        /// Returns a number of different module prototypes used by the sub-detector.
        unsigned GetNumberOfModulePrototypes() const;

        /// Sets a number of different module prototypes used by the sub-detector.
        void SetNumberOfModulePrototypes(unsigned p_uNumberOfModulePrototypes);

        /// Setups a module prototype for the specified zone ID.
        void InitializeModulePrototype(unsigned p_uZoneId,
                                       const Module::SensorArray& p_oSensorArray,
                                       const Module::ChipArray& p_oChipArray,
                                       const Module::Support& p_oModuleSupport);

        /// Returns a reference to a module prototype for a zone with the specified ID.
        Module& GetModulePrototype(unsigned p_uZoneId);

        /// Returns a constant reference to a module prototype for a zone with the specified ID.
        const Module& GetModulePrototype(unsigned p_uZoneId) const;

    protected: // Virtual methods.
        /// Returns a pointer to a new module prototype.
        virtual P<Module> MakeModulePrototype(const Module::SensorArray& p_oSensorArray,
                                              const Module::ChipArray& p_oChipArray,
                                              const Module::Support& p_oModuleSupport);

    public: // IAssemblable implementations.
        /// @copydoc Silc::IAssemblable::Assemble()
        /// If the sub-detector is not assembled, assembles the module prototypes for all zones.
        /// Otherwise, does nothing.
        virtual void Assemble();

    private: // Data members.
        /// Indicates if the sub-detector is assembled or not.
        bool m_bIsAssembled;

        /// A vector of the module prototypes for each zone.
        ModulePrototypeArray m_oModulePrototypes;
    };
}
