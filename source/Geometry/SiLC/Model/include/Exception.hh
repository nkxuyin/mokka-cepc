/*! \file Exception.hh
    \brief A definition of Silc::Exception class.

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

#include <string>
#include <exception>

namespace Silc
{
    /// An exception thrown by the SiLC project classes.
    class Exception : public std::exception
    {
    public: // Basic methods.
        /// A constructor with a message which describes the exception reason.
        Exception(std::string p_sMessage) throw();

        /// A constructor with a description message and with a reference to the innter exception.
        Exception(std::string p_sMessage, const std::exception& p_oInnerException) throw();

        /// A safe virtual destructor.
        virtual ~Exception() throw();

        /// Returns a pointer to a C-like string with the message which should explain the exception reason.
        virtual const char* what() const throw();

        /// Returns a reference to a STL string with the message which should explain the exception reason.
        virtual const std::string& GetMessage() const throw();

    private: // Data members.
        ///< A message which describes the exception reason.
        std::string m_sMessage;
    };
}
