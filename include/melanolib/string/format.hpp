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

/**
 * \brief Format specification
 */
struct FormatSpec
{
    /**
     * \brief Alignment direction
     */
    enum class Alignment
    {
        Default = 0,    ///< Natural alignment for the given type/format
        Left    = '<',  ///< Text to the left, padding to the right
        Right   = '>',  ///< Text to the right, padding to the left
        Center  = '^',  ///< Text in the middle, padding on both sides
        Sign    = '=',  ///< (For numbers) padding between sign (or prefix) and the rest
    };

    /**
     * \brief Visibility of the sign for positive numbers
     */
    enum class PositiveSign
    {
        None  = '-',    ///< None
        Plus  = '+',    ///< Shows a "+"
        Space = ' ',    ///< Shows a single space
    };

    char            fill_char = ' ';                    ///< Character used for padding
    Alignment       alignment = Alignment::Default;     ///< Alignment directrion
    PositiveSign    positive_sign = PositiveSign::None; ///< Visibility of the positive sign
    bool            base_prefix = false;                ///< Whether to show base prefix for integers
    std::size_t     width = 0;                          ///< Minimum width (enables padding)
    /// Maximum width for string and floating point precision
    std::size_t     precision = std::numeric_limits<std::size_t>::max();
    char            format = ' ';                       ///< Format specifier

    /**
     * \brief The format represents automatic/default formatting
     */
    bool type_auto() const
    {
        return format == ' ';
    }

    /**
     * \brief The format represents a string
     */
    bool type_string() const
    {
        return format == 's';
    }

    /**
     * \brief The format represents a character
     */
    bool type_char() const
    {
        return format == 'c';
    }

    /**
     * \brief The format represents an integer number
     */
    bool type_int() const
    {
        return format == 'd' || format == 'i' || format == 'o' ||
               format == 'b' || format == 'x' || format == 'X';
    }

    /**
     * \brief The format represents a floating-point number
     */
    bool type_float() const
    {
        char fmt = ascii::to_lower(format);
        return fmt == 'e' || fmt == 'g' || fmt == 'f' || fmt == '%';
    }

    /**
     * \brief The format represents number in its natural format
     */
    bool type_auto_number() const
    {
        return ascii::to_lower(format) == 'n';
    }

    /**
     * \brief The format represents any number
     */
    bool type_numeric() const
    {
        return type_float() || type_int() || type_auto_number();
    }

    /**
     * \brief Parses a format specifier
     * \see https://docs.python.org/2/library/string.html#format-specification-mini-language
     */
    static FormatSpec parse(QuickStream& stream);
};

namespace detail {

    /**
     * \brief Converts a non-negative integer into a string
     * \param value The value to be converted
     * \param base  Base to represent the number in
     * \param caps  Whether the alphabetic values should be uppercase
     * \returns \p value formatted in base \p base
     * \pre \p value >= 0 && \p base > 1 && \p base <= 36
     */
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

    /**
     * \brief Extracts the numeric base from \p spec
     * \param[in]  spec     Format specification
     * \param[out] base     Resulting base
     * \param[out] prefix   If \p spec requires it, the base prefix will be
     *                      appended to it
     * \returns \b true on success
     */
    bool get_int_base(const FormatSpec& spec, int& base, std::string& prefix);

    /**
     * \brief Inserts a number into \p out, padding it as from \p spec
     * \param spec      Format specification
     * \param prefix    Prefix, might contain sign and number base
     *                  (Depending on \p spec, might be separated from
     *                  \p mantissa by some padding)
     * \param mantissa  Main body of the number, already formatted.
     * \param out       Output stream
     */
    void pad_num(const FormatSpec& spec, const std::string& prefix,
                const std::string& mantissa, std::ostream& out);

