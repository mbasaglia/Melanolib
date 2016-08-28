/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2016 Mattia Basaglia
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
#define BOOST_TEST_MODULE Test_TypeUtils

#include <boost/test/unit_test.hpp>

#include "melanolib/utils/type_utils.hpp"

using namespace melanolib;

struct ExplicitBool
{
    explicit operator bool() const { return false; }
};

struct ImplicitBool
{
    operator bool() const { return false; }
};

struct NotBool
{
};

BOOST_AUTO_TEST_CASE( test_explicitly_convertible )
{
    BOOST_CHECK( (ExplicitlyConvertible<bool, bool>::value) );
    BOOST_CHECK( (ExplicitlyConvertible<int, bool>::value) );
    BOOST_CHECK( (ExplicitlyConvertible<ImplicitBool, bool>::value) );
    BOOST_CHECK( (ExplicitlyConvertible<ExplicitBool, bool>::value) );
    BOOST_CHECK( (!ExplicitlyConvertible<NotBool, bool>::value) );
}

BOOST_AUTO_TEST_CASE( test_is_callable_funcptr )
{
    BOOST_CHECK( (IsCallable<void(*)()>::value) );
    BOOST_CHECK( !(IsCallable<void(*)(), int>::value) );
    BOOST_CHECK( !(IsCallable<void(*)(), void, int>::value) );

    BOOST_CHECK( !(IsCallable<int(*)()>::value) );
    BOOST_CHECK( (IsCallable<int(*)(), int>::value) );
    BOOST_CHECK( !(IsCallable<int(*)(), int, int>::value) );

    BOOST_CHECK( !(IsCallable<void(*)(int)>::value) );
    BOOST_CHECK( !(IsCallable<void(*)(int), int>::value) );
    BOOST_CHECK( (IsCallable<void(*)(int), void, int>::value) );

    BOOST_CHECK((IsCallable<int(*)(), float>::value) );
}

BOOST_AUTO_TEST_CASE( test_is_callable_lambda )
{
    auto lambdav = [](){};
    BOOST_CHECK( (IsCallable<decltype(lambdav)>::value) );
    BOOST_CHECK( !(IsCallable<decltype(lambdav), int>::value) );
    BOOST_CHECK( !(IsCallable<decltype(lambdav), void, int>::value) );

    auto lambdai = []()->int{ return 0; };
    BOOST_CHECK( !(IsCallable<decltype(lambdai)>::value) );
    BOOST_CHECK( (IsCallable<decltype(lambdai), int>::value) );
    BOOST_CHECK( !(IsCallable<decltype(lambdai), void, int>::value) );
    BOOST_CHECK( (IsCallable<decltype(lambdai), float>::value) );

    auto lambdavi = [](int){};
    BOOST_CHECK( !(IsCallable<decltype(lambdavi)>::value) );
    BOOST_CHECK( !(IsCallable<decltype(lambdavi), int>::value) );
    BOOST_CHECK( (IsCallable<decltype(lambdavi), void, int>::value) );
}

BOOST_AUTO_TEST_CASE( test_is_callable_anyreturn_funcptr )
{
    BOOST_CHECK( (IsCallableAnyReturn<void(*)()>::value) );
    BOOST_CHECK( !(IsCallableAnyReturn<void(*)(), int>::value) );

    BOOST_CHECK( (IsCallableAnyReturn<int(*)()>::value) );
    BOOST_CHECK( !(IsCallableAnyReturn<int(*)(), int>::value) );

    BOOST_CHECK( !(IsCallableAnyReturn<void(*)(int)>::value) );
    BOOST_CHECK( (IsCallableAnyReturn<void(*)(int), int>::value) );
}

BOOST_AUTO_TEST_CASE( test_is_callable_anyreturn_lambda )
{
    auto lambdav = [](){};
    BOOST_CHECK( (IsCallableAnyReturn<decltype(lambdav)>::value) );
    BOOST_CHECK( !(IsCallableAnyReturn<decltype(lambdav), int>::value) );

    auto lambdai = []()->int{ return 0; };
    BOOST_CHECK( (IsCallableAnyReturn<decltype(lambdai)>::value) );
    BOOST_CHECK( !(IsCallableAnyReturn<decltype(lambdai), int>::value) );

    auto lambdavi = [](int){};
    BOOST_CHECK( !(IsCallableAnyReturn<decltype(lambdavi)>::value) );
    BOOST_CHECK( (IsCallableAnyReturn<decltype(lambdavi), int>::value) );
}
