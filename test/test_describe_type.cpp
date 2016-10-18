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

#define BOOST_TEST_MODULE Test_describe_type

#include <boost/test/unit_test.hpp>

#include "melanolib/utils/describe_type.hpp"

using namespace melanolib;

struct Foo
{
    virtual ~Foo(){}
    int bar;
    void baz() const {}
};

BOOST_AUTO_TEST_CASE( test_describe_type )
{
    BOOST_CHECK_EQUAL(
        describe_type<const int** const *&>(),
        std::string("reference to pointer to const pointer to pointer to const built-in ")
        + typeid(int).name()
    );

    BOOST_CHECK_EQUAL(
        describe_type<decltype(&Foo::bar)>(),
        std::string("pointer to member of polymorphic class ")
        + typeid(Foo).name() + " of type built-in " + typeid(int).name()
    );

    BOOST_CHECK_EQUAL(
        describe_type<decltype(&Foo::baz)>(),
        std::string("pointer to member function of polymorphic class ") + typeid(Foo).name()
    );
}
