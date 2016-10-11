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
#include <boost/optional.hpp> // This is needed to trigger potential issues with boost::get
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
    TypeSystem ns;
    ns.register_type<SomeClass>("SomeClass")
        .add_readonly("data", &SomeClass::data_member)
        .add_readonly("method", &SomeClass::member_function)
    ;
    ns.register_type<std::string>("string");

    BOOST_CHECK_EQUAL( ns.object(std::string("foo")).to_string(), "foo" );
    BOOST_CHECK_EQUAL( ns.object(SomeClass()).to_string(), "SomeClass" );
}

BOOST_AUTO_TEST_CASE( test_getter )
{
    TypeSystem ns;
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
    TypeSystem ns;
    ns.register_type<SomeClass>("SomeClass")
        .add_readonly("data", &SomeClass::data_member)
        .add_readonly("method", &SomeClass::member_function)
    ;
    BOOST_CHECK_THROW( ns.object(SomeClass()).get({"data"}), TypeError );
}

BOOST_AUTO_TEST_CASE( test_builtin )
{
    TypeSystem ns;
    ns.register_type<int>();

    BOOST_CHECK_EQUAL( ns.object(123).to_string(), "123" );
}

BOOST_AUTO_TEST_CASE( test_register_type_name )
{
    TypeSystem ns;
    BOOST_CHECK_EQUAL( ns.register_type<int>().name(), typeid(int).name() );
    BOOST_CHECK_EQUAL( ns.register_type<float>("real").name(), "real" );
}

BOOST_AUTO_TEST_CASE( test_fallback_getter_member )
{
    using Class = std::unordered_map<std::string, std::string>;
    using Ptr = const Class::mapped_type& (Class::*)(const Class::key_type&) const;
    TypeSystem ns;
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
    TypeSystem ns;
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
    TypeSystem ns;
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
    TypeSystem ns;
    ns.register_type<int>();
    ns.register_type<float>();

    BOOST_CHECK_EQUAL( ns.object(123).cast<int>(), 123 );
    BOOST_CHECK_THROW( ns.object(123).cast<float>(), TypeError );
    BOOST_CHECK_THROW( ns.object(123).cast<double>(), TypeError );
}

BOOST_AUTO_TEST_CASE( test_type_name )
{
    TypeSystem ns;
    ns.register_type<int>("Number");

    BOOST_CHECK_EQUAL( ns.type_name<int>(), "Number" );
    BOOST_CHECK_EQUAL( ns.type_name<float>(), typeid(float).name() );
    BOOST_CHECK_THROW( ns.type_name<float>(true), TypeError );
}

BOOST_AUTO_TEST_CASE( test_has_type )
{
    TypeSystem ns;
    ns.register_type<int>();
    ns.register_type<float>();

    BOOST_CHECK( ns.object(123).has_type<int>() );
    BOOST_CHECK( !ns.object(123).has_type<float>() );
    BOOST_CHECK( !ns.object(123).has_type<double>() );
}

struct SomeClassWithMethods
{
    std::string data = "data";

    std::string method_noargs() const
    {
        return "-" + data;
    }

    std::string method_arg(const std::string& arg) const
    {
        return "-" + arg + data;
    }

    std::string method_noconst()
    {
        return "+" + data;
    }

};

BOOST_AUTO_TEST_CASE( test_method_access_method )
{
    TypeSystem ns;
    ns.register_type<SomeClassWithMethods>()
        .add_readonly("data", &SomeClassWithMethods::data)
        .add_method("method_noargs", &SomeClassWithMethods::method_noargs)
        .add_method("method_arg", &SomeClassWithMethods::method_arg)
        .add_method("method_noconst", &SomeClassWithMethods::method_noconst)
    ;
    ns.register_type<std::string>();

    Object object = ns.object(SomeClassWithMethods());
    BOOST_CHECK_EQUAL( object.get({"data"}).to_string(), "data" );

    BOOST_CHECK_EQUAL( object.call("method_noargs", {}).to_string(), "-data");
    BOOST_CHECK_THROW( object.call("method_arg", {}).to_string(), MemberNotFound);
    BOOST_CHECK_EQUAL( object.call("method_noconst", {}).to_string(), "+data");
    Object arg = ns.object<std::string>("foo");
    BOOST_CHECK_THROW( object.call("method_noargs", {arg}).to_string(), MemberNotFound);
    BOOST_CHECK_EQUAL( object.call("method_arg", {arg}).to_string(), "-foodata");

}

