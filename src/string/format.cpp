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
 *
 */
#include "melanolib/string/format.hpp"

namespace melanolib {
namespace string {
namespace format {

FormatSpec FormatSpec::parse(QuickStream& stream)
{
    FormatSpec out;
    if ( stream.eof() )
        return out;

    auto next_char = stream.next();
    static const auto eof = QuickStream::traits_type::eof();

    if ( is_aligmnent(next_char) )
    {
        out.alignment = Alignment(next_char);
        next_char = stream.next();
        if ( next_char == eof )
            return out;
    }
    else
    {
        auto maybe_align = stream.peek();
        if ( is_aligmnent(maybe_align) )
        {
            stream.ignore();
            out.fill_char = next_char;
            out.alignment = Alignment(maybe_align);
            next_char = stream.next();
            if ( next_char == eof )
                return out;
        }
    }

    if ( next_char == '-' || next_char == '+' || next_char == ' ' )
    {
        out.positive_sign = PositiveSign(next_char);
        next_char = stream.next();
        if ( next_char == eof )
            return out;
    }

    if ( next_char == '#' )
    {
        out.base_prefix = true;
        next_char = stream.next();
        if ( next_char == eof )
            return out;
    }

    if ( next_char == '0' )
    {
        out.fill_char = next_char;
        out.alignment = Alignment::Sign;
        next_char = stream.next();
        if ( next_char == eof )
            return out;
    }

    if ( ascii::is_digit(next_char) )
    {
        stream.unget();
        std::string width = stream.get_until([](char c){return !ascii::is_digit(c);});
        if ( !width.empty() )
        {
            out.width = std::stoul(width);
            if ( !ascii::is_digit(stream.peek_back()) )
                stream.unget();
        }
        next_char = stream.next();
        if ( next_char == eof )
            return out;
    }

    if ( next_char == ',' )
    {
        // skip it
        next_char = stream.next();
        if ( next_char == eof )
            return out;
    }

    if ( next_char == '.' )
    {
        std::string width = stream.get_until([](char c){return !ascii::is_digit(c);});
        if ( !width.empty() )
        {
            out.precision = std::stoul(width);
            if ( !ascii::is_digit(stream.peek_back()) )
                stream.unget();
        }
        else
        {
            out.precision = 0;
        }
        next_char = stream.next();
        if ( next_char == eof )
            return out;
    }

    out.format = next_char;
    return out;
}

static std::string ull_to_string(unsigned long long value, int base, bool caps)
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

static bool get_int_base(const FormatSpec& spec, int& base, std::string& prefix)
{
    if ( spec.format == 'd' || spec.format == 'n' ||
         spec.format == ' ' || spec.format == 'i' )
    {
        base = 10;
    }
    else if ( spec.format == 'b' )
    {
        if ( spec.base_prefix )
            prefix += "0b";
        base = 2;
    }
    else if ( spec.format == 'o' )
    {
        if ( spec.base_prefix )
            prefix += "0o";
        base = 8;
    }
    else if ( spec.format == 'x' || spec.format == 'X' )
    {
        if ( spec.base_prefix )
            prefix += "0x";
        base = 16;
    }
    else
    {
        return false;
    }
    return true;
}

static void pad_int(const FormatSpec& spec, const std::string& prefix,
                    const std::string& mantissa, std::ostream& out)
{
    std::string padding;
    auto string_length = mantissa.size() + prefix.size();
    if ( string_length < spec.width )
        padding = std::string(spec.width - string_length, spec.fill_char);

    if ( spec.alignment == FormatSpec::Alignment::Left )
        out << prefix << mantissa << padding;
    else if ( spec.alignment == FormatSpec::Alignment::Center )
        out << padding.substr(0, padding.size()/2) << prefix << mantissa << padding.substr(padding.size()/2);
    else if ( spec.alignment == FormatSpec::Alignment::Sign )
        out << prefix << padding << mantissa;
    else
        out << padding << prefix << mantissa;
}

bool format(const FormatSpec& spec, long long value, std::ostream& out)
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
    if ( !get_int_base(spec, base, prefix) )
        return false;

