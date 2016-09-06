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

#define BOOST_TEST_MODULE Test_Gradient

#include <boost/test/unit_test.hpp>

#include "melanolib/color/gradient.hpp"

using namespace melanolib::color;

BOOST_AUTO_TEST_CASE( test_container_access )
{
    Gradient gradient({{255, 0, 0}, {255, 255, 255}, {0, 0, 255}});
    const Gradient& const_gradient = gradient;

    BOOST_CHECK(!gradient.empty());
    BOOST_CHECK(gradient.size() == 3);

    BOOST_CHECK(gradient.colors() == const_gradient.colors());
    BOOST_CHECK(gradient.colors().size() == 3);

    BOOST_CHECK(gradient.colors().begin() == gradient.begin());
    BOOST_CHECK(gradient.colors().end() == gradient.end());
    BOOST_CHECK(gradient.colors().cbegin() == gradient.cbegin());
    BOOST_CHECK(gradient.colors().cend() == gradient.cend());
    BOOST_CHECK(const_gradient.begin() == gradient.begin());
    BOOST_CHECK(const_gradient.end() == gradient.end());

    gradient.colors().pop_back();
    BOOST_CHECK(gradient.size() == 2);

    gradient = Gradient(4, Color{255, 0, 0});
    BOOST_CHECK(gradient.size() == 4);
}

BOOST_AUTO_TEST_CASE( test_color_in_range )
{
    Color a{255, 0, 0};
    Color b{255, 255, 255};
    Color c{0, 0, 255};
    Gradient gradient({a, b, c});

    BOOST_CHECK(gradient.color(0) == a);
    BOOST_CHECK(gradient.color(0.5) == b);
    BOOST_CHECK(gradient.color(1) == c);

    BOOST_CHECK(gradient.color(0.125) == a.blend(b, 0.25));
    BOOST_CHECK(gradient.color(0.250) == a.blend(b, 0.50));
    BOOST_CHECK(gradient.color(0.375) == a.blend(b, 0.75));

    BOOST_CHECK(gradient.color(0.625) == b.blend(c, 0.25));
    BOOST_CHECK(gradient.color(0.750) == b.blend(c, 0.50));
    BOOST_CHECK(gradient.color(0.875) == b.blend(c, 0.75));
}

BOOST_AUTO_TEST_CASE( test_color_clamp )
{
    Color a{255, 0, 0};
    Color b{255, 255, 255};
    Color c{0, 0, 255};
    Gradient gradient({a, b, c});

    BOOST_CHECK(gradient.color(-0.25) == a);
    BOOST_CHECK(gradient.color(-0.50) == a);
    BOOST_CHECK(gradient.color(-0.75) == a);
    BOOST_CHECK(gradient.color(-1.25) == a);
    BOOST_CHECK(gradient.color(-2.25) == a);
    BOOST_CHECK(gradient.color(-3.25) == a);

    BOOST_CHECK(gradient.color(1.25) == c);
    BOOST_CHECK(gradient.color(1.50) == c);
    BOOST_CHECK(gradient.color(1.75) == c);
    BOOST_CHECK(gradient.color(2.25) == c);
    BOOST_CHECK(gradient.color(3.25) == c);
    BOOST_CHECK(gradient.color(4.25) == c);
}

BOOST_AUTO_TEST_CASE( test_color_wrap )
{
    Color a{255, 0, 0};
    Color b{255, 255, 255};
    Color c{0, 0, 255};
    Gradient gradient({a, b, c});
    auto mode = Gradient::OverflowMode::Wrap;

    BOOST_CHECK(gradient.color(-0.25, mode) == gradient.color(0.75));
    BOOST_CHECK(gradient.color(-0.50, mode) == gradient.color(0.50));
    BOOST_CHECK(gradient.color(-0.75, mode) == gradient.color(0.25));
    BOOST_CHECK(gradient.color(-1.25, mode) == gradient.color(0.75));
    BOOST_CHECK(gradient.color(-2.25, mode) == gradient.color(0.75));
    BOOST_CHECK(gradient.color(-3.25, mode) == gradient.color(0.75));

    BOOST_CHECK(gradient.color(1.25, mode) == gradient.color(0.25));
    BOOST_CHECK(gradient.color(1.50, mode) == gradient.color(0.50));
    BOOST_CHECK(gradient.color(1.75, mode) == gradient.color(0.75));
    BOOST_CHECK(gradient.color(2.25, mode) == gradient.color(0.25));
    BOOST_CHECK(gradient.color(3.25, mode) == gradient.color(0.25));
    BOOST_CHECK(gradient.color(4.25, mode) == gradient.color(0.25));
}

BOOST_AUTO_TEST_CASE( test_color_mirror )
{
    Color a{255, 0, 0};
    Color b{255, 255, 255};
    Color c{0, 0, 255};
    Gradient gradient({a, b, c});
    auto mode = Gradient::OverflowMode::Mirror;

    BOOST_CHECK(gradient.color(-0.25, mode) == gradient.color(0.25));
    BOOST_CHECK(gradient.color(-0.50, mode) == gradient.color(0.50));
    BOOST_CHECK(gradient.color(-0.75, mode) == gradient.color(0.75));
    BOOST_CHECK(gradient.color(-1.25, mode) == gradient.color(0.25));
    BOOST_CHECK(gradient.color(-2.25, mode) == gradient.color(0.75));
    BOOST_CHECK(gradient.color(-3.25, mode) == gradient.color(0.25));

    BOOST_CHECK(gradient.color(1.25, mode) == gradient.color(0.75));
    BOOST_CHECK(gradient.color(1.50, mode) == gradient.color(0.50));
    BOOST_CHECK(gradient.color(1.75, mode) == gradient.color(0.25));
    BOOST_CHECK(gradient.color(2.25, mode) == gradient.color(0.25));
    BOOST_CHECK(gradient.color(3.25, mode) == gradient.color(0.75));
    BOOST_CHECK(gradient.color(4.25, mode) == gradient.color(0.25));
}

BOOST_AUTO_TEST_CASE( test_range )
{
    Color a{255, 0, 0};
    Color b{255, 255, 255};
    Color c{0, 0, 255};
    Gradient gradient({a, b, c});

    std::size_t count = 0;
    for ( Color c : gradient.range(7) )
    {
        ++count;
        (void)c;
    }
    BOOST_CHECK_EQUAL(count, 7);

    auto container_iter = gradient.begin();
    auto gradient_iter = gradient.range(3).begin();

    BOOST_CHECK(container_iter[0] == gradient_iter[0]);
    BOOST_CHECK(container_iter[1] == gradient_iter[1]);
    BOOST_CHECK(container_iter[2] == gradient_iter[2]);

    gradient_iter = gradient.range(9).begin();
    BOOST_CHECK(gradient_iter[0] == a);
    BOOST_CHECK(gradient_iter[1] == a.blend(b, 0.25));
    BOOST_CHECK(gradient_iter[2] == a.blend(b, 0.50));
    BOOST_CHECK(gradient_iter[3] == a.blend(b, 0.75));
    BOOST_CHECK(gradient_iter[4] == b);
    BOOST_CHECK(gradient_iter[5] == b.blend(c, 0.25));
    BOOST_CHECK(gradient_iter[6] == b.blend(c, 0.50));
    BOOST_CHECK(gradient_iter[7] == b.blend(c, 0.75));
    BOOST_CHECK(gradient_iter[8] == c);
}
