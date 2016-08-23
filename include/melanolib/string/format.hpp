/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2016 Mattia Basaglia
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
 */
#ifndef MELANOLIB_STRING_FORMAT_HPP
#define MELANOLIB_STRING_FORMAT_HPP

#include <limits>

#include "melanolib/string/quickstream.hpp"
#include "melanolib/string/ascii.hpp"
#include "melanolib/math/math.hpp"

namespace melanolib {
namespace string {
namespace format {

struct FormatSpec
{
    enum class Alignment
    {
        Default = 0,
        Left    = '<',
        Right   = '>',
        Center  = '^',
        Sign    = '=',
    };

    enum class PositiveSign
    {
        None  = '-',
        Plus  = '+',
        Space = ' ',
    };

    char            fill_char = ' ';
    Alignment       alignment = Alignment::Default;
    PositiveSign    positive_sign = PositiveSign::None;
    bool            base_prefix = false;
    std::size_t     width = 0;
    std::size_t     precision = std::numeric_limits<std::size_t>::max();
    char            format = ' ';

    static constexpr bool is_aligmnent(int ch)
    {
        return ch == '<' || ch == '>' || ch == '^' || ch == '=';
    }

    /**
     * \see https://docs.python.org/2/library/string.html#format-specification-mini-language
     */
    static FormatSpec parse(QuickStream& stream);
};

template<class T>
    std::enable_if_t<!std::is_arithmetic<std::remove_reference_t<T>>::value, bool>
    format(const FormatSpec& spec, T&& value, std::ostream& out)
{
    return false;
}

bool format(const FormatSpec& spec, long long value, std::ostream& out);

bool format(const FormatSpec& spec, unsigned long long value, std::ostream& out);

bool format(const FormatSpec& spec, long double value, std::ostream& out);

template<class T>
    std::enable_if_t<
        std::is_integral<std::remove_reference_t<T>>::value &&
        std::is_unsigned<std::remove_reference_t<T>>::value,
        bool>
    format(const FormatSpec& spec, T value, std::ostream& out)
{
    return format(spec, (unsigned long long)value, out);
}

template<class T>
    std::enable_if_t<
        std::is_integral<std::remove_reference_t<T>>::value &&
        std::is_signed<std::remove_reference_t<T>>::value,
        bool>
    format(const FormatSpec& spec, T value, std::ostream& out)
{
    return format(spec, (long long)value, out);
}
template<class T>
    std::enable_if_t<std::is_floating_point<std::remove_reference_t<T>>::value, bool>
    format(const FormatSpec& spec, T value, std::ostream& out)
{
    return format(spec, (long double)value, out);
}

bool format(const FormatSpec& spec, std::string value, std::ostream& out);

inline bool format(const FormatSpec& spec, const char* value, std::ostream& out)
{
    return format(spec, std::string(value), out);
}

inline bool format(const FormatSpec& spec, char value, std::ostream& out)
{
    if ( spec.format != 's' && spec.format != ' ' && spec.format != 'c' )
        return format(spec, std::string(1, value), out);
    else
        return format(spec, int(value), out);
}

inline bool printf(QuickStream& input, std::ostream& output)
{
    while ( !input.eof() )
    {
        char next = input.next();
        if ( next != '%' )
        {
            output.put(next);
        }
        else if ( input.peek() == '%' )
        {
            input.ignore();
            output.put('%');
        }
        else
        {
            return false;
        }
    }
    return true;
}

template<class Head, class... Args>
    bool printf(QuickStream& input, std::ostream& output, Head&& head, Args&&... args)
{
    while ( !input.eof() )
    {
        char next = input.next();
        if ( next != '%' )
        {
            output.put(next);
        }
        else if ( input.peek() == '%' )
        {
            input.ignore();
            output.put('%');
        }
        else
        {
            if ( !format(FormatSpec::parse(input), std::forward<Head>(head), output) )
                return false;
            return printf(input, output, std::forward<Args>(args)...);
        }
    }
    return sizeof...(args) == 0;
}

template<class... Args>
    void printf(const std::string& input, std::ostream& output, Args&&... args)
{
    QuickStream in_stream(input);
    return printf(in_stream, output, std::forward<Args>(args)...);
}

template<class... Args>
    std::string sprintf(const std::string& input, Args&&... args)
{
    QuickStream in_stream(input);
    std::ostringstream out_stream;
    if ( printf(in_stream, out_stream, std::forward<Args>(args)...) )
        return out_stream.str();
    return {};
}

} // namespace format
} // namespace string
} // namespace melanolib
#endif // MELANOLIB_STRING_FORMAT_HPP
