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

#define BOOST_TEST_MODULE Test_ColorRange

#include <vector>
#include <boost/test/unit_test.hpp>

#include "melanolib/color/color_range.hpp"

using namespace melanolib::color;

BOOST_AUTO_TEST_CASE( test_size )
{
    ColorRange range({0, 0, 0}, {255, 255, 255}, 3);

    BOOST_CHECK(range.size() == 3);

    std::size_t count = 0;
    for ( auto c : range )
        count++, (void)c;

    BOOST_CHECK(count == range.size());

    range.resize(5);
    BOOST_CHECK(range.size() == 5);
    count = 0;
    for ( auto c : range )
        count++, (void)c;
    BOOST_CHECK(count == range.size());
}

BOOST_AUTO_TEST_CASE( test_iterator_category )
{
    using iterator = ColorRange::iterator;
    BOOST_CHECK((std::is_convertible<
        std::iterator_traits<iterator>::iterator_category,
        std::random_access_iterator_tag>::value));
}

BOOST_AUTO_TEST_CASE( test_colors )
{
    ColorRange range({0, 0, 0}, {255, 255, 255}, 3);
    std::vector<Color> colors(range.begin(), range.end());
    BOOST_CHECK(colors[0] == Color(0, 0, 0));
    BOOST_CHECK(colors[1] == Color(128, 128, 128));
    BOOST_CHECK(colors[2] == Color(255, 255, 255));

    range.resize(5);
    BOOST_CHECK(range[0] == Color(0, 0, 0));
    BOOST_CHECK(range[1] == Color(64, 64, 64));
    BOOST_CHECK(range[2] == Color(128, 128, 128));
    BOOST_CHECK(range[3] == Color(191, 191, 191));
    BOOST_CHECK(range[4] == Color(255, 255, 255));

    range.resize(2);
    BOOST_CHECK(range[0] == Color(0, 0, 0));
    BOOST_CHECK(range[1] == Color(255, 255, 255));

    range.resize(1);
    BOOST_CHECK(range[0] == Color(0, 0, 0));
}

BOOST_AUTO_TEST_CASE( test_iterator_cmp )
{
    ColorRange range1({0, 0, 0}, {255, 255, 255}, 3);
    ColorRange range2 = range1;
    ColorRange::iterator invalid;
    ColorRange::iterator b1 = range1.begin();
    ColorRange::iterator b2 = range2.begin();
    ColorRange::iterator e1 = range1.end();

    BOOST_CHECK(b1 == range1.begin());
    BOOST_CHECK(b1 + 3 == e1);
    BOOST_CHECK(invalid == ColorRange::iterator());
    BOOST_CHECK(e1 == range1.end());

    BOOST_CHECK(invalid != b1);
    BOOST_CHECK(b1 != invalid);
    BOOST_CHECK(b1 != b2);
    BOOST_CHECK(b1 != e1);

    BOOST_CHECK(b1 < e1);
    BOOST_CHECK(!(b1 < b1));
    BOOST_CHECK(!(e1 < b1));
    BOOST_CHECK(!(e1 < e1));

    BOOST_CHECK(b1 <= e1);
    BOOST_CHECK(b1 <= b1);
    BOOST_CHECK(!(e1 <= b1));
    BOOST_CHECK(e1 <= e1);

    BOOST_CHECK(!(b1 >= e1));
    BOOST_CHECK(b1 >= b1);
    BOOST_CHECK(e1 >= b1);
    BOOST_CHECK(e1 >= e1);

    BOOST_CHECK(!(b1 > e1));
    BOOST_CHECK(!(b1 > b1));
    BOOST_CHECK(e1 > b1);
    BOOST_CHECK(!(e1 > e1));
}

BOOST_AUTO_TEST_CASE( test_iterator_offsets )
{
    ColorRange range({0, 0, 0}, {255, 255, 255}, 3);
    ColorRange::iterator b = range.begin();
    ColorRange::iterator e = range.end();

    BOOST_CHECK(std::distance(b, e) == 3);
    BOOST_CHECK(b + 3 == e);
    BOOST_CHECK(3 + b == e);
    BOOST_CHECK(-3 + e == b);
    BOOST_CHECK(e - 3 == b);
    BOOST_CHECK(e - b == 3);

    BOOST_CHECK(b[2] == Color(255, 255, 255));
    BOOST_CHECK(e[-1] == Color(255, 255, 255));

    ColorRange::iterator b1 = range.begin();
    BOOST_CHECK(++b1 == b+1);
    BOOST_CHECK(b1++ == b+1);
    BOOST_CHECK(b1 == b+2);

    BOOST_CHECK(--b1 == b+1);
    BOOST_CHECK(b1-- == b+1);
    BOOST_CHECK(b1 == b);
}

BOOST_AUTO_TEST_CASE( test_iterator_offset_cap )
{
    ColorRange range({0, 0, 0}, {255, 255, 255}, 3);
    ColorRange::iterator b = range.begin();
    ColorRange::iterator e = range.end();

    BOOST_CHECK(b + 300 == e);
    BOOST_CHECK(e - 300 == b);

    ColorRange::iterator b1 = range.begin();
    BOOST_CHECK(--b1 == b);
    ColorRange::iterator e1 = range.end();
    BOOST_CHECK(++e1 == e);
}

BOOST_AUTO_TEST_CASE( test_iterator_valid )
{
    BOOST_CHECK(!ColorRange::iterator().valid());

    ColorRange range({0, 0, 0}, {255, 255, 255}, 5);
    BOOST_CHECK(range.end().valid());
    BOOST_CHECK(range.begin().valid());

    ColorRange::iterator b = range.begin();
    ColorRange::iterator e = range.end();
    range.resize(2);
    BOOST_CHECK(b.valid());
    BOOST_CHECK(!e.valid());

}

BOOST_AUTO_TEST_CASE( test_hsv )
{
    repr::HSVf c1{0, 0, 0};
    repr::HSVf c2{1, 1, 1};
    BasicColorRange<repr::HSVf> range(c1, c2, 3);
    auto it = range.begin();
    BOOST_CHECK((*it).vec() == c1.vec());
    BOOST_CHECK((*++it).vec() == repr::HSVf(.5, .5, .5).vec());
    BOOST_CHECK((*++it).vec() == c2.vec());
    ++it; ++it;

    range.resize(1);
    it = range.begin();
    BOOST_CHECK((*it).vec() == c1.vec());
}