std::string function_const(const SomeClass& obj, const std::string& arg)
{
    return obj.data_member + arg;
}

BOOST_AUTO_TEST_CASE( test_method_access_functor_object_const )
{
    TypeSystem ns;
    ns.register_type<SomeClass>()
        .add_method("lambda_noargs", [](const SomeClass& obj) {
            return obj.data_member;
        })
        .add_method("lambda_arg", [](const SomeClass& obj, const std::string& arg) {
            return arg + obj.data_member;
        })
        .add_method("fnptr", &function_const)
        .add_method("lambda_copy", [](SomeClass obj) {
            return obj.data_member;
        })
    ;
    ns.register_type<std::string>();

    Object object = ns.object(SomeClass());
    BOOST_CHECK_EQUAL( object.call("lambda_noargs", {}).to_string(), "data member");
    BOOST_CHECK_THROW( object.call("lambda_arg", {}).to_string(), MemberNotFound);
    BOOST_CHECK_EQUAL( object.call("lambda_copy", {}).to_string(), "data member");
    Object arg = ns.object<std::string>("foo");
    BOOST_CHECK_THROW( object.call("lambda_noargs", {arg}).to_string(), MemberNotFound);
    BOOST_CHECK_EQUAL( object.call("lambda_arg", {arg}).to_string(), "foodata member");
    BOOST_CHECK_EQUAL( object.call("fnptr", {arg}).to_string(), "data memberfoo");

}

std::string function_noconst(SomeClass& obj, const std::string& arg)
{
    return obj.data_member + arg;
}

BOOST_AUTO_TEST_CASE( test_method_access_functor_object_noconst )
{
    TypeSystem ns;
    ns.register_type<SomeClass>()
        .add_method("lambda_noargs", [](SomeClass& obj) {
            return obj.data_member;
        })
        .add_method("lambda_arg", [](SomeClass& obj, const std::string& arg) {
            return arg + obj.data_member;
        })
        .add_method("fnptr", &function_noconst)
    ;
    ns.register_type<std::string>();

    Object object = ns.object(SomeClass());
    BOOST_CHECK_EQUAL( object.call("lambda_noargs", {}).to_string(), "data member");
    BOOST_CHECK_THROW( object.call("lambda_arg", {}).to_string(), MemberNotFound);
    Object arg = ns.object<std::string>("foo");
    BOOST_CHECK_THROW( object.call("lambda_noargs", {arg}).to_string(), MemberNotFound);
    BOOST_CHECK_EQUAL( object.call("lambda_arg", {arg}).to_string(), "foodata member");
    BOOST_CHECK_EQUAL( object.call("fnptr", {arg}).to_string(), "data memberfoo");
}

std::string function_noobject(const std::string& arg)
{
    return arg + arg;
}

BOOST_AUTO_TEST_CASE( test_method_access_functor_object_noobject )
{
    TypeSystem ns;
    ns.register_type<SomeClass>()
        .add_method("lambda_noargs", []() {
            return std::string("noargs");
        })
        .add_method("lambda_arg", [](const std::string& arg) {
            return arg;
        })
        .add_method("fnptr", &function_noobject)
    ;
    ns.register_type<std::string>();

    Object object = ns.object(SomeClass());
    BOOST_CHECK_EQUAL( object.call("lambda_noargs", {}).to_string(), "noargs");
    BOOST_CHECK_THROW( object.call("lambda_arg", {}).to_string(), MemberNotFound);
    Object arg = ns.object<std::string>("foo");
    BOOST_CHECK_THROW( object.call("lambda_noargs", {arg}).to_string(), MemberNotFound);
    BOOST_CHECK_EQUAL( object.call("lambda_arg", {arg}).to_string(), "foo");
    BOOST_CHECK_EQUAL( object.call("fnptr", {arg}).to_string(), "foofoo");
}

