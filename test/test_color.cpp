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

#define BOOST_TEST_MODULE Test_Color

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include "melanolib/color/color.hpp"

using boost::test_tools::output_test_stream;
using namespace melanolib::color;

BOOST_AUTO_TEST_CASE( test_default_ctor )
{
    Color color;

    BOOST_CHECK( !color.valid() );
}

BOOST_AUTO_TEST_CASE( test_rgb_ctor )
{
    Color color1(1, 2, 3);
    BOOST_CHECK( color1.valid() );
    BOOST_CHECK_EQUAL( color1.red(), 1 );
    BOOST_CHECK_EQUAL( color1.green(), 2 );
    BOOST_CHECK_EQUAL( color1.blue(), 3 );
    BOOST_CHECK_EQUAL( color1.alpha(), 255 );

    Color color2(1, 2, 3, 4);
    BOOST_CHECK( color2.valid() );
    BOOST_CHECK_EQUAL( color2.red(), 1 );
    BOOST_CHECK_EQUAL( color2.green(), 2 );
    BOOST_CHECK_EQUAL( color2.blue(), 3 );
    BOOST_CHECK_EQUAL( color2.alpha(), 4 );
}

BOOST_AUTO_TEST_CASE( test_comparison )
{
    BOOST_CHECK_EQUAL( Color(1, 2, 3), Color(1, 2, 3) );
    BOOST_CHECK_NE( Color(1, 2, 3), Color(1, 2, 3, 4) );
    BOOST_CHECK_EQUAL( Color(1, 2, 3, 4), Color(1, 2, 3, 4) );
    BOOST_CHECK_NE( Color(1, 2, 3), Color() );
    BOOST_CHECK_NE( Color(), Color(1, 2, 3) );
    BOOST_CHECK_EQUAL( Color(), Color() );
    BOOST_CHECK_NE( Color(1, 2, 3), Color(1, 2, 0) );
    BOOST_CHECK_NE( Color(1, 2, 3), Color(1, 0, 3) );
    BOOST_CHECK_NE( Color(1, 2, 3), Color(0, 2, 3) );
}

BOOST_AUTO_TEST_CASE( test_stream )
{
    output_test_stream output;

    output << Color();
    BOOST_CHECK( output.is_equal( "rgb()" ) );

    output << Color(1, 2, 3);
    BOOST_CHECK( output.is_equal( "rgb(1, 2, 3)" ) );

    output << Color(1, 2, 3, 4);
    BOOST_CHECK( output.is_equal( "rgba(1, 2, 3, 4)" ) );
}

BOOST_AUTO_TEST_CASE( test_repr_ctor )
{
    Color a(repr::RGB(1, 2, 3));
    BOOST_CHECK( a.valid() );
    BOOST_CHECK_EQUAL( a.red(), 1 );
    BOOST_CHECK_EQUAL( a.green(), 2 );
    BOOST_CHECK_EQUAL( a.blue(), 3 );
    BOOST_CHECK_EQUAL( a.alpha(), 255 );

    Color b(repr::RGB(1, 2, 3), 4);
    BOOST_CHECK( b.valid() );
    BOOST_CHECK_EQUAL( b.red(), 1 );
    BOOST_CHECK_EQUAL( b.green(), 2 );
    BOOST_CHECK_EQUAL( b.blue(), 3 );
    BOOST_CHECK_EQUAL( b.alpha(), 4 );

    Color c(repr::RGBf(0, 1, 0));
    BOOST_CHECK( c.valid() );
    BOOST_CHECK_EQUAL( c.red(), 0 );
    BOOST_CHECK_EQUAL( c.green(), 255 );
    BOOST_CHECK_EQUAL( c.blue(), 0 );
    BOOST_CHECK_EQUAL( c.alpha(), 255 );

    Color d(repr::RGBf(0, 1, 0), 0.5);
    BOOST_CHECK( d.valid() );
    BOOST_CHECK_EQUAL( d.red(), 0 );
    BOOST_CHECK_EQUAL( d.green(), 255 );
    BOOST_CHECK_EQUAL( d.blue(), 0 );
    BOOST_CHECK_EQUAL( d.alpha(), 128 );
}

