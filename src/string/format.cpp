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
            if ( stream.peek_back() != '.' )
                stream.unget();
        }
        next_char = stream.next();
        if ( next_char == eof )
            return out;
    }

    out.format = next_char;
    return out;
}


namespace detail {
void pad_num(const FormatSpec& spec, const std::string& prefix,
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

void format_body(char format, const std::string& mantissa,
                 std::size_t precision, int exponent, std::string& body)
{
    char fmt = std::tolower(format);
    bool showfrac = true;
    exponent -= 1;

    if ( fmt == 'g' || fmt == 'n' )
    {
        if ( -4 <= exponent && exponent < int(precision) )
            fmt = 'f';
        else
            fmt = 'e';

        auto last_nonzero = mantissa.find_last_not_of('0');
        if ( last_nonzero == std::string::npos )
        {
            showfrac = false;
        }
        else if ( last_nonzero + 1 < precision )
        {
            precision = last_nonzero + 1;
            if ( int(last_nonzero) <= exponent || (fmt == 'e' && last_nonzero == 0) )
                showfrac = false;
        }
    }

    if ( fmt == 'e' )
    {
        body.push_back(mantissa[0]);
        if ( showfrac )
        {
            body.push_back('.');
            body += mantissa.substr(1, precision);
        }
        body.push_back(ascii::is_upper(format) ? 'E' : 'e');
        if ( exponent < 10 && exponent >= 0 )
        {
            body += '0';
            body += exponent + '0';
        }
        else if ( exponent > -10 && exponent < 0 )
        {
            body += "-0";
            body += -exponent + '0';
        }
        else
        {
            body += std::to_string(exponent);
        }
    }
    else
    {
        exponent += 1;
        if ( exponent > 0 )
        {
            body += mantissa.substr(0, exponent);
        }
        else
        {
            exponent = 0;
            body.push_back('0');
        }

        if ( showfrac && mantissa.size() > std::size_t(exponent) )
            body += "." + mantissa.substr(exponent, precision);
    }
}

bool get_int_base(const FormatSpec& spec, int& base, std::string& prefix)
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

} // namespace detail

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

} // namespace format
} // namespace string
} // namespace melanolib