    /**
     * \brief Finds the exponent for \p value in the given \p base
     * \param value Value to be examined
     * \param base  Numerical base to represent \p value
     * \pre \p base > 1
     * \returns The maximum value so that
     *      \f$ \lfloor \mbox{base}^{\mbox{exponent}} \rfloor \le \mbox{value} \f$
     */
    template<class Float>
        int extract_exponent(Float value, int base)
        {
            if ( value == 0 )
                return 0;
            return math::floor(math::log(value, base));
        }

    /**
    * \brief Rounds a decimal string
    * \returns \b true if it overflows
    */
    bool round_mantissa(std::string& mantissa, std::size_t next_pos, bool next_rounds = false);

    /**
     * \brief Extracts the mantissa from a floating point value
     * \param value     Value to format
     * \param base      Numerical base (Only <= 10 currently supported)
     * \param n_digits  Number of digits to extract
     * \param exponent  Maximum value so that
     *      \f$ \lfloor \mbox{base}^{\mbox{exponent}} \rfloor \le \mbox{value} \f$
     * \note After rounding \p exponent might be increased.
     * \return The mantissa as a decimal string
     */
    template<class Float>
        std::string extract_digits(Float value, int base, std::size_t n_digits, int& exponent)
    {
        std::string mantissa;
        Float q = value / math::pow(base, exponent + 1);
        int digit = 0;
        for ( std::size_t i = 0; i < n_digits; i++ )
        {
            Float scaled = q * base;
            digit = math::floor(scaled);
            q = scaled - digit;
            mantissa += '0' + digit;
        }

        if ( round_mantissa(mantissa, mantissa.size(), q >= 0.5) )
        {
            exponent += 1;
            mantissa.insert(mantissa.begin(), '1');
        }

        return mantissa;
    }

    /**
     * \brief Formats a floating point body
     *
     * ie: the numerical value of the float, without sign or padding.
     *
     * \param format    Format as from an FormatSpec
     * \param mantissa  Significant digits as a string
     * \param precision Precision/number of decimal places to display
     * \param exponent  Exponent to applu to the \p mantissa
     * \param body      Output string.
     */
    void format_body(char format, std::string mantissa, std::size_t precision, int exponent, std::string& body);

    /**
     * \brief format() callback for functions, it forwards its arguments to
     * \p generator.
     * \param generator Object used to access the replacements
     * \param key       Name of the value to expand
     * \param spec      Format specification
     * \param output    Output stream
     * \returns the return value from \p generator, casted to \b bool.
     */
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

    /**
     * \brief format() callback for functions, it forwards its arguments to
     * \p generator.
     *
     * This overload is selected when \p generator does not return a value
     * convertible to bool.
     *
     * \param generator Object used to access the replacements
     * \param key       Name of the value to expand
     * \param spec      Format specification
     * \param output    Output stream
     * \returns \b true
     */
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

    /**
     * \brief format() callback for functions, it assumes generator(key) will
     * return the value to format.
     * \param generator Object used to access the replacements
     * \param key       Name of the value to expand
     * \param spec      Format specification
     * \param output    Output stream
     * \returns \b true on success
     */
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

    /**
     * \brief format() callback for non-functions (tries subscript)
     * \param generator Object used to access the replacements
     * \param key       Name of the value to expand
     * \param spec      Format specification
     * \param output    Output stream
     * \returns \b true on success
     */
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

    /**
     * \brief Whether the float "g" format should use "e" (instead of "f")
     */
    inline static bool g_uses_exp_notation(int exponent, int precision)
    {
        return -4 > exponent || exponent >= int(precision);
    }

} // namespace detail

/**
 * \brief Default formatting
 * \param spec  Format specification
 * \param value Value to format
 * \param out   Output stream
 * \return \b true on success
 *
 * This is the fallback if no other overload is found,
 * it uses the stream operator<< for the given value.
 */
template<class T>
    std::enable_if_t<!std::is_arithmetic<std::remove_reference_t<T>>::value, bool>
    format_item(const FormatSpec& spec, T&& value, std::ostream& out)
{
    return !!(out << std::forward<T>(value));
}

