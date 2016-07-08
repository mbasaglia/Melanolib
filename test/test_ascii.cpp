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
#define BOOST_TEST_MODULE Test_String

#include <boost/test/unit_test.hpp>


#include "melanolib/string/ascii.hpp"

using namespace melanolib::string::ascii;


BOOST_AUTO_TEST_CASE( test_ascii_is )
{
    for ( int c = 0; c <= 255; c++ )
    {
        if ( c <= 127 )
           BOOST_CHECK( is_ascii(c) );
        else
            BOOST_CHECK( !is_ascii(c) );
    }

    // control
    for ( char c = 0x00; c <= 0x08; c++ )
    {
        BOOST_CHECK( is_cntrl(c) );
        BOOST_CHECK( !is_print(c) );
        BOOST_CHECK( !is_space(c) );
        BOOST_CHECK( !is_blank(c) );
        BOOST_CHECK( !is_space_noline(c) );
        BOOST_CHECK( !is_graph(c) );
        BOOST_CHECK( !is_punct(c) );
        BOOST_CHECK( !is_alnum(c) );
        BOOST_CHECK( !is_alpha(c) );
        BOOST_CHECK( !is_upper(c) );
        BOOST_CHECK( !is_lower(c) );
        BOOST_CHECK( !is_digit(c) );
        BOOST_CHECK( !is_xdigit(c) );
    }

    // tab
    {
        char c = '\t';
        BOOST_CHECK( is_cntrl(c) );
        BOOST_CHECK( !is_print(c) );
        BOOST_CHECK( is_space(c) );
        BOOST_CHECK( is_blank(c) );
        BOOST_CHECK( is_space_noline(c) );
        BOOST_CHECK( !is_graph(c) );
        BOOST_CHECK( !is_punct(c) );
        BOOST_CHECK( !is_alnum(c) );
        BOOST_CHECK( !is_alpha(c) );
        BOOST_CHECK( !is_upper(c) );
        BOOST_CHECK( !is_lower(c) );
        BOOST_CHECK( !is_digit(c) );
        BOOST_CHECK( !is_xdigit(c) );
    }

    // newline
    {
        char c = '\n';
        BOOST_CHECK( is_cntrl(c) );
        BOOST_CHECK( !is_print(c) );
        BOOST_CHECK( is_space(c) );
        BOOST_CHECK( !is_blank(c) );
        BOOST_CHECK( !is_space_noline(c) );
        BOOST_CHECK( !is_graph(c) );
        BOOST_CHECK( !is_punct(c) );
        BOOST_CHECK( !is_alnum(c) );
        BOOST_CHECK( !is_alpha(c) );
        BOOST_CHECK( !is_upper(c) );
        BOOST_CHECK( !is_lower(c) );
        BOOST_CHECK( !is_digit(c) );
        BOOST_CHECK( !is_xdigit(c) );
    }

    // vtab
    {
        char c = '\v';
        BOOST_CHECK( is_cntrl(c) );
        BOOST_CHECK( !is_print(c) );
        BOOST_CHECK( is_space(c) );
        BOOST_CHECK( !is_blank(c) );
        BOOST_CHECK( is_space_noline(c) );
        BOOST_CHECK( !is_graph(c) );
        BOOST_CHECK( !is_punct(c) );
        BOOST_CHECK( !is_alnum(c) );
        BOOST_CHECK( !is_alpha(c) );
        BOOST_CHECK( !is_upper(c) );
        BOOST_CHECK( !is_lower(c) );
        BOOST_CHECK( !is_digit(c) );
        BOOST_CHECK( !is_xdigit(c) );
    }

    // feed
    {
        char c = '\f';
        BOOST_CHECK( is_cntrl(c) );
        BOOST_CHECK( !is_print(c) );
        BOOST_CHECK( is_space(c) );
        BOOST_CHECK( !is_blank(c) );
        BOOST_CHECK( is_space_noline(c) );
        BOOST_CHECK( !is_graph(c) );
        BOOST_CHECK( !is_punct(c) );
        BOOST_CHECK( !is_alnum(c) );
        BOOST_CHECK( !is_alpha(c) );
        BOOST_CHECK( !is_upper(c) );
        BOOST_CHECK( !is_lower(c) );
        BOOST_CHECK( !is_digit(c) );
        BOOST_CHECK( !is_xdigit(c) );
    }

    // carriage return
    {
        char c = '\r';
        BOOST_CHECK( is_cntrl(c) );
        BOOST_CHECK( !is_print(c) );
        BOOST_CHECK( is_space(c) );
        BOOST_CHECK( !is_blank(c) );
        BOOST_CHECK( !is_space_noline(c) );
        BOOST_CHECK( !is_graph(c) );
        BOOST_CHECK( !is_punct(c) );
        BOOST_CHECK( !is_alnum(c) );
        BOOST_CHECK( !is_alpha(c) );
        BOOST_CHECK( !is_upper(c) );
        BOOST_CHECK( !is_lower(c) );
        BOOST_CHECK( !is_digit(c) );
        BOOST_CHECK( !is_xdigit(c) );
    }

    // control
    for ( char c = 0x0e; c <= 0x1f; c++ )
    {
        BOOST_CHECK( is_cntrl(c) );
        BOOST_CHECK( !is_print(c) );
        BOOST_CHECK( !is_space(c) );
        BOOST_CHECK( !is_blank(c) );
        BOOST_CHECK( !is_space_noline(c) );
        BOOST_CHECK( !is_graph(c) );
        BOOST_CHECK( !is_punct(c) );
        BOOST_CHECK( !is_alnum(c) );
        BOOST_CHECK( !is_alpha(c) );
        BOOST_CHECK( !is_upper(c) );
        BOOST_CHECK( !is_lower(c) );
        BOOST_CHECK( !is_digit(c) );
        BOOST_CHECK( !is_xdigit(c) );
    }

    // space
    {
        char c = ' ';
        BOOST_CHECK( !is_cntrl(c) );
        BOOST_CHECK( is_print(c) );
        BOOST_CHECK( is_space(c) );
        BOOST_CHECK( is_blank(c) );
        BOOST_CHECK( is_space_noline(c) );
        BOOST_CHECK( !is_graph(c) );
        BOOST_CHECK( !is_punct(c) );
        BOOST_CHECK( !is_alnum(c) );
        BOOST_CHECK( !is_alpha(c) );
        BOOST_CHECK( !is_upper(c) );
        BOOST_CHECK( !is_lower(c) );
        BOOST_CHECK( !is_digit(c) );
        BOOST_CHECK( !is_xdigit(c) );
    }

    // punctuation
    for ( char c = 0x21; c <= 0x2f; c++ )
    {
        BOOST_CHECK( !is_cntrl(c) );
        BOOST_CHECK( is_print(c) );
        BOOST_CHECK( !is_space(c) );
        BOOST_CHECK( !is_blank(c) );
        BOOST_CHECK( !is_space_noline(c) );
        BOOST_CHECK( is_graph(c) );
        BOOST_CHECK( is_punct(c) );
        BOOST_CHECK( !is_alnum(c) );
        BOOST_CHECK( !is_alpha(c) );
        BOOST_CHECK( !is_upper(c) );
        BOOST_CHECK( !is_lower(c) );
        BOOST_CHECK( !is_digit(c) );
        BOOST_CHECK( !is_xdigit(c) );
    }

    // digits
    for ( char c = 0x30; c <= 0x39; c++ )
    {
        BOOST_CHECK( !is_cntrl(c) );
        BOOST_CHECK( is_print(c) );
        BOOST_CHECK( !is_space(c) );
        BOOST_CHECK( !is_blank(c) );
        BOOST_CHECK( !is_space_noline(c) );
        BOOST_CHECK( is_graph(c) );
        BOOST_CHECK( !is_punct(c) );
        BOOST_CHECK( is_alnum(c) );
        BOOST_CHECK( !is_alpha(c) );
        BOOST_CHECK( !is_upper(c) );
        BOOST_CHECK( !is_lower(c) );
        BOOST_CHECK( is_digit(c) );
        BOOST_CHECK( is_xdigit(c) );
    }

    // punctuation
    for ( char c = 0x3a; c <= 0x40; c++ )
    {
        BOOST_CHECK( !is_cntrl(c) );
        BOOST_CHECK( is_print(c) );
        BOOST_CHECK( !is_space(c) );
        BOOST_CHECK( !is_blank(c) );
        BOOST_CHECK( !is_space_noline(c) );
        BOOST_CHECK( is_graph(c) );
        BOOST_CHECK( is_punct(c) );
        BOOST_CHECK( !is_alnum(c) );
        BOOST_CHECK( !is_alpha(c) );
        BOOST_CHECK( !is_upper(c) );
        BOOST_CHECK( !is_lower(c) );
        BOOST_CHECK( !is_digit(c) );
        BOOST_CHECK( !is_xdigit(c) );
    }

    // A-F
    for ( char c = 0x41; c <= 0x46; c++ )
    {
        BOOST_CHECK( !is_cntrl(c) );
        BOOST_CHECK( is_print(c) );
        BOOST_CHECK( !is_space(c) );
        BOOST_CHECK( !is_blank(c) );
        BOOST_CHECK( !is_space_noline(c) );
        BOOST_CHECK( is_graph(c) );
        BOOST_CHECK( !is_punct(c) );
        BOOST_CHECK( is_alnum(c) );
        BOOST_CHECK( is_alpha(c) );
        BOOST_CHECK( is_upper(c) );
        BOOST_CHECK( !is_lower(c) );
        BOOST_CHECK( !is_digit(c) );
        BOOST_CHECK( is_xdigit(c) );
    }

    // G-Z
    for ( char c = 0x47; c <= 0x5a; c++ )
    {
        BOOST_CHECK( !is_cntrl(c) );
        BOOST_CHECK( is_print(c) );
        BOOST_CHECK( !is_space(c) );
        BOOST_CHECK( !is_blank(c) );
        BOOST_CHECK( !is_space_noline(c) );
        BOOST_CHECK( is_graph(c) );
        BOOST_CHECK( !is_punct(c) );
        BOOST_CHECK( is_alnum(c) );
        BOOST_CHECK( is_alpha(c) );
        BOOST_CHECK( is_upper(c) );
        BOOST_CHECK( !is_lower(c) );
        BOOST_CHECK( !is_digit(c) );
        BOOST_CHECK( !is_xdigit(c) );
    }

    // punctuation
    for ( char c = 0x5b; c <= 0x60; c++ )
    {
        BOOST_CHECK( !is_cntrl(c) );
        BOOST_CHECK( is_print(c) );
        BOOST_CHECK( !is_space(c) );
        BOOST_CHECK( !is_blank(c) );
        BOOST_CHECK( !is_space_noline(c) );
        BOOST_CHECK( is_graph(c) );
        BOOST_CHECK( is_punct(c) );
        BOOST_CHECK( !is_alnum(c) );
        BOOST_CHECK( !is_alpha(c) );
        BOOST_CHECK( !is_upper(c) );
        BOOST_CHECK( !is_lower(c) );
        BOOST_CHECK( !is_digit(c) );
        BOOST_CHECK( !is_xdigit(c) );
    }

    // a-f
    for ( char c = 0x61; c <= 0x66; c++ )
    {
        BOOST_CHECK( !is_cntrl(c) );
        BOOST_CHECK( is_print(c) );
        BOOST_CHECK( !is_space(c) );
        BOOST_CHECK( !is_blank(c) );
        BOOST_CHECK( !is_space_noline(c) );
        BOOST_CHECK( is_graph(c) );
        BOOST_CHECK( !is_punct(c) );
        BOOST_CHECK( is_alnum(c) );
        BOOST_CHECK( is_alpha(c) );
        BOOST_CHECK( !is_upper(c) );
        BOOST_CHECK( is_lower(c) );
        BOOST_CHECK( !is_digit(c) );
        BOOST_CHECK( is_xdigit(c) );
    }

    // g-z
    for ( char c = 0x67; c <= 0x7a; c++ )
    {
        BOOST_CHECK( !is_cntrl(c) );
        BOOST_CHECK( is_print(c) );
        BOOST_CHECK( !is_space(c) );
        BOOST_CHECK( !is_blank(c) );
        BOOST_CHECK( !is_space_noline(c) );
        BOOST_CHECK( is_graph(c) );
        BOOST_CHECK( !is_punct(c) );
        BOOST_CHECK( is_alnum(c) );
        BOOST_CHECK( is_alpha(c) );
        BOOST_CHECK( !is_upper(c) );
        BOOST_CHECK( is_lower(c) );
        BOOST_CHECK( !is_digit(c) );
        BOOST_CHECK( !is_xdigit(c) );
    }

    // punctuation
    for ( char c = 0x7b; c <= 0x7e; c++ )
    {
        BOOST_CHECK( !is_cntrl(c) );
        BOOST_CHECK( is_print(c) );
        BOOST_CHECK( !is_space(c) );
        BOOST_CHECK( !is_blank(c) );
        BOOST_CHECK( !is_space_noline(c) );
        BOOST_CHECK( is_graph(c) );
        BOOST_CHECK( is_punct(c) );
        BOOST_CHECK( !is_alnum(c) );
        BOOST_CHECK( !is_alpha(c) );
        BOOST_CHECK( !is_upper(c) );
        BOOST_CHECK( !is_lower(c) );
        BOOST_CHECK( !is_digit(c) );
        BOOST_CHECK( !is_xdigit(c) );
    }

    // del
    {
        char c = 127;
        BOOST_CHECK( is_cntrl(c) );
        BOOST_CHECK( !is_print(c) );
        BOOST_CHECK( !is_space(c) );
        BOOST_CHECK( !is_blank(c) );
        BOOST_CHECK( !is_space_noline(c) );
        BOOST_CHECK( !is_graph(c) );
        BOOST_CHECK( !is_punct(c) );
        BOOST_CHECK( !is_alnum(c) );
        BOOST_CHECK( !is_alpha(c) );
        BOOST_CHECK( !is_upper(c) );
        BOOST_CHECK( !is_lower(c) );
        BOOST_CHECK( !is_digit(c) );
        BOOST_CHECK( !is_xdigit(c) );
    }
}

BOOST_AUTO_TEST_CASE( test_ascii_get_hex )
{
    BOOST_CHECK_EQUAL( get_hex('7'), 0x7 );
    BOOST_CHECK_EQUAL( get_hex('a'), 0xa );
    BOOST_CHECK_EQUAL( get_hex('B'), 0xb );
}

BOOST_AUTO_TEST_CASE( test_ascii_hex_digit )
{
    BOOST_CHECK_EQUAL( hex_digit(0x7), '7' );
    BOOST_CHECK_EQUAL( hex_digit(0xa), 'A' );
}

BOOST_AUTO_TEST_CASE( test_ascii_to_upper_lower )
{
    BOOST_CHECK_EQUAL( to_upper('7'), '7' );
    BOOST_CHECK_EQUAL( to_upper('a'), 'A' );
    BOOST_CHECK_EQUAL( to_upper('B'), 'B' );

    BOOST_CHECK_EQUAL( to_lower('7'), '7' );
    BOOST_CHECK_EQUAL( to_lower('a'), 'a' );
    BOOST_CHECK_EQUAL( to_lower('B'), 'b' );
}
