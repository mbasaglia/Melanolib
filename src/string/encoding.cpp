/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2015-2016 Mattia Basaglia
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "melanolib/string/encoding.hpp"
#include "melanolib/utils/functional.hpp"

#ifdef HAS_ICONV
#       include <iconv.h>
#endif

namespace melanolib {
namespace string {

static bool setup_iconv()
{
    // With the C locale, //TRANSLIT won't work properly
    static const char* const only_once = setlocale (LC_ALL, "");
    return only_once && only_once[0] != 'C';
}

bool Utf8Parser::has_iconv()
{
#ifdef HAS_ICONV
    return setup_iconv();
#else
    return false;
#endif
}

char Utf8Parser::to_ascii(uint32_t unicode)
{
    if ( unicode < 128 )
        return char(unicode);
    return to_ascii(encode(unicode));
}

char Utf8Parser::to_ascii(const std::string& utf8)
{
#ifdef HAS_ICONV
    setup_iconv();
    char ascii = '?';
    char * cutf8 = (char*)utf8.data();
    size_t cutf8size = utf8.size();
    char * cascii = &ascii;
    size_t casciisize = 1;

    iconv_t iconvobj  = iconv_open("ASCII//TRANSLIT", "UTF-8");
    iconv(iconvobj, &cutf8, &cutf8size, &cascii, &casciisize);
    iconv_close(iconvobj);
    return ascii;
#else
    return '?';
#endif
}

Utf8Parser::Utf8Parser(const std::string& string)
    : input(string)
{
}

Unicode Utf8Parser::next()
{
    while ( !input.eof() )
    {
        Byte byte = input.next();

        if ( byte_type(byte) == ByteType::ASCII )
        {
            return Unicode(std::string(1, byte), byte);
        }
        else if ( byte_type(byte) == ByteType::MultiHead )
        {
            std::string utf8;
            uint32_t unicode = 0;
            unsigned length = 0;

            utf8.push_back(byte);

            std::tie(length, unicode) = head_length_value(byte);

            while ( utf8.size() < length )
            {
                byte = input.next();

                if ( !input )
                    return Unicode("", 0);

                if ( byte_type(byte) != ByteType::MultiTail )
                {
                    input.unget();
                    return next();
                }

                utf8.push_back(byte);
                unicode <<= 6;
                unicode |= tail_value(byte);
            }
            return Unicode(utf8, unicode);

        }
    }
    return Unicode("", 0);
}

QuickStream::traits_type::int_type Utf8Parser::next_ascii(bool skip_utf8)
{
    while ( true )
    {
        Byte byte = input.next();

        if ( finished() )
            return QuickStream::traits_type::eof();

        if ( byte_type(byte) == ByteType::ASCII )
        {
            return byte;
        }
        else if ( !skip_utf8 && byte_type(byte) == ByteType::MultiHead )
        {
            input.unget();
            return byte;
        }
    }
}

std::string Utf8Parser::encode(uint32_t value)
{
    if ( value < 128 )
        return std::string(1, char(value));

    std::basic_string<Byte> s;

    Byte head = 0;
    while ( value )
    {
        s.push_back(tail_value(value) | leading_bit);
        value >>= 6;
        head <<= 1;
        head |= 1;
    }

    if ( tail_value(s.back()) > (1 << (7 - s.size())) )
    {
        head <<= 1;
        head |= 1;
        s.push_back(0);
    }

    s.back() |= head << (8 - s.size());

    return std::string(s.rbegin(), s.rend());
}

Unicode::Unicode(uint32_t point)
    : utf8_(Utf8Parser::encode(point)), point_(point)
{}

} // namespace string
} // namespace melanolib