template<class Float>
    std::enable_if_t<std::is_floating_point<std::remove_reference_t<Float>>::value, bool>
    format_item(const FormatSpec& spec, Float value, std::ostream& out);

/**
 * \brief Formats a single integer
 * \param spec  Format specification
 * \param value Value to format
 * \param out   Output stream
 * \return \b true on success
 *
 * If \p spec contains an float format, \p value will be truncated and
 * formatted as a float.
 */
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

/**
 * \brief Formats a single floating point number
 * \param spec  Format specification
 * \param value Value to format
 * \param out   Output stream
 * \return \b true on success
 *
 * If \p spec contains an integer format, \p value will be truncated and
 * formatted as an integer.
 *
 * Not a number values are formatted as "NaN", infinity as "Inf".
 */
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

        bool exp_notation =  ascii::to_lower(spec.format) == 'e';

        if ( std::tolower(spec.format) == 'g' || std::tolower(spec.format) == 'n' )
            exp_notation = detail::g_uses_exp_notation(exponent, precision);

        std::size_t digit_count = precision;
        if ( exp_notation )
            digit_count += 1;
        else if ( exponent + 1 >= -int(digit_count) )
            digit_count += exponent + 1;
        else
            digit_count = 0;

        std::string mantissa = detail::extract_digits(value, base, digit_count, exponent);
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

/**
 * \brief Formats a single string
 * \param spec  Format specification
 * \param value Value to format
 * \param out   Output stream
 * \return \b true on success
 */
bool format_item(const FormatSpec& spec, std::string value, std::ostream& out);

/**
 * \brief Formats a single string
 * \param spec  Format specification
 * \param value Value to format
 * \param out   Output stream
 * \return \b true on success
 */
inline bool format_item(const FormatSpec& spec, const char* value, std::ostream& out)
{
    return format_item(spec, std::string(value), out);
}

/**
 * \brief Formats a single character
 * \param spec  Format specification
 * \param value Value to format
 * \param out   Output stream
 * \return \b true on success
 *
 * By default uses string formatting, if \p spec contains an integer format,
 * it will be formatted as an integer.
 */
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

/**
 * \brief Formats a template
 * \param input     Template string
 * \param output    Output stream
 * \param args      Values to expand
 * \return \b true on success
 *
 * Normal character remain unchanged, format specifiers are introduced by %.
 * The percent sing can be doubled to be escaped.
 *
 * eg: "%.2f%%" will expand to something like "74.39%"
 *
 * The format specification syntax is based on the Python str.format() function.
 * \see https://docs.python.org/2/library/string.html#formatspec
 */
template<class... Args>
    bool printf(const std::string& input, std::ostream& output, Args&&... args)
{
    QuickStream in_stream(input);
    return printf(in_stream, output, std::forward<Args>(args)...);
}

/**
 * \brief printf into a string
 * \see printf()
 * \return The formatted string (empty string on error)
 */
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
        if ( (next == '{' || next == '}') && input.peek() == next )
        {
            input.ignore();
            output.put(next);
        }
        else if ( next != '{' )
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

/**
 * \brief Formats a template
 * \param input Template string
 * \param output Output stream
 * \param callback Object used to extract the value of a placeholder
 * \return \b true on success
 *
 * Normal character remain unchanged, placeholders are sorrounded by braces.
 * Braces can be doubled to be escaped.
 *
 * eg: "{foo} {{bar}}"  Will expand foo but not bar.
 *                      Double braces will become single.
 * eg: "{foo:#02x}"     Adds some explicit formatting to foo.
 *
 * The template syntax is based on the Python str.format() function.
 * \see https://docs.python.org/2/library/string.html#formatstrings
 */
template<class Callback>
    bool format(const std::string& input, std::ostream& output, Callback&& callback)
{
    QuickStream in_stream(input);
    return format(in_stream, std::forward<Callback>(callback), output);
}

/**
 * \brief Format into a string
 * \see format()
 * \return The formatted string (empty string on error)
 */
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
