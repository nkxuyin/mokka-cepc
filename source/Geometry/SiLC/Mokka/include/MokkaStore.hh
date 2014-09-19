/*! \file MokkaStore.hh
    \brief A definition of Silc::MokkaReader and Silc::MokkaWriter class.

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

#include "G4Globals.hh"

namespace Silc
{
    /// Reads parameters from the Mokka geometry environment.
    class MokkaReader
    {
    public: // Basic methods.
        /// A constructor.
        MokkaReader(const CGAGeometryEnvironment& p_oEnvironment) throw()
            : m_oEnvironment(p_oEnvironment)
        { }

        /// Reads an entry from the Mokka geometry environment.
        /** If entry name will not be found in the environment, Mokka will abort execution. */
        template<typename OutputType>
        void Read(const string& p_sEntryName, OutputType& p_oResult) const throw(Exception)
        {
            try
            {
                G4String l_sValue = m_oEnvironment.GetParameterAsString(p_sEntryName);
                p_oResult = std_ext::TypeConverter<OutputType>(l_sValue);
            }
            catch(ios_base::failure& e)
            {
                const string l_sMessage = "MokkaReader::Read : A Mokka parameter '" + p_sEntryName
                                          + "' has an incorrect type.";
                throw Exception(l_sMessage, e);
            }
        }

        /// Reads a list of entries from the Mokka geometry environment.
        /** Returns a number of read elements.
            All entries is stored as a single string with the spetial delimiters.
            If entry name will not be found in the environment, Mokka will abort execution. */
        template<typename OutputType>
        unsigned ReadList(const string& p_sEntryName, vector<OutputType>& p_lResult, unsigned p_uExpectedCount = 0)
        const throw(Exception)
        {
            static const string LIST_DELIMITERS = " ;";
            try
            {
                G4String l_sValuesString = m_oEnvironment.GetParameterAsString(p_sEntryName);
                vector<string> l_vValues;
                std_ext::split(l_sValuesString, l_vValues, LIST_DELIMITERS);
                assert_ex(!p_uExpectedCount || p_uExpectedCount == l_vValues.size(), Exception);
                for(vector<string>::const_iterator l_pIter = l_vValues.begin(); l_pIter != l_vValues.end(); ++l_pIter)
                {
                    OutputType l_oValue = std_ext::TypeConverter<OutputType>(*l_pIter);
                    p_lResult.push_back(l_oValue);
                }
                return l_vValues.size();
            }
            catch(ios_base::failure& e)
            {
                const string l_sMessage = "MokkaReader::ReadList : A Mokka parameter '" + p_sEntryName
                                          + "' has an incorrect format.";
                throw Exception(l_sMessage, e);
            }
            catch(Exception& e)
            {
                const string l_sMessage = "MokkaReader::ReadList : A number of elements into '" + p_sEntryName
                                          + "' is not equal to the expected amount.";
                throw Exception(l_sMessage, e);
            }
        }

        /// A correction factor which should be applied for all metrical values after the read operation.
        TLength MetricalUnitsCorrection() const throw()
        {
            return millimeter;
        }

        /// A correction factor which should be applied for all angular values after the read operation.
        TLength AngularUnitsCorrection() const throw()
        {
            return degree;
        }

    private: // Data members.
        /// A reference to the Mokka geometry environment.
        const CGAGeometryEnvironment& m_oEnvironment;
    };

    /// Writes parameters to the Mokka environment.
    class MokkaWriter
    {
    public:
        /// Writes an entry to the Mokka global model parameters collection.
        template<typename ValueType>
        void Write(const string p_sEntryName, const ValueType& p_oValue) const throw(Exception)
        {
            try
            {
                stringstream l_oStream;
                l_oStream.exceptions(ifstream::failbit);
                l_oStream << p_oValue;
                assert_ex(Control::globalModelParameters != nullptr, Exception);
                (*Control::globalModelParameters)[p_sEntryName] = l_oStream.str();
            }
            catch(ios_base::failure& e)
            {
                const string l_sMessage = "MokkaWriter::Write : En error was occured while writing '" + p_sEntryName
                                          + "' to the Mokka global model parameters collection.";
                throw Exception(l_sMessage, e);
            }
        }

        /// A correction factor which should be applied for all metrical values before the write operation.
        TLength MetricalUnitsCorrection() const throw()
        {
            return 1.0 / millimeter;
        }

        /// A correction factor which should be applied for all angular values before the write operation.
        TLength AngularUnitsCorrection() const throw()
        {
            return 1.0 / degree;
        }
    };
}
