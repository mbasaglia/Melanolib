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
#define BOOST_TEST_MODULE Test_Math

#include <boost/test/unit_test.hpp>

#include <unordered_map>

#include "melanolib/math/math.hpp"

using namespace melanolib;


BOOST_AUTO_TEST_CASE( test_fuzzy_compare )
{
    BOOST_CHECK(math::fuzzy_compare(1.0, 1.0));
    BOOST_CHECK(math::fuzzy_compare(-1.0, -1.0));
    BOOST_CHECK(math::fuzzy_compare(0.0, 0.0));
    BOOST_CHECK(math::fuzzy_compare(0.0, -0.0));
    BOOST_CHECK(math::fuzzy_compare(-0.0, 0.0));
    BOOST_CHECK(math::fuzzy_compare(-0.0, -0.0));

    double x = 0;
    for ( int i = 0; i < 10; i++ )
        x += 0.1;
    BOOST_CHECK(math::fuzzy_compare(x, 1.0));

    math::compare_equals<double> comparator;
    BOOST_CHECK(comparator(1.0, 1.0));
    BOOST_CHECK(comparator(-1.0, -1.0));
    BOOST_CHECK(comparator(0.0, 0.0));
    BOOST_CHECK(comparator(0.0, -0.0));
    BOOST_CHECK(comparator(-0.0, 0.0));
    BOOST_CHECK(comparator(-0.0, -0.0));
    BOOST_CHECK(comparator(x, 1.0));

}

BOOST_AUTO_TEST_CASE( test_rounding )
{
    BOOST_CHECK ( math::truncate(5.6) == 5 );
    BOOST_CHECK ( math::truncate(-5.6) == -5 );
    BOOST_CHECK ( math::truncate(5.5) == 5 );
    BOOST_CHECK ( math::truncate(-5.5) == -5 );
    BOOST_CHECK ( math::truncate(5.4) == 5 );
    BOOST_CHECK ( math::truncate(-5.4) == -5 );
    BOOST_CHECK ( math::truncate(5.0) == 5 );
    BOOST_CHECK ( math::truncate(-5.0) == -5 );
    BOOST_CHECK ( math::truncate(5.00001) == 5 );
    BOOST_CHECK ( math::truncate(5.99999) == 5 );

    BOOST_CHECK ( math::round(5.6) == 6 );
    BOOST_CHECK ( math::round(-5.6) == -6 );
    BOOST_CHECK ( math::round(5.5) == 6 );
    BOOST_CHECK ( math::round(-5.5) == -6 );
    BOOST_CHECK ( math::round(5.4) == 5 );
    BOOST_CHECK ( math::round(-5.4) == -5 );
    BOOST_CHECK ( math::round(5.0) == 5 );
    BOOST_CHECK ( math::round(-5.0) == -5 );
    BOOST_CHECK ( math::round(5.00001) == 5 );
    BOOST_CHECK ( math::round(5.99999) == 6 );

    BOOST_CHECK ( math::floor(5.6) == 5 );
    BOOST_CHECK ( math::floor(-5.6) == -6 );
    BOOST_CHECK ( math::floor(5.5) == 5 );
    BOOST_CHECK ( math::floor(-5.5) == -6 );
    BOOST_CHECK ( math::floor(5.4) == 5 );
    BOOST_CHECK ( math::floor(-5.4) == -6 );
    BOOST_CHECK ( math::floor(5.0) == 5 );
    BOOST_CHECK ( math::floor(-5.0) == -5 );
    BOOST_CHECK ( math::floor(5.00001) == 5 );
    BOOST_CHECK ( math::floor(5.99999) == 5 );

    BOOST_CHECK ( math::ceil(5.6) == 6 );
    BOOST_CHECK ( math::ceil(-5.6) == -5 );
    BOOST_CHECK ( math::ceil(5.5) == 6 );
    BOOST_CHECK ( math::ceil(-5.5) == -5 );
    BOOST_CHECK ( math::ceil(5.4) == 6 );
    BOOST_CHECK ( math::ceil(-5.4) == -5 );
    BOOST_CHECK ( math::ceil(5.0) == 5 );
    BOOST_CHECK ( math::ceil(-5.0) == -5 );
    BOOST_CHECK ( math::ceil(5.00001) == 6 );
    BOOST_CHECK ( math::ceil(5.99999) == 6 );

    BOOST_CHECK(math::fuzzy_compare(math::fractional( 5.6),     .6) );
    BOOST_CHECK(math::fuzzy_compare(math::fractional(-5.6),    -.6) );
    BOOST_CHECK(math::fuzzy_compare(math::fractional( 5.5),     .5) );
    BOOST_CHECK(math::fuzzy_compare(math::fractional(-5.5),    -.5) );
    BOOST_CHECK(math::fuzzy_compare(math::fractional( 5.4),     .4) );
    BOOST_CHECK(math::fuzzy_compare(math::fractional(-5.4),    -.4) );
    BOOST_CHECK(math::fuzzy_compare(math::fractional( 5.0),    0  ) );
    BOOST_CHECK(math::fuzzy_compare(math::fractional(-5.0),    0  ) );
    BOOST_CHECK(math::fuzzy_compare(math::fractional( 5.00001), .00001) );
    BOOST_CHECK(math::fuzzy_compare(math::fractional( 5.99999), .99999) );

    BOOST_CHECK(math::fuzzy_compare(math::positive_fractional( 5.6),     .6) );
    BOOST_CHECK(math::fuzzy_compare(math::positive_fractional(-5.6),     .4) );
    BOOST_CHECK(math::fuzzy_compare(math::positive_fractional( 5.5),     .5) );
    BOOST_CHECK(math::fuzzy_compare(math::positive_fractional(-5.5),     .5) );
    BOOST_CHECK(math::fuzzy_compare(math::positive_fractional( 5.4),     .4) );
    BOOST_CHECK(math::fuzzy_compare(math::positive_fractional(-5.4),     .6) );
    BOOST_CHECK(math::fuzzy_compare(math::positive_fractional( 5.0),    0  ) );
    BOOST_CHECK(math::fuzzy_compare(math::positive_fractional(-5.0),    0  ) );
    BOOST_CHECK(math::fuzzy_compare(math::positive_fractional( 5.00001), .00001) );
    BOOST_CHECK(math::fuzzy_compare(math::positive_fractional( 5.99999), .99999) );
}

