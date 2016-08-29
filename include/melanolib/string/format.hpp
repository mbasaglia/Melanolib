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
#include "melanolib/utils/type_utils.hpp"

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

    bool type_auto() const
    {
        return format == ' ';
    }

    bool type_string() const
    {
        return format == 's';
    }

    bool type_char() const
    {
        return format == 'c';
    }

    bool type_int() const
    {
        return format == 'd' || format == 'i' || format == 'o' ||
               format == 'b' || format == 'x' || format == 'X';
    }

    bool type_float() const
    {
        char fmt = ascii::to_lower(format);
        return fmt == 'e' || fmt == 'g' || fmt == 'f' || fmt == '%';
    }

    bool type_auto_number() const
    {
        return ascii::to_lower(format) == 'n';
    }

    bool type_numeric() const
    {
        return type_float() || type_int();
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

/**
 * \brief Rounds a decimal string
 * \returns \b true if it overflows
 */
bool round_mantissa(std::string& mantissa, std::size_t at);

template<class Float>
    std::string extract_digits(Float value, int base, std::size_t precision, int& exponent)
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

    if ( round_mantissa(mantissa, mantissa.size() - 1) )
    {
        exponent += 1;
        mantissa.insert(mantissa.begin(), '1');
    }

    return mantissa;
}

void format_body(char format, std::string mantissa, std::size_t precision, int exponent, std::string& body);

template<class Generator>
    std::enable_if_t<
        IsCallable<Generator, bool, const std::string&, const FormatSpec&, std::ostream&>::value,
        bool
    >
    format_callback(Generator&& generator, const std::string& key,
                    const FormatSpec& spec, std::ostream& output)
{
    return generator(key, spec, output);
}

template<class Generator>
    std::enable_if_t<
        !IsCallable<Generator, bool, const std::string&, const FormatSpec&, std::ostream&>::value &&
        IsCallableAnyReturn<Generator, const std::string&, const FormatSpec&, std::ostream&>::value,
        bool
    >
    format_callback(Generator&& generator, const std::string& key,
                    const FormatSpec& spec, std::ostream& output)
{
    generator(key, spec, output);
    return true;
}

template<class Generator>
    std::enable_if_t<
        !IsCallableAnyReturn<Generator, const std::string&, const FormatSpec&, std::ostream&>::value &&
        !IsCallable<Generator, void, const std::string&>::value &&
        IsCallableAnyReturn<Generator, const std::string&>::value,
        bool
    >
    format_callback(Generator&& generator, const std::string& key,
                    const FormatSpec& spec, std::ostream& output)
{
    return format_item(spec, generator(key), output);
}

template<class Generator>
    std::enable_if_t<
        !IsCallableAnyReturn<Generator, const std::string&, const FormatSpec&, std::ostream&>::value &&
        !IsCallableAnyReturn<Generator, const std::string&>::value,
        bool
    >
    format_callback(Generator&& generator, const std::string& key,
                    const FormatSpec& spec, std::ostream& output)
{
    return format_item(spec, generator[key], output);
}

} // namespace detail

template<class T>
    std::enable_if_t<!std::is_arithmetic<std::remove_reference_t<T>>::value, bool>
    format_item(const FormatSpec& spec, T&& value, std::ostream& out)
{
    return !!(out << std::forward<T>(value));
}

template<class Float>
    std::enable_if_t<std::is_floating_point<std::remove_reference_t<Float>>::value, bool>
    format_item(const FormatSpec& spec, Float value, std::ostream& out);

template<class T>
    std::enable_if_t<std::is_integral<std::remove_reference_t<T>>::value, bool>
    format_item(const FormatSpec& spec, T value, std::ostream& out)
{
    if ( spec.type_float() )
        return format_item(spec, (long double)value, out);

    if ( !spec.type_auto() && !spec.type_auto_number() && !spec.type_int() )
        return false;

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
    format_item(const FormatSpec& spec, Float value, std::ostream& out)
{
    if ( spec.type_int() )
    {
        if ( value < 0 )
            return format_item(spec, (long long)value, out);
        return format_item(spec, (unsigned long long)value, out);
    }

    if ( !spec.type_auto() && !spec.type_auto_number() && !spec.type_float() )
        return false;

    bool negative = false;
    std::string body;
    std::string suffix;

    if ( spec.format == '%' )
    {
        suffix = "%";
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

bool format_item(const FormatSpec& spec, std::string value, std::ostream& out);

inline bool format_item(const FormatSpec& spec, const char* value, std::ostream& out)
{
    return format_item(spec, std::string(value), out);
}

inline bool format_item(const FormatSpec& spec, char value, std::ostream& out)
{
    if ( spec.type_string() || spec.type_char() || spec.type_auto() )
        return format_item(spec, std::string(1, value), out);
    else
        return format_item(spec, int(value), out);
}

inline bool printf(QuickStream& input, std::ostream& output)
{
    bool too_many = false;
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
            FormatSpec::parse(input);
            too_many = true;
        }
    }
    return !too_many;
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
            if ( !format_item(FormatSpec::parse(input), std::forward<Head>(head), output) )
                return false;
            return printf(input, output, std::forward<Args>(args)...);
        }
    }
    // too many arguments
    return false;
}

template<class... Args>
    bool printf(const std::string& input, std::ostream& output, Args&&... args)
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

template<class Callback>
    bool format(QuickStream& input, std::ostream& output, Callback&& callback)
{
    bool ok = true;
    while ( !input.eof() )
    {
        char next = input.next();
        if ( next != '{' )
        {
            output.put(next);
        }
        else
        {
            std::string name = input.get_until([](char c){ return c == ':' || c == '}'; });
            FormatSpec spec;
            if ( input.peek_back() == ':' )
            {
                spec = FormatSpec::parse(input);
                if ( input.peek() != '}' )
                    return false;
                input.ignore();
            }
            if ( !detail::format_callback(callback, name, spec, output) )
                ok = false;
        }
    }
    return ok;
}

template<class Callback>
    bool format(const std::string& input, std::ostream& output, Callback&& callback)
{
    QuickStream in_stream(input);
    return format(in_stream, std::forward<Callback>(callback), output);
}

template<class Callback>
    std::string sformat(const std::string& input, Callback&& callback)
{
    QuickStream in_stream(input);
    std::ostringstream out_stream;
    if ( format(in_stream, out_stream, std::forward<Callback>(callback)) )
        return out_stream.str();
    return {};
}

} // namespace format
} // namespace string
} // namespace melanolib
#endif // MELANOLIB_STRING_FORMAT_HPP
