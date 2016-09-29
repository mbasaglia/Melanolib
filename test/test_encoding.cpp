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
#define BOOST_TEST_MODULE Test_Encoding

#include <boost/test/unit_test.hpp>

#include "melanolib/string/encoding.hpp"
#include <melanolib/string/ascii.hpp>

using namespace melanolib::string;

BOOST_AUTO_TEST_CASE( test_Utf8_encode )
{
    for ( unsigned char c = 0; c < 128; c++ )
    {
        BOOST_CHECK( Utf8Parser::encode(c) == std::string(1, c) );
    }

    BOOST_CHECK( Utf8Parser::encode(0x00A7) == u8"§" );
    BOOST_CHECK( Utf8Parser::encode(0x110E) == u8"ᄎ" );
    BOOST_CHECK( Utf8Parser::encode(0x26060) == u8"𦁠" );
}

BOOST_AUTO_TEST_CASE( test_Utf8_to_ascii )
{
    for ( unsigned char c = 0; c < 128; c++ )
    {
        BOOST_CHECK( Utf8Parser::to_ascii(c) == c );
    }

    if ( Utf8Parser::has_iconv() )
    {
        BOOST_CHECK_EQUAL( Utf8Parser::to_ascii("è"), 'e' );
        BOOST_CHECK_EQUAL( Utf8Parser::to_ascii("à"), 'a' );
        BOOST_CHECK_EQUAL( Utf8Parser::to_ascii("ç"), 'c' );
        BOOST_CHECK_EQUAL( Utf8Parser::to_ascii(0x00E7), 'c' );
    }
    else
    {
        BOOST_CHECK_EQUAL( Utf8Parser::to_ascii("è"), '?' );
        BOOST_CHECK_EQUAL( Utf8Parser::to_ascii("à"), '?' );
        BOOST_CHECK_EQUAL( Utf8Parser::to_ascii("ç"), '?' );
        BOOST_CHECK_EQUAL( Utf8Parser::to_ascii(0x00E7), '?' );
    }
}

void check_unicode(const Unicode& unicode, const std::string& utf8, uint32_t point)
{
    BOOST_CHECK_EQUAL( unicode.utf8(), utf8 );
    BOOST_CHECK_EQUAL( unicode.point(), point );
}

BOOST_AUTO_TEST_CASE( test_Utf8_parse_unicode )
{
    Utf8Parser parser(u8"fooᄎ§bar𦁠");
    check_unicode(parser.next(), "f", 'f');
    check_unicode(parser.next(), "o", 'o');
    check_unicode(parser.next(), "o", 'o');
    check_unicode(parser.next(), u8"ᄎ", 0x110E);
    check_unicode(parser.next(), u8"§", 0x00A7);
    check_unicode(parser.next(), "b", 'b');
    check_unicode(parser.next(), "a", 'a');
    check_unicode(parser.next(), "r", 'r');
    check_unicode(parser.next(), u8"𦁠", 0x26060);
    BOOST_CHECK(parser.finished());
}

BOOST_AUTO_TEST_CASE( test_Utf8_parse_ascii_utf8 )
{
    Utf8Parser parser(u8"fooᄎ§bar𦁠");
    BOOST_CHECK_EQUAL(parser.next_ascii(), 'f');
    BOOST_CHECK_EQUAL(parser.next_ascii(), 'o');
    BOOST_CHECK_EQUAL(parser.next_ascii(), 'o');
    BOOST_CHECK(!ascii::is_ascii(parser.next_ascii()));
    check_unicode(parser.next(), u8"ᄎ", 0x110E);
    BOOST_CHECK(!ascii::is_ascii(parser.next_ascii()));
    check_unicode(parser.next(), u8"§", 0x00A7);
    BOOST_CHECK_EQUAL(parser.next_ascii(), 'b');
    BOOST_CHECK_EQUAL(parser.next_ascii(), 'a');
    BOOST_CHECK_EQUAL(parser.next_ascii(), 'r');
    BOOST_CHECK(!ascii::is_ascii(parser.next_ascii()));
    BOOST_CHECK(!parser.finished());
    check_unicode(parser.next(), u8"𦁠", 0x26060);
    BOOST_CHECK(parser.finished());
}

BOOST_AUTO_TEST_CASE( test_Utf8_parse_ascii )
{
    Utf8Parser parser(u8"fooᄎ§bar𦁠");
    BOOST_CHECK_EQUAL(parser.next_ascii(true), 'f');
    BOOST_CHECK_EQUAL(parser.next_ascii(true), 'o');
    BOOST_CHECK_EQUAL(parser.next_ascii(true), 'o');
    BOOST_CHECK_EQUAL(parser.next_ascii(true), 'b');
    BOOST_CHECK_EQUAL(parser.next_ascii(true), 'a');
    BOOST_CHECK_EQUAL(parser.next_ascii(true), 'r');
    BOOST_CHECK_EQUAL(parser.next_ascii(true), std::char_traits<Utf8Parser::Byte>::eof());
    BOOST_CHECK(parser.finished());
}

BOOST_AUTO_TEST_CASE( test_Utf8_parse_unicode_invalid )
{
    Utf8Parser parser("z\xe1y\xe1\x80\xe1\x80\x80x\x80w\xe1");
    check_unicode(parser.next(), "z", 'z');
    check_unicode(parser.next(), "y", 'y');
    check_unicode(parser.next(), u8"\xe1\x80\x80", 4096);
    check_unicode(parser.next(), "x", 'x');
    check_unicode(parser.next(), "w", 'w');
    BOOST_CHECK(!parser.next().valid());
    BOOST_CHECK(parser.finished());
}

BOOST_AUTO_TEST_CASE( test_Utf8_parse_ascii_utf8_invalid )
{
    Utf8Parser parser("z\xe1y\xe1\x80\xe1\x80\x80x\x80w\xe1");
    BOOST_CHECK_EQUAL(parser.next_ascii(), 'z');
    BOOST_CHECK(!ascii::is_ascii(parser.next_ascii()));
    check_unicode(parser.next(), "y", 'y');
    BOOST_CHECK(!ascii::is_ascii(parser.next_ascii()));
    check_unicode(parser.next(), u8"\xe1\x80\x80", 4096);
    BOOST_CHECK_EQUAL(parser.next_ascii(), 'x');
    BOOST_CHECK_EQUAL(parser.next_ascii(), 'w');
    BOOST_CHECK(!ascii::is_ascii(parser.next_ascii()));
    BOOST_CHECK(!parser.next().valid());
    BOOST_CHECK(parser.finished());
}

BOOST_AUTO_TEST_CASE( test_Utf8_parse_ascii_invalid )
{
    Utf8Parser parser("z\xe1y\xe1\x80\xe1\x80\x80x\x80w\xe1");
    BOOST_CHECK_EQUAL(parser.next_ascii(true), 'z');
    BOOST_CHECK_EQUAL(parser.next_ascii(true), 'y');
    BOOST_CHECK_EQUAL(parser.next_ascii(true), 'x');
    BOOST_CHECK_EQUAL(parser.next_ascii(true), 'w');
    BOOST_CHECK_EQUAL(parser.next_ascii(true), std::char_traits<Utf8Parser::Byte>::eof());
    BOOST_CHECK(parser.finished());
}