BOOST_AUTO_TEST_CASE( test_method_overload )
{
    TypeSystem ns;
    ns.register_type<SomeClassWithMethods>()
        .add_readonly("data", &SomeClassWithMethods::data)
        .add_method("method", &SomeClassWithMethods::method_noargs)
        .add_method("method", &SomeClassWithMethods::method_arg)
        .add_method("method", [](const std::string& arg1, const std::string& arg2) {
            return arg1 + arg2;
        })
    ;
    ns.register_type<std::string>();
    ns.register_type<int>();

    Object object = ns.object(SomeClassWithMethods());
    Object arg = ns.object<std::string>("foo");
    BOOST_CHECK_EQUAL( object.call("method", {}).to_string(), "-data");
    BOOST_CHECK_EQUAL( object.call("method", {arg}).to_string(), "-foodata");
    BOOST_CHECK_EQUAL( object.call("method", {arg, arg}).to_string(), "foofoo");
    BOOST_CHECK_THROW( object.call("method", {arg, arg, arg}).to_string(), MemberNotFound);
    BOOST_CHECK_THROW( object.call("method", {ns.object(1)}).to_string(), MemberNotFound);
}

struct SettableClass
{
    std::string attribute = "value";

    std::string getter() const
    {
        return attribute;
    }

    void setter(const std::string& value)
    {
        attribute = value + "(setter)";
    }
};

BOOST_AUTO_TEST_CASE( test_setter )
{
    std::string other_value;
    TypeSystem ns;
    ns.register_type<SettableClass>()
        .add_readwrite("attribute", &SettableClass::attribute)
        .add_readwrite("property", &SettableClass::getter, &SettableClass::setter)
        .add_readwrite("external", &SettableClass::getter,
            [](SettableClass& obj, const std::string& value) {
                obj.attribute = value + "(lambda)";
        })
        .add_readwrite("unbound",
            [&other_value]() { return other_value; },
            [&other_value](const std::string& value) { other_value = value; }
        )
    ;
    ns.register_type<std::string>();
    ns.register_type<int>();

    auto object = ns.object(SettableClass());
    BOOST_CHECK_EQUAL( object.get({"attribute"}).to_string(), "value" );
    object.set({"attribute"}, ns.object<std::string>("foo"));
    BOOST_CHECK_EQUAL( object.get({"attribute"}).to_string(), "foo" );
    BOOST_CHECK_EQUAL( object.get({"property"}).to_string(), "foo" );
    object.set({"property"}, ns.object<std::string>("bar"));
    BOOST_CHECK_EQUAL( object.get({"property"}).to_string(), "bar(setter)" );
    object.set({"external"}, ns.object<std::string>("foo"));
    BOOST_CHECK_EQUAL( object.get({"external"}).to_string(), "foo(lambda)" );

    object.set({"unbound"}, ns.object<std::string>("bar"));
    BOOST_CHECK_EQUAL( object.get({"external"}).to_string(), "foo(lambda)" );
    BOOST_CHECK_EQUAL( object.get({"unbound"}).to_string(), "bar" );
    BOOST_CHECK_EQUAL( other_value, "bar" );

    BOOST_CHECK_THROW( object.set({"attribute"}, ns.object<int>(1)), TypeError );
    BOOST_CHECK_THROW( object.set({"property"},  ns.object<int>(1)), TypeError );
    BOOST_CHECK_THROW( object.set({"external"}, ns.object<int>(1)), TypeError );
    BOOST_CHECK_THROW( object.set({"unbound"},  ns.object<int>(1)), TypeError );
    BOOST_CHECK_THROW( object.set({"not_found"}, ns.object<int>(1)), MemberNotFound );
}

struct FallbackClass
{
    void set(const std::string& name, const std::string& value)
    {
        attrs[name] = value;
    }

    std::string get(const std::string& name) const
    {
        return attrs.at(name);
    }

    std::unordered_map<std::string, std::string> attrs;
};

BOOST_AUTO_TEST_CASE( test_fallback_setter_member )
{
    TypeSystem ns;
    ns.register_type<FallbackClass>()
        .fallback_getter(&FallbackClass::get)
        .fallback_setter(&FallbackClass::set)
    ;
    ns.register_type<std::string>();

    auto object = ns.object(FallbackClass());
    object.set("foo", ns.object<std::string>("bar"));
    BOOST_CHECK_EQUAL( object.get("foo").to_string(), "bar" );
}