BOOST_AUTO_TEST_CASE( test_from_rgbf )
{
    Color c(repr::RGBf(0, 0.5, 1));
    BOOST_CHECK( c.valid() );
    BOOST_CHECK_EQUAL( c.red(), 0 );
    BOOST_CHECK_EQUAL( c.green(), 128 );
    BOOST_CHECK_EQUAL( c.blue(), 255 );
}

BOOST_AUTO_TEST_CASE( test_to_rgbf )
{
    double tolerance = 1; // Tolerance percentage
    repr::RGBf start(0, 0.5, 1);
    repr::RGBf converted = Color(start).to<repr::RGBf>();
    BOOST_CHECK_CLOSE( start.r, converted.r, tolerance );
    BOOST_CHECK_CLOSE( start.g, converted.g, tolerance );
    BOOST_CHECK_CLOSE( start.b, converted.b, tolerance );
}

BOOST_AUTO_TEST_CASE( test_from_hsvf )
{
    BOOST_CHECK_EQUAL(Color(repr::HSVf(0, 0, 0)), Color(0, 0, 0));
    BOOST_CHECK_EQUAL(Color(repr::HSVf(0.5, 0, 0)), Color(0, 0, 0));
    BOOST_CHECK_EQUAL(Color(repr::HSVf(0.5, 1, 0)), Color(0, 0, 0));

    BOOST_CHECK_EQUAL(Color(repr::HSVf(0, 1, 1)), Color(255, 0, 0));
    BOOST_CHECK_EQUAL(Color(repr::HSVf(-1, 1, 1)), Color(255, 0, 0));
    BOOST_CHECK_EQUAL(Color(repr::HSVf(1, 1, 1)), Color(255, 0, 0));
    BOOST_CHECK_EQUAL(Color(repr::HSVf(0.5, 1, 1)), Color(0, 255, 255));
    BOOST_CHECK_EQUAL(Color(repr::HSVf(1.5, 1, 1)), Color(0, 255, 255));

    BOOST_CHECK_EQUAL(Color(repr::HSVf(0/6.0, 1, 1)), Color(255, 0, 0));
    BOOST_CHECK_EQUAL(Color(repr::HSVf(1/6.0, 1, 1)), Color(255, 255, 0));
    BOOST_CHECK_EQUAL(Color(repr::HSVf(2/6.0, 1, 1)), Color(0, 255, 0));
    BOOST_CHECK_EQUAL(Color(repr::HSVf(3/6.0, 1, 1)), Color(0, 255, 255));
    BOOST_CHECK_EQUAL(Color(repr::HSVf(4/6.0, 1, 1)), Color(0, 0, 255));
    BOOST_CHECK_EQUAL(Color(repr::HSVf(5/6.0, 1, 1)), Color(255, 0, 255));
    BOOST_CHECK_EQUAL(Color(repr::HSVf(6/6.0, 1, 1)), Color(255, 0, 0));
}

BOOST_AUTO_TEST_CASE( test_to_hsvf )
{
    double tolerance = 1; // Tolerance percentage

    // Macro to preserve line numbers and stuff
    #define check(rgb, expected_hsv) \
    { \
        auto converted = rgb.to<repr::HSVf>(); \
        BOOST_CHECK_CLOSE( converted.h, expected_hsv.h, tolerance ); \
        BOOST_CHECK_CLOSE( converted.s, expected_hsv.s, tolerance ); \
        BOOST_CHECK_CLOSE( converted.v, expected_hsv.v, tolerance ); \
    }

    check(Color(255, 0, 0), repr::HSVf(0/6.0, 1, 1));
    check(Color(255, 255, 0), repr::HSVf(1/6.0, 1, 1));
    check(Color(0, 255, 0), repr::HSVf(2/6.0, 1, 1));
    check(Color(0, 255, 255), repr::HSVf(3/6.0, 1, 1));
    check(Color(0, 0, 255), repr::HSVf(4/6.0, 1, 1));
    check(Color(255, 0, 255), repr::HSVf(5/6.0, 1, 1));

    check(Color(255, 255, 255), repr::HSVf(0, 0, 1));
    check(Color(0, 0, 0), repr::HSVf(0, 0, 0));

    #undef check
}

