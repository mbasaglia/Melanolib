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

namespace detail {

template<class Int>
    std::string uint_to_string(Int value, int base, bool caps)
{
    if ( value == 0 )
        return "0";
    std::string result;
    result.reserve(math::ceil(math::log(value, base)));
    while ( value )
    {
        char digit = value % base;
        if ( digit < 10 )
            digit += '0';
        else if ( caps )
            digit += 'A' - 10;
        else
            digit += 'a' - 10;

        result.push_back(digit);
        value /= base;
    }

    std::reverse(result.begin(), result.end());

    return result;
}

bool get_int_base(const FormatSpec& spec, int& base, std::string& prefix);

void pad_num(const FormatSpec& spec, const std::string& prefix,
             const std::string& mantissa, std::ostream& out);

template<class Float>
    int extract_exponent(Float value, int base)
    {
        if ( value == 0 )
            return 0;
        return math::ceil(math::log(value, base));
    }

template<class Float>
    std::string extract_digits(Float value, int base, std::size_t precision, int exponent)
{
    std::string mantissa;
    Float q = value / math::pow(base, exponent);
    int digit = 0;
    for ( std::size_t i = 0; i < precision + math::max(1, exponent); i++ )
    {
        Float scaled = q * base;
        digit = math::floor(scaled);
        q = scaled - digit;
        mantissa += '0' + digit;
    }

    if ( digit >= 5 )
    {
        for ( auto it = mantissa.rbegin(); it != mantissa.rend(); ++it )
        {
            *it += 1;
            if ( *it > '9' )
                *it = '0';
            else
                break;
        }
    }
    return mantissa;
}

void format_body(char format, const std::string& mantissa,
                 std::size_t precision, int exponent, std::string& body);

} // namespace detail

template<class T>
    std::enable_if_t<!std::is_arithmetic<std::remove_reference_t<T>>::value, bool>
    format(const FormatSpec& spec, T&& value, std::ostream& out)
{
    return !!(out << std::forward<T>(value));
}

template<class Float>
    std::enable_if_t<std::is_floating_point<std::remove_reference_t<Float>>::value, bool>
    format(const FormatSpec& spec, Float value, std::ostream& out);

template<class T>
    std::enable_if_t<std::is_integral<std::remove_reference_t<T>>::value, bool>
    format(const FormatSpec& spec, T value, std::ostream& out)
{
    char fmt = ascii::to_lower(spec.format);
    if ( fmt == 'e' || fmt == 'g' || fmt == 'f' || fmt == '%' )
        return format(spec, (long double)value, out);

    std::string prefix;
    if ( value < 0 )
    {
        prefix = "-";
        value = -value;
    }
    else if ( spec.positive_sign == FormatSpec::PositiveSign::Plus )
    {
        prefix = "+";
    }
    else if ( spec.positive_sign == FormatSpec::PositiveSign::Space )
    {
        prefix = " ";
    }

    int base;
    if ( !detail::get_int_base(spec, base, prefix) )
        return false;

    std::string mantissa = detail::uint_to_string(value, base, ascii::is_upper(spec.format));
    detail::pad_num(spec, prefix, mantissa, out);

    return true;
}

template<class Float>
    std::enable_if_t<std::is_floating_point<std::remove_reference_t<Float>>::value, bool>
    format(const FormatSpec& spec, Float value, std::ostream& out)
{
    if ( spec.format == 'd' || spec.format == 'i' || spec.format == 'o' ||
         spec.format == 'b' || spec.format == 'x' || spec.format == 'X' )
    {
        if ( value < 0 )
            return format(spec, (long long)value, out);
        return format(spec, (unsigned long long)value, out);
    }

    char fmt = ascii::to_lower(spec.format);
    if ( fmt != 'e' && fmt != 'g' && fmt != 'f' && fmt != 'n' && fmt != '%' )
        return false;

    bool negative = false;
    std::string body;
    std::string suffix;

    if ( fmt == '%' )
    {
        suffix = "%";
        fmt = 'f';
        value *= 100;
    }

    if ( std::isnan(value) )
    {
        body = "NaN";
    }
    else if ( std::isinf(value) )
    {
        body = "Inf";
        negative = value < 0;
    }
    else
    {
        if ( value < 0 )
        {
            negative = true;
            value = -value;
        }

        static const int base = 10;
        int exponent = detail::extract_exponent(value, base);
        std::size_t precision = spec.precision;
        if ( precision == std::numeric_limits<std::size_t>::max() )
            precision = 6;
        std::string mantissa = detail::extract_digits(value, base, precision, exponent);
        detail::format_body(spec.format, mantissa, precision, exponent, body);
    }

    std::string prefix;
    if ( negative )
        prefix = "-";
    else if ( spec.positive_sign == FormatSpec::PositiveSign::Plus )
        prefix = "+";
    else if ( spec.positive_sign == FormatSpec::PositiveSign::Space )
        prefix = " ";

    detail::pad_num(spec, prefix, body+suffix, out);
    return true;
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