BOOST_AUTO_TEST_CASE( test_fallback_setter_functor )
{
    std::unordered_map<std::string, std::string> map;
    TypeSystem ns;
    ns.register_type<SomeClass>()
        .add_readonly("data", &SomeClass::data_member)
        .fallback_setter(
            [&map](const SomeClass& obj, const std::string& name, const std::string& value){
                map[name] = obj.data_member + value;
        })
    ;
    ns.register_type<std::string>();

    auto object = ns.object(SomeClass());
    BOOST_CHECK_EQUAL( object.get({"data"}).to_string(), "data member" );
    object.set("foo", ns.object<std::string>("bar"));
    BOOST_CHECK_EQUAL( map["foo"], "data memberbar" );
}

BOOST_AUTO_TEST_CASE( test_fallback_setter_functor_no_object )
{
    std::unordered_map<std::string, std::string> map;
    TypeSystem ns;
    ns.register_type<SomeClass>()
        .add_readonly("data", &SomeClass::data_member)
        .fallback_setter(
            [&map](const std::string& name, const std::string& value){
                map[name] = value;
        })
    ;
    ns.register_type<std::string>();

    auto object = ns.object(SomeClass());
    BOOST_CHECK_EQUAL( object.get({"data"}).to_string(), "data member" );
    object.set("foo", ns.object<std::string>("bar"));
    BOOST_CHECK_EQUAL( map["foo"], "bar" );
}

struct Constructible
{
    Constructible(std::string data) : data(data) {}
    std::string data;
};

BOOST_AUTO_TEST_CASE( test_constructor_functor )
{
    TypeSystem ns;
    ns.register_type<Constructible>("Constructible")
        .add_readonly("data", &Constructible::data)
        .constructor([](const std::string& data){
            return Constructible(data);
        })
    ;
    ns.register_type<std::string>();
    ns.register_type<int>();

    auto param = ns.object<std::string>("foo");
    BOOST_CHECK_EQUAL( ns.object("Constructible", {param}).get({"data"}).to_string(), "foo" );
    BOOST_CHECK_THROW( ns.object("Constructible", {param, param}), MemberNotFound );
    BOOST_CHECK_THROW( ns.object("Constructible", {ns.object(1)}), MemberNotFound );
}

BOOST_AUTO_TEST_CASE( test_no_constructor )
{
    TypeSystem ns;
    ns.register_type<Constructible>("Constructible")
        .add_readonly("data", &Constructible::data)
    ;
    ns.register_type<std::string>();

    auto param = ns.object<std::string>("foo");
    BOOST_CHECK_THROW( ns.object("Constructible", {param}), MemberNotFound );
}

BOOST_AUTO_TEST_CASE( test_constructor_noarg )
{
    TypeSystem ns;
    ns.register_type<Constructible>("Constructible")
        .add_readonly("data", &Constructible::data)
        .constructor([](){
            return Constructible("data");
        })
    ;
    ns.register_type<std::string>();

    BOOST_CHECK_EQUAL( ns.object("Constructible", {}).get({"data"}).to_string(), "data" );
}

BOOST_AUTO_TEST_CASE( test_constructor_raw )
{
    TypeSystem ns;
    ns.register_type<Constructible>("Constructible")
        .add_readonly("data", &Constructible::data)
        .constructor<std::string>()
    ;
    ns.register_type<std::string>();
    ns.register_type<int>();

    auto param = ns.object<std::string>("foo");
    BOOST_CHECK_EQUAL( ns.object("Constructible", {param}).get({"data"}).to_string(), "foo" );
    BOOST_CHECK_THROW( ns.object("Constructible", {param, param}), MemberNotFound );
    BOOST_CHECK_THROW( ns.object("Constructible", {ns.object(1)}), MemberNotFound );
}

BOOST_AUTO_TEST_CASE( test_constructor_overload )
{
    TypeSystem ns;
    ns.register_type<Constructible>("Constructible")
        .add_readonly("data", &Constructible::data)
        .constructor([](const std::string& data){
            return Constructible(data);
        })
        .constructor([](const std::string& data, int i){
            return Constructible(data + std::to_string(i));
        })
    ;
    ns.register_type<std::string>();
    ns.register_type<int>();

    auto param = ns.object<std::string>("foo");
    BOOST_CHECK_EQUAL( ns.object("Constructible", {param}).get({"data"}).to_string(), "foo" );
    BOOST_CHECK_THROW( ns.object("Constructible", {param, param}), MemberNotFound );
    BOOST_CHECK_THROW( ns.object("Constructible", {ns.object(1)}), MemberNotFound );
    BOOST_CHECK_EQUAL( ns.object("Constructible", {param, ns.object(1)}).get({"data"}).to_string(), "foo1" );
}