BOOST_AUTO_TEST_CASE( test_lab )
{
    double tolerance = 1.1; // Tolerance percentage
    #define check(rgb, expected) \
    { \
        auto converted = rgb.to<repr::Lab>(); \
        BOOST_CHECK_CLOSE( converted.l + 1, expected.l + 1, tolerance ); \
        BOOST_CHECK_CLOSE( converted.a + 1, expected.a + 1, tolerance ); \
        BOOST_CHECK_CLOSE( converted.b + 1, expected.b + 1, tolerance ); \
    }

    check(Color(0, 0, 0), repr::Lab(0, 0, 0));
    check(Color(255, 0, 0), repr::Lab(53, 80, 67));
    check(Color(255, 255, 255), repr::Lab(100, 0, 0));

    #undef check

    BOOST_CHECK_EQUAL(Color(repr::Lab(0, 0, 0)), Color(0, 0, 0));
    BOOST_CHECK_EQUAL(Color(repr::Lab(53.233, 80.109, 67.220)), Color(255, 0, 0));
    BOOST_CHECK_EQUAL(Color(repr::Lab(100, 0.005, -0.01)), Color(255, 255, 255));
}

BOOST_AUTO_TEST_CASE( test_distance )
{
    Color a(71, 92, 43);
    Color b(73, 92, 43);
    Color c(74, 92, 43);
    Color d(67, 103, 28);
    Color e(129, 92, 43);
    BOOST_CHECK_SMALL(a.distance(a), 0.01f);
    BOOST_CHECK_LT(a.distance(b), a.distance(c));
    BOOST_CHECK_LT(a.distance(d), a.distance(e));
}

BOOST_AUTO_TEST_CASE( test_blend )
{
    Color a(10, 20, 30, 0);
    Color b(110, 120, 130);
    BOOST_CHECK_EQUAL(a.blend(b), Color(60, 70, 80, 128));
    BOOST_CHECK_EQUAL(a.blend(b, 0), a);
    BOOST_CHECK_EQUAL(a.blend(b, 1), b);
}

BOOST_AUTO_TEST_CASE( test_rgb_int24 )
{
    BOOST_CHECK_EQUAL(Color(0x12, 0x23, 0x34).to<repr::RGB_int24>().rgb, 0x122334);
    BOOST_CHECK_EQUAL(Color(repr::RGB_int24(0x122334)), Color(0x12, 0x23, 0x34));
    BOOST_CHECK_EQUAL(repr::RGB_int24(0x122334).rgba(), 0x122334ff);
    BOOST_CHECK_EQUAL(repr::RGB_int24(0x122334).rgba(0x45), 0x12233445);
    BOOST_CHECK_EQUAL(repr::RGB_int24(0x122334).argb(), 0xff122334);
    BOOST_CHECK_EQUAL(repr::RGB_int24(0x122334).argb(0x01), 0x01122334);
}

BOOST_AUTO_TEST_CASE( test_rgb_int12 )
{
    BOOST_CHECK_EQUAL(Color(0x12, 0x23, 0x34).to<repr::RGB_int12>().rgb, 0x123);
    BOOST_CHECK_EQUAL(Color(repr::RGB_int12(0x123)), Color(0x11, 0x22, 0x33));
    BOOST_CHECK_EQUAL(repr::RGB_int12(0x123).rgba(), 0x123f);
    BOOST_CHECK_EQUAL(repr::RGB_int12(0x123).rgba(0x4), 0x1234);
    BOOST_CHECK_EQUAL(repr::RGB_int12(0x123).argb(), 0xf123);
    BOOST_CHECK_EQUAL(repr::RGB_int12(0x123).argb(0x0), 0x0123);
}

