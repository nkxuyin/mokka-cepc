/*! \file MaterialObject.hh
    \brief A definition of Silc::MaterialObject class.

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

#include "Silc_Globals.hh"

namespace Silc
{
    /// Represents a material object.
    class MaterialObject
    {
    public: // Basic methods.
        /// The Silc::MaterialObject constructor.
        MaterialObject(const string& p_sMaterialName) throw();

        /// A virtual destructor.
        virtual ~MaterialObject() throw() {}

        /// Returns a name of the material from which the object is built.
        const string& GetMaterialName() const throw();

        /// Sets the basic material properties which could be used during the reconstruction.
        void SetMaterialProperties(/*TEnergyLinearDensity p_dEnergyLoss,*/ TLength p_dRadiationLength)
        throw(std_ext::out_of_range_exception);

        /// Indicates if a definition of the object contains material properties.
        bool HasMaterialProperties() const throw();

//        /// Returns an energy loss per length unit for a particule which will pass through the object.
//        TEnergyLinearDensity GetEnergyLoss() const throw(std_ext::invalid_operation_exception);

        /// Returns a radiation length.
        TLength GetRadiationLength() const throw(std_ext::invalid_operation_exception);

    private: // Data members.
        /// The name of the material from which the object is build.
        string m_sMaterialName;

        /// The indicator of the material properties presence.
        bool m_bHasMaterialProperties;

//        /// The energy loss per length unit for a particle which will pass through the object.
//        TEnergyLinearDensity m_dEnergyLoss;

        /// The radiation length.
        TLength m_dRadiationLength;
    };
}
