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

#include "melanolib/math/vector.hpp"

using namespace melanolib::math;

using Vec = Vector<int, 2>;

Vec vec(int a, int b)
{
    return Vec{a,b};
}

BOOST_AUTO_TEST_CASE( test_default_ctor )
{
    Vec v;
    BOOST_CHECK_EQUAL(v[0], 0);
    BOOST_CHECK_EQUAL(v[1], 0);
}

BOOST_AUTO_TEST_CASE( test_brace_ctor )
{
    Vec a{1, 2};
    BOOST_CHECK_EQUAL(a[0], 1);
    BOOST_CHECK_EQUAL(a[1], 2);

    Vec b = {1, 2};
    BOOST_CHECK_EQUAL(b[0], 1);
    BOOST_CHECK_EQUAL(b[1], 2);

    Vec c({1, 2});
    BOOST_CHECK_EQUAL(c[0], 1);
    BOOST_CHECK_EQUAL(c[1], 2);
}

BOOST_AUTO_TEST_CASE( test_copy_ctor )
{
    Vec a{1, 2};
    Vec b = a;
    BOOST_CHECK_EQUAL(b[0], a[0]);
    BOOST_CHECK_EQUAL(b[1], a[1]);
}

BOOST_AUTO_TEST_CASE( test_sequence )
{
    Vec a{1, 2};
    const Vec b = a;

    BOOST_CHECK_EQUAL(b[0], a[0]);

    BOOST_CHECK_NE(b.data(), a.data());

    BOOST_CHECK_EQUAL(a.data(), a.begin());
    BOOST_CHECK_EQUAL(b.data(), b.begin());
    BOOST_CHECK_EQUAL(a.begin(), a.cbegin());
    BOOST_CHECK_EQUAL(a.data() + 2, a.end());
    BOOST_CHECK_EQUAL(b.data() + 2, b.end());
    BOOST_CHECK_EQUAL(a.end(), a.cend());

    BOOST_CHECK_EQUAL(a.rbegin().base(), a.end());
    BOOST_CHECK_EQUAL(a.crbegin().base(), a.end());
    BOOST_CHECK_EQUAL(a.rend().base(), a.begin());
    BOOST_CHECK_EQUAL(a.crend().base(), a.begin());
    BOOST_CHECK_EQUAL(b.rbegin().base(), b.end());
    BOOST_CHECK_EQUAL(b.rend().base(), b.begin());

    BOOST_CHECK(!a.empty());
    BOOST_CHECK_EQUAL(a.size(), 2);
    BOOST_CHECK_EQUAL(a.max_size(), 2);
}

BOOST_AUTO_TEST_CASE( test_sum )
{
    BOOST_CHECK(vec(1,2) + vec(10,20) == vec(11, 22));
    BOOST_CHECK(vec(11,22) - vec(10,20) == vec(1, 2));

    BOOST_CHECK(+vec(1,2) == vec(1, 2));
    BOOST_CHECK(-vec(1,2) == vec(-1, -2));
}

BOOST_AUTO_TEST_CASE( test_multiplication )
{
    BOOST_CHECK(vec(1,2) * 11 == vec(11, 22));
    BOOST_CHECK(11 * vec(1,2) == vec(11, 22));
    BOOST_CHECK(vec(11,22) / 11 == vec(1, 2));
}

BOOST_AUTO_TEST_CASE( test_comparison )
{
    BOOST_CHECK(vec(1, 2) == vec(1, 2));
    BOOST_CHECK(vec(1, 2) != vec(1, 20));
    BOOST_CHECK(vec(1, 2) != vec(10, 2));


    BOOST_CHECK(vec(1, 2) < vec(10, 2));
    BOOST_CHECK(vec(1, 2) <= vec(10, 2));
    BOOST_CHECK(vec(1, 2) <= vec(1, 2));

    BOOST_CHECK(vec(10, 2) > vec(1, 2));
    BOOST_CHECK(vec(10, 2) >= vec(1, 2));
    BOOST_CHECK(vec(1, 2) >= vec(1, 2));

    BOOST_CHECK(!(vec(10, 2) < vec(1, 2)));
    BOOST_CHECK(!(vec(1, 2) < vec(1, 2)));
    BOOST_CHECK(!(vec(10, 2) <= vec(1, 2)));

    BOOST_CHECK(!(vec(1, 2) > vec(10, 2)));
    BOOST_CHECK(!(vec(1, 2) > vec(1, 2)));
    BOOST_CHECK(!(vec(1, 2) >= vec(10, 2)));
}
