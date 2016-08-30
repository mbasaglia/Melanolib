/**
 * \file
 * \author Mattia Basaglia
 * \copyright Copyright 2015-2016 Mattia Basaglia
 * \section License
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_TEST_MODULE Test_Format


#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include "melanolib/string/format.hpp"

using boost::test_tools::output_test_stream;
using namespace melanolib::string;

BOOST_AUTO_TEST_CASE( test_sprintf_string )
{
    BOOST_CHECK_EQUAL( format::sprintf("%s", "foobar"), "foobar" );
    BOOST_CHECK_EQUAL( format::sprintf("%.3s", "foobar"), "foo" );
    BOOST_CHECK_EQUAL( format::sprintf("%4s", "foo"), "foo " );
    BOOST_CHECK_EQUAL( format::sprintf("%d<4s", "foo"), "food" );
    BOOST_CHECK_EQUAL( format::sprintf("%<5s", "foo"), "foo  " );
    BOOST_CHECK_EQUAL( format::sprintf("%^5s", "foo"), " foo " );
    BOOST_CHECK_EQUAL( format::sprintf("%>5s", "foo"), "  foo" );
    BOOST_CHECK_EQUAL( format::sprintf("%w^5.3s", "foobar"), "wfoow" );
}

BOOST_AUTO_TEST_CASE( test_sprintf_int )
{
    BOOST_CHECK_EQUAL( format::sprintf("%d", 123), "123" );

    BOOST_CHECK_EQUAL( format::sprintf("%d", -123), "-123" );
    BOOST_CHECK_EQUAL( format::sprintf("%+d", 123), "+123" );
    BOOST_CHECK_EQUAL( format::sprintf("% d", 123), " 123" );

    BOOST_CHECK_EQUAL( format::sprintf("%b", 123), "1111011" );
    BOOST_CHECK_EQUAL( format::sprintf("%x", 123), "7b" );
    BOOST_CHECK_EQUAL( format::sprintf("%X", 123), "7B" );
    BOOST_CHECK_EQUAL( format::sprintf("%o", 123), "173" );

    BOOST_CHECK_EQUAL( format::sprintf("%#b", 123), "0b1111011" );
    BOOST_CHECK_EQUAL( format::sprintf("%#x", 123), "0x7b" );
    BOOST_CHECK_EQUAL( format::sprintf("%#o", 123), "0o173" );
    BOOST_CHECK_EQUAL( format::sprintf("%#d", 123), "123" );

    BOOST_CHECK_EQUAL( format::sprintf("%6d",   -123), "  -123" );
    BOOST_CHECK_EQUAL( format::sprintf("%06d",  -123), "-00123" );
    BOOST_CHECK_EQUAL( format::sprintf("%0<6d", -123), "-12300" );
    BOOST_CHECK_EQUAL( format::sprintf("%0>6d", -123), "00-123" );
    BOOST_CHECK_EQUAL( format::sprintf("%0^6d", -123), "0-1230" );
    BOOST_CHECK_EQUAL( format::sprintf("%0=6d", -123), "-00123" );
    BOOST_CHECK_EQUAL( format::sprintf("%.>6d", -123), "..-123" );
}

BOOST_AUTO_TEST_CASE( test_sprintf_uint )
{
    BOOST_CHECK_EQUAL( format::sprintf("%d", 123u), "123" );

    BOOST_CHECK_EQUAL( format::sprintf("%+d", 123u), "+123" );
    BOOST_CHECK_EQUAL( format::sprintf("% d", 123u), " 123" );

    BOOST_CHECK_EQUAL( format::sprintf("%b", 123u), "1111011" );
    BOOST_CHECK_EQUAL( format::sprintf("%x", 123u), "7b" );
    BOOST_CHECK_EQUAL( format::sprintf("%X", 123u), "7B" );
    BOOST_CHECK_EQUAL( format::sprintf("%o", 123u), "173" );

    BOOST_CHECK_EQUAL( format::sprintf("%#x", 123u), "0x7b" );

    BOOST_CHECK_EQUAL( format::sprintf("%#6x",   123u), "  0x7b" );
    BOOST_CHECK_EQUAL( format::sprintf("%#06x",  123u), "0x007b" );
    BOOST_CHECK_EQUAL( format::sprintf("%0<#6x", 123u), "0x7b00" );
    BOOST_CHECK_EQUAL( format::sprintf("%0>#6x", 123u), "000x7b" );
    BOOST_CHECK_EQUAL( format::sprintf("%0^#6x", 123u), "00x7b0" );
    BOOST_CHECK_EQUAL( format::sprintf("%0=#6x", 123u), "0x007b" );
    BOOST_CHECK_EQUAL( format::sprintf("%.>#6x", 123u), "..0x7b" );
}

BOOST_AUTO_TEST_CASE( test_sprintf_float_percent )
{
    BOOST_CHECK_EQUAL( format::sprintf("%.1%", 0.3), "30.0%" );

}
BOOST_AUTO_TEST_CASE( test_sprintf_float_precision )
{
    BOOST_CHECK_EQUAL( format::sprintf("%.3f", 0.3), "0.300" );
    BOOST_CHECK_EQUAL( format::sprintf("%.3e", 0.3), "3.000e-01" );

    BOOST_CHECK_EQUAL( format::sprintf("%.f", 1.3), "1" );

    BOOST_CHECK_EQUAL( format::sprintf("%.3f", 0.2999), "0.300" );
}

BOOST_AUTO_TEST_CASE( test_sprintf_float_align )
{
    BOOST_CHECK_EQUAL( format::sprintf("%6g", -0.3), "  -0.3" );
    BOOST_CHECK_EQUAL( format::sprintf("%<6g", -0.3), "-0.3  " );
    BOOST_CHECK_EQUAL( format::sprintf("%>6g", -0.3), "  -0.3" );
    BOOST_CHECK_EQUAL( format::sprintf("%=6g", -0.3), "-  0.3" );
    BOOST_CHECK_EQUAL( format::sprintf("%^6g", -0.3), " -0.3 " );
}

BOOST_AUTO_TEST_CASE( test_sprintf_float_sign )
{
    BOOST_CHECK_EQUAL( format::sprintf("%g", 0.3), "0.3" );
    BOOST_CHECK_EQUAL( format::sprintf("%g", -0.3), "-0.3" );
    BOOST_CHECK_EQUAL( format::sprintf("%+g", 0.3), "+0.3" );
    BOOST_CHECK_EQUAL( format::sprintf("% g", 0.3), " 0.3" );
}

BOOST_AUTO_TEST_CASE( test_sprintf_float_as_int )
{
    BOOST_CHECK_EQUAL( format::sprintf("%d", 13.2), "13" );
    BOOST_CHECK_EQUAL( format::sprintf("%x", -13.2), "-d" );
    BOOST_CHECK_EQUAL( format::sprintf("%i", 13.2), "13" );
    BOOST_CHECK_EQUAL( format::sprintf("%o", 13.2), "15" );
    BOOST_CHECK_EQUAL( format::sprintf("%b", 13.2), "1101" );
}

BOOST_AUTO_TEST_CASE( test_sprintf_float_unknown )
{
    BOOST_CHECK_EQUAL( format::sprintf("%w", 0.3), "" );
}

BOOST_AUTO_TEST_CASE( test_sprintf_float_special_values )
{
    BOOST_CHECK_EQUAL( format::sprintf("%g", std::numeric_limits<float>::quiet_NaN()), "NaN" );
    BOOST_CHECK_EQUAL( format::sprintf("%g", std::numeric_limits<float>::infinity()), "Inf" );
    BOOST_CHECK_EQUAL( format::sprintf("%g", -std::numeric_limits<float>::infinity()), "-Inf" );
    BOOST_CHECK_EQUAL( format::sprintf("%=6g", -std::numeric_limits<float>::infinity()), "-  Inf" );
}
BOOST_AUTO_TEST_CASE( test_sprintf_float_g )
{
    BOOST_CHECK_EQUAL( format::sprintf("%g", 0.3), "0.3" );
    BOOST_CHECK_EQUAL( format::sprintf("%n", 0.3), "0.3" );

    BOOST_CHECK_EQUAL( format::sprintf("%g", 3e-1), "0.3" );
    BOOST_CHECK_EQUAL( format::sprintf("%g", 3e0), "3" );
    BOOST_CHECK_EQUAL( format::sprintf("%g", 3e1), "30" );
    BOOST_CHECK_EQUAL( format::sprintf("%g", 3e10), "3e10" );
    BOOST_CHECK_EQUAL( format::sprintf("%g", 3e-10), "3e-10" );
    BOOST_CHECK_EQUAL( format::sprintf("%g", 0), "0" );
    BOOST_CHECK_EQUAL( format::sprintf("%.3g", 0.1234), "0.123" );
}

BOOST_AUTO_TEST_CASE( test_sprintf_float_e )
{
    BOOST_CHECK_EQUAL( format::sprintf("%e", 0.3), "3.000000e-01" );
    BOOST_CHECK_EQUAL( format::sprintf("%E", 0.3), "3.000000E-01" );

    BOOST_CHECK_EQUAL( format::sprintf("%.1e", 3e1), "3.0e01" );
    BOOST_CHECK_EQUAL( format::sprintf("%.1e", 3e10), "3.0e10" );
    BOOST_CHECK_EQUAL( format::sprintf("%.1e", 3e-1), "3.0e-01" );
    BOOST_CHECK_EQUAL( format::sprintf("%.1e", 3e-10), "3.0e-10" );
}

BOOST_AUTO_TEST_CASE( test_sprintf_float_f )
{
    BOOST_CHECK_EQUAL( format::sprintf("%f", 3),     "3.000000" );
    BOOST_CHECK_EQUAL( format::sprintf("%f", 0.3),   "0.300000" );
    BOOST_CHECK_EQUAL( format::sprintf("%f", 0.003), "0.003000" );
    BOOST_CHECK_EQUAL( format::sprintf("%f", 3e-6),  "0.000003" );
    BOOST_CHECK_EQUAL( format::sprintf("%f", 3e-7),  "0.000000" );
    BOOST_CHECK_EQUAL( format::sprintf("%f", 9e-7),  "0.000001" );
}

BOOST_AUTO_TEST_CASE( test_sprintf_float_rounding)
{
    BOOST_CHECK_EQUAL( format::sprintf("%f", 0.999999),  "0.999999" );
    BOOST_CHECK_EQUAL( format::sprintf("%f", 0.9999999), "1.000000" );
    BOOST_CHECK_EQUAL( format::sprintf("%f", 0.0009999), "0.001000" );
    BOOST_CHECK_EQUAL( format::sprintf("%e", 0.999999),  "9.999990e-01" );
    BOOST_CHECK_EQUAL( format::sprintf("%e", 0.99999999), "1.000000e00" );
    BOOST_CHECK_EQUAL( format::sprintf("%e", 0.0009999), "9.999000e-04" );

    BOOST_CHECK_EQUAL( format::sprintf("%.3f", 0.27058),  "0.271" );
}

struct SomeClass
{
    friend std::ostream& operator<<(std::ostream& os, const SomeClass&)
    {
        return os << "SomeClass";
    }
};

BOOST_AUTO_TEST_CASE( test_sprintf_custom_default )
{
    BOOST_CHECK_EQUAL( format::sprintf("%s", SomeClass{}), "SomeClass" );
}

BOOST_AUTO_TEST_CASE( test_sprintf_pattern_text )
{
    BOOST_CHECK_EQUAL( format::sprintf("100%%"), "100%" );
}

BOOST_AUTO_TEST_CASE( test_printf_arg_count )
{
    output_test_stream stream;

    BOOST_CHECK( format::printf("success! %i %s!", stream, 123, "foo") );
    BOOST_CHECK( stream.is_equal("success! 123 foo!") );

    BOOST_CHECK( !format::printf("too many arguments! %i %s!", stream, 123, "foo", "bar") );
    BOOST_CHECK( stream.is_equal("too many arguments! 123 foo!") );

    BOOST_CHECK( !format::printf("too many arguments! %i %s!", stream, 123) );
    BOOST_CHECK( stream.is_equal("too many arguments! 123 !") );
}

BOOST_AUTO_TEST_CASE( test_format_map )
{
    std::map<std::string, int> values = {
        {"foo", 1},
        {"bar", 2}
    };
    BOOST_CHECK_EQUAL( format::sformat("{foo}", values), "1" );
    BOOST_CHECK_EQUAL( format::sformat("{foo} {bar:#x}", values), "1 0x2" );
    BOOST_CHECK_EQUAL( format::sformat("{foo} {bar:#x", values), "" );
}

BOOST_AUTO_TEST_CASE( test_format_functor_map )
{
    auto values = [](const std::string& name){ return name; };
    BOOST_CHECK_EQUAL( format::sformat("hello {foo}!", values), "hello foo!" );
}

BOOST_AUTO_TEST_CASE( test_format_functor_boolreturn )
{
    auto values = [](const std::string& name, const format::FormatSpec& spec, std::ostream& os){
        return format::format_item(spec, 123, os);
    };
    BOOST_CHECK_EQUAL( format::sformat("hello {foo}!", values), "hello 123!" );
}

BOOST_AUTO_TEST_CASE( test_format_functor_voidreturn )
{
    auto values = [](const std::string& name, const format::FormatSpec& spec, std::ostream& os){
        format::format_item(spec, 123, os);
    };
    BOOST_CHECK_EQUAL( format::sformat("hello {foo}!", values), "hello 123!" );
}

BOOST_AUTO_TEST_CASE( test_format_escape_braces )
{
    std::map<std::string, int> values = {
        {"foo", 1},
        {"bar", 2}
    };
    BOOST_CHECK_EQUAL( format::sformat("{{foo}} {foo}}", values), "{foo} 1}" );
}

