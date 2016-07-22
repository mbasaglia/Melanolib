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

#include "melanolib/utils/movable.hpp"

using namespace melanolib;

struct Foo
{
    Foo(int i) : i ( i ) {}
    Foo(const Foo&) = delete;
    Foo& operator=(const Foo&) = delete;

    int i;
};

BOOST_AUTO_TEST_CASE( test_ctor )
{
    melanolib::Movable<Foo> foo(123);
    BOOST_CHECK(foo->i == 123);
}

BOOST_AUTO_TEST_CASE( test_access )
{
    melanolib::Movable<Foo> foo(123);
    BOOST_CHECK(((Foo&)foo).i == 123);
    BOOST_CHECK(foo->i == 123);
    BOOST_CHECK((*foo).i == 123);
}

BOOST_AUTO_TEST_CASE( test_const_access )
{
    const melanolib::Movable<Foo> foo(123);
    BOOST_CHECK(((const Foo&)foo).i == 123);
    BOOST_CHECK(foo->i == 123);
    BOOST_CHECK((*foo).i == 123);
}

BOOST_AUTO_TEST_CASE( test_move )
{
    melanolib::Movable<Foo> foo(123);
    melanolib::Movable<Foo> bar = std::move(foo);
    BOOST_CHECK(bar->i == 123);
}
