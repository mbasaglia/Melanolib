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

using namespace melanolib::string;

BOOST_AUTO_TEST_CASE( test_Utf8Parser )
{
    for ( unsigned char c = 0; c < 128; c++ ) // C++ XD
    {
        BOOST_CHECK( Utf8Parser::to_ascii(c) == c );
        BOOST_CHECK( Utf8Parser::encode(c) == std::string(1, c) );
    }

    if ( Utf8Parser::has_iconv() )
    {
        BOOST_CHECK_EQUAL( Utf8Parser::to_ascii("è"), 'e' );
        BOOST_CHECK_EQUAL( Utf8Parser::to_ascii("à"), 'a' );
        BOOST_CHECK_EQUAL( Utf8Parser::to_ascii("ç"), 'c' );
        BOOST_CHECK_EQUAL( Utf8Parser::to_ascii(0x00E7), 'c' );
    }

    BOOST_CHECK( Utf8Parser::encode(0x00A7) == u8"§" );
    BOOST_CHECK( Utf8Parser::encode(0x110E) == u8"ᄎ" );
    BOOST_CHECK( Utf8Parser::encode(0x26060) == u8"𦁠" );

}