BOOST_AUTO_TEST_CASE( test_converter_explicit )
{
    TypeSystem ns;
    ns.register_type<int>()
        .conversion<float>([](int i) -> float { return i; })
    ;
    ns.register_type<float>();
    ns.register_type<double>();

    auto object = ns.object(1234);
    BOOST_CHECK_EQUAL( object.cast<int>(), 1234 );
    BOOST_CHECK_THROW( object.cast<float>(), TypeError );
    BOOST_CHECK_EQUAL( object.converted_cast<int>(), 1234 );
    BOOST_CHECK_CLOSE( object.converted_cast<float>(), 1234.f, 0.0001 );
    BOOST_CHECK_THROW( object.converted_cast<double>(), MemberNotFound );
}

BOOST_AUTO_TEST_CASE( test_converter_implicit )
{
    TypeSystem ns;
    ns.register_type<int>()
        .conversion([](int i) -> float { return i; })
    ;
    ns.register_type<float>();
    ns.register_type<double>();

    auto object = ns.object(1234);
    BOOST_CHECK_EQUAL( object.cast<int>(), 1234 );
    BOOST_CHECK_THROW( object.cast<float>(), TypeError );
    BOOST_CHECK_EQUAL( object.converted_cast<int>(), 1234 );
    BOOST_CHECK_CLOSE( object.converted_cast<float>(), 1234.f, 0.0001 );
    BOOST_CHECK_THROW( object.converted_cast<double>(), MemberNotFound );
}

BOOST_AUTO_TEST_CASE( test_reference_wrapping )
{
    TypeSystem ns;
    ns.register_type<SomeClass>("SomeClass")
        .add_readwrite("data", &SomeClass::data_member)
    ;
    ns.register_type<std::string>("string");
    SomeClass object;
    Object wrapper = ns.reference(object);
    BOOST_CHECK_EQUAL( wrapper.get("data").to_string(), "data member" );
    wrapper.set("data", ns.object<std::string>("foo"));
    BOOST_CHECK_EQUAL( wrapper.get("data").to_string(), "foo" );
    BOOST_CHECK_EQUAL( object.data_member, "foo" );
}

BOOST_AUTO_TEST_CASE( test_reference_wrapping_ref_tag )
{
    TypeSystem ns;
    ns.register_type<SomeClass>("SomeClass")
        .add_readwrite("data", &SomeClass::data_member)
    ;
    ns.register_type<std::string>("string");
    SomeClass object;
    Object wrapper = ns.object(wrap_reference(object));
    BOOST_CHECK_EQUAL( wrapper.get("data").to_string(), "data member" );
    wrapper.set("data", ns.object<std::string>("foo"));
    BOOST_CHECK_EQUAL( wrapper.get("data").to_string(), "foo" );
    BOOST_CHECK_EQUAL( object.data_member, "foo" );
}

BOOST_AUTO_TEST_CASE( test_reference_wrapping_policy )
{
    TypeSystem ns;
    ns.register_type<SomeClass>("SomeClass")
        .add_readwrite("data", &SomeClass::data_member)
    ;
    ns.register_type<std::string>("string");
    SomeClass object;

    Object reference = ns.bind(object, WrapReferencePolicy{});
    BOOST_CHECK_EQUAL( reference.get("data").to_string(), "data member" );
    reference.set("data", ns.object<std::string>("foo"));
    BOOST_CHECK_EQUAL( reference.get("data").to_string(), "foo" );
    BOOST_CHECK_EQUAL( object.data_member, "foo" );

    Object copy = ns.bind(object, CopyPolicy{});
    BOOST_CHECK_EQUAL( copy.get("data").to_string(), "foo" );
    copy.set("data", ns.object<std::string>("bar"));
    BOOST_CHECK_EQUAL( copy.get("data").to_string(), "bar" );
    BOOST_CHECK_EQUAL( object.data_member, "foo" );
}

