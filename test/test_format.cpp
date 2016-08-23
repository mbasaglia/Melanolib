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

BOOST_AUTO_TEST_CASE( test_printf_string )
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

BOOST_AUTO_TEST_CASE( test_printf_int )
{
    BOOST_CHECK_EQUAL( format::sprintf("%d", 123), "123" );

    BOOST_CHECK_EQUAL( format::sprintf("%d", -123), "-123" );
    BOOST_CHECK_EQUAL( format::sprintf("%+d", 123), "+123" );
    BOOST_CHECK_EQUAL( format::sprintf("% d", 123), " 123" );

    BOOST_CHECK_EQUAL( format::sprintf("%b", 123), "1111011" );
    BOOST_CHECK_EQUAL( format::sprintf("%x", 123), "7b" );
    BOOST_CHECK_EQUAL( format::sprintf("%X", 123), "7B" );
    BOOST_CHECK_EQUAL( format::sprintf("%o", 123), "173" );

    BOOST_CHECK_EQUAL( format::sprintf("%#x", 123), "0x7b" );

    BOOST_CHECK_EQUAL( format::sprintf("%6d",   -123), "  -123" );
    BOOST_CHECK_EQUAL( format::sprintf("%06d",  -123), "-00123" );
    BOOST_CHECK_EQUAL( format::sprintf("%0<6d", -123), "-12300" );
    BOOST_CHECK_EQUAL( format::sprintf("%0>6d", -123), "00-123" );
    BOOST_CHECK_EQUAL( format::sprintf("%0^6d", -123), "0-1230" );
    BOOST_CHECK_EQUAL( format::sprintf("%0=6d", -123), "-00123" );
    BOOST_CHECK_EQUAL( format::sprintf("%.>6d", -123), "..-123" );
}

BOOST_AUTO_TEST_CASE( test_printf_uint )
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

BOOST_AUTO_TEST_CASE( test_printf_float )
{
    BOOST_CHECK_EQUAL( format::sprintf("%g", 0.3), "0.3" );
    BOOST_CHECK_EQUAL( format::sprintf("%f", 0.3), "0.300000" );
    BOOST_CHECK_EQUAL( format::sprintf("%e", 0.3), "3.000000e-01" );
    BOOST_CHECK_EQUAL( format::sprintf("%E", 0.3), "3.000000E-01" );
    /// \todo BOOST_CHECK_EQUAL( format::sprintf("%.1%", 0.3), "30.0%" );

    BOOST_CHECK_EQUAL( format::sprintf("%.3f", 0.3), "0.300" );
    BOOST_CHECK_EQUAL( format::sprintf("%.3e", 0.3), "3.000e-01" );

    BOOST_CHECK_EQUAL( format::sprintf("%6g", -0.3), "  -0.3" );
    BOOST_CHECK_EQUAL( format::sprintf("%<6g", -0.3), "-0.3  " );
    BOOST_CHECK_EQUAL( format::sprintf("%>6g", -0.3), "  -0.3" );
    BOOST_CHECK_EQUAL( format::sprintf("%=6g", -0.3), "-  0.3" );
    /// \todo BOOST_CHECK_EQUAL( format::sprintf("%^6g", -0.3), " -0.3 " );

    BOOST_CHECK_EQUAL( format::sprintf("%g", 0.3), "0.3" );
    BOOST_CHECK_EQUAL( format::sprintf("%g", -0.3), "-0.3" );
    BOOST_CHECK_EQUAL( format::sprintf("%+g", 0.3), "+0.3" );
    /// \todo BOOST_CHECK_EQUAL( format::sprintf("% g", 0.3), " 0.3" );
}
