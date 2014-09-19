/*! \file stl_extensions.hh
    \brief Defines some usefull extensions for the STL functionality.

Copyright (c) 2005-2011 Konstantin Androsov <konstantin.androsov@gmail.com>.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <cmath>

/// Null pointer constant.
#define nullptr 0

/*! \def diagnostics(value)
    \brief Provides a debug diagnostics.

    Provides a functionality to output a debug diagnostics messages to the standard log. If VERBOSE_DEBUG is not
    defined, this macros will do nothing.
*/
/*! \def d_message(value)
    \brief Provides an output of debug messages.

    Provides a functionality to output a debug messages to the standard log. If VERBOSE_DEBUG is not defined,
    this macros will do nothing.
*/
#ifdef VERBOSE_DEBUG
#define diagnostics(value)                                                                    \
        std::clog << "DEBUG MESSAGE: " << #value << " = " << (value) << ". Location: file '"  \
                  << __FILE__ << "' line #" << __LINE__ << "." << std::endl;
#define d_message(value) \
        std::clog << __FILE__ << ": " << (value) << std::endl;
#else
#define diagnostics(value)
#define d_message(value)
#endif

/*! \def assert_ex(value, exception_class)
    \brief Provides an assertion functionality for the debug purposes.

    If assertion fails, an 'exception_class' exception will be thrown.
*/
/*! \def assert(value)
    \brief #assert_ex with std_ext::out_of_range_exception exception class.
*/
#ifdef SAFE_DEBUG
#define assert_ex(value, exception_class)                                                                         \
        if(!(value))                                                                                              \
        {                                                                                                         \
            std::stringstream exception_message;                                                                  \
            exception_message << "EXCEPTION: Pre-condition '" << #value << "' is not fulfilled. The '"            \
                              << #exception_class << "' was thrown. Location: file '" << __FILE__ << "' line #"   \
                              << __LINE__ << ".";                                                                 \
            throw exception_class(exception_message.str());                                                       \
        }
#undef assert
#define assert(value) assert_ex(value, std_ext::out_of_range_exception)
#else
#define assert_ex(value, exception_class)
#undef assert
#define assert(value)
#endif

/// Some usefull extensions for the STL functionality.
namespace std_ext
{
    /// C++-style analog of sprintf.
    /** Makes an output string under the control of the 'format_string' which specifies how the subsequent arguments
        shell be converted for output. Only C-like types are supported as the subsequent arguments. */
    std::string format(std::string format_string, ...);

    /// Splits string into substrings using delimiters.
    void split(const std::string& str, std::vector<std::string>& substrings, const std::string& delimiters = " ");

    /// Calculates the square of value.
    template<typename T> inline T sqr(T value) throw()
    {
        return value * value;
    }

    /// Compares floating point numbers with maximal allowed relative error.
    /*! For additional information see: http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm */
    template<typename T> bool fcmp(T value1, T value2, T max_relative_error)
    {
        if (value1 == value2)
            return true;
        const T delta = value1 - value2;
        const T relative_error = fabs( fabs(value2) > fabs(value1) ? delta / value2 : delta / value1 );
        return relative_error <= max_relative_error;
    }
    template<typename T> bool fcmp(T value1, T value2)
    {
        static const T max_relative_error = 0.00001;
        return fcmp(value1, value2, max_relative_error);
    }

    namespace internals
    {
        class basic_ptr
        {
        protected:
            unsigned* count;
            basic_ptr(unsigned* p_count) throw() : count(p_count) {}
            inline void inc() throw()
            {
                ++(*count);
            }
            inline void dec() throw()
            {
                --(*count);
            }
            inline void clone(const basic_ptr& p) throw()
            {
                count = p.count;
            }
            virtual ~basic_ptr() throw() {}
        };
    }

    /// Smart pointer with referce counter.
    template<typename T>
    class P : public internals::basic_ptr
    {
    private:
        T* value;
    public:
        P() throw() : basic_ptr(new unsigned(0) ), value(nullptr)
        {
            inc();
        }

        P(T* p_value) throw() : basic_ptr(new unsigned(0) ), value(p_value)
        {
            inc();
        }

        P(const P& p) throw() : basic_ptr(nullptr), value(p)
        {
            clone(p);
            inc();
        }

        template<typename X>
        P(const P<X>& p) throw() : basic_ptr(nullptr), value(p)
        {
            clone(p);
            inc();
        }

        ~P() throw()
        {
            destroy();
        }

        P& operator= (const P& p) throw()
        {
            copy(p, p);
            return *this;
        }

        template<class X>
        P& operator= (const P<X>& p) throw()
        {
            copy(p, p);
            return *this;
        }

        inline T* operator ->() const throw()
        {
            return value;
        }

        inline operator T*() const throw()
        {
            return value;
        }

    private:
        void destroy() throw()
        {
            dec();
            if(*count)
                return;
            delete count;
            delete value;
        }

        void copy(const basic_ptr& p, T *p_value) throw()
        {
            if(value == p_value)
                return;
            destroy();
            value = p_value;
            clone(p);
            inc();
        }
    };

    /// Represents a generic exception with a descriptive message.
    class ext_exception : public std::exception
    {
    public: // Basic methods.
        /// A default constructor.
        ext_exception() throw() : m_message("") {}

        /// A common constructor.
        ext_exception(const std::string& p_message) throw() : m_message(p_message) {}

        /// A virtual destructor.
        virtual ~ext_exception() throw() {}

        /// Returns an explicative message about the exception causes.
        const std::string& message() throw()
        {
            return m_message;
        }

        /// Returns a null terminated character sequence containing a generic description of the exception.
        virtual const char* what() const throw()
        {
            return m_message.c_str();
        }

    private: // Data members.
        /// An explicative message about the exception causes.
        const std::string m_message;
    };

    /// Represents en exception which is thrown when some parameter is out of range.
    class out_of_range_exception : public ext_exception
    {
    public: // Basic methods.
        /// A default constructor.
        out_of_range_exception() throw() : ext_exception() {}

        /// A common constructor.
        out_of_range_exception(const std::string& message) throw() : ext_exception(message) {}

        /// A virtual destructor.
        virtual ~out_of_range_exception() throw() {}
    };

    /// Represents en exception which is thrown when call of some operation is not correct for the current object state.
    class invalid_operation_exception : public ext_exception
    {
    public: // Basic methods.
        /// A default constructor.
        invalid_operation_exception() throw() : ext_exception() {}

        /// A common constructor.
        invalid_operation_exception(const std::string& message) throw() : ext_exception(message) {}

        /// A virtual destructor.
        virtual ~invalid_operation_exception() throw() {}
    };

    /// Converts a value from it string representation to the specified output type.
    /** This function uses standard input and output operators to perform a conversion. */
    template<typename OutputType>
    OutputType TypeConverter(const std::string& p_sValue) throw(std::ios_base::failure)
    {
        std::stringstream l_oStream;
        l_oStream.exceptions(std::ios_base::failbit);
        l_oStream << p_sValue;
        OutputType result;
        l_oStream >> result;
        return result;
    }
}