BOOST_AUTO_TEST_CASE( test_reference_return_policy )
{
    SomeClass child;
    TypeSystem ns;
    ns.register_type<SomeClass>("SomeClass")
        .add_readwrite("data", &SomeClass::data_member)
        .add_readonly("child",
            [&child]() -> SomeClass& { return child; },
            WrapReferencePolicy{}
        )
    ;
    ns.register_type<std::string>("string");

    Object object = ns.object<SomeClass>();
    Object reference = object.get("child");
    BOOST_CHECK_EQUAL( reference.get("data").to_string(), "data member" );
    reference.set("data", ns.object<std::string>("foo"));
    BOOST_CHECK_EQUAL( reference.get("data").to_string(), "foo" );
    BOOST_CHECK_EQUAL( child.data_member, "foo" );
    BOOST_CHECK_EQUAL( object.get("data").to_string(), "data member" );
}

BOOST_AUTO_TEST_CASE( test_reference_fixed_value_return_policy )
{
    SomeClass child;
    TypeSystem ns;
    ns.register_type<SomeClass>("SomeClass")
        .add_readwrite("data", &SomeClass::data_member)
        .add_readonly("child", child, WrapReferencePolicy{})
        .add_readonly("child_copy", child, CopyPolicy{})
    ;
    ns.register_type<std::string>("string");

    Object object = ns.object<SomeClass>();
    Object reference = object.get("child");
    BOOST_CHECK_EQUAL( reference.get("data").to_string(), "data member" );

    reference.set("data", ns.object<std::string>("foo"));
    BOOST_CHECK_EQUAL( reference.get("data").to_string(), "foo" );
    BOOST_CHECK_EQUAL( child.data_member, "foo" );
    BOOST_CHECK_EQUAL( object.get("data").to_string(), "data member" );

    child.data_member = "bar";
    BOOST_CHECK_EQUAL( reference.get("data").to_string(), "bar" );
    BOOST_CHECK_EQUAL( object.get("data").to_string(), "data member" );

    /// \todo a system where the fixed value is stored as reference but copied on access
    Object copy = object.get("child_copy");
    BOOST_CHECK_EQUAL( copy.get("data").to_string(), "data member" );
    copy.set("data", ns.object<std::string>("foo"));
    BOOST_CHECK_EQUAL( copy.get("data").to_string(), "foo" );
    BOOST_CHECK_EQUAL( child.data_member, "bar" );
}

BOOST_AUTO_TEST_CASE( test_object_forwarding )
{
    TypeSystem ns;
    ns.register_type<std::string>("string")
        .add_method("getobject", [&ns](){return ns.object<std::string>("bar");})
    ;

    Object foo = ns.object<std::string>("foo");
    BOOST_CHECK_EQUAL( foo.call("getobject", {}).to_string(), "bar" );
}

struct NestedClass
{
    SomeClass value;
};

BOOST_AUTO_TEST_CASE( test_reference_member_return_policy )
{
    TypeSystem ns;
    ns.register_type<SomeClass>()
        .add_readwrite("data", &SomeClass::data_member)
    ;
    ns.register_type<NestedClass>()
        .add_readwrite("value", &NestedClass::value, WrapReferencePolicy{})
    ;
    ns.register_type<std::string>();

    Object object = ns.object<NestedClass>();
    Object reference = object.get("value");
    BOOST_CHECK_EQUAL( reference.get("data").to_string(), "data member" );
    reference.set("data", ns.object<std::string>("foo"));
    BOOST_CHECK_EQUAL( reference.get("data").to_string(), "foo" );
    BOOST_CHECK_EQUAL( object.get({"value", "data"}).to_string(), "foo" );
}

BOOST_AUTO_TEST_CASE( test_auto_register )
{
    TypeSystem ns;
    ns.register_type<SimpleType>();
    ns.register_type<std::string>();

    Object object = ns.object<SimpleType>();
    BOOST_CHECK_THROW( object.get("foo").to_string(), MemberNotFound );
    object.set("foo", ns.object<std::string>("bar"));
    BOOST_CHECK_EQUAL( object.get("foo").to_string(), "bar" );
}

