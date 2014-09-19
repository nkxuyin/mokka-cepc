/*! \file Exception.cc
    \brief An implementation of Silc::Exception class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "Exception.hh"

#include <sstream>

using namespace std;
using namespace Silc;

Exception::Exception(string p_sMessage) throw()
    : m_sMessage(p_sMessage)
{
}

Exception::Exception(string p_sMessage, const exception& p_oInnerException) throw()
{
    stringstream l_oStream;
    l_oStream << p_oInnerException.what() << endl << p_sMessage << endl;
    m_sMessage = l_oStream.str();
}

Exception::~Exception() throw()
{
}

const char* Exception::what() const throw()
{
    return m_sMessage.c_str();
}

const string& Exception::GetMessage() const throw()
{
    return m_sMessage;
}
