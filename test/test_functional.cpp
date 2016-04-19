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

#define BOOST_TEST_MODULE Test_Functional
#include <boost/test/unit_test.hpp>

#include "melanolib/utils/functional.hpp"
#include "melanolib/utils/type_utils.hpp"

using namespace melanolib;

BOOST_AUTO_TEST_CASE( test_callback )
{
    int i = 0;
    auto lambda = [&i](int j){ i = j; };
    std::function<void (int)> functor;

    callback(functor, 7);
    BOOST_CHECK_EQUAL( i, 0 );

    functor = lambda;
    callback(functor, 7);
    BOOST_CHECK_EQUAL( i, 7 );
}

BOOST_AUTO_TEST_CASE( test_call_range )
{
    std::vector<int> numbers = { 1, 2, 3 };
    BOOST_CHECK_EQUAL( range_call_overload<int>(std::accumulate, numbers, 0), 6 );

    const auto& const_numbers = numbers;
    BOOST_CHECK_EQUAL( range_call_overload<int>(std::accumulate, const_numbers, 0), 6 );
}