BOOST_AUTO_TEST_CASE( test_import_type )
{
    TypeSystem source_ns;
    source_ns.register_type<SomeClass>("SomeClass")
        .add_readonly("data", &SomeClass::data_member)
    ;
    source_ns.register_type<std::string>("string");

    TypeSystem ns;
    BOOST_CHECK_THROW( ns.object<SomeClass>(), TypeError );

    ns.import_type<SomeClass>(source_ns);
    BOOST_CHECK_NO_THROW( ns.object<SomeClass>() );
    BOOST_CHECK_THROW( ns.object<SomeClass>().get("data"), TypeError );

    ns.import_type(source_ns, "string");
    BOOST_CHECK_EQUAL( ns.object<SomeClass>().get("data").to_string(), "data member" );
}

BOOST_AUTO_TEST_CASE( test_import )
{
    TypeSystem source_ns;
    source_ns.register_type<SomeClass>("SomeClass")
        .add_readonly("data", &SomeClass::data_member)
    ;
    source_ns.register_type<std::string>("string");

    TypeSystem ns;
    BOOST_CHECK_THROW( ns.object<SomeClass>(), TypeError );

    ns.import(source_ns);
    BOOST_CHECK_NO_THROW( ns.object<SomeClass>() );
    BOOST_CHECK_EQUAL( ns.object<SomeClass>().get("data").to_string(), "data member" );
}

BOOST_AUTO_TEST_CASE( test_ensure_type_existing )
{
    TypeSystem ns;
    ns.register_type<SomeClass>("SomeClass")
        .add_readonly("data", &SomeClass::data_member)
    ;
    ns.register_type<std::string>("string");
    ns.ensure_type<SomeClass>("SomeClass");
    BOOST_CHECK_EQUAL( ns.object<SomeClass>().get("data").to_string(), "data member" );
}

struct NoCopy
{
    NoCopy() = default;
    NoCopy(const NoCopy&) = delete;
    NoCopy& operator=(const NoCopy&) = delete;
    std::string data = "data member";
};

BOOST_AUTO_TEST_CASE( test_nocopy )
{
    TypeSystem ns;
    ns.register_type<NoCopy>().add_readonly("data", &NoCopy::data);
    ns.register_type<std::string>();
    NoCopy obj;
    auto dynamic = ns.reference(obj);
    BOOST_CHECK_EQUAL( dynamic.get("data").to_string(), obj.data );
}

struct VirtualBase
{
    virtual ~VirtualBase() = default;
    virtual std::string method() const = 0;
};

struct VirtualDerived: VirtualBase
{
    std::string method() const override
    {
        return "foo";
    }
};

BOOST_AUTO_TEST_CASE( test_abstract )
{
    TypeSystem ns;
    ns.register_type<VirtualBase>("VirtualBase")
        .add_readonly("method", &VirtualBase::method)
    ;
    ns.register_type<std::string>();
    VirtualDerived obj;
    auto dynamic = ns.reference<VirtualBase>(obj);
    BOOST_CHECK_EQUAL( dynamic.get("method").to_string(), obj.method() );
    BOOST_CHECK_EQUAL( &dynamic.cast<VirtualBase>(), &obj );
    BOOST_CHECK_EQUAL( dynamic.to_string(), "VirtualBase" );
}

BOOST_AUTO_TEST_CASE( test_iterable )
{
    TypeSystem ns;
    ns.register_type<std::vector<std::string>>("Array")
        .make_iterable()
    ;
    ns.register_type<std::string>();
    std::vector<std::string> array{"foo", "bar"};
    auto dynamic = ns.reference(array);
    std::string out;
    dynamic.iterate([&out](const Object& obj){ out += obj.to_string(); });
    BOOST_CHECK_EQUAL( out, "foobar" );
}

BOOST_AUTO_TEST_CASE( test_iterable_functor )
{
    TypeSystem ns;
    ns.register_type<std::vector<std::string>>("Array")
        .make_iterable(
            [](std::vector<std::string>& vec){ return vec.rbegin(); },
            [](std::vector<std::string>& vec){ return vec.rend(); }
        )
    ;
    ns.register_type<std::string>();
    std::vector<std::string> array{"foo", "bar"};
    auto dynamic = ns.reference(array);
    std::string out;
    dynamic.iterate([&out](const Object& obj){ out += obj.to_string(); });
    BOOST_CHECK_EQUAL( out, "barfoo" );
}

