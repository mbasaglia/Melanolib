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

#define BOOST_TEST_MODULE Test_Geometry
#include "melanolib/geo/geo.hpp"
#include "melanolib/math/math.hpp"
#include <boost/test/unit_test.hpp>

using namespace melanolib::geo::geo_double;

BOOST_AUTO_TEST_CASE( test_point )
{
    BOOST_CHECK ( distance(Point(0, 3), Point(4, 0)) == 5 );
    BOOST_CHECK ( -Point(3, 4) * 2 == Point(-6, -8) );
    BOOST_CHECK ( -2 * Point(3, 4) == Point(-6, -8) );
}

BOOST_AUTO_TEST_CASE( test_rectangle )
{
    BOOST_CHECK( !Rectangle().is_valid() );

    Rectangle r(10, 20, 100, 100);

    BOOST_CHECK( r.top() == 20 );
    BOOST_CHECK( r.left() == 10 );
    BOOST_CHECK( r.right() == 100 + 10 );
    BOOST_CHECK( r.bottom() == 100 + 20 );

    BOOST_CHECK( r.top_left() == Point(10, 20) );
    BOOST_CHECK( r.top_right() == Point(100 + 10, 20) );
    BOOST_CHECK( r.bottom_left() == Point(10, 20 + 100) );
    BOOST_CHECK( r.bottom_right() == Point(100 + 10, 20 + 100) );
    BOOST_CHECK( r.center() == Point(10 + 50, 20 + 50) );

    BOOST_CHECK( r.area() == 100 * 100 );

    BOOST_CHECK( r.contains(30, 40) );
    BOOST_CHECK( r.contains(10, 40) );
    BOOST_CHECK( !r.contains(50, 10) );

    BOOST_CHECK(r.nearest(Point(40, 40)) == Point(40, 40));
    BOOST_CHECK(r.nearest(Point(40, 400)) == Point(40, 120));

    Rectangle r2(30, 10, 100, 100);

    BOOST_CHECK(r.intersects(r2));
    BOOST_CHECK(!r.intersects(r.translated(100, 0)));
    BOOST_CHECK(r.intersection(r2) == Rectangle(Point(30, 20), Point(110, 110)) );
    BOOST_CHECK(!r.intersection(r.translated(100, 0)).is_valid());
    BOOST_CHECK(!r.intersection(Rectangle()).is_valid());
    BOOST_CHECK(!Rectangle().intersection(r).is_valid());

    BOOST_CHECK(r.united(r2) == Rectangle(Point(10, 10), Point(130, 120)) );
    BOOST_CHECK(r.united(Rectangle()) == r);
    BOOST_CHECK(Rectangle().united(r) == r);

}

BOOST_AUTO_TEST_CASE( test_circle )
{
    Circle c ( 0, 0, 100 );

    BOOST_CHECK ( c.contains(Point(10, 10)) );
    BOOST_CHECK ( c.contains(Point(0, 100)) );
    BOOST_CHECK ( !c.contains(Point(1, 100)) );

    BOOST_CHECK ( c.intersects(Circle(10, 10, 20)) );
    BOOST_CHECK ( c.intersects(Circle(10, 10, 200)) );
    BOOST_CHECK ( c.intersects(Circle(130, 0, 50)) );
    BOOST_CHECK ( c.intersects(Circle(130, 0, 30)) );
    BOOST_CHECK ( !c.intersects(Circle(130, 0, 20)) );

    BOOST_CHECK ( c.contains(Rectangle(-5, -5, 10, 10)) );
    BOOST_CHECK ( !c.contains(Rectangle(-100, -100, 100, 100)) );
    BOOST_CHECK ( !c.contains(Rectangle(95, 0, 10, 10)) );

    BOOST_CHECK ( c.intersects(Rectangle(-5, -5, 10, 10)) );
    BOOST_CHECK ( c.intersects(Rectangle(-100, -100, 100, 100)) );
    BOOST_CHECK ( c.intersects(Rectangle(95, 0, 10, 10)) );
    BOOST_CHECK ( !c.intersects(Rectangle(101, 0, 10, 10)) );
}

#define BOOST_CHECK_FLOAT(a, b) melanolib::math::fuzzy_compare(a, b)

BOOST_AUTO_TEST_CASE( test_line )
{
    Line l ({0, 0}, {10, 10});
    BOOST_CHECK ( l == Line(Point(0, 0), 10 * melanolib::math::sqrt(2), melanolib::math::pi / 4) );
    BOOST_CHECK_FLOAT ( l.length(), melanolib::math::sqrt(2) );
    BOOST_CHECK_FLOAT ( l.angle(), melanolib::math::pi / 4 );

    l.set_angle(0);
    BOOST_CHECK ( l.p2 == Point(10 * melanolib::math::sqrt(2), 0) );
    l.set_length(10);
    BOOST_CHECK ( l.p2 == Point(10, 0) );

    for ( int i = 0; i < 10; i++ )
        BOOST_CHECK ( l.point_at(i / 10.0) == Point(i, 0) );
}
