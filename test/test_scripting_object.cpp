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

#include "melanolib/scripting/object.hpp"

#define BOOST_TEST_MODULE Test_String

#include <boost/test/unit_test.hpp>

using namespace melanolib::scripting;

class SomeClass
{
public:
    std::string data_member = "data member";
    std::string member_function() const
    {
        return "member function";
    }

    SomeClass other_object() const
    {
        SomeClass value;
        value.data_member = "other object data";
        return value;
    }
};

BOOST_AUTO_TEST_CASE( test_to_string )
{
    Namespace ns;
    ns.register_class<SomeClass>("SomeClass")
        .add("data", &SomeClass::data_member)
        .add("method", &SomeClass::member_function)
    ;
    ns.register_class<std::string>("string");

    BOOST_CHECK_EQUAL( ns.object(std::string("foo")).to_string(), "foo" );
    BOOST_CHECK_EQUAL( ns.object(SomeClass()).to_string(), "SomeClass" );
}

BOOST_AUTO_TEST_CASE( test_member_access )
{
    Namespace ns;
    ns.register_class<SomeClass>("SomeClass")
        .add("data", &SomeClass::data_member)
        .add("method", &SomeClass::member_function)
        .add("other_object", &SomeClass::other_object)
        .add("functor1", []() { return std::string("functor value"); })
        .add("fixed_value", std::string("some value"))
        .add("functor2", [](const SomeClass& obj) {
            return obj.data_member + " something";
        })
    ;
    ns.register_class<std::string>("string");

    auto object = ns.object(SomeClass());
    BOOST_CHECK_EQUAL( object.get({"data"}).to_string(), "data member" );
    BOOST_CHECK_EQUAL( object.get({"method"}).to_string(), "member function");
    BOOST_CHECK_EQUAL( object.get({"functor1"}).to_string(), "functor value");
    BOOST_CHECK_EQUAL( object.get({"functor2"}).to_string(), "data member something");
    BOOST_CHECK_THROW( object.get({"not_found"}).to_string(), MemberNotFound );
    BOOST_CHECK_EQUAL( object.get({"other_object", "data"}).to_string(), "other object data" );
}

BOOST_AUTO_TEST_CASE( test_class_not_found )
{
    Namespace ns;
    ns.register_class<SomeClass>("SomeClass")
        .add("data", &SomeClass::data_member)
        .add("method", &SomeClass::member_function)
    ;
    BOOST_CHECK_THROW( ns.object(SomeClass()).get({"data"}), ClassNotFound );
}
