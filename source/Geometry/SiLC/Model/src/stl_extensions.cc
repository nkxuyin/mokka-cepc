/*! \file stl_extensions.cc
    \brief Implements all features defined in stl_extensions.hh.

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

#include "stl_extensions.hh"

#include <cstdarg>
#include <cstdio>

using namespace std;

namespace std_ext
{

    std::string format(std::string format_string, ...)
    {
        static const size_t BUFF_SIZE = 4 * 1024;
        va_list ap;
        char text[BUFF_SIZE] = {0,};

        va_start(ap, format_string);
        vsnprintf(text, BUFF_SIZE, format_string.c_str(), ap);
        va_end(ap);

        return string (text);
    }

    void split(const std::string& str, std::vector<std::string>& substrings, const std::string& delimiters)
    {
        string::size_type begin_position = str.find_first_not_of( delimiters, 0 );
        string::size_type end_position = str.find_first_of( delimiters, begin_position );

        while( end_position != string::npos || begin_position != string::npos )
        {
            substrings.push_back( str.substr( begin_position, end_position - begin_position ) );
            begin_position = str.find_first_not_of( delimiters, end_position );
            end_position = str.find_first_of( delimiters, begin_position );
        }
    }
}
