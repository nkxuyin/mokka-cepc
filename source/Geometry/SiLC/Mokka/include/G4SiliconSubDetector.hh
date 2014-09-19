/*! \file G4SiliconSubDetector.hh
    \brief A definition of Silc::G4SiliconSubDetector class.

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
#include "G4Module.hh"

namespace Silc
{
    /// A GEANT4 representation of a silicon sub-detector.
    class G4SiliconSubDetector : public virtual SiliconSubDetector
    {
    private: // Type definitions.
        /// A correspondance between a Module pointer and the original G4Module pointer.
        typedef map< Module*, G4Module* > ModuleOriginMap;

        /// A correspondance between constant a Module pointer and the original G4Module pointer.
        typedef map< const Module*, const G4Module* > ModuleConstOriginMap;

    public: // Basic methods.
        /// Returns a reference to a GEANT4 module prototype for a zone with the specified ID.
        G4Module& GetG4ModulePrototype(unsigned p_uZoneId);

        /// Returns a constant reference to a GEANT4 module prototype for a zone with the specified ID.
        const G4Module& GetG4ModulePrototype(unsigned p_uZoneId) const;

    public: // Silc::SiliconSubDetector overrides.
        /// @copydoc Silc::SiliconSubDetector::MakeModulePrototype()
        virtual P<Module> MakeModulePrototype(const Module::SensorArray& p_oSensorArray,
                                              const Module::ChipArray& p_oChipArray,
                                              const Module::Support& p_oModuleSupport);

    private: // Data members.
        /// The correspondance between a Module pointer and the original G4Module pointer.
        ModuleOriginMap m_mModuleOrigins;

        /// The correspondance between a constant Module pointer and the original G4Module pointer.
        ModuleConstOriginMap m_mModuleConstOrigins;
    };
}