    std::string mantissa = ull_to_string(value, base, ascii::is_upper(spec.format));
    pad_int(spec, prefix, mantissa, out);

    return true;
}

bool format(const FormatSpec& spec, unsigned long long value, std::ostream& out)
{
    char fmt = ascii::to_lower(spec.format);
    if ( fmt == 'e' || fmt == 'g' || fmt == 'f' || fmt == '%' )
        return format(spec, double(value), out);

    std::string prefix;

    if ( spec.positive_sign == FormatSpec::PositiveSign::Plus )
        prefix = "+";
    else if ( spec.positive_sign == FormatSpec::PositiveSign::Space )
        prefix = " ";

    int base;
    if ( !get_int_base(spec, base, prefix) )
        return false;

    std::string mantissa = ull_to_string(value, base, ascii::is_upper(spec.format));
    pad_int(spec, prefix, mantissa, out);

    return true;
}

bool format(const FormatSpec& spec, std::string value, std::ostream& out)
{
    if ( spec.format != 's' && spec.format != ' ' && spec.format != 'c' )
        return false;

    if ( value.size() > spec.precision )
        value.resize(spec.precision);

    std::string padding;

    if ( value.size() < spec.width )
        padding = std::string(spec.width - value.size(), spec.fill_char);

    if ( spec.alignment == FormatSpec::Alignment::Right )
        out << padding << value;
    else if ( spec.alignment == FormatSpec::Alignment::Center )
        out << padding.substr(0, padding.size()/2) << value << padding.substr(padding.size()/2);
    else
        out << value << padding;

    return true;
}

bool format(const FormatSpec& spec, long double value, std::ostream& out)
{
    if ( spec.format == 'd' || spec.format == 'n' || spec.format == 'i' ||
         spec.format == 'o' || spec.format == 'b' || spec.format == 'x' ||
         spec.format == 'X' )
    {
        if ( value < 0 )
            return format(spec, (long long)value, out);
        return format(spec, (unsigned long long)value, out);
    }
    char fmt = ascii::to_lower(spec.format);
    if ( fmt != 'e' && fmt != 'g' && fmt != 'f' && fmt != '%' )
        return false;

    /// \todo % format

    auto precision = out.precision();
    if ( spec.precision != std::numeric_limits<std::size_t>::max() )
        out.precision(spec.precision);
    else
        out.precision(6);

    auto width = out.width();
    out.width(spec.width);

    auto fill = out.fill();
    out.fill(spec.fill_char);

    auto flags = out.flags();

    if ( ascii::is_upper(spec.format) )
        out.setf(std::ios::uppercase);
    else
        out.unsetf(std::ios::uppercase);

    if ( fmt == 'f' )
        out.setf(std::ios::fixed, std::ios::floatfield);
    else if ( fmt == 'e' )
        out.setf(std::ios::scientific, std::ios::floatfield);
    else if ( fmt == 'g' )
        out.unsetf(std::ios::floatfield);

    switch ( spec.alignment )
    {
        case FormatSpec::Alignment::Center:
            /// \todo
            break;
        case FormatSpec::Alignment::Left:
            out.setf(std::ios::left, std::ios::adjustfield);
            break;
        case FormatSpec::Alignment::Default:
        case FormatSpec::Alignment::Right:
            out.setf(std::ios::right, std::ios::adjustfield);
            break;
        case FormatSpec::Alignment::Sign:
            out.setf(std::ios::internal, std::ios::adjustfield);
            break;
    }

    switch ( spec.positive_sign )
    {
        case FormatSpec::PositiveSign::None:
            out.unsetf(std::ios::showpos);
            break;
        case FormatSpec::PositiveSign::Plus:
            out.setf(std::ios::showpos);
            break;
        case FormatSpec::PositiveSign::Space:
            /// \todo
            break;
    }

    out << value;

    out.flags(flags);
    out.fill(fill);
    out.width(width);
    out.precision(precision);
    return !out.fail();
}

} // namespace format
} // namespace string
} // namespace melanolib
