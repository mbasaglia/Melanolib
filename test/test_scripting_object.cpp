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
    ns.register_type<SomeClass>("SomeClass")
        .add_readonly("data", &SomeClass::data_member)
        .add_readonly("method", &SomeClass::member_function)
    ;
    ns.register_type<std::string>("string");

    BOOST_CHECK_EQUAL( ns.object(std::string("foo")).to_string(), "foo" );
    BOOST_CHECK_EQUAL( ns.object(SomeClass()).to_string(), "SomeClass" );
}

BOOST_AUTO_TEST_CASE( test_member_access )
{
    Namespace ns;
    ns.register_type<SomeClass>("SomeClass")
        .add_readonly("data", &SomeClass::data_member)
        .add_readonly("method", &SomeClass::member_function)
        .add_readonly("other_object", &SomeClass::other_object)
        .add_readonly("functor1", []() { return std::string("functor value"); })
        .add_readonly("fixed_value", std::string("some value"))
        .add_readonly("functor2", [](const SomeClass& obj) {
            return obj.data_member + " something";
        })
    ;
    ns.register_type<std::string>("string");

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
    ns.register_type<SomeClass>("SomeClass")
        .add_readonly("data", &SomeClass::data_member)
        .add_readonly("method", &SomeClass::member_function)
    ;
    BOOST_CHECK_THROW( ns.object(SomeClass()).get({"data"}), TypeError );
}

BOOST_AUTO_TEST_CASE( test_builtin )
{
    Namespace ns;
    ns.register_type<int>();

    BOOST_CHECK_EQUAL( ns.object(123).to_string(), "123" );
}

BOOST_AUTO_TEST_CASE( test_register_type_name )
{
    Namespace ns;
    BOOST_CHECK_EQUAL( ns.register_type<int>().name(), typeid(int).name() );
    BOOST_CHECK_EQUAL( ns.register_type<float>("real").name(), "real" );
}

BOOST_AUTO_TEST_CASE( test_fallback_getter_member )
{
    using Class = std::unordered_map<std::string, std::string>;
    using Ptr = const Class::mapped_type& (Class::*)(const Class::key_type&) const;
    Namespace ns;
    ns.register_type<Class>()
        .add_readonly("size", &Class::size)
        .fallback_getter(Ptr(&Class::at))
    ;
    ns.register_type<std::string>();
    ns.register_type<Class::size_type>();

    Class map{
        {"foo", "bar"},
        {"hello", "world"},
    };
    auto object = ns.object(map);
    BOOST_CHECK_EQUAL( object.get({"size"}).to_string(), "2" );
    BOOST_CHECK_EQUAL( object.get({"foo"}).to_string(), "bar");
    BOOST_CHECK_EQUAL( object.get({"hello"}).to_string(), "world");
}

BOOST_AUTO_TEST_CASE( test_fallback_getter_functor )
{
    std::unordered_map<std::string, std::string> map{
        {"foo", "bar"},
        {"hello", "world"},
    };
    Namespace ns;
    ns.register_type<SomeClass>("SomeClass")
        .add_readonly("data", &SomeClass::data_member)
        .fallback_getter([&map](const SomeClass& obj, const std::string& name){
            return obj.data_member + ' ' + map[name];
        })
    ;
    ns.register_type<std::string>();

    auto object = ns.object(SomeClass());
    BOOST_CHECK_EQUAL( object.get({"data"}).to_string(), "data member" );
    BOOST_CHECK_EQUAL( object.get({"foo"}).to_string(), "data member bar");
    BOOST_CHECK_EQUAL( object.get({"hello"}).to_string(), "data member world");
}

BOOST_AUTO_TEST_CASE( test_fallback_getter_functor_no_object )
{
    std::unordered_map<std::string, std::string> map{
        {"foo", "bar"},
        {"hello", "world"},
    };
    Namespace ns;
    ns.register_type<SomeClass>("SomeClass")
        .add_readonly("data", &SomeClass::data_member)
        .fallback_getter([&map](const std::string& name){
            return map[name];
        })
    ;
    ns.register_type<std::string>();

    auto object = ns.object(SomeClass());
    BOOST_CHECK_EQUAL( object.get({"data"}).to_string(), "data member" );
    BOOST_CHECK_EQUAL( object.get({"foo"}).to_string(), "bar");
    BOOST_CHECK_EQUAL( object.get({"hello"}).to_string(), "world");
}

BOOST_AUTO_TEST_CASE( test_cast )
{
    Namespace ns;
    ns.register_type<int>();
    ns.register_type<float>();

    BOOST_CHECK_EQUAL( ns.object(123).cast<int>(), 123 );
    BOOST_CHECK_THROW( ns.object(123).cast<float>(), TypeError );
    BOOST_CHECK_THROW( ns.object(123).cast<double>(), TypeError );
}

BOOST_AUTO_TEST_CASE( test_type_name )
{
    Namespace ns;
    ns.register_type<int>("Number");

    BOOST_CHECK_EQUAL( ns.type_name<int>(), "Number" );
    BOOST_CHECK_EQUAL( ns.type_name<float>(), typeid(float).name() );
    BOOST_CHECK_THROW( ns.type_name<float>(true), TypeError );
}

BOOST_AUTO_TEST_CASE( test_has_type )
{
    Namespace ns;
    ns.register_type<int>();
    ns.register_type<float>();

    BOOST_CHECK( ns.object(123).has_type<int>() );
    BOOST_CHECK( !ns.object(123).has_type<float>() );
    BOOST_CHECK( !ns.object(123).has_type<double>() );
}