BOOST_AUTO_TEST_CASE( test_to_rgb_int3 )
{
    BOOST_CHECK_EQUAL(Color(255, 0, 0).to<repr::RGB_int3>().rgb(), 0b001);
    BOOST_CHECK_EQUAL(Color(255, 255, 0).to<repr::RGB_int3>().rgb(), 0b011);
    BOOST_CHECK_EQUAL(Color(0, 255, 0).to<repr::RGB_int3>().rgb(), 0b010);
    BOOST_CHECK_EQUAL(Color(0, 255, 255).to<repr::RGB_int3>().rgb(), 0b110);
    BOOST_CHECK_EQUAL(Color(0, 0, 255).to<repr::RGB_int3>().rgb(), 0b100);
    BOOST_CHECK_EQUAL(Color(255, 0, 255).to<repr::RGB_int3>().rgb(), 0b101);
    BOOST_CHECK(Color(255, 0, 0).to<repr::RGB_int3>().bright());

    BOOST_CHECK_EQUAL(Color(128, 0, 0).to<repr::RGB_int3>().rgb(), 0b001);
    BOOST_CHECK_EQUAL(Color(128, 128, 0).to<repr::RGB_int3>().rgb(), 0b011);
    BOOST_CHECK_EQUAL(Color(0, 128, 0).to<repr::RGB_int3>().rgb(), 0b010);
    BOOST_CHECK_EQUAL(Color(0, 128, 128).to<repr::RGB_int3>().rgb(), 0b110);
    BOOST_CHECK_EQUAL(Color(0, 0, 128).to<repr::RGB_int3>().rgb(), 0b100);
    BOOST_CHECK_EQUAL(Color(128, 0, 128).to<repr::RGB_int3>().rgb(), 0b101);
    BOOST_CHECK_EQUAL(Color(128, 128, 128).to<repr::RGB_int3>().rgb(), 0b111);
    BOOST_CHECK(!Color(128, 0, 0).to<repr::RGB_int3>().bright());

    BOOST_CHECK_EQUAL(Color(200, 50, 87).to<repr::RGB_int3>().rgb(), 0b001);

    BOOST_CHECK_EQUAL(Color(255, 255, 255).to<repr::RGB_int3>().rgb(), 0b111);
    BOOST_CHECK(Color(255, 255, 255).to<repr::RGB_int3>().bright());
    BOOST_CHECK_EQUAL(Color(136, 136, 136).to<repr::RGB_int3>().rgb(), 0b111);
    BOOST_CHECK(!Color(136, 136, 136).to<repr::RGB_int3>().bright());
    BOOST_CHECK_EQUAL(Color(70, 70, 70).to<repr::RGB_int3>().rgb(), 0b000);
    BOOST_CHECK(Color(70, 70, 70).to<repr::RGB_int3>().bright());
    BOOST_CHECK_EQUAL(Color(0, 0, 0).to<repr::RGB_int3>().rgb(), 0b000);
    BOOST_CHECK(!Color(0, 0, 0).to<repr::RGB_int3>().bright());
}

BOOST_AUTO_TEST_CASE( test_from_rgb_int3 )
{
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b000, true)), Color(70, 70, 70));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b001, true)), Color(255, 0, 0));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b010, true)), Color(0, 255, 0));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b011, true)), Color(255, 255, 0));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b100, true)), Color(0, 0, 255));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b101, true)), Color(255, 0, 255));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b110, true)), Color(0, 255, 255));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b111, true)), Color(255, 255, 255));

    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b000, false)), Color(0, 0, 0));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b001, false)), Color(128, 0, 0));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b010, false)), Color(0, 128, 0));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b011, false)), Color(128, 128, 0));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b100, false)), Color(0, 0, 128));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b101, false)), Color(128, 0, 128));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b110, false)), Color(0, 128, 128));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b111, false)), Color(136, 136, 136));

    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b1000)), Color(70, 70, 70));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b1001)), Color(255, 0, 0));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b1010)), Color(0, 255, 0));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b1011)), Color(255, 255, 0));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b1100)), Color(0, 0, 255));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b1101)), Color(255, 0, 255));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b1110)), Color(0, 255, 255));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b1111)), Color(255, 255, 255));

    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b0000)), Color(0, 0, 0));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b0001)), Color(128, 0, 0));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b0010)), Color(0, 128, 0));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b0011)), Color(128, 128, 0));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b0100)), Color(0, 0, 128));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b0101)), Color(128, 0, 128));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b0110)), Color(0, 128, 128));
    BOOST_CHECK_EQUAL(Color(repr::RGB_int3(0b0111)), Color(136, 136, 136));
}

BOOST_AUTO_TEST_CASE( test_rgb_int3_round_trip )
{
    for ( int i = 0b0000; i < 0b1111; i++ )
    {
        repr::RGB_int3 rgb3(i);
        auto converted = Color(rgb3).to<repr::RGB_int3>();
        BOOST_CHECK_EQUAL(converted.color, rgb3.color);
    }
}