struct StableInt
{
    int value;
    int order;

    StableInt(int value, int order) : value(value), order(order) {}
    
    bool operator<(const StableInt& oth) const
    {
        return value < oth.value;
    }
};

BOOST_AUTO_TEST_CASE( test_minmax )
{
    BOOST_CHECK(math::min(1, 2) == 1);
    BOOST_CHECK(math::min(2, 1) == 1);
    BOOST_CHECK(math::min(StableInt{1, 1}, StableInt{1, 2}).order == 1);
    BOOST_CHECK(math::min(1, 2, 3) == 1);
    BOOST_CHECK(math::min(2, 1, 3) == 1);
    BOOST_CHECK(math::min(3, 2, 1) == 1);
    BOOST_CHECK(math::min(StableInt{2, 1}, StableInt{1, 1}, StableInt{3, 1}, StableInt{1, 2}).order == 1);

    BOOST_CHECK(math::max(1, 2) == 2);
    BOOST_CHECK(math::max(2, 1) == 2);
    BOOST_CHECK(math::max(StableInt{1, 1}, StableInt{1, 2}).order == 1);
    BOOST_CHECK(math::max(1, 2, 3) == 3);
    BOOST_CHECK(math::max(2, 3, 1) == 3);
    BOOST_CHECK(math::max(3, 2, 1) == 3);
    BOOST_CHECK(math::max(StableInt{2, 1}, StableInt{4, 1}, StableInt{3, 1}, StableInt{4, 2}).order == 1);
}


BOOST_AUTO_TEST_CASE( test_abs )
{
    BOOST_CHECK(math::abs(1) == 1);
    BOOST_CHECK(math::abs(-1) == 1);
    BOOST_CHECK(math::abs(0) == 0);
}

BOOST_AUTO_TEST_CASE( test_normalize )
{
    BOOST_CHECK(math::fuzzy_compare(math::normalize<double>(-3, -3, 17), 0));
    BOOST_CHECK(math::fuzzy_compare(math::normalize<double>(17, -3, 17), 1));
    BOOST_CHECK(math::fuzzy_compare(math::normalize<double>(7, -3, 17), 0.5));
    BOOST_CHECK(math::fuzzy_compare(math::normalize<double>(9, -3, 17), 0.6));
}

BOOST_AUTO_TEST_CASE( test_denormalize )
{
    BOOST_CHECK(math::fuzzy_compare(math::denormalize<double>(0, -3, 17), -3));
    BOOST_CHECK(math::fuzzy_compare(math::denormalize<double>(1, -3, 17), 17));
    BOOST_CHECK(math::fuzzy_compare(math::denormalize<double>(0.5, -3, 17), 7));
    BOOST_CHECK(math::fuzzy_compare(math::denormalize<double>(0.6, -3, 17), 9));
}

BOOST_AUTO_TEST_CASE( test_bound )
{
    BOOST_CHECK(math::bound(-5, -10, 5) == -5);
    BOOST_CHECK(math::bound(-5, -5, 5) == -5);
    BOOST_CHECK(math::bound(-5, 0, 5) == 0);
    BOOST_CHECK(math::bound(-5, 1, 5) == 1);
    BOOST_CHECK(math::bound(-5, 5, 5) == 5);
    BOOST_CHECK(math::bound(-5, 10, 5) == 5);

    BOOST_CHECK(math::fuzzy_compare(math::bound(-5, -10.0, 5), -5));
    BOOST_CHECK(math::fuzzy_compare(math::bound(-5, -5.0, 5), -5));
    BOOST_CHECK(math::fuzzy_compare(math::bound(-5, 0.0, 5), 0));
    BOOST_CHECK(math::fuzzy_compare(math::bound(-5, 1.0, 5), 1));
    BOOST_CHECK(math::fuzzy_compare(math::bound(-5, 5.0, 5), 5));
    BOOST_CHECK(math::fuzzy_compare(math::bound(-5, 10.0, 5), 5));
}

BOOST_AUTO_TEST_CASE( test_linear_interpolation )
{
    BOOST_CHECK_EQUAL( math::linear_interpolation(3, 23, 0), 3 );
    BOOST_CHECK_EQUAL( math::linear_interpolation(3, 23, 0.5), 13 );
    BOOST_CHECK_EQUAL( math::linear_interpolation(3, 23, 1), 23 );
}
