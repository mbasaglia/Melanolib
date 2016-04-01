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

#define BOOST_TEST_MODULE Test_GSL

#include <boost/test/unit_test.hpp>

#include "melanolib/utils/gsl.hpp"
#include <array>
#include <vector>
#include <list>
#include <limits>

namespace gsl = melanolib::gsl;

BOOST_AUTO_TEST_CASE( test_owner )
{
    gsl::owner<int*> owner = new int{42};
    BOOST_CHECK(owner);
    BOOST_CHECK(*owner == 42);
    int* not_owner = owner;
    BOOST_CHECK(*not_owner == 42);
    BOOST_CHECK(not_owner == owner);
    delete owner;
}

unsigned subtract(unsigned a, unsigned b)
{
    gsl::Expects(b > 0);
    auto r = a - b;
    gsl::Ensures(r < a);
    return r;
}

BOOST_AUTO_TEST_CASE( test_pre_post )
{
    BOOST_CHECK(subtract(5, 4) == 1);
    BOOST_CHECK_THROW(subtract(5, 0), std::exception);
    BOOST_CHECK_THROW(subtract(5, 6), std::exception);
}

BOOST_AUTO_TEST_CASE( test_not_null )
{
    int* null = nullptr;
    int val = 5;
    int* not_null = &val;
    gsl::not_null<int*> ptr(&val);
    int* maybe_null;
    BOOST_CHECK_THROW( gsl::not_null<int*> foo(null), std::exception);
    BOOST_CHECK( gsl::not_null<int*>(not_null) == not_null );
    BOOST_CHECK( ptr );
    BOOST_CHECK( *ptr == val );
    BOOST_CHECK( ptr == &val );
    BOOST_CHECK( ptr != null );
    BOOST_CHECK_THROW( ptr = null, std::exception);
    BOOST_CHECK( ptr = not_null );
    BOOST_CHECK( ptr == &val );
    BOOST_CHECK( maybe_null = ptr );
    BOOST_CHECK( *maybe_null = val );
    /// \todo Test with smart pointers
}

int fifth(const gsl::array_view<int>& array)
{
    return array[5];
}

BOOST_AUTO_TEST_CASE( test_array_view )
{
    using array_view = gsl::array_view<int>;
    std::vector<int> intv = { 1, 2, 3, 4, 5, 6, 7, 8 };
    std::array<int, 8> inta = { 1, 2, 3, 4, 5, 6, 7, 8 };
    std::list<int> intl = { 1, 2, 3, 4, 5, 6, 7, 8 };
    int intca[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };

    BOOST_CHECK(fifth(intv) == 6);
    BOOST_CHECK(fifth(inta) == 6);
    BOOST_CHECK(fifth(intca) == 6);
    BOOST_CHECK(fifth({intca, 8}) == 6);
    BOOST_CHECK_THROW(fifth({intl.begin(), intl.end()}), std::exception);
    BOOST_CHECK_THROW(fifth({intca, 4}), std::exception);

    array_view view = intv;
    array_view view2{intv.begin(), intv.end()};
    BOOST_CHECK(view);
    BOOST_CHECK(view.begin() == &intv.front());
    BOOST_CHECK(view.end() == &intv.back()+1);
    BOOST_CHECK(view.size() == intv.size());
    BOOST_CHECK(view == view2);
    view = inta;
    BOOST_CHECK(view.begin() == &inta.front());
    BOOST_CHECK(view.end() == &inta.back()+1);
    BOOST_CHECK(view.size() == inta.size());;
    BOOST_CHECK(view != view2);
    view = {intca, std::size_t(0)};
    BOOST_CHECK(view.begin() == intca);
    BOOST_CHECK(view.size() == 0);
}

BOOST_AUTO_TEST_CASE( test_finally )
{
    int i = 0;
    auto lambda = [&i]{++i;};

    {
        auto f = gsl::finally(lambda);
        BOOST_CHECK(i == 0);
    }

    BOOST_CHECK(i == 1);
}

BOOST_AUTO_TEST_CASE( test_narrow )
{
    long big = std::numeric_limits<char>::max();
    big *= 2;
    BOOST_CHECK_THROW(gsl::narrow<char>(big), gsl::narrowing_error);
    BOOST_CHECK(gsl::narrow<int>(big) == big);
}
