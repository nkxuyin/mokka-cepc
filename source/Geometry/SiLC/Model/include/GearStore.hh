/*! \file GearStore.hh
    \brief A definition of Silc::GearWriter and Silc::GearReader classes.

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
#include "gearimpl/GearParametersImpl.h"
#include "gear/GearMgr.h"

namespace Silc
{
    /// Writes parameters into the GEAR configuration.
    class GearWriter
    {
    private: // Internal functionality.
        template<typename ValueType>
        struct WriteImpl;

    public: // Basic methods.
        /// A default constructor.
        GearWriter() throw()
            : m_pGearParameters(new gear::GearParametersImpl())
        { }

        /// Transmits to a gear manager the collection of all written parameters.
        void Finalize(const string& p_sCollectionName, gear::GearMgr& p_oGearManager) throw()
        {
            p_oGearManager.setGearParameters(p_sCollectionName, m_pGearParameters);
        }

        /// Writes an entry into the GEAR parametes collection.
        template<typename ValueType>
        void Write(const string& p_sEntryName, const ValueType& p_oValue) throw(Exception)
        {
            WriteImpl<ValueType>::Write(*m_pGearParameters, p_sEntryName, p_oValue);
        }

        /// A correction factor which should be applied for all metrical values before the write operation.
        TLength MetricalUnitsCorrection() const throw()
        {
            return 1;
        }

        /// A correction factor which should be applied for all angular values before the write operation.
        TLength AngularUnitsCorrection() const throw()
        {
            return 1;
        }

    private: // Data members.
        /// A collection of the GEAR parameters.
        gear::GearParametersImpl* m_pGearParameters;
    };

    /// Reads parameters from the GEAR configuration.
    class GearReader
    {
    private: // Internal functionality.
        template<typename OutputType>
        struct ReadImpl;

        template<typename OutputType>
        struct ReadListImpl;

    public: // Basic methods.
        /// A default constructor.
        GearReader(const gear::GearParameters& p_oGearParameters) throw()
            : m_oGearParameters(p_oGearParameters)
        { }

        /// Reads an entry from the GEAR parameters collection.
        template<typename OutputType>
        void Read(const string& p_sEntryName, OutputType& p_oResult) const throw(Exception)
        {
            return ReadImpl<OutputType>::Read(m_oGearParameters, p_sEntryName, p_oResult);
        }

        /// Reads a list of entries from the GEAR parameters collection.
        /** Returns a number of read elements. */
        template<typename OutputType>
        unsigned ReadList(const string& p_sEntryName, vector<OutputType>& p_lResult, unsigned p_uExpectedCount = 0)
        const throw(Exception)
        {
            return ReadListImpl<OutputType>::ReadList(m_oGearParameters, p_sEntryName, p_lResult, p_uExpectedCount);
        }

        /// A correction factor which should be applied for all metrical values after the read operation.
        TLength MetricalUnitsCorrection() const
        {
            return 1;
        }

        /// A correction factor which should be applied for all angular values after the read operation.
        TLength AngularUnitsCorrection() const
        {
            return 1;
        }

    private:
        /// A collection of the GEAR parameters.
        const gear::GearParameters& m_oGearParameters;
    };

    /// An implementation of GearWriter::Write functionality which allows specialization.
    template<typename ValueType>
    struct GearWriter::WriteImpl
    {
        /// Writes an entry into the GEAR parametes collection.
        static void Write(gear::GearParametersImpl& p_oGearParameters, const string& p_sEntryName,
                          const ValueType& p_oValue) throw(Exception)
        {
            try
            {
                std::stringstream l_oStream;
                l_oStream.exceptions(std::ifstream::failbit);
                l_oStream << p_oValue;
                p_oGearParameters.setStringVal(p_sEntryName, l_oStream.str());
            }
            catch(std::ios_base::failure& e)
            {
                const string l_sMessage = "GearWriter::Write : An error was occured while saving '" + p_sEntryName
                                          + "' into the GEAR parameters collection.";
                throw Exception(l_sMessage, e);
            }
        }
    };

    /// A specialization of GearWriter::WriteImpl for the 'double' entry type.
    template<>
    struct GearWriter::WriteImpl<double>
    {
        /// Writes an entry into the GEAR parametes collection.
        static void Write(gear::GearParametersImpl& p_oGearParameters, const string& p_sEntryName,
                          const double& p_oValue) throw(Exception)
        {
            p_oGearParameters.setDoubleVal(p_sEntryName, p_oValue);
        }
    };

    /// A specialization of GearWriter::WriteImpl for the 'unsigned' entry type.
    template<>
    struct GearWriter::WriteImpl<unsigned>
    {
        /// Writes an entry into the GEAR parametes collection.
        static void Write(gear::GearParametersImpl& p_oGearParameters, const string& p_sEntryName,
                          const unsigned& p_oValue) throw(Exception)
        {
            p_oGearParameters.setIntVal(p_sEntryName, (int)p_oValue);
        }
    };

    /// A specialization of GearWriter::WriteImpl for the 'vector<double>' entry type.
    template<>
    struct GearWriter::WriteImpl< vector<double> >
    {
        /// Writes an entry into the GEAR parametes collection.
        static void Write(gear::GearParametersImpl& p_oGearParameters, const string& p_sEntryName,
                          const vector<double>& p_oValue) throw(Exception)
        {
            p_oGearParameters.setDoubleVals(p_sEntryName, p_oValue);
        }
    };

    /// A specialization of GearWriter::WriteImpl for the 'vector<unsigned>' entry type.
    template<>
    struct GearWriter::WriteImpl< vector<unsigned> >
    {
        /// Writes an entry into the GEAR parametes collection.
        static void Write(gear::GearParametersImpl& p_oGearParameters, const string& p_sEntryName,
                          const vector<unsigned>& p_oValue) throw(Exception)
        {
            vector<int> l_vIntVector(p_oValue.size());
            for(unsigned n = 0; n < p_oValue.size(); ++n)
                l_vIntVector[n] = (int) p_oValue[n];
            p_oGearParameters.setIntVals(p_sEntryName, l_vIntVector);
        }
    };

    /// A specialization of GearWriter::WriteImpl for the 'vector<string>' entry type.
    template<>
    struct GearWriter::WriteImpl< vector<string> >
    {
        /// Writes an entry into the GEAR parametes collection.
        static void Write(gear::GearParametersImpl& p_oGearParameters, const string& p_sEntryName,
                          const vector<string>& p_oValue) throw(Exception)
        {
            p_oGearParameters.setStringVals(p_sEntryName, p_oValue);
        }
    };

    /// An implementation of GearReader::Read functionality which allows specialization.
    template<typename OutputType>
    struct GearReader::ReadImpl
    {
        /// Reads an entry from the GEAR parameters collection.
        static void Read(const gear::GearParameters& p_oGearParameters, const string& p_sEntryName,
                         OutputType& p_oResult) throw(Exception)
        {
            try
            {
                const string& l_sValue = p_oGearParameters.getStringVal(p_sEntryName);
                p_oResult = std_ext::TypeConverter<OutputType>(l_sValue);
            }
            catch(gear::UnknownParameterException& e)
            {
                const string l_sMessage = "GearReader::Read : '" + p_sEntryName
                                          + "' was not found into the GEAR parameters collection.";
                throw Exception(l_sMessage, e);
            }
            catch(std::ios_base::failure& e)
            {
                const string l_sMessage = "GearReader::Read : A GEAR parameter '" + p_sEntryName
                                          + "' has an incorrect type.";
                throw Exception(l_sMessage, e);
            }
            catch(std::exception& e)
            {
                const string l_sMessage = "GearReader::Read : An error was occured while loading '" + p_sEntryName
                                          + "' from the GEAR parameters collection.";
                throw Exception(l_sMessage, e);
            }
        }
    };

    /// A specialization of GearReader::ReadImpl for the 'double' entry type.
    template<>
    struct GearReader::ReadImpl<double>
    {
        /// Reads an entry from the GEAR parameters collection.
        static void Read(const gear::GearParameters& p_oGearParameters, const string& p_sEntryName,
                         double& p_dResult) throw(Exception)
        {
            try
            {
                p_dResult = p_oGearParameters.getDoubleVal(p_sEntryName);
            }
            catch(gear::UnknownParameterException& e)
            {
                const string l_sMessage = "GearReader::Read : '" + p_sEntryName
                                          + "' was not found into the GEAR parameters collection.";
                throw Exception(l_sMessage, e);
            }
            catch(std::exception& e)
            {
                const string l_sMessage = "GearReader::Read : An error was occured while loading '" + p_sEntryName
                                          + "' from the GEAR parameters collection.";
                throw Exception(l_sMessage, e);
            }
        }
    };

    /// A specialization of GearReader::ReadImpl for the 'unsigned' entry type.
    template<>
    struct GearReader::ReadImpl<unsigned>
    {
        /// Reads an entry from the GEAR parameters collection.
        static void Read(const gear::GearParameters& p_oGearParameters, const string& p_sEntryName,
                         unsigned& p_uResult) throw(Exception)
        {
            try
            {
                p_uResult = (unsigned) p_oGearParameters.getIntVal(p_sEntryName);
            }
            catch(gear::UnknownParameterException& e)
            {
                const string l_sMessage = "GearReader::Read : '" + p_sEntryName
                                          + "' was not found into the GEAR parameters collection.";
                throw Exception(l_sMessage, e);
            }
            catch(std::exception& e)
            {
                const string l_sMessage = "GearReader::Read : An error was occured while loading '" + p_sEntryName
                                          + "' from the GEAR parameters collection.";
                throw Exception(l_sMessage, e);
            }
        }
    };

    /// An implementation of GearReader::Read functionality which allows specialization.
    template<typename OutputType>
    struct GearReader::ReadListImpl
    {
        /// Reads a list of entries from the GEAR parameters collection.
        /** Returns a number of read elements. */
        static unsigned ReadList(const gear::GearParameters& p_oGearParameters, const string& p_sEntryName,
                                 vector<OutputType>& p_lResult, unsigned p_uExpectedCount) throw(Exception)
        {
            try
            {
                const vector<string>& l_vsValues = p_oGearParameters.getStringVals(p_sEntryName);
                assert_ex(!p_uExpectedCount || p_uExpectedCount == l_vsValues.size(), Exception);
                for(vector<string>::const_iterator iter = l_vsValues.begin(); iter != l_vsValues.end(); ++iter)
                {
                    const OutputType l_oValue = std_ext::TypeConverter<OutputType>(*iter);
                    p_lResult.push_back(l_oValue);
                }
                return l_vsValues.size();
            }
            catch(gear::UnknownParameterException& e)
            {
                const string l_sMessage = "GearReader::ReadList : '" + p_sEntryName
                                          + "' was not found into the GEAR parameters collection.";
                throw Exception(l_sMessage, e);
            }
            catch(std::ios_base::failure& e)
            {
                const string l_sMessage = "GearReader::ReadList : A GEAR parameter '" + p_sEntryName
                                          + "' has an incorrect type.";
                throw Exception(l_sMessage, e);
            }
            catch(Exception& e)
            {
                const string l_sMessage = "GearReader::ReadList : A number of elements into '" + p_sEntryName
                                          + "' is not equal to the expected amount.";
                throw Exception(l_sMessage, e);
            }
            catch(std::exception& e)
            {
                const string l_sMessage = "GearReader::ReadList : An error was occured while loading '" + p_sEntryName
                                          + "' from the GEAR parameters collection.";
                throw Exception(l_sMessage, e);
            }
        }
    };

    /// A specialization of GearReader::ReadListImpl for the 'double' entry type.
    template<>
    struct GearReader::ReadListImpl<double>
    {
        /// Reads a list of entries from the GEAR parameters collection.
        /** Returns a number of read elements. */
        static unsigned ReadList(const gear::GearParameters& p_oGearParameters, const string& p_sEntryName,
                                 vector<double>& p_lResult, unsigned p_uExpectedCount) throw(Exception)
        {
            try
            {
                const vector<double>& l_vdValues = p_oGearParameters.getDoubleVals(p_sEntryName);
                assert_ex(!p_uExpectedCount || p_uExpectedCount == l_vdValues.size(), Exception);
                for(vector<double>::const_iterator iter = l_vdValues.begin(); iter != l_vdValues.end(); ++iter)
                {
                    const double l_dValue = *iter;
                    p_lResult.push_back(l_dValue);
                }
                return l_vdValues.size();
            }
            catch(gear::UnknownParameterException& e)
            {
                string l_sMessage = "GearReader::ReadList : '" + p_sEntryName
                                    + "' was not found into the GEAR parameters collection.";
                throw Exception(l_sMessage, e);
            }
            catch(Exception& e)
            {
                string l_sMessage = "GearReader::ReadList : A number of elements into '" + p_sEntryName
                                    + "' is not equal to the expected amount.";
                throw Exception(l_sMessage, e);
            }
            catch(std::exception& e)
            {
                string l_sMessage = "GearReader::ReadList : An error was occured while loading '" + p_sEntryName
                                    + "' from the GEAR parameters collection.";
                throw Exception(l_sMessage, e);
            }
        }
    };

    /// A specialization of GearReader::ReadListImpl for the 'unsigned' entry type.
    template<>
    struct GearReader::ReadListImpl<unsigned>
    {
        /// Reads a list of entries from the GEAR parameters collection.
        /** Returns a number of read elements. */
        static unsigned ReadList(const gear::GearParameters& p_oGearParameters, const string& p_sEntryName,
                                 vector<unsigned>& p_lResult, unsigned p_uExpectedCount) throw(Exception)
        {
            try
            {
                const vector<int>& l_viValues = p_oGearParameters.getIntVals(p_sEntryName);
                assert_ex(!p_uExpectedCount || p_uExpectedCount == l_viValues.size(), Exception);
                for(vector<int>::const_iterator iter = l_viValues.begin(); iter != l_viValues.end(); ++iter)
                {
                    const int l_iValue = *iter;
                    p_lResult.push_back((unsigned)l_iValue);
                }
                return l_viValues.size();
            }
            catch(gear::UnknownParameterException& e)
            {
                string l_sMessage = "GearReader::ReadList : '" + p_sEntryName
                                    + "' was not found into the GEAR parameters collection.";
                throw Exception(l_sMessage, e);
            }
            catch(Exception& e)
            {
                string l_sMessage = "GearReader::ReadList : A number of elements into '" + p_sEntryName
                                    + "' is not equal to the expected amount.";
                throw Exception(l_sMessage, e);
            }
            catch(std::exception& e)
            {
                string l_sMessage = "GearReader::ReadList : An error was occured while loading '" + p_sEntryName
                                    + "' from the GEAR parameters collection.";
                throw Exception(l_sMessage, e);
            }
        }
    };
}
